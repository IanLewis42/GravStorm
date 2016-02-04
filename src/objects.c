/*
	Ian's Thrust Game
    Copyright (C) 2015 Ian Lewis

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

#include "allegro5/allegro.h"
#include "allegro5/allegro_image.h"
#include "allegro5/allegro_primitives.h"
#include "allegro5/allegro_font.h"
#include "allegro5/allegro_audio.h"
#include "allegro5/allegro_acodec.h"

#include "game.h"
#include "objects.h"

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
//                          normal random double triple heavy mine  heat spread lava sentry
int TTL[BULLET_TYPES]    = {300,   300,   300,   300,   60,   5000, 150, 300,   300, 300};	//Life (in frames)
int reload[BULLET_TYPES] = {5,     5,     5,     15,    0,    0,    0,   0,     0,   0};	//frames in between shots (if fire held down) N/A for heavy weapons
float Mass[BULLET_TYPES] = {0.04,  0.04,  0.04,  0.04,  0.1,  0.1,  0.1, 0.04,  0.1, 0.1};	//Really bullet mass/ship mass; only used in collisions
int Damage[BULLET_TYPES] = {3,     3,     3,     3,     80,   50,   50,  3,     10,  50};   //points off shield when collision happens

ALLEGRO_SAMPLE *shoota;
ALLEGRO_SAMPLE *shootb;
ALLEGRO_SAMPLE *particle;
ALLEGRO_SAMPLE *dead;

void UpdateLandedShip(int ship_num);	//int
//void CreateExplosion(int ship_num);		//int
void CreateExplosion(float xpos, float ypos, int num_rings, int num_particles, float xv, float yv);//float outward_v);
void NewShipBullet (int ship_num, int type, int flags); //int
void NewBullet (int x,int y,int xv,int yv,int angle,float speed, int type,int random);
void FireNormal(int i);	//int
void FireSpecial(int i);	//int


/****************************************************
** int UpdateShips(int num_ships)
** Calculate Ship positions based on controls
** Check for ship inside specail areas & race start/finish zones.
** Also handle firing, updating fuel, ammo, lives etc.
** if lives go down to 0, return game_over countdown.
****************************************************/
float x,y,r,r_squared,xg,yg;	//blackhole vars
int UpdateShips(int num_ships)
{
	int i,j;
	//float x,y,r,r_squared,xg,yg;	//blackhole vars

	for (i=0 ; i<num_ships ; i++)
	{
		if (Map.mission)
			if(Ship[i].ypos < 64)
				return(100);	//game over

		if (Ship[i].reincarnate_timer)	//we're waiting to watch the explosion
		{
			Ship[i].reincarnate_timer--;	//decrement the timer
			if (Ship[i].reincarnate_timer == 0)	//finished, so re-init
			{
				if (Ship[i].lives == 0)
				{
					Ship[i].reincarnate_timer = 1;	//to stop it reappearing at the end of the game.
					return(100);	//game over
				}
				reinit_ship(i);
				fprintf(logfile,"Ship %d reincarnated\n",i);
			}
		}
		else if (Ship[i].shield <= 0)
		{
			//CreateExplosion(i);
			CreateExplosion(Ship[i].xpos, Ship[i].ypos, 2, 8, Ship[i].xv, Ship[i].yv);//float outward_v);
			al_play_sample(dead, 1, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
			Ship[i].reincarnate_timer = 100;
			Ship[i].lives--;
			fprintf(logfile,"Ship %d destroyed\n",i);
			//if (Ship[i].lives == 0)
			//	return(100);	//game over
		}
		else if(Ship[i].landed)
		{
			//if (Map.mission)
			//	if(Ship[i].pad == Map.num_pads-1)
			//		return(100);	//game over

			UpdateLandedShip(i);		//separate functiom, as it was getting unweildy
		}
		else
		{
			//if (Ship[i].right) //increment angle. wrap if necessary.
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
				//velocity = velocity + (Thrust - Drag)/Mass
				Ship[i].xv += (((Ship[i].thrust*sinlut[Ship[i].angle]) - Ship[i].xv*Ship[i].drag)/Ship[i].mass) - xg;
				//Position = position + velocity
				Ship[i].xpos += Ship[i].xv;

				//velocity = velocity + ((Thrust - Drag)/Mass) - Gravity
				Ship[i].yv += (((Ship[i].thrust*coslut[Ship[i].angle]) - Ship[i].yv*Ship[i].drag)/Ship[i].mass) + yg;
				//Position = position + velocity
				Ship[i].ypos -= Ship[i].yv;
			}
			else
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
					break;
				}
			}

			//Forcefields
			for (j=0 ; j<Map.num_forcefields ; j++)
			{
				if (Map.sentry[Map.forcefield[j].sentry].alive)
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
				if (Ship[i].racing)
					Ship[i].current_lap_time += FrameTime;

				if (Ship[i].approaching == true)	//if we were approaching last frame
				{
					//Check 'after' area
					if ( (Ship[i].xpos > Map.after_minx) && (Ship[i].xpos < Map.after_maxx)
					  && (Ship[i].ypos > Map.after_miny) && (Ship[i].ypos < Map.after_maxy) )
					{
						if (Ship[i].racing)	//if we were on a lap
						{
							Ship[i].last_lap_time = Ship[i].current_lap_time; //remember it;
							Ship[i].lap_complete = true;
						}
						Ship[i].current_lap_time = 0;
						Ship[i].racing = true;
					}
				}

				//Check 'before' area
				if ( (Ship[i].xpos > Map.before_minx) && (Ship[i].xpos < Map.before_maxx)
				  && (Ship[i].ypos > Map.before_miny) && (Ship[i].ypos < Map.before_maxy) )
					Ship[i].approaching = true;
				else
					Ship[i].approaching = false;
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
						al_play_sample(shoota, 1, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
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
				//Ship[i].ypos++;		//DEBUG

			if (Ship[i].fire2_down)
			{
				Ship[i].fire2_down = false;

				if (Ship[i].ammo2)
				{
					al_play_sample(shootb, 1, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
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

/****************************************************
** void UpdateLandedShip(int i)
** Called from UpdateShips()
** Handles ship config menu & recharging fuel, ammo etc.
****************************************************/
void UpdateLandedShip(int i)
{
	if (Ship[i].fire1_reload != 0)	//decrement reload counter
	Ship[i].fire1_reload--;

	//thrust makes us live again
	if (Ship[i].thrust_held)
	{
		Ship[i].menu = FALSE;
		Ship[i].landed = FALSE;
		Ship[i].thrust = THRUST;
	}

	if (Map.race)
	{
		if (Ship[i].racing)
			Ship[i].current_lap_time += FrameTime;
	}

	else if (Ship[i].menu)
	{
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
				fprintf(logfile,"Default in menu switch statement (%d)\n",Ship[i].menu_state);
			break;
		}
	}

	else if (Ship[i].fire2_down)	//down takes us into menu
	{
		Ship[i].fire2_down = false;
		if (Ship[i].pad == Ship[i].home_pad)	//but only on home pad.
		{
			Ship[i].menu = TRUE;
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
				al_play_sample(shoota, 1, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
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

	if (Map.mission)
	{
		if ((Ship[i].recharge_timer & 0x001f) == 0)	//inc every 32 frame ~1s
		{
			//if (Map.pad[Ship[i].pad].miners)
			//{
			//	Ship[i].miners++;
			//	Map.pad[Ship[i].pad].miners--;
			//}
			//if (Map.pad[Ship[i].pad].jewels)
			//{
			//	Ship[i].jewels++;
			//	Map.pad[Ship[i].pad].jewels--;
			//}
		}
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
	int i,j,k;
	int type = 0;	//different type?

	for (k=1 ; k<num_rings+1 ; k++)	//1-2 for 2 rings of particles; fast and slow
	{
		for (j=0 ; j<num_particles ; j++)
		{

			NewBullet(xpos, ypos, xv, yv, j*5, 0.5 * k * BULLET_SPEED, BLT_NORMAL, 0);
			//NewBullet(Ship[ship_num].xpos, Ship[ship_num].ypos, Ship[ship_num].xv, Ship[ship_num].yv, j*5, 0.5 * k * BULLET_SPEED, BLT_NORMAL, 0);
			/*
			for (i=0 ; 	Bullet[i].ttl != 0 ; i++)	//scan till you find a free one.
			{
				if (i == MAX_BULLETS)
				{
					fprintf(logfile,"Max Bullets exceeded (explosion)\n");
					return;
				}
			}

			if (first_bullet == END_OF_LIST)
				first_bullet = i;
			else
				Bullet[last_bullet].next_bullet = i;	//point previous end of list to new end of list

			//init new bullet
			Bullet[i].type = type;
			Bullet[i].ttl = TTL[type];
			Bullet[i].mass = Mass[type];
			Bullet[i].damage = Damage[type];
			Bullet[i].xpos = Ship[ship_num].xpos;// + (40 * sinlut[j*5]);
			Bullet[i].ypos = Ship[ship_num].ypos;// - (40 * coslut[j*5]);
			Bullet[i].xv = Ship[ship_num].xv + (0.5 * k * BULLET_SPEED * sinlut[j*5]);
			Bullet[i].yv = Ship[ship_num].yv + (0.5 * k * BULLET_SPEED * coslut[j*5]);
			Bullet[i].next_bullet = END_OF_LIST;

			last_bullet = i;			//remember this is the end of the list
			*/
		}
	}
}

/****************************************************
** void FireNormal(int i)
** Handles 'normal' firing (i.e. up key, rapid fire weapon)
****************************************************/
void FireNormal(int i)
{
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
	Ship[i].ammo1--;
	return;

}

/****************************************************
** void FireSpecial(int ship_num)
** Handles 'special' firing (i.e. down key, slow fire heavy weapon)
****************************************************/
void FireSpecial(int ship_num)
{
	int i,j,k;
	int type = 0;	//different type?

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
					          j*5, 0.02 * k * BULLET_SPEED, BLT_NORMAL, 0);
					/*
					for (i=0 ; 	Bullet[i].ttl != 0 ; i++);	//scan till you find a free one.

					if (first_bullet == END_OF_LIST)
						first_bullet = i;
					else
						Bullet[last_bullet].next_bullet = i;	//point previous end of list to new end of list

					//init new bullet
					Bullet[i].type = type;
					Bullet[i].ttl = TTL[type];
					Bullet[i].mass = Mass[type];
					Bullet[i].damage = Damage[type];
					Bullet[i].xpos = Ship[ship_num].xpos + (SHIP_SIZE_X/2 * sinlut[Ship[ship_num].angle]);
					Bullet[i].ypos = Ship[ship_num].ypos - (SHIP_SIZE_Y/2 * coslut[Ship[ship_num].angle]);
					Bullet[i].xv = Ship[ship_num].xv + (BULLET_SPEED * sinlut[Ship[ship_num].angle]) + (0.02 * k * BULLET_SPEED * sinlut[j*5]);
					Bullet[i].yv = Ship[ship_num].yv + (BULLET_SPEED * coslut[Ship[ship_num].angle]) + (0.02 * k * BULLET_SPEED * coslut[j*5]);
					Bullet[i].next_bullet = END_OF_LIST;

					last_bullet = i;			//remember this is the end of the list
				*/
				}
			}

		break;
		default:
		break;
	}
}

//
float x_dis, y_dis, distance, direction;

void UpdateSentries(void)
{
	int i,j;
	//float x_dis, y_dis, distance, direction;

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
				if (rand()%100 < Map.sentry[i].probability)
				{
					if (Map.sentry[i].type == 1)	//volcano
						NewBullet(Map.sentry[i].x, Map.sentry[i].y, 0, 0, Map.sentry[i].direction, BULLET_SPEED, BLT_LAVA, Map.sentry[i].random);

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

								NewBullet(Map.sentry[i].x, Map.sentry[i].y, 0, 0, direction, BULLET_SPEED, BLT_SENTRY, Map.sentry[i].random);
							}
						}
					}
					else	//normal gun
						NewBullet(Map.sentry[i].x, Map.sentry[i].y, 0, 0, Map.sentry[i].direction, BULLET_SPEED, BLT_SENTRY, Map.sentry[i].random);
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
	int i;
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

	NewBullet(Ship[ship_num].xpos + (SHIP_SIZE_X/2 * sinlut[angle]), Ship[ship_num].ypos - (SHIP_SIZE_Y/2 * coslut[angle]), Ship[ship_num].xv, Ship[ship_num].yv, Ship[ship_num].angle, speed, type, random);

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
void NewBullet (int x,int y,int xv,int yv,int angle,float speed,int type,int random)//(int ship_num, int type, int flags)// signed int position, int random)
{
	int i;

	for (i=0 ; 	Bullet[i].ttl != 0 ; i++)	//scan till you find a free entry in the array.
	{
		if (i == MAX_BULLETS)
		{
			fprintf(logfile,"***Max Bullets exceeded\n");
			return;
		}
	}

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
	int i,current_bullet, previous_bullet;
	float x,y,r,r_squared,hsax,hsay;	//heatseeker vars

	current_bullet = first_bullet;
	previous_bullet = END_OF_LIST;

	while(current_bullet != END_OF_LIST)		//handle all 'live' bullets.
	{
		Bullet[current_bullet].ttl--;			//decrement life

		if (Bullet[current_bullet].ttl <= 0)	//if expired
		{										//pass index up the list

			if (previous_bullet == END_OF_LIST)							//if we're the first bullet
			//if (current_bullet == first_bullet)					//(should be equivalent to above)
				first_bullet = Bullet[current_bullet].next_bullet;	//first_bullet is now the next one.

			else													//if we're not the first, pass our linking pointer one up the list
				Bullet[previous_bullet].next_bullet = Bullet[current_bullet].next_bullet;

			if (Bullet[current_bullet].next_bullet == END_OF_LIST)		//in either case, if we were the last bullet, last bullet is
				last_bullet = previous_bullet;						// now one up the list.

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

			Bullet[current_bullet].xv += hsax;// - Bullet[current_bullet].xv*Map.drag/300;
			Bullet[current_bullet].xpos += Bullet[current_bullet].xv;

			Bullet[current_bullet].yv -= hsay + Map.gravity/2;// + Bullet[current_bullet].yv*Map.drag/300;
			Bullet[current_bullet].ypos -= Bullet[current_bullet].yv;

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
