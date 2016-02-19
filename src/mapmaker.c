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



/* Simple Allegro 5 tilemap example from allegro.cc:
 *
 * http://www.allegro.cc/forums/thread/606482
 *
 * Also see here for more info:
 *
 * http://wiki.allegro.cc/index.php?title=Allegro_5_Tutorial
 *

 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "allegro5/allegro.h"
#include "allegro5/allegro_image.h"
#include "allegro5/allegro_primitives.h"
#include "allegro5/allegro_font.h"

#include "game.h"
/* Our window. */
ALLEGRO_DISPLAY *display;
/* Our tiles atlas. */
ALLEGRO_BITMAP *tiles;
ALLEGRO_BITMAP *ships;
ALLEGRO_BITMAP *mouse_bmp;
ALLEGRO_BITMAP *sentries;
/* Our tilemap. */
#define MAX_MAP_WIDTH 100
#define MAX_MAP_HEIGHT 100
int tile_map[MAX_MAP_WIDTH * MAX_MAP_HEIGHT];
/* Keep track of pressed mouse button. */
int mouse;
/* Camera parameters. */
float zoom = 1.0, rotate;
float scroll_x, scroll_y;
/* Our icon and font. */
ALLEGRO_BITMAP *icon;
ALLEGRO_FONT *font;
/* Simple FPS counter. */
int fps, fps_accum;
double fps_time;

MapType Map;
int map_height=0, map_width=0;
int tile_width = 64;
int tile_height = 64;	//for hex tiles

int tile_offset = 3;	//for displaying tiles in sidebar

int mouse_x, mouse_y;
int mouse_tile_x, mouse_tile_y, dragged_tile;
int dragging;
float w, h;
int saved = 0;
int grid = 0;
#define MAX_GRID 3

void map_draw(void);
void sidebar_draw(void);
void load_map_file(void);
void save_map_file (void);
int init_map(char *map_file_name);
void Exit(void);

//need to specify: ascii map file, tiles bitmap, tile width/height
//map width height can be derived from ascii file.
//tile width fixed at 64? tile hight read from bitmap?
//so maybe just 2 files.....
int main (int argc, char *argv[]){
    ALLEGRO_TIMER *timer;
    ALLEGRO_EVENT_QUEUE *queue;
    bool redraw = true;
	int up_key = false,down_key = false,left_key = false,right_key = false;
	int pgup_key = false, pgdn_key = false, tileup_key = false, tiledown_key = false;
	int i,j;

    if (argc != 2)
    {
		fprintf(stderr,"Map preview utility for Ian's thrust game\nSyntax: ./mapmaker map_info_file_name\nPRE-ALPHA VERSION; UNDER HEAVY DEVELOPMENT!!!!!\n\n");
		return 0;
	}

    //srand(time(NULL));

    /* Init Allegro 5 + addons. */
    al_init();
    al_init_image_addon();
    al_init_primitives_addon();
    al_init_font_addon();
    al_init_ttf_addon();
    al_install_mouse();
    al_install_keyboard();

	//set path
    ALLEGRO_PATH *path = al_get_standard_path(ALLEGRO_RESOURCES_PATH);
	al_change_directory(al_path_cstr(path, '/'));  // change the working directory
	//change directory to data, where all resources live (images, fonts, sounds and text files)
	al_append_path_component(path, "data");
	al_change_directory(al_path_cstr(path, '/'));  // change the working directory
    /* Create our window. */
    al_set_new_display_flags(ALLEGRO_RESIZABLE);
    //display = al_create_display(640, 480);
    display = al_create_display(1280, 720);
    al_set_window_title(display, "Mapmaker");

    //al_set_new_bitmap_flags(ALLEGRO_MIN_LINEAR | ALLEGRO_MAG_LINEAR);

    //tile_map_create();

    for(i=0 ; i<(MAX_MAP_WIDTH*MAX_MAP_HEIGHT) ; i++)
    	tile_map[i] = 0;

    init_map(argv[1]);

	if ((font = al_load_font("miriam.ttf", 30, 0)) == NULL)  fprintf(stderr,"miriam.ttf load fail"); //debug font
	if ((tiles = al_load_bitmap(Map.display_file_name)) == NULL)  fprintf(stderr,"%s load fail",Map.display_file_name);
    if ((ships = al_load_bitmap("ships.png")) == NULL)  fprintf(stderr,"ships.png load fail");
    if ((mouse_bmp = al_load_bitmap("mouse.png")) == NULL)  fprintf(stderr,"mouse.png load fail");
	if ((sentries = al_load_bitmap(Map.sentry_file_name)) == NULL)  fprintf(stderr,"%s",Map.sentry_file_name);
	load_map_file();

	/* Center of map. */
	scroll_x = ((map_width * tile_width)-150) / 2;
    scroll_y = map_height * tile_height / 2;

    mouse_x = 0;
    mouse_y = 0;

    timer = al_create_timer(1.0 / 60);
    queue = al_create_event_queue();
    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_mouse_event_source());
    al_register_event_source(queue, al_get_display_event_source(display));
    al_register_event_source(queue, al_get_timer_event_source(timer));
    al_start_timer(timer);

    while (1) {
        ALLEGRO_EVENT event;
        al_wait_for_event(queue, &event);

        if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
            break;

        if (event.type == ALLEGRO_EVENT_KEY_DOWN)
        {
            //one-shot keys
            if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
                break;
            if (event.keyboard.keycode == ALLEGRO_KEY_ENTER)
                init_map(argv[1]);
                load_map_file();
            if (event.keyboard.keycode == ALLEGRO_KEY_S)
                save_map_file();
            if (event.keyboard.keycode == ALLEGRO_KEY_HOME)
            {
                zoom = 1;
				scroll_x = ((map_width * tile_width)-150) / 2;
			    scroll_y = map_height * tile_height / 2;
			}
            if (event.keyboard.keycode == ALLEGRO_KEY_Q)
                tile_offset++;
            if (event.keyboard.keycode == ALLEGRO_KEY_A)
                tile_offset--;

            if (event.keyboard.keycode == ALLEGRO_KEY_G)
            {
				grid++;
				if (grid > MAX_GRID) grid = 0;
			}

			//hold down keys
            if (event.keyboard.keycode == ALLEGRO_KEY_UP)
                up_key = true;
            if (event.keyboard.keycode == ALLEGRO_KEY_DOWN)
                down_key = true;
            if (event.keyboard.keycode == ALLEGRO_KEY_LEFT)
                left_key = true;
            if (event.keyboard.keycode == ALLEGRO_KEY_RIGHT)
                right_key = true;
            if (event.keyboard.keycode == ALLEGRO_KEY_PGUP)
                pgup_key = true;
            if (event.keyboard.keycode == ALLEGRO_KEY_PGDN)
                pgdn_key = true;
        }
        if (event.type == ALLEGRO_EVENT_KEY_UP)
        {
            //release of hold down keys
            if (event.keyboard.keycode == ALLEGRO_KEY_UP)
                up_key = false;
            if (event.keyboard.keycode == ALLEGRO_KEY_DOWN)
                down_key = false;
            if (event.keyboard.keycode == ALLEGRO_KEY_LEFT)
                left_key = false;
            if (event.keyboard.keycode == ALLEGRO_KEY_RIGHT)
                right_key = false;
            if (event.keyboard.keycode == ALLEGRO_KEY_PGUP)
                pgup_key = false;
            if (event.keyboard.keycode == ALLEGRO_KEY_PGDN)
                pgdn_key = false;
        }

        if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
        {
            mouse = event.mouse.button;
            if (mouse == 1)	//left click
            {
            	if (mouse_x < 150)	//in sidebar so pick up 'new' tile
            	{
					dragging = true;
					dragged_tile = mouse_y/(tile_height+5) - tile_offset;
				}
				//check in total map here?
				else if (dragging)	//if we have already picked one up, then place it
				{
					if (dragged_tile != tile_map[mouse_tile_x + mouse_tile_y * MAX_MAP_WIDTH])
						tile_map[mouse_tile_x + mouse_tile_y * MAX_MAP_WIDTH] = dragged_tile;
				}
				else if (mouse_tile_x >= 0 && mouse_tile_x < map_width &&	//otherwise, check we're in the map
            			 mouse_tile_y >= 0 && mouse_tile_y < map_height)	//and pick up what we clicked on
				{
					dragging = true;
					dragged_tile = tile_map[mouse_tile_x + mouse_tile_y * MAX_MAP_WIDTH];	//pick up tile
				}

            	/*
            	else if (mouse_tile_x >= 0 && mouse_tile_x < map_width &&	//check we're in the map
            			 mouse_tile_y >= 0 && mouse_tile_y < map_height)
            	{																			//if so,
            		dragged_tile = tile_map[mouse_tile_x + mouse_tile_y * MAX_MAP_WIDTH];	//pick up tile
            		tile_map[mouse_tile_x + mouse_tile_y * MAX_MAP_WIDTH] = 0;				//and remove it from the map
				}
				*/
			}
			else if (mouse == 2)	//right click
			{
				if (dragging) 			//if we're dragging, stop it
					dragging = false;
				else if (mouse_x >150)	//if not dragging, and not in status bar
				{
					if (mouse_tile_x >= 0 && mouse_tile_x < map_width &&	//and we're in the map
            			 mouse_tile_y >= 0 && mouse_tile_y < map_height)
					{
						tile_map[mouse_tile_x + mouse_tile_y * MAX_MAP_WIDTH] = 0;	//erase what we clicked on.
					}
				}
			}
        }
        if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP)
        {
            /*
            if (mouse == 1) //if we were dragging a tile
            {
            	if (mouse_x < 150)	//in  sidebar
            	{
					 if (mouse_y < 110)	//in top-left, so we're trying to delete, so do nothing
					 {}
					 //else
					 //{
					//	dragged_tile = mouse_y/37 - tile_offset; //pick up a tile
					 //}
				}

            	else if (mouse_tile_x >= 0 && mouse_tile_x < MAX_MAP_WIDTH &&	//check if we're in the entire array
            			 mouse_tile_y >= 0 && mouse_tile_y < MAX_MAP_HEIGHT)
				{																			//if so,
					tile_map[mouse_tile_x + mouse_tile_y * MAX_MAP_WIDTH] = dragged_tile;	//place tile
					if (mouse_tile_x >= map_width) map_width = mouse_tile_x+1;					//extend map if required
					if (mouse_tile_y >= map_height) map_height = mouse_tile_y+1;
				}
			}
			*/
            mouse = 0;
        }

        if (event.type == ALLEGRO_EVENT_MOUSE_AXES) //mouse has moved
        {
            // Left button scrolls.
            //if (mouse == 1) {
                mouse_x += event.mouse.dx;// / zoom;
                mouse_y += event.mouse.dy;// / zoom;
                //scroll_x -= x;// * cos(rotate) + y * sin(rotate);
                //scroll_y -= y;// * cos(rotate) - x * sin(rotate);
            //}
            /*
            // Right button zooms/rotates.
            if (mouse == 2) {
                //rotate += event.mouse.dx * 0.01;
                zoom += event.mouse.dy * 0.01 * zoom;
            }
            zoom += event.mouse.dz * 0.1 * zoom;
            if (zoom < 0.1) zoom = 0.1;
            if (zoom > 10) zoom = 10;
            */

            if (mouse == 1)	//left button held
            {
				if (mouse_x > 150)	//not in sidebar
				{
					if (dragging)
					{
						if (dragged_tile != tile_map[mouse_tile_x + mouse_tile_y * MAX_MAP_WIDTH])
							tile_map[mouse_tile_x + mouse_tile_y * MAX_MAP_WIDTH] = dragged_tile;
					}
				}
			}
        }

        if (event.type == ALLEGRO_EVENT_TIMER)
            redraw = true;
        if (event.type == ALLEGRO_EVENT_DISPLAY_RESIZE) {
            al_acknowledge_resize(display);
            redraw = true;
        }

        if (redraw && al_is_event_queue_empty(queue)) {
            redraw = false;
            double t = al_get_time();

            if (up_key)		scroll_y -= 10/zoom;//10/(1+(5*(zoom-1)));
            if (down_key)	scroll_y += 10/zoom;//10/(1+(5*(zoom-1)));
            if (left_key)  	scroll_x -= 10/zoom;//10/(1+(5*(zoom-1)));
            if (right_key) 	scroll_x += 10/zoom;//10/(1+(5*(zoom-1)));
            if (pgup_key)  	zoom += 0.03*zoom;
            if (pgdn_key)  	zoom -= 0.03*zoom;

            if (zoom < 0.1) zoom = 0.1;
            if (zoom > 10)	zoom = 10;

           	map_draw();

            sidebar_draw();

            al_flip_display();
        }
	}
}

#if 0
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
            al_draw_filled_circle(x + w * 0.5, y + h * 0.5, w * 0.475,
                yellow);
            break;
        case 2:
            al_draw_filled_rectangle(x, y, x + w, y + h, yellow);
            al_draw_filled_triangle(x + w * 0.5, y + h * 0.125,
                x + w * 0.125, y + h * 0.875,
                x + w * 0.875, y + h * 0.875, red);
            break;
        case 3:
            al_draw_filled_rectangle(x, y, x + w, y + h, black);
            if (icon)
                al_draw_scaled_bitmap(icon, 0, 0, 48, 48,
                    x, y, w, h, 0);
            break;
    }
}

/* Creates the tiles and a random 100x100 map. */
void tile_map_create(void) {
    int i;
    int x, y;
    /* Create the tile atlas. */
    tiles = al_create_bitmap(1024, 1024);
    al_set_target_bitmap(tiles);
    al_clear_to_color(al_map_rgba(0, 0, 0, 0));
    for (i = 0; i < 4; i++) {
        /* We draw the tiles a bit bigger (66x66 instead of 64x64)
         * to account for the linear filtering. Normally just leaving
         * the border transparent for sprites or repeating the border
         * for tiling tiles should work well.
         */
        tile_draw(i, i * 66, 0, 66, 66);
    }
    al_set_target_backbuffer(display);

    /* Create the random map. */
    for (y = 0; y < 100; y++) {
        for (x = 0; x < 100; x++) {
            tile_map[x + y * 100] = rand() % 4;
        }
    }

    /* Center of map. */
    scroll_x = 100 * 64 / 2;
    scroll_y = 100 * 64 / 2;
}
#endif

/* Draws the complete map. */
void map_draw(void) {
    int x=0, y=0;
    int i;
    ALLEGRO_TRANSFORM transform;
    ALLEGRO_COLOR grid_colour;

    w = al_get_display_width(display);
    h = al_get_display_height(display);

    /* Initialize transformation. */
    al_identity_transform(&transform);
    /* Move to scroll position. */
    al_translate_transform(&transform, -scroll_x, -scroll_y);
    /* Rotate and scale around the center first. */
    //al_rotate_transform(&transform, rotate);
    al_scale_transform(&transform, zoom, zoom);
    /* Move scroll position to screen center. */
    al_translate_transform(&transform, w * 0.5, h * 0.5);
    /* All subsequent drawing is transformed. */
    al_use_transform(&transform);

    al_clear_to_color(al_map_rgb(0, 0, 0));

    if (Map.type == 0)//draw tr style map
    	//al_draw_scaled_bitmap(tiles, (scroll_x-w*0.5)/2,(scroll_y-h*0.5)/2, w/2, h/2, x, y, w, h, 0); //src x,y,w,h dst x,y,w,h,flags
		al_draw_scaled_bitmap(tiles, 0,0, al_get_bitmap_width(tiles), al_get_bitmap_width(tiles), x, y, al_get_bitmap_width(tiles)*2, al_get_bitmap_width(tiles)*2, 0); //src x,y,w,h dst x,y,w,h,flags
    else
    {			//draw tiled map
		al_hold_bitmap_drawing(1);
		for (y = 0; y < map_height; y++)
		{
			for (x = 0; x < map_width; x++)
			{
				i = tile_map[x + y * MAX_MAP_WIDTH];
				float u = i * tile_width;
				float v = 0;
										   //sx  sy sw  sh  dx      dy      dw  dh
				//al_draw_scaled_bitmap(tiles, u,  v, 64, 64, x * 32, y * 32, 32, 32, 0);
				al_draw_bitmap_region(tiles, u, v, tile_width, tile_height, x*tile_width, y*tile_height,0);
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
				al_draw_filled_rectangle(i*tile_width,0,i*tile_width+1,map_height*tile_height,grid_colour);
			}
			for (i=0 ; i<map_height ; i++)
			{
				al_draw_filled_rectangle(0,i*tile_height,map_width*tile_width,i*tile_height+1,grid_colour);
			}

		}
	}

	//then sentries
	for(i=0 ; i<Map.num_sentries ; i++)
	{
		//if (Map.sentry[i].alive)
		{
			al_draw_bitmap_region(sentries,Map.sentry[i].alive_sprite*64,0, 64, 64,Map.sentry[i].x-64/2,Map.sentry[i].y-64/2, 0);
		}
	}

	//forcefields
	for(i=0 ; i<Map.num_forcefields ; i++)
	{
		//if (Map.sentry[Map.forcefield[i].sentry].alive)
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


    al_translate_transform(&transform, scroll_x*zoom, scroll_y*zoom);	//remove scroll so ship is always in the centre
    al_use_transform(&transform);
    al_draw_bitmap_region(ships,0,0,SHIP_SIZE_X, SHIP_SIZE_Y,0,0, 0);

    //al_translate_transform(&transform, w * -0.5 *zoom, h * -0.5*zoom);	//remove centring so dragged tile is always in the centre
    //al_use_transform(&transform);											//DOESN'T WORK!
	//if (mouse == 1)
	//   	al_draw_bitmap_region(tiles, dragged_tile*tile_width, 0, tile_width, tile_height,  mouse_x-0.5*tile_width, mouse_y-0.5*tile_height,0);


    al_identity_transform(&transform);
    al_use_transform(&transform);
}

//draws all the non-translated and scaled stuff, sidebar, mouse pointer, notifications etc....
void sidebar_draw(void)
{
	int i, i_char;
	float left_edge, top_edge;

	al_draw_filled_rectangle(0,0,150,al_get_display_height(display),al_map_rgb(128, 128, 128));	//sidebar

	if (Map.type == 1)	//tiled
	{
		for (i=0 ; i<al_get_bitmap_width(tiles)>>6 ; i++)	//draw tiles in sidebar
		{
			if (i>=0 && i<=9) i_char = i+'0';
			else if (i>=10 && i<=36) i_char = i-10+'A';
			al_draw_textf(font, al_map_rgb(255, 255, 255),10, (i+tile_offset)*(tile_height+5) ,  ALLEGRO_ALIGN_LEFT, "%c", i_char);
			al_draw_bitmap_region(tiles, i*tile_width, 0, tile_width, tile_height, 50, (i+tile_offset)*(tile_height+5),0);
		}
	}

	al_draw_filled_rectangle(0,0,150,110,al_map_rgb(64, 64, 64));	//darker bit with info

	left_edge = (-scroll_x*zoom)+(0.5*w);
	top_edge  = (-scroll_y*zoom)+(0.5*h);
	mouse_tile_x = (int)(mouse_x - left_edge)/(zoom*tile_width);
	mouse_tile_y = (int)(mouse_y -  top_edge)/(zoom*tile_height);

	i=0;
	al_draw_textf(font, al_map_rgb(255, 255, 255),10, i++*37,  ALLEGRO_ALIGN_LEFT, "x:%.0f %d", scroll_x+SHIP_SIZE_X/2, mouse_x);
	al_draw_textf(font, al_map_rgb(255, 255, 255),10, i++*37,  ALLEGRO_ALIGN_LEFT, "y:%.0f %d", scroll_y+SHIP_SIZE_Y/2, mouse_y);
	al_draw_textf(font, al_map_rgb(255, 255, 255),10, i++*37,  ALLEGRO_ALIGN_LEFT, "zoom:%.2f", zoom);

	if (saved)
	{
		saved--;//, 720
		al_draw_filled_rounded_rectangle(1280/2, 720/2, 1280/2+100, 720/2+30, 8, 8, al_map_rgba(0, 0, 0, 200));
		al_draw_textf(font, al_map_rgb(255, 255, 255),1280/2+10, 720/2, ALLEGRO_ALIGN_LEFT, "Saved.");
	}

	//if (mouse == 1)
	if (dragging)
    	//al_draw_bitmap_region(tiles, dragged_tile*tile_width, 0, tile_width, tile_height,  mouse_x-0.5*tile_width, mouse_y-0.5*tile_height,0);
		al_draw_scaled_bitmap(tiles, dragged_tile*tile_width, 0, tile_width, tile_height,  mouse_x-0.5*tile_width, mouse_y-0.5*tile_height, tile_width*zoom, tile_height*zoom, 0);
	al_draw_bitmap_region(mouse_bmp, 0, 0, 25, 25, mouse_x, mouse_y,0);	//mouse cursor
}
//parse:
//align label first column, parameters sep by spaces, suits scanf I think.
//so read line, check label, scanf, pattern determined by label.

//need tile width, height
//collision tiles half size
//display ships on pads
//DRAG AND DROP:
// EDITING: spot tile on left click (might not be that easy, with scroll/zoom...)
//          edit map array to replace clicked tile with 0
//			set flag to draw 'moving tile' - set to the one you clicked on (so remember before you change!)
//			on left button release, spot tile position, clear moving tile flag, edit map to put that tile in new position.
// DELETING:drop tile on sidebar :-)
// ADDING:  left click in sidebar, spot tile, don't need to edit map array
//			rest as per editing.
// on exit, write map back to file.

int init_map(char *map_file_name)
{
	FILE* map_file;

	int i=0, j=0,k=0,l=0,m=0,n=0;	//counters for pads, special areas, blackholes, sentries etc.
	//char map_file_name[MAP_NAME_LENGTH];
	char line[100];

	//strncpy(map_file_name, (map_names + map*MAP_NAME_LENGTH), MAP_NAME_LENGTH);

	//strcat(map_file_name, txt);

	map_file = fopen(map_file_name,"r");

	if (map_file == NULL)
	{
		fprintf(stderr,"Couldn't open file %s\n",map_file_name);
		Exit();
	}

    //Map.description_file_name[0] = 0;
    Map.radial_gravity = false;
    Map.race = false;
    Map.max_players=1;

	while (fgets(line, 100, map_file) != NULL)
	{
		if (strncmp(line,"map_type",8) == 0)
		{
			sscanf(line+8," %d",&Map.type);
			fprintf(stderr,"Map type:%d\n",Map.type);
		}

		else if (strncmp(line,"display_map",11) == 0)
		{
			sscanf(line+11," %s",&Map.display_file_name);
			fprintf(stderr,"Display Map:%s\n",Map.display_file_name);
		}

		else if (strncmp(line,"collision_map",13) == 0)
		{
			sscanf(line+13," %s",&Map.collision_file_name);
			fprintf(stderr,"Collision Map:%s\n",Map.collision_file_name);
		}

		else if (strncmp(line,"ascii_map",9) == 0)
		{
			sscanf(line+9," %s",&Map.ascii_map_file_name);
			fprintf(stderr,"ASCII map file:%s\n",Map.ascii_map_file_name);
		}

		else if (strncmp(line,"sentry_display",14) == 0)
		{
			sscanf(line+14," %s",&Map.sentry_file_name);
			fprintf(stderr,"Sentry Image file:%s\n",Map.sentry_file_name);
		}

		else if (strncmp(line,"ship_first",10) == 0)
		{
			sscanf(line+10," %d",&Map.ship_first);
			fprintf(stderr,"Ship first:%d\n",Map.ship_first);
		}

		else if (strncmp(line,"pad",3) == 0)
		{
			sscanf(line+3," %x %d %d %d",&Map.pad[i].type,&Map.pad[i].y,&Map.pad[i].min_x,&Map.pad[i].max_x);
			fprintf(stderr,"Pad %d: type:%02x y:%d x:%d x:%d\n",i,Map.pad[i].type,Map.pad[i].y,Map.pad[i].min_x,Map.pad[i].max_x);

            //Ship[Map.pad[i].type & 0x000f].home_pad = i;    //bottom nibble of type gives ship which this is home pad for.

            i++;
		}

		else if (strncmp(line,"area",4) == 0)
		{
			sscanf(line+4," %d %d %d %d %f %f",                           &Map.area[j].min_x,&Map.area[j].max_x,&Map.area[j].min_y,&Map.area[j].max_y,&Map.area[j].gravity,&Map.area[j].drag);
			fprintf(stderr,"area %d: x:%d x:%d y:%d y:%d g:%f drag:%f\n",j,Map.area[j].min_x, Map.area[j].max_x, Map.area[j].min_y, Map.area[j].max_y, Map.area[j].gravity, Map.area[j].drag);
			j++;
		}

		else if (strncmp(line,"blackhole",9) == 0)
		{
			Map.radial_gravity = true;
			if (l==0)
				fprintf(stderr,"Radial Gravity On\n");

			sscanf(line+9," %d %d %f",                        &Map.blackhole[l].x,&Map.blackhole[l].y,&Map.blackhole[l].g);
			fprintf(stderr,"blackhole %d: x:%d y:%d g:%f\n",l,Map.blackhole[l].x,Map.blackhole[l].y,Map.blackhole[l].g);
			l++;
		}

		//      x y type(0/1) gun volcano firing period probability random/targeted
		//sentry

		else if (strncmp(line,"sentry",6) == 0)
		{
			sscanf(line+6," %i %i %i %i %i %i %i %i %i %i",                                                                   &Map.sentry[m].x, &Map.sentry[m].y, &Map.sentry[m].direction, &Map.sentry[m].type, &Map.sentry[m].period, &Map.sentry[m].probability, &Map.sentry[m].random, &Map.sentry[m].range, &Map.sentry[m].alive_sprite, &Map.sentry[m].dead_sprite);
			fprintf(stderr,"Sentry %i: x:%i, y:%i, Direction:%i, Type:%i, Period:%i, Prob:%i, Random:%i, Range:%i, Sprite%d, Sprite%d\n",m, Map.sentry[m].x,  Map.sentry[m].y,  Map.sentry[m].direction,  Map.sentry[m].type,  Map.sentry[m].period,  Map.sentry[m].probability,  Map.sentry[m].random,  Map.sentry[m].range, Map.sentry[m].alive_sprite, Map.sentry[m].dead_sprite);
			//Map.sentry[m].range = Map.sentry[m].range * Map.sentry[m].range;	//square to save square rooting later.
			//Map.sentry[m].count = Map.sentry[m].period;	//init countdown timer
			//Map.sentry[m].shield = SENTRY_SHIELD;		//init shield
			//Map.sentry[m].alive = 1;
			m++;
		}

		else if (strncmp(line,"race",4) == 0)
		{
			//fprintf(stderr,"RACE\n");
			sscanf(line+4," %d %d %d %d",&Map.raceline_minx,&Map.raceline_maxx,&Map.raceline_miny,&Map.raceline_maxy);
			//fprintf(stderr,"Race start/finish line: min x:%d max x:%d min y:%d max y:%d\n",Map.raceline_minx,Map.raceline_maxx,Map.raceline_miny,Map.raceline_maxy);
			Map.race = true;
			if (Map.raceline_minx == Map.raceline_maxx)
			{
				Map.before_maxx = Map.raceline_minx + 20;	//experimental
				Map.after_minx  = Map.raceline_minx - 20;

                Map.before_minx = Map.raceline_minx;
                Map.after_maxx  = Map.raceline_minx;

				Map.before_miny = Map.raceline_miny;
				Map.after_miny  = Map.raceline_miny;

				Map.before_maxy = Map.raceline_maxy;
				Map.after_maxy  = Map.raceline_maxy;
			}
			else if (Map.raceline_miny == Map.raceline_maxy)
			{
				Map.before_miny = Map.raceline_miny - 20;
				Map.after_maxy  = Map.raceline_miny + 20;

				Map.before_maxy = Map.raceline_miny;
				Map.after_miny  = Map.raceline_miny;

				Map.before_minx = Map.raceline_minx;
				Map.after_minx  = Map.raceline_minx;

				Map.before_maxx = Map.raceline_maxx;
				Map.after_maxx  = Map.raceline_maxx;
			}

			else fprintf(stderr,"X or Y values must match in race start/finish line (i.e. line must be horizontal or vertical)\n");

			fprintf(stderr,"Race:   x:%d x:%d y:%d y:%d\n",Map.raceline_minx,Map.raceline_maxx,Map.raceline_miny,Map.raceline_maxy);
			fprintf(stderr,"Before: x:%d x:%d y:%d y:%d\n",Map.before_minx,Map.before_maxx,Map.before_miny,Map.before_maxy);
			fprintf(stderr,"After:  x:%d x:%d y:%d y:%d\n",Map.after_minx,Map.after_maxx,Map.after_miny,Map.after_maxy);
		}


		else if (strncmp(line,"forcefield",10) == 0)
		{
			sscanf(line+10," %d %d %d %d %f %d %d %d",&Map.forcefield[n].min_x,&Map.forcefield[n].max_x,&Map.forcefield[n].min_y,&Map.forcefield[n].max_y,&Map.forcefield[n].strength,&Map.forcefield[n].sentry,&Map.forcefield[n].alive_sprite,&Map.forcefield[n].dead_sprite);
			if (Map.forcefield[n].min_x == Map.forcefield[n].max_x)	//vertical line
			{
				Map.forcefield[n].half_x = Map.forcefield[n].min_x;
				Map.forcefield[n].min_x = Map.forcefield[n].half_x - 20;
				Map.forcefield[n].max_x = Map.forcefield[n].half_x + 20;

				Map.forcefield[n].half_y = Map.forcefield[n].min_y;	//don't need half_y, so set = min_y and use as test for line direction.

				//Map.forcefield[n].x_force = strength = ff_strength;
			}
			else if (Map.forcefield[n].min_y == Map.forcefield[n].max_y)	//horizontal line
			{
				Map.forcefield[n].half_y = Map.forcefield[n].min_y;
				Map.forcefield[n].min_y = Map.forcefield[n].half_y - 20;
				Map.forcefield[n].max_y = Map.forcefield[n].half_y + 20;

				Map.forcefield[n].half_x = Map.forcefield[n].min_x;

				//Map.forcefield[n].y_force = strength = ff_strength;
			}

			else fprintf(stderr,"X or Y values must match in forcefield (i.e. line must be horizontal or vertical)\n");

			fprintf(stderr,"ForceField: x:%d x:%d x:%d y:%d y:%d y:%d\n",Map.forcefield[n].min_x,Map.forcefield[n].half_x,Map.forcefield[n].max_x,Map.forcefield[n].min_y,Map.forcefield[n].half_y,Map.forcefield[n].max_y);
			fprintf(stderr,"ForceField: strength:%0.0f sentry:%d\n",Map.forcefield[n].strength,Map.forcefield[n].sentry);
			n++;
		}
	}

	Map.num_pads = i;
	Map.num_special_areas = j;
	Map.num_blackholes = l;
	Map.num_sentries = m;
	Map.num_forcefields = n;

	fclose  (map_file);
	return 0;
}

void load_map_file(void)
{
	int i,j,found;
    FILE* map_file;
	char line[200];

	if (Map.type == 0) return;	//whole map file, not tiles

	if ((map_file = fopen(Map.ascii_map_file_name,"r")) == NULL)  fprintf(stderr,"Couldn't open %s for reading.\n",Map.ascii_map_file_name);

	j=0;
	while(1)
	{
		if (fgets(line,200,map_file) == NULL)	//get a line from the file, exit on end of file
			break;

		i=0;
		found = false;
		while(line[i] != 0x0a && line[i] != 0x0d)	//line ends with CR or LF
		{

			if (line[i] == ' ')							//space counts as zero
				tile_map[i+MAX_MAP_WIDTH*j] = 0;
			else if (line[i] >='0' && line [i] <= '9')	//ascii 0-9 map to integer 0-9
				tile_map[i+MAX_MAP_WIDTH*j] = line[i]-'0';
			else if (toupper(line[i]) >='A' && toupper(line[i]) <= 'Z')	//ascii A-Z(or a-z) map to 10-36
				tile_map[i+MAX_MAP_WIDTH*j] = line[i]-'A'+10;

			if (line[i] != ' ')
			{
				found = true;
				if(i >= map_width) map_width = i+1; //width is last valid char
			}
			i++;

		}
		//fprintf(stderr,"%d ",j);
		j++;
		if (found) 	map_height = j;	//height is number of lines read
	}

	fclose(map_file);

	fprintf(stderr,"H:%d W:%d\n",map_height,map_width);
}

void save_map_file (void)
{
	int i,j;
    FILE* map_file;
    char tile,tile_char;
	//char line[200];

	if ((map_file = fopen(Map.ascii_map_file_name,"w")) == NULL)  fprintf(stderr,"Couldn't open %s for writing.\n",Map.ascii_map_file_name);

	for( j=0 ; j<MAX_MAP_HEIGHT ; j++)
	{
		for (i=0 ; i<MAX_MAP_WIDTH ; i++)
		{
			tile = tile_map[i+j*MAX_MAP_WIDTH];

			if (tile == 0) tile_char = ' ';
			else if (tile >   0 && tile <=9)  tile_char = tile + '0';
			else if (tile >= 10 && tile <=36) tile_char = tile -10 + 'A';

			fprintf(map_file,"%c",tile_char);
		}
		fprintf(map_file,"\n");
	}

	fclose(map_file);

	printf("Saved map: W=%d H=%d",map_width,map_height);

	saved = 100;	//frame timer for display of saved message.

	//fprintf(stderr,"1");

}

void Exit(void)
{
	printf("Exiting\n");
    if (tiles)
    {
        al_destroy_bitmap(tiles);
        tiles = NULL;
    }
    al_destroy_display(display);
	exit(0);
}