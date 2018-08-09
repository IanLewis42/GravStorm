/*
	GravStorm
    Copyright (C) 2015-2018 Ian Lewis

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

//Functions for Autonomous ships.

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define ALLEGRO_UNSTABLE 1  //needed for haptics.

#include "allegro5/allegro.h"
#include "allegro5/allegro_image.h"
#include "allegro5/allegro_primitives.h"
#include "allegro5/allegro_font.h"
#include "allegro5/allegro_ttf.h"
#include "allegro5/allegro_audio.h"
#include "allegro5/allegro_acodec.h"
#ifdef ANDROID
  #include <allegro5/allegro_android.h>
#endif

#include "game.h"
#include "drawing.h"
#include "init.h"
#include "collisions.h"
#include "objects.h"
#include "inputs.h"
#include "gameover.h"
#include "network.h"
#include "auto.h"

void Takeoff(int i);
void Cruise(int i);
void Hunt(int i);
void Return(int i);
void Land(int i);

void Pilot1(int i);
void Pilot2(int i);


WallType walls[40];

float sumx,sumy,ratio;
int avoid_angle,target_angle;
float targetx,targety,avoidx,avoidy;

void AutoShip(int i)
{
    switch(Ship[i].automode)
    {
        case MANUAL:
        break;
        case TAKEOFF:
            Takeoff(i);
        break;
        case CRUISE:
            Cruise(i);
        break;
        case HUNT:
            Hunt(i);
        break;
        case RETURN:
            Return(i);
        break;
        case LAND:
            Land(i);
        break;
        default:
        break;
    }
    return;
}

void GotoTakeoff(int i)
{
    Ship[i].automode = TAKEOFF;
    Ship[i].automodechangetime = Map.timer+10;
}

void GotoCruise(int i)
{
    Ship[i].automode = CRUISE;                  //set state variable
    Ship[i].autoalgorithm = 1;                  //choose initial pilot algorithm

    Ship[i].autoratio = 0;                      //reset pilot algorithms
    Ship[i].autovectintegrator = 5000;
    Ship[i].autovsqint = 200;

    Ship[i].autotargetx = Map.pad[i].exit_x;
    Ship[i].autotargety = Map.pad[i].exit_y;
    Ship[i].autotargetchangetime = Map.timer+150;

    Ship[i].autotargetdistance = 200;
}

void GotoHunt(int i)
{
    Ship[i].automode = HUNT;                  //set state variable
    Ship[i].autoalgorithm = 1;                //choose initial pilot algorithm

    Ship[i].autoratio = 0;                    //reset pilot algorithms
    Ship[i].autovectintegrator = 5000;
    Ship[i].autovsqint = 200;

    Ship[i].autofire1toggletime = Map.timer+30;
}
void GotoReturn(int i)
{
    Ship[i].automode = RETURN;                  //set state variable
    Ship[i].autoalgorithm = 1;                  //choose initial pilot algorithm

    Ship[i].autoratio = 0;                      //reset pilot algorithms
    Ship[i].autovectintegrator = 5000;
    Ship[i].autovsqint = 200;

    Ship[i].autotargetx = Map.pad[i].return_x;  //Set target
    Ship[i].autotargety = Map.pad[i].return_y;
}

void GotoLand(i)
{
    Ship[i].fangle = 0;
    Ship[i].thrust_held = false;
    Ship[i].automode = LAND;
}

void Takeoff(int i)
{
    if (Map.timer == Ship[i].automodechangetime)
    {
        GotoCruise(i);
    }

    Ship[i].fangle = 0.0f;                //fangle range is 0-360
    Ship[i].saved_angle = 0.0f;
    Ship[i].angle = 0;
    Ship[i].thrust_held = (Ship[i].yv < 4); //Map.auto_v
}

#define OOR2 0.707  //One Over Root 2

void Cruise(int i)
{
	//int j,max,min,closest_wall,wall_threshold=15,limit=200; //w_t from mapfile
    //int deviation,temp_angle;
    //int change;
    //int average_walls[8];
    //float v_target = 36.0;
    //float v_hys = 0.0;

    //Ship[i].autotimer=30;
    if (Map.timer == Ship[i].autotargetchangetime ||
        Ship[i].autotargetdistance < 100)
    //if (Ship[i].autovectintegrator < 2000)
    {
        Ship[i].autotargetchangetime = Map.timer+300;

        do
        {
            Ship[i].autotargetx = rand() % mapx;
            Ship[i].autotargety = rand() % mapy;
        }while (!IsClear(Ship[i].autotargetx,Ship[i].autotargety));

        if(Map.type != 1)   //non-tiled maps, switch to vector sum algorithm on first target change.
        {
            Ship[i].autoalgorithm = 0;
            Ship[i].autoratio = 0;
            Ship[i].autostate = NORMAL;
            Ship[i].autovectintegrator = 5000;
        }
        else
        {
            Ship[i].autoalgorithm = 0;
            Ship[i].autovsqint = 200;
        }
    }

    //Ship[i].autotargetx = 0.0*Ship[i].autotargetx + 1.0*Ship[i].autonewtargetx;
    //Ship[i].autotargety = 0.0*Ship[i].autotargety + 1.0*Ship[i].autonewtargety;
    //if(Map.type == 1)
    if (Ship[i].autoalgorithm)
        Pilot2(i);
    else
        Pilot1(i);

    if (Ship[i].fuel>>4 < 20)
    {
        GotoReturn(i);
    }

    //Ship[i].autotimer--;
    //if (Ship[i].autotimer == 0)
    //{
    //    Ship[i].autotimer = 30;
    //    change = (rand()%20) - 10;
    //    Ship[i].angle += change;
    //}

    /*PILOT
    find_walls(i);

    float limit45 = limit * OOR2;

    avoidx = -1*(0 + (limit45-walls[1].x) + (limit-walls[2].x) + (limit45-walls[3].x) + 0 + (-1*limit45-walls[5].x) + (-1*limit-walls[6].x) + (-1*limit45-walls[7].x)) ;
    avoidy =  1*((-1*limit-walls[0].y) + (-1*limit45-walls[1].y) + 0 + (limit45-walls[3].y) + (limit-walls[4].y) + (limit45-walls[5].y) + 0 + (-1*limit45-walls[7].y)) ;

    //get unit-length 'target' vector
    //could do (subtract) pythagoras, divide. dunno which is quicker, sqare root & 2 divides, or atan?
    target_angle = (int)((20/PI)* atan2(Ship[i].autotargetx-Ship[i].xpos, -1*(Ship[i].autotargety-Ship[i].ypos))); //angle to head for target position
    if (target_angle < 0) target_angle += 40;
    targetx = limit*sinlut[target_angle];
    targety = limit*coslut[target_angle];

    ratio = 0.45f;
    */


    // fly away from from min direction (unfinished....

/*
    //find closest wall
    min = 0;
    for (j=0 ; j<8 ; j++)
    {
        if (walls[j].distance < walls[min].distance)
            min = j;
    }
*/
    //Get unit-length 'avoid' vector
/*
    avoid_angle = ((min+4)%8)*5;    //angle to fly away from closest wall
    avoidx = sinlut[avoid_angle];
    avoidy = coslut[avoid_angle];
*/
/*
    //Alternate avoid vector by vector sum. Seems worse, sadly...
    #define OOR2 0.707  //One Over Root 2

    avoidx = 1.0 * (float)(walls[2] - walls[6]) + (OOR2 * (float)(walls[1] + walls[3] - walls[5] - walls[7]));
    avoidy = 1.0 * (float)(walls[4] - walls[0]) + (OOR2 * (float)(walls[3] + walls[5] - walls[1] - walls[7]));
    //normalise
    avoid_angle =   (int)((20/PI)* atan2(avoidx, -1*avoidy));
    if (avoid_angle < 0) avoid_angle += 40;
    avoidx = sinlut[avoid_angle];
    avoidy = coslut[avoid_angle];
*/

/*
    //get unit-length 'target' vector
    //could do (subtract) pythagoras, divide. dunno which is quicker, sqare root & 2 divides, or atan?
    target_angle = (int)((20/PI)* atan2(Ship[i].autotargetx-Ship[i].xpos, -1*(Ship[i].autotargety-Ship[i].ypos))); //angle to head for target position
    if (target_angle < 0) target_angle += 40;
    targetx = sinlut[target_angle];
    targety = coslut[target_angle];


    //work out ratios of 2 vectors
    ratio = walls[min] - wall_threshold;
    if (ratio < 0) ratio = 0;
    ratio = 0.2+(ratio * 0.8/(limit-wall_threshold));   //factor for target

*/
/*PILOT
    //weighted sum of vectors
    sumx = ratio*targetx + (1-ratio)*avoidx;
    sumy = ratio*targety + (1-ratio)*avoidy;

    sumx -= 4*Ship[i].xv;
    sumy -= 4*Ship[i].yv;


    //atan to get angle from vector.
    Ship[i].fangle = (180/PI)*atan2(sumx, sumy);                //fangle range is 0-360
    if (Ship[i].fangle < 0) Ship[i].fangle +=360;


    if (v_squared < 16.0f)
        Ship[i].thrust_held = ((sumx*sumx + sumy*sumy) > 2000); //Map.auto_v
    else
        Ship[i].thrust_held = false;
     */


     //Ship[i].fangle = ((min+4)%8)*5*9;    //fly away from min vector
     //destination

    //proportions from walls[min]
    //distance dest away
    //min      0.1  0.9
    //max      1.0  0.0

    //dest = (dist-min)*0.9 + 0.1;
    //away = 1-dest;

    //need Ship[].autodestination (float angle??)
    //how to average between them??

    //might still need velocity correction..... or maybe not?



/*
    if (walls[min] < wall_threshold)  //deviate to find closest direction that we can safely fly.
    {
         Ship[i].fangle = ((min+4)%8)*5*9;
    }

*/

   // fly in max direction
/*
    max = 0;
    for (j=0 ; j<8 ; j++)
    {
        if (walls[j] > walls[max])
            max = j;
    }
    Ship[i].fangle = max*5*9;                //fangle range is 0-360
*/

/*
   // fly in average max direction
    for (j=0 ; j<8 ; j++)
    {
       int before = (j-1);
       if (before < 0)
        before += 8;

       int after  = (j+1) % 8;

       average_walls[j] = before + walls[j] + after;
    }


    max = 0;
    for (j=0 ; j<8 ; j++)
    {
        if (average_walls[j] > average_walls[max])
            max = j;
    }
    Ship[i].fangle = max*5*9;                //fangle range is 0-360

*/


    //Vector sum
/*
    #define OOR2 0.707  //One Over Root 2

    sumx = (float)(walls[2] - walls[6]) + (OOR2 * (float)(walls[1] + walls[3] - walls[5] - walls[7]));
    sumy = (float)(walls[4] - walls[0]) + (OOR2 * (float)(walls[3] + walls[5] - walls[1] - walls[7]));

    Ship[i].fangle = (180/PI)*atan2(sumx, -sumy);                //fangle range is 0-360
    if (Ship[i].fangle < 0) Ship[i].fangle +=360;
*/


/*
//THIS WAS IN *IPL
    //work out ratios of 2 vectors
    ratio = walls[min] - wall_threshold;
    if (ratio < 0) ratio = 0;
    ratio = 0.2+(ratio * 0.8/(limit-wall_threshold));   //factor for target


    //ratio = walls[min] - wall_threshold*2;
    //if (ratio < 0) ratio = 0;
    */
    /*
    float v = v_target;// * (0.5 + ratio * 0.5/(limit-wall_threshold*2));



    //variable speed???
    if (Ship[i].thrust_held)
        Ship[i].thrust_held = (v_squared < v + v_hys); //Map.auto_v
    else
        Ship[i].thrust_held = (v_squared < v - v_hys); //Map.auto_v
*/
    //Ship[i].fire1_held = distance[min] < 400;
    //Ship[i].fire2_down = distance[min] < 200;

}

void Pilot2(int i)
{
    int j,close_walls = 0;
    float limit = 200,v_squared;
    float x,y,target_distance,av_wall;
    WallType target_wall;

    //get 'target' vector
    x = Ship[i].autotargetx-Ship[i].xpos;
    y = Ship[i].autotargety-Ship[i].ypos;

    Ship[i].autotargetdistance = sqrt((x*x) + (y*y));   //and distance

    target_angle = (int)((20/PI)* atan2(x, -1*y)); //angle to head for target position
    if (target_angle < 0) target_angle += 40;

    find_walls40(i);                          //find all wall distance/angles

    int metric = 0,max=-1000,best_angle;    //decide which of 40 directions is best compromise of 'where we want to go' and 'won't hit a wall'

    for (j=0 ; j<40 ; j++)
    {
        metric = walls[j].distance-3*abs(j-target_angle);

        if (metric >= max)
        {
            max = metric;
            best_angle = j;
        }
    }

    targetx = limit*sinlut[best_angle]; //get xy. from angle
    targety = limit*coslut[best_angle];

    targetx -= 35*Ship[i].xv;           //correct for current velocity
    targety -= 35*Ship[i].yv;

    //targetx -= 35*Ship[i].;
    targety -= 20*Ship[i].gravity;      //and gravity

    //atan to get angle from vector.
    Ship[i].fangle = (180/PI)*atan2(targetx, targety);          //fangle range is 0-360
    if (Ship[i].fangle < 0) Ship[i].fangle +=360;               //wrap

    v_squared = Ship[i].xv*Ship[i].xv + Ship[i].yv*Ship[i].yv;  //current velocity (squared)

    Ship[i].autovsqint = 0.99*Ship[i].autovsqint + 0.1*v_squared; //average

    //if we're stuck, switch to other algorithm.
    if (Ship[i].autovsqint < 120)
    {
        Ship[i].autoalgorithm = 0;
    }

    if (v_squared < 25.0f)                                      //if less than max (desired) velocity
        //if (walls[best_angle].distance > 100)
        if (walls[(int)(Ship[i].fangle/9)].distance > 50)
            Ship[i].thrust_held = true;
        else
            Ship[i].thrust_held = false;
        //Ship[i].thrust_held = (walls[best_angle].distance > 100);                  //thrust if vector is big
    else
        Ship[i].thrust_held = false;                            //off if not.

}

void Pilot1(int i)
{
    int j,close_walls = 0;
    float limit = 200,v_squared;
    float x,y,target_distance,av_wall;
    WallType target_wall;

    find_walls40(i);                          //find all wall distance/angles

    av_wall = 0;                            //average them, to get a feel for how tight space is.
    for (j=0 ; j<40 ; j++)
    {
        av_wall += walls[j].distance;
        if (walls[j].distance != limit)
            close_walls++;
    }

    av_wall /=40;

    float limit45 = limit * OOR2;

    //invert wall vectors (so close walls have large effect) and sum them to get a 'this way to avoid the walls' vector
    //avoidx = -1*(0 + (limit45-walls[1].x) + (limit-walls[2].x) + (limit45-walls[3].x) + 0 + (-1*limit45-walls[5].x) + (-1*limit-walls[6].x) + (-1*limit45-walls[7].x)) ;
    //avoidy =  1*((-1*limit-walls[0].y) + (-1*limit45-walls[1].y) + 0 + (limit45-walls[3].y) + (limit-walls[4].y) + (limit45-walls[5].y) + 0 + (-1*limit45-walls[7].y)) ;

    avoidx=0;
    avoidy=0;

    for (j=0 ; j<40 ; j++)
    {
        avoidx-=0.2*(limit-walls[j].distance) * sinlut[j];
        avoidy-=0.2*(limit-walls[j].distance) * coslut[j];
    }


    if (close_walls == 0)
        close_walls = 1;

    //avoidx *= 8.0/close_walls;
    //avoidy *= 8.0/close_walls;

    //get 'target' vector
    //could do (subtract) pythagoras, divide. dunno which is quicker, sqare root & 2 divides, or atan?
    x = Ship[i].autotargetx-Ship[i].xpos;
    y = Ship[i].autotargety-Ship[i].ypos;

    //state machine, if we're stuck, switch target vector by +/- 90 degrees for a few seconds.
    switch (Ship[i].autostate)
    {
        case NORMAL:
            target_angle = (int)((20/PI)* atan2(x, -1*y)); //angle to head for target position
            if (target_angle < 0) target_angle += 40;

            if (Ship[i].autovectintegrator < 1900)
            {
                if (rand()&1)
                {
                    Ship[i].autoalgorithm = 1;
                    Ship[i].autovsqint = 200;
                }
                else
                {
                    if (rand()&1){
                        Ship[i].autostate = ESCAPE_PLUS;}
                    else{
                        Ship[i].autostate = ESCAPE_MINUS;}

                    Ship[i].autostatechangetime = Map.timer + 150;
                    Ship[i].autoratio = 0;
                }
            }
        break;
        case ESCAPE_PLUS:
            target_angle = (int)((20/PI)* atan2(x, -1*y))+5; //angle to head for target position
            if (target_angle < 0)  target_angle += 40;
            if (target_angle > 40) target_angle -= 40;

            if (Map.timer == Ship[i].autostatechangetime)
            {
               Ship[i].autostate = NORMAL;
               Ship[i].autoratio = 0;
            }
        break;
        case ESCAPE_MINUS:
            target_angle = (int)((20/PI)* atan2(x, -1*y))-5; //angle to head for target position
            if (target_angle < 0)  target_angle += 40;
            if (target_angle > 40) target_angle -= 40;

            if (Map.timer == Ship[i].autostatechangetime)
            {
               Ship[i].autostate = NORMAL;
               Ship[i].autoratio = 0;
            }
        break;
    }

    Ship[i].autotargetdistance = sqrt((x*x) + (y*y));

    if (Ship[i].autotargetdistance > limit)                //limit, so it doesn't override avoid vector
        Ship[i].autotargetdistance = limit;

    targetx = Ship[i].autotargetdistance*sinlut[target_angle]; //get xy. possibly only used for drawing debug....
    targety = Ship[i].autotargetdistance*coslut[target_angle];

    find_wall(i,target_angle,&target_wall);

    //if (target_wall.distance == limit)
    //    Ship[i].autoratio  = 0.8;
    //else
      //ratio = 0.5f;                                 //0.3 /800 good for ekolos.
      ratio = 0.30+av_wall/800;                       //ratio is used for summing target/avoid vectors. adjust for 'tightness'

    Ship[i].autoratio = 0.9*Ship[i].autoratio + 0.1*ratio;
    ratio = Ship[i].autoratio;

    //weighted sum of vectors
    sumx = ratio*targetx + (1-ratio)*avoidx;
    sumy = ratio*targety + (1-ratio)*avoidy;

    sumx -= 4*Ship[i].xv;                       //compensate for current velocity
    sumy -= 4*Ship[i].yv;

    sumy -= 50*Ship[i].gravity;

    float summag = sumx*sumx + sumy*sumy;       //get (squared) magnitude of resultant vector

    Ship[i].autovectintegrator *= 0.97 ;        //and leaky-integrator average it.
    Ship[i].autovectintegrator += 0.03 * summag;

    //atan to get angle from vector.
    Ship[i].fangle = (180/PI)*atan2(sumx, sumy);                //fangle range is 0-360
    if (Ship[i].fangle < 0) Ship[i].fangle +=360;   //wrap

    v_squared = Ship[i].xv*Ship[i].xv + Ship[i].yv*Ship[i].yv;  //current velocity (squared)

    if (v_squared < 25.0f)                                      //if less than max (desired) velocity
        Ship[i].thrust_held = (summag > 2000);                  //thrust if vector is big
    else
        Ship[i].thrust_held = false;                            //off if not.

}





void Hunt(int i)   //so we are ship[i]
{
	int j,k,min,closest_wall,wall_threshold=200; //w_t from mapfile
	float dx,dy,distance[MAX_SHIPS-1],angle[MAX_SHIPS-1];
    int deviation,temp_angle;

	for (j=0 ; j<num_ships ; j++)
	{
		if (i==j)
		{
			distance[j] = 9999999;	//big, so we shouldn't choose it
			angle[j] = 120;	//degrees, hopefully noticeable if we do choose it!
		}
		else
		{
			dx = -1*(Ship[i].xpos - Ship[j].xpos);
			dy =     Ship[i].ypos - Ship[j].ypos;

			//for each ship, work out distance, work out direction

	/*		___o___Opponent(Ship[j])
			|     /
			|    /
		       a|   /
			|  /
			|t/
			|/
			Us(Ship[i])

			tan(t) = o/a
			t = arctan o/a
	*/
			distance[j] = sqrt((dx * dx) + (dy * dy));
			angle[j] = (20/PI) * atan2(dx,dy);	//40 increments
			if (angle[j] < 0) angle[j] += 40;
        }
	}
    //find closest
    min = i;
    for (k=0 ; k<num_ships ; k++)
    {
        if (Ship[k].reincarnate_timer == 0)     //don't consider dead ships
            if (distance[k] < distance[min])
                min = k;
    }


    Ship[i].autotargetx = Ship[min].xpos;
    Ship[i].autotargety = Ship[min].ypos;
/*
    if (distance[min] > 200 && Ship[i].autovectintegrator < 2000)
    {
        Ship[i].autonewtargetx += (rand()%500 - 250);
        Ship[i].autonewtargety += (rand()%500 - 250);
    }
*/
    //Ship[i].autotargetx = 0.9*Ship[i].autotargetx + 0.1*Ship[i].autonewtargetx;
    //Ship[i].autotargety = 0.9*Ship[i].autotargety + 0.1*Ship[i].autonewtargety;

    Pilot1(i);

    //if (Map.timer == Ship[i].autofire1toggletime)
    //{
    //    Ship[i].autofire1 = !Ship[i].autofire1;
    //    Ship[i].autofire1toggletime = Map.timer+30;
    //}

    Ship[i].fire1_held = false;
    Ship[i].fire2_down = false;

    if (min != i)   //don't target ourselves if we're the only live ship.
    {
        if (distance[min]<100)
        {
            Ship[i].fangle = angle[min] * 9;
            if (Ship[i].angle == angle[min])
            {
                Ship[i].thrust_held = false;
                Ship[i].fire1_held = true;
                Ship[i].fire2_down = (rand()%100 < 10);
            }
        }

        else if (distance[min]<300)
        {
            if (abs(Ship[i].angle - angle[min]) < 2)
            {
                if (Ship[i].fire1_reload == 0)
                    Ship[i].fire1_held = (rand()%10 < 5);

                //if (distance[min] < 150)
                    Ship[i].fire2_down = (rand()%100 < 3);
            }
        }
    }

    if ((Ship[i].ammo1 == 0 && Ship[i].ammo2 == 0) || Ship[i].fuel>>4 < 20)
    {
        GotoReturn(i);
    }


    //Ship[i].autolastdmin = distance[min];

    return;



//    closest_wall = find_wall(i,angle[k]);

    if (closest_wall < wall_threshold)  //deviate to find closest direction that we can safely fly.
    {
        for (deviation = 1 ; deviation < 40 ; deviation += 1)
        {
            temp_angle = ((int)angle[min] + deviation)%40;

            //if (find_wall(i,temp_angle) < wall_threshold)
                break;

            temp_angle = (int)angle[min] - deviation;
            if (temp_angle < 0) temp_angle += 40;

            //if (find_wall(i,temp_angle) < wall_threshold)
                break;

        }
    }
    angle[min] = temp_angle;

    Ship[i].fangle = angle[min]*9;                //fangle range is 0-360
    //Ship[i].thrust_held = (v_squared < 16.0f); //Map.auto_v
    Ship[i].fire1_held = distance[min] < 400;
    Ship[i].fire2_down = distance[min] < 200;
}


void Return(int i)
{
    if (Ship[i].autotargetdistance < 100)
    {
         Ship[i].autotargetx = (Map.pad[i].min_x + Map.pad[i].max_x)/2;
         Ship[i].autotargety = Map.pad[i].y-100;
         Ship[i].autoalgorithm = 1;
         Ship[i].autovsqint = 200;
    }

    if (Ship[i].autoalgorithm)
        Pilot2(i);
    else
        Pilot1(i);


    if(Ship[i].xpos > Map.pad[i].min_x+5 &&
       Ship[i].xpos < Map.pad[i].max_x-5 &&
       Ship[i].ypos < Map.pad[i].y     &&
       Ship[i].ypos > Map.pad[i].y - 230 &&
       fabsf(Ship[i].xv) < 0.5f)
    {
        GotoLand(i);
    }
}

void Land(int i)
{
    if (Ship[i].fuel>>4 == Ship[i].user_fuel &&
        Ship[i].ammo1 == Ship[i].user_ammo1 &&
        Ship[i].ammo2 == Ship[i].user_ammo2)
    {
        GotoTakeoff(i);
    }

}

void find_walls(int i)
{
    int j;
    for (j=0 ; j<8 ; j++)
    {
        find_wall(i,j*5,&walls[j]);
    }
}

void find_walls40(int i)
{
    int j;
    for (j=0 ; j<40 ; j++)
    {
        find_wall(i,j,&walls[j]);
    }
}





