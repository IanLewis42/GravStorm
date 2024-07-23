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

//#define ALLEGRO_UNSTABLE 1  //needed for haptics.

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
#include "auto.h"

#define MAX_MAP_WIDTH 100
#define MAX_MAP_HEIGHT 100
int tile_map[MAX_MAP_WIDTH * MAX_MAP_HEIGHT];

int map_height=0, map_width=0;

ALLEGRO_BITMAP *inst_bmp = NULL;
ALLEGRO_BITMAP *map_text_bmp;
ALLEGRO_BITMAP *marker_bmp;
ALLEGRO_BITMAP *marker2_bmp;

void draw_background(int scrollx, int scrolly, int win_x, int win_y, int w, int h);
void draw_map(int scrollx, int scrolly, int x, int y, int w, int h);
void draw_ships(int scrollx, int scrolly, int x, int y, int w, int h);
void draw_menu(int ship_num, int x, int y, int w, int h);
void display_instructions(void);

//#define LINE_SPACE 55
#define LINE_SPACE 35
//#define line_space 35

int grid = 0;

void display_new_menu(void)//int num_maps, int selected)	//show list of maps
{
	ALLEGRO_TRANSFORM transform;
	int i,j,y=0;
	int w,h;
	int line_space,col0,col1,col1in,col2,col3;
	int bgx,bgy,bgw,bgh;    //background
	int glow;

	float local_scale;
	char keys[]  = "Keys";
	char gpio[]  = "GPIO Joy";
	//char usb0[]  = "USB Joy 1";
	//char usb1[]  = "USB Joy 2";
	char joy[10];
	char* control_string;
#ifdef ANDROID
    char local[]  = "Local Game (no network)";
    char host[]  = "Host Network Game (choose level)";
#else
	char local[]  = "Local Game";
	char host[]  = "Host Network Game";
#endif
	char client[]    = "Join Network Game";
    char inst[]    = "Instructions";

	if (halted) return;

    ALLEGRO_COLOR colour;
	//use pre-multiplied alpha, i.e. rgb components must be multiplied by a.
    ALLEGRO_COLOR GroupActive    = al_map_rgba_f(0, 1, 0.5, 1);
    ALLEGRO_COLOR GroupGlow      = al_map_rgba_f(0, 0.188, 0.0, 0.078);
    ALLEGRO_COLOR GroupInactive  = al_map_rgba_f(0, 0.3, 0, 0.078);
    ALLEGRO_COLOR ItemCurrent    = al_map_rgba_f(1, 1, 0.2, 1);
    ALLEGRO_COLOR ItemCurrentGlow= al_map_rgba_f(0.2, 0.2, 0.04, 0.078);
    ALLEGRO_COLOR ItemSelected   = al_map_rgba_f(0.5, 0, 0, 1);
    ALLEGRO_COLOR ItemUnselected = al_map_rgba_f(1, 1, 0.8, 1);
    ALLEGRO_COLOR ItemExcluded   = al_map_rgba_f(0.5, 0.5, 0.5, 1);

	w = al_get_display_width(display);
    h = al_get_display_height(display);

	al_set_clipping_rectangle(0, 0, w, h);

    Menu.offset = 0;

    line_space = 35*font_scale;

    col0 = 20*font_scale;
    col1 = 30*font_scale;
    col1in = 40*font_scale; //indented
    col2 = 470*font_scale;  //network text
    col3 = 320*font_scale;  //number of players, keys.

	al_clear_to_color(al_map_rgb(0, 0, 0));

	//al_draw_bitmap(menu_bg_bmp,xoffset,yoffset,0);

	bgw = al_get_bitmap_width(menu_bg_bmp);
	bgh = al_get_bitmap_height(menu_bg_bmp);

	for(bgy=0 ; bgy<h ; bgy+=bgh)
    {
        for(bgx=0 ; bgx<w ; bgx+=bgw)
        {
            al_draw_bitmap(menu_bg_bmp,bgx,bgy,0);
        }
    }

    //Gravstorm logo
    //shadow
    local_scale = 0.4;
	al_identity_transform(&transform);			/* Initialize transformation. */
	al_scale_transform(&transform, local_scale, local_scale);	/* Rotate and scale around the center first. */
	al_translate_transform(&transform,w/2,h*0.9);//y+Ctrl.ctrl[BACK].size);
	al_use_transform(&transform);
	al_draw_textf(title_font, al_map_rgba(0, 0, 0,160),0, (al_get_font_ascent(title_font)/2)*local_scale,  ALLEGRO_ALIGN_CENTRE, "%s", NAME);
    //image
	bgw = al_get_bitmap_width(logo);
	al_identity_transform(&transform);
	al_scale_transform(&transform, local_scale*font_scale, local_scale*font_scale);	/* Rotate and scale around the center first. */
	al_translate_transform(&transform,0,h*0.89);//w/2+xoffset-7,y+yoffset+7);
	al_use_transform(&transform);
	//al_draw_textf(title_font, al_map_rgb(128, 128, 0),0, (al_get_font_ascent(title_font)/2)*scale,  ALLEGRO_ALIGN_CENTRE, "%s", NAME);
	int x = (bgw+40)*local_scale*font_scale;
	//al_draw_bitmap(logo,(w/2+x),0,0);//-w/2+17,47,0); //NUMBERS!!!

	al_draw_bitmap(logo,((w-x)/(2*local_scale*font_scale)),(int)(40*font_scale),0);//-w/2+17,47,0); //NUMBERS!!!

	//reset transform
	al_identity_transform(&transform);
	al_use_transform(&transform);

#ifdef ANDROID
    y = Select.sumdymax;

    al_set_clipping_rectangle(0, Ctrl.ctrl[BACK].y + Ctrl.ctrl[BACK].size, w, h);

    al_scale_transform(&transform,2,2);
    al_use_transform(&transform);
#else
    y=line_space;
#endif

    if (Menu.state == GAMETYPE)
    {
        al_draw_textf(small_font,      GroupActive,col0, y,  ALLEGRO_ALIGN_LEFT, "How do you want to play?");
        al_draw_textf(small_glow_font, GroupGlow,  col0, y,  ALLEGRO_ALIGN_LEFT, "How do you want to play?");

        y+=line_space;
        if (Menu.gametype == SOLO)
        {
           al_draw_textf(small_font, ItemCurrent    ,col1, y,  ALLEGRO_ALIGN_LEFT, "Play a solo mission");
           al_draw_textf(small_glow_font, ItemCurrentGlow,col1, y,  ALLEGRO_ALIGN_LEFT, "Play a solo mission");
        }
        else
            al_draw_textf(small_font, ItemUnselected,col1, y,  ALLEGRO_ALIGN_LEFT, "Play a solo mission");

        y+=line_space;
        if (Menu.gametype == COMPUTER)
        {
           al_draw_textf(small_font, ItemCurrent    ,col1, y,  ALLEGRO_ALIGN_LEFT, "Play against the AI");
           al_draw_textf(small_glow_font, ItemCurrentGlow,col1, y,  ALLEGRO_ALIGN_LEFT, "Play against the AI");
        }
        else
            al_draw_textf(small_font, ItemUnselected,col1, y,  ALLEGRO_ALIGN_LEFT, "Play against the AI");

        y+=line_space;
        if (Menu.gametype == FRIENDS)
        {
           al_draw_textf(small_font, ItemCurrent    ,col1, y,  ALLEGRO_ALIGN_LEFT, "Play with friends");
           al_draw_textf(small_glow_font, ItemCurrentGlow,col1, y,  ALLEGRO_ALIGN_LEFT, "Play with friends");
        }
        else
            al_draw_textf(small_font, ItemUnselected,col1, y,  ALLEGRO_ALIGN_LEFT, "Play with friends");

        y+=line_space;
        if (Menu.gametype == INST)
        {
           al_draw_textf(small_font, ItemCurrent    ,col1, y,  ALLEGRO_ALIGN_LEFT, "Instructions");
           al_draw_textf(small_glow_font, ItemCurrentGlow,col1, y,  ALLEGRO_ALIGN_LEFT, "Instructions");
        }
        else
            al_draw_textf(small_font, ItemUnselected,col1, y,  ALLEGRO_ALIGN_LEFT, "Instructions");
    }

    if (Menu.state == NETWORK)
    {
        al_draw_textf(small_font,      GroupActive,col0, y,  ALLEGRO_ALIGN_LEFT, "Do you want to:");
        al_draw_textf(small_glow_font, GroupGlow,  col0, y,  ALLEGRO_ALIGN_LEFT, "Do you want to:");
        y+=line_space;

#ifndef ANDROID
        if (Menu.netmode == LOCAL)
        {
           al_draw_textf(small_font, ItemCurrent    ,col1, y,  ALLEGRO_ALIGN_LEFT, "Play on this computer");
           al_draw_textf(small_glow_font, ItemCurrentGlow,col1, y,  ALLEGRO_ALIGN_LEFT, "Play on this computer");

           //al_draw_textf(small_font, ItemUnselected,col2, y,  ALLEGRO_ALIGN_LEFT, "Single player, or all players on one device.");

        }
        else
            al_draw_textf(small_font, ItemUnselected,col1, y,  ALLEGRO_ALIGN_LEFT, "Play on this computer");

        y+=line_space;
#endif
        if (Menu.netmode == HOST)
        {
           al_draw_textf(small_font, ItemCurrent    ,col1, y,  ALLEGRO_ALIGN_LEFT, "Start a local network game");
           al_draw_textf(small_glow_font, ItemCurrentGlow,col1, y,  ALLEGRO_ALIGN_LEFT, "Start a local network game");
#ifndef ANDROID
           //l_draw_textf(small_font, ItemUnselected,col2, y,  ALLEGRO_ALIGN_LEFT, "Host chooses level to play on.");
#endif
        }
        else
            al_draw_textf(small_font, ItemUnselected,col1, y,  ALLEGRO_ALIGN_LEFT, "Start a local network game");

        y+=line_space;

        if (Menu.netmode == CLIENT)
        {
           al_draw_textf(small_font, ItemCurrent    ,col1, y,  ALLEGRO_ALIGN_LEFT, "Join a local network game");
           al_draw_textf(small_glow_font, ItemCurrentGlow,col1, y,  ALLEGRO_ALIGN_LEFT, "Join a local network game");
#ifndef ANDROID
            //al_draw_textf(small_font, ItemUnselected,col2, y,  ALLEGRO_ALIGN_LEFT, "Join a local network game");
#endif
        }
        else
            al_draw_textf(small_font, ItemUnselected,col1, y,  ALLEGRO_ALIGN_LEFT, "Join a local network game");

        y+=line_space;
/*
        if (Menu.netmode == INST)
        {
            al_draw_textf(small_font, ItemCurrent    ,col1, y,  ALLEGRO_ALIGN_LEFT, "%s",inst);
            al_draw_textf(small_glow_font, ItemCurrentGlow,col1, y,  ALLEGRO_ALIGN_LEFT, "%s",inst);
            //al_draw_textf(small_font, ItemUnselected,col2, y,  ALLEGRO_ALIGN_LEFT, "Join a game someone else is hosting.");
        }
        else
            al_draw_textf(small_font, ItemUnselected,col1, y,  ALLEGRO_ALIGN_LEFT, "%s",inst);
*/
        y+=line_space*3;

        if (Net.client_state == ABORTED)
        {
            al_identity_transform(&transform);
            al_use_transform(&transform);
            al_draw_textf(small_font, ItemUnselected, w/2, 0.8*h,  ALLEGRO_ALIGN_CENTRE, "Previous game aborted by host.");
        }


        //else
        //    al_draw_textf(small_font, ItemUnselected,col1, y,  ALLEGRO_ALIGN_LEFT, "%s",netmodestr);
        /*
        y+=35;

        if (Menu.netmode == CLIENT)
            colour = ItemUnselected;
        else colour = ItemExcluded;

        al_draw_textf(small_font, colour,col1, y,  ALLEGRO_ALIGN_LEFT, "Address:");

        y+=35;

        strncpy(display_address,Net.temp_address,16);
        strncat(display_address,&cursor,1);

        if (Menu.col_pos == 1)
        {
           al_draw_textf(small_font, ItemCurrent    ,col1, y,  ALLEGRO_ALIGN_LEFT, "%s",display_address);
           //al_draw_textf(small_glow_font, ItemCurrentGlow,col1, y,  ALLEGRO_ALIGN_LEFT, "%s",Net.temp_address);
        }
        else
            al_draw_textf(small_font, colour,col1, y,  ALLEGRO_ALIGN_LEFT, "%s",Net.menuaddress);

        //al_draw_textf(small_font, colour,col1, y+35,  ALLEGRO_ALIGN_LEFT, "%08X",timer);
        */
    }
    else if (Menu.state == INSTRUCTIONS)
    {
        al_identity_transform(&transform);
        al_use_transform(&transform);
        display_instructions();
        return;
    }
	else if (Menu.state == LEVEL)
    {
	//Display maps; display all group names, and maps in current group
#ifdef ANDROID
        y = Select.sumdy;
#endif
        al_draw_textf(small_font, GroupActive, col0, y,  ALLEGRO_ALIGN_LEFT, "Level");
        al_draw_textf(small_glow_font, GroupGlow,   col0, y,  ALLEGRO_ALIGN_LEFT, "Level");

        if (Menu.gametype == SOLO)
            i = 0;
        else
            i = 1;
#ifdef ANDROID
        char temp[50];
        for ( ; i<Menu.display_groups ; i++) {
            al_draw_textf(small_font, GroupActive, col0, y += line_space, ALLEGRO_ALIGN_LEFT, "%s", (char *) &MapNames[i].Group);
            al_draw_textf(small_glow_font, GroupGlow, col0, y/*+=LINE_SPACE*/, ALLEGRO_ALIGN_LEFT, "%s", (char *) &MapNames[i].Group);

            for (j = 0; j < MapNames[i].Count; j++) {
                if (MapNames[i].Map[j].players > 1)
                    sprintf(temp,"%s (%d P)",(char *) &MapNames[i].Map[j],MapNames[i].Map[j].players);
                else
                    sprintf(temp,"%s",(char *) &MapNames[i].Map[j]);

                if (i == Menu.group && j == Menu.map) {
                    al_draw_textf(small_font, ItemCurrent, col1, y += Menu.expand, ALLEGRO_ALIGN_LEFT, "%s", (char*) temp);
                    al_draw_textf(small_glow_font, ItemCurrentGlow, col1, y, ALLEGRO_ALIGN_LEFT, "%s", (char *) temp);
                } else
                    al_draw_textf(small_font, ItemUnselected, col1, y += Menu.expand, ALLEGRO_ALIGN_LEFT, "%s", (char*) temp);
            }
        }
#else

        for ( ; i<Menu.display_groups ; i++)
        {
            if (i == Menu.group)
            {
                al_draw_textf(small_font, GroupActive ,col0, y+=line_space,  ALLEGRO_ALIGN_LEFT, "%s", (char*)&MapNames[i].Group);
                al_draw_textf(small_glow_font, GroupGlow ,col0, y/*+=LINE_SPACE*/,  ALLEGRO_ALIGN_LEFT, "%s", (char*)&MapNames[i].Group);

                for (j=0 ; j<MapNames[Menu.group].Count ; j++)
                {
                    glow = false;
                    if (j == Menu.map)
                    {
                        colour = ItemCurrent;   //so yellow if we're in col 0, i.e. changing this
                        glow = true;
                    }
                    else
                        colour = ItemUnselected;

                    al_draw_textf(small_font, colour ,col1, y+=Menu.expand,  ALLEGRO_ALIGN_LEFT, "%s", (char*)&MapNames[i].Map[j]);
                    if (glow)
                    {
                        al_draw_textf(small_glow_font, ItemCurrentGlow, col1,  y,  ALLEGRO_ALIGN_LEFT, "%s", (char*)&MapNames[i].Map[j]);
                        al_draw_textf(small_font, ItemUnselected,  col3, y,  ALLEGRO_ALIGN_LEFT, "Max players: %d", MapNames[i].Map[j].players);//Map.max_players);
                    }
                }
            }
            else
            {
                colour = GroupInactive;
                al_draw_textf(small_font, colour ,col0, y+=line_space,  ALLEGRO_ALIGN_LEFT, "%s", (char*)&MapNames[i].Group);
            }
        }
#endif
        if (Menu.netmode == HOST)
        {
            al_draw_textf(small_font, ItemUnselected,  Menu.offset+w/2, h*0.8,  ALLEGRO_ALIGN_CENTRE, "Select level to start network game.");
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
        //y=line_space;
        //y+= yoffset;

#ifndef ANDROID
        if (!(Net.client || Net.host) && Map.max_players > 1)
        {
            if (Menu.col_pos == 0)
            {
                al_draw_textf(small_font, GroupActive,col0, y,  ALLEGRO_ALIGN_LEFT, "Human Players:");
                al_draw_textf(small_glow_font, GroupGlow,col0, y,  ALLEGRO_ALIGN_LEFT, "Human Players:");
                al_draw_textf(small_font, ItemCurrent    ,(int)(300*font_scale), y,  ALLEGRO_ALIGN_LEFT, "%d",num_ships);
                al_draw_textf(small_glow_font, ItemCurrentGlow,(int)(300*font_scale), y,  ALLEGRO_ALIGN_LEFT, "%d",num_ships);
            }
            else
            {
                al_draw_textf(small_font, ItemUnselected,col0, y,  ALLEGRO_ALIGN_LEFT, "Human Players:");
                al_draw_textf(small_font, ItemUnselected,(int)(300*font_scale), y,  ALLEGRO_ALIGN_LEFT, "%d",num_ships);
            }
        }
        else
        {
            al_draw_textf(small_glow_font, GroupGlow,col0, y,  ALLEGRO_ALIGN_LEFT, "Choose your ship");
            al_draw_textf(small_font, GroupActive,col0, y,  ALLEGRO_ALIGN_LEFT, "Choose your ship");
        }

        y+=line_space*1.2;
#endif
        //debug
        //al_draw_textf(small_font, ItemUnselected,col10, y,  ALLEGRO_ALIGN_LEFT, "player:%d item:%d",Menu.player,Menu.item);
        //end debug

        for (i=0 ; i<Menu.ships ; i++)			//List players
        {
            if (i>num_ships-1) colour = ItemExcluded;
            else colour = ItemUnselected;
#ifdef ANDROID
            al_draw_textf(small_glow_font, GroupGlow,w/4, y,  ALLEGRO_ALIGN_CENTRE, "Choose your ship");
            al_draw_textf(small_font, GroupActive,w/4, y,  ALLEGRO_ALIGN_CENTRE, "Choose your ship");

            w = al_get_display_width(display);
            int step = w/16;
            y+=line_space *2;

            al_draw_filled_rounded_rectangle(Ship[i].image*step,y,Ship[i].image*step+52*scale,y+52*scale,10,10,al_map_rgba(64,64,0,64));

            for (j=0 ; j<8 ; j++)
            {
                al_draw_scaled_bitmap(ships,Ship[i].angle*SHIP_SIZE_X,2*j*SHIP_SIZE_Y, SHIP_SIZE_X, SHIP_SIZE_Y,step*j,y,SHIP_SIZE_X*scale,SHIP_SIZE_Y*scale, 0);
            }
#else
            if (!Net.client && !Net.server && Map.max_players > 1)
            {
                if (Menu.player == i && Menu.col_pos != 0)
                {
                    al_draw_textf(small_font, GroupActive,col0, y,  ALLEGRO_ALIGN_LEFT, "Player %d",i+1);
                    al_draw_textf(small_glow_font, GroupGlow,col0, y,  ALLEGRO_ALIGN_LEFT, "Player %d",i+1);
                }
                else
                    al_draw_textf(small_font, GroupInactive,col0, y,  ALLEGRO_ALIGN_LEFT, "Player %d",i+1);
            }

            y+=line_space*1.2;

            //scale = font_scale;

            //draw spinny ship & highlight
            if (Menu.item == 0 && Menu.player == i)
            {
                //yellow highlight
                al_draw_filled_rounded_rectangle(Menu.offset+103*scale,y,Menu.offset+(103+52)*scale,y+52*scale,10,10,al_map_rgba(64,64,0,64));
            }

            int temp = Ship[i].image-1;
            if (temp < 0) temp += 8;

            //greyed-out ship before
            al_draw_tinted_scaled_bitmap(grey_ships,al_map_rgba(160,160,160,160),Ship[i].angle*SHIP_SIZE_X,1*temp*SHIP_SIZE_Y, SHIP_SIZE_X, SHIP_SIZE_Y,Menu.offset+(25+Ship[i].offset)*scale,y+2,SHIP_SIZE_X*scale,SHIP_SIZE_Y*scale, 0);
            temp+=2;
            if (temp > 7) temp -=8;
            //and after
            al_draw_tinted_scaled_bitmap(grey_ships,al_map_rgba(160,160,160,160),Ship[i].angle*SHIP_SIZE_X,1*temp*SHIP_SIZE_Y, SHIP_SIZE_X, SHIP_SIZE_Y,Menu.offset+(185+Ship[i].offset)*scale,y+2,SHIP_SIZE_X*scale,SHIP_SIZE_Y*scale, 0);

            y+=2;
            //selected ship
            al_draw_scaled_bitmap(ships,Ship[i].angle*SHIP_SIZE_X,2*Ship[i].image*SHIP_SIZE_Y, SHIP_SIZE_X, SHIP_SIZE_Y,Menu.offset+(105+Ship[i].offset)*scale,y,SHIP_SIZE_X*scale,SHIP_SIZE_Y*scale, 0);

            if (Ship[i].offset > 0) Ship[i].offset-=6;
            if (Ship[i].offset < 0) Ship[i].offset+=6;
#endif
            Ship[i].angle++;
            if (Ship[i].angle == NUM_ANGLES) Ship[i].angle = 0;
            y+=45;
#ifndef ANDROID
            //controls - if keys, show keys
            if (Ship[i].controller == KEYS)
                control_string = keys;
            else if (Ship[i].controller == GPIO_JOYSTICK)
                control_string = gpio;
            /*else if (Ship[i].controller == USB_JOYSTICK0)
                control_string = usb0;
            else //if (Ship[i].controller == USB_JOYSTICK1)
                control_string = usb1;
            //else if (Ship[i].controller == NA)
            //    control_string = na;
            */
            else
            {
                sprintf(joy,"JOY %d",Ship[i].controller - USB_JOYSTICK0);
                control_string = joy;
            }

            if (Menu.item == 1 && Menu.player == i)
            {
                al_draw_textf(small_glow_font, ItemCurrentGlow,col1in, y,  ALLEGRO_ALIGN_LEFT, "%s",control_string);		//Control method for selected player
                al_draw_textf(small_font, ItemCurrent,col1in, y,  ALLEGRO_ALIGN_LEFT, "%s",control_string);		//Control method for selected player
            }
            else
                al_draw_textf(small_font, colour,col1in, y,  ALLEGRO_ALIGN_LEFT, "%s",control_string);		//Control method for selected player

            y+=line_space;

            char define_str[25];

            if (Ship[i].controller == GPIO_JOYSTICK)
                strncpy(define_str," ",25);
            else if (Ship[i].controller == KEYS)
                strncpy(define_str,"define keys ->",25);
            else//if (Ship[Menu.player].controller == GPIO_JOYSTICK)
                strncpy(define_str,"config ->",25);

           if (Menu.item == 2 && Menu.player == i)
            {
                al_draw_textf(small_glow_font, ItemCurrentGlow,col1in, y,  ALLEGRO_ALIGN_LEFT, define_str);		//Control method for selected player
                al_draw_textf(small_font, ItemCurrent,col1in, y,  ALLEGRO_ALIGN_LEFT, define_str);		//Control method for selected player
            }
            else
            {
                if (Ship[i].controller == GPIO_JOYSTICK) colour = ItemExcluded;
                al_draw_textf(small_font, colour,col1in, y,  ALLEGRO_ALIGN_LEFT, define_str);		//Control method for selected player
            }

            int key_start = 80*font_scale;

            if (Menu.item == 2 && Menu.player == i)// && Ship[i].controller == KEYS)
            {
                char left[30],right[30],up[30],down[30],thrust[30];

                if (Ship[i].controller == KEYS)
                {
                    strncpy(left,al_keycode_to_name(Ship[Menu.player].left_key),10);
                    strncpy(right,al_keycode_to_name(Ship[Menu.player].right_key),10);
                    strncpy(up,al_keycode_to_name(Ship[Menu.player].up_key),10);
                    strncpy(down,al_keycode_to_name(Ship[Menu.player].down_key),10);
                    strncpy(thrust,al_keycode_to_name(Ship[Menu.player].thrust_key),10);
                }
                else if (Ship[i].controller >= USB_JOYSTICK0 && Ship[i].controller <= USB_JOYSTICK3)
                {
                    int joy = Ship[Menu.player].controller-USB_JOYSTICK0;
                    char neg[4]="-ve",pos[4]="+ve";

                    if(USBJoystick[joy].Map[0].Type == STICK)
                        sprintf(left,"Stick %d | Axis %d | %s",USBJoystick[joy].Map[0].StickIdx,USBJoystick[joy].Map[0].AxisIdx, USBJoystick[joy].Map[0].Threshold>0?pos:neg);
                    else
                        sprintf(left,"Button %d",USBJoystick[joy].Map[0].ButIdx);

                    if(USBJoystick[joy].Map[1].Type == STICK)
                        sprintf(right,"Stick %d | Axis %d | %s",USBJoystick[joy].Map[1].StickIdx,USBJoystick[joy].Map[1].AxisIdx, USBJoystick[joy].Map[1].Threshold>0?pos:neg);
                    else
                        sprintf(right,"Button %d",USBJoystick[joy].Map[1].ButIdx);

                    if(USBJoystick[joy].Map[2].Type == STICK)
                        sprintf(up,"Stick %d | Axis %d | %s",USBJoystick[joy].Map[2].StickIdx,USBJoystick[joy].Map[2].AxisIdx, USBJoystick[joy].Map[2].Threshold>0?pos:neg);
                    else
                        sprintf(up,"Button %d",USBJoystick[joy].Map[2].ButIdx);

                    if(USBJoystick[joy].Map[3].Type == STICK)
                        sprintf(down,"Stick %d | Axis %d | %s",USBJoystick[joy].Map[3].StickIdx,USBJoystick[joy].Map[3].AxisIdx, USBJoystick[joy].Map[3].Threshold>0?pos:neg);
                    else
                        sprintf(down,"Button %d",USBJoystick[joy].Map[3].ButIdx);

                    if(USBJoystick[joy].Map[4].Type == STICK)
                        sprintf(thrust,"Stick %d | Axis %d | %s",USBJoystick[joy].Map[4].StickIdx,USBJoystick[joy].Map[4].AxisIdx, USBJoystick[joy].Map[4].Threshold>0?pos:neg);
                    else
                        sprintf(thrust,"Button %d",USBJoystick[joy].Map[4].ButIdx);
                }

                al_draw_textf(small_font, ItemUnselected ,col3, key_start+line_space*0,  ALLEGRO_ALIGN_LEFT, "Rotate Left :");
                if (Menu.define_keys && Menu.current_key == 0){colour = ItemCurrent; glow = true;}
                else {colour = ItemSelected; glow = false;}
                al_draw_textf(small_font, colour,col3, key_start+line_space*1,  ALLEGRO_ALIGN_LEFT, " %s",left);
                if (glow) al_draw_textf(small_glow_font, ItemCurrentGlow,col3, key_start+line_space*1,  ALLEGRO_ALIGN_LEFT, " %s",left);

                al_draw_textf(small_font, ItemUnselected, col3, key_start+line_space*2,  ALLEGRO_ALIGN_LEFT, "Rotate Right :");
                if (Menu.define_keys && Menu.current_key == 1){colour = ItemCurrent; glow = true;}
                else {colour = ItemSelected; glow = false;}
                al_draw_textf(small_font, colour,col3, key_start+line_space*3,  ALLEGRO_ALIGN_LEFT, " %s",right);
                if (glow) al_draw_textf(small_glow_font, ItemCurrentGlow,col3, key_start+line_space*3,  ALLEGRO_ALIGN_LEFT, " %s",right);

                al_draw_textf(small_font, ItemUnselected,col3, key_start+line_space*4,  ALLEGRO_ALIGN_LEFT, "Fire1 :");
                if (Menu.define_keys && Menu.current_key == 2){colour = ItemCurrent; glow = true;}
                else {colour = ItemSelected; glow = false;}
                al_draw_textf(small_font, colour,col3, key_start+line_space*5,  ALLEGRO_ALIGN_LEFT, " %s",up);
                if (glow)al_draw_textf(small_glow_font, ItemCurrentGlow ,col3, key_start+line_space*5,  ALLEGRO_ALIGN_LEFT, " %s",up);

                al_draw_textf(small_font, ItemUnselected,col3, key_start+line_space*6,  ALLEGRO_ALIGN_LEFT, "Fire2 :");
                if (Menu.define_keys && Menu.current_key == 3){colour = ItemCurrent; glow = true;}
                else {colour = ItemSelected; glow = false;}
                al_draw_textf(small_font, colour,col3, key_start+line_space*7,  ALLEGRO_ALIGN_LEFT, " %s",down);
                if (glow) al_draw_textf(small_glow_font, ItemCurrentGlow,col3, key_start+line_space*7,  ALLEGRO_ALIGN_LEFT, " %s",down);

                al_draw_textf(small_font, ItemUnselected,col3, key_start+line_space*8,  ALLEGRO_ALIGN_LEFT, "Thrust :");
                if (Menu.define_keys && Menu.current_key == 4){colour = ItemCurrent; glow = true;}
                else {colour = ItemSelected; glow = false;}
                al_draw_textf(small_font, colour,col3, key_start+line_space*9,  ALLEGRO_ALIGN_LEFT, " %s",thrust);
                if (glow) al_draw_textf(small_glow_font, ItemCurrentGlow,col3, key_start+line_space*9,  ALLEGRO_ALIGN_LEFT, " %s",thrust);
            }

            y+=line_space;
#endif
        }

        al_identity_transform(&transform);
        al_use_transform(&transform);

        if (Menu.netmode == HOST)
        {
            if (Net.address.host == 0)
                al_draw_textf(small_font, ItemUnselected,  w/2, 0.8*h,  ALLEGRO_ALIGN_CENTRE, "Network host started");
            else
                al_draw_textf(small_font, ItemUnselected,  w/2, 0.8*h,  ALLEGRO_ALIGN_CENTRE, "Network host started on %s", Net.myaddress);

            if (num_ships >= Map.max_players)
                al_draw_textf(small_font, ItemUnselected,  w/2, 0.85*h,  ALLEGRO_ALIGN_CENTRE, "%d players (no more space)", num_ships);//Net.clients);
            else
                al_draw_textf(small_font, ItemUnselected,  w/2, 0.85*h,  ALLEGRO_ALIGN_CENTRE, "%d players", num_ships);//Net.clients);
        }

        else if (Menu.netmode == CLIENT)
        {
            if (Net.client_state == SEARCHING)
                al_draw_textf(small_font, ItemUnselected,   w/2, 0.8*h,  ALLEGRO_ALIGN_CENTRE, "Searching for host...");
            else if (Net.client_state == FOUND)
                al_draw_textf(small_font, ItemUnselected,  w/2, 0.8*h,  ALLEGRO_ALIGN_CENTRE, "Found host at %s", Net.menuaddress);

            else if (Net.client_state == CONNECTED)
            {
                al_draw_textf(small_font, ItemUnselected,  w/2, 0.8*h,  ALLEGRO_ALIGN_CENTRE, "Connected to host at %s", Net.menuaddress);
                al_draw_textf(small_font, ItemUnselected,  w/2, 0.85*h,  ALLEGRO_ALIGN_CENTRE, "Host selected level '%s'", Net.mapfile);
            }

            else if (Net.client_state == NO_SPACE)
                al_draw_textf(small_font, ItemUnselected,   w/2, 0.8*h,  ALLEGRO_ALIGN_CENTRE, "Host at %s has no space", Net.menuaddress);

            else if (Net.client_state == NO_MAP)
                al_draw_textf(small_font, ItemUnselected,  w/2, 0.85*h,  ALLEGRO_ALIGN_CENTRE, "Failed to open host selected level '%s'", Net.mapfile);
        }

    }
    else if (Menu.state == AI)
    {
        float y2;
        char Easy[] = "Easy", Medium[] = "Med.", Hard[]= "Hard", Insane[] = "Insane";
        char *dif_str[] = {Easy, Medium, Hard, Insane};
#ifdef ANDROID
        y  = h/4 - 2*line_space;
        y2 = h/4 + 1*line_space;

        al_draw_textf(small_font, GroupActive,col0, y,  ALLEGRO_ALIGN_LEFT, "AI Ships:");
        al_draw_textf(small_glow_font, GroupGlow,col0, y,  ALLEGRO_ALIGN_LEFT, "AI Ships:");

        al_draw_textf(small_font, GroupActive,col0, y2,  ALLEGRO_ALIGN_LEFT, "Difficulty:");
        al_draw_textf(small_glow_font, GroupGlow,col0, y2,  ALLEGRO_ALIGN_LEFT, "Difficulty:");



#else
        y = line_space*2;
        y2 = line_space*4;
        if (Menu.col_pos == 0)
        {
            al_draw_textf(small_font, GroupActive,col0, y,  ALLEGRO_ALIGN_LEFT, "AI Ships:");
            al_draw_textf(small_glow_font, GroupGlow,col0, y,  ALLEGRO_ALIGN_LEFT, "AI Ships:");
            al_draw_textf(small_font, GroupInactive,col0, y2,  ALLEGRO_ALIGN_LEFT, "Difficulty:");
        }
        else
        {
            al_draw_textf(small_font, GroupInactive,col0, y,  ALLEGRO_ALIGN_LEFT, "AI Ships:");
            al_draw_textf(small_font, GroupActive,col0, y2,  ALLEGRO_ALIGN_LEFT, "Difficulty:");
            al_draw_textf(small_glow_font, GroupGlow,col0, y2,  ALLEGRO_ALIGN_LEFT, "Difficulty:");
        }
#endif

        for (i=0 ; i<MAX_SHIPS ; i++)
        {
            if (i > Map.max_players-num_ships)
                colour = ItemExcluded;
            else if (i == Menu.ai_ships)
            {
                colour = ItemCurrent;
                al_draw_textf(small_glow_font, ItemCurrentGlow,w/6+i*w/12, y,  ALLEGRO_ALIGN_LEFT, "%d",i);
            }
            else
                colour = ItemUnselected;

            al_draw_textf(small_font, colour,w/6+i*w/12, y,  ALLEGRO_ALIGN_LEFT, "%d",i);
        }

        for (i=0 ; i<4 ; i++)
        {
            if (i == Menu.difficulty)
            {
                colour = ItemCurrent;
                al_draw_textf(small_glow_font, ItemCurrentGlow,w/6+i*w/12, y2,  ALLEGRO_ALIGN_LEFT, "%s",dif_str[i]);
            }
            else
                colour = ItemUnselected;

            al_draw_textf(small_font, colour,w/6+i*w/12, y2,  ALLEGRO_ALIGN_LEFT, "%s",dif_str[i]);
        }



    }
    //reset transform
    al_identity_transform(&transform);
    al_use_transform(&transform);
    al_set_clipping_rectangle(0, 0, w, h);

    //#define ANDROID
    #ifdef ANDROID

    //al_draw_bitmap(Ctrl.directionbg.bmp, Ctrl.directionbg.x, Ctrl.directionbg.y,0);
    //al_draw_bitmap(Ctrl.direction.bmp, Ctrl.direction.x, Ctrl.direction.y,0);

    //al_draw_bitmap(Ctrl.thrust.bmp, Ctrl.thrust.x, Ctrl.thrust.y,0);
    //do start/select here
    //al_draw_bitmap(Ctrl.escape.bmp, Ctrl.escape.x, Ctrl.escape.y,0);

    //for (i=0 ; i<NO_BUTTON ; i++)
    //    if (Ctrl.ctrl[i].active)
    //        al_draw_scaled_bitmap(Ctrl.controls, Ctrl.ctrl[i].idx*200, i*200, 200,200, Ctrl.ctrl[i].x, Ctrl.ctrl[i].y, Ctrl.ctrl[i].size, Ctrl.ctrl[i].size, 0);
    draw_controls(al_map_rgba_f(0.5,0.5,0.5,0.5));

    #endif

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
            al_draw_textf(small_font, ItemUnselected ,Menu.offset+1400, y+line_space*0,  ALLEGRO_ALIGN_LEFT, "Rotate Left :");
            if (Menu.col == 3 && Menu.current_key == 0){colour = ItemCurrent; glow = true;}
            else {colour = ItemSelected; glow = false;}
            al_draw_textf(small_font, colour,Menu.offset+1400, y+line_space*1,  ALLEGRO_ALIGN_LEFT, " %s",al_keycode_to_name(Ship[Menu.player].left_key));
            if (glow) al_draw_textf(small_glow_font, ItemCurrentGlow,Menu.offset+1400, y+line_space*1,  ALLEGRO_ALIGN_LEFT, " %s",al_keycode_to_name(Ship[Menu.player].left_key));

            al_draw_textf(small_font, ItemUnselected, Menu.offset+1400, y+line_space*2,  ALLEGRO_ALIGN_LEFT, "Rotate Right :");
            if (Menu.col == 3 && Menu.current_key == 1){colour = ItemCurrent; glow = true;}
            else {colour = ItemSelected; glow = false;}
            al_draw_textf(small_font, colour,Menu.offset+1400, y+line_space*3,  ALLEGRO_ALIGN_LEFT, " %s",al_keycode_to_name(Ship[Menu.player].right_key));
            if (glow) al_draw_textf(small_glow_font, ItemCurrentGlow,Menu.offset+1400, y+line_space*3,  ALLEGRO_ALIGN_LEFT, " %s",al_keycode_to_name(Ship[Menu.player].right_key));

            al_draw_textf(small_font, ItemUnselected,Menu.offset+1400, y+line_space*4,  ALLEGRO_ALIGN_LEFT, "Fire1 :");
            if (Menu.col == 3 && Menu.current_key == 2){colour = ItemCurrent; glow = true;}
            else {colour = ItemSelected; glow = false;}
            al_draw_textf(small_font, colour,Menu.offset+1400, y+line_space*5,  ALLEGRO_ALIGN_LEFT, " %s",al_keycode_to_name(Ship[Menu.player].up_key));
            if (glow)al_draw_textf(small_glow_font, ItemCurrentGlow ,Menu.offset+1400, y+line_space*5,  ALLEGRO_ALIGN_LEFT, " %s",al_keycode_to_name(Ship[Menu.player].up_key));

            al_draw_textf(small_font, ItemUnselected,Menu.offset+1400, y+line_space*6,  ALLEGRO_ALIGN_LEFT, "Fire2 :");
            if (Menu.col == 3 && Menu.current_key == 3){colour = ItemCurrent; glow = true;}
            else {colour = ItemSelected; glow = false;}
            al_draw_textf(small_font, colour,Menu.offset+1400, y+line_space*7,  ALLEGRO_ALIGN_LEFT, " %s",al_keycode_to_name(Ship[Menu.player].down_key));
            if (glow) al_draw_textf(small_glow_font, ItemCurrentGlow,Menu.offset+1400, y+line_space*7,  ALLEGRO_ALIGN_LEFT, " %s",al_keycode_to_name(Ship[Menu.player].down_key));

            al_draw_textf(small_font, ItemUnselected,Menu.offset+1400, y+line_space*8,  ALLEGRO_ALIGN_LEFT, "Thrust :");
            if (Menu.col == 3 && Menu.current_key == 4){colour = ItemCurrent; glow = true;}
            else {colour = ItemSelected; glow = false;}
            al_draw_textf(small_font, colour,Menu.offset+1400, y+line_space*9,  ALLEGRO_ALIGN_LEFT, " %s",al_keycode_to_name(Ship[Menu.player].thrust_key));
            if (glow) al_draw_textf(small_glow_font, ItemCurrentGlow,Menu.offset+1400, y+line_space*9,  ALLEGRO_ALIGN_LEFT, " %s",al_keycode_to_name(Ship[Menu.player].thrust_key));
        }
    }
    */
	al_flip_display();

	return;
}

void display_map_text(int done, int timer)
{
    //ALLEGRO_FILE * description_file;
    //char line[200];
    //int i=20;
    //int w,h,bgw,bgh,bgx,bgy;
    //int line_space;
    //ALLEGRO_USTR *ustr = NULL;

    if (halted) return;

    al_clear_to_color(al_map_rgb(0, 0, 0));
    al_draw_bitmap(map_text_bmp,0,0,0);

/*
    line_space = 35*font_scale;

#ifdef ANDROID
    i+=2*line_space;
#endif

    al_clear_to_color(al_map_rgb(0, 0, 0));

    w = al_get_display_width(display);
    h = al_get_display_height(display);
    bgw = al_get_bitmap_width(menu_bg_bmp);
    bgh = al_get_bitmap_height(menu_bg_bmp);

    for(bgy=0 ; bgy<h ; bgy+=bgh)
    {
        for(bgx=0 ; bgx<w ; bgx+=bgw)
        {
            al_draw_bitmap(menu_bg_bmp,bgx,bgy,0);
        }
    }

	al_draw_filled_rectangle(0,0,w,h,al_map_rgba(0,0,0,timer*6));	//black filter to darken?

	if ((description_file = al_fopen(Map.description_file_name,"r")))
	{
		while (al_fgets(description_file, line, 200) != NULL)
		{
            ustr = al_ustr_new(line);
            al_draw_ustr(small_font, al_map_rgb(timer*8, timer*8, timer*8),(int)(20*font_scale), i,  ALLEGRO_ALIGN_LEFT, ustr);
			i+=line_space;
            if (ustr) al_ustr_free(ustr);
		}

		al_fclose(description_file); //close file
	}
	else
	{
		al_draw_textf(small_font, al_map_rgb(timer*8, timer*8, timer*8),(int)(20*font_scale), i,  ALLEGRO_ALIGN_LEFT, "%s",(char*)&MapNames[Menu.group].Map[Menu.map]);
		i+=line_space;
		al_draw_textf(small_font, al_map_rgb(timer*8, timer*8, timer*8),(int)(20*font_scale), i,  ALLEGRO_ALIGN_LEFT, "Couldn't open description file: %s",Map.description_file_name);
	}

    if (done)
        al_draw_textf(small_font, al_map_rgb(128, 0, 0),(int)(20*font_scale), i+line_space,  ALLEGRO_ALIGN_LEFT, "Painna Nappa / Press Button");
       */
    Ctrl.ctrl[SELECT].idx = 0;
    draw_controls(al_map_rgba_f(0.5,0.5,0.5,0.5));

    al_flip_display();

    return;
}


//idea: draw inst to mem bmp (once)
//draw bmp to display every time, offset by scroll.

void display_instructions(void)
{
    int w,h,bgw,bgh,bgx,bgy;
    int line_space;

    if (halted) return;

    line_space = 35*font_scale;

    w = al_get_display_width(display);
    h = al_get_display_height(display);
    bgw = al_get_bitmap_width(menu_bg_bmp);
    bgh = al_get_bitmap_height(menu_bg_bmp);

    al_set_clipping_rectangle(0,0,w,h);

    al_clear_to_color(al_map_rgb(0, 0, 0));

    for (bgy = 0; bgy < h; bgy += bgh) {
        for (bgx = 0; bgx < w; bgx += bgw) {
            al_draw_bitmap(menu_bg_bmp, bgx, bgy, 0);
        }
    }

    al_draw_filled_rectangle(0, 0, w, h, al_map_rgba(0, 0, 0, 180));    //black filter to darken?

#ifdef ANDROID
    al_set_clipping_rectangle(0, Ctrl.ctrl[BACK].y + Ctrl.ctrl[BACK].size, w, (h - 3 * line_space));
    Menu.y_origin = Select.sumdy + Ctrl.ctrl[BACK].y + Ctrl.ctrl[BACK].size - 4.5*line_space;
#endif

    if (inst_bmp == NULL)
    	al_fprintf(logfile,"inst_bmp == NULL\n");
    else
    	al_draw_bitmap(inst_bmp,0,Menu.y_origin + 2.5*line_space,0);

    al_set_clipping_rectangle(0,0,w,h);
    draw_controls(al_map_rgba_f(0.5,0.5,0.5,0.5));
    al_flip_display();

    return;
}

void display_wait_text(void)
{
    int w,h,bgw,bgh;
    int bgx,bgy;

    if (halted) return;

    w = al_get_display_width(display);
    h = al_get_display_height(display);
    bgw = al_get_bitmap_width(menu_bg_bmp);
    bgh = al_get_bitmap_height(menu_bg_bmp);

    al_clear_to_color(al_map_rgb(0, 0, 0));

    for(bgy=0 ; bgy<h ; bgy+=bgh)
    {
        for(bgx=0 ; bgx<w ; bgx+=bgw)
        {
            al_draw_bitmap(menu_bg_bmp,bgx,bgy,0);
        }
    }

    al_draw_textf(small_font, al_map_rgb_f(1, 1, 0.8), w/2, h/2,  ALLEGRO_ALIGN_CENTRE, "Waiting for host...");

    al_flip_display();
}

void __attribute__ ((noinline)) make_bullet_bitmap(void)
{
    al_fprintf(logfile,"Creating bullets bitmap\n");
    bullets_bmp = al_create_bitmap(10, 10);			//create a bitmap
	al_set_target_bitmap(bullets_bmp);					//set it as the default target for all al_draw_ operations
    al_draw_filled_circle(2, 2, 2, al_map_rgb(255, 255, 255));

    marker_bmp = al_create_bitmap(20, 20);			//create a bitmap
	al_set_target_bitmap(marker_bmp);					//set it as the default target for all al_draw_ operations
    al_draw_filled_circle(10,10,10,al_map_rgba(128,0,0,128));

    marker2_bmp = al_create_bitmap(20, 20);			//create a bitmap
	al_set_target_bitmap(marker2_bmp);					//set it as the default target for all al_draw_ operations
    al_draw_filled_circle(10,10,10,al_map_rgb(255,255,255));

    al_set_target_backbuffer(display);			//Put default target back
}


void make_map_text_bitmap(void)
{
    int line_space;
    char line[200];

    ALLEGRO_FILE * description_file;
    int i=20;
    int w,h,bgw,bgh,bgx,bgy;
    ALLEGRO_USTR *ustr = NULL;

    //line_space = 30*font_scale;

    map_text_bmp = al_create_bitmap(al_get_display_width(display), al_get_display_height(display));			//create a bitmap
    al_set_target_bitmap(map_text_bmp);					//set it as the default target for all al_draw_ operations

   line_space = 35*font_scale;

#ifdef ANDROID
    i+=2*line_space;
#endif

    al_clear_to_color(al_map_rgb(0, 0, 0));

    w = al_get_display_width(display);
    h = al_get_display_height(display);
    bgw = al_get_bitmap_width(menu_bg_bmp);
    bgh = al_get_bitmap_height(menu_bg_bmp);

    for(bgy=0 ; bgy<h ; bgy+=bgh)
    {
        for(bgx=0 ; bgx<w ; bgx+=bgw)
        {
            al_draw_bitmap(menu_bg_bmp,bgx,bgy,0);
        }
    }

	al_draw_filled_rectangle(0,0,w,h,al_map_rgba(0,0,0,180));	//black filter to darken?

	if ((description_file = al_fopen(Map.description_file_name,"r")))
	{
		while (al_fgets(description_file, line, 200) != NULL)
		{
            ustr = al_ustr_new(line);
            al_draw_ustr(small_font, al_map_rgb(240, 240, 240),(int)(20*font_scale), i,  ALLEGRO_ALIGN_LEFT, ustr);
			i+=line_space;
            if (ustr) al_ustr_free(ustr);
		}

		al_fclose(description_file); //close file
	}
	else
	{
		al_draw_textf(small_font, al_map_rgb(240, 240, 240),(int)(20*font_scale), i,  ALLEGRO_ALIGN_LEFT, "%s",(char*)&MapNames[Menu.group].Map[Menu.map]);
		i+=line_space;
		al_draw_textf(small_font, al_map_rgb(240, 240, 240),(int)(20*font_scale), i,  ALLEGRO_ALIGN_LEFT, "Couldn't open description file: %s",Map.description_file_name);
	}

    al_draw_textf(small_font, al_map_rgb(128, 0, 0),(int)(20*font_scale), i+line_space,  ALLEGRO_ALIGN_LEFT, "Painna Nappa / Press Button");

    //Ctrl.ctrl[SELECT].idx = 0;
    //draw_controls(al_map_rgba_f(0.5,0.5,0.5,0.5));

    al_set_target_backbuffer(display);			//Put default target back

}
void make_instructions_bitmap(void)
{
    ALLEGRO_FILE * inst;
    int i=0, j, line_space, idx, len;
    char line[200];

    if (inst_bmp != NULL) return;

    al_fprintf(logfile,"Making inst_bmp\n");

    line_space = 30*font_scale;

	int max_bmp = al_get_display_option(display, ALLEGRO_MAX_BITMAP_SIZE);

	al_fprintf(logfile,"Max bmp = %d\n",max_bmp);

	if (al_get_display_height(display)*4 < max_bmp)
		max_bmp = al_get_display_height(display)*4;

    inst_bmp = al_create_bitmap(al_get_display_width(display), max_bmp);			//create a bitmap
    al_set_target_bitmap(inst_bmp);					//set it as the default target for all al_draw_ operations

    if ((inst = al_fopen("instructions.txt", "r"))) {
        while (al_fgets(inst, line, 200) != NULL) {
            //ustr = al_ustr_new(line);
            //al_draw_ustr(small_font, al_map_rgb(240, 240, 240),(int)(20*font_scale), i,  ALLEGRO_ALIGN_LEFT, ustr);
            int drawn = false;
            len = strlen(line)-6;
            for(j=0 ; j<len ; j++)
            {
                if (strncmp(&line[j], "[ctrl]", 6) == 0) {
                    drawn = true;
                    //i += line_space / 2;
                    idx = strtol(&line[j] + 6, NULL, 10);
                    al_draw_tinted_scaled_bitmap(Ctrl.controls, al_map_rgba_f(0.5, 0.5, 0.5, 0.5),
                                                 0, idx * 200, 200, 200, 50*font_scale*j/6, i, Ctrl.ctrl[idx].size*0.8, Ctrl.ctrl[idx].size*0.8, 0);
                    //i += Ctrl.ctrl[idx].size;

                } else if (strncmp(&line[j], "[status]", 8) == 0) {
                    drawn = true;
                    i += line_space;
                    al_draw_scaled_bitmap(ui, 0, 0, 120, 80, 50*font_scale, i, 120*font_scale*2.0, 80*font_scale*2.0, 0);
                    i -= line_space*0.2;
                }
            }
            if (!drawn) {
                al_draw_text(small_font, al_map_rgb(240, 240, 240), (int) (50 * font_scale), i,
                             ALLEGRO_ALIGN_LEFT, line);
                i += line_space;
            }
        }
        Menu.max_scroll = -1*(i - (Ctrl.ctrl[SELECT].y - 4 * line_space));
        al_fclose(inst); //close file
    }
    al_set_target_backbuffer(display);			//Put default target back
    al_fprintf(logfile,"Done(%d).\n",i);
    al_fflush(logfile);
}

int component;
void draw_split_screen(ViewportType viewport, int ship_num)
{
    //ALLEGRO_TRANSFORM transform;
    //int i;
    int x=0, y=0, sbx=0, sby=0;
    int w, h;
	int scrollx, scrolly;	//These are the centre of the 'viewport' - normally the ship position, except when you get near an edge.

    if (halted) return;


    //int ctrl_size;
	//replace with split screen window size.
    w = al_get_display_width(display);
    h = al_get_display_height(display);

    //al_identity_transform(&transform);  		            /* Initialize transformation. */
    //al_scale_transform(&transform, scale, scale); /* Move to scroll position. */
    //al_translate_transform(&transform, w>>1, h>>1);         /* Move scroll position to screen center. */
    //al_use_transform(&transform);                           /* All subsequent drawing is transformed. */

    //use this here, also determined by passed in params
    switch (viewport)
    {
    	case FULL:
    		//x=STATUS_BAR_WIDTH;
    		//y=0;
    		//w=w-STATUS_BAR_WIDTH ;
    		//h=h;
    		break;
    	case TOP:
    		//x=STATUS_BAR_WIDTH;
    		//y=0;
    		//w=w-STATUS_BAR_WIDTH ;
    		h=h/2;
    		break;
    	case BOTTOM:
    		//x=STATUS_BAR_WIDTH;
    		y=h/2;
    		sby = y+2;
    		//w=w-STATUS_BAR_WIDTH ;
    		h=h/2 ;
    		break;
    	case TOPLEFT:
    		//x=STATUS_BAR_WIDTH;
    		//y=0;
    		//w=(w-STATUS_BAR_WIDTH)/2;
    		w=w/2;
    		h=h/2;
    		break;
    	case TOPRIGHT:
    		//x=STATUS_BAR_WIDTH + (w-STATUS_BAR_WIDTH)/2;
    		x=w/2;
    		sbx = x+2;
    		//y=0;
    		//w=(w-STATUS_BAR_WIDTH)/2;
    		w=w/2;
    		h=h/2;
    		break;
    	case BOTTOMLEFT:
    		//x=STATUS_BAR_WIDTH;
    		y=h/2;
    		sby=y+2;
    		//w=(w-STATUS_BAR_WIDTH)/2;
    		w=w/2;
    		h=h/2;
    		break;
    	case BOTTOMRIGHT:
    		//x=STATUS_BAR_WIDTH + (w-STATUS_BAR_WIDTH)/2;
    		x=w/2;
    		sbx = x+2;
    		y=h/2;
    		sby = y+2;
    		//w=(w-STATUS_BAR_WIDTH)/2;
    		w=w/2;
    		h=h/2;
    		break;
        default:
    		//x=STATUS_BAR_WIDTH;
    		//y=0;
    		//w=w-STATUS_BAR_WIDTH ;
    		//h=h;
    		break;
	}
	al_set_clipping_rectangle(x, y, w, h);

    float sw,sh;

    sw = w*invscale;
    sh = h*invscale;

	//stop scrolling when you get near to the edge.
	//scrollx is ship xpos, except saturated at half screen width
	if (mapx < (sw))	//for small maps....
		scrollx = (int)(0.5*sw);
	else if (Ship[ship_num].xpos < (int)(0.5*sw)) 						//if the ship is <0.5 viewport width from edge, stop following it.
		scrollx = (int)(0.5*sw);
	else if (Ship[ship_num].xpos > mapx-(int)(0.5*sw)) //likewise from far edge
		scrollx = mapx-(int)(0.5*sw);
	else
		scrollx = Ship[ship_num].xpos;

	if (Ship[ship_num].ypos < (int)(0.5*sh)) 						//...and for y
		scrolly = (int)(0.5*sh);
	else if (Ship[ship_num].ypos > mapy-(int)(0.5*sh))
		scrolly = mapy-(int)(0.5*sh);
	else
		scrolly = Ship[ship_num].ypos;

    //scrollx*=scale;
    //scrolly*=scale;

    //scale = 1.1;

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

	draw_status_bar(ship_num,sbx,sby);



	return;
}

void draw_radar(void)
{
    ALLEGRO_TRANSFORM transform;
    int i;

    int w = al_get_display_width(display);

	if (Radar.on)
    {
        al_identity_transform(&transform);
        al_scale_transform(&transform,scale,scale);
        al_use_transform(&transform);

        float radarx = w*invscale-Radar.width;

        al_draw_bitmap(Radar.display,radarx,0,0);   //tinted? alpha?
        //draw ships.
        for (i=0 ; i<num_ships ; i++)
        {
            if (Ship[i].reincarnate_timer == 0)
                al_draw_tinted_bitmap(bullets_bmp, Ship[i].colour,(radarx + Ship[i].xpos/16)-2, (Ship[i].ypos/16)-2,0);
        }
        al_identity_transform(&transform);
        al_use_transform(&transform);
    }
}

float soverm;

//scrollx, scrolly are saturated (clipped?) ship position
//win_x, win_y are top-left of viewport
//w,h are size of viewport
void draw_background(int scrollx, int scrolly, int win_x, int win_y, int w, int h)
{
	ALLEGRO_TRANSFORM transform;
	int x,y;
	int min_x, min_y, max_x, max_y;
    int bw,bh;
	ALLEGRO_COLOR fade;
	int i;
    float bscale = scale, invbscale = invscale;

	int bglayers = 2;

    int firstlayer;

    if (Map.mission)
        firstlayer = 0;
    else
        firstlayer = 1;

    for (i=firstlayer ; i<bglayers ; i++)
    {
        if (Map.background_fade)
        {
            soverm = (float)(scrolly-Map.bg_fade_thresh)/(float)mapy;

            soverm /= (2-i);  //3,1    factor, - (factor-1)*i

            if (soverm < 0) soverm = 0;

            fade = al_map_rgba_f(soverm,soverm,soverm,1.0);
        }
        else
            fade = al_map_rgba_f(1.0,1.0,1.0,1.0);


        int bscrollx = scrollx >> (bglayers-i);  //to make the background move half as much
        int bscrolly = scrolly >> (bglayers-i);

        if (Map.mission)
        {
            bscale = scale * (0.8 + 0.1*i);
            invbscale = 1/bscale;
        }

        bw = al_get_bitmap_width(background);
        bh = al_get_bitmap_height(background);

        al_identity_transform(&transform);  		            /* Initialize transformation. */
        al_translate_transform(&transform, -bscrollx, -bscrolly); /* Move to scroll position. */
        al_scale_transform(&transform,bscale,bscale);
        al_translate_transform(&transform, w>>1, h>>1);         /* Move scroll position to screen center. */
        al_use_transform(&transform);                           /* All subsequent drawing is transformed. */

        min_x = ((bscrollx - (w*invbscale/2) )/bw)-1;
        min_y = ((bscrolly - (h*invbscale/2) )/bh)-1;

        max_x = ((bscrollx + (w*invbscale/2) )/bw)+1;
        max_y = ((bscrolly + (h*invbscale/2) )/bh)+1;

        al_hold_bitmap_drawing(1);

        for (y = min_y; y < max_y; y++)
        {
            for (x = min_x; x < max_x; x++)
            {
                al_draw_tinted_bitmap(background, fade , win_x + x*bw,  win_y + y*bh, 0 );
            }
        }

        al_hold_bitmap_drawing(0);

        al_identity_transform(&transform);
        al_use_transform(&transform);
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
        al_scale_transform(&transform,scale,scale);
        al_translate_transform(&transform, w>>1, h>>1);         /* Move scroll position to screen center. */

        al_use_transform(&transform);                           /* All subsequent drawing is transformed. */

		min_x = (int)((scrollx - (w*invscale) )*1) >> TILE_SHIFTS;// /(tile_width);	//optimise by using shifts, rather than divides....
		min_y = (int)((scrolly - (h*invscale) )*1) >> TILE_SHIFTS;// /(tile_height);

		max_x = ((int)((scrollx + (w*invscale) )*1) >> TILE_SHIFTS )+1;// /(tile_width)) +1;
		max_y = ((int)((scrolly + (h*invscale) )*1) >> TILE_SHIFTS )+1;// /(tile_height)) +1;

		al_hold_bitmap_drawing(1);
		for (y = min_y; y < max_y; y++)
		{
			for (x = min_x; x < max_x; x++)
			{
				int i = tile_map[x + y * MAX_MAP_WIDTH];    //pull tile index From array
				//int u = i << 6;                             //multiply by 64 to get pixel index
				//int v=0;

				//int u = (i & 0x0007)<<6;    //bottom 3 bits * 64
				//int v = (i & 0xfff8)<<3;    //upper bits /8 then * 64
				int u = 1+(i & 0x0007)*66;    //bottom 3 bits * 66
				int v = 1+((i & 0xfff8)>>3)*66;    //upper bits /8 then * 66

                                                        //sx sy sw          sh           dx                        dy
				//if (i != 0)
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
        al_identity_transform(&transform);  		            /* Initialize transformation. */
        al_translate_transform(&transform, -scrollx, -scrolly); /* Move to scroll position. */
        al_scale_transform(&transform,scale,scale);
        al_translate_transform(&transform, w>>1, h>>1);         /* Move scroll position to screen center. */
        al_use_transform(&transform);                           /* All subsequent drawing is transformed. */

        //al_draw_scaled_bitmap(tr_map, (scrollx-w*0.5),(scrolly-h*0.5), w, h, win_x, win_y, w, h, 0); //src x,y,w,h dst x,y,w,h,flags
        al_draw_bitmap(tr_map,win_x, win_y,0); //src x,y,w,h dst x,y,w,h,flags

        al_identity_transform(&transform);  		            /* Initialize transformation. */
        al_use_transform(&transform);                           /* All subsequent drawing is transformed. */
    }
	else	//tr-style single image map, upscaled by 2
	{
    	//al_draw_scaled_bitmap(tr_map, (scrollx-w*0.5)/2,(scrolly-h*0.5)/2, w/2, h/2, win_x, win_y, w, h, 0); //src x,y,w,h dst x,y,w,h,flags

        al_identity_transform(&transform);  		            /* Initialize transformation. */

        al_translate_transform(&transform, -scrollx/2, -scrolly/2); /* Move to scroll position. */
        al_scale_transform(&transform,scale*2,scale*2);
        al_translate_transform(&transform, w>>1, h>>1);         /* Move scroll position to screen center. */

        al_translate_transform(&transform, -win_x, -win_y); /* Move to window position. */

        al_use_transform(&transform);                           /* All subsequent drawing is transformed. */

        al_draw_bitmap(tr_map,win_x, win_y,0); //src x,y,w,h dst x,y,w,h,flags

        al_identity_transform(&transform);  		            /* Initialize transformation. */
        al_use_transform(&transform);                           /* All subsequent drawing is transformed. */
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

    //static float tx_scale=1, ty_scale=1;

    //tx_scale = scale;//1-(scale-1)/5;
    //ty_scale = scale;//1+(scale-1)/5;

    /* Move to scroll position. */
    //I'm thinking of this like a camera - so we move the camera to where the ship is.
    //Actually, as the numbers are -ve, it seems more like the camera/viewport is fixed, and we're moving the world.
    //so we move everything (ship, bullets, tiles) to 0,0

    al_translate_transform(&transform, -scrollx, -scrolly);
    //al_translate_transform(&transform, )

    al_scale_transform(&transform,scale,scale);

    /* Move scroll position to screen center. */
    //Now we move it to centre of viewport/window thing
    //al_translate_transform(&transform, w * 0.5 + STATUS_BAR_WIDTH, h * 0.5);
    al_translate_transform(&transform, x + w*0.5 , y + h * 0.5);



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
					al_draw_tinted_bitmap_region(sentries,al_map_rgba(alpha, alpha, alpha, alpha), Map.forcefield[i].closed_sprite*66+1,0,   64, 64, x,   y, 0);
					x+=64;
					if (x+64>Map.forcefield[i].max_x)
					{
						//just draw part of the sprite, dummy!
						//al_draw_bitmap_region(sentries,Map.forcefield[i].alive_sprite*64,0,   64, 64, Map.forcefield[i].max_x-64,   y, 0);
						al_draw_tinted_bitmap_region(sentries, al_map_rgba(alpha, alpha, alpha, alpha), Map.forcefield[i].closed_sprite*66+1,0,   Map.forcefield[i].max_x-x, 64, x, y, 0);
						break;
					}
				}
			}
			else
			{
				while (y<Map.forcefield[i].max_y)
			 	{	                      //bmp    srcx                              srcy size    dstx dsty
					al_draw_tinted_bitmap_region(sentries, al_map_rgba(alpha, alpha, alpha, alpha), Map.forcefield[i].closed_sprite*66+1,0,   64, 64, x,   y, 0);
					y+=64;
					if (y+64>Map.forcefield[i].max_y)
					{
						//al_draw_bitmap_region(sentries,Map.forcefield[i].alive_sprite*64,0,   64, 64, x, Map.forcefield[i].max_y-64, 0);
						al_draw_tinted_bitmap_region(sentries, al_map_rgba(alpha, alpha, alpha, alpha), Map.forcefield[i].closed_sprite*66+1,0,   64, Map.forcefield[i].max_y-y, x, y, 0);
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
			al_draw_bitmap_region(sentries,1+Map.sentry[i].alive_sprite*66,0, 64, 64,Map.sentry[i].x-64/2,Map.sentry[i].y-64/2, 0);
		else
			al_draw_bitmap_region(sentries,1+Map.sentry[i].dead_sprite*66,0, 64, 64,Map.sentry[i].x-64/2,Map.sentry[i].y-64/2, 0);
	}

	for(i=0 ; i<Map.num_switches ; i++)
	{
		if (Map.switches[i].open)
			al_draw_bitmap_region(sentries,1+Map.switches[i].open_sprite*66,  0, 64, 64,Map.switches[i].x-64/2,Map.switches[i].y-64/2, 0);
		else
			al_draw_bitmap_region(sentries,1+Map.switches[i].closed_sprite*66,0, 64, 64,Map.switches[i].x-64/2,Map.switches[i].y-64/2, 0);
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
            //al_draw_bitmap(marker_bmp,Ship[i].xpos,Ship[i].ypos,0);

            int j;
            if (tracking && Ship[i].automode)
            {
                al_draw_bitmap(marker_bmp,Ship[i].xpos-10,Ship[i].ypos-10,0);

                find_obstructions(i);

                //red, walls
                for (j=0 ; j<NUM_ANGLES ; j++)
                {
                    //al_draw_line(Ship[i].xpos,Ship[i].ypos,walls[j]*sinlut[j]*5,walls[j]*coslut[j]*5,al_map_rgba(128,0,0,128),3);
                    int x = Ship[i].xpos-10+walls[j].distance*sinlut[j];
                    int y = Ship[i].ypos-10-walls[j].distance*coslut[j];
                    al_draw_bitmap(marker_bmp,x,y,0);
                    //al_draw_filled_circle(x,y,10,al_map_rgb(255,255,255));
                }
                //al_draw_tinted_bitmap(marker2_bmp,al_map_rgb(0,255,0),Ship[i].xpos-10+100*sinlut[target_angle],Ship[i].ypos-10-100*coslut[target_angle],0);
                //al_draw_tinted_bitmap(marker2_bmp,al_map_rgb(0,0,255),Ship[i].xpos-10+100*sinlut[avoid_angle],Ship[i].ypos-10-100*coslut[avoid_angle],0);

                //green, target vector (vector sum algorithm)
                al_draw_tinted_bitmap(marker2_bmp,al_map_rgb(0,255,0),Ship[i].xpos-10+1*targetx,Ship[i].ypos-10-1*targety,0);
                //blue, avoid vector (vector sum algorithm)
                al_draw_tinted_bitmap(marker2_bmp,al_map_rgb(0,0,255),Ship[i].xpos-10+1*avoidx,Ship[i].ypos-10-1*avoidy,0);

                //white, ship
                al_draw_bitmap(marker2_bmp,Ship[i].xpos-10+1*sumx,Ship[i].ypos-10-1*sumy,0);
                //yellow, autotarget
                al_draw_tinted_bitmap(marker2_bmp,al_map_rgb(255,255,0),Ship[i].autotargetx-10,Ship[i].autotargety-10,0);
                //magenta, radial g vector
                al_draw_tinted_bitmap(marker2_bmp,al_map_rgb(255,0,255),Ship[i].xpos-1000*Ship[i].xg,Ship[i].ypos-1000*Ship[i].yg,0);
            }
		}

	}

    al_hold_bitmap_drawing(0);

    al_identity_transform(&transform);
    al_use_transform(&transform);
}

//ship config menu (i.e. weapons, ammo, fuel)
void draw_menu(int ship_num, int x, int y, int w, int h)
{
	ALLEGRO_TRANSFORM transform;
    int j;
	int bmw,bmh;

	bmw = al_get_bitmap_width(panel_bmp);
	bmh = al_get_bitmap_height(panel_bmp);

#ifdef ANDROID
 /*
    float xscale,yscale,menuscale;
    xscale = (float)w/bmw;
    yscale = (float)h/bmh;

    if (xscale > yscale)
        menuscale = yscale;
    else
        menuscale = xscale;
*/
    al_identity_transform(&transform);
    al_scale_transform(&transform,0.8*Scale.menuscale,0.8*Scale.menuscale);
    al_translate_transform(&transform, 0.1*w,0.1*h);
    al_use_transform(&transform);

#else
        x+=(w-bmw)/2;
        y+=(h-bmh)/2;
#endif
		al_draw_bitmap(panel_bmp,x, y,0);
#ifndef ANDROID
		//selection bar
		al_draw_filled_rectangle(x+430,y+45+60*Ship[ship_num].menu_state,x+550,y+75+60*Ship[ship_num].menu_state,al_map_rgba(32, 0, 0, 20));
#endif
		//currently pressed buttons
		al_draw_bitmap_region(panel_pressed_bmp,20+Ship[ship_num].ammo1_type*110,70,80,80,x+20+Ship[ship_num].ammo1_type*110,y+70,0); //src x,y,w,h dst x,y,flags
		al_draw_bitmap_region(panel_pressed_bmp,20+(Ship[ship_num].ammo2_type-4)*110,200,80,80,x+20+(Ship[ship_num].ammo2_type-4)*110,y+200,0); //src x,y,w,h dst x,y,flags

		//ammo1 bar
		al_draw_filled_rectangle(x+35,y+55,x+35+Ship[ship_num].user_ammo1*3.8/2,y+65,al_map_rgb(128, 0, 0));				//ammo1
		al_draw_filled_rectangle(x+35,y+57,x+35+Ship[ship_num].user_ammo1*3.8/2,y+59,al_map_rgba(255, 255, 255, 20));

		//ammo2 blobs
		for (j=0 ; j<Ship[ship_num].user_ammo2 ; j++)
		{
			al_draw_filled_rounded_rectangle(x+35+j*47,y+175,x+35+j*47+37,y+185,4,4,al_map_rgb(0, 128, 128));
			al_draw_filled_rectangle        (x+37+j*47,y+177,x+35+j*47+35,y+179,al_map_rgba(255, 255, 255, 20));
		}

		//fuel bar
		al_draw_filled_rectangle(x+35,y+295,x+35+Ship[ship_num].user_fuel*3.8,y+305,al_map_rgb(128, 128, 0));				//fuel
		al_draw_filled_rectangle(x+35,y+297,x+35+Ship[ship_num].user_fuel*3.8,y+299,al_map_rgba(255, 255, 255, 20));

    al_identity_transform(&transform);
    al_use_transform(&transform);

		return;
}

//void draw_status_bar(int num_ships)
void draw_status_bar(int ship, int x, int y)
{
	ALLEGRO_TRANSFORM transform;
    int i,j;
	//int w,h,yoffset;
	int bs;	//size of coloured block;
	//int bmh;//height of background bitmap
	//int first_ship;

    al_identity_transform(&transform);
    al_scale_transform(&transform,scale,scale);
    al_use_transform(&transform);

	//al_set_clipping_rectangle(0, 0, SCREENX, SCREENY);
	//al_set_clipping_rectangle(0, 0, w, h);

	//bmh = al_get_bitmap_height(status_bg);

	//yoffset = (h-STATUS_BAR_HEIGHT)/2;
	//yoffset = 0;

	//Status info
	//for (i=0 ; i<h ; i+= bmh)
    //    al_draw_bitmap(status_bg,0,i,0);

	if (Map.mission)
		bs = 122;                       //extra space for time, miners, jewels
	else if (Ship[ship].lap_complete)
        bs = 108;                       //extra space for current + finished times
	else if (Map.race)
	    bs = 90;                        //extra space for current time only
	else
		bs=72;                          //minimum space; shield, ammo x2, fuel, lives

	//yoffset += 0;//5;
/*
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
*/
    i = ship;

	//for (i=first_ship ; i<num_ships ; i++)
	{
		#ifdef ANDROID   //ANDROID - 2 rows, 5 bars on each row:
                        //          shield ; ammo     ; smmo     ; fuel   ; lives
                        //          time   ; last lap ; best lap ; miners ; jewels
		int c = invscale *(al_get_display_width(display)/2);    //centre
		int bw = 102;                               //bar width

		x = c - 2.5*bw;

		al_draw_filled_rounded_rectangle(x,y,x+5*bw,y+28,5,5,al_map_rgba(0, 0, 0, 192));

		al_draw_bitmap_region(ui,100-Ship[i].shield,    0,  Ship[i].shield,       14, x+0*bw, y, 0);
        al_draw_bitmap_region(ui,100-Ship[i].ammo1/2,   14, Ship[i].ammo1,        14, x+1*bw, y, 0);
        al_draw_bitmap_region(ui,0                 ,    28, (Ship[i].ammo2*25)/2, 14, x+2*bw, y, 0);
        al_draw_bitmap_region(ui,100-(Ship[i].fuel>>4), 42, (Ship[i].fuel>>4),    14, x+3*bw, y, 0);

        al_draw_bitmap_region(ui,0, 56, Ship[i].lives*17, 14, x+4*bw,y,0);

		if (Map.mission)
		{
			for (j=0 ; j<Ship[i].miners ; j++)
				al_draw_bitmap_region(pickups,16,0,16,16,x+3*bw+j*12+1,y+14,0);
			for (j=0 ; j<Ship[i].jewels ; j++)
				al_draw_bitmap_region(pickups,0,0,16,16,x+4*bw+j*12+1,y+14,0);
		}

		if (Ship[i].racing)
			al_draw_textf(race_font, al_map_rgb(255, 255, 255),x+0*bw, y+14, ALLEGRO_ALIGN_LEFT, "%0.3f", Ship[i].current_lap_time);
		if (Ship[i].lap_complete)
        {
			al_draw_textf(race_font, al_map_rgb(255, 255,   0),x+1*bw,  y+14, ALLEGRO_ALIGN_LEFT, "%0.3f", Ship[i].last_lap_time);
			al_draw_textf(race_font, al_map_rgb(255, 128,   0),x+2*bw, y+14, ALLEGRO_ALIGN_LEFT, "%0.3f", Ship[i].best_lap_time);
        }

		#else
		al_draw_filled_rounded_rectangle(x,y,x+102,y+bs,5,5,al_map_rgba(0, 0, 0, 128));

		al_draw_bitmap_region(ui,100-Ship[i].shield,    0,  Ship[i].shield,       14, x+0, y+0, 0);
        al_draw_bitmap_region(ui,100-Ship[i].ammo1/2,   14, Ship[i].ammo1,        14, x+0, y+14, 0);
        al_draw_bitmap_region(ui,0                 ,    28, (Ship[i].ammo2*25)/2, 14, x+0, y+28, 0);
        al_draw_bitmap_region(ui,100-(Ship[i].fuel>>4), 42, (Ship[i].fuel>>4),    14, x+0, y+42, 0);

        al_draw_bitmap_region(ui,0, 56, Ship[i].lives*17, 14, x+0,y+56,0);

		if (Map.mission)
		{
			for (j=0 ; j<Ship[i].miners ; j++)
				al_draw_bitmap_region(pickups,16,0,16,16,0+j*12+1,70,0);
			for (j=0 ; j<Ship[i].jewels ; j++)
				al_draw_bitmap_region(pickups,0,0,16,16,0+j*12+1,86,0);
		}

		if (Ship[i].racing)
			al_draw_textf(race_font, al_map_rgb(255, 255, 255),5, y+bs-20, ALLEGRO_ALIGN_LEFT, "%0.3f", Ship[i].current_lap_time);
		if (Ship[i].lap_complete)
        {
			al_draw_textf(race_font, al_map_rgb(255, 255,   0),5,  y+bs-38, ALLEGRO_ALIGN_LEFT, "%0.3f", Ship[i].last_lap_time);
			al_draw_textf(race_font, al_map_rgb(255, 128,   0),50, y+bs-38, ALLEGRO_ALIGN_LEFT, "%0.3f", Ship[i].best_lap_time);
        }
        #endif // 1
	}

    al_identity_transform(&transform);
    al_use_transform(&transform);
}
#ifdef ANDROID
void draw_controls(ALLEGRO_COLOR tint)
{
    int i;

    if (halted) return;

    for (i=0 ; i<NO_BUTTON ; i++)
        if (Ctrl.ctrl[i].active)
            al_draw_tinted_scaled_bitmap(Ctrl.controls, tint, Ctrl.ctrl[i].idx*200, i*200, 200,200, Ctrl.ctrl[i].x, Ctrl.ctrl[i].y, Ctrl.ctrl[i].size, Ctrl.ctrl[i].size, 0);
    return;
}
#else
void draw_controls(ALLEGRO_COLOR tint)
{
    return;
}
#endif // ANDROID
void draw_dividers(int ships)
{
    int w,h;

    if (halted) return;

    w = al_get_display_width(display);
    h = al_get_display_height(display);

    al_set_clipping_rectangle(0, 0, w, h);

	//dividers
	//if (!Net.net)
	//if (!Net.client && !Net.server)
    {
        if (ships > 1)
            //al_draw_filled_rectangle(STATUS_BAR_WIDTH,SCREENY/2-5,SCREENX,SCREENY/2+5,al_map_rgb(128, 128, 128));
            //al_draw_filled_rectangle(STATUS_BAR_WIDTH,h/2-2,w,h/2+2,al_map_rgb(128, 128, 128));
            al_draw_filled_rectangle(0,(h/2)-2,w,(h/2)+2,al_map_rgb(128, 128, 128));
        if (ships > 2)
            //al_draw_filled_rectangle(((w-STATUS_BAR_WIDTH)/2)+STATUS_BAR_WIDTH-2,0,(w-STATUS_BAR_WIDTH)/2+STATUS_BAR_WIDTH+2,h,al_map_rgb(128, 128, 128));
            al_draw_filled_rectangle((w/2)-2,0,(w/2)+2,h,al_map_rgb(128, 128, 128));
    }

	return;
}

