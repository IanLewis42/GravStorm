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
//#include <math.h>

#include "allegro5/allegro.h"
#include "allegro5/allegro_image.h"
#include "allegro5/allegro_primitives.h"
#include "allegro5/allegro_font.h"
#include "allegro5/allegro_ttf.h"
#include "allegro5/allegro_audio.h"
#include "allegro5/allegro_acodec.h"

#include "game.h"
#include "drawing.h"
#include "init.h"
#include "collisions.h"
#include "objects.h"
#include "inputs.h"


int DoTitle(ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_EVENT event)
{
	ALLEGRO_TRANSFORM transform;
    ALLEGRO_SAMPLE *slam;
    ALLEGRO_SAMPLE_INSTANCE *slam_inst;
	ALLEGRO_SAMPLE_INSTANCE *wind_inst;


	float ht = 100;	//total height (light)
	float hc = 10;	//camera height
	float y = 0;	//how far the text has fallen
	float a = 10;	//width of text
	float shadow_scale, text_scale;
	int sound_latency;

	char fade_in[200],fade_out[200],visible[200];
	int fade_in_y = 600, visible_y = 570, fade_out_y = 540;
	int fade_count=0;
	float alpha;

	FILE* credits;

	if ((menu_bg_bmp = al_load_bitmap("menu_bg.png")) == NULL) fprintf(logfile,"menu_bg.png load fail\n");
	if ((logo = al_load_bitmap("gs.png")) == NULL) fprintf(logfile,"gs.png load fail\n");
	if ((credits = fopen ("credits.txt","r")) == NULL)  fprintf(logfile,"credits.txt load fail\n");
	fflush(logfile);

	fade_in[0] = 0;
	visible[0] = 0;
	fade_out[0] = 0;

#if RPI
    sound_latency = 15;
#else
    sound_latency = 1;
#endif // RPI

    if ((slam = al_load_sample   ("slam.wav"))    == NULL)  fprintf(logfile,"slam.wav load fail");
	slam_inst = al_create_sample_instance(slam);
    al_attach_sample_instance_to_mixer(slam_inst, mixer);

    wind_inst = al_create_sample_instance(wind);
    al_attach_sample_instance_to_mixer(wind_inst, mixer);
    al_set_sample_instance_playmode(wind_inst, ALLEGRO_PLAYMODE_LOOP);

	al_play_sample_instance(wind_inst);//, 1, 0, 1, ALLEGRO_PLAYMODE_LOOP, NULL);

	//al_play_sample(wind, 5, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);

	while(1)
	{
		al_wait_for_event(queue, &event);

        if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
            return 1;

        if (event.type == ALLEGRO_EVENT_DISPLAY_RESIZE)
            al_acknowledge_resize(display);

		if (event.type == ALLEGRO_EVENT_TIMER)
		{
			if (y < ht) y++;

			al_set_sample_instance_gain(wind_inst,5*y/ht);
			if (y == ht-sound_latency)
				//al_play_sample(clunk, 5, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
				al_play_sample_instance(slam_inst);

			shadow_scale = (ht*a/y)/hc;	//sin theta. try arcsin???
			text_scale = a/(y-(ht-hc)); //ditto

			al_clear_to_color(al_map_rgb(0, 0, 0));
			al_draw_bitmap(menu_bg_bmp,0,0,0);

			//al_draw_textf(font, al_map_rgb(255, 255, 0),200, 200,  ALLEGRO_ALIGN_LEFT, "Count:%d",count);

			al_identity_transform(&transform);			/* Initialize transformation. */
			al_scale_transform(&transform, shadow_scale, shadow_scale);	/* Rotate and scale around the center first. */
			al_translate_transform(&transform,SCREENX/2,SCREENY/2);
			al_use_transform(&transform);

			//try (y/ht) squared
			al_draw_textf(title_font, al_map_rgba(0, 0, 0,128*(y/ht)*(y/ht)),0, -1*al_get_font_ascent(title_font)/2,  ALLEGRO_ALIGN_CENTRE, "%s", NAME);

    		if (y > (ht-hc))
    		{
				al_identity_transform(&transform);
				al_scale_transform(&transform, text_scale, text_scale);	/* Rotate and scale around the center first. */
				al_translate_transform(&transform,SCREENX/2-10,SCREENY/2-10);
				al_use_transform(&transform);
				//al_draw_textf(title_font, al_map_rgb(128, 128, 0),0, -1*al_get_font_ascent(title_font)/2,  ALLEGRO_ALIGN_CENTRE, "%s", NAME);
				al_draw_bitmap(logo,-SCREENX/2+15,-12,0);
			}

			al_identity_transform(&transform);
    		al_use_transform(&transform);

    		if (y==ht)	//start credits
    		{
				if (fade_count == 0)
				{
					strcpy(fade_out,visible);
					strcpy(visible,fade_in);
					if (fgets(fade_in,200,credits) == NULL) break;
					fade_count = 30;
				}
				alpha = (1.0/30)*(30-fade_count);
				al_draw_textf(small_font, al_map_rgba_f(0.35*alpha, 0, 0, alpha)/*8*(30-fade_count))*/,SCREENX/2, fade_in_y+fade_count,  ALLEGRO_ALIGN_CENTRE, "%s",fade_in);
				al_draw_textf(small_font, al_map_rgba_f(0.35, 0, 0, 1),SCREENX/2, visible_y+fade_count,  ALLEGRO_ALIGN_CENTRE, "%s",visible);
				alpha = (1.0/30)*fade_count;
				al_draw_textf(small_font, al_map_rgba_f(0.35*alpha, 0, 0, alpha)/*8*fade_count)*/,SCREENX/2, fade_out_y+fade_count,  ALLEGRO_ALIGN_CENTRE, "%s",fade_out);

				fade_count--;
			}

    		al_flip_display();
		}
		else if (event.type == ALLEGRO_EVENT_KEY_DOWN)
			break;
	}
	fclose(credits);
	al_stop_sample_instance(wind_inst);
	al_destroy_sample_instance(wind_inst);
	//al_destroy_mixer(mixer);
	//al_destroy_voice(voice);

	FreeMenuBitmaps();
	return 0;
}

int DoMenu(ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_EVENT event)
{
	int i;
	int w,h,xoffset,yoffset;
	ShipType AnyShip;

	fprintf(logfile,"Init Keyboard\n");
	init_keys(pressed_keys);
	init_keys(key_down_log);
	init_keys(key_up_log);

	AnyShip.left_down = false;
	AnyShip.right_down = false;
	AnyShip.fire1_down = false;
	AnyShip.fire2_down = false;
	AnyShip.thrust_down = false;

	for (i=0 ; i<MAX_SHIPS ; i++)
    {
        Ship[i].left_down = false;
        Ship[i].right_down = false;
        Ship[i].fire1_down = false;
        Ship[i].fire2_down = false;
        Ship[i].thrust_down = false;
    }

	Menu.num_groups = read_maps();	//read 'maps.txt' and store strings
	//RETURN VALUE HERE
	//MAYBE NEED CURRENT GROUP / CURRENT MAP in struct somewhere???

    w = al_get_display_width(display);
    h = al_get_display_height(display);

    al_set_clipping_rectangle(0, 0, w, h);

    yoffset = (h-SCREENY)/2;
    if (yoffset < 0) yoffset = 0;

    xoffset = (w-SCREENX)/2;
    if (xoffset < 0) xoffset = 0;

    Menu.x_origin = xoffset;
    Menu.y_origin = yoffset;

	//Menu.map = 0;
	Menu.col = 0;
	Menu.offset = 0;

	menu_bg_bmp = al_load_bitmap("menu_bg.png");
    logo = al_load_bitmap("gs.png");;

	fprintf(logfile,"\nSelected map %d,%d\n",Menu.group,Menu.map);

	if (init_map(Menu.group, Menu.map))		//opens 'map name.txt' and copies into MapTypeStruct.
	{								//everything else (load map, make col map, init ships etc. reads from MapType struct
		fprintf(logfile,"Failed to open map file\n");
		return 0;
	}

	while(1)
	{
		al_wait_for_event(queue, &event);

        if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
            return 1;

			if (event.type == ALLEGRO_EVENT_DISPLAY_RESIZE)
            {
                al_acknowledge_resize(display);

                w = al_get_display_width(display);
                h = al_get_display_height(display);

                al_set_clipping_rectangle(0, 0, w, h);

                yoffset = (h-SCREENY)/2;
                if (yoffset < 0) yoffset = 0;

                xoffset = (w-SCREENX)/2;
                if (xoffset < 0) xoffset = 0;

                Menu.x_origin = xoffset;
                Menu.y_origin = yoffset;

                if (Menu.col >= 2)
                    Menu.offset = Menu.x_origin-460;
                else
                    Menu.offset = Menu.x_origin;
            }

		if (event.type == ALLEGRO_EVENT_TIMER && al_is_event_queue_empty(queue))
		{
			display_menu();//Menu.num_maps, Menu.map);	//show strings, highlight selected

			//sliding columns
			if (Menu.col >= 2)
			{
				if (Menu.offset > Menu.x_origin-460)
					Menu.offset -= 20;
			}
			else
			{
				if (Menu.offset < Menu.x_origin)
					Menu.offset += 20;
			}

			//expanding maps
			if (Menu.expand < 55)
				Menu.expand += 5;

            if (gpio_active) ReadGPIOJoystick();
		}

		else if (event.type == ALLEGRO_EVENT_KEY_DOWN)
		{
			//al_play_sample(clunk, 1, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
            if (al_get_sample_instance_playing(clunk_inst))
                al_stop_sample_instance(clunk_inst);
            al_play_sample_instance(clunk_inst);

			//Escape to exit trumps everything
 			if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE) Exit();

			//Otherwise, check if we're defining keys, so we can allow any keys (other than escape) here
			//NB only way out of column 3 is to get to the end of this process (i.e. define all 5 keys
			//which automagically dumps us back into col 2.
			else if (Menu.col == 3)
			{
				switch (Menu.current_key)
				{
					case 0:
						Ship[Menu.player].left_key = event.keyboard.keycode;
						Menu.current_key++;
						break;
					case 1:
						Ship[Menu.player].right_key = event.keyboard.keycode;
						Menu.current_key++;
						break;
					case 2:
						Ship[Menu.player].up_key = event.keyboard.keycode;
						Menu.current_key++;
						break;
					case 3:
						Ship[Menu.player].down_key = event.keyboard.keycode;
						Menu.current_key++;
						break;
					case 4:
						Ship[Menu.player].thrust_key = event.keyboard.keycode;
						Menu.current_key = 0;
						Menu.col = 2;
						break;

					default:
					break;
				}
			}
			//otherwise just log the key
            else
            {
                key_down_log[event.keyboard.keycode]=true;
	 		}
		}

		//See if it's a USB joystick event. If it is, log it.
 		else CheckUSBJoyStick(event);

		//turn any pending keypresses / Joystick events into Ship controls
        ScanInputs(MAX_SHIPS);

		//Take any ship control to control menu. Also, cursor keys / return always work.
        for (i=0 ; i<MAX_SHIPS ; i++)
        {
			if (Ship[i].left_down || key_down_log[ALLEGRO_KEY_LEFT])
			{
				AnyShip.left_down = true;
				Ship[i].left_down = false;
	 			key_down_log[ALLEGRO_KEY_LEFT] = false;

			}
			if (Ship[i].right_down || key_down_log[ALLEGRO_KEY_RIGHT])
			{
				AnyShip.right_down = true;
				Ship[i].right_down = false;
				key_down_log[ALLEGRO_KEY_RIGHT] = false;
			}
			if (Ship[i].fire1_down || key_down_log[ALLEGRO_KEY_UP])
			{
				AnyShip.fire1_down = true;
				Ship[i].fire1_down = false;
				key_down_log[ALLEGRO_KEY_UP] = false;
			}
			if (Ship[i].fire2_down || key_down_log[ALLEGRO_KEY_DOWN])
			{
				AnyShip.fire2_down = true;
				Ship[i].fire2_down = false;
				key_down_log[ALLEGRO_KEY_DOWN] = false;
			}
			if (Ship[i].thrust_down || key_down_log[ALLEGRO_KEY_ENTER])
			{
				AnyShip.thrust_down = true;
				Ship[i].thrust_down = false;
				key_down_log[ALLEGRO_KEY_ENTER] = false;
			}
		}

		//Next, check to see if we're exiting menu to game
		//else if (event.keyboard.keycode == ALLEGRO_KEY_ENTER)// || event.keyboard.keycode == ALLEGRO_KEY_LCTRL)
		if (AnyShip.thrust_down)
		{
			AnyShip.thrust_down = false;
			break;
		}

		//Next, check for column change
		//else if (event.keyboard.keycode == ALLEGRO_KEY_RIGHT)
		else if (AnyShip.right_down)
		{
			AnyShip.right_down = false;
			if (Menu.col < 3)
			{
				Menu.col++;
				if (Menu.col == 3)
					if (Ship[Menu.player].controller != KEYS)
						Menu.col = 2;
			}
		}
		//else if (event.keyboard.keycode == ALLEGRO_KEY_LEFT)
		else if (AnyShip.left_down)
		{
			AnyShip.left_down = false;
			if (Menu.col > 0)
				Menu.col--;
		}

		//If we're not doing any of those things, decide what to do depending on which column we're in
		else
		{
			switch (Menu.col)
			{
				case 0:	//maps list
					//if (event.keyboard.keycode == ALLEGRO_KEY_DOWN)
					if (AnyShip.fire2_down)
					{
						AnyShip.fire2_down = false;

						if (Menu.map < MapNames[Menu.group].Count-1)	//more maps in group, so inc map count
							Menu.map++;
						else if(Menu.group < Menu.num_groups-1)		//no more maps, but another group so inc group
						{
							Menu.group++;
							Menu.map = 0;
							Menu.expand = 0;
						}

						fprintf(logfile,"\nSelected map %d,%d\n",Menu.group,Menu.map);
						init_map(Menu.group, Menu.map);
						//if (Menu.player > Map.max_players-1)
                        //    Menu.player = Map.max_players-1;
					}
					//else if (event.keyboard.keycode == ALLEGRO_KEY_UP)
					else if(AnyShip.fire1_down)
					{
						AnyShip.fire1_down = false;

						if (Menu.map > 0)	//more maps in group, so dec map count
							Menu.map--;
						else if(Menu.group > 0)		//no more maps, but another group so dec group
						{
							Menu.group--;
							Menu.map = MapNames[Menu.group].Count-1;
							Menu.expand = 0;
						}

						fprintf(logfile,"\nSelected map %d,%d\n",Menu.group,Menu.map);
						init_map(Menu.group, Menu.map);
						//if (Menu.player > Map.max_players-1)
                        //    Menu.player = Map.max_players-1;
					}
					for (i=0 ; i<MAX_SHIPS ; i++)	//work out number of players (1st N/A signals the end)
					{
						if (Ship[i].controller == NA)
						{
							//num_ships = i;
							break;
						}
					}
					num_ships = i;
					if (Menu.player > num_ships-1)
                        Menu.player = num_ships-1;

				break;

				case 1:	//player list
					//if (event.keyboard.keycode == ALLEGRO_KEY_DOWN)
					if (AnyShip.fire2_down)
					{
						AnyShip.fire2_down = false;
						if (Menu.player < Map.max_players-1)
							Menu.player++;
					}
					//else if (event.keyboard.keycode == ALLEGRO_KEY_UP)
					else if (AnyShip.fire1_down)
					{
						AnyShip.fire1_down = false;
						if (Menu.player > 0)
							Menu.player--;
					}
				break;

				case 2:	//controller types
					//if (event.keyboard.keycode == ALLEGRO_KEY_DOWN)
					if (AnyShip.fire2_down)
					{
						AnyShip.fire2_down = false;
						Ship[Menu.player].controller++;

						if (!gpio_active)
							if (Ship[Menu.player].controller == GPIO_JOYSTICK)
								Ship[Menu.player].controller++;

						if (Ship[Menu.player].controller > NA)
							Ship[Menu.player].controller = NA;

					}
					//else if (event.keyboard.keycode == ALLEGRO_KEY_UP)
					else if (AnyShip.fire1_down)
					{
						AnyShip.fire1_down = false;
						Ship[Menu.player].controller--;

						if (!gpio_active)
							if (Ship[Menu.player].controller == GPIO_JOYSTICK)
								Ship[Menu.player].controller--;

						if (Ship[Menu.player].controller < 0)
							Ship[Menu.player].controller = 0;
					}
					Ship[Menu.player].selected_controller = Ship[Menu.player].controller;	//keep a copy

					for (i=0 ; i<MAX_SHIPS ; i++)	//work out number of players (1st N/A signals the end)
					{
						if (Ship[i].controller == NA)
						{
							//num_ships = i;
							break;
						}
					}
					num_ships = i;
				break;
				default:
				break;
			}
		}
		//
	}

	fprintf(logfile,"Final Selected map %d\n",Menu.map);
	fprintf(logfile,"Number of players %d\n",num_ships);

	for (i=0 ; i<30 ; )
	{
		al_wait_for_event(queue, &event);
		if (event.type == ALLEGRO_EVENT_TIMER)
		{
			i++;
			display_map_text(0,i);	//this is the description text file. i fades the colours in and out
		}
	}

	return 0;
}

