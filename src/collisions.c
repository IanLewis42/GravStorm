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
#include <math.h>

#define ALLEGRO_UNSTABLE 1  //needed for haptics.

#include "allegro5/allegro.h"
#include "allegro5/allegro_image.h"
#include "allegro5/allegro_primitives.h"
#include "allegro5/allegro_font.h"
#include "allegro5/allegro_ttf.h"
#include "allegro5/allegro_audio.h"
#include "allegro5/allegro_acodec.h"

#include "collisions.h"
#include "game.h"
#include "objects.h"
#include "network.h"

ALLEGRO_BITMAP *ship_mask,*map_mask,*sentry_mask;
extern ALLEGRO_SAMPLE *particle;

ALLEGRO_FILE *map_file;

#define MAX_MAP_X	(1440 + 64) //sizes in pixels for tr-style single-picture maps (isemania = 1440 wide)
#define MAX_MAP_Y	(960  + 24) //extra is for padding to prevent collisions on 'wrapping' maps (neutron start = 960 high)

unsigned long int ship_col_mask[1*NUM_ANGLES*24];
unsigned long int sentry_col_mask[1*MAX_SENTRIES*32];
unsigned long int map_col_mask[((MAX_MAP_X/32)+1) * MAX_MAP_Y];	//make bigger for likvidious, but its just flat.....

short int mask_width,mask_height;
char r,g,b;

void CheckForLanding(int i);

void make_ship_col_mask()
{
	//make collision masks
	int i,j,k;

	al_set_new_bitmap_flags(ALLEGRO_MEMORY_BITMAP);
	ship_mask = al_load_bitmap("ship_masks.png");
	mask_width = al_get_bitmap_width(ship_mask);
	mask_height = al_get_bitmap_height(ship_mask);

	al_fprintf(logfile,"ship mask:%d,%d \n",mask_width,mask_height);

	for (i=0 ; i<mask_height ; i++)
	{
		//al_fprintf(logfile,"%02d ",i);

		for(j=0 ; j<NUM_ANGLES ; j++)
		{
			ship_col_mask[i*NUM_ANGLES+j] = 0;	//NUM_ANGLES

			for(k=0 ; k<24 ; k++)
			{
				if(!EquivalentColour(al_get_pixel(ship_mask,(j*24)+k,i), al_map_rgb(255,0,255))) //SHIP_SIZE/2
				{
					ship_col_mask[i*40+j] |= (0x80000000 >> k);
					//al_fprintf(logfile,"1,");
				}
				//else al_fprintf(logfile,"0,");
			}
			//al_fprintf(logfile,"%08X ",ship_col_mask[i*40+j]);

			//if(j==1)	//duplicate middle column
			//{
			//	j++;
			//	ship_col_mask[i*j] = ship_col_mask[i*(j-1)];
			//	al_fprintf(logfile,"%08X ",ship_col_mask[i*(j-1)]);
			//}

		}
		//al_fprintf(logfile,"\n");

	}
	al_destroy_bitmap(ship_mask);
    ship_mask = NULL;
	al_set_new_bitmap_flags(ALLEGRO_VIDEO_BITMAP);
	//return 0;
}

void make_sentry_col_mask()
{
	//make collision masks
	int i,j,k;

	al_set_new_bitmap_flags(ALLEGRO_MEMORY_BITMAP);
	sentry_mask = al_load_bitmap(Map.sentry_collision_file_name);
	if (sentry_mask == NULL)
	{
		al_fprintf(logfile,"No sentry mask\n");

		for (i=0 ; i< 1*MAX_SENTRIES*32 ; i++)
			sentry_col_mask[i] = 0;

		return;
	}

	mask_width = al_get_bitmap_width(sentry_mask);
	mask_height = al_get_bitmap_height(sentry_mask);

	al_fprintf(logfile,"sentry mask:%d,%d \n",mask_width,mask_height);

	Map.num_sentry_sprites = mask_width/32;

	for (i=0 ; i<mask_height ; i++)
	{
		//al_fprintf(logfile,"%02d ",i);

		for(j=0 ; j<(Map.num_sentry_sprites) ; j++)
		{
			sentry_col_mask[i*(Map.num_sentry_sprites)+j] = 0;

			for(k=0 ; k<32 ; k++)
			{
				if(!EquivalentColour(al_get_pixel(sentry_mask,(j*32)+k,i), al_map_rgb(255,0,255))) //SHIP_SIZE/2
				{
					sentry_col_mask[i*(Map.num_sentry_sprites)+j] |= (0x80000000 >> k);
					//al_fprintf(logfile,"1,");
				}
				//else al_fprintf(logfile,"0");
			}
			//al_fprintf(logfile,"%08X ",sentry_col_mask[i*(mask_width/32)+j]);
		}
		//al_fprintf(logfile,"\n");

	}
	al_destroy_bitmap(sentry_mask);
	sentry_mask = NULL;
	al_set_new_bitmap_flags(ALLEGRO_VIDEO_BITMAP);
	//return 0;
}

int words_per_row;

void make_map_col_mask(void)
{
	int i,j=0,k;
	int row_start, row_end, col_start, col_end;
	//int words_per_row;
	//int radar[200*200];     //array to be turned into bitmap

	//for (i=0 ; i<200*200 ; i++) radar[i] = 0;   //zero it

    for (i=0 ; i< ( ( (MAX_MAP_X/32)+1) * MAX_MAP_Y) ; i++ )
        map_col_mask[i] = 0;

	al_set_new_bitmap_flags(ALLEGRO_MEMORY_BITMAP);
	map_mask = al_load_bitmap(Map.collision_file_name);

	mask_width = al_get_bitmap_width(map_mask);
	mask_height = al_get_bitmap_height(map_mask);

	map_file = al_fopen("mapfile.txt","w");

	words_per_row = (mask_width/32);//+2;// +2 for padding.

	if (Map.type == 0 || Map.type == 2)
		words_per_row +=2;

	al_fprintf(logfile,"map:%d,%d wpr:%d (including padding)\n",mask_width,mask_height,words_per_row);
	if ((mask_width & 0x001f) != 0)
		al_fprintf(logfile,"ERROR:map width must be a multiple of 32!\n");

	if (Map.type == 0 || Map.type == 2)
	{
		for (i=0 ; i<12 ; i++)
		{
			for(j=0 ; j<words_per_row ; j++)
			{
				map_col_mask[i*words_per_row+j] = 0;	//top padding
				if (map_file) al_fprintf(map_file,"%08lX ",map_col_mask[i*words_per_row+j]);
			}
            if (map_file) al_fprintf(map_file,"\n");
		}
		row_start = 12;
		row_end = mask_height+12;
		col_start = 1;
		col_end = words_per_row-1;
	}
	else
	{
		row_start = 0;
		row_end = mask_height;
		col_start = 0;
		col_end = words_per_row;
	}

	for (i=row_start ; i<row_end ; i++)
	{
		if (Map.type == 0 || Map.type == 2)
		{
			map_col_mask[i*words_per_row+0] = 0;	//left hand padding column
            if (map_file) al_fprintf(map_file,"%08lX ",map_col_mask[i*words_per_row+j]);
		}

		for(j=col_start ; j<col_end ; j++)
		{
			map_col_mask[i*words_per_row+j] = 0;

			for(k=0 ; k<32 ; k++)
			{
				if(!EquivalentColour(al_get_pixel(map_mask,((j-col_start)*32)+k,i-row_start), al_map_rgb(255,0,255)))
				{
					map_col_mask[i*words_per_row+j] |= (0x80000000 >> k);	//20
					//radartemp = 1;

				}
			}
			//if (i<320)
            if (map_file) al_fprintf(map_file,"%08lX ",map_col_mask[i*words_per_row+j]);
		}

		if (Map.type == 0 || Map.type == 2)
		{
			map_col_mask[i*words_per_row+j] = 0;	//right hand padding column
            if (map_file) al_fprintf(map_file,"%08lX ",map_col_mask[i*words_per_row+j]);
		}

		//if (i<320)
        if (map_file) al_fprintf(map_file,"\n");
		//for (k=0 ; k<65535 ; k++)
		//{
		//	asm("nop");
		//}
		//al_fprintf(logfile,"%d\n",i);

	}

	if (Map.type == 0 || Map.type == 2)
	{
		for (i=mask_height+12 ; i<mask_height+24 ; i++)
		{
			for(j=0 ; j<words_per_row ; j++)
			{
				map_col_mask[i*words_per_row+j] = 0;	//top padding
                if (map_file) al_fprintf(map_file,"%08lX ",map_col_mask[i*words_per_row+j]);
			}
            if (map_file) al_fprintf(map_file,"\n");
		}
	}


	al_destroy_bitmap(map_mask);
	map_mask = NULL;
    if (map_file)

    al_fclose(map_file);



	//al_fprintf(logfile,"File done\n");

	//return 0;
	//
	//for (i=0 ; i<mask_height ; i++)
	//{
	//	for (j=0 ; j<(mask_width>>5) ; j++)
	//	{
	//		al_fprintf(map_file,"%08X ",map_col_mask[i*20+j]);
	//	}
	//	al_fprintf(map_file,"\n");
	//	al_fprintf(logfile,"%d\n",i);
	//}
	//fclose(map_file);
	//al_fprintf(logfile,"File done\n");

	//return 0;

	al_set_new_bitmap_flags(ALLEGRO_VIDEO_BITMAP);
}

void make_radar_bitmap(void)
{
    int i,j,k;
    int x, startx=0, y=0;

    if (Map.type == 0 || Map.type == 2)
    {
        startx=1;
        y=12;
    }

    Radar.height = (mask_height)/8;
    Radar.width  = (mask_width)/8;
    al_fprintf(logfile,"Radar Mask Height = %d, Width = %d\n",Radar.height, Radar.width);

    //al_set_new_bitmap_flags(ALLEGRO_MEMORY_BITMAP);
    Radar.mask = al_create_bitmap(Radar.width, Radar.height);	//create a bitmap - 400
    al_set_target_bitmap(Radar.mask);				//set it as the default target for all al_draw_ operations

    al_clear_to_color(al_map_rgba(0, 0, 0, 128));

    for (k=0 ; k<Radar.height ; k++)
    //for (k=320 ; k<400 ; k++)
    //for (k=(MAX_MAP_HEIGHT * RADAR_PPT)-Radar.height ; k<(MAX_MAP_HEIGHT * RADAR_PPT) ; k++)
    {
        x=startx;

        for (j=0 ; j<Radar.width/4 ; j++)
        {
            //one square - 8x8 in col map = 1 pixel in radar
            for (i=0 ; i<8 ; i++)
            {
                if ((map_col_mask[(y+i)*words_per_row + x] >> 24 & 0xf) != 0)
                {
                    al_put_pixel(4*j,k,al_map_rgba(128,128,128,128));//(255,255,255));    ////make pixel opaque , break
                    break;
                }
            }
            //next square / pixel
            for (i=0 ; i<8 ; i++)
            {
                if ((map_col_mask[(y+i)*words_per_row + x] >> 16 & 0xf) != 0)
                {
                    al_put_pixel(4*j+1,k,al_map_rgba(128,128,128,128));//255,255,255));  //make pixel opaque
                    break;
                }
            }
            //next...
            for (i=0 ; i<8 ; i++)
            {
                if ((map_col_mask[(y+i)*words_per_row + x] >> 8 & 0xf) != 0)
                {
                    al_put_pixel(4*j+2,k,al_map_rgba(128,128,128,128));//(255,255,255));  //make pixel opaque , break
                    break;
                }
            }
            //next square / pixel
            for (i=0 ; i<8 ; i++)
            {
                if ((map_col_mask[(y+i)*words_per_row + x] >> 0 & 0xf) != 0)
                {
                    al_put_pixel(4*j+3,k,al_map_rgba(128,128,128,128));//(255,255,255));  //make pixel opaque
                    break;
                }
            }
            x++;
        }
        y+=8;
    }

    if (Map.type == 0 || Map.type == 2)
        Radar.display = Radar.mask;
    else    //tiled
    {
        Radar.height = map_height*4;
        Radar.width  = map_width*4;
        Radar.display = al_create_bitmap(Radar.width, Radar.height);	//create a bitmap -
        al_fprintf(logfile,"Radar Display Height = %d, Width = %d\n",Radar.height, Radar.width);

        al_set_target_bitmap(Radar.display);				//set it as the default target for all al_draw_ operations
        al_clear_to_color(al_map_rgba(0, 0, 0, 0));

        //use al_draw_bitmap_region to copy 4x4 chunks from .mask to .display
        for (y=0 ; y<map_height ; y++)
        {
            for (x=0 ; x<map_width ; x++)
            {
                i = tile_map[x + y * MAX_MAP_WIDTH];    //pull tile index From array
                int u = (i & 0x0007)<<2;    //bottom 3 bits * 4
				int v = (i & 0xfff8)>>1;    //upper bits RS by 3 then * 4

                                                //sx sy sw sh dx dy
                al_draw_bitmap_region(Radar.mask, u,v,4,4,x*4,y*4,0);
            }
        }
    }

    al_set_target_backbuffer(display);			//Put default target back
    al_set_new_bitmap_flags(ALLEGRO_VIDEO_BITMAP);
}

//int y_overlap,y_offset,i_row,j_row,shift,shipi_word,shipj_word; //global for debug

void CheckSSCollisions(int num_ships)	//Ship-to-ship collisions
{
	int i,j,k;
	int y_overlap,y_offset,i_row,j_row,shift,shipi_word,shipj_word;

	for (i=0 ; i<num_ships ; i++)
	{
		if (Ship[i].reincarnate_timer == 0)
		{
			for (j=i+1 ; j<num_ships ; j++)
			{
				if (Ship[j].reincarnate_timer == 0)
				{
					//check collision between Ship[i] and Ship[j];
					if (fabsf(Ship[i].xpos-Ship[j].xpos) < SHIP_SIZE_X)
					{
						if (fabsf(Ship[i].ypos-Ship[j].ypos) < SHIP_SIZE_Y)
						{
							//now do pixel checking

							//Say ship[i] centre is 100,100 ; Ship[j] is 90, 90

							y_offset = (int)(Ship[i].ypos - Ship[j].ypos)>>1;
							//y_offset is 10, meaning j centre is 10 above i centre
							//so we need i mask row 0, and j mask row 10, except that ship_col_mask
							//is half resolution, so it's 0 and 5.

							y_overlap = 24-abs(y_offset);

							//if it's the other way around, we need j mask 0, and i mask 5
							if (y_offset > 0)
							{
								i_row = 0;
								j_row = y_offset;
							}
							else
							{
								i_row = -1*y_offset;
								j_row = 0;
							}

							shift = (int)((Ship[i].xpos)-Ship[j].xpos)>>1;	//this can go -ve
							//shift is 10, meaning that ship[i] centre is 10 left of ship[j] centre.
							//so left shift j by 10.
							//except it's all half reolution, so its 5.
							{
								for (k=0 ; k<y_overlap ; k++)	//not 24 - just the overlap.
								{
									shipi_word = (ship_col_mask[(i_row+k)*40 + Ship[i].angle]);
									if (shift > 0)
										shipj_word = (int)(ship_col_mask[(j_row+k)*40 + Ship[j].angle])<<shift;
									else
										shipj_word = (int)(ship_col_mask[(j_row+k)*40 + Ship[j].angle])>>(-1*shift);

									if (shipi_word & shipj_word)
									{
										Ship[i].shield = 0;
										Ship[j].shield = 0;
										Ship[i].crashed++;
										Ship[j].crashed++;
										Ship[i].bullet.damage+=100;    //for network play
										Ship[j].bullet.damage+=100;
									}
								}
							}
						}
					}
				}
			}
		}
	}
}


//int ship_word, bullet_word, y_offset, shift;

void CheckBSCollisions(int num_ships)	//Bullet-to-ship collisions
{
	int i,j,k;
	int ship_word, bullet_word, y_offset, shift;

	if (first_bullet == END_OF_LIST) return;

	for (i=0 ; i<num_ships ; i++)
	{
		if (Ship[i].reincarnate_timer == 0)
		{
			for (j=first_bullet ; j != END_OF_LIST ; j = Bullet[j].next_bullet)
			{
				if (j > MAX_BULLETS) al_fprintf(logfile,"bullet index = %d\n",j);

				//check collision between Ship[i] and bullet[j];
				if (fabsf(Ship[i].xpos-Bullet[j].xpos) < SHIP_SIZE_X/2)	//does bounding box save you much here??
				{
					if (fabsf(Ship[i].ypos-Bullet[j].ypos) < SHIP_SIZE_Y/2)
					{
						//now do pixel checking
						//mostly good, occasionally fast bullets miss the tip (presumably 'cos they go from one side to the other in a frame....)
						//solution: check half/quarter frame back/forward
                        //Hmmmm.... checking back sometimes makes you hit yourself with just fired bullet. I think...
                        //so just check forwards.
						//for (k=-5 ; k<3 ; k++)
						for (k=0 ; k<4 ; k++)
						{
							//Say ship centre is 100,100 ; bullet is 90, 90

							//y_offset = (Ship[i].ypos - (Bullet[j].ypos + 0.1*k*Bullet[j].yv));
							y_offset = (Ship[i].ypos - (Bullet[j].ypos + 0.25*k*Bullet[j].yv));

							//y_offset is 10, meaning bullet centre is 10 above ship centre
							//so we need ship mask row 10 above centre, except that ship_col_mask
							//is half resolution, so it's 5 rows above centre. Centre is 12, so we need 7 down

							y_offset = 12 - (y_offset>>1)-1; //what if y_offset is -ve??? - think it works OK.

							ship_word = (ship_col_mask[y_offset*NUM_ANGLES + Ship[i].angle]);

							shift = ((int)((Ship[i].xpos)-(Bullet[j].xpos + 0.25*k*Bullet[j].xv)))>>1;
							//shift is 10, meaning that bullet centre is 10 left of ship centre.
							//so left shift bullet_word by 10.
							//except it's all half reolution, so its 5.
							//apparently shift by negative number doesn't work, so....

							if (shift >=0)
								bullet_word = 0x00180000 << shift;
							else
								bullet_word = 0x00180000 >> -1*shift;

							if (ship_word & bullet_word)
							{
								//al_play_sample(particle, 1, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
								al_play_sample_instance(particle_inst[i]);
								Net.sounds |= PARTICLE;
								Bullet[j].ttl=1;	//decrement, picked up next time.

                                if (Net.server && i!=0)				//for other ships in network play.
                                {
                                    Ship[i].bullet.damage += Bullet[j].damage;
                                    Ship[i].bullet.owner = Bullet[j].owner;
                                    Ship[i].bullet.type = Bullet[j].type;
                                    Ship[i].bullet.xv = Bullet[j].xv;
                                    Ship[i].bullet.yv = Bullet[j].yv;
                                }
                                else //local, or network, and it's the server that's been hit
                                {
                                    if (Ship[i].shield > 0) {                               //prevent multiple kills from 1 'spreader'
                                        Ship[i].shield -= Bullet[j].damage;                //decrement shield
                                        if (Ship[i].shield <= 0) {
                                            Ship[i].killed++;
                                            if (Bullet[j].owner != NO_OWNER)
                                                Ship[Bullet[j].owner].kills++;
                                        }
                                        Ship[i].xv += Bullet[j].mass *
                                                      Bullet[j].xv;    //momentum from bullet to ship
                                        Ship[i].yv += Bullet[j].mass * Bullet[j].yv;
                                        ScheduleVibrate(Bullet[j].damage);
                                    }
                                }

                                if (Bullet[j].type == BLT_HEAVY)    //stop explosion on hitting ship
                                    Bullet[j].type = BLT_NORMAL;


								break;
							}
						}
					}
				}
			}
		}
	}
}

//#define NUM_SENTRY_SPRITES 5

void CheckBSentryCollisions(void)	//Bullet-to-sentry collisions
{
	int i,j,k;
	int sentry_word, bullet_word, y_offset, shift;

	if (first_bullet == END_OF_LIST) return;

	for (i=0 ; i<Map.num_sentries ; i++)
	{
		if (Map.sentry[i].alive)
		{
			for (j=first_bullet ; j != END_OF_LIST ; j = Bullet[j].next_bullet)
			{
				if (j > MAX_BULLETS) al_fprintf(logfile,"ERROR:bullet index = %d\n",j);
				if (Bullet[j].type != BLT_SENTRY && Bullet[j].type != BLT_LAVA)
				{
					//check collision between sentry[i] and bullet[j];
					//bounding box, arbitrary 100 pixels from sentry centre (to allow for forward checking, see later)
					if (fabsf(Map.sentry[i].x-Bullet[j].xpos) < 100)
					{
						if (fabsf(Map.sentry[i].y-Bullet[j].ypos) < 100)
						{
							//now do pixel checking
							//Occasionally fast bullets missed the tip (presumably 'cos they go from one side to the other in a frame....)
							//solution: check forwards in quarter frames.
							for (k=0 ; k<3 ; k++)
							{
								//Say sentry centre is 100,100 ; bullet is 90, 90
                                //find diffenece in position, divide by 2 'cos collision masks are half size.
								y_offset = (int)(Map.sentry[i].y - (Bullet[j].ypos + 0.25*k*Bullet[j].yv))>>1;
								shift    = (int)(Map.sentry[i].x - (Bullet[j].xpos + 0.25*k*Bullet[j].xv))>>1;

                                //check if the bullet is inside collision mask
								if ( (abs(shift) <= 16) && (abs(y_offset) <= 16) ) //collision map is 32x32, sentry x,y co-ords are centre.
								{
                                    //example continued. y_offset is 10, meaning bullet centre is 10 above sentry centre
                                    //so we need sentry mask row 10 above centre, except that sentry_col_mask
                                    //is half resolution, so it's 5 rows above centre. Centre is 16, so we need 11 down

                                    y_offset = 16 - (y_offset);//-1; //what if y_offset is -ve??? - think it works OK.

                                    sentry_word = (sentry_col_mask[y_offset*Map.num_sentry_sprites + Map.sentry[i].alive_sprite]);

                                    //shift is 10, meaning that bullet centre is 10 left of sentry centre.
                                    //so left shift bullet_word by 10.
                                    //except it's all half reolution, so its 5.
                                    //apparently shift by negative number doesn't work, so....

                                    if (shift >=0)
                                        bullet_word = 0x00180000 << shift;
                                    else
                                        bullet_word = 0x00180000 >> -1*shift;

                                    if (sentry_word & bullet_word)
                                    {
                                        //al_play_sample(particle, 1, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
                                        al_play_sample_instance(sentry_particle_inst);
                                        Bullet[j].ttl=1;	//decrement, picked up next time.
                                        Map.sentry[i].shield -= Bullet[j].damage;				//decrement shield
                                        if (Bullet[j].type == BLT_HEAVY)    //stop explosion on hitting sentry
                                            Bullet[j].type = BLT_NORMAL;

                                        break;
                                    }
								}
							}
						}
					}
				}
			}
		}
	}
}

void CheckBSwitchCollisions(void)	//Bullet-to-switch collisions, copied from ..sentry
{
	int i,j,k;
	int sentry_word, bullet_word, y_offset, shift;

	if (first_bullet == END_OF_LIST) return;

	for (i=0 ; i<Map.num_switches ; i++)
	{
		//if (Map.sentry[i].alive)
		{
			for (j=first_bullet ; j != END_OF_LIST ; j = Bullet[j].next_bullet)
			{
				if (j > MAX_BULLETS) al_fprintf(logfile,"ERROR:bullet index = %d\n",j);
				if (Bullet[j].type != BLT_SENTRY && Bullet[j].type != BLT_LAVA)
				{
					//check collision between sentry[i] and bullet[j];
					//bounding box, arbitrary 100 pixels from sentry centre (to allow for forward checking, see later)
					if (fabsf(Map.switches[i].x-Bullet[j].xpos) < 100)
					{
						if (fabsf(Map.switches[i].y-Bullet[j].ypos) < 100)
						{
							//now do pixel checking
							//Occasionally fast bullets missed the tip (presumably 'cos they go from one side to the other in a frame....)
							//solution: check forwards in quarter frames.
							for (k=0 ; k<3 ; k++)
							{
								//Say sentry centre is 100,100 ; bullet is 90, 90
                                //find diffenece in position, divide by 2 'cos collision masks are half size.
								y_offset = (int)(Map.switches[i].y - (Bullet[j].ypos + 0.25*k*Bullet[j].yv))>>1;
								shift    = (int)(Map.switches[i].x - (Bullet[j].xpos + 0.25*k*Bullet[j].xv))>>1;

                                //check if the bullet is inside collision mask
								if ( (abs(shift) <= 16) && (abs(y_offset) <= 16) ) //collision map is 32x32, sentry x,y co-ords are centre.
								{
                                    //example continued. y_offset is 10, meaning bullet centre is 10 above sentry centre
                                    //so we need sentry mask row 10 above centre, except that sentry_col_mask
                                    //is half resolution, so it's 5 rows above centre. Centre is 16, so we need 11 down

                                    y_offset = 16 - (y_offset);//-1; //what if y_offset is -ve??? - think it works OK.

                                    sentry_word = (sentry_col_mask[y_offset*Map.num_sentry_sprites + Map.sentry[i].alive_sprite]);

                                    //shift is 10, meaning that bullet centre is 10 left of sentry centre.
                                    //so left shift bullet_word by 10.
                                    //except it's all half reolution, so its 5.
                                    //apparently shift by negative number doesn't work, so....

                                    if (shift >=0)
                                        bullet_word = 0x00180000 << shift;
                                    else
                                        bullet_word = 0x00180000 >> -1*shift;

                                    if (sentry_word & bullet_word)
                                    {
                                        //al_play_sample(particle, 1, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
                                        al_play_sample_instance(sentry_particle_inst);
                                        Bullet[j].ttl=1;	//decrement, picked up next time.
                                        Map.switches[i].shield -= Bullet[j].damage;				//decrement shield
                                        break;
                                    }
								}
							}
						}
					}
				}
			}
		}
	}
}

//global vars for debug
int map_idx_x, map_idx_y;
int map_idx, map_word1, map_word2;
int shift1, shift2;
int ship_word1, ship_word1_shift;
int ship_word2, ship_word2_shift;
int collision;

int tile_x, tile_y, tile_idx, start_tile_idx, tile, tl_tile;
int start_row,current_row,rows;

void CheckSWCollisions(int num_ships)
{
	int i,j;//,k;
	//int shift;
	//int map_word;
	//int tile_x, tile_y, tile_idx, tile;
	//int start_row,current_row,rows;

	int first_ship = 0;

	if (Net.client)
    {
        first_ship = Net.id;
        num_ships = first_ship+1;
    }
    if (Net.server)
    {
        first_ship = 0;
        num_ships = first_ship+1;
    }

	if (Map.type == 0 || Map.type == 2)
	{
		for (i=first_ship ; i<num_ships ; i++)
		//need to start top left
		//need to increment map word in y direction in j loop
		{
			if (Ship[i].reincarnate_timer == 0)
			{
				map_idx_x = (((int)Ship[i].xpos)-24 + 64) >> 6;	//-24 to get left hand side, +64 because of 2 columns padding.																//>>5 to divide by 32, >>1 more because display map x2 collision map
				map_idx_y = (((int)Ship[i].ypos)-24 + 24) >> 1;	// as above, +24 rows padding
				map_idx =  map_idx_x + map_idx_y * words_per_row;	//This assumes that display map and collision map are the same size

				shift1 =  ((((int)Ship[i].xpos)-24 ) >> 1) & 0x001F;
                shift2 = 31-shift1;

				//DEBUG
				j=12;
				ship_word1 = (ship_col_mask[j*NUM_ANGLES + Ship[i].angle]);
				ship_word1_shift = ship_word1 >> shift1;
				//shift2 = 30-shift1;	//seems better. don't know why!
				ship_word2 = (ship_col_mask[j*NUM_ANGLES + Ship[i].angle]);
				ship_word2_shift = ship_word2 << shift2;
                collision=0;
                //END DEBUG

				for (j=0 ; j<24 ; j++)
				{
					map_word1 = map_col_mask[map_idx];

					//ship_word1 = (ship_col_mask[j*NUM_ANGLES + Ship[i].angle]);
					//ship_word1_shift = ship_word1 >> shift1;

					if((ship_col_mask[j*NUM_ANGLES + Ship[i].angle] >> shift1) & map_word1)
					{
						CheckForLanding(i);			//check for landing
						if (!Ship[i].landed)
						{
							Ship[i].shield = 0;
							Ship[i].crashed++;
							collision = 1;
							break;
						}
					}
					//else collision = 0;

					//shift2 = 32-shift1;
					//shift2 = 30-shift1;	//seems better. don't know why!

					//ship_word2 = (ship_col_mask[j*NUM_ANGLES + Ship[i].angle]);

					//ship_word2_shift = ship_word2 << shift2;

					if (shift2 < 32)
					{
						map_word2 = map_col_mask[map_idx+1];

						if((ship_col_mask[j*NUM_ANGLES + Ship[i].angle] << shift2) & map_word2)
						{
							CheckForLanding(i);			//check for landing
							if (!Ship[i].landed)
							{
								Ship[i].shield = 0;
								Ship[i].crashed++;
								collision = 2;
								break;
							}
						}
						//else collision = 0;
					}
					map_idx += words_per_row;//mapx>>6;//This assumes that display map and collision map are the same size

				}
			}		//end of (if reincarnate timer)
		}			//end of for (ships)
	}				//end of if(Map.type == 0)

	else	//Map.type == 1, i.e. tiled
	{
		for (i=first_ship ; i<num_ships ; i++)
		{
			collision = 0;

			if (Ship[i].reincarnate_timer == 0)
			{
				//find the tile which the top-left corner of the ship is in
				tile_x = (((int)Ship[i].xpos)-24) >> 6;	//dividing by 64 here, as display tiles are 64
				tile_y = (((int)Ship[i].ypos)-24) >> 6;

				if (tile_x < 0) tile_x = 0;
				if (tile_y < 0) tile_y = 0;

				start_tile_idx =  tile_x + tile_y * MAX_MAP_WIDTH;	//index into tile array for the tile top-left corner is in
				tile = tile_map[start_tile_idx];	//value in tile array, gives index into tile atlas
				tl_tile = tile;

				//
				shift1 = ((((int)Ship[i].xpos)-24) & 0x003f)>>1;	//bottom 6 bits, rs by 1 because col masks are half size
				start_row = ((((int)Ship[i].ypos)-24) & 0x003f)>>1;	//
				current_row = start_row;
				rows = 32-start_row;		//rows until we get to the bottom of the tile

				if (tile != 0)	//if it's not an empty tile
				{
					for (j=0 ; j<rows ; j++)
					{
						if (j==24) break;
						//map_word1 = map_col_mask[tile + current_row*words_per_row]; //here
						map_word1 = map_col_mask[(tile & 0x07) + (((tile&0xfff8)<<2)+current_row)*words_per_row]; //bottom 3 bits of 'tile' index across (8 tiles per row in tilemap)
						ship_word1 = (ship_col_mask[j*NUM_ANGLES + Ship[i].angle]);                             //upper bits of 'tile' index down - R shift by 3, but then *32 => L shift by 2
						ship_word1_shift = ship_word1 >> shift1;

						if((ship_col_mask[j*NUM_ANGLES + Ship[i].angle] >> shift1) & map_word1)
						{
							CheckForLanding(i);			//check for landing
							if (!Ship[i].landed)		//if not landed, we're dead
							{
								if (!collision)
                                {
                                    Ship[i].shield = 0;
                                    Ship[i].crashed++;
                                    collision = 1;
                                }
								break;
							}
						}
						current_row++;
					}
				}

				if (rows < 24)//we're running into the next tile down.
				{
					tile_idx = start_tile_idx + MAX_MAP_WIDTH;
					tile = tile_map[tile_idx];	//value in tile array, gives index into tile atlas

					if (tile != 0)	//if it's not an empty tile
					{
						shift1 = ((((int)Ship[i].xpos)-24) & 0x003f)>>1;	//bottom 6 bits, rs by 1 because col masks are half size
						start_row = 0;
						current_row = start_row;

						for (j=rows ; j<24 ; j++)	//start j where we left off - it's only used to count the ship mask.
						{
							//map_word1 = map_col_mask[map_idx];
							map_word1 = map_col_mask[(tile & 0x07) + (((tile&0xfff8)<<2)+current_row)*words_per_row];
							ship_word1 = (ship_col_mask[j*NUM_ANGLES + Ship[i].angle]);
							ship_word1_shift = ship_word1 >> shift1;

							if((ship_col_mask[j*NUM_ANGLES + Ship[i].angle] >> shift1) & map_word1)
							{
								CheckForLanding(i);			//check for landing
								if (!Ship[i].landed)		//if not landed, we're dead
								{
									if (!collision)
									{
									    Ship[i].shield = 0;
                                        Ship[i].crashed++;
                                        collision = 2;
									}
									break;
								}
							}
							current_row++;
						}
					}
				}

				if (shift1 > 8)	//running into next right tile
				{
					tile_idx = start_tile_idx + 1;
					tile = tile_map[tile_idx];	//value in tile array, gives index into tile atlas

					if (tile != 0)	//if it's not an empty tile
					{
						//shift1 = ((((int)Ship[i].xpos)-24) & 0x003f)>>1;	//bottom 6 bits, rs by 1 because col masks are half size
						shift2 = 31-shift1;
						start_row = ((((int)Ship[i].ypos)-24) & 0x003f)>>1;	//
						current_row = start_row;
						rows = 32-start_row;

						for (j=0 ; j<rows ; j++)
						{
							if (j==24) break;
							//map_word1 = map_col_mask[tile + current_row*words_per_row]; //again
							map_word1 = map_col_mask[(tile & 0x07) + (((tile&0xfff8)<<2)+current_row)*words_per_row];
							ship_word1 = (ship_col_mask[j*NUM_ANGLES + Ship[i].angle]);
							ship_word1_shift = ship_word1 << shift2;

							if((ship_col_mask[j*NUM_ANGLES + Ship[i].angle] << shift2) & map_word1)
							{
								CheckForLanding(i);			//check for landing
								if (!Ship[i].landed)		//if not landed, we're dead
								{
									if (!collision)
                                    {
                                        collision = 3;
                                        Ship[i].shield = 0;
                                        Ship[i].crashed++;
                                    }
									break;
								}
							}
							current_row++;
						}
					}

					//if (j != 24)	//we're running into the next tile down.
					if (rows < 24)
					{
						tile_idx = start_tile_idx + MAX_MAP_WIDTH + 1;
						tile = tile_map[tile_idx];	//value in tile array, gives index into tile atlas

						if (tile != 0)	//if it's not an empty tile
						{
							//shift1 = ((((int)Ship[i].xpos)-24) & 0x003f)>>1;	//bottom 6 bits, rs by 1 because col masks are half size
							shift2 = 31-shift1;
							start_row = 0;
							current_row = start_row;

							for (j=rows ; j<24 ; j++)	//start j where we left off - it's only used to count the ship mask.
							{
								//map_word1 = map_col_mask[map_idx];
								//map_word1 = map_col_mask[tile + current_row*words_per_row]; //and finally
								map_word1 = map_col_mask[(tile & 0x07) + (((tile&0xfff8)<<2)+current_row)*words_per_row];
								ship_word1 = (ship_col_mask[j*NUM_ANGLES + Ship[i].angle]);
								ship_word1_shift = ship_word1 << shift2;

								if((ship_col_mask[j*NUM_ANGLES + Ship[i].angle] << shift2) & map_word1)
								{
									CheckForLanding(i);			//check for landing
									if (!Ship[i].landed)		//if not landed, we're dead
									{
										if (!collision)
                                        {
                                            Ship[i].shield = 0;
                                            Ship[i].crashed++;
                                            collision = 4;
                                        }
										break;
									}
								}
								current_row++;
							}
						}
					}
				}
            }		//end of (if reincarnate timer)
		}			//end of for (ships)
	}
}

int find_wall(int i, int angle)
{
    int j,k;
    int distance = 0;
    int step = 10;
    int limit = 200;
    int x,y;

    for (j=0 ; j < limit ; j+=step)
    {
        x = (int)(Ship[i].xpos) + j*sinlut[angle];
        y = (int)(Ship[i].ypos) - j*coslut[angle];

        if (Map.type == 0 || Map.type == 2)
        {
            //TBD
        }
        else //Map.type == 1, i.e. tiled
        {
            //find the tile which the top-left corner of the ship is in
            tile_x = (x) >> 6;	//dividing by 64 here, as display tiles are 64
            tile_y = (y) >> 6;

            if (tile_x < 0) tile_x = 0;
            if (tile_y < 0) tile_y = 0;

            start_tile_idx =  tile_x + tile_y * MAX_MAP_WIDTH;	//index into tile array for the tile top-left corner is in
            tile = tile_map[start_tile_idx];	//value in tile array, gives index into tile atlas
            tl_tile = tile;

            shift1 = ((x-24) & 0x003f)>>1;	//bottom 6 bits, rs by 1 because col masks are half size
            start_row = ((y-24) & 0x003f)>>1;	//
            current_row = start_row;
            rows = 32-start_row;		//rows until we get to the bottom of the tile

            if (tile != 0)	//if it's not an empty tile
            {
                //for (k=0 ; k<rows ; k++)
                {
                    //if (k==24) break;
                    //map_word1 = map_col_mask[tile + current_row*words_per_row]; //here
                    map_word1 = map_col_mask[(tile & 0x07) + (((tile&0xfff8)<<2)+current_row)*words_per_row]; //bottom 3 bits of 'tile' index across (8 tiles per row in tilemap)
                    //ship_word1 = (ship_col_mask[12*NUM_ANGLES + angle]);                             //upper bits of 'tile' index down - R shift by 3, but then *32 => L shift by 2
                    //ship_word1_shift = ship_word1 >> shift1;

                    if(/*ship_word1_shift & */map_word1)
                    {
                        break;
                    }
                    current_row++;
                }
            }
        }
    }

    return j;
}

//we know we've collided; check if we're roughly vertical, and on a pad.
void CheckForLanding(int i)
{
	int k;

	if ((Ship[i].angle) < 5 || (Ship[i].angle) > NUM_ANGLES-5)
	{
		for(k=0 ; k<Map.num_pads ; k++)
		{
			if(Ship[i].ypos < Map.pad[k].y + 4 && Ship[i].ypos > Map.pad[k].y - 4 )
            {
				if(Ship[i].xpos > Map.pad[k].min_x)
                {
					if(Ship[i].xpos < Map.pad[k].max_x)
					{
						Ship[i].landed = 1;
						Ship[i].pad = k;
						Ship[i].fangle = 0;
						Ship[i].angle = 0;			//position correctly
						Ship[i].ypos = Map.pad[k].y-2;
						Ship[i].xv = 0;
						Ship[i].yv = 0;

						if (Map.mission)			//restart on last pad.
						{
							Ship[i].home_pad = k;

							if (k == Map.num_pads-1)	//if it's the last pad
							{
								Ship[i].racing = true;	//stop the clock
							}
						}
					}
                }
            }
		}
	}
	return;
}

/*
for tiled maps:
Need to find 2 words, as above. Difference is that second word may be in a different tile, and so not consecutive.
Further difference is that as we work our way down, we may run into the next tile down.

square tiles easier, so assume we have 64x64 (display) tiles, 32x32 collision

try something like:

//top left
map_idx similar to above - shift / add (shifting awkward with non-square tile?)
get from map_idx to tile - lookup.

if (tile == 0) ; //empty tile
else
{
	shift ship word
	for (i=0 ; i<24 ; i++)
	{
		if (ship_word & map_word)
			collision();

		if ( y .... ) break;		//exit loop if we get to the bottom of the tile
	}
}

if (shift < something) //all of ship word, so don't need another tile to the right

else
{
	map_idx++;	//repeat
	lookup
}



*/

int bullet_word, map_word;

void CheckBWCollisions(void)
{
	int i,j=0,shift;

	if (Map.type == 0 || Map.type == 2)
	{
		for (i=first_bullet ; i != END_OF_LIST ; i = Bullet[i].next_bullet)
		{
				for (j=-3 ; j<4 ; j++)
				{
					map_idx_x = (int)(Bullet[i].xpos + 0.25*j*Bullet[i].xv +64) >> 6;

					map_idx_y = (int)(Bullet[i].ypos + 0.25*j*Bullet[i].yv +24) >> 1;

					map_idx =  map_idx_x + map_idx_y * words_per_row; //(mapx >> 6);	//This assumes that display map and collision map are the same size

					if (map_idx < 0) map_idx = 0;

					shift =  ( (int)(Bullet[i].xpos + 0.25*j*Bullet[i].xv ) >> 1) & 0x001F;

					map_word = map_col_mask[map_idx];

					//if (shift >=0)
					//	bullet_word = 0x00018000 << shift;
					//else
					//	bullet_word = 0x00018000 >> -1*shift;

					bullet_word = 0x80000000 >> shift;

					if (map_word & bullet_word)
					{
						if (Bullet[i].type == BLT_MINE)
						{
							Bullet[i].xv = 0;
							Bullet[i].yv = 0;
						}
						else
							Bullet[i].ttl = 1;
					}
				}
		}
	}
	else	//tiled map
	{
		//breaks if bullets go off edge of map. force x/y to map dimensions......
		for (i=first_bullet ; i != END_OF_LIST ; i = Bullet[i].next_bullet)
		{
			for (j=-3 ; j<4 ; j++)
			{
				tile_x = (int)(Bullet[i].xpos + 0.25*j*Bullet[i].xv) >> 6;	//dividing by 64 here, as display tiles are 64
				tile_y = (int)(Bullet[i].ypos + 0.25*j*Bullet[i].yv) >> 6;
				current_row = ((int)(Bullet[i].ypos + 0.25*j*Bullet[i].yv) & 0x003f) >> 1;

				start_tile_idx =  tile_x + tile_y * MAX_MAP_WIDTH;	//index into tile array for the tile top-left corner is in

				if (start_tile_idx < 0) start_tile_idx = 0;

				tile = tile_map[start_tile_idx];	//value in tile array, gives index into tile atlas

				//map_word = map_col_mask[tile + current_row*words_per_row];
				map_word = map_col_mask[(tile & 0x07) + (((tile & 0xfff8)<<2)+current_row)*words_per_row];

				shift =  ( (int)(Bullet[i].xpos + 0.25*j*Bullet[i].xv ) >> 1) & 0x001F;

				//if (shift >=0)
				//	bullet_word = 0x00018000 << shift;
				//else
				//	bullet_word = 0x00018000 >> -1*shift;

				bullet_word = 0x80000000 >> shift;

				if (map_word & bullet_word)
				{
					if (Bullet[i].type == BLT_MINE)
					{
						Bullet[i].xv = 0;
						Bullet[i].yv = 0;
					}
					else
						Bullet[i].ttl = 1;
				}
			}
		}
	}
}

int EquivalentColour(ALLEGRO_COLOR col1, ALLEGRO_COLOR col2)
{
	return col1.r == col2.r && col1.g == col2.g && col1.b == col2.b && col1.a == col2.a;
}
