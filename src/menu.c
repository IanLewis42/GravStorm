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

#define ALLEGRO_UNSTABLE 1  //needed for haptics.

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
int DoNewMenu(ALLEGRO_EVENT_QUEUE *queue);

void GotoNetwork(void);
void GotoLevel(void);
void GotoPlayers(void);
void GotoAI(void);

int DoTitle(ALLEGRO_EVENT_QUEUE *queue)
{
	ALLEGRO_TRANSFORM transform;
    ALLEGRO_SAMPLE_INSTANCE *slam_inst;
	ALLEGRO_SAMPLE_INSTANCE *wind_inst;
    ALLEGRO_EVENT event;
	ALLEGRO_BITMAP *ttg_logo;

	float ht = 100;	//total height (light)
	float hc = 10;	//camera height
	float y = 0;	//how far the text has fallen
	float a = 10;	//width of text
	float shadow_scale, text_scale;
	int sound_latency;
	//int line_space;
	int i;

	//char fade_in[200],fade_out[200],visible[200];
	//int fade_in_y = 600, visible_y = 570, fade_out_y = 540;
	//int fade_in_y = 600, visible_y = 570, fade_out_y = 540;
	int fade_count=30;
	//float alpha;

	int w,h,bgw,bgh,bgx,bgy,lw;

	ALLEGRO_FILE* credits;

    if ((ttg_logo = al_load_bitmap("tootired.png")) == NULL) al_fprintf(logfile,"tootired.png load fail\n");

    w = al_get_display_width(display);
    h = al_get_display_height(display);
    bgw = al_get_bitmap_width(ttg_logo);
    bgh = al_get_bitmap_height(ttg_logo);

    al_clear_to_color(al_map_rgb(255, 255, 255));
    al_draw_bitmap(ttg_logo,(w-bgw)/2,(h-bgh)/2,0);

    al_flip_display();

	if ((menu_bg_bmp = al_load_bitmap("menu_bg.png")) == NULL) al_fprintf(logfile,"menu_bg.png load fail\n");
	if ((logo = al_load_bitmap("gs2.png")) == NULL) al_fprintf(logfile,"gs.png load fail\n");


	if ((credits = al_fopen ("credits.txt","r")) == NULL)  al_fprintf(logfile,"credits.txt load fail\n");
	al_fflush(logfile);


    //fade_in_y  = 0.75*h +30*font_scale;
    //visible_y  = 0.75*h;
    //fade_out_y = 0.75*h-30*font_scale;

	al_fprintf(logfile,"Title Screen\n");

	//fade_in[0] = 0;
	//visible[0] = 0;
	//fade_out[0] = 0;

#if RPI
    sound_latency = 15;
#else
    sound_latency = 1;
#endif // RPI

    LoadSamples();

	slam_inst = al_create_sample_instance(slam);
    al_attach_sample_instance_to_mixer(slam_inst, mixer);

    wind_inst = al_create_sample_instance(wind);
    al_attach_sample_instance_to_mixer(wind_inst, mixer);
    al_set_sample_instance_playmode(wind_inst, ALLEGRO_PLAYMODE_LOOP);

	for (i=0 ; i<1 ; )
	{
		al_wait_for_event(queue, &event);

            if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
                return 1;

            if (event.type == ALLEGRO_EVENT_DISPLAY_RESIZE)
                al_acknowledge_resize(display);
            if (event.type ==
                ALLEGRO_EVENT_DISPLAY_HALT_DRAWING)   //we've been sidelined by the user/os
            {
                al_acknowledge_drawing_halt(display);   //acknowledge
                halted = true;                          //flag to drawing routines to do nothing
                al_stop_timer(
                        timer);                   //no more timer events, so we should do nothing, saving battery
                #ifdef ANDROID
                    al_set_default_voice(NULL);             //destroy voice, so no more sound events, ditto.
                #endif // ANDROID
                //break;
            }
            if (event.type == ALLEGRO_EVENT_DISPLAY_RESUME_DRAWING) //we've been restored
            {
                al_acknowledge_drawing_resume(display); //ack
                halted = false;                         //remove flag
                al_start_timer(timer);                  //restart timer
                voice = al_create_voice(44100, ALLEGRO_AUDIO_DEPTH_INT16,
                                        ALLEGRO_CHANNEL_CONF_2);  //restart audio
                al_attach_mixer_to_voice(mixer, voice);
                //break;
            }
            if (event.type == ALLEGRO_EVENT_TIMER) {
                i++;
            }
	}

    bgw = al_get_bitmap_width(menu_bg_bmp);
    bgh = al_get_bitmap_height(menu_bg_bmp);

	al_play_sample_instance(wind_inst);//, 1, 0, 1, ALLEGRO_PLAYMODE_LOOP, NULL);

	//al_play_sample(wind, 5, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);

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
            LoadFonts(0);
            //fade_in_y  = 0.75*h +30*font_scale;
            //visible_y  = 0.75*h;
            //fade_out_y = 0.75*h-30*font_scale;
        }

		if (event.type == ALLEGRO_EVENT_TIMER)
		{
			if (y < ht) y++;

			al_set_sample_instance_gain(wind_inst,5*y/ht);
			if (y == ht-sound_latency) {
                //al_play_sample(clunk, 5, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
                al_play_sample_instance(slam_inst);
                //Vibrate(200);
            }

			shadow_scale = (ht*a/y)/hc;	//sin theta. try arcsin???
			text_scale = a/(y-(ht-hc)); //ditto

			al_clear_to_color(al_map_rgb(0, 0, 0));
			//al_draw_bitmap(menu_bg_bmp,0,0,0);
            al_identity_transform(&transform);			/* Initialize transformation. */
            //al_scale_transform(&transform, shadow_scale, shadow_scale);	/* Rotate and scale around the center first. */
            //al_translate_transform(&transform,w/2,h/2);
            al_use_transform(&transform);
            for(bgy=0 ; bgy<h ; bgy+=bgh)
            {
                for(bgx=0 ; bgx<w ; bgx+=bgw)
                {
                    al_draw_bitmap(menu_bg_bmp,bgx,bgy,0);
                }
            }

			//al_draw_textf(font, al_map_rgb(255, 255, 0),200, 200,  ALLEGRO_ALIGN_LEFT, "Y:%0.1f",y);

			al_identity_transform(&transform);			/* Initialize transformation. */
			al_scale_transform(&transform, shadow_scale, shadow_scale);	/* Rotate and scale around the center first. */
			al_translate_transform(&transform,w*1.02/2,h/2);
			al_use_transform(&transform);

			//try (y/ht) squared
			al_draw_textf(title_font, al_map_rgba(0, 0, 0,160*(y/ht)*(y/ht)),0, -1*al_get_font_ascent(title_font)/2,  ALLEGRO_ALIGN_CENTRE, "%s", NAME);

    		if (y > (ht-hc))
    		{
                lw = al_get_bitmap_width(logo);
                al_identity_transform(&transform);
				al_scale_transform(&transform, text_scale*font_scale, text_scale*font_scale);	/* Rotate and scale around the center first. */
				al_translate_transform(&transform,(w/2)-(lw*text_scale*font_scale/2),(h/2)-(int)(26*font_scale));
				al_use_transform(&transform);
				//al_draw_textf(title_font, al_map_rgb(128, 128, 0),0, -1*al_get_font_ascent(title_font)/2,  ALLEGRO_ALIGN_CENTRE, "%s", NAME);
				//al_draw_bitmap(logo,-w/(2*font_scale),0,0);
                al_draw_bitmap(logo,0,0,0);
			}

			al_identity_transform(&transform);
    		al_use_transform(&transform);

    		//line_space = 30*font_scale;

    		if (y==ht)	//start credits
    		{
				if (fade_count == 20)
                    Vibrate(100);

                if (fade_count == 0)
				{
					break;
                    //strcpy(fade_out,visible);
					//strcpy(visible,fade_in);
					////if (fgets(fade_in,200,credits) == NULL) break;
                    //if (al_fgets(credits,fade_in,200) == NULL) break;
					//fade_count = line_space;
				}
				//alpha = (1.0/line_space)*(line_space-fade_count);
				//al_draw_textf(small_font, al_map_rgba_f(0.35*alpha, 0, 0, alpha)/*8*(30-fade_count))*/,w/2, fade_in_y+fade_count,  ALLEGRO_ALIGN_CENTRE, "%s",fade_in);
				//al_draw_textf(small_font, al_map_rgba_f(0.35, 0, 0, 1),w/2, visible_y+fade_count,  ALLEGRO_ALIGN_CENTRE, "%s",visible);
				//alpha = (1.0/line_space)*fade_count;
				//al_draw_textf(small_font, al_map_rgba_f(0.35*alpha, 0, 0, alpha)/*8*fade_count)*/,w/2, fade_out_y+fade_count,  ALLEGRO_ALIGN_CENTRE, "%s",fade_out);

				fade_count--;
			}

    		al_flip_display();
		}
		else if (event.type == ALLEGRO_EVENT_KEY_DOWN)
			break;
        else if (event.type == ALLEGRO_EVENT_TOUCH_BEGIN)
        {
            int i = 0;                              //log touch for consistency with later code
            while (Touch[i].id != NO_TOUCH) i++;    //need protection for running out of array.....

            Touch[i].id = event.touch.id;
            //Touch[i].button = FindButton(event.touch.x, event.touch.y);   //no buttons displayed.....

            break;
        }
	}
	al_fclose(credits);
	al_stop_sample_instance(wind_inst);
	al_destroy_sample_instance(wind_inst);
	//al_destroy_mixer(mixer);
	//al_destroy_voice(voice);

	FreeMenuBitmaps();
	al_fprintf(logfile,"Exit Title Screen\n");
	return 0;
}

ShipType AnyShip;

int DoMenu(ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_EVENT event)
{
	int i;
	int w,h;//,xoffset,yoffset;

	ALLEGRO_SAMPLE_INSTANCE *loop_inst;

	al_fprintf(logfile,"\nStart Menu\n");

	al_fprintf(logfile,"Init Keyboard\n");
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

        Ship[i].image = i;
        Ship[i].colour = ShipColour[i];
        Ship[i].statuscolour = StatusColour[i];
    }

	Menu.num_groups = read_maps();	//read 'maps.txt' and store strings
	//RETURN VALUE HERE
	//MAYBE NEED CURRENT GROUP / CURRENT MAP in struct somewhere???

    w = al_get_display_width(display);
    h = al_get_display_height(display);

    al_set_clipping_rectangle(0, 0, w, h);

	//Menu.map = 0;
	Menu.col = 0;
	Menu.offset = 0;

	menu_bg_bmp = al_load_bitmap("menu_bg.png");
    logo = al_load_bitmap("gs2.png");

    if ((ships = al_load_bitmap("ships.png")) == NULL)  al_fprintf(logfile,"ships.png load fail");
    if ((grey_ships = al_load_bitmap("grey_ships.png")) == NULL)  al_fprintf(logfile,"grey_ships.png load fail");

	al_fprintf(logfile,"\nSelected map %d,%d\n",Menu.group,Menu.map);

	if (init_map(Menu.group, Menu.map))		//opens 'map name.txt' and copies into MapTypeStruct.
	{								//everything else (load map, make col map, init ships etc. reads from MapType struct
		al_fprintf(logfile,"Failed to open map file\n");
		return 0;
	}
    //play background music
    loop_inst = al_create_sample_instance(loop);
    al_attach_sample_instance_to_mixer(loop_inst, mixer);
    al_set_sample_instance_playmode(loop_inst, ALLEGRO_PLAYMODE_LOOP);
    al_set_sample_instance_gain(loop_inst, 2);  //match music vol to fx vol
    al_play_sample_instance(loop_inst);

	//if (DoOldMenu(queue, event, AnyShip)) return 1;
	if (DoNewMenu(queue)) return 1;

	//stop music
	al_stop_sample_instance(loop_inst);
	al_destroy_sample_instance(loop_inst);

	al_fprintf(logfile,"\nPlaying map %s (%d,%d)\n",(char*)&MapNames[Menu.group].Map[Menu.map],Menu.group,Menu.map);
	al_fprintf(logfile,"Number of players %d\n",num_ships);

	/*
	for (i=0 ; i<30 ; )
	{
		al_wait_for_event(queue, &event);
		if (event.type == ALLEGRO_EVENT_TIMER)
		{
			i++;
			display_map_text(0,i);	//this is the description text file. i fades the colours in and out
		}
	}
    */
	return 0;
}

ALLEGRO_FILE *hostfile,*clientfile;

int total_lines;
float line_space;

int DoNewMenu(ALLEGRO_EVENT_QUEUE *queue)
{
	ALLEGRO_EVENT event;
	int i,j;
	int w,h;//,xoffset,yoffset;
    //int line_space = (int)(35*font_scale);

    Ship[0].angle = 0;
    Ship[1].angle = 10;
    Ship[2].angle = 20;
    Ship[3].angle = 30;

    event.keyboard.keycode = 0;
    Net.pingtimer = 0;
    Menu.expand = 0;

    int total_maps = 0;
    for (i=0 ; i< Menu.num_groups ; i++)
        total_maps += MapNames[i].Count;
    total_lines = total_maps + Menu.num_groups;

    for (i=0 ; i<Menu.num_groups ; i++ )
    {
        for(j=0 ; j<MapNames[i].Count ; j++)
        {
            MapNames[i].Map[j].players = get_map_players(i,j);
        }
    }

    line_space = 35*font_scale;

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

            LoadFonts(0);
			if (Menu.expand > 35*font_scale)
				Menu.expand = 35*font_scale;
        }
        if (event.type == ALLEGRO_EVENT_DISPLAY_HALT_DRAWING)   //we've been sidelined by the user/os
        {
            al_acknowledge_drawing_halt(display);   //acknowledge
            halted = true;                          //flag to drawing routines to do nothing
            al_stop_timer(timer);                   //no more timer events, so we should do nothing, saving battery
            al_destroy_voice(voice);           //destroy voice, so no more sound events, ditto
            //break;
        }
        if (event.type == ALLEGRO_EVENT_DISPLAY_RESUME_DRAWING) //we've been restored
        {
            al_acknowledge_drawing_resume(display); //ack
            halted = false;                         //remove flag
            al_start_timer(timer);                  //restart timer
            voice = al_create_voice(44100, ALLEGRO_AUDIO_DEPTH_INT16, ALLEGRO_CHANNEL_CONF_2);  //restart audio
            al_attach_mixer_to_voice(mixer, voice);
            //break;
        }

		else if (event.type == ALLEGRO_EVENT_TIMER && al_is_event_queue_empty(queue))
		{
			display_new_menu();

            UpdateTouches();

#ifdef ANDROID
            Menu.expand = 35*font_scale;
#else

			//expanding maps
			if (Menu.expand < 35*font_scale)
				Menu.expand += 5*font_scale;

            if (Menu.state == INSTRUCTIONS) {
                    Menu.y_origin += Menu.scroll;
                if (Menu.y_origin > 0)
                    Menu.y_origin = 0;
                if (Menu.y_origin < (Menu.max_scroll))
                    Menu.y_origin = Menu.max_scroll;
            }
#endif
            if (gpio_active) ReadGPIOJoystick();

            Net.pingtimer++;   //used by client searching for server
            Net.msgtimer++;

		}

		else if (event.type == ALLEGRO_EVENT_KEY_DOWN)
		{
            if (al_get_sample_instance_playing(clunk_inst))
                al_stop_sample_instance(clunk_inst);
            al_play_sample_instance(clunk_inst);

            //non-joystick things
            if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
            {
                Command.goback = true;
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
        else if (event.type == ALLEGRO_EVENT_KEY_UP)
        {
            pressed_keys[event.keyboard.keycode]=false;
            key_up_log[event.keyboard.keycode]=true;
        }

		//See if it's a USB joystick or touch event. If it is, log it.
 		else
        {
            CheckUSBJoyStick(event);
            if (al_is_touch_input_installed())
            {
                if (event.any.source == al_get_touch_input_event_source())
                {
                    CheckTouchControls(event);
                }
            }
        }

		//finished checking events

		//turn any pending keypresses / Joystick events into Ship controls
        ScanInputs(MAX_SHIPS);

		//Take any ship control to control menu. Also, cursor keys / return always work.
        AnyShip.fire1_held = false;
        AnyShip.fire2_held = false;
        for (i=0 ; i<num_ships ; i++)
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
            if (Ship[i].fire1_held)
            {
                AnyShip.fire1_held = true;
            }

            if (Ship[i].fire2_down || key_down_log[ALLEGRO_KEY_DOWN])
			{
				AnyShip.fire2_down = true;
				Ship[i].fire2_down = false;
				key_down_log[ALLEGRO_KEY_DOWN] = false;
			}
            if (Ship[i].fire2_held)
            {
                AnyShip.fire2_held = true;
            }

			if (Ship[i].thrust_down || key_down_log[ALLEGRO_KEY_ENTER])
			{
				//AnyShip.thrust_down = true;
				Command.goforward = true;
				Ship[i].thrust_down = false;
				key_down_log[ALLEGRO_KEY_ENTER] = false;
			}
		}
		//now have all inputs recorded as AnyShip elements (non-ship keypresses will be in key_down_log / Command struct)
		//actions are dependent on state....

        if (Select.action == RELEASE) {
            w = al_get_display_width(display);
            //if (Select.x < 0.75*w)
                Select.line = (int)((Select.y - 2*Select.sumdy)/(line_space * 2)-0.0);
            //else
            //    Select.action = NO_ACTION;
        }

        switch (Menu.state)
        {
            case NETWORK:
                if (Command.goback)
                {
                    Command.goback = false;
                    Exit();   //escape to exit
                    break;
                }
                if (Command.goforward)
                {
                    Command.goforward = false;
                    if (Menu.netmode == LOCAL)
                    {
                        Menu.group = 0;
                        Menu.map = 0;
                        GotoLevel();
                    }
                    else if (Menu.netmode == HOST)
                    {
                        Menu.group = 1;
                        Menu.map = 0;
                        GotoLevel();
                    }
                    else if (Menu.netmode == CLIENT)
                    {
                        GotoPlayers();
                    }
                    else if (Menu.netmode == INST)
                    {
                        Menu.state = INSTRUCTIONS;               //go to next menu
                        if ((ui = al_load_bitmap("ui.png")) == NULL)  al_fprintf(logfile,"ui.png load fail");
                        make_instructions_bitmap();
                        Ctrl.ctrl[SELECT].active=FALSE;
                        Select.sumdymin = Menu.max_scroll;//-1*total_lines*line_space;
                        Select.sumdy = Select.sumdymax;
                    }
                }

                else if (AnyShip.fire2_down)    //down
                {
                    AnyShip.fire2_down = false;
                    //if (Menu.col_pos < 2/*Menu.max_col_pos*/)
                    if (Menu.netmode < INST)
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
                else if (Select.action == RELEASE)
                {
                    Select.action = NO_ACTION;
                    //if (Select.x < 0.75*w)
                        Menu.netmode = Select.line-1;
                    if (Menu.netmode == INST)
                        Command.goforward = TRUE;
                }
            break;
            case INSTRUCTIONS:
                if (Command.goback || Command.goforward)
                    GotoNetwork();
                if (AnyShip.fire2_held)    //down
                {
                    Menu.scroll = -10*font_scale;
                }
                else if (AnyShip.fire1_held)    //up
                {
                    Menu.scroll = 10*font_scale;
                }
                else if (Select.action == RELEASE) {
                    Select.action = NO_ACTION;
                }
                else Menu.scroll = 0;
            break;
            case LEVEL: //map selection
                if (Command.goback)
                {
                    GotoNetwork();
                }
                if (Command.goforward)
                {
                    GotoPlayers();
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

                    al_fprintf(logfile,"\nSelected map %d,%d\n",Menu.group,Menu.map);
                    //init_map(Menu.group, Menu.map);
                    get_map_players( Menu.group, Menu.map);
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

                    al_fprintf(logfile,"\nSelected map %d,%d\n",Menu.group,Menu.map);
                    //init_map(Menu.group, Menu.map);
                    get_map_players( Menu.group, Menu.map);

                }
                else if (Select.action == RELEASE)
                {
                    Select.action = NO_ACTION;
                    if (Select.x < 0.75*w) {
                        for (i = 0; i < Menu.num_groups; i++) {
                            if (Select.line == 0) break;
                            Select.line--;                          //group name

                            for (j = 0; j < MapNames[i].Count; j++) {
                                // MapNames[i].Map[j].players = get_map_players(i,j);
                                Select.line--;
                                if (Select.line == 0) {
                                    Menu.group = i;
                                    Menu.map = j;
                                    get_map_players( Menu.group, Menu.map);
                                    break;
                                }
                            }
                        }
                    }
                }
            break;

            case PLAYERS: //players/ship/controls
                if (Command.goback)
                {
                    Command.goback = false;
                    if (Menu.netmode == CLIENT)
                    {
                        GotoNetwork();
                        if (Net.client_state == CONNECTED || Net.client_state == RUNNING)
                        {
                            NetDisconnectClient();
                            while (Net.client_state != IDLE)
                            {
                                ServiceNetwork();
                            }
                        }
                        NetStopClient();
                    }
                    else if (Menu.netmode == HOST)
                    {
                        GotoLevel();
                        //disconnect clients
                        NetStopListen();    //stop new connections
                        NetSendAbort();         //tell clients to disconnect
                        while (num_ships > 1)   //wait until they do
                            ServiceNetwork();

                        NetStopServer();        //kill server
                        Net.server = false;
                    }
                    else
                        GotoLevel();
                    break;
                }

                if (Net.client)
                {
                    if (Net.client_state == IDLE)
                    {
                        NetStopClient();
                        Net.client = true;
                        Net.client_state = SEARCHING;       //start looking again
                    }
                }

                if (Command.goforward)
                {
                    Command.goforward = false;    //thrust to start game;

                    //if (Map.max_players == 1 || Menu.netmode != LOCAL)  //skip AI screen if 1 player map, or network game.
                    if (Map.max_players == num_ships || Menu.netmode != LOCAL)  //skip AI screen if all players human, or network game.
                    {
                        init_map(Menu.group, Menu.map);

                        if (Menu.netmode == CLIENT)
                        {
                            if (Net.client_state == CONNECTED)
                                GotoNetwork();     //set state for when we return to menu
                            else
                                break;
                        }
                        else
                            GotoLevel();     //set state for when we return to menu

                        return 0;                       //return from function to start game
                    }
                    else
                    {
                        GotoAI();
                    }
                }
#ifndef ANDROID
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
#endif
                else if (AnyShip.left_down)    //left
                {
                    AnyShip.left_down = false;
                    if (Menu.item == 0)        //ship image
                    {
                        Ship[Menu.player].image--;
                        Ship[Menu.player].image&=0x07;
                        Ship[Menu.player].colour = ShipColour[Ship[Menu.player].image];
                        Ship[Menu.player].statuscolour = StatusColour[Ship[Menu.player].image];
                        Ship[Menu.player].offset = -60;
                    }
#ifndef ANDROID
                    else if (Menu.col_pos == 0)          //num of players
                    {
                        //if (!Net.net)
                            if (num_ships > 1)
                            {
                                num_ships--;
                                Menu.ships--;
                            }
                    }
                    else if (Menu.item == 1)        //controls
                    {
                        if (Ship[Menu.player].controller > 0)
                        Ship[Menu.player].controller--;
                        if (Ship[Menu.player].controller == GPIO_JOYSTICK)
                        {
                            if (!gpio_active)
                            {
                                Ship[Menu.player].controller--;
                            }
                        }
                    }
#endif
                }
                else if (AnyShip.right_down)    //right
                {
                    AnyShip.right_down = false;
                    if (Menu.item == 0) //ship
                    {
                        Ship[Menu.player].image++;
                        Ship[Menu.player].image&=0x07;
                        Ship[Menu.player].colour = ShipColour[Ship[Menu.player].image];
                        Ship[Menu.player].statuscolour = StatusColour[Ship[Menu.player].image];
                        Ship[Menu.player].offset = 60;
                    }
#ifndef ANDROID
                    else if (Menu.col_pos == 0)
                    {
                        //if (!Net.net)
                        {
                            if (num_ships < Map.max_players)
                            {
                                num_ships++;
                                Menu.ships++;
                            }
                        }
                    }
                    if (Menu.item == 1)    //controls
                    {
                        if (Ship[Menu.player].controller < 3)
                            Ship[Menu.player].controller++;
                        if (Ship[Menu.player].controller == GPIO_JOYSTICK)
                        {
                            if (!gpio_active)
                            {
                                Ship[Menu.player].controller++;
                            }
                        }
                    }
                    else if (Menu.item == 2)    //define keys
                    {
                        Menu.define_keys = true;    //done above
                    }
#endif
                }
#ifdef ANDROID
                //Ship[Menu.player].image = Select.sumdx/100;
                //Ship[Menu.player].offset = 60;
                if (Select.action == RELEASE) {
                    Select.action = NO_ACTION;
                    w = al_get_display_width(display);
                    Ship[Menu.player].image = 8*(0.0+(Select.x/w));
                    Ship[Menu.player].colour = ShipColour[Ship[Menu.player].image];
                }
#endif

                Menu.player = (Menu.col_pos-1) / 3;
                Menu.item   = (Menu.col_pos-1) % 3;
            break;
            case AI:
                if (Command.goback)
                {
                    GotoPlayers();
                    break;
                }
                if (Command.goforward)
                {
                    init_map(Menu.group, Menu.map);

                    if (Menu.netmode == CLIENT)
                    {
                        if (Net.client_state == CONNECTED)
                            GotoNetwork();     //set state for when we return to menu
                        else
                            break;
                    }
                    else
                        GotoLevel();     //set state for when we return to menu

                    return 0;                       //return from function to start game
                }
                if (AnyShip.fire2_down)
                {
                    AnyShip.fire2_down = false;
                    if (Menu.col_pos < 1) Menu.col_pos++;
                }
                if (AnyShip.fire1_down)
                {
                    AnyShip.fire1_down = false;
                    if (Menu.col_pos > 0) Menu.col_pos--;
                }
                if (AnyShip.left_down)
                {
                    AnyShip.left_down = false;
                    if(Menu.col_pos == 0) //number of ships
                    {
                        if (Menu.ai_ships > 0)
                            Menu.ai_ships--;
                    }
                    else    //must be difficulty
                    {
                        if (Menu.difficulty > 0)
                            Menu.difficulty--;
                    }
                }
                if (AnyShip.right_down)
                {
                    AnyShip.right_down = false;
                    if(Menu.col_pos == 0) //number of ships
                    {
                        if (Menu.ai_ships < (Map.max_players-num_ships))
                            Menu.ai_ships++;
                    }
                    else    //must be difficulty
                    {
                        if (Menu.difficulty < 3)
                            Menu.difficulty++;
                    }
                }
#ifdef ANDROID
                if (Select.action == RELEASE) {
                    Select.action = NO_ACTION;
                    w = al_get_display_width(display);
                    h = al_get_display_height(display);
                    if (Select.y < h/2)
                        Menu.ai_ships = 0.5 + (Select.x - w/3) / (w/6);
                    else
                        Menu.difficulty = 0.5 + (Select.x - w/3) / (w/6);
                }
#endif

            break;
            default:
                GotoNetwork();
            break;
        }
        event.keyboard.keycode = 0;
        ServiceNetwork();
	}
}

void GotoNetwork(void)
{
	Command.goback = false;
	Command.goforward = false;

	Menu.state = NETWORK;            //back to previous menu
	Select.sumdymin = 0;//-4*line_space;
	Select.sumdy = Select.sumdymax;
	Ctrl.ctrl[SELECT].active=TRUE;
	AnyShip.fire1_down = false;     //stop scroll up on exit
}

void GotoLevel(void)
{
	int i,y=0;

	if (Command.goback)
    {
        if (Menu.netmode == HOST)
        {//disconnect clients
            if(Net.host)
            {
                NetStopListen();    //stop new connections
                NetSendAbort();         //tell clients to disconnect
                while (num_ships > 1)   //wait until they do
                    ServiceNetwork();

                NetStopServer();        //kill server
                Net.server = false;
            }
        }
    }

	Command.goback = false;
	Command.goforward = false;

	Menu.state = LEVEL;               		//go to next menu
	Select.sumdymin = -1*total_lines*line_space;    //set max scroll
	Select.sumdy = Select.sumdymax;                 //reset current scroll

	for (i=0 ; i<Menu.group ; i++)
    {
        y+= MapNames[i].Count;  //levels in each group
        y++;                    //one more for group name
    }
	Select.sumdy -= (y+Menu.map) * line_space;//add in current map within group and bump scroll to correct place
	Menu.col_pos = 0;

    init_map(Menu.group, Menu.map);
}

void GotoPlayers(void)
{
	Command.goback = false;
	Command.goforward = false;

	Menu.state = PLAYERS;

	Select.sumdymin = Select.sumdymax;
	Select.sumdy = Select.sumdymax;
	Select.sumdxmin = -1000;
	Select.sumdxmax = 1000;
	Select.sumdx = 0;

	AnyShip.left_down = false;
	AnyShip.right_down = false;

    if (Menu.netmode == HOST)
    {
        Net.server = true;
        num_ships = 1;
        hostfile = al_fopen("../host.txt","w");
        NetStartNetwork();
        get_map_players( Menu.group, Menu.map);
        NetStartHost(Map.max_players);

        Menu.ships = 1;
        Menu.col_pos = 1;
    }

    else if (Menu.netmode == CLIENT)	//from network
    {
        Menu.ships = 1;
        Menu.col_pos = 1;

        Net.server = false;
        Net.client = true;
        clientfile = al_fopen("../client.txt","w");
        NetStartNetwork();
        Net.client_state = SEARCHING;   //starts pinging
    }
    else //local
    {
#ifdef ANDROID
        num_ships = 1;
        Menu.col_pos = 1;
        Menu.player = 0;
        Menu.item = 0;
#else
        get_map_players( Menu.group, Menu.map);
        if (Map.max_players == 1) num_ships = 1;    //default 2 players, unless max is 1
        else num_ships = 2;
        Menu.col_pos = 0;
#endif
        Menu.ships = num_ships; //for local
    }
}

void GotoAI(void)
{
    Menu.state = AI;
    Menu.col_pos = 0;
}

