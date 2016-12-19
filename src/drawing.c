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

#include "allegro5/allegro.h"
#include "allegro5/allegro_image.h"
#include "allegro5/allegro_primitives.h"
#include "allegro5/allegro_font.h"
#include "allegro5/allegro_audio.h"

#include "game.h"
#include "drawing.h"
#include "objects.h"
#include "inputs.h"
#include "network.h"

#define MAX_MAP_WIDTH 100
#define MAX_MAP_HEIGHT 100
int tile_map[MAX_MAP_WIDTH * MAX_MAP_HEIGHT];

int map_height=0, map_width=0;

void draw_background(int scrollx, int scrolly, int win_x, int win_y, int w, int h);
void draw_map(int scrollx, int scrolly, int x, int y, int w, int h);
void draw_ships(int scrollx, int scrolly, int x, int y, int w, int h);
void draw_menu(int ship_num, int x, int y, int w, int h);

//#define LINE_SPACE 55
#define LINE_SPACE 35
#define SMALL_LINE_SPACE 35

int grid = 0;

#if 0
void display_menu(void)//int num_maps, int selected)	//show list of maps
{
	ALLEGRO_TRANSFORM transform;
	int i,j,y=-10;
	int w,h,xoffset,yoffset;
	int glow;
	float scale;
	char keys[]  = "Keys";
	char gpio[]  = "GPIO Joy";
	char usb0[]  = "USB Joy 1";
	char usb1[]  = "USB Joy 2";
	char na[]    = "N/A";
	char* control_string;
	//static int temp = 0,temp2=0;

	ALLEGRO_COLOR colour;
	//use pre-multiplied alpha, i.e. rgb components must be multiplied by a.
    ALLEGRO_COLOR GroupActive    = al_map_rgba_f(0, 1, 0.5, 1);
    ALLEGRO_COLOR GroupGlow      = al_map_rgba_f(0, 0.188, 0.0, 0.078);
    ALLEGRO_COLOR GroupInactive  = al_map_rgba_f(0, 0.3, 0, 0.078);
    ALLEGRO_COLOR ItemCurrent    = al_map_rgba_f(1, 1, 0.2, 1);
    ALLEGRO_COLOR ItemCurrentGlow= al_map_rgba_f(0.2, 0.2, 0.04, 0.078);
    ALLEGRO_COLOR ItemSelected   = al_map_rgba_f(0.5, 0, 0, 1);
    ALLEGRO_COLOR ItemUnselected = al_map_rgba_f(1, 1, 0.8, 1);
    ALLEGRO_COLOR ItemExcluded   = al_map_rgba_f(0.25, 0.25, 0.25, 1);

	w = al_get_display_width(display);
    h = al_get_display_height(display);

	al_set_clipping_rectangle(0, 0, w, h);

	yoffset = (h-SCREENY)/2;
	if (yoffset < 0) yoffset = 0;

	xoffset = (w-SCREENX)/2;
    if (xoffset < 0) xoffset = 0;

    //Menu.offset += xoffset;

	al_clear_to_color(al_map_rgb(0, 0, 0));

	al_draw_bitmap(menu_bg_bmp,xoffset,yoffset,0);

    //Gravstorm logo
    //shadow
	scale = 0.4;
	al_identity_transform(&transform);			/* Initialize transformation. */
	al_scale_transform(&transform, scale, scale);	/* Rotate and scale around the center first. */
	al_translate_transform(&transform,SCREENX/2,y+yoffset);
	al_use_transform(&transform);
	al_draw_textf(title_font, al_map_rgba(0, 0, 0,64),0, (al_get_font_ascent(title_font)/2)*scale,  ALLEGRO_ALIGN_CENTRE, "%s", NAME);
    //image
	al_identity_transform(&transform);
	al_scale_transform(&transform, scale, scale);	/* Rotate and scale around the center first. */
	al_translate_transform(&transform,SCREENX/2-7,y+yoffset+7);
	al_use_transform(&transform);
	//al_draw_textf(title_font, al_map_rgb(128, 128, 0),0, (al_get_font_ascent(title_font)/2)*scale,  ALLEGRO_ALIGN_CENTRE, "%s", NAME);
	al_draw_bitmap(logo,-SCREENX/2+17,47,0);

	//reset transform
	al_identity_transform(&transform);
	al_use_transform(&transform);
	y+= 25;
    y+= yoffset;


	//Display maps; display all group names, and maps in current group
    for (i=0 ; i<Menu.num_groups ; i++)
    {
        if (i == Menu.group)
        {
            //colour = GroupActive;
            al_draw_textf(menu_font, GroupActive ,Menu.offset+20, y+=LINE_SPACE,  ALLEGRO_ALIGN_LEFT, "%s", (char*)&MapNames[i].Group);

            //colour = GroupInactive;
            al_draw_textf(glow_font, GroupGlow ,Menu.offset+20, y/*+=LINE_SPACE*/,  ALLEGRO_ALIGN_LEFT, "%s", (char*)&MapNames[i].Group);

            for (j=0 ; j<MapNames[Menu.group].Count ; j++)
            {
                glow = false;
                if (j == Menu.map)
                {
                    if (Menu.col == 0)
                    {
                        colour = ItemCurrent;   //so yellow if we're in col 0, i.e. changing this
                        glow = true;
                    }
                    else
                        colour = ItemSelected;     //red if another col, so you can see it's the current one
                }
                else
                    colour = ItemUnselected;

                al_draw_textf(menu_font, colour ,Menu.offset+30, y+=Menu.expand,  ALLEGRO_ALIGN_LEFT, "%s", (char*)&MapNames[i].Map[j]);
                if (glow) al_draw_textf(glow_font, ItemCurrentGlow ,Menu.offset+30, y,  ALLEGRO_ALIGN_LEFT, "%s", (char*)&MapNames[i].Map[j]);
            }
        }
        else
        {
            colour = GroupInactive;
            al_draw_textf(menu_font, colour ,Menu.offset+20, y+=LINE_SPACE,  ALLEGRO_ALIGN_LEFT, "%s", (char*)&MapNames[i].Group);
        }
    }


    y=80;
    y+= yoffset;

    for (i=0 ; i<4 ; i++)			//List players
    {
        glow = false;
        if (i >= Map.max_players)                   //can never select this, so grey
            colour = ItemExcluded;

        else if (i == Menu.player)                  //selected player
        {
            if(Menu.col == 1)
            {
                colour = ItemCurrent;   //so yellow if we're in col 1, i.e. changing this
                glow = true;
            }
            else
                colour = ItemSelected;     //red if another col, so you can see it's the current one
        }
        else
            colour = ItemUnselected;

        al_draw_textf(menu_font, colour,Menu.offset+470, y+i*LINE_SPACE,  ALLEGRO_ALIGN_LEFT, "Player %d",i+1);
        if (glow) al_draw_textf(glow_font, ItemCurrentGlow,Menu.offset+470, y+i*LINE_SPACE,  ALLEGRO_ALIGN_LEFT, "Player %d",i+1);
    }


    for (i=0 ; i<4 ; i++)			//List controls
    {
        if (Ship[i].controller == KEYS)
            control_string = keys;
        else if (Ship[i].controller == GPIO_JOYSTICK)
            control_string = gpio;
        else if (Ship[i].controller == USB_JOYSTICK0)
            control_string = usb0;
        else if (Ship[i].controller == USB_JOYSTICK1)
            control_string = usb1;
        else//if (Ship[i].controller == NA)
            control_string = na;

        glow = false;
        //if (i >= Map.max_players)
        if (i != Menu.player)
            colour = ItemExcluded;
        else if (i == Menu.player)
        {
            if (Menu.col == 2)
            {
                colour = ItemCurrent;
                glow = true;
            }
            else
                colour = ItemSelected;
        }
        else
            colour = ItemUnselected;

        al_draw_textf(menu_font, colour,Menu.offset+940, y+i*LINE_SPACE,  ALLEGRO_ALIGN_LEFT, "%s",control_string);		//Control method for selected player
        if (glow)al_draw_textf(glow_font, ItemCurrentGlow,Menu.offset+940, y+i*LINE_SPACE ,  ALLEGRO_ALIGN_LEFT, "%s",control_string);		//Control method for selected player
    }
    colour = ItemUnselected;
    al_draw_textf(small_font, colour, Menu.offset+940, y+(i+1)*LINE_SPACE,  ALLEGRO_ALIGN_LEFT, "Max Players: %d",Map.max_players);
    al_draw_textf(small_font, colour, Menu.offset+940, y+(i+2)*LINE_SPACE,  ALLEGRO_ALIGN_LEFT, "Active Players: %d",num_ships);

    if (Ship[Menu.player].controller == KEYS)
    {
        al_draw_textf(small_font, ItemUnselected ,Menu.offset+1400, y+SMALL_LINE_SPACE*0,  ALLEGRO_ALIGN_LEFT, "Rotate Left :");
        if (Menu.col == 3 && Menu.current_key == 0){colour = ItemCurrent; glow = true;}
        else {colour = ItemSelected; glow = false;}
        al_draw_textf(small_font, colour,Menu.offset+1400, y+SMALL_LINE_SPACE*1,  ALLEGRO_ALIGN_LEFT, " %s",al_keycode_to_name(Ship[Menu.player].left_key));
        if (glow) al_draw_textf(small_glow_font, ItemCurrentGlow,Menu.offset+1400, y+SMALL_LINE_SPACE*1,  ALLEGRO_ALIGN_LEFT, " %s",al_keycode_to_name(Ship[Menu.player].left_key));

        al_draw_textf(small_font, ItemUnselected, Menu.offset+1400, y+SMALL_LINE_SPACE*2,  ALLEGRO_ALIGN_LEFT, "Rotate Right :");
        if (Menu.col == 3 && Menu.current_key == 1){colour = ItemCurrent; glow = true;}
        else {colour = ItemSelected; glow = false;}
        al_draw_textf(small_font, colour,Menu.offset+1400, y+SMALL_LINE_SPACE*3,  ALLEGRO_ALIGN_LEFT, " %s",al_keycode_to_name(Ship[Menu.player].right_key));
        if (glow) al_draw_textf(small_glow_font, ItemCurrentGlow,Menu.offset+1400, y+SMALL_LINE_SPACE*3,  ALLEGRO_ALIGN_LEFT, " %s",al_keycode_to_name(Ship[Menu.player].right_key));

        al_draw_textf(small_font, ItemUnselected,Menu.offset+1400, y+SMALL_LINE_SPACE*4,  ALLEGRO_ALIGN_LEFT, "Fire1 :");
        if (Menu.col == 3 && Menu.current_key == 2){colour = ItemCurrent; glow = true;}
        else {colour = ItemSelected; glow = false;}
        al_draw_textf(small_font, colour,Menu.offset+1400, y+SMALL_LINE_SPACE*5,  ALLEGRO_ALIGN_LEFT, " %s",al_keycode_to_name(Ship[Menu.player].up_key));
        if (glow)al_draw_textf(small_glow_font, ItemCurrentGlow ,Menu.offset+1400, y+SMALL_LINE_SPACE*5,  ALLEGRO_ALIGN_LEFT, " %s",al_keycode_to_name(Ship[Menu.player].up_key));

        al_draw_textf(small_font, ItemUnselected,Menu.offset+1400, y+SMALL_LINE_SPACE*6,  ALLEGRO_ALIGN_LEFT, "Fire2 :");
        if (Menu.col == 3 && Menu.current_key == 3){colour = ItemCurrent; glow = true;}
        else {colour = ItemSelected; glow = false;}
        al_draw_textf(small_font, colour,Menu.offset+1400, y+SMALL_LINE_SPACE*7,  ALLEGRO_ALIGN_LEFT, " %s",al_keycode_to_name(Ship[Menu.player].down_key));
        if (glow) al_draw_textf(small_glow_font, ItemCurrentGlow,Menu.offset+1400, y+SMALL_LINE_SPACE*7,  ALLEGRO_ALIGN_LEFT, " %s",al_keycode_to_name(Ship[Menu.player].down_key));

        al_draw_textf(small_font, ItemUnselected,Menu.offset+1400, y+SMALL_LINE_SPACE*8,  ALLEGRO_ALIGN_LEFT, "Thrust :");
        if (Menu.col == 3 && Menu.current_key == 4){colour = ItemCurrent; glow = true;}
        else {colour = ItemSelected; glow = false;}
        al_draw_textf(small_font, colour,Menu.offset+1400, y+SMALL_LINE_SPACE*9,  ALLEGRO_ALIGN_LEFT, " %s",al_keycode_to_name(Ship[Menu.player].thrust_key));
        if (glow) al_draw_textf(small_glow_font, ItemCurrentGlow,Menu.offset+1400, y+SMALL_LINE_SPACE*9,  ALLEGRO_ALIGN_LEFT, " %s",al_keycode_to_name(Ship[Menu.player].thrust_key));
    }

	al_flip_display();

	return;
}
#endif

void display_new_menu(void)//int num_maps, int selected)	//show list of maps
{
	ALLEGRO_TRANSFORM transform;
	int i,j,y=-10;
	int w,h,xoffset,yoffset;
	int glow;
	float scale;
	char keys[]  = "Keys";
	char gpio[]  = "GPIO Joy";
	char usb0[]  = "USB Joy 1";
	char usb1[]  = "USB Joy 2";
	//char na[]    = "N/A";
	char* control_string;
	char local[]  = "Local Game";
	char host[]  = "Host Network Game";
	char client[]    = "Join Network Game";
	//char* netmodestr;
	//static int timer = 0;
	//char cursor;
	//char display_address[17];


	//static int temp = 0,temp2=0;

	ALLEGRO_COLOR colour;
	//use pre-multiplied alpha, i.e. rgb components must be multiplied by a.
    ALLEGRO_COLOR GroupActive    = al_map_rgba_f(0, 1, 0.5, 1);
    ALLEGRO_COLOR GroupGlow      = al_map_rgba_f(0, 0.188, 0.0, 0.078);
    ALLEGRO_COLOR GroupInactive  = al_map_rgba_f(0, 0.3, 0, 0.078);
    ALLEGRO_COLOR ItemCurrent    = al_map_rgba_f(1, 1, 0.2, 1);
    ALLEGRO_COLOR ItemCurrentGlow= al_map_rgba_f(0.2, 0.2, 0.04, 0.078);
    ALLEGRO_COLOR ItemSelected   = al_map_rgba_f(0.5, 0, 0, 1);
    ALLEGRO_COLOR ItemUnselected = al_map_rgba_f(1, 1, 0.8, 1);
    ALLEGRO_COLOR ItemExcluded   = al_map_rgba_f(0.25, 0.25, 0.25, 1);

	w = al_get_display_width(display);
    h = al_get_display_height(display);

	al_set_clipping_rectangle(0, 0, w, h);

	yoffset = (h-SCREENY)/2;
	if (yoffset < 0) yoffset = 0;

	xoffset = (w-SCREENX)/2;
    if (xoffset < 0) xoffset = 0;

    //Menu.offset += xoffset;

	al_clear_to_color(al_map_rgb(0, 0, 0));

	al_draw_bitmap(menu_bg_bmp,xoffset,yoffset,0);

    //Gravstorm logo
    //shadow
	scale = 0.4;
	al_identity_transform(&transform);			/* Initialize transformation. */
	al_scale_transform(&transform, scale, scale);	/* Rotate and scale around the center first. */
	al_translate_transform(&transform,SCREENX/2,y+yoffset);
	al_use_transform(&transform);
	al_draw_textf(title_font, al_map_rgba(0, 0, 0,64),0, (al_get_font_ascent(title_font)/2)*scale,  ALLEGRO_ALIGN_CENTRE, "%s", NAME);
    //image
	al_identity_transform(&transform);
	al_scale_transform(&transform, scale, scale);	/* Rotate and scale around the center first. */
	al_translate_transform(&transform,SCREENX/2-7,y+yoffset+7);
	al_use_transform(&transform);
	//al_draw_textf(title_font, al_map_rgb(128, 128, 0),0, (al_get_font_ascent(title_font)/2)*scale,  ALLEGRO_ALIGN_CENTRE, "%s", NAME);
	al_draw_bitmap(logo,-SCREENX/2+17,47,0);

	//reset transform
	al_identity_transform(&transform);
	al_use_transform(&transform);
	y+= 40;
    y+= yoffset;

    //timer++;            //blinking cursor
    //if (timer & 0x10)
    //    cursor = ' ';
    //else
    //    cursor = '_';

    if (Menu.state == NETWORK)
    {
        al_draw_textf(small_font, ItemUnselected,Menu.offset+20, y,  ALLEGRO_ALIGN_LEFT, "Mode");
        /*
             if (Menu.netmode == LOCAL)  netmodestr = local;
        else if (Menu.netmode == HOST)   netmodestr = host;
        else                             netmodestr = client;
        */
        y+=35;

        if (Menu.netmode == LOCAL)
        {
           al_draw_textf(small_font, ItemCurrent    ,Menu.offset+30, y,  ALLEGRO_ALIGN_LEFT, "%s",local);
           al_draw_textf(small_glow_font, ItemCurrentGlow,Menu.offset+30, y,  ALLEGRO_ALIGN_LEFT, "%s",local);
           al_draw_textf(small_font, ItemUnselected,Menu.offset+470, y,  ALLEGRO_ALIGN_LEFT, "Single player, or all players on one device.");
        }
        else
            al_draw_textf(small_font, ItemUnselected,Menu.offset+30, y,  ALLEGRO_ALIGN_LEFT, "%s",local);

        y+=35;

        if (Menu.netmode == HOST)
        {
           al_draw_textf(small_font, ItemCurrent    ,Menu.offset+30, y,  ALLEGRO_ALIGN_LEFT, "%s",host);
           al_draw_textf(small_glow_font, ItemCurrentGlow,Menu.offset+30, y,  ALLEGRO_ALIGN_LEFT, "%s",host);
           al_draw_textf(small_font, ItemUnselected,Menu.offset+470, y,  ALLEGRO_ALIGN_LEFT, "Host chooses level to play on.");
        }
        else
            al_draw_textf(small_font, ItemUnselected,Menu.offset+30, y,  ALLEGRO_ALIGN_LEFT, "%s",host);

        y+=35;

        if (Menu.netmode == CLIENT)
        {
           al_draw_textf(small_font, ItemCurrent    ,Menu.offset+30, y,  ALLEGRO_ALIGN_LEFT, "%s",client);
           al_draw_textf(small_glow_font, ItemCurrentGlow,Menu.offset+30, y,  ALLEGRO_ALIGN_LEFT, "%s",client);
           al_draw_textf(small_font, ItemUnselected,Menu.offset+470, y,  ALLEGRO_ALIGN_LEFT, "Join a game someone else is hosting.");
        }
        else
            al_draw_textf(small_font, ItemUnselected,Menu.offset+30, y,  ALLEGRO_ALIGN_LEFT, "%s",client);

        y+=135;
        if (Net.client_state == ABORTED)
            al_draw_textf(small_font, ItemUnselected, Menu.offset+SCREENX/2, y,  ALLEGRO_ALIGN_CENTRE, "Previous game aborted by host.");

        //else
        //    al_draw_textf(small_font, ItemUnselected,Menu.offset+30, y,  ALLEGRO_ALIGN_LEFT, "%s",netmodestr);
        /*
        y+=35;

        if (Menu.netmode == CLIENT)
            colour = ItemUnselected;
        else colour = ItemExcluded;

        al_draw_textf(small_font, colour,Menu.offset+30, y,  ALLEGRO_ALIGN_LEFT, "Address:");

        y+=35;

        strncpy(display_address,Net.temp_address,16);
        strncat(display_address,&cursor,1);

        if (Menu.col_pos == 1)
        {
           al_draw_textf(small_font, ItemCurrent    ,Menu.offset+30, y,  ALLEGRO_ALIGN_LEFT, "%s",display_address);
           //al_draw_textf(small_glow_font, ItemCurrentGlow,Menu.offset+30, y,  ALLEGRO_ALIGN_LEFT, "%s",Net.temp_address);
        }
        else
            al_draw_textf(small_font, colour,Menu.offset+30, y,  ALLEGRO_ALIGN_LEFT, "%s",Net.menuaddress);

        //al_draw_textf(small_font, colour,Menu.offset+30, y+35,  ALLEGRO_ALIGN_LEFT, "%08X",timer);
        */
    }
	else if (Menu.state == LEVEL)
    {
	//Display maps; display all group names, and maps in current group
        for (i=0 ; i<Menu.num_groups ; i++)
        {
            if (i == Menu.group)
            {
                //colour = GroupActive;
                al_draw_textf(small_font, GroupActive ,Menu.offset+20, y+=LINE_SPACE,  ALLEGRO_ALIGN_LEFT, "%s", (char*)&MapNames[i].Group);

                //colour = GroupInactive;
                al_draw_textf(small_glow_font, GroupGlow ,Menu.offset+20, y/*+=LINE_SPACE*/,  ALLEGRO_ALIGN_LEFT, "%s", (char*)&MapNames[i].Group);

                for (j=0 ; j<MapNames[Menu.group].Count ; j++)
                {
                    glow = false;
                    if (j == Menu.map)
                    {
                        if (Menu.col == 0)
                        {
                            colour = ItemCurrent;   //so yellow if we're in col 0, i.e. changing this
                            glow = true;
                        }
                        else
                            colour = ItemSelected;     //red if another col, so you can see it's the current one
                    }
                    else
                        colour = ItemUnselected;

                    al_draw_textf(small_font, colour ,Menu.offset+30, y+=Menu.expand,  ALLEGRO_ALIGN_LEFT, "%s", (char*)&MapNames[i].Map[j]);
                    if (glow)
                    {
                        al_draw_textf(small_glow_font, ItemCurrentGlow, Menu.offset+30,  y,  ALLEGRO_ALIGN_LEFT, "%s", (char*)&MapNames[i].Map[j]);
                        al_draw_textf(small_font, ItemUnselected,  Menu.offset+470, y,  ALLEGRO_ALIGN_LEFT, "Max players:%d", Map.max_players);
                    }
                }
            }
            else
            {
                colour = GroupInactive;
                al_draw_textf(small_font, colour ,Menu.offset+20, y+=LINE_SPACE,  ALLEGRO_ALIGN_LEFT, "%s", (char*)&MapNames[i].Group);
            }
        }
        if (Menu.netmode == HOST)
        {
            al_draw_textf(small_font, ItemUnselected,  Menu.offset+SCREENX/2, SCREENY-85,  ALLEGRO_ALIGN_CENTRE, "Select level to start network game.");
            //al_draw_textf(small_font, ItemUnselected,  Menu.offset+SCREENX/2, SCREENY-50,  ALLEGRO_ALIGN_CENTRE, "%d players connected", Net.clients);
        }
 /*
        else if (Menu.netmode == CLIENT)
        {
            if (Net.connected)
                al_draw_textf(small_font, ItemUnselected,  Menu.offset+SCREENX/2, SCREENY-50,  ALLEGRO_ALIGN_CENTRE, "Connected to host at %s ", Net.menuaddress);
            else
                al_draw_textf(small_font, ItemUnselected,   Menu.offset+SCREENX/2, SCREENY-50,  ALLEGRO_ALIGN_CENTRE, "Searching for host at %s", Net.menuaddress);
        }
*/
    }

    else if (Menu.state == PLAYERS)
    {
        y=40;
        y+= yoffset;

        //if (!Net.net || Net.server)
        if (!Net.client)
        {
            al_draw_textf(small_font, ItemUnselected,Menu.offset+20, y,  ALLEGRO_ALIGN_LEFT, "Players:");

            if (Menu.col_pos == 0)
            {
               al_draw_textf(small_font, ItemCurrent    ,Menu.offset+180, y,  ALLEGRO_ALIGN_LEFT, "%d",num_ships);
               al_draw_textf(small_glow_font, ItemCurrentGlow,Menu.offset+180, y,  ALLEGRO_ALIGN_LEFT, "%d",num_ships);
            }
            else
            {
                //if (!Net.client)
                //if (!Net.net)
                    al_draw_textf(small_font, ItemUnselected,Menu.offset+180, y,  ALLEGRO_ALIGN_LEFT, "%d",num_ships);
                //else if (Net.server)
                //    al_draw_textf(small_font, ItemUnselected,Menu.offset+180, y,  ALLEGRO_ALIGN_LEFT, "%d",Net.clients);
            }
        }

        y+=35;

        //debug
        //al_draw_textf(small_font, ItemUnselected,Menu.offset+300, y,  ALLEGRO_ALIGN_LEFT, "player:%d item:%d",Menu.player,Menu.item);
        //end debug

        for (i=0 ; i<Menu.ships ; i++)			//List players
        {
            if (i>num_ships-1) colour = ItemExcluded;
            else colour = ItemUnselected;

            //if (!Net.net)
            if (!Net.client && !Net.server)
                al_draw_textf(small_font, colour,Menu.offset+20, y,  ALLEGRO_ALIGN_LEFT, "Player %d",i+1);

            y+=40;

            //draw spinny ship & highlight
            if (Menu.item == 0 && Menu.player == i)
                al_draw_filled_rounded_rectangle(Menu.offset+63,y,Menu.offset+115,y+52,10,10,al_map_rgba(64,64,0,64));

            y+=2;

            al_draw_bitmap_region(ships,Ship[i].angle*SHIP_SIZE_X,(2*Ship[i].image)*SHIP_SIZE_Y, SHIP_SIZE_X, SHIP_SIZE_Y,Menu.offset+65,y, 0);
            Ship[i].angle++;
            if (Ship[i].angle == 40) Ship[i].angle = 0;

            y+=45;

            //controls - if keys, show keys
            if (Ship[i].controller == KEYS)
                control_string = keys;
            else if (Ship[i].controller == GPIO_JOYSTICK)
                control_string = gpio;
            else if (Ship[i].controller == USB_JOYSTICK0)
                control_string = usb0;
            else /*if (Ship[i].controller == USB_JOYSTICK1)*/
                control_string = usb1;
            //else if (Ship[i].controller == NA)
            //    control_string = na;

            if (Menu.item == 1 && Menu.player == i)
            {
                al_draw_textf(small_glow_font, ItemCurrentGlow,Menu.offset+40, y,  ALLEGRO_ALIGN_LEFT, "%s",control_string);		//Control method for selected player
                al_draw_textf(small_font, ItemCurrent,Menu.offset+40, y,  ALLEGRO_ALIGN_LEFT, "%s",control_string);		//Control method for selected player
            }
            else
                al_draw_textf(small_font, colour,Menu.offset+40, y,  ALLEGRO_ALIGN_LEFT, "%s",control_string);		//Control method for selected player

            y+=35;

           if (Menu.item == 2 && Menu.player == i)
            {
                al_draw_textf(small_glow_font, ItemCurrentGlow,Menu.offset+40, y,  ALLEGRO_ALIGN_LEFT, "define keys ->");		//Control method for selected player
                al_draw_textf(small_font, ItemCurrent,Menu.offset+40, y,  ALLEGRO_ALIGN_LEFT, "define keys ->");		//Control method for selected player
            }
            else
            {
                if (Ship[i].controller != KEYS) colour = ItemExcluded;
                al_draw_textf(small_font, colour,Menu.offset+40, y,  ALLEGRO_ALIGN_LEFT, "define keys ->");		//Control method for selected player
            }

            if (Menu.item == 2 && Menu.player == i && Ship[i].controller == KEYS)
            {
                al_draw_textf(small_font, ItemUnselected ,Menu.offset+470, 80+SMALL_LINE_SPACE*0,  ALLEGRO_ALIGN_LEFT, "Rotate Left :");
                if (Menu.define_keys && Menu.current_key == 0){colour = ItemCurrent; glow = true;}
                else {colour = ItemSelected; glow = false;}
                al_draw_textf(small_font, colour,Menu.offset+470, 80+SMALL_LINE_SPACE*1,  ALLEGRO_ALIGN_LEFT, " %s",al_keycode_to_name(Ship[Menu.player].left_key));
                if (glow) al_draw_textf(small_glow_font, ItemCurrentGlow,Menu.offset+470, 80+SMALL_LINE_SPACE*1,  ALLEGRO_ALIGN_LEFT, " %s",al_keycode_to_name(Ship[Menu.player].left_key));

                al_draw_textf(small_font, ItemUnselected, Menu.offset+470, 80+SMALL_LINE_SPACE*2,  ALLEGRO_ALIGN_LEFT, "Rotate Right :");
                if (Menu.define_keys && Menu.current_key == 1){colour = ItemCurrent; glow = true;}
                else {colour = ItemSelected; glow = false;}
                al_draw_textf(small_font, colour,Menu.offset+470, 80+SMALL_LINE_SPACE*3,  ALLEGRO_ALIGN_LEFT, " %s",al_keycode_to_name(Ship[Menu.player].right_key));
                if (glow) al_draw_textf(small_glow_font, ItemCurrentGlow,Menu.offset+470, 80+SMALL_LINE_SPACE*3,  ALLEGRO_ALIGN_LEFT, " %s",al_keycode_to_name(Ship[Menu.player].right_key));

                al_draw_textf(small_font, ItemUnselected,Menu.offset+470, 80+SMALL_LINE_SPACE*4,  ALLEGRO_ALIGN_LEFT, "Fire1 :");
                if (Menu.define_keys && Menu.current_key == 2){colour = ItemCurrent; glow = true;}
                else {colour = ItemSelected; glow = false;}
                al_draw_textf(small_font, colour,Menu.offset+470, 80+SMALL_LINE_SPACE*5,  ALLEGRO_ALIGN_LEFT, " %s",al_keycode_to_name(Ship[Menu.player].up_key));
                if (glow)al_draw_textf(small_glow_font, ItemCurrentGlow ,Menu.offset+470, 80+SMALL_LINE_SPACE*5,  ALLEGRO_ALIGN_LEFT, " %s",al_keycode_to_name(Ship[Menu.player].up_key));

                al_draw_textf(small_font, ItemUnselected,Menu.offset+470, 80+SMALL_LINE_SPACE*6,  ALLEGRO_ALIGN_LEFT, "Fire2 :");
                if (Menu.define_keys && Menu.current_key == 3){colour = ItemCurrent; glow = true;}
                else {colour = ItemSelected; glow = false;}
                al_draw_textf(small_font, colour,Menu.offset+470, 80+SMALL_LINE_SPACE*7,  ALLEGRO_ALIGN_LEFT, " %s",al_keycode_to_name(Ship[Menu.player].down_key));
                if (glow) al_draw_textf(small_glow_font, ItemCurrentGlow,Menu.offset+470, 80+SMALL_LINE_SPACE*7,  ALLEGRO_ALIGN_LEFT, " %s",al_keycode_to_name(Ship[Menu.player].down_key));

                al_draw_textf(small_font, ItemUnselected,Menu.offset+470, 80+SMALL_LINE_SPACE*8,  ALLEGRO_ALIGN_LEFT, "Thrust :");
                if (Menu.define_keys && Menu.current_key == 4){colour = ItemCurrent; glow = true;}
                else {colour = ItemSelected; glow = false;}
                al_draw_textf(small_font, colour,Menu.offset+470, 80+SMALL_LINE_SPACE*9,  ALLEGRO_ALIGN_LEFT, " %s",al_keycode_to_name(Ship[Menu.player].thrust_key));
                if (glow) al_draw_textf(small_glow_font, ItemCurrentGlow,Menu.offset+470, 80+SMALL_LINE_SPACE*9,  ALLEGRO_ALIGN_LEFT, " %s",al_keycode_to_name(Ship[Menu.player].thrust_key));
            }

            y+=35;
        }
        if (Menu.netmode == HOST)
        {
            if (Net.address.host == 0)
                al_draw_textf(small_font, ItemUnselected,  Menu.offset+SCREENX/2, SCREENY-85,  ALLEGRO_ALIGN_CENTRE, "Network host started");
            else
                al_draw_textf(small_font, ItemUnselected,  Menu.offset+SCREENX/2, SCREENY-85,  ALLEGRO_ALIGN_CENTRE, "Network host started on %s", Net.myaddress);
            al_draw_textf(small_font, ItemUnselected,  Menu.offset+SCREENX/2, SCREENY-50,  ALLEGRO_ALIGN_CENTRE, "%d players", num_ships);//Net.clients);
        }

        else if (Menu.netmode == CLIENT)
        {
            if (Net.client_state == SEARCHING)
                al_draw_textf(small_font, ItemUnselected,   Menu.offset+SCREENX/2, SCREENY-85,  ALLEGRO_ALIGN_CENTRE, "Searching for host...");
            else if (Net.client_state == FOUND)
                al_draw_textf(small_font, ItemUnselected,   Menu.offset+SCREENX/2, SCREENY-85,  ALLEGRO_ALIGN_CENTRE, "Found host at %s", Net.menuaddress);

            else if (Net.client_state == CONNECTED)
            {
                al_draw_textf(small_font, ItemUnselected,  Menu.offset+SCREENX/2, SCREENY-85,  ALLEGRO_ALIGN_CENTRE, "Connected to host at %s", Net.menuaddress);
                al_draw_textf(small_font, ItemUnselected,  Menu.offset+SCREENX/2, SCREENY-50,  ALLEGRO_ALIGN_CENTRE, "Host selected level '%s'", Net.mapfile);
            }

            else if (Net.client_state == NO_SPACE)
                al_draw_textf(small_font, ItemUnselected,   Menu.offset+SCREENX/2, SCREENY-85,  ALLEGRO_ALIGN_CENTRE, "Host at %s has no space", Net.menuaddress);

            else if (Net.client_state == NO_MAP)
                al_draw_textf(small_font, ItemUnselected,  Menu.offset+SCREENX/2, SCREENY-50,  ALLEGRO_ALIGN_CENTRE, "Failed to open host selected level '%s'", Net.mapfile);
        }

    }
/*
        for (i=0 ; i<4 ; i++)			//List controls
        {
            if (Ship[i].controller == KEYS)
                control_string = keys;
            else if (Ship[i].controller == GPIO_JOYSTICK)
                control_string = gpio;
            else if (Ship[i].controller == USB_JOYSTICK0)
                control_string = usb0;
            else if (Ship[i].controller == USB_JOYSTICK1)
                control_string = usb1;
            else//if (Ship[i].controller == NA)
                control_string = na;

            glow = false;
            //if (i >= Map.max_players)
            if (i != Menu.player)
                colour = ItemExcluded;
            else if (i == Menu.player)
            {
                if (Menu.col == 2)
                {
                    colour = ItemCurrent;
                    glow = true;
                }
                else
                    colour = ItemSelected;
            }
            else
                colour = ItemUnselected;

            al_draw_textf(menu_font, colour,Menu.offset+940, y+i*LINE_SPACE,  ALLEGRO_ALIGN_LEFT, "%s",control_string);		//Control method for selected player
            if (glow)al_draw_textf(glow_font, ItemCurrentGlow,Menu.offset+940, y+i*LINE_SPACE ,  ALLEGRO_ALIGN_LEFT, "%s",control_string);		//Control method for selected player
        }
        colour = ItemUnselected;
        al_draw_textf(small_font, colour, Menu.offset+940, y+(i+1)*LINE_SPACE,  ALLEGRO_ALIGN_LEFT, "Max Players: %d",Map.max_players);
        al_draw_textf(small_font, colour, Menu.offset+940, y+(i+2)*LINE_SPACE,  ALLEGRO_ALIGN_LEFT, "Active Players: %d",num_ships);

        if (Ship[Menu.player].controller == KEYS)
        {
            al_draw_textf(small_font, ItemUnselected ,Menu.offset+1400, y+SMALL_LINE_SPACE*0,  ALLEGRO_ALIGN_LEFT, "Rotate Left :");
            if (Menu.col == 3 && Menu.current_key == 0){colour = ItemCurrent; glow = true;}
            else {colour = ItemSelected; glow = false;}
            al_draw_textf(small_font, colour,Menu.offset+1400, y+SMALL_LINE_SPACE*1,  ALLEGRO_ALIGN_LEFT, " %s",al_keycode_to_name(Ship[Menu.player].left_key));
            if (glow) al_draw_textf(small_glow_font, ItemCurrentGlow,Menu.offset+1400, y+SMALL_LINE_SPACE*1,  ALLEGRO_ALIGN_LEFT, " %s",al_keycode_to_name(Ship[Menu.player].left_key));

            al_draw_textf(small_font, ItemUnselected, Menu.offset+1400, y+SMALL_LINE_SPACE*2,  ALLEGRO_ALIGN_LEFT, "Rotate Right :");
            if (Menu.col == 3 && Menu.current_key == 1){colour = ItemCurrent; glow = true;}
            else {colour = ItemSelected; glow = false;}
            al_draw_textf(small_font, colour,Menu.offset+1400, y+SMALL_LINE_SPACE*3,  ALLEGRO_ALIGN_LEFT, " %s",al_keycode_to_name(Ship[Menu.player].right_key));
            if (glow) al_draw_textf(small_glow_font, ItemCurrentGlow,Menu.offset+1400, y+SMALL_LINE_SPACE*3,  ALLEGRO_ALIGN_LEFT, " %s",al_keycode_to_name(Ship[Menu.player].right_key));

            al_draw_textf(small_font, ItemUnselected,Menu.offset+1400, y+SMALL_LINE_SPACE*4,  ALLEGRO_ALIGN_LEFT, "Fire1 :");
            if (Menu.col == 3 && Menu.current_key == 2){colour = ItemCurrent; glow = true;}
            else {colour = ItemSelected; glow = false;}
            al_draw_textf(small_font, colour,Menu.offset+1400, y+SMALL_LINE_SPACE*5,  ALLEGRO_ALIGN_LEFT, " %s",al_keycode_to_name(Ship[Menu.player].up_key));
            if (glow)al_draw_textf(small_glow_font, ItemCurrentGlow ,Menu.offset+1400, y+SMALL_LINE_SPACE*5,  ALLEGRO_ALIGN_LEFT, " %s",al_keycode_to_name(Ship[Menu.player].up_key));

            al_draw_textf(small_font, ItemUnselected,Menu.offset+1400, y+SMALL_LINE_SPACE*6,  ALLEGRO_ALIGN_LEFT, "Fire2 :");
            if (Menu.col == 3 && Menu.current_key == 3){colour = ItemCurrent; glow = true;}
            else {colour = ItemSelected; glow = false;}
            al_draw_textf(small_font, colour,Menu.offset+1400, y+SMALL_LINE_SPACE*7,  ALLEGRO_ALIGN_LEFT, " %s",al_keycode_to_name(Ship[Menu.player].down_key));
            if (glow) al_draw_textf(small_glow_font, ItemCurrentGlow,Menu.offset+1400, y+SMALL_LINE_SPACE*7,  ALLEGRO_ALIGN_LEFT, " %s",al_keycode_to_name(Ship[Menu.player].down_key));

            al_draw_textf(small_font, ItemUnselected,Menu.offset+1400, y+SMALL_LINE_SPACE*8,  ALLEGRO_ALIGN_LEFT, "Thrust :");
            if (Menu.col == 3 && Menu.current_key == 4){colour = ItemCurrent; glow = true;}
            else {colour = ItemSelected; glow = false;}
            al_draw_textf(small_font, colour,Menu.offset+1400, y+SMALL_LINE_SPACE*9,  ALLEGRO_ALIGN_LEFT, " %s",al_keycode_to_name(Ship[Menu.player].thrust_key));
            if (glow) al_draw_textf(small_glow_font, ItemCurrentGlow,Menu.offset+1400, y+SMALL_LINE_SPACE*9,  ALLEGRO_ALIGN_LEFT, " %s",al_keycode_to_name(Ship[Menu.player].thrust_key));
        }
    }
    */
	al_flip_display();

	return;
}

void display_map_text(int done, int timer)
{
    FILE * description_file;
    char line[200];
    int i=20;
    ALLEGRO_USTR *ustr = NULL;

    al_clear_to_color(al_map_rgb(0, 0, 0));

    al_draw_bitmap(menu_bg_bmp,0,0,0);

	al_draw_filled_rectangle(0,0,SCREENX,SCREENY,al_map_rgba(0,0,0,timer*6));	//black filter to darken?

	if ((description_file = fopen(Map.description_file_name,"r")))
	{
		while (fgets(line, 200, description_file) != NULL)
		{
			 ustr = al_ustr_new(line);

			//al_draw_textf(small_font, al_map_rgb(timer*8, timer*8, timer*8),0, i,  ALLEGRO_ALIGN_LEFT, "%s",line);
			al_draw_ustr(small_font, al_map_rgb(timer*8, timer*8, timer*8),0, i,  ALLEGRO_ALIGN_LEFT, ustr);
			i+=35;
		}

		if (ustr) al_ustr_free(ustr);
		fclose(description_file); //close file
	}
	else
	{
		al_draw_textf(small_font, al_map_rgb(timer*8, timer*8, timer*8),0, i,  ALLEGRO_ALIGN_LEFT, "%s",(char*)&MapNames[Menu.group].Map[Menu.map]);
		i+=35;
		al_draw_textf(small_font, al_map_rgb(timer*8, timer*8, timer*8),0, i,  ALLEGRO_ALIGN_LEFT, "Couldn't open description file: %s",Map.description_file_name);
	}

    if (done)
        al_draw_textf(small_font, al_map_rgb(128, 0, 0),0, i+35,  ALLEGRO_ALIGN_LEFT, "Painna Nappa / Press Button");

    al_flip_display();

    return;
}

void display_wait_text(void)
{
    al_clear_to_color(al_map_rgb(0, 0, 0));
    al_draw_bitmap(menu_bg_bmp,0,0,0);

    al_draw_textf(small_font, al_map_rgb_f(1, 1, 0.8), SCREENX/2, SCREENY/2,  ALLEGRO_ALIGN_CENTRE, "Waiting for host...");

    al_flip_display();
}

void make_bullet_bitmap(void)
{
    fprintf(logfile,"Creating bullets bitmap\n");
    bullets_bmp = al_create_bitmap(10, 10);			//create a bitmap
	al_set_target_bitmap(bullets_bmp);					//set it as the default target for all al_draw_ operations
    al_draw_filled_circle(2, 2, 2, al_map_rgb(255, 255, 255));

    al_set_target_backbuffer(display);			//Put default target back
}

int component;
void draw_split_screen(ViewportType viewport, int ship_num)
{
    int i;
    int x, y;
    int w, h;
	int scrollx, scrolly;	//These are the centre of the 'viewport' - normally the ship position, except when you get near an edge.

	//replace with split screen window size.
    w = al_get_display_width(display);
    h = al_get_display_height(display);

    //use this here, also determined by passed in params
    switch (viewport)
    {
    	case FULL:
    		x=STATUS_BAR_WIDTH;
    		y=0;
    		w=w-STATUS_BAR_WIDTH ;
    		h=h;
    		break;
    	case TOP:
    		x=STATUS_BAR_WIDTH;
    		y=0;
    		w=w-STATUS_BAR_WIDTH ;
    		h=h/2;
    		break;
    	case BOTTOM:
    		x=STATUS_BAR_WIDTH;
    		y=h/2;
    		w=w-STATUS_BAR_WIDTH ;
    		h=h/2 ;
    		break;
    	case TOPLEFT:
    		x=STATUS_BAR_WIDTH;
    		y=0;
    		w=(w-STATUS_BAR_WIDTH)/2;
    		h=h/2;
    		break;
    	case TOPRIGHT:
    		x=STATUS_BAR_WIDTH + (w-STATUS_BAR_WIDTH)/2;
    		y=0;
    		w=(w-STATUS_BAR_WIDTH)/2;
    		h=h/2;
    		break;
    	case BOTTOMLEFT:
    		x=STATUS_BAR_WIDTH;
    		y=h/2;
    		w=(w-STATUS_BAR_WIDTH)/2;
    		h=h/2;
    		break;
    	case BOTTOMRIGHT:
    		x=STATUS_BAR_WIDTH + (w-STATUS_BAR_WIDTH)/2;
    		y=h/2;
    		w=(w-STATUS_BAR_WIDTH)/2;
    		h=h/2;
    		break;
        default:
    		x=STATUS_BAR_WIDTH;
    		y=0;
    		w=w-STATUS_BAR_WIDTH ;
    		h=h;
    		break;
	}
	al_set_clipping_rectangle(x, y, w, h);

	//stop scrolling when you get near to the edge.
	//scrollx is ship xpos, except saturated at half screen width
	if (mapx < (SCREENX-STATUS_BAR_WIDTH))	//for small maps....
		scrollx = 0.5*w;
	else if (Ship[ship_num].xpos < 0.5*w) 						//if the ship is <0.5 viewport width from edge, stop following it.
		scrollx = 0.5*w;
	else if (Ship[ship_num].xpos > mapx-0.5*w) //likewise from far edge
		scrollx = mapx-0.5*w;
	else
		scrollx = Ship[ship_num].xpos;

	if (Ship[ship_num].ypos < 0.5*h) 						//...and for y
		scrolly = 0.5*h;
	else if (Ship[ship_num].ypos > mapy-0.5*h)
		scrolly = mapy-0.5*h;
	else
		scrolly = Ship[ship_num].ypos;

	//clear to black
	al_clear_to_color(al_map_rgb(0, 0, 0));

    if (Ship[ship_num].menu)	//draw the menu instead of the normal screen
    {
		draw_menu(ship_num,x,y,w,h);
	}

	else if (Map.ship_first)
	{
		if (Map.background_file_name[0] != 0) draw_background(scrollx, scrolly,x,y,w,h);
		draw_ships(scrollx, scrolly,x,y,w,h);
		draw_map(scrollx, scrolly,x,y,w,h);
	}
	else
	{
		if (Map.background_file_name[0] != 0) draw_background(scrollx, scrolly,x,y,w,h);
		draw_map(scrollx, scrolly,x,y,w,h);
		draw_ships(scrollx, scrolly,x,y,w,h);
	}

	if (Radar.on)
    {
        //al_draw_bitmap(Radar.display,SCREENX-Radar.width,-1*((MAX_MAP_HEIGHT * RADAR_PPT)-Radar.height),0);   //tinted? alpha?
        //al_draw_bitmap(Radar.display,SCREENX-Radar.width,-320,0);   //tinted? alpha?
        al_draw_bitmap(Radar.display,SCREENX-Radar.width,0,0);   //tinted? alpha?
        //draw ships. flashing? coloured?
        for (i=0 ; i<num_ships ; i++)
        {
            al_draw_tinted_bitmap(bullets_bmp, Ship[i].colour,(SCREENX-Radar.width + Ship[i].xpos/16)-2, (Ship[i].ypos/16)-2,0);
        }
    }

	return;
}

float soverm;

void draw_background(int scrollx, int scrolly, int win_x, int win_y, int w, int h)
{
	ALLEGRO_TRANSFORM transform;
	int x,y;
	int min_x, min_y, max_x, max_y;
	ALLEGRO_COLOR fade;

	if (Map.background_fade)
    {
        soverm = (float)(scrolly-Map.bg_fade_thresh)/(float)mapy;

        if (soverm < 0) soverm = 0;

        fade = al_map_rgba_f(soverm,soverm,soverm,1.0);//(scrolly/mapy,scrolly/mapy,scrolly/mapy,scrolly/mapy);
    }
    else
        fade = al_map_rgba_f(1.0,1.0,1.0,1.0);

	scrollx >>= 1;
	scrolly >>= 1;

	//stop scrolling when you get near to the edge.
	//scrollx is ship xpos, except saturated at half screen width
	if (mapx < (SCREENX-STATUS_BAR_WIDTH))	//for small maps....
		scrollx = 0.5*w;
	else if (scrollx < 0.5*w) 						//if the ship is <0.5 viewport width from edge, stop following it.
		scrollx = 0.5*w;
	else if (scrollx > mapx-0.5*w) //likewise from far edge
		scrollx = mapx-0.5*w;
	else
		scrollx = scrollx;

	if (scrolly < 0.5*h) 						//...and for y
		scrolly = 0.5*h;
	else if (scrolly > mapy-0.5*h)
		scrolly = mapy-0.5*h;
	else
		scrolly = scrolly;

	if (Map.type==1)	//1 for tiled
	{
		al_identity_transform(&transform);  		            /* Initialize transformation. */
		al_translate_transform(&transform, -scrollx, -scrolly); /* Move to scroll position. */
		al_translate_transform(&transform, w>>1, h>>1);         /* Move scroll position to screen center. */
		al_use_transform(&transform);                           /* All subsequent drawing is transformed. */

		min_x = (scrollx - (w>>1) ) >> (TILE_SHIFTS+1);// /(tile_width);	//optimise by using shifts, rather than divides....
		min_y = (scrolly - (h>>1) ) >> (TILE_SHIFTS+1);// /(tile_height);

		max_x = ((scrollx + (w>>1) ) >> (TILE_SHIFTS+1) )+1;// /(tile_width)) +1;   T_S+1 because bg tile 128, not 64
		max_y = ((scrolly + (h>>1) ) >> (TILE_SHIFTS+1) )+1;// /(tile_height)) +1;

        //if (min_y < 0.5) min_y = 0.5;

		al_hold_bitmap_drawing(1);
		for (y = min_y; y < max_y; y++)
		{
			for (x = min_x; x < max_x; x++)
			{
				//int i = tile_map[x + y * MAX_MAP_WIDTH];    //pull tile index rom array
				//int u = i << 6;                             //multiply by 64 to get pixel index
				//int v=0;

				//int u = (i & 0x0007)<<6;    //bottom 3 bits * 64
				//int v = (i & 0xfff8)<<3;    //upper bits /8 then * 64

                                            //sx sy sw         sh         dx                        dy
				//al_draw_bitmap_region(tr_map, u, v, TILE_WIDTH, TILE_HEIGHT, win_x + (x<<TILE_SHIFTS), win_y + (y<<TILE_SHIFTS), 0);
				//al_draw_bitmap(background, win_x + (x<<(TILE_SHIFTS+1)),  win_y + (y<<(TILE_SHIFTS+1)), 0 );
				al_draw_tinted_bitmap(background, fade , win_x + (x<<(TILE_SHIFTS+1)),  win_y + (y<<(TILE_SHIFTS+1)), 0 );
			}
		}
		al_hold_bitmap_drawing(0);

		al_identity_transform(&transform);
    	al_use_transform(&transform);
	}
	else if (Map.type == 0)	//tr-style single image map
	{
    	al_draw_scaled_bitmap(background, (scrollx-w*0.5)/2,(scrolly-h*0.5)/2, w/2, h/2, win_x, win_y, w, h, 0); //src x,y,w,h dst x,y,w,h,flags
	}
	else if (Map.type == 2)
	{
    	al_draw_scaled_bitmap(background, (scrollx-w*0.5),(scrolly-h*0.5), w, h, win_x, win_y, w, h, 0); //src x,y,w,h dst x,y,w,h,flags
	}

}

void draw_map(int scrollx, int scrolly, int win_x, int win_y, int w, int h)
{
	ALLEGRO_TRANSFORM transform;
	ALLEGRO_COLOR grid_colour;
	int x,y;
	int min_x, min_y, max_x, max_y;
	int i;

	if (Map.type==1)	//1 for tiled
	{
		al_identity_transform(&transform);  		            /* Initialize transformation. */
		al_translate_transform(&transform, -scrollx, -scrolly); /* Move to scroll position. */
		al_translate_transform(&transform, w>>1, h>>1);         /* Move scroll position to screen center. */
		al_use_transform(&transform);                           /* All subsequent drawing is transformed. */

		min_x = (scrollx - (w>>1) ) >> TILE_SHIFTS;// /(tile_width);	//optimise by using shifts, rather than divides....
		min_y = (scrolly - (h>>1) ) >> TILE_SHIFTS;// /(tile_height);

		max_x = ((scrollx + (w>>1) ) >> TILE_SHIFTS )+1;// /(tile_width)) +1;
		max_y = ((scrolly + (h>>1) ) >> TILE_SHIFTS )+1;// /(tile_height)) +1;

		al_hold_bitmap_drawing(1);
		for (y = min_y; y < max_y; y++)
		{
			for (x = min_x; x < max_x; x++)
			{
				int i = tile_map[x + y * MAX_MAP_WIDTH];    //pull tile index From array
				//int u = i << 6;                             //multiply by 64 to get pixel index
				//int v=0;

				int u = (i & 0x0007)<<6;    //bottom 3 bits * 64
				int v = (i & 0xfff8)<<3;    //upper bits /8 then * 64

                                            //sx sy sw         sh         dx                        dy
				al_draw_bitmap_region(tr_map, u, v, TILE_WIDTH, TILE_HEIGHT, win_x + (x<<TILE_SHIFTS), win_y + (y<<TILE_SHIFTS), 0);
			}
		}
		al_hold_bitmap_drawing(0);

		if (grid)
		{
			if (grid == 1) grid_colour = al_map_rgb(255, 255, 255);
			if (grid == 2) grid_colour = al_map_rgb(128, 128, 128);
			if (grid == 3) grid_colour = al_map_rgb(0, 0, 0);

			for (i=0 ; i<map_width ; i++)
			{
				al_draw_filled_rectangle(i*TILE_WIDTH+win_x,0+win_y,i*TILE_WIDTH+win_x+1,map_height*TILE_HEIGHT+win_y,grid_colour);
			}
			for (i=0 ; i<map_height ; i++)
			{
				al_draw_filled_rectangle(0+win_x,i*TILE_HEIGHT+win_y,map_width*TILE_WIDTH+win_x,i*TILE_HEIGHT+win_y+1,grid_colour);
			}

		}

		al_identity_transform(&transform);
    	al_use_transform(&transform);
	}
	else if (Map.type ==2)  //single image map, but not upscaled
    {
        al_draw_scaled_bitmap(tr_map, (scrollx-w*0.5),(scrolly-h*0.5), w, h, win_x, win_y, w, h, 0); //src x,y,w,h dst x,y,w,h,flags
    }
	else	//tr-style single image map, upscaled by 2
	{
    	al_draw_scaled_bitmap(tr_map, (scrollx-w*0.5)/2,(scrolly-h*0.5)/2, w/2, h/2, win_x, win_y, w, h, 0); //src x,y,w,h dst x,y,w,h,flags
		if (grid)
		{
			if (grid == 1) grid_colour = al_map_rgb(255, 255, 255);
			if (grid == 2) grid_colour = al_map_rgb(128, 128, 128);
			if (grid == 3) grid_colour = al_map_rgb(0, 0, 0);

			for (i=0 ; i<map_width ; i++)
			{
				al_draw_filled_rectangle(i*TILE_WIDTH+win_x,0+win_y,i*TILE_WIDTH+win_x+1,map_height*TILE_HEIGHT+win_y,grid_colour);
			}
			for (i=0 ; i<map_height ; i++)
			{
				al_draw_filled_rectangle(0+win_x,i*TILE_HEIGHT+win_y,map_width*TILE_WIDTH+win_x,i*TILE_HEIGHT+win_y+1,grid_colour);
			}

		}
	}
}

//also does bullets and sentries
void draw_ships(int scrollx, int scrolly, int x, int y, int w, int h)
{
	ALLEGRO_TRANSFORM transform;
	int i;
	int miner_x, jewel_x;
	unsigned char r, g, b, alpha;

	/* Initialize transformation. */
    al_identity_transform(&transform);

    /* Move to scroll position. */
    //I'm thinking of this like a camera - so we move the camera to where the ship is.
    //Actually, as the numbers are -ve, it seems more like the camera/viewport is fixed, and we're moving the world.
    //so we move everything (ship, bullets, tiles) to 0,0
    al_translate_transform(&transform, -scrollx, -scrolly);

    /* Move scroll position to screen center. */
    //Now we move it to centre of viewport/window thing
    //al_translate_transform(&transform, w * 0.5 + STATUS_BAR_WIDTH, h * 0.5);
    al_translate_transform(&transform, x + w * 0.5 , y + h * 0.5);

    /* All subsequent drawing is transformed. */
	al_use_transform(&transform);

	al_hold_bitmap_drawing(1);

	//draw bullets first
	int current_bullet;//, previous_bullet;

	current_bullet = first_bullet;
	//previous_bullet = END_OF_LIST;

	while(current_bullet != END_OF_LIST)
	{
		//only for colour-changing bullets
		switch(Bullet[current_bullet].type)
		{
			case BLT_HEATSEEKER:
				if (Bullet[current_bullet].ttl&0x4)
					Bullet[current_bullet].colour = al_map_rgb(0,255,0);
				else
					Bullet[current_bullet].colour = al_map_rgb(0,0,0);
			break;
			case BLT_MINE:
				al_unmap_rgb(Bullet[current_bullet].colour, &r,&g,&b);

				if ((Bullet[current_bullet].ttl & 0x1f) < 0x10)
					r -= 16;
				else
					r += 16;
				Bullet[current_bullet].colour = al_map_rgb(r,r,0);
			break;
			case BLT_HEAVY:
                Bullet[current_bullet].colour = al_map_rgb(255,0,0);
            break;
            case BLT_LAVA:
                Bullet[current_bullet].colour = al_map_rgb(255,(Bullet[current_bullet].ttl*Bullet[current_bullet].ttl/256),0);
            break;
			default:
			    Bullet[current_bullet].colour = al_map_rgb(255,255,255);
			break;
		}

		al_draw_tinted_bitmap(bullets_bmp,Bullet[current_bullet].colour,Bullet[current_bullet].xpos, Bullet[current_bullet].ypos,0);
		current_bullet = Bullet[current_bullet].next_bullet;
	}

	for(i=0 ; i<Map.num_forcefields ; i++)
	{
		//if (Map.sentry[Map.forcefield[i].sentry].alive)
		if (Map.forcefield[i].alpha)
		{
			x=Map.forcefield[i].min_x;
			y=Map.forcefield[i].min_y;

			alpha = Map.forcefield[i].alpha;

			if (x == Map.forcefield[i].half_x )	//horizontal - see init code
			{
				while (x<Map.forcefield[i].max_x)
			 	{	                      //bmp    srcx                              srcy size    dstx dsty
					al_draw_tinted_bitmap_region(sentries,al_map_rgba(alpha, alpha, alpha, alpha), Map.forcefield[i].closed_sprite*64,0,   64, 64, x,   y, 0);
					x+=64;
					if (x+64>Map.forcefield[i].max_x)
					{
						//just draw part of the sprite, dummy!
						//al_draw_bitmap_region(sentries,Map.forcefield[i].alive_sprite*64,0,   64, 64, Map.forcefield[i].max_x-64,   y, 0);
						al_draw_tinted_bitmap_region(sentries, al_map_rgba(alpha, alpha, alpha, alpha), Map.forcefield[i].closed_sprite*64,0,   Map.forcefield[i].max_x-x, 64, x, y, 0);
						break;
					}
				}
			}
			else
			{
				while (y<Map.forcefield[i].max_y)
			 	{	                      //bmp    srcx                              srcy size    dstx dsty
					al_draw_tinted_bitmap_region(sentries, al_map_rgba(alpha, alpha, alpha, alpha), Map.forcefield[i].closed_sprite*64,0,   64, 64, x,   y, 0);
					y+=64;
					if (y+64>Map.forcefield[i].max_y)
					{
						//al_draw_bitmap_region(sentries,Map.forcefield[i].alive_sprite*64,0,   64, 64, x, Map.forcefield[i].max_y-64, 0);
						al_draw_tinted_bitmap_region(sentries, al_map_rgba(alpha, alpha, alpha, alpha), Map.forcefield[i].closed_sprite*64,0,   64, Map.forcefield[i].max_y-y, x, y, 0);
						break;
					}
				}
			}
		}
	}

	//then sentries
	for(i=0 ; i<Map.num_sentries ; i++)
	{
		if (Map.sentry[i].alive)
			al_draw_bitmap_region(sentries,Map.sentry[i].alive_sprite*64,0, 64, 64,Map.sentry[i].x-64/2,Map.sentry[i].y-64/2, 0);
		else
			al_draw_bitmap_region(sentries,Map.sentry[i].dead_sprite*64,0, 64, 64,Map.sentry[i].x-64/2,Map.sentry[i].y-64/2, 0);
	}

	for(i=0 ; i<Map.num_switches ; i++)
	{
		if (Map.switches[i].open)
			al_draw_bitmap_region(sentries,Map.switches[i].open_sprite*64,  0, 64, 64,Map.switches[i].x-64/2,Map.switches[i].y-64/2, 0);
		else
			al_draw_bitmap_region(sentries,Map.switches[i].closed_sprite*64,0, 64, 64,Map.switches[i].x-64/2,Map.switches[i].y-64/2, 0);
	}

	//finally ships, miners and jewels
	//Use draw_bitmap_region with offset for sprite sheet
	for(i=0 ; i<num_ships ; i++)
	{
		if (Ship[i].reincarnate_timer)
		{}
		else
		{
			if (Ship[i].landed)
			{
				//draw running miner
				if (Map.pad[Ship[i].pad].miners)
				{
					Ship[i].miner_count++; //inc count
					miner_x = Map.pad[Ship[i].pad].min_x + (Ship[i].miner_count)*2;	//2 pixels per frame
					if (miner_x > Ship[i].xpos - 12)
					{
						Ship[i].miner_count = 0;	//restart
						Map.pad[Ship[i].pad].miners--;	//don't do this in UpdateLandedShip() !!!
						Ship[i].miners++;
						//al_play_sample(yippee, 1, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
						al_play_sample_instance(yippee_inst);
					}
					//                    bmp    srcx                      srcy size      dstx    dsty
					al_draw_bitmap_region(miner,(Ship[i].miner_count%8)*24,0,   24,  24,  miner_x,Map.pad[Ship[i].pad].y-13,0);
				}
				//draw rolling jewel
				if (Map.pad[Ship[i].pad].jewels)
				{
					Ship[i].jewel_count++; //inc count
					jewel_x = Map.pad[Ship[i].pad].min_x + (Ship[i].jewel_count)*2;	//2 pixels per frame
					if (jewel_x > Ship[i].xpos - 12)
					{
						Ship[i].jewel_count = 0;	//restart
						Map.pad[Ship[i].pad].jewels--;	//don't do this in UpdateLandedShip() !!!
						Ship[i].jewels++;
						//al_play_sample(clunk, 1, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
						al_play_sample_instance(clunk_inst);
					}
					//                    bmp    srcx                      srcy size      dstx    dsty
					al_draw_bitmap_region(jewel,(Ship[i].jewel_count%16)*24,0,   24,  24,  jewel_x,Map.pad[Ship[i].pad].y-17,0);
				}
			}
			//ship
			al_draw_bitmap_region(ships,Ship[i].angle*SHIP_SIZE_X,(2*Ship[i].image + (Ship[i].thrust?1:0) )*SHIP_SIZE_Y, SHIP_SIZE_X, SHIP_SIZE_Y,Ship[i].xpos-SHIP_SIZE_X/2,Ship[i].ypos-SHIP_SIZE_Y/2, 0);
		}
	}

    al_hold_bitmap_drawing(0);

    al_identity_transform(&transform);
    al_use_transform(&transform);
}

//ship config menu (i.e. weapons, ammo, fuel)
void draw_menu(int ship_num, int x, int y, int w, int h)
{
	int j;

	if (num_ships == 1)
		{
			x+=w/4;
			y+=h/4;
		}
		if (num_ships == 2)
			x+=w/4;

		al_draw_bitmap(panel_bmp,x, y,0);

		al_draw_filled_rectangle(x+430,y+45+60*Ship[ship_num].menu_state,x+550,y+75+60*Ship[ship_num].menu_state,al_map_rgba(32, 0, 0, 20));

		al_draw_bitmap_region(panel_pressed_bmp,20+Ship[ship_num].ammo1_type*110,70,80,80,x+20+Ship[ship_num].ammo1_type*110,y+70,0); //src x,y,w,h dst x,y,flags
		al_draw_bitmap_region(panel_pressed_bmp,20+(Ship[ship_num].ammo2_type-4)*110,200,80,80,x+20+(Ship[ship_num].ammo2_type-4)*110,y+200,0); //src x,y,w,h dst x,y,flags

		al_draw_filled_rectangle(x+35,y+55,x+35+Ship[ship_num].user_ammo1*3.8,y+65,al_map_rgb(128, 0, 0));				//ammo1
		al_draw_filled_rectangle(x+35,y+57,x+35+Ship[ship_num].user_ammo1*3.8,y+59,al_map_rgba(255, 255, 255, 20));

		for (j=0 ; j<Ship[ship_num].user_ammo2 ; j++)
		{
			al_draw_filled_rounded_rectangle(x+35+j*47,y+175,x+35+j*47+37,y+185,4,4,al_map_rgb(0, 128, 128));
			al_draw_filled_rectangle        (x+37+j*47,y+177,x+35+j*47+35,y+179,al_map_rgba(255, 255, 255, 20));
		}

		al_draw_filled_rectangle(x+35,y+295,x+35+Ship[ship_num].user_fuel*3.8,y+305,al_map_rgb(128, 128, 0));				//fuel
		al_draw_filled_rectangle(x+35,y+297,x+35+Ship[ship_num].user_fuel*3.8,y+299,al_map_rgba(255, 255, 255, 20));

		return;
}

void draw_status_bar(int num_ships)
{
	int i,j;
	int w,h,yoffset;
	int bs;	//size of coloured block;
	int first_ship;

    w = al_get_display_width(display);
    h = al_get_display_height(display);

	//al_set_clipping_rectangle(0, 0, SCREENX, SCREENY);
	al_set_clipping_rectangle(0, 0, w, h);

	yoffset = (h-STATUS_BAR_HEIGHT)/2;

	//Status info
	al_draw_bitmap(status_bg,0,yoffset,0);

	if (Map.mission)
		bs = 160;
	else
		bs = 140;

	yoffset += 5;

	//if (Net.net)
	if (Net.client || Net.server)
    {
        first_ship = Net.id;
        num_ships = first_ship+1;
        yoffset -= first_ship*bs;
    }
    else
    {
        first_ship=0;
    }

	for (i=first_ship ; i<num_ships ; i++)
	{
		//al_draw_filled_rectangle(0,i*100,120,i*100+90,Ship[i].colour);	//120*90 - make a nice bitmap background.
		al_draw_filled_rectangle(15,yoffset+i*bs,135,yoffset+i*bs+(bs-10),Ship[i].statuscolour);
		//al_draw_bitmap(Ship[i].status_bg,0,i*100,0);

		al_draw_filled_rectangle(15,yoffset+i*bs+10,15+Ship[i].shield,yoffset+i*bs+20,al_map_rgb(0, 128, 0));
		al_draw_filled_rectangle(15,yoffset+i*bs+12,15+Ship[i].shield-2,yoffset+i*bs+14,al_map_rgba(255, 255, 255, 64));

		al_draw_filled_rectangle(15,yoffset+i*bs+30,15+Ship[i].ammo1,yoffset+i*bs+40,al_map_rgb(128, 0, 0));
		al_draw_filled_rectangle(15,yoffset+i*bs+32,15+Ship[i].ammo1-2,yoffset+i*bs+34,al_map_rgba(255, 255, 255, 64));

		for (j=0 ; j<Ship[i].ammo2 ; j++)
		{
			al_draw_filled_rectangle(15+j*12+1,yoffset+i*bs+50,15+j*12+11,yoffset+i*bs+60,al_map_rgb(0, 128, 128));
			al_draw_filled_rectangle(15+j*12+1,yoffset+i*bs+52,15+j*12+11,yoffset+i*bs+54,al_map_rgba(255, 255, 255, 64));
		}


		al_draw_filled_rectangle(15,yoffset+i*bs+70,15+(Ship[i].fuel>>4),yoffset+i*bs+80,al_map_rgb(128, 128, 0));
		al_draw_filled_rectangle(15,yoffset+i*bs+72,15+(Ship[i].fuel>>4)-2,yoffset+i*bs+74,al_map_rgba(255, 255, 255, 64));

		for (j=0 ; j<Ship[i].lives ; j++)
			//al_draw_filled_circle(110, i*100+j*12+10, 5, al_map_rgb(255, 255, 255));
			al_draw_filled_triangle(15+106, yoffset+i*bs+j*12+15, 15+114, yoffset+i*bs+j*12+15,  15+110, yoffset+i*bs+j*12+5, al_map_rgb(255, 255, 255));

		if (Map.mission)
		{
			for (j=0 ; j<Ship[i].miners ; j++)
				al_draw_bitmap_region(pickups,16,0,16,16,15+j*12+1,yoffset+i*bs+90,0);
			for (j=0 ; j<Ship[i].jewels ; j++)
				al_draw_bitmap_region(pickups,0,0,16,16,15+j*12+1,yoffset+i*bs+110,0);
		}

		if (Ship[i].racing)
			al_draw_textf(race_font, al_map_rgb(255, 255, 255),15, yoffset+i*bs+(bs-35), ALLEGRO_ALIGN_LEFT, "%0.3f", Ship[i].current_lap_time);
		if (Ship[i].lap_complete)
        {
			al_draw_textf(race_font, al_map_rgb(255, 255,   0),15, yoffset+i*bs+(bs-55), ALLEGRO_ALIGN_LEFT, "%0.3f", Ship[i].last_lap_time);
			al_draw_textf(race_font, al_map_rgb(255, 128,   0),70, yoffset+i*bs+(bs-55), ALLEGRO_ALIGN_LEFT, "%0.3f", Ship[i].best_lap_time);
        }

	}

	//dividers
	//if (!Net.net)
	if (!Net.client && !Net.server)
    {
        if (num_ships > 1)
            //al_draw_filled_rectangle(STATUS_BAR_WIDTH,SCREENY/2-5,SCREENX,SCREENY/2+5,al_map_rgb(128, 128, 128));
            al_draw_filled_rectangle(STATUS_BAR_WIDTH,h/2-5,w,h/2+5,al_map_rgb(128, 128, 128));
        if (num_ships > 2)
            al_draw_filled_rectangle(((w-STATUS_BAR_WIDTH)/2)+STATUS_BAR_WIDTH-5,0,(w-STATUS_BAR_WIDTH)/2+STATUS_BAR_WIDTH+5,h,al_map_rgb(128, 128, 128));
    }

	return;
}

