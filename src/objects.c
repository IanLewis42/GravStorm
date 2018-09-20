/*
	GravStorm
    Copyright (C) 2015-2016 Ian Lewis

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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define ALLEGRO_UNSTABLE 1  //needed for haptics.

#include "allegro5/allegro.h"
#include "allegro5/allegro_image.h"
#include "allegro5/allegro_primitives.h"
#include "allegro5/allegro_font.h"
#include "allegro5/allegro_audio.h"
#include "allegro5/allegro_acodec.h"

#include "game.h"
#include "objects.h"
#include "init.h"
#include "gameover.h"
#include "network.h"
#include "inputs.h"

//Trig LUTs
float sinlut[NUM_ANGLES];
float coslut[NUM_ANGLES];

float FrameTime = 0.03333333333333;	//used in race timing

//Structs for objects
ShipType Ship[MAX_SHIPS];
int num_ships = 2;

BulletType Bullet[MAX_BULLETS];

int first_bullet = END_OF_LIST;	//index to first bullet in list END_OF_LIST => 'end of list, => no bullets.
int last_bullet = END_OF_LIST;	//init this when you make a new bullet.

//Parameters for bullets
//                          normal random double triple heavy mine  heat spread lava sentry shrapnel
int TTL[BULLET_TYPES]    = {300,   300,   300,   300,   50,   5000, 150, 300,   256, 300,   300};   //Life (in frames)
int reload[BULLET_TYPES] = {5,     5,     5,     15,    0,    0,    0,   0,     0,   0,     0};	    //frames in between shots (if fire held down) N/A for heavy weapons
float Mass[BULLET_TYPES] = {0.04,  0.04,  0.04,  0.04,  0.1,  0.1,  0.1, 0.04,  0.1, 0.1,   0.04};	//Really bullet mass/ship mass; only used in collisions
int Damage[BULLET_TYPES] = {9,     9,     9,     9,     80,   50,   30,  6,     10,  50,    4};     //points off shield when collision happens

void UpdateLandedShip(int ship_num);	//int
void NewShipBullet (int ship_num, int type, int flags); //int
void NewBullet (int x,int y,int xv,int yv,int angle,float speed, int type,int random,int owner);



/****************************************************
** int UpdateShips(int num_ships)
** Calculate Ship positions based on controls
** Check for ship inside special areas & race start/finish zones.
** Also handle firing, updating fuel, ammo, lives etc.
** if lives go down to 0, return game_over countdown.
****************************************************/
//float x,y,r,r_squared,xg,yg;	//blackhole vars    - global for debug
int UpdateShips(int num_ships)
{
	int i,j;
	float x,y,r,r_squared,xg,yg;	//blackhole vars

	int first_ship = 0;

	if (Net.client || Net.server)
    {
        first_ship = Net.id;
        num_ships = 1;
    }

    Map.timer++;

	for (i=first_ship ; i<first_ship+num_ships ; i++)
	{
		if (Map.mission)
			if(Ship[i].ypos < 64)
				return(GO_TIMER);	//game over

		if (Ship[i].reincarnate_timer)	//we're waiting to watch the explosion
		{
			Ship[i].reincarnate_timer--;	//decrement the timer
			if (Ship[i].reincarnate_timer == 0)	//finished, so re-init
			{
				if (Ship[i].lives == 0)
				{
					Ship[i].reincarnate_timer = 100;	//to stop it reappearing at the end of the game.
					if (Net.client)
                    {
                        NetSendOutOfLives();
                        return 0;
                    }
                    //Ship[i].automode = MANUAL;
                    Ship[i].thrust_held = 0;
					return(GO_TIMER);	//game over
				}
				reinit_ship(i);
				//al_fprintf(logfile,"Ship %d reincarnated\n",i);
			}
		}
		else if (Ship[i].shield <= 0)
		{
			//send to server?? - or put in collision??
			if (Net.client)
            {
                Ship[i].actions |= EXPLODE;
            }
            else
            {
                CreateExplosion(Ship[i].xpos, Ship[i].ypos, 2, 8, Ship[i].xv, Ship[i].yv);//float outward_v);
            }
			//al_play_sample(dead, 1, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
			al_play_sample_instance(dead_inst[i]);
			Net.sounds |= DEAD;
            ScheduleVibrate(70);
			Ship[i].reincarnate_timer = 100;
			Ship[i].lives--;
			Ship[i].thrust = 0; //stop engine noise
			Ship[i].xv = 0;
			Ship[i].yv = 0;
			//al_fprintf(logfile,"Ship %d destroyed\n",i);
		}
		else if(Ship[i].landed)
		{
			UpdateLandedShip(i);		//separate functiom, as it was getting unweildy
		}
		else
		{
			//if (Ship[i].right) //increment angle. wrap if necessary.
#define ASTICK_SPEED 0.5
#ifdef ANDROID
			//Ship[i].fangle += TouchJoystick.spin;
			//if (Ship[i].fangle > NUM_ANGLES) Ship[i].fangle -= NUM_ANGLES;
            //if (Ship[i].fangle < 0) Ship[i].fangle += NUM_ANGLES;
            //Ship[i].angle = (int)Ship[i].fangle;

            //int temp = Ship[i].angle*9;
            //if (temp > 19) temp -= NUM_ANGLES;  //delta >180.......?? 0-360. if (difference < 180) sum -= 180; //??


            float temp = ASTICK_SPEED*(float)Ship[i].angle*9 + (1-ASTICK_SPEED)*(Ship[i].fangle); //convert 'angle' index to degrees, and go part way to control angle

            if (fabsf(((float)Ship[i].angle*9 - Ship[i].fangle)) > 180)
                temp +=180;
            if (temp > 360)
                temp -=360;

            Ship[i].angle = (int)(temp/9);




            //if (Ship[i].angle < 0) Ship[i].angle += NUM_ANGLES;

#else
            if (Ship[i].automode != MANUAL)
            {
                float temp = ASTICK_SPEED*(float)Ship[i].saved_angle + (1-ASTICK_SPEED)*(Ship[i].fangle); //convert 'angle' index to degrees, and go part way to control angle

                if (fabsf(((float)Ship[i].saved_angle - Ship[i].fangle)) > 180)
                    temp +=180;
                if (temp > 360)
                    temp -=360;

                Ship[i].saved_angle = temp;
                Ship[i].angle = (int)((temp/9)+0.5);
                if (Ship[i].angle == 40)
                    Ship[i].angle = 0;
            }
			if (Ship[i].right_held)
			{
				//Ship[i].xpos++;	//DEBUG
				Ship[i].angle++;
				if (Ship[i].angle == NUM_ANGLES)
				  Ship[i].angle = 0;
			}
			//if (Ship[i].left)
			if (Ship[i].left_held)
			{
				//Ship[i].xpos--;	//DEBUG
				Ship[i].angle--;
				if (Ship[i].angle == -1)
				  Ship[i].angle = NUM_ANGLES-1;
			}
#endif
			//if (Ship[i].thrust)
			if (Ship[i].thrust_held && Ship[i].fuel)
            {
                //Ship[i].angle++;	//DEBUG
                //if (Ship[i].angle == NUM_ANGLES)
                //	Ship[i].angle = 0;

                Ship[i].thrust = THRUST;
                Ship[i].fuel--;
            }
			else
				Ship[i].thrust = 0;

			if (Map.radial_gravity)
			{
				xg = 0;
				yg = 0;

				for (j=0 ; j<Map.num_blackholes ; j++)
				{
					x = Ship[i].xpos - Map.blackhole[j].x;
					y = Ship[i].ypos - Map.blackhole[j].y;

					r_squared = x*x + y*y;
					r = sqrt(r_squared);                            //distance between blackhole and ship
					xg += G*Map.blackhole[j].g * x/(r*r_squared);     //acceleration proportional to 1/r^2...
					yg += G*Map.blackhole[j].g * y/(r*r_squared);     //..and scale along normalised (unit) vector
					                                                //These accumulate up for all blackholes to get a final overall value
				}
				Ship[i].xg = xg;
				Ship[i].yg = yg;
				//velocity = velocity + (Thrust - Drag)/Mass
				Ship[i].xv += (((Ship[i].thrust*sinlut[Ship[i].angle]) - Ship[i].xv*Ship[i].drag)/Ship[i].mass) - xg;
				//Position = position + velocity
				Ship[i].xpos += Ship[i].xv;

				//velocity = velocity + ((Thrust - Drag)/Mass) - Gravity
				Ship[i].yv += (((Ship[i].thrust*coslut[Ship[i].angle]) - Ship[i].yv*Ship[i].drag)/Ship[i].mass) + yg  - Ship[i].gravity;
				//Position = position + velocity
				Ship[i].ypos -= Ship[i].yv;
			}
			else //if (Ship[i].gravity)
			{
				//velocity = velocity + (Thrust - Drag)/Mass
				Ship[i].xv += ((Ship[i].thrust*sinlut[Ship[i].angle]) - Ship[i].xv*Ship[i].drag + Ship[i].x_force)/Ship[i].mass;
				//Position = position + velocity
				Ship[i].xpos += Ship[i].xv;

				//velocity = velocity + ((Thrust - Drag)/Mass) - Gravity
				Ship[i].yv += (((Ship[i].thrust*coslut[Ship[i].angle]) - Ship[i].yv*Ship[i].drag + Ship[i].y_force)/Ship[i].mass) - Ship[i].gravity;
				//Position = position + velocity
				Ship[i].ypos -= Ship[i].yv;
			}
			//Special areas
			for (j=0 ; j<Map.num_special_areas ; j++)
			{
				if ( (Ship[i].xpos > Map.area[j].min_x) && (Ship[i].xpos < Map.area[j].max_x)
				  && (Ship[i].ypos > Map.area[j].min_y) && (Ship[i].ypos < Map.area[j].max_y) )
				{
					Ship[i].gravity = Map.area[j].gravity;
					Ship[i].drag = Map.area[j].drag;
					break;
				}
				else
				{
					Ship[i].gravity = Map.gravity;
					Ship[i].drag = Map.drag;
					//break;
				}
			}

			//Forcefields
			for (j=0 ; j<Map.num_forcefields ; j++)
			{
				if (Map.forcefield[j].state == CLOSED)
				{
					//first check if inside entire ff box (both sides of line)
					if ( (Ship[i].xpos > Map.forcefield[j].min_x) && (Ship[i].xpos < Map.forcefield[j].max_x)
					  && (Ship[i].ypos > Map.forcefield[j].min_y) && (Ship[i].ypos < Map.forcefield[j].max_y) )
					{
						if (Map.forcefield[j].half_y == Map.forcefield[j].min_y)	//vertical line - see init code
						{
							if (Ship[i].xpos < Map.forcefield[j].half_x)		//first half
								Ship[i].x_force = -1*Map.forcefield[j].strength;
							else												//second half
								Ship[i].x_force = Map.forcefield[j].strength;
						}
						else													//horizontal line
						{
							if (Ship[i].ypos < Map.forcefield[j].half_y)		//first half
								Ship[i].y_force = Map.forcefield[j].strength;
							else												//second half
								Ship[i].y_force = -1*Map.forcefield[j].strength;
						}
						break;
					}
					else
					{
						Ship[i].x_force = 0;
						Ship[i].y_force = 0;
					}
				}
			}

			if (Map.race)
			{
				int p = Ship[i].current_raceline;   //convenience....

				if (Ship[i].racing)
					Ship[i].current_lap_time += FrameTime;

                //Always do start/finish line
				if (Ship[i].approaching_sf == true)	//if we were approaching start/finish line last frame
                {
					//Check 'after' area
					if ( (Ship[i].xpos > Map.raceline[0].after_minx) && (Ship[i].xpos < Map.raceline[0].after_maxx)
					  && (Ship[i].ypos > Map.raceline[0].after_miny) && (Ship[i].ypos < Map.raceline[0].after_maxy) )
					{
                        //crossed s/f line
                        if (Ship[i].current_raceline == Map.num_racelines)	//if we were about to finish
                        {
                            Ship[i].last_lap_time = Ship[i].current_lap_time; //remember lap time;
                            if (Ship[i].last_lap_time < Ship[i].best_lap_time)
                                Ship[i].best_lap_time = Ship[i].last_lap_time;
                            Ship[i].lap_complete = true;
                        }
                        //always reset lap time and start looking for line 1
                        Ship[i].current_lap_time = 0;
                        Ship[i].current_raceline = 1;
                        Ship[i].racing = true;
					}
                }

				//Check if we're approaching s/f line
				if ( (Ship[i].xpos > Map.raceline[0].before_minx) && (Ship[i].xpos < Map.raceline[0].before_maxx)
				  && (Ship[i].ypos > Map.raceline[0].before_miny) && (Ship[i].ypos < Map.raceline[0].before_maxy) )
					Ship[i].approaching_sf = true;
				else
					Ship[i].approaching_sf = false;


				//Do 'next line'
				if (Ship[i].approaching_next == true)	//if we were approaching last frame
				{
					//Check 'after' area
					if ( (Ship[i].xpos > Map.raceline[p].after_minx) && (Ship[i].xpos < Map.raceline[p].after_maxx)
					  && (Ship[i].ypos > Map.raceline[p].after_miny) && (Ship[i].ypos < Map.raceline[p].after_maxy) )
					{
						//we have just crossed the line, so look for next one. Reset on s/f line.
						Ship[i].current_raceline++;
					}
				}

				//Check 'before' area
				if ( (Ship[i].xpos > Map.raceline[p].before_minx) && (Ship[i].xpos < Map.raceline[p].before_maxx)
				  && (Ship[i].ypos > Map.raceline[p].before_miny) && (Ship[i].ypos < Map.raceline[p].before_maxy) )
					Ship[i].approaching_next = true;
				else
					Ship[i].approaching_next = false;
			}

			//Limit position to map, just while we don't have collision detection!
			//now repurpose as wrapping!
			//Ooh, now make it switchable.
			//want 'sky' on hex map, wrapping on neutron star
			if (Ship[i].xpos < 0)
			{
				if (Map.wrap)
					Ship[i].xpos += mapx;
				else
				{
					Ship[i].xpos = 0;
					Ship[i].xv = 0;
				}
			}
			if (Ship[i].xpos > mapx)
			{
				if (Map.wrap)
					Ship[i].xpos -= mapx;
				else
				{
					Ship[i].xpos = mapx;
					Ship[i].xv = 0;
				}
			}

			if (Ship[i].ypos < 0)
			{
				if (Map.wrap)
					Ship[i].ypos += mapy;
				else
				{
					Ship[i].ypos = 0;
					Ship[i].yv = 0;
				}
			}
			if (Ship[i].ypos > mapy)
			{
				if (Map.wrap)
					Ship[i].ypos -= mapy;
				else
				{
					Ship[i].ypos = mapy;
					Ship[i].yv = 0;
				}
			}


			//'Normal' firing
			//if (Ship[i].fire1)
			if (Ship[i].fire1_held)
			{
				//Ship[i].ypos--;		//DEBUG

				if (Ship[i].ammo1)
				{
					if (Ship[i].fire1_reload == 0)
					{
						//al_play_sample(shoota, 1, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
						//al_play_sample_instance(shoota_inst[i]);
						Ship[i].fire1_reload = reload[Ship[i].ammo1_type];	//frames to reload
						FireNormal(i);
						//Ship[i].ammo1--;
					}
				}
			}

			if (Ship[i].fire1_reload != 0)	//decrement reload counter
				Ship[i].fire1_reload--;


			//'Special' firing
			//if (Ship[i].fire2)
			//if (Ship[i].fire2_held)
			//	Ship[i].ypos++;		//DEBUG

			if (Ship[i].fire2_down)
			{
				Ship[i].fire2_down = false;

				if (Ship[i].ammo2)
				{
					//al_play_sample(shootb, 1, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
					//al_play_sample_instance(shootb_inst[i]);
					FireSpecial(i);
					Ship[i].fire2 = false;		//oneshot
					Ship[i].ammo2--;
				}
			}

			Ship[i].mass = ShipMass(i);

		} //end of if(reincarnate_timer) / shield<=0 / Landed else

	} //end of for(num_ships) loop

	//server posts new positions to network clients

	return 0;
}

//EMPTY_SHIP_MASS = 50, fully loaded ship should be ~200, so ~50 each from fuel, ammo1, ammo2
int  ShipMass(int ship_num)
{
	return EMPTY_SHIP_MASS + (Ship[ship_num].fuel>>5) + (Ship[ship_num].ammo1>>1) + (6 * Ship[ship_num].ammo2) + (MINER_MASS * Ship[ship_num].miners) + (JEWEL_MASS * Ship[ship_num].jewels);
}

//Called by client if no packet rx'd from server
void UpdateRemoteShips(void)
{
    int i,j;
    float x,y,r,r_squared,xg,yg;	//blackhole vars

    for (i=0 ; i<num_ships ; i++)
    {
        if (i != Net.id)
        {
            if (Ship[i].right_held)
            {
                Ship[i].angle++;
                if (Ship[i].angle == NUM_ANGLES)
                  Ship[i].angle = 0;
            }
            if (Ship[i].left_held)
            {
                Ship[i].angle--;
                if (Ship[i].angle == -1)
                  Ship[i].angle = NUM_ANGLES-1;
            }

			if (Map.radial_gravity)
			{
				xg = 0;
				yg = 0;

				for (j=0 ; j<Map.num_blackholes ; j++)
				{
					x = Ship[i].xpos - Map.blackhole[j].x;
					y = Ship[i].ypos - Map.blackhole[j].y;

					r_squared = x*x + y*y;
					r = sqrt(r_squared);                            //distance between blackhole and ship
					xg += G*Map.blackhole[j].g * x/(r*r_squared);     //acceleration proportional to 1/r^2...
					yg += G*Map.blackhole[j].g * y/(r*r_squared);     //..and scale along normalised (unit) vector
					                                                //These accumulate up for all blackholes to get a final overall value
				}
				//velocity = velocity + (Thrust - Drag)/Mass
				Ship[i].xv += (((Ship[i].thrust*sinlut[Ship[i].angle]) - Ship[i].xv*Map.drag)/Ship[i].mass) - xg;
				//Position = position + velocity
				Ship[i].xpos += Ship[i].xv;

				//velocity = velocity + ((Thrust - Drag)/Mass) - Gravity
				Ship[i].yv += (((Ship[i].thrust*coslut[Ship[i].angle]) - Ship[i].yv*Map.drag)/Ship[i].mass) + yg  - Map.gravity;
				//Position = position + velocity
				Ship[i].ypos -= Ship[i].yv;
			}
			else
            //normal gravity
            {
                //velocity = velocity + (Thrust - Drag)/Mass
                Ship[i].xv += ((Ship[i].thrust*sinlut[Ship[i].angle]) - Ship[i].xv*Map.drag)/Ship[i].mass;
                //Position = position + velocity
                Ship[i].xpos += Ship[i].xv;

                //velocity = velocity + ((Thrust - Drag)/Mass) - Gravity
                Ship[i].yv += (((Ship[i].thrust*coslut[Ship[i].angle]) - Ship[i].yv*Map.drag)/Ship[i].mass) - Map.gravity;
                //Position = position + velocity
                Ship[i].ypos -= Ship[i].yv;
            }
        }
    }
    return;
}

/****************************************************
** void UpdateLandedShip(int i)
** Called from UpdateShips()
** Handles ship config menu & recharging fuel, ammo etc.
****************************************************/
void UpdateLandedShip(int i)
{
	if (Ship[i].fire1_reload != 0)	//decrement reload counter
	Ship[i].fire1_reload--;

	if (Map.race)
	{
		if (Ship[i].racing)
			Ship[i].current_lap_time += FrameTime;
	}

	//thrust makes us live again
	if (Ship[i].thrust_held)
	{
		Ship[i].landed = FALSE;
		Ship[i].thrust = THRUST;
        Ship[i].fangle = 0;
        Ship[i].angle = 0;
        Ctrl.ctrl[ASTICK2].x = Ctrl.ctrl[ASTICK].x + Ctrl.ctrl[ASTICK].size/2  - Ctrl.ctrl[ASTICK2].size/2;
        Ctrl.ctrl[ASTICK2].y = Ctrl.ctrl[ASTICK].y + Ctrl.ctrl[ASTICK].size/10 - Ctrl.ctrl[ASTICK2].size/2;
	}

	else if (Ship[i].menu)
	{
#ifdef ANDROID
		if (Select.action == TOUCH || Select.action == MOVE)
        {
            Select.action = NO_ACTION;
            if (Select.x > Scale.xmin && Select.x < Scale.xmax)
            {                                                                //thresholds are top of corresponding area
                if (Select.y > Scale.ammo1level)                             //below top of ammolevel...
                {
                    if (Select.y < Scale.ammo1type) {                       //above ammo1 type, so adjust ammo1 level
                        Ship[i].user_ammo1 = ((Select.x - Scale.xmin) * 100) / Scale.xdiff;
                        ScheduleVibrate(30);
                        if (Ship[i].user_ammo1 < Ship[i].ammo1)
                            Ship[i].ammo1 = Ship[i].user_ammo1;

                    }
                    else if (Select.y < Scale.ammo2level) {                 //above ammo2 level, so adjust ammo1 type
                        if (Ship[i].ammo1_type != (int)(((Select.x - Scale.xmin) * 4) / Scale.xdiff)) {
                            Ship[i].ammo1_type = ((Select.x - Scale.xmin) * 4) / Scale.xdiff;
                            al_play_sample_instance(clunk_inst);
                            ScheduleVibrate(30);
                        }
                    }
                    else if (Select.y < Scale.ammo2type) {
                        Ship[i].user_ammo2 = 0.5 + ((Select.x - Scale.xmin) * 8) / Scale.xdiff;
                        ScheduleVibrate(30);
                        if (Ship[i].user_ammo2 < Ship[i].ammo2)
                            Ship[i].ammo2 = Ship[i].user_ammo2;
                    }
                    else if (Select.y < Scale.fuellevel) {
                        if (Ship[i].ammo2_type != (int)(4 + ((Select.x - Scale.xmin) * 4) / Scale.xdiff)) {
                            Ship[i].ammo2_type = 4 + ((Select.x - Scale.xmin) * 4) / Scale.xdiff;
                            al_play_sample_instance(clunk_inst);
                            ScheduleVibrate(30);
                        }
                    }
                    else if (Select.y < Scale.ymax) {
                        Ship[i].user_fuel = ((Select.x - Scale.xmin) * 100) / Scale.xdiff;
                        ScheduleVibrate(30);
                        if (Ship[i].user_fuel<<4 < Ship[i].fuel)
                            Ship[i].fuel = Ship[i].user_fuel<<4;
                    }
                }
            }
        }
#endif

        if (Ship[i].thrust_held || Command.goforward)
        {
            Command.goforward = FALSE;
            Ship[i].menu = FALSE;
            GameControls();
        }

        if (Ship[i].fire1_down)	//move up
		{
			Ship[i].fire1_down = false;	//one shot

			if (Ship[i].menu_state > 0)
				Ship[i].menu_state--;
		}
		if (Ship[i].fire2_down)  //move down
		{
			Ship[i].fire2_down = false;	//one shot
			if (Ship[i].menu_state < MENU_MAX)
				Ship[i].menu_state++;
		}

		switch(Ship[i].menu_state)
		{
			case MENU_AMMO1_LEVEL:
				if (Ship[i].right_held)
					if (Ship[i].user_ammo1 < 100)
						Ship[i].user_ammo1++;

				if (Ship[i].left_held)
					if (Ship[i].user_ammo1 > 0)
					{
						Ship[i].user_ammo1--;
						Ship[i].ammo1 = Ship[i].user_ammo1;
					}
			break;

			case MENU_AMMO1_TYPE:
				if (Ship[i].right_down)
				{
					Ship[i].right_down = false;	//one shot
					if (Ship[i].ammo1_type < 3)
						Ship[i].ammo1_type++;
				}

				if (Ship[i].left_down)
				{
					Ship[i].left_down = false;	//one shot
					if (Ship[i].ammo1_type > 0)
						Ship[i].ammo1_type--;
				}
			break;

			case MENU_AMMO2_LEVEL:
				if (Ship[i].right_down)
				{
					Ship[i].right_down = false;	//one shot
					if (Ship[i].user_ammo2 < 8)
						Ship[i].user_ammo2++;
				}

				if (Ship[i].left_down)
				{
					Ship[i].left_down = false;	//one shot
					if (Ship[i].user_ammo2 > 0)
					{
						Ship[i].user_ammo2--;
						Ship[i].ammo2 = Ship[i].user_ammo2;
					}
				}
			break;

			case MENU_AMMO2_TYPE:
				if (Ship[i].right_down)
				{
					Ship[i].right_down = false;	//one shot
					if (Ship[i].ammo2_type < 7)
						Ship[i].ammo2_type++;
				}

				if (Ship[i].left_down)
				{
					Ship[i].left_down = false;	//one shot
					if (Ship[i].ammo2_type > 4)
						Ship[i].ammo2_type--;
				}
			break;

			case MENU_FUEL_LEVEL:
				if (Ship[i].right_held)
					if (Ship[i].user_fuel < 100)
						Ship[i].user_fuel++;

				if (Ship[i].left_held)
					if (Ship[i].user_fuel > 0)
					{
						Ship[i].user_fuel--;
						Ship[i].fuel = Ship[i].user_fuel<<4;
					}
			break;
			default:
				al_fprintf(logfile,"Default in menu switch statement (%d)\n",Ship[i].menu_state);
			break;
		}
	}

	else if (Ship[i].fire2_down)	//down takes us into menu
	{
		Ship[i].fire2_down = false;
		if (Ship[i].pad == Ship[i].home_pad)	//but only on home pad.
		{
			Ship[i].menu = TRUE;
            //Ship[i].thrust_held = FALSE;
            Command.goforward = FALSE;
            MenuControls();
		}
	}

	//'Normal' firing is allowed while landed, but not when in menu
	//else if (Ship[i].fire1)
	else if (Ship[i].fire1_held)
	{
		if (Ship[i].ammo1)
		{
			if (Ship[i].fire1_reload == 0)
			{
				//al_play_sample(shoota, 1, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
				Ship[i].fire1_reload = reload[Ship[i].ammo1_type];//RELOAD;	//10 frames to reload
				FireNormal(i);
				//Ship[i].ammo1--;
			}
		}
	}

	//Recharge always when on pad
	Ship[i].recharge_timer++;

	if ((Ship[i].recharge_timer & 0x0007) == 0) //inc every 8 frames ~0.25s
	{
		if(Ship[i].pad == Ship[i].home_pad || (Map.pad[Ship[i].pad].type & PAD_FUEL) )	//if (on home pad OR current pad is a refuelling pad)
			if (Ship[i].fuel < Ship[i].user_fuel<<4)					//  if (below user set limit)
				Ship[i].fuel+=16;										//    increment fule (by 16, 'cos we burn it fast!)

        if(Ship[i].pad == Ship[i].home_pad || (Map.pad[Ship[i].pad].type & PAD_AMMO1) )//same for ammo 1
			if (Ship[i].ammo1 < Ship[i].user_ammo1)
				Ship[i].ammo1++;

        if(Ship[i].pad == Ship[i].home_pad || (Map.pad[Ship[i].pad].type & PAD_SHIELD) )//same for shield
			if (Ship[i].shield < 100)
				Ship[i].shield++;
	}

	if ((Ship[i].recharge_timer & 0x001f) == 0)	//inc every 32 frame ~1s
	{
        if(Ship[i].pad == Ship[i].home_pad || (Map.pad[Ship[i].pad].type & PAD_AMMO2) ) //same for ammo 2 (just slower)
			if (Ship[i].ammo2 < Ship[i].user_ammo2)
				Ship[i].ammo2++;
	}

	return;
}

//make Ship[i] explode
//potential problem ; creating many bullets with same ttl will cause many to expire on same pass.
//do we handle this correctly???? I think so, yes....
//Could make more generic function here, for ship explosion & spreader (& sentry explosion??)
// Something like void CreateExplosion(xpos, ypos, num_rings? (and/or particles_per_ring?), forward_v, outward_v);
//void CreateExplosion(ship_num)
void CreateExplosion(float xpos, float ypos, int num_rings, int num_particles, float xv, float yv)//float outward_v);
{
	int j,k,angle_inc;

	//al_fprintf(logfile,"Explosion.\n");

	angle_inc = 40/num_particles;

	for (k=1 ; k<num_rings+1 ; k++)	//1-2 for 2 rings of particles; fast and slow
	{
		for (j=0 ; j<num_particles ; j++)
		{
			NewBullet(xpos, ypos, xv, yv, j*angle_inc, 0.5 * k * BULLET_SPEED, BLT_SHRAPNEL, 0, NO_OWNER);
		}
	}
}

/****************************************************
** void FireNormal(int i)
** Handles 'normal' firing (i.e. up key, rapid fire weapon)
****************************************************/
void FireNormal(int i)
{
	if (al_get_sample_instance_playing(shoota_inst[i]))
        al_stop_sample_instance(shoota_inst[i]);

    al_play_sample_instance(shoota_inst[i]);
    Net.sounds |= SHOOTA;

	//al_rumble_haptic(hap, 1.0, 1.0, hapID);
    //_jni_callVoidMethodV(_al_android_get_jnienv(), _al_android_activity_object(), "shakeItBaby", "(I)V", 100);

    Ship[i].ammo1--;

	if (Net.client)
    {
        Ship[i].actions |= FIRE_NORMAL;   //sent to server
        return;
    }

	switch (Ship[i].ammo1_type)
	{
		case BLT_NORMAL:	//normal
			NewShipBullet(i,BLT_NORMAL,0);
		break;

		case BLT_RANDOM:	//random
			NewShipBullet(i,BLT_NORMAL,RANDOM);
		break;

		case BLT_DOUBLE:	//left/right
			if (Ship[i].last_position == RIGHT)
			{
				NewShipBullet(i,BLT_NORMAL,LEFT);
				Ship[i].last_position = LEFT;
			}
			else
			{
				NewShipBullet(i,BLT_NORMAL, RIGHT);
				Ship[i].last_position = RIGHT;
			}

		break;

		case BLT_TRIPLE:	//triple
			NewShipBullet(i,BLT_NORMAL, LEFT);
			NewShipBullet(i,BLT_NORMAL, 0);
			NewShipBullet(i,BLT_NORMAL, RIGHT);
			Ship[i].ammo1 -= 2;			//two extra shots
		break;

		default:
		break;
	}

	return;

}

/****************************************************
** void FireSpecial(int ship_num)
** Handles 'special' firing (i.e. down key, slow fire heavy weapon)
****************************************************/
void FireSpecial(int ship_num)
{
	int j,k;

	al_play_sample_instance(shootb_inst[ship_num]);
	Net.sounds |= SHOOTB;
	//al_rumble_haptic(hap, 1.0, 1.0, hapID);
    //_jni_callVoidMethodV(_al_android_get_jnienv(), _al_android_activity_object(), "shakeItBaby", "(I)V", 200);

	if (Net.client)
    {
        Ship[ship_num].actions |= FIRE_SPECIAL;   //sent to server
        return;
    }

	switch (Ship[ship_num].ammo2_type)
	{
		case BLT_HEAVY:	//heavy
			NewShipBullet(ship_num,BLT_HEAVY,0);
		break;

		case BLT_MINE://mine
			NewShipBullet(ship_num,BLT_MINE,0);
		break;

		case BLT_HEATSEEKER: //heatseeker
			NewShipBullet(ship_num,BLT_HEATSEEKER,0);		//seeking handled in UpdateBullets()
		break;

		case BLT_SPREADER: //spreader
			for (k=1 ; k<3 ; k++)	//1-2 for 2 rings of particles; fast and slow
			{
				for (j=0 ; j<8 ; j++)
				{
					NewBullet(Ship[ship_num].xpos + (SHIP_SIZE_X/2 * sinlut[Ship[ship_num].angle]), Ship[ship_num].ypos - (SHIP_SIZE_Y/2 * coslut[Ship[ship_num].angle]),
					          Ship[ship_num].xv + (BULLET_SPEED * sinlut[Ship[ship_num].angle]), Ship[ship_num].yv + (BULLET_SPEED * coslut[Ship[ship_num].angle]),
					          j*5, 0.02 * k * BULLET_SPEED, BLT_SPREADER, 0, ship_num);
				}
			}

		break;
		default:
		break;
	}
}


 void UpdateSwitches(void)
 {
    int i;
	for(i=0 ; i<Map.num_switches ; i++)
	{
        if (Map.switches[i].open)
        {
            Map.switches[i].open_timer--;
            if (Map.switches[i].open_timer == 0)
            {
                Map.switches[i].open = 0;
                Map.switches[i].shield = SENTRY_SHIELD;
            }
        }
        else
        {
            if (Map.switches[i].shield < 0)
            {
                Map.switches[i].open = 1;
                Map.switches[i].open_timer = Map.switches[i].open_time;
            }
        }
	}
 }

void UpdateForcefields(void)
{
    int i;
	for(i=0 ; i<Map.num_forcefields ; i++)
	{
		switch(Map.forcefield[i].state)
		{
        case CLOSED:
                if (Map.switches[Map.forcefield[i].switch1].open || Map.switches[Map.forcefield[i].switch2].open)
                    Map.forcefield[i].state = FADE_OUT;
        break;
        case FADE_OUT:
                Map.forcefield[i].alpha-=10;
                if (Map.forcefield[i].alpha < 0)
                {
                    Map.forcefield[i].alpha = 0;
                    Map.forcefield[i].state = OPEN;
                }
        break;
        case OPEN:
                if (!Map.switches[Map.forcefield[i].switch1].open && !Map.switches[Map.forcefield[i].switch2].open)
                    Map.forcefield[i].state = FADE_IN;
        break;
        case FADE_IN:
                Map.forcefield[i].alpha+=10;
                if (Map.forcefield[i].alpha > 255)
                {
                    Map.forcefield[i].alpha = 255;
                    Map.forcefield[i].state = CLOSED;
                }
        break;
		}
	}
}

//float x_dis, y_dis, distance, direction;  //global for debug
//int random, random100,shots=0,count=0;
void UpdateSentries(void)
{
	int i,j;
	float x_dis, y_dis, distance, direction;

	for (i=0 ; i<Map.num_sentries ; i++)
	{
		if (Map.sentry[i].alive)
		{
			if (Map.sentry[i].shield < 0)
			{
				Map.sentry[i].alive = 0;
				CreateExplosion(Map.sentry[i].x, Map.sentry[i].y, 2, 8, 0, 0);//explode
				Ship[0].sentries++;
			}

			Map.sentry[i].count--;
			if (Map.sentry[i].count == 0)
			{
				//count++;
				//random = rand();
				//random100 = random%100;
				//if (random100 < Map.sentry[i].probability)
				if (rand()%100 < Map.sentry[i].probability)
				{
					//shots++;
					if (Map.sentry[i].type == 1)	//volcano
						NewBullet(Map.sentry[i].x, Map.sentry[i].y, 0, 0, Map.sentry[i].direction, BULLET_SPEED, BLT_LAVA, Map.sentry[i].random, NO_OWNER);

					else if (Map.sentry[i].type == 2)	//targeted gun
					{
						//for each ship
						//work out distance
						//if distance < range
						//work out direction
						// then fire!

	/*					___o___Ship
						|     /
						|    /
					   a|   /
						|  /
						|t/
						|/
						Sentry

						tan(t) = o/a
						t = arctan o/a
	*/
						for (j=0 ; j<num_ships ; j++)
						{
							x_dis = -1*(Map.sentry[i].x - Ship[j].xpos);
							y_dis =    (Map.sentry[i].y - Ship[j].ypos);

							distance = (x_dis * x_dis) + (y_dis * y_dis); //distance squared - don't bother square rooting.

							if (distance < Map.sentry[i].range)	//squared value from file.
							{
								direction = (40/(2*3.14)) * atan2(x_dis,y_dis);	//scaled to 40 = 2*PI
								if (direction < 0) direction += 40;

								NewBullet(Map.sentry[i].x, Map.sentry[i].y, 0, 0, direction, BULLET_SPEED, BLT_SENTRY, Map.sentry[i].random, NO_OWNER);
							}
						}
					}
					else	//normal gun
						NewBullet(Map.sentry[i].x, Map.sentry[i].y, 0, 0, Map.sentry[i].direction, BULLET_SPEED, BLT_SENTRY, Map.sentry[i].random, NO_OWNER);
				}
				Map.sentry[i].count = Map.sentry[i].period;
			}
		}
	}
}

//NewShipBullet()
//work out bullet parameters from ship parameters
void NewShipBullet (int ship_num, int type, int flags)// signed int position, int random)
{
	//int i;
	int position = 0, angle, speed, random = 0;

	//Set start location for bullet
	if (flags & LEFT)  position = -3;	//left/right offset
	if (flags & RIGHT) position = 3;

	angle = Ship[ship_num].angle+position;	//just used to calculate start position

	if (type == BLT_MINE)	//mines go backwards
	{
		angle = (angle + 20);		//angle goes 0 to 40 for full rotation, so +20 is 180 degrees - just used to calculate start position
		speed = BULLET_SPEED * -0.2;//slow and backwards
	}
	else
		speed = BULLET_SPEED;

	if (angle < 0) angle += NUM_ANGLES;	//wrap
	else if (angle > NUM_ANGLES) angle -= NUM_ANGLES;

	if (flags & RANDOM) random = 1;

	NewBullet(Ship[ship_num].xpos + (SHIP_SIZE_X/2 * sinlut[angle]), Ship[ship_num].ypos - (SHIP_SIZE_Y/2 * coslut[angle]), Ship[ship_num].xv, Ship[ship_num].yv, Ship[ship_num].angle, speed, type, random, ship_num);

	return;

}

// NewBullet()
// Find a free entry in the bullets array
// populate it with parameters passed in:
// x,y start position of bullet
// xv, yv 'base' velocity (e.g. speed of ship firing it)
// angle - direction to fire in (0-39)
// speed - speed of bullet in this direction
// type - used to look up mass, damage etc.
// random - how much randomness to apply to velocity
//sort out linked list:
//scan through array until you find one with ttl = 0 => dead, free.
//when you make first bullet, put it's index in first_bullet. and END_OF_LIST in it's next_bullet field.
//when you make next bullet, copy next bullets index into first ones' n_b field, and END_OF_LIST into that one.
//when you destroy (ttl decremented to zero) copy next bullet index up the chain.
//if you destroy first one, copy it's n_b into first_bullet.
void NewBullet (int x,int y,int xv,int yv,int angle,float speed,int type,int random,int owner)//(int ship_num, int type, int flags)// signed int position, int random)
{
	int i;

	for (i=0 ; 	Bullet[i].ttl != 0 ; i++)	//scan till you find a free entry in the array.
	{
		if (i == MAX_BULLETS)
		{
			al_fprintf(logfile,"***Max Bullets exceeded\n");
			return;
		}
	}

	//al_fprintf(logfile,"Made Bullet %d\n",i);

	if (first_bullet == END_OF_LIST)	//if we have no live bullets currently
		first_bullet = i;		//remember that this is the first one
	else
		Bullet[last_bullet].next_bullet = i;	//otherwise, point previous end of list to new end of list

	//init new bullet
	Bullet[i].type = type;
	Bullet[i].ttl = TTL[type];
	Bullet[i].mass = Mass[type];
	Bullet[i].damage = Damage[type];
	Bullet[i].xpos = x;
	Bullet[i].ypos = y;
	Bullet[i].xv = xv + (speed * sinlut[angle]) + (random * 0.1 * ((rand()&0xf) - 7));
	Bullet[i].yv = yv + (speed * coslut[angle]) + (random * 0.1 * ((rand()&0xf) - 7));
    Bullet[i].owner = owner;
	switch(Bullet[i].type)
	{
		case BLT_HEAVY:
			Bullet[i].colour = al_map_rgb(255,0,0);				//Red
		break;
		case BLT_MINE:
			Bullet[i].colour = al_map_rgb(255,255,0);			//start yellow, dims and brightens
		break;
		case BLT_HEATSEEKER:
				Bullet[i].colour = al_map_rgb(0,255,0);			//start green, flashes green / black
		break;
		case BLT_LAVA:
			Bullet[i].colour = al_map_rgb(255,rand()%255,0);	//random red to yellow
		break;
		default:
			Bullet[i].colour = al_map_rgb(255,255,255);			//everything else white
		break;
    }

	Bullet[i].next_bullet = END_OF_LIST;	//show that this is the end of the list

	last_bullet = i;			//remember this is the end of the list
}

//float x,y,r,r_squared,hsax,hsay;	//heatseeker vars
void UpdateBullets(void)
{
	int i,j,current_bullet, previous_bullet;
	float x,y,r,r_squared,hsax,hsay;	//heatseeker vars
	float xg,yg;	//blackhole vars

	current_bullet = first_bullet;
	previous_bullet = END_OF_LIST;

	while(current_bullet != END_OF_LIST)		//handle all 'live' bullets.
	{
		if (current_bullet >= MAX_BULLETS)
        {
            al_fprintf(logfile,"ERROR: Bullet index outside array!!!\n");
            al_fflush(logfile);
            break;   //just in case...
        }

		Bullet[current_bullet].ttl--;			//decrement life

		if (Bullet[current_bullet].ttl <= 0)	//if expired
		{										//pass index up the list
            //al_fprintf(logfile,"Expired Bullet %d\n",current_bullet);

			if (previous_bullet == END_OF_LIST)							//if we're the first bullet
			//if (current_bullet == first_bullet)					//(should be equivalent to above)
				first_bullet = Bullet[current_bullet].next_bullet;	//first_bullet is now the next one.

			else													//if we're not the first, pass our linking pointer one up the list
				Bullet[previous_bullet].next_bullet = Bullet[current_bullet].next_bullet;

			if (Bullet[current_bullet].next_bullet == END_OF_LIST)		//in either case, if we were the last bullet, last bullet is
				last_bullet = previous_bullet;						// now one up the list.

			if (Bullet[current_bullet].type == BLT_HEAVY)
                CreateExplosion(Bullet[current_bullet].xpos, Bullet[current_bullet].ypos, 1, 8, 0, 0);

		}

		else	//update bullet position
		{
			hsax = 0;
			hsay = 0;

			if (Bullet[current_bullet].type == BLT_HEATSEEKER)
			{
				if (Bullet[current_bullet].ttl < 145) //starts at 150 - let it get clear
				{
					for(i=0 ; i<num_ships ; i++)
					{
						x = Ship[i].xpos - Bullet[current_bullet].xpos; //x-y vector from current bullet to Ship[i]
						y = Ship[i].ypos - Bullet[current_bullet].ypos;

						r_squared = x*x + y*y;
                        r = sqrt(r_squared);                            //distance between bullet and ship

						if (r_squared < MIN_R_SQUARED)	//if the bullet is closer than 200, thrust is capped.
							r_squared = MIN_R_SQUARED;

						hsax += HS_ACCN * x/(r*r_squared);              //acceleration proportional to 1/r^2...
                        hsay += HS_ACCN * y/(r*r_squared);              //..and scale along normalised (unit) vector
					                                                    //These accumulate up for all ships to get a final overall value.
                    }

                    hsax -= Bullet[current_bullet].xv*Map.drag;         //apply drag to limit max speed
                    hsay += Bullet[current_bullet].yv*Map.drag;

                    hsax /= 100;    //accn = force/mass
                    hsay /= 100;
				}
			}

			if (Bullet[current_bullet].type == BLT_MINE)	//mine
			{
				if(Bullet[current_bullet].yv == 0)	//don't jump before it's settled
				{
					if((Bullet[current_bullet].ttl & 0x007F) == 0)
					{
						Bullet[current_bullet].ypos -= 20; //lift out of ground!
						Bullet[current_bullet].yv = 3;	   //go up.
					}
				}
			}

			if (Map.radial_gravity)
			{
				xg = 0;
				yg = 0;

				for (j=0 ; j<Map.num_blackholes ; j++)
				{
					x = Bullet[current_bullet].xpos - Map.blackhole[j].x;
					y = Bullet[current_bullet].ypos - Map.blackhole[j].y;

					r_squared = x*x + y*y;
					r = sqrt(r_squared);                            //distance between blackhole and ship
					xg += 5*G*Map.blackhole[j].g * x/(r*r_squared);     //acceleration proportional to 1/r^2...
					yg += 5*G*Map.blackhole[j].g * y/(r*r_squared);     //..and scale along normalised (unit) vector
					                                                //These accumulate up for all blackholes to get a final overall value
				}
				//velocity = velocity + (Thrust - Drag)/Mass
				Bullet[current_bullet].xv += hsax - xg;
				//Position = position + velocity
				Bullet[current_bullet].xpos += Bullet[current_bullet].xv;

				//velocity = velocity + ((Thrust - Drag)/Mass) - Gravity
				Bullet[current_bullet].yv += (-hsay + yg);
				//Position = position + velocity
				Bullet[current_bullet].ypos -= Bullet[current_bullet].yv;
			}
			else
			{
                Bullet[current_bullet].xv += hsax;// - Bullet[current_bullet].xv*Map.drag/300;
                Bullet[current_bullet].xpos += Bullet[current_bullet].xv;

                Bullet[current_bullet].yv -= hsay + Map.gravity/2;// + Bullet[current_bullet].yv*Map.drag/300;
                Bullet[current_bullet].ypos -= Bullet[current_bullet].yv;
			}

			//wrapping / position limit copied from ships.
			if (Bullet[current_bullet].xpos < 0)
			{
				if (Map.wrap)
					Bullet[current_bullet].xpos += mapx;
				else
					Bullet[current_bullet].ttl = 1;
			}
			if (Bullet[current_bullet].xpos > mapx)
			{
				if (Map.wrap)
					Bullet[current_bullet].xpos -= mapx;
				else
					Bullet[current_bullet].ttl = 1;
			}

			if (Bullet[current_bullet].ypos < 0)
			{
				if (Map.wrap)
					Bullet[current_bullet].ypos += mapy;
				else
					Bullet[current_bullet].ttl = 1;
			}
			if (Bullet[current_bullet].ypos > mapy)
			{
				if (Map.wrap)
					Bullet[current_bullet].ypos -= mapy;
				else
					Bullet[current_bullet].ttl = 1;
			}

			previous_bullet = current_bullet;
		}

		//previous_bullet = current_bullet;	//moved up *IPL
		current_bullet = Bullet[current_bullet].next_bullet;
	}

	return;
}


void ScheduleVibrate(int time)
{
    if (time < 30)
        time = 30;
    vibrate_time = time;
    vibrate_timer = 3;

}

void Vibrate(int time)
{
#ifdef ANDROID
	_jni_callVoidMethodV(_al_android_get_jnienv(), _al_android_activity_object(), "shakeItBaby","(I)V", time);
#endif
}
