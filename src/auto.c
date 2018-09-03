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

#define MODE_DURATION   120 //600

void Takeoff(int i);
void Cruise(int i);
void Hunt(int i);
void Return(int i);
void Land(int i);

void Pilot0(int i);
void Pilot1(int i);


WallType walls[40];
WallType ship_av[40];//[MAX_SHIPS];

float sumx,sumy,ratio;
int avoid_angle,target_angle;
float targetx,targety,avoidx,avoidy;
int fire_state;

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

//Mode transition functions
void GotoTakeoff(int i)
{
    Ship[i].automode = TAKEOFF;
    Ship[i].automodechangetime = Map.timer+10;

    Ship[i].ammo1_type = rand()%3;

    do{
        Ship[i].ammo2_type = BLT_HEAVY + rand()%3;
    }while (Ship[i].ammo2_type == BLT_MINE);

    //Menu.difficulty = 1;
}

void GotoCruise(int i)
{
    Ship[i].automode = CRUISE;                  //set state variable
    if (Menu.difficulty > 0)
    {
        Ship[i].automodechangetime = Map.timer + MODE_DURATION;
    }

    Ship[i].autoalgorithm = 1;                  //choose initial pilot algorithm

    Ship[i].autoratio = 0;                      //reset pilot algorithms
    Ship[i].autovectintegrator = 5000;
    Ship[i].autovsqint = 200;

    Ship[i].autotargetx = Map.pad[i].exit_x;
    Ship[i].autotargety = Map.pad[i].exit_y;
    Ship[i].autotargetchangetime = Map.timer+150;

    Ship[i].autotargetdistance = 200;

    Ship[i].fire1_held = false;
}

void GotoHunt(int i)
{
    Ship[i].automode = HUNT;                  //set state variable
    if (Menu.difficulty < 3)
    {
        Ship[i].automodechangetime = Map.timer + MODE_DURATION;
    }

    Ship[i].autoalgorithm = 0;                //choose initial pilot algorithm

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

void GotoLand(int i)
{
    Ship[i].fangle = 0;
    Ship[i].thrust_held = false;
    Ship[i].automode = LAND;
}

//Functions for each state
void Takeoff(int i)
{
    if (Map.timer == Ship[i].automodechangetime)
    {
        //if (i&1)
            GotoCruise(i);
        //else
        //    GotoHunt(i);
    }

    Ship[i].fangle = 0.0f;                //fangle range is 0-360
    Ship[i].saved_angle = 0.0f;
    Ship[i].angle = 0;
    Ship[i].thrust_held = (Ship[i].yv < 4);
}

void Cruise(int i)
{
    if (Map.timer >= Ship[i].automodechangetime)
    {
        //Ship[i].automodechangetime = Map.timer+MODE_DURATION;
        switch(Menu.difficulty)
        {
            case 0:
            break;
            case 1:
                if (rand()%10 > 7) GotoHunt(i);
            break;
            case 2:
                if (rand()%10 > 3) GotoHunt(i);
            break;
            case 3:
                GotoHunt(i);
            break;
        }
    }

    //if timer expires, or we get close to target, get a new one
    if (Map.timer >= Ship[i].autotargetchangetime || Ship[i].autotargetdistance < 100)
    {
        Ship[i].autotargetchangetime = Map.timer+300;   //set new timer

        do
        {
            Ship[i].autotargetx = rand() % mapx;
            Ship[i].autotargety = rand() % mapy;
        }while (!IsClear(Ship[i].autotargetx,Ship[i].autotargety)); //make sure target isn't in a wall.

        if(Map.type != 1)   //non-tiled maps, switch to vector sum algorithm on target change (and reset memory)
        {
            Ship[i].autoalgorithm = 0;
            Ship[i].autoratio = 0;
            Ship[i].autostate = NORMAL;
            Ship[i].autovectintegrator = 5000;
        }
        else                //tiled maps, switch to mimum-deviation algorithm
        {
            Ship[i].autoalgorithm = 0;
            Ship[i].autovsqint = 200;
        }
    }

    if (Ship[i].autoalgorithm)  //run pilot algorithm
        Pilot1(i);
    else
        Pilot0(i);

    if (Ship[i].fuel>>4 < 20)   //go and refuel if we're low.
    {
        GotoReturn(i);
    }
}

void Hunt(int i)   //so we are ship[i]
{
	int j,k,min;//,closest_wall,wall_threshold=200; //w_t from mapfile
	float dx,dy,distance[MAX_SHIPS-1],angle[MAX_SHIPS-1];
    //int deviation,temp_angle;

    if (Map.timer >= Ship[i].automodechangetime)
    {
        //Ship[i].automodechangetime = Map.timer+MODE_DURATION;//600;
        switch(Menu.difficulty)
        {
            case 0:
                GotoCruise(i);
            break;
            case 1:
                if (rand()%10 > 3) GotoCruise(i);
            break;
            case 2:
                if (rand()%10 > 7) GotoCruise(i);
            break;
            case 3:
            break;
        }
    }

    find_ships(i);

    //find closest ship
    min = i;
    for (k=0 ; k<num_ships ; k++)
    {
        if (Ship[k].reincarnate_timer == 0)     //don't consider dead ships
            if (ship_av[k].distance < ship_av[min].distance)
                min = k;
    }

    Ship[i].autotargetx = Ship[min].xpos;
    Ship[i].autotargety = Ship[min].ypos;

    if (Ship[i].autoalgorithm)
        Pilot1(i);
    else
        Pilot0(i);

    //if (Map.timer == Ship[i].autofire1toggletime)
    //{
    //    Ship[i].autofire1 = !Ship[i].autofire1;
    //    Ship[i].autofire1toggletime = Map.timer+30;
    //}

    Ship[i].fire1_held = false;
    Ship[i].fire2_down = false;

    fire_state = 0;

    if (min != i)   //don't target ourselves if we're the only live ship.
    {
        fire_state = 1;

        /*if (ship_av[min].distance<100)
        {
            Ship[i].fangle = ship_av[min].angle * 9;
            if (Ship[i].angle == ship_av[min].angle)
            {
                Ship[i].thrust_held = false;
                Ship[i].fire1_held = true;
                Ship[i].fire2_down = (rand()%100 < 10);
            }
        }


        else */
        if (ship_av[min].distance<200)              //if we're close enough to the nearest ship
        {
            fire_state = 2; //DEBUG
            Ship[i].autoalgorithm = 0;              //1 is too crashy....

            if ((walls[ship_av[min].angle].distance * 2 + 10) >= (int)Ship[i].autotargetdistance)   //if the ship is closer than the wall (/2 because ship pos is inserted
            {                                                                               //into walls[] at half real distance to minimize ship-shup crashes
                fire_state = 3; //DEBUG
                if (abs(Ship[i].angle - ship_av[min].angle) < 2)                            //if we're pointing roughly the right way
                {
                    fire_state = 4; //DEBUG
                    Ship[i].fire1_held = true;                                              //FIRE 1
                    Ship[i].fire2_down = (rand()%100 < 3);                                  //and maybe fire 2
                }
            }
        }
        else if (ship_av[min].distance<500)              //if we're close enough to the nearest ship
        {
            if ((walls[ship_av[min].angle].distance) == 200)
            {
                if (abs(Ship[i].angle - ship_av[min].angle) < 2)                            //if we're pointing roughly the right way
                {
                    Ship[i].fire1_held = (rand()%100 < 50);                                              //FIRE 1
                    Ship[i].fire2_down = (rand()%100 < 1);                                  //and maybe fire 2
                }
            }


        }
    }

    if (Ship[i].ammo1 == 0 || Ship[i].fuel>>4 < 20)
    {
        Ship[i].fire1_held = false;
        Ship[i].fire2_down = false;
        GotoReturn(i);
    }

    return;
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
        Pilot1(i);
    else
        Pilot0(i);


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

//'minimum deviation' algorithm
void Pilot1(int i)
{
    int j;//,close_walls = 0;
    float limit = 200,v_squared;
    float x,y;//,target_distance;//,av_wall;
    //WallType target_wall;

    //get 'target' vector
    x = Ship[i].autotargetx-Ship[i].xpos;
    y = Ship[i].autotargety-Ship[i].ypos;

    Ship[i].autotargetdistance = sqrt((x*x) + (y*y));   //and distance

    target_angle = (int)((20/PI)* atan2(x, -1*y)); //angle to head for target position
    if (target_angle < 0) target_angle += 40;

    find_obstructions(i);                         //find all wall distance/angles

    /*
    for (j=0 ; j<num_ships ; j++)
    {
        if (ship_av[j].distance < walls[ship_av[j].angle].distance)
            walls[ship_av[j].angle].distance = ship_av[j].distance;
    }
*/
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

    sumx = limit*sinlut[best_angle]; //get xy. from angle
    sumy = limit*coslut[best_angle];

    sumx -= 35*Ship[i].xv;           //correct for current velocity
    sumy -= 35*Ship[i].yv;

    //sumx -= 35*Ship[i].;
    sumy -= 20*Ship[i].gravity;      //and gravity

    //atan to get angle from vector.
    Ship[i].fangle = (180/PI)*atan2(sumx, sumy);          //fangle range is 0-360
    if (Ship[i].fangle < 0) Ship[i].fangle +=360;               //wrap

    v_squared = Ship[i].xv*Ship[i].xv + Ship[i].yv*Ship[i].yv;  //current velocity (squared)

    Ship[i].autovsqint = 0.99*Ship[i].autovsqint + 0.1*v_squared; //average

    //if we're stuck, switch to other algorithm.
    if (Ship[i].autovsqint < 120)
    {
        Ship[i].autoalgorithm = 0;
    }

    static float threshold = 25.0;

    if (v_squared < threshold && walls[best_angle].distance > 100)
    {
        Ship[i].thrust_held = true;
        //threshold = 30.0;
    }
    else
    {
        Ship[i].thrust_held = false;
        //threshold = 20.0;
    }

/*
    if (v_squared < 25.0f)//threshold)//25.0f)                                      //if less than max (desired) velocity
    {
        if (walls[best_angle].distance > 100)
        {
            Ship[i].thrust_held = true;
            threshold = 30;
        }
        else
         {
            Ship[i].thrust_held = false;
            threshold = 20;
         }
    }

        //Ship[i].thrust_held = (walls[best_angle].distance > 100);                  //thrust if vector is big
    else
        Ship[i].thrust_held = false;                            //off if not.
*/

    if (Ship[i].automode == HUNT && Ship[i].autotargetdistance < 200 && !Ship[i].thrust_held)
        Ship[i].fangle = (180/PI)*atan2(x,-1*y);

    if (Ship[i].fangle < 0) Ship[i].fangle +=360;   //wrap

}

//'vector sum' algorithm
void Pilot0(int i)
{
    int j;//,close_walls = 0;
    float limit = 200,v_squared;
    float x,y/*,target_distance*/,av_wall;
    WallType target_wall;

    find_obstructions(i);                          //find all wall distance/angles

    av_wall = 0;                            //average them, to get a feel for how tight space is.
    for (j=0 ; j<40 ; j++)
    {
        av_wall += walls[j].distance;
        //if (walls[j].distance != limit)
        //    close_walls++;
    }

    av_wall /=40;

    //float limit45 = limit * OOR2;

    avoidx=0;
    avoidy=0;

    for (j=0 ; j<40 ; j++)
    {
        avoidx-=0.2*(limit-walls[j].distance) * sinlut[j];
        avoidy-=0.2*(limit-walls[j].distance) * coslut[j];
    }
    /*
    for (j=0 ; j<num_ships ; j++)
    {
        if (ship_av[j].distance < limit)
        {
            avoidx-=0.6*(limit - (ship_av[j].distance/2) * sinlut[ship_av[j].angle];
            avoidx-=0.6*(limit - (ship_av[j].distance/2)) * sinlut[ship_av[j].angle];
        }
    }
*/

    //if (close_walls == 0)
    //    close_walls = 1;

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

            if (Map.timer >= Ship[i].autostatechangetime)
            {
               Ship[i].autostate = NORMAL;
               Ship[i].autoratio = 0;
            }
        break;
        case ESCAPE_MINUS:
            target_angle = (int)((20/PI)* atan2(x, -1*y))-5; //angle to head for target position
            if (target_angle < 0)  target_angle += 40;
            if (target_angle > 40) target_angle -= 40;

            if (Map.timer >= Ship[i].autostatechangetime)
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

    v_squared = Ship[i].xv*Ship[i].xv + Ship[i].yv*Ship[i].yv;  //current velocity (squared)

    if (v_squared < 25.0f)                                      //if less than max (desired) velocity
        Ship[i].thrust_held = (summag > 2000);                  //thrust if vector is big
    else
        Ship[i].thrust_held = false;                            //off if not.

    if (Ship[i].automode == HUNT && Ship[i].autotargetdistance < 200 && !Ship[i].thrust_held)
        Ship[i].fangle = (180/PI)*atan2(x,-1*y);
    else
        Ship[i].fangle = (180/PI)*atan2(sumx, sumy);                //fangle range is 0-360

    if (Ship[i].fangle < 0) Ship[i].fangle +=360;   //wrap
}

void find_obstructions(int i)
{
    int j;

    find_walls(i);
    find_ships(i);

    for (j=0 ; j<num_ships ; j++)
    {
        if (ship_av[j].distance < walls[ship_av[j].angle].distance)
        {
            walls[(ship_av[j].angle-1)%40].distance = ship_av[j].distance/2;
            walls[ship_av[j].angle].distance = ship_av[j].distance/2;
            walls[(ship_av[j].angle+1)%40].distance = ship_av[j].distance/2;
        }
    }

}

void find_ships(int i)
{
    int j;
    float dx,dy;

	for (j=0 ; j<num_ships ; j++)
	{
		if (i==j)
		{
			ship_av[j].distance = 9999999;	//big, so we shouldn't choose it
			ship_av[j].angle = 0;	//
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
			ship_av[j].distance = sqrt((dx * dx) + (dy * dy));
			ship_av[j].angle = (20/PI) * atan2(dx,dy);	//40 increments
			if (ship_av[j].angle < 0) ship_av[j].angle += 40;
        }
	}
}

void find_walls8(int i)
{
    int j;
    for (j=0 ; j<8 ; j++)
    {
        find_wall(i,j*5,&walls[j]);
    }
}

void find_walls(int i)
{
    int j;
    for (j=0 ; j<40 ; j++)
    {
        find_wall(i,j,&walls[j]);
    }
}





