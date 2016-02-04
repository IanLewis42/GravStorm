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

#include "allegro5/allegro.h"
#include "allegro5/allegro_image.h"
#include "allegro5/allegro_primitives.h"
#include "allegro5/allegro_font.h"
#include "allegro5/allegro_audio.h"

#include "game.h"
#include "drawing.h"
#include "objects.h"
#include "inputs.h"


 /* Our tiles atlas. */
 ALLEGRO_BITMAP *tiles; //not used currently
 //ALLEGRO_BITMAP *bullets_bmp;
 //ALLEGRO_BITMAP *tr_map;
 //ALLEGRO_BITMAP *tr_col_map;//Collision map. Might not need this to be an allegro bitmap, just a made-up binary format.
 //ALLEGRO_BITMAP *tr_map_section;


/* Our tilemap. */
#define MAX_MAP_WIDTH 100
#define MAX_MAP_HEIGHT 100
int tile_map[MAX_MAP_WIDTH * MAX_MAP_HEIGHT];

int map_height=0, map_width=0;

void draw_map(int scrollx, int scrolly, int x, int y, int w, int h);
void draw_ships(int scrollx, int scrolly, int x, int y, int w, int h);
void draw_menu(int ship_num, int x, int y, int w, int h);

#define LINE_SPACE 55
#define SMALL_LINE_SPACE 40

int grid = 0;

void display_menu(void)//int num_maps, int selected)	//show list of maps
{
	ALLEGRO_TRANSFORM transform;
	int i,j,y=-10;
	float scale;
	char keys[]  = "Keys";
	char gpio[]  = "GPIO Joy";
	char usb0[]  = "USB Joy 1";
	char usb1[]  = "USB Joy 2";
	char na[]    = "N/A";
	char* control_string;
	static int temp = 0,temp2=0;

	ALLEGRO_COLOR colour;
	al_clear_to_color(al_map_rgb(0, 0, 0));

	al_draw_bitmap(menu_bg_bmp,0,0,0);

	/*
	temp++;
	if (temp == 30)
	{
		temp = 0;
		temp2++;
	}
	al_draw_textf(title_font, al_map_rgba(0, 0, 0,128),10, 10,  ALLEGRO_ALIGN_LEFT, "%i", temp2);
	*/

	scale = 0.4;
	al_identity_transform(&transform);			/* Initialize transformation. */
	al_scale_transform(&transform, scale, scale);	/* Rotate and scale around the center first. */
	al_translate_transform(&transform,SCREENX/2,y);
	al_use_transform(&transform);
	al_draw_textf(title_font, al_map_rgba(0, 0, 0,128),0, (al_get_font_ascent(title_font)/2)*scale,  ALLEGRO_ALIGN_CENTRE, "%s", NAME);

	al_identity_transform(&transform);
	al_scale_transform(&transform, scale, scale);	/* Rotate and scale around the center first. */
	al_translate_transform(&transform,SCREENX/2-7,y+7);
	al_use_transform(&transform);
	al_draw_textf(title_font, al_map_rgb(128, 128, 0),0, (al_get_font_ascent(title_font)/2)*scale,  ALLEGRO_ALIGN_CENTRE, "%s", NAME);

	al_identity_transform(&transform);
	al_use_transform(&transform);

	y+= 25;


	//Display maps; display all group names, and maps in current group
	for (i=0 ; i<Menu.num_groups ; i++)
	{

		if (i == Menu.group)
        {
			colour = al_map_rgba(0, 255, 0, 20);
			al_draw_textf(menu_font, colour ,Menu.offset+20, y+=LINE_SPACE,  ALLEGRO_ALIGN_LEFT, "%s", &MapNames[i].Group);

            for (j=0 ; j<MapNames[Menu.group].Count ; j++)
            {
        		if (j == Menu.map)
        		{
        	    	if (Menu.col == 0 )
        	    	    colour = al_map_rgba(255, 255, 0, 20);   //so yellow if we're in col 0, i.e. changing this
        	    	else
        	    	    colour = al_map_rgba(128, 0, 0, 20);     //red if another col, so you can see it's the current one
				}
        		 else
					colour = al_map_rgba(255, 255, 255, 20);

        		al_draw_textf(menu_font, colour ,Menu.offset+30, y+=Menu.expand,  ALLEGRO_ALIGN_LEFT, "%s", &MapNames[i].Map[j]);
			}
        }
        else
		{
			colour = al_map_rgba(0, 48, 0, 20);
			al_draw_textf(menu_font, colour ,Menu.offset+20, y+=LINE_SPACE,  ALLEGRO_ALIGN_LEFT, "%s", &MapNames[i].Group);
		}
	}

	//al_draw_textf(menu_font, colour ,Menu.offset+20, y+=LINE_SPACE,  ALLEGRO_ALIGN_LEFT, "%d %d", Menu.group, Menu.map);

	//al_draw_textf(menu_font, colour ,20, 20+i*70,  ALLEGRO_ALIGN_LEFT, "%d", Menu.col);

	//colour = al_map_rgb(255, 255, 255);
	//al_draw_textf(menu_font, colour ,Menu.offset+480, 20,  ALLEGRO_ALIGN_LEFT, "PLAYERS");

	y=80;

	for (i=0 ; i<4 ; i++)			//List players
	{
		if (i >= Map.max_players)                   //can never select this, so grey
			colour = al_map_rgb(64, 64, 64);

        else if (i == Menu.player)                  //selected player
        {
            if(Menu.col == 1)
                colour = al_map_rgb(255, 255, 0);   //so yellow if we're in col 1, i.e. changing this
            else
                colour = al_map_rgb(128, 0, 0);     //red if another col, so you can see it's the current one
        }
		else
			colour = al_map_rgb(255, 255, 255);

		al_draw_textf(menu_font, colour,Menu.offset+470, y+i*LINE_SPACE,  ALLEGRO_ALIGN_LEFT, "Player %d",i+1);
	}


	for (i=0 ; i<4 ; i++)			//List controls
	{
		if (Ship[i].controller == KEYS)
			control_string = keys;
		if (Ship[i].controller == GPIO_JOYSTICK)
			control_string = gpio;
		if (Ship[i].controller == USB_JOYSTICK0)
			control_string = usb0;
		if (Ship[i].controller == USB_JOYSTICK1)
			control_string = usb1;
		if (Ship[i].controller == NA)
			control_string = na;

		//if (i >= Map.max_players)
		if (i != Menu.player)
			colour = al_map_rgb(64, 64, 64);
		else if (i == Menu.player)
        {
            if (Menu.col == 2)
                colour = al_map_rgb(255, 255, 0);
            else
                colour = al_map_rgb(128, 0, 0);
        }
        else
			colour = al_map_rgb(255, 255, 255);

		al_draw_textf(menu_font, colour,Menu.offset+940, y+i*LINE_SPACE,  ALLEGRO_ALIGN_LEFT, "%s",control_string);		//Control method for selected player
	}

    colour = al_map_rgb(255, 255, 255);
    al_draw_textf(small_font, colour, Menu.offset+940, (i+2)*LINE_SPACE,  ALLEGRO_ALIGN_LEFT, "%d Active Players",num_ships);


	if (Ship[Menu.player].controller == KEYS)
	{
        al_draw_textf(small_font, al_map_rgb(255, 255, 255),Menu.offset+1400, y+SMALL_LINE_SPACE*0,  ALLEGRO_ALIGN_LEFT, "Rotate Left :");
        if (Menu.col == 3 && Menu.current_key == 0) colour = al_map_rgb(255, 255, 0);
		else colour = al_map_rgb(255, 255, 255);
        al_draw_textf(small_font, colour,Menu.offset+1400, y+SMALL_LINE_SPACE*1,  ALLEGRO_ALIGN_LEFT, " %s",al_keycode_to_name(Ship[Menu.player].left_key));

        al_draw_textf(small_font, al_map_rgb(255, 255, 255),Menu.offset+1400, y+SMALL_LINE_SPACE*2,  ALLEGRO_ALIGN_LEFT, "Rotate Right :");
		if (Menu.col == 3 && Menu.current_key == 1) colour = al_map_rgb(255, 255, 0);
		else colour = al_map_rgb(255, 255, 255);
        al_draw_textf(small_font, colour,Menu.offset+1400, y+SMALL_LINE_SPACE*3,  ALLEGRO_ALIGN_LEFT, " %s",al_keycode_to_name(Ship[Menu.player].right_key));

        al_draw_textf(small_font, al_map_rgb(255, 255, 255),Menu.offset+1400, y+SMALL_LINE_SPACE*4,  ALLEGRO_ALIGN_LEFT, "Fire1 :");
		if (Menu.col == 3 && Menu.current_key == 2) colour = al_map_rgb(255, 255, 0);
		else colour = al_map_rgb(255, 255, 255);
        al_draw_textf(small_font, colour,Menu.offset+1400, y+SMALL_LINE_SPACE*5,  ALLEGRO_ALIGN_LEFT, " %s",al_keycode_to_name(Ship[Menu.player].up_key));

        al_draw_textf(small_font, al_map_rgb(255, 255, 255),Menu.offset+1400, y+SMALL_LINE_SPACE*6,  ALLEGRO_ALIGN_LEFT, "Fire2 :");
		if (Menu.col == 3 && Menu.current_key == 3) colour = al_map_rgb(255, 255, 0);
		else colour = al_map_rgb(255, 255, 255);
        al_draw_textf(small_font, colour,Menu.offset+1400, y+SMALL_LINE_SPACE*7,  ALLEGRO_ALIGN_LEFT, " %s",al_keycode_to_name(Ship[Menu.player].down_key));

        al_draw_textf(small_font, al_map_rgb(255, 255, 255),Menu.offset+1400, y+SMALL_LINE_SPACE*8,  ALLEGRO_ALIGN_LEFT, "Thrust :");
		if (Menu.col == 3 && Menu.current_key == 4) colour = al_map_rgb(255, 255, 0);
		else colour = al_map_rgb(255, 255, 255);
        al_draw_textf(small_font, colour,Menu.offset+1400, y+SMALL_LINE_SPACE*9,  ALLEGRO_ALIGN_LEFT, " %s",al_keycode_to_name(Ship[Menu.player].thrust_key));
	}

	al_flip_display();

	return;
}

void display_map_text(int done, int timer)
{
    FILE * description_file;
    char line[200];
    int i=20;
    ALLEGRO_USTR *ustr;

    al_clear_to_color(al_map_rgb(0, 0, 0));

    al_draw_bitmap(menu_bg_bmp,0,0,0);

	al_draw_filled_rectangle(0,0,SCREENX,SCREENY,al_map_rgba(0,0,0,timer*6));	//black filter to darken?

	if (description_file = fopen(Map.description_file_name,"r"))
	{
		while (fgets(line, 200, description_file) != NULL)
		{
			 ustr = al_ustr_new(line);

			//al_draw_textf(small_font, al_map_rgb(timer*8, timer*8, timer*8),0, i,  ALLEGRO_ALIGN_LEFT, "%s",line);
			al_draw_ustr(small_font, al_map_rgb(timer*8, timer*8, timer*8),0, i,  ALLEGRO_ALIGN_LEFT, ustr);
			i+=35;
		}

		al_ustr_free(ustr);
		fclose(description_file); //close file
	}
	else
	{
		al_draw_textf(small_font, al_map_rgb(timer*8, timer*8, timer*8),0, i,  ALLEGRO_ALIGN_LEFT, "%s",&MapNames[Menu.group].Map[Menu.map]);
		i+=35;
		al_draw_textf(small_font, al_map_rgb(timer*8, timer*8, timer*8),0, i,  ALLEGRO_ALIGN_LEFT, "Couldn't open description file: %s",Map.description_file_name);
	}

    if (done)
        al_draw_textf(small_font, al_map_rgb(128, 0, 0),0, i+35,  ALLEGRO_ALIGN_LEFT, "Painna Nappa / Press Button");

    al_flip_display();

    return;
}

/* Places a single tile into the tile atlas.
 * Normally you would load the tiles from a file.
 */
void tile_draw(int i, float x, float y, float w, float h) {
    ALLEGRO_COLOR black = al_map_rgb(0, 0, 0);
    ALLEGRO_COLOR yellow = al_map_rgb(255, 255, 0);
    ALLEGRO_COLOR red = al_map_rgb(255, 0, 0);
    switch (i) {
        case 0:
            al_draw_filled_rectangle(x, y, x + w, y + h, black);
            break;
        case 1:
            al_draw_filled_rectangle(x, y, x + w, y + h, red);
            al_draw_filled_circle(x + w * 0.5, y + h * 0.5, w * 0.475,yellow);
            break;
        case 2:
            al_draw_filled_rectangle(x, y, x + w, y + h, yellow);
            al_draw_filled_triangle(x + w * 0.5, y + h * 0.125,
                x + w * 0.125, y + h * 0.875,
                x + w * 0.875, y + h * 0.875, red);
            break;
        case 3:
            al_draw_filled_rectangle(x, y, x + w, y + h, black);
            //if (icon)
            //    al_draw_scaled_bitmap(icon, 0, 0, 48, 48,
            //        x, y, w, h, 0);
            break;
    }
}

/* Creates the tiles and a random 100x100 map. */
void tile_map_create(void) {
    int i;
    int x, y;

    /* Create the tile atlas. */
    //This is a bitmap with all the tile types in. Would normally be created elsewhere and loaded in.
    fprintf(logfile,"Creating tiles bitmap\n");
    tiles = al_create_bitmap(1024, 1024);			//create a bitmap
    al_set_target_bitmap(tiles);					//set it as the default target for all al_draw_ operations
    al_clear_to_color(al_map_rgba(0, 0, 0, 0));		//paint it balck.

    for (i = 0; i < 4; i++)						// We draw the tiles a bit bigger (66x66 instead of 64x64)
    {											// to account for the linear filtering. Normally just leaving
        tile_draw(i, i * 66, 0, 66, 66);		// the border transparent for sprites or repeating the border
    }											// for tiling tiles should work well.
    //fprintf(logfile,"Creating bullets bitmap\n");
    //bullets_bmp = al_create_bitmap(10, 10);			//create a bitmap
	//al_set_target_bitmap(bullets_bmp);					//set it as the default target for all al_draw_ operations
    //al_draw_filled_circle(2, 2, 2, al_map_rgb(255, 255, 255));


    al_set_target_backbuffer(display);			//Put default target back

    /* Create the random map. */
    //for (y = 0; y < 100; y++) {
    //    for (x = 0; x < 100; x++) {
    //        tile_map[x + y * 100] = rand() % 4;
    //    }
    //}

    //tiles at borders
    //load from file for real map....
    for (x=0 ; x<100 ; x++)
    {
    	tile_map[x + 0  * 100] = 1;
    	tile_map[x + 99 * 100] = 1;
	}
    for (y=0 ; y<100 ; y++)
    {
    	tile_map[0  + y * 100] = 1;
    	tile_map[99 + y * 100] = 1;
	}


 	//mapx = tile size * x tiles
 	//mapy...

    /* Center of map. */
    //scroll_x = 100 * 32 / 2;
    //scroll_y = 100 * 32 / 2;
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
//void tile_map_draw(void) {
    int x, y;
    int min_x, min_y;
    ALLEGRO_TRANSFORM transform;
    int w, h;
    int i,j;

	int scrollx, scrolly;	//These are the centre of the 'viewport' - normally the ship position, except when you get near an edge.

	ALLEGRO_COLOR bullet_colour;
	//int component;
	unsigned char r, g, b;

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
		draw_ships(scrollx, scrolly,x,y,w,h);
		draw_map(scrollx, scrolly,x,y,w,h);
	}
	else
	{
		draw_map(scrollx, scrolly,x,y,w,h);
		draw_ships(scrollx, scrolly,x,y,w,h);
	}

	return;
}

void draw_map(int scrollx, int scrolly, int win_x, int win_y, int w, int h)
{
	ALLEGRO_TRANSFORM transform;
	ALLEGRO_COLOR grid_colour;
	int x,y;
	int min_x, min_y, max_x, max_y;
	int i;

	if (Map.type)	//1 for tiled
	{
		/* Initialize transformation. */
		al_identity_transform(&transform);
		/* Move to scroll position. */
		al_translate_transform(&transform, -scrollx, -scrolly);
		/* Rotate and scale around the center first. */
		//al_rotate_transform(&transform, rotate);
		//al_scale_transform(&transform, zoom, zoom);
		/* Move scroll position to screen center. */
		al_translate_transform(&transform, w * 0.5, h * 0.5);
		/* All subsequent drawing is transformed. */
		al_use_transform(&transform);


		//x had status bar width info; or window positioning, we reused it as a loop count. sort it out!!!!

		min_x = (scrollx - 0.5*w)/(tile_width);	//optimise by using shifts, rather than divides....
		min_y = (scrolly - 0.5*h)/(tile_height);

		max_x = ((scrollx + 0.5*w)/(tile_width)) +1;
		max_y = ((scrolly + 0.5*h)/(tile_height)) +1;

		al_hold_bitmap_drawing(1);
		for (y = min_y; y < max_y; y++)
		{
			for (x = min_x; x < max_x; x++)
			{
				int i = tile_map[x + y * MAX_MAP_WIDTH];
				float u = i * tile_width;
				float v = 0;
										   //sx  sy sw          sh           dx                    dy                  dw  dh
				//al_draw_scaled_bitmap(tiles, u,  v, 64, 64, x * 32, y * 32, 32, 32, 0);
				//al_draw_bitmap_region(tr_map, u, v, tile_width, tile_height, win_x + x*tile_width, win_y + y*tile_height,0);
				al_draw_scaled_bitmap(tr_map, u, v, tile_width, tile_height, win_x + x*tile_width, win_y + y*tile_height, tile_width, tile_height,0);
				//al_draw_scaled_bitmap(tiles, u, v, tile_width, tile_height, x*tile_width, y*tile_height, tile_width+2, tile_height+2,0);
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
				al_draw_filled_rectangle(i*tile_width+win_x,0,i*tile_width+win_x+1,map_height*tile_height,grid_colour);
			}
			for (i=0 ; i<map_height ; i++)
			{
				al_draw_filled_rectangle(0,i*tile_height+win_y,map_width*tile_width,i*tile_height+win_y+1,grid_colour);
			}

		}

		al_identity_transform(&transform);
    	al_use_transform(&transform);
	}
	else	//tr-style single image map
	{
    	al_draw_scaled_bitmap(tr_map, (scrollx-w*0.5)/2,(scrolly-h*0.5)/2, w/2, h/2, win_x, win_y, w, h, 0); //src x,y,w,h dst x,y,w,h,flags
	}
}

//also does bullets and sentries
void draw_ships(int scrollx, int scrolly, int x, int y, int w, int h)
{
	ALLEGRO_TRANSFORM transform;
	int i;
	int miner_x, jewel_x;
	unsigned char r, g, b;

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
	int current_bullet, previous_bullet;

	current_bullet = first_bullet;
	previous_bullet = END_OF_LIST;

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

				if (Bullet[current_bullet].ttl&0x1f < 0x10)
					r -= 16;
				else
					r += 16;
				Bullet[current_bullet].colour = al_map_rgb(r,r,0);
			break;
			default:
			break;
		}

		al_draw_tinted_bitmap(bullets_bmp,Bullet[current_bullet].colour,Bullet[current_bullet].xpos, Bullet[current_bullet].ypos,0);
		current_bullet = Bullet[current_bullet].next_bullet;
	}

	//then sentries
	for(i=0 ; i<Map.num_sentries ; i++)
	{
		if (Map.sentry[i].alive)
			al_draw_bitmap_region(sentries,Map.sentry[i].alive_sprite*64,0, 64, 64,Map.sentry[i].x-64/2,Map.sentry[i].y-64/2, 0);
		else
			al_draw_bitmap_region(sentries,Map.sentry[i].dead_sprite*64,0, 64, 64,Map.sentry[i].x-64/2,Map.sentry[i].y-64/2, 0);
	}

	//forcefields
	for(i=0 ; i<Map.num_forcefields ; i++)
	{
		if (Map.sentry[Map.forcefield[i].sentry].alive)
		{
			x=Map.forcefield[i].min_x;
			y=Map.forcefield[i].min_y;

			if (x == Map.forcefield[i].half_x )	//horizontal - see init code
			{
				while (x<Map.forcefield[i].max_x)
			 	{	                      //bmp    srcx                              srcy size    dstx dsty
					al_draw_bitmap_region(sentries,Map.forcefield[i].alive_sprite*64,0,   64, 64, x,   y, 0);
					x+=64;
					if (x+64>Map.forcefield[i].max_x)
					{
						//just draw part of the sprite, dummy!
						//al_draw_bitmap_region(sentries,Map.forcefield[i].alive_sprite*64,0,   64, 64, Map.forcefield[i].max_x-64,   y, 0);
						al_draw_bitmap_region(sentries,Map.forcefield[i].alive_sprite*64,0,   Map.forcefield[i].max_x-x, 64, x, y, 0);
						break;
					}
				}
			}
			else
			{
				while (y<Map.forcefield[i].max_y)
			 	{	                      //bmp    srcx                              srcy size    dstx dsty
					al_draw_bitmap_region(sentries,Map.forcefield[i].alive_sprite*64,0,   64, 64, x,   y, 0);
					y+=64;
					if (y+64>Map.forcefield[i].max_y)
					{
						//al_draw_bitmap_region(sentries,Map.forcefield[i].alive_sprite*64,0,   64, 64, x, Map.forcefield[i].max_y-64, 0);
						al_draw_bitmap_region(sentries,Map.forcefield[i].alive_sprite*64,0,   64, Map.forcefield[i].max_y-y, x, y, 0);
						break;
					}
				}
			}
		}
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
					}
					//                    bmp    srcx                      srcy size      dstx    dsty
					al_draw_bitmap_region(miner,(Ship[i].miner_count%8)*24,0,   24,  24,  miner_x,Map.pad[Ship[i].pad].y-13,0);
					//free miner bmp
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
					}
					//                    bmp    srcx                      srcy size      dstx    dsty
					al_draw_bitmap_region(jewel,(Ship[i].jewel_count%16)*24,0,   24,  24,  jewel_x,Map.pad[Ship[i].pad].y-17,0);
					//free  bmp
				}

			}
			//ship
			al_draw_bitmap_region(ships,Ship[i].angle*SHIP_SIZE_X,(2*i + (Ship[i].thrust?1:0) )*SHIP_SIZE_Y, SHIP_SIZE_X, SHIP_SIZE_Y,Ship[i].xpos-SHIP_SIZE_X/2,Ship[i].ypos-SHIP_SIZE_Y/2, 0);
		}
	}

    al_hold_bitmap_drawing(0);

    al_identity_transform(&transform);
    al_use_transform(&transform);
}

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


#if 0
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
    //al_use_transform(&transform);

 #if 0
    al_clear_to_color(al_map_rgb(0, 0, 0));
    al_hold_bitmap_drawing(1);

	//only draw the bit of the map on the screen/viewport
	//Draw an extra row/column, but make sure we don't draw outside the buffer.
	min_y = (scrolly-h*0.5)/TILE_SIZE_Y;	//so if the viewport edge (cente - half height(or width) is more than 1 tile from the 'zero' edge,
	if (min_y > 0)							//  draw an extra row(/col) of tiles. As you approach the edge, the last tile will be drawn
		min_y--;							//  before you get there, and scroll in, then stop :-) The >0 is required to stop us drawing outside the backbuffer.
	min_x = (scrollx-w*0.5)/TILE_SIZE_X;
	if (min_x > 0)							//This check may not be needed for X, as we have the statusbar on the left.....
		min_x--;
	/*
	for (y = min_y; y < ((scrolly+h*0.5)/TILE_SIZE_Y)+1; y++)
	{
        for (x = min_x; x < ((scrollx+w*0.5)/TILE_SIZE_X)+1; x++)
        {
            int i = tile_map[x + y * MAP_Y];
            float u = 1 + i * 66;
            float v = 1;
            al_draw_scaled_bitmap(tiles, u, v, 64, 64,x * 64, y * 64, 64, 64, 0); //src x,y,w,h dst x,y,w,h,flags
			//al_draw_bitmap(tiles, );
			//try with non-scaled......
			//al_draw_bitmap(tiles, u, v, 64, 64,x * 64, y * 64, 0); //src x,y,w,h dst x,y,flags
        }
    }
    */
#endif
//draw map at (scrollx-w*0.5)/2, scrollx is saturated xpos. map scaled by 2
//ship gets drawn at xpos, but transformed to -scrollx, +0.5*w. this uses pre-saturated scrollx....
//so, even assuming the translation is inverted wrt offsets in draw functions, the translations are similar, but not the same
//best thing to do might be:

//if (ship first) drw ship; draw map
//else draw map; draw ship
// draw ship does transform, draws ship, bullets.
//draw map checks tiled, creates new var for saturated scroll
//uses identity transform, draws map/tiles using offsets, not transform.

    if (!Map.ship_first)	//draw thw map first
    {
    	//actually now drawing the whole map, and letting the clipping rectangle sort it out.
    	al_draw_scaled_bitmap(tr_map, (scrollx-w*0.5)/2,(scrolly-h*0.5)/2, w/2, h/2, x, y, w, h, 0); //src x,y,w,h dst x,y,w,h,flags
#if 0
//from mapmaker. this had use_transform called first.....
		if (Map.type == 0)//draw tr style map
			//al_draw_scaled_bitmap(tiles, (scroll_x-w*0.5)/2,(scroll_y-h*0.5)/2, w/2, h/2, x, y, w, h, 0); //src x,y,w,h dst x,y,w,h,flags
			al_draw_scaled_bitmap(tr_map, 0,0, al_get_bitmap_width(tiles), al_get_bitmap_width(tiles), x, y, al_get_bitmap_width(tiles)*2, al_get_bitmap_width(tiles)*2, 0); //src x,y,w,h dst x,y,w,h,flags
		else
		{			//draw tiled map
			al_hold_bitmap_drawing(1);
			for (y = 0; y < map_height; y++)
			{
				for (x = 0; x < map_width; x++)
				{
					int i = tile_map[x + y * MAX_MAP_WIDTH];
					float u = i * tile_width;
					float v = 0;
											   //sx  sy sw  sh  dx      dy      dw  dh
					//al_draw_scaled_bitmap(tiles, u,  v, 64, 64, x * 32, y * 32, 32, 32, 0);
					al_draw_bitmap_region(tr_map, u, v, tile_width, tile_height, x*tile_width, y*tile_height,0);
					//al_draw_scaled_bitmap(tiles, u, v, tile_width, tile_height, x*tile_width, y*tile_height, tile_width+2, tile_height+2,0);
				}
			}
			al_hold_bitmap_drawing(0);
		}
#endif
	}



    al_use_transform(&transform);
	//Use draw_bitmap_region with offset for sprite sheet
	for(i=0 ; i<num_ships ; i++)
	{
		if (Ship[i].reincarnate_timer)
		{}
		else
		{
			al_draw_bitmap_region(ships,Ship[i].angle*SHIP_SIZE_X,(2*i + (Ship[i].thrust?1:0) )*SHIP_SIZE_Y, SHIP_SIZE_X, SHIP_SIZE_Y,Ship[i].xpos-SHIP_SIZE_X/2,Ship[i].ypos-SHIP_SIZE_Y/2, 0);
			//if (Ship[i].thrust)
			//	al_draw_rotated_bitmap(Ship[i].ship_flame_bmp,SHIP_SIZE_X/2,SHIP_SIZE_Y/2,Ship[i].xpos,Ship[i].ypos,ANGLE_INC_RAD*Ship[i].angle,0); //Use draw_bitmap_region with offset for sprite sheet
			//else
			//	al_draw_rotated_bitmap(Ship[i].ship_bmp,SHIP_SIZE_X/2,SHIP_SIZE_Y/2,Ship[i].xpos,Ship[i].ypos,ANGLE_INC_RAD*Ship[i].angle,0); //Use draw_bitmap_region with offset for sprite sheet
		}
	}

	al_hold_bitmap_drawing(1);

	//draw bullets
	int current_bullet, previous_bullet;

	current_bullet = first_bullet;
	previous_bullet = END_OF_LIST;

	while(current_bullet != END_OF_LIST)
	{
		//assuming type 0
		//al_draw_bitmap(bullets_bmp,Bullet[current_bullet].xpos, Bullet[current_bullet].ypos,0);

		//only for colour-changing bullets
		switch(Bullet[current_bullet].type)
		{
			case BLT_LAVA:
				//bullet_colour = al_map_rgb(255,128,0);
				//bullet_colour = al_map_rgb(255,(Bullet[current_bullet].ttl>>1)0xFF,0);
				//bullet_colour = al_map_rgb(255,rand()%255,0);
				//bullet[i].colour = modify....
			break;
			case BLT_HEATSEEKER:
				if (Bullet[current_bullet].ttl&0x4)
					Bullet[current_bullet].colour = al_map_rgb(0,255,0);
				else
					Bullet[current_bullet].colour = al_map_rgb(0,0,0);
			break;
			case BLT_MINE:
				al_unmap_rgb(Bullet[current_bullet].colour, &r,&g,&b);

				if (Bullet[current_bullet].ttl&0x1f < 0x10)
					r -= 16;
				else
					r += 16;
				Bullet[current_bullet].colour = al_map_rgb(r,r,0);
			break;
			default:
				//bullet_colour = al_map_rgb(255,255,255);
			break;
		}

		al_draw_tinted_bitmap(bullets_bmp,Bullet[current_bullet].colour,Bullet[current_bullet].xpos, Bullet[current_bullet].ypos,0);
		current_bullet = Bullet[current_bullet].next_bullet;
	}

    al_hold_bitmap_drawing(0);

    al_identity_transform(&transform);
    al_use_transform(&transform);
    //ok, if black was transparent.....
    //works as long as you set palette to 24 bit....
    //undesirable for sitimus, as you go behind the road markings,
    //nice for other levesl, so maybe a map parameter to switch.

    if(Map.ship_first)	//draw map last
    	al_draw_scaled_bitmap(tr_map, (scrollx-w*0.5)/2,(scrolly-h*0.5)/2, w/2, h/2, x, y, w, h, 0); //src x,y,w,h dst x,y,w,h,flags
}
#endif

void draw_status_bar(num_ships)
{
	int i,j;
	int bs;	//size of coloured block;

	al_set_clipping_rectangle(0, 0, SCREENX, SCREENY);

	//Status info
	al_draw_bitmap(status_bg,0,0,0);

	if (Map.mission)
		bs = 160;
	else
		bs = 120;

	for (i=0 ; i<num_ships ; i++)
	{
		//al_draw_filled_rectangle(0,i*100,120,i*100+90,Ship[i].colour);	//120*90 - make a nice bitmap background.
		al_draw_filled_rectangle(15,5+i*bs,135,5+i*bs+(bs-10),Ship[i].colour);
		//al_draw_bitmap(Ship[i].status_bg,0,i*100,0);

		al_draw_filled_rectangle(15,5+i*bs+10,15+Ship[i].shield,5+i*bs+20,al_map_rgb(0, 128, 0));
		al_draw_filled_rectangle(15,5+i*bs+12,15+Ship[i].shield-2,5+i*bs+14,al_map_rgba(255, 255, 255, 64));

		al_draw_filled_rectangle(15,5+i*bs+30,15+Ship[i].ammo1,5+i*bs+40,al_map_rgb(128, 0, 0));
		al_draw_filled_rectangle(15,5+i*bs+32,15+Ship[i].ammo1-2,5+i*bs+34,al_map_rgba(255, 255, 255, 64));

		for (j=0 ; j<Ship[i].ammo2 ; j++)
		{
			al_draw_filled_rectangle(15+j*12+1,5+i*bs+50,15+j*12+11,5+i*bs+60,al_map_rgb(0, 128, 128));
			al_draw_filled_rectangle(15+j*12+1,5+i*bs+52,15+j*12+11,5+i*bs+54,al_map_rgba(255, 255, 255, 64));
		}


		al_draw_filled_rectangle(15,5+i*bs+70,15+(Ship[i].fuel>>4),5+i*bs+80,al_map_rgb(128, 128, 0));
		al_draw_filled_rectangle(15,5+i*bs+72,15+(Ship[i].fuel>>4)-2,5+i*bs+74,al_map_rgba(255, 255, 255, 64));

		for (j=0 ; j<Ship[i].lives ; j++)
			//al_draw_filled_circle(110, i*100+j*12+10, 5, al_map_rgb(255, 255, 255));
			al_draw_filled_triangle(15+106, 5+i*bs+j*12+15, 15+114, 5+i*bs+j*12+15,  15+110, 5+i*bs+j*12+5, al_map_rgb(255, 255, 255));

		if (Map.mission)
		{
			for (j=0 ; j<Ship[i].miners ; j++)
				al_draw_bitmap_region(pickups,16,0,16,16,15+j*12+1,5+i*bs+90,0);
			for (j=0 ; j<Ship[i].jewels ; j++)
				al_draw_bitmap_region(pickups,0,0,16,16,15+j*12+1,5+i*bs+110,0);
		}

		//miners
		//jewels

		if (Ship[i].racing)
			al_draw_textf(race_font, al_map_rgb(255, 255, 255),15, 5+i*bs+(bs-35), ALLEGRO_ALIGN_LEFT, "%0.3f", Ship[i].current_lap_time);
		if (Ship[i].lap_complete)
			al_draw_textf(race_font, al_map_rgb(255, 255, 0),70, 5+i*bs+(bs-35), ALLEGRO_ALIGN_LEFT, "%0.3f", Ship[i].last_lap_time);

	}

	//dividers
	if (num_ships > 1)
		al_draw_filled_rectangle(STATUS_BAR_WIDTH,SCREENY/2-5,SCREENX,SCREENY/2+5,al_map_rgb(128, 128, 128));
	if (num_ships > 2)
		al_draw_filled_rectangle(((SCREENX-STATUS_BAR_WIDTH)/2)+STATUS_BAR_WIDTH-5,0,(SCREENX-STATUS_BAR_WIDTH)/2+STATUS_BAR_WIDTH+5,SCREENY,al_map_rgb(128, 128, 128));


	return;
}

