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
int walls[8];

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

void Takeoff(int i)
{
    Ship[i].autotimer--;
    if (Ship[i].autotimer == 0)
        Ship[i].automode = CRUISE;

    Ship[i].fangle = 0.0f;                //fangle range is 0-360
    Ship[i].saved_angle = 0.0f;
    Ship[i].angle = 0;
    Ship[i].thrust_held = (v_squared < 16.0f); //Map.auto_v
}

void Cruise(int i)
{
	int j,max,closest_wall,wall_threshold=200; //w_t from mapfile
    int deviation,temp_angle;
    int change;

    //Ship[i].autotimer=30;

    //Ship[i].autotimer--;
    //if (Ship[i].autotimer == 0)
    //{
    //    Ship[i].autotimer = 30;
    //    change = (rand()%20) - 10;
    //    Ship[i].angle += change;
    //}

    find_walls(i);

/*
    closest_wall = find_wall(i,Ship[i].angle);

    if (closest_wall < wall_threshold)  //deviate to find closest direction that we can safely fly.
    {
        for (deviation = 1 ; deviation < 40 ; deviation += 1)
        {
            temp_angle = (Ship[i].angle + deviation)%40;

            if (find_wall(i,temp_angle) < wall_threshold)
                break;

            temp_angle = Ship[i].angle - deviation;
            if (temp_angle < 0) temp_angle += 40;

            if (find_wall(i,temp_angle) < wall_threshold)
                break;

        }
    }
*/
    max = 0;
    for (j=0 ; j<8 ; j++)
    {
        if (walls[j] > walls[max])
            max = j;
    }

    Ship[i].fangle = max*5*9;                //fangle range is 0-360
    Ship[i].thrust_held = (v_squared < 16.0f); //Map.auto_v
    //Ship[i].fire1_held = distance[min] < 400;
    //Ship[i].fire2_down = distance[min] < 200;

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
			angle[j] = (20/PI) * atan2(dx,dx);	//40 increments
			if (angle[j] < 0) angle[j] += 40;
        }
	}
    //find closest
    min = i;
    for (k=0 ; k<num_ships ; k++)
    {
        if (distance[k] < distance[min])
        min = k;
    }

    closest_wall = find_wall(i,angle[k]);

    if (closest_wall < wall_threshold)  //deviate to find closest direction that we can safely fly.
    {
        for (deviation = 1 ; deviation < 40 ; deviation += 1)
        {
            temp_angle = ((int)angle[min] + deviation)%40;

            if (find_wall(i,temp_angle) < wall_threshold)
                break;

            temp_angle = (int)angle[min] - deviation;
            if (temp_angle < 0) temp_angle += 40;

            if (find_wall(i,temp_angle) < wall_threshold)
                break;

        }
    }
    angle[min] = temp_angle;

    Ship[i].fangle = angle[min]*9;                //fangle range is 0-360
    Ship[i].thrust_held = (v_squared < 16.0f); //Map.auto_v
    Ship[i].fire1_held = distance[min] < 400;
    Ship[i].fire2_down = distance[min] < 200;
}

void Return(int i)
{

}

void Land(int i)
{

}

void find_walls(int i)
{
    int j;
    for (j=0 ; j<8 ; j++)
    {
        walls[j] = find_wall(i,j*5);
    }
}







