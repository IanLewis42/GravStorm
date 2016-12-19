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
#include "network.h"

//int DoOldMenu(ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_EVENT event, ShipType AnyShip);
int DoNewMenu(ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_EVENT event, ShipType AnyShip);

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

	fprintf(logfile,"Title Screen\n");

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
	fprintf(logfile,"Exit Title Screen\n");
	return 0;
}

int DoMenu(ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_EVENT event)
{
	int i;
	int w,h,xoffset,yoffset;
	ShipType AnyShip;
	ALLEGRO_SAMPLE_INSTANCE *loop_inst;

	fprintf(logfile,"\nStart Menu\n");

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
    logo = al_load_bitmap("gs.png");
    if ((ships = al_load_bitmap("ships.png")) == NULL)  fprintf(logfile,"ships.png load fail");

	fprintf(logfile,"\nSelected map %d,%d\n",Menu.group,Menu.map);

	if (init_map(Menu.group, Menu.map))		//opens 'map name.txt' and copies into MapTypeStruct.
	{								//everything else (load map, make col map, init ships etc. reads from MapType struct
		fprintf(logfile,"Failed to open map file\n");
		return 0;
	}
    //play background music
    loop_inst = al_create_sample_instance(loop);
    al_attach_sample_instance_to_mixer(loop_inst, mixer);
    al_set_sample_instance_playmode(loop_inst, ALLEGRO_PLAYMODE_LOOP);
    al_play_sample_instance(loop_inst);

	//if (DoOldMenu(queue, event, AnyShip)) return 1;
	if (DoNewMenu(queue, event, AnyShip)) return 1;

	//stop music
	al_stop_sample_instance(loop_inst);
	al_destroy_sample_instance(loop_inst);

	fprintf(logfile,"\nPlaying map %s (%d,%d)\n",(char*)&MapNames[Menu.group].Map[Menu.map],Menu.group,Menu.map);
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

FILE *hostfile,*clientfile;

int DoNewMenu(ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_EVENT event, ShipType AnyShip)
{
	int i;
	int w,h,xoffset,yoffset;

    Ship[0].angle = 0;
    Ship[1].angle = 10;
    Ship[2].angle = 20;
    Ship[3].angle = 30;

    event.keyboard.keycode = 0;

    Net.pingtimer = 0;

    while(1)
	{
		al_wait_for_event(queue, &event);

        if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
            return 1;

        else if (event.type == ALLEGRO_EVENT_DISPLAY_RESIZE)
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

		else if (event.type == ALLEGRO_EVENT_TIMER && al_is_event_queue_empty(queue))
		{
			display_new_menu();

			//expanding maps
			if (Menu.expand < 35)
				Menu.expand += 5;

            if (gpio_active) ReadGPIOJoystick();

             Net.pingtimer++;   //used by client searching for server
		}

		else if (event.type == ALLEGRO_EVENT_KEY_DOWN)
		{
            if (al_get_sample_instance_playing(clunk_inst))
                al_stop_sample_instance(clunk_inst);
            al_play_sample_instance(clunk_inst);

            //non-joystick things
            if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
            {
                switch (Menu.state)
                {
                case NETWORK:
                    Exit();   //escape to exit
                break;
                case LEVEL:
                    Menu.state = NETWORK;            //or back to previous menu
                break;
                case PLAYERS:
                    if (Menu.netmode == CLIENT)
                    {
                        Menu.state = NETWORK;            //or back to previous menu

                        if (Net.client_state == CONNECTED || Net.client_state == RUNNING)
                        {
                            NetDisconnectClient();

                            while (Net.client_state != IDLE)
                            {
                                ServiceNetwork();
                            }
                            NetStopClient();
                        }
                        //Net.client_state = IDLE;        //stop client ping
                        //Net.client = false;
                    }
                    else if (Menu.netmode == HOST)
                    {
                        Menu.state = LEVEL;
                        //disconnect clients
                        NetStopListen();    //stop new connections

                        NetSendAbort();         //tell clients to disconnect
                        while (num_ships > 1)   //wait until they do
                            ServiceNetwork();

                        NetStopServer();        //kill server
                        Net.server = false;
                    }
                    else
                        Menu.state = LEVEL;

                break;
                }
                //Menu.col_pos = 0;
                //Menu.netmode = LOCAL;
            }

			else if (Menu.define_keys)// && event.type == ALLEGRO_EVENT_KEY_DOWN)
			{
				switch (Menu.current_key)
				{
					case 0:
						Ship[Menu.player].left_key = event.keyboard.keycode;
						//event.keyboard.keycode = 0;
						Menu.current_key++;
						break;
					case 1:
						Ship[Menu.player].right_key = event.keyboard.keycode;
						//event.keyboard.keycode = 0;
						Menu.current_key++;
						break;
					case 2:
						Ship[Menu.player].up_key = event.keyboard.keycode;
						//event.keyboard.keycode = 0;
						Menu.current_key++;
						break;
					case 3:
						Ship[Menu.player].down_key = event.keyboard.keycode;
						//event.keyboard.keycode = 0;
						Menu.current_key++;
						break;
					case 4:
						Ship[Menu.player].thrust_key = event.keyboard.keycode;
						//event.keyboard.keycode = 0;
						Menu.current_key = 0;
						Menu.define_keys = false;
						break;
				}
			}

            else
            key_down_log[event.keyboard.keycode]=true;  //log it
		}
		//_CHAR event for typing in IP address
		/*
		else if (event.type == ALLEGRO_EVENT_KEY_CHAR)
        {
            if (Menu.state == NETWORK && Menu.col_pos == 1) //defining address
            {
                //return picked up elsewhere....
                if (event.keyboard.keycode == ALLEGRO_KEY_BACKSPACE)  //backspace
                {
                    if (strlen(Net.temp_address) > 0)
                        Net.temp_address[strlen(Net.temp_address)-1] = 0;   //so terminate string one earlier
                }
                else
                    strncat(Net.temp_address,(char*)&event.keyboard.unichar,1);    //append char
            }
        }
        */

		//See if it's a USB joystick event. If it is, log it.
 		else CheckUSBJoyStick(event);

		//finished checking events

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

		//now have all inputs recorded as AnyShip elements (non-ship keypresses will be in key_down_log)
		//actions are dependent on state....
		switch (Menu.state)
        {
            case NETWORK:
                //Menu.state = LEVEL; //no network yet....
                if (AnyShip.thrust_down)
                {
                    AnyShip.thrust_down = false;
                    /*
                    if (Menu.col_pos == 1)  //editing address
                    {
                        strncpy(Net.menuaddress,Net.temp_address,16);
                        //Net.temp_address[0] = 0;
                    }
                    */
                    if (Menu.netmode == HOST)
                    {
                        Menu.state = LEVEL;               //go to next menu
                        //Menu.col_pos = 0;
                        Menu.group = 1;
                        init_map(Menu.group, Menu.map);
                    }
                    if (Menu.netmode == CLIENT)
                    {
                        Menu.ships = 1;
                        Menu.state = PLAYERS;
                        Menu.col_pos = 1;

                        //Net.net = true;
                        Net.server = false;
                        Net.client = true;
                        //Net.connected = 0;
                        clientfile = fopen("../client.txt","w");
                        NetStartNetwork();
                        //NetStartClient();
                        Net.client_state = SEARCHING;   //starts pinging
                    }
                    else
                    {
                        //Net.net = false;
                        Menu.state = LEVEL;               //go to next menu
                        Menu.col_pos = 0;
                    }
                }

                else if (AnyShip.fire2_down)    //down
                {
                    AnyShip.fire2_down = false;
                    //if (Menu.col_pos < 2/*Menu.max_col_pos*/)
                    if (Menu.netmode < CLIENT)
                    {
                        //Menu.col_pos++;
                        Menu.netmode++;
                    }
                }

                else if(AnyShip.fire1_down) //up!
                {
                    AnyShip.fire1_down = false;
                    //if (Menu.col_pos > 0)
                    if (Menu.netmode > LOCAL)
                    {
                        //Menu.col_pos--;
                        Menu.netmode--;
                    }
                }

                //Menu.netmode = Menu.col_pos;
                /*
                else if (AnyShip.left_down)    //left
                {
                    AnyShip.left_down = false;
                    //if (Menu.col_pos == 0)          //
                    {
                        if (Menu.netmode > 0)
                            Menu.netmode--;
                    }
                }
                else if (AnyShip.right_down)    //right
                {
                    AnyShip.right_down = false;
                    //if (Menu.col_pos == 0)
                    {
                        if (Menu.netmode < 2)
                            Menu.netmode++;
                    }
                }
                //if (Menu.netmode == CLIENT)
                //    Menu.max_col_pos = 1;
                //else
                    Menu.max_col_pos = 0;
                */
            break;
            case LEVEL: //map selection
                if (AnyShip.thrust_down)
                {
                    AnyShip.thrust_down = false;
                    Menu.state = PLAYERS;               //thrust to go to next menu

                    if (Map.max_players == 1) num_ships = 1;    //default 2 players, unless max is 1
                    else num_ships = 2;

                    Menu.ships = num_ships; //for local
                    Menu.col_pos = 0;

                    if (Menu.netmode == HOST)
                    {
                        //Net.net = true;
                        Net.server = true;
                        //Net.clients = 1;    //that's us!
                        num_ships = 1;
                        //Net.clients_ready = 0;
                        hostfile = fopen("../host.txt","w");
                        NetStartNetwork();
                        NetStartHost(Map.max_players);
                        Menu.state = PLAYERS;               //go to next menu
                        Menu.ships = 1;
                        Menu.col_pos = 1;
                    }
                }

                else if (AnyShip.fire2_down)    //down
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
                }

                else if(AnyShip.fire1_down) //up!
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
                }
            break;

            case PLAYERS: //players/ship/controls
                if (Net.client)
                {
                    if (Net.client_state == IDLE)
                    {
                        NetStopClient();
                        Net.client = true;
                        Net.client_state = SEARCHING;       //start looking again
                    }
                }

                if (AnyShip.thrust_down)
                {
                    AnyShip.thrust_down = false;    //thrust to start game;
                    if (Menu.netmode == HOST)
                    {
                        Menu.state = LEVEL;           //set state for when we return to menu
                        return 0;                       //return from function to start game
                    }
                    if (Menu.netmode == CLIENT)
                    {
                        if (Net.client_state == CONNECTED)  //if not connected, ignore.
                        {
                            Menu.state = NETWORK;           //set state for when we return to menu
                            Menu.col_pos = 1;
                            return 0;                       //return from function to start game
                        }
                    }
                    else
                    {
                        Menu.state = LEVEL;
                        Menu.col_pos = 0;
                        return 0;                       //return from function to start game
                    }

                }
                else if (AnyShip.fire2_down)    //down
                {
                    AnyShip.fire2_down = false;
                    if (Menu.col_pos < Menu.ships*3) Menu.col_pos++;             //move down, unless we're at the end -
                    Menu.player = (Menu.col_pos-1) / 3;                         //work out player
                    Menu.item   = (Menu.col_pos-1) % 3;                         //..and item (ship, controller, define keys)
                    if (Menu.item == 2 && Ship[Menu.player].controller != KEYS) //skip 'define keys' if we haven't selected keys
                    {
                        if (Menu.col_pos < Menu.ships*3)
                            Menu.col_pos++;
                        else Menu.col_pos--;
                    }
                }
                else if (AnyShip.fire1_down)    //up
                {
                    AnyShip.fire1_down = false;
                    if (Menu.col_pos > 0)
                    {
                        Menu.col_pos--;
                        if (Menu.col_pos == 0 && (Net.client || Net.server))            //can't change this if networked.
                            Menu.col_pos++;
                    }
                    Menu.player = (Menu.col_pos-1) / 3;
                    Menu.item   = (Menu.col_pos-1) % 3;
                    if (Menu.item == 2 && Ship[Menu.player].controller != KEYS)
                        Menu.col_pos--;
                }
                else if (AnyShip.left_down)    //left
                {
                    AnyShip.left_down = false;
                    if (Menu.col_pos == 0)          //num of players
                    {
                        //if (!Net.net)
                            if (num_ships > 1)
                            {
                                num_ships--;
                                Menu.ships--;
                            }
                    }
                    else if (Menu.item == 0)        //ship image
                    {
                        Ship[Menu.player].image--;
                        Ship[Menu.player].image&=0x07;
                        Ship[Menu.player].colour = ShipColour[Ship[Menu.player].image];
                        Ship[Menu.player].statuscolour = StatusColour[Ship[Menu.player].image];
                    }
                    else if (Menu.item == 1)        //controls
                    {
                        if (Ship[Menu.player].controller > 0)
                        Ship[Menu.player].controller--;
                    }
                }
                else if (AnyShip.right_down)    //right
                {
                    AnyShip.right_down = false;
                    if (Menu.col_pos == 0)
                    {
                       // if (!Net.net)
                        {

                            if (num_ships < Map.max_players)
                            {
                                num_ships++;
                                Menu.ships++;
                            }
                        }
                    }

                    else if (Menu.item == 0) //ship
                    {
                        Ship[Menu.player].image++;
                        Ship[Menu.player].image&=0x07;
                        Ship[Menu.player].colour = ShipColour[Ship[Menu.player].image];
                        Ship[Menu.player].statuscolour = StatusColour[Ship[Menu.player].image];
                    }
                    else if (Menu.item == 1)    //controls
                    {
                        if (Ship[Menu.player].controller < 3)
                            Ship[Menu.player].controller++;
                    }
                    else if (Menu.item == 2)    //define keys
                    {
                        Menu.define_keys = true;    //done above
                    }
                }

                Menu.player = (Menu.col_pos-1) / 3;
                Menu.item   = (Menu.col_pos-1) % 3;

            break;
            default:
                Menu.state = 0;
            break;
        }
        event.keyboard.keycode = 0;

        ServiceNetwork();

	}
}

