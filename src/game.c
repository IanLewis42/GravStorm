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

/* Ian's Turbo Raketti clone */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "allegro5/allegro.h"
#include "allegro5/allegro_image.h"
#include "allegro5/allegro_primitives.h"
#include "allegro5/allegro_font.h"
#include "allegro5/allegro_ttf.h"
#include "allegro5/allegro_audio.h"
#include "allegro5/allegro_acodec.h"

#include <wiringPi.h>

#include "game.h"
#include "drawing.h"
#include "init.h"
#include "collisions.h"
#include "objects.h"
#include "inputs.h"

ALLEGRO_DISPLAY *display;

ALLEGRO_FONT *font;         //debug
ALLEGRO_FONT *menu_font;
ALLEGRO_FONT *small_font;
ALLEGRO_FONT *big_font;
ALLEGRO_FONT *race_font;
ALLEGRO_FONT *title_font;

int fps, fps_accum;
double fps_time;

//Bitmaps
ALLEGRO_BITMAP *ships;
ALLEGRO_BITMAP *status_bg;
ALLEGRO_BITMAP *panel_bmp;
ALLEGRO_BITMAP *panel_pressed_bmp;
ALLEGRO_BITMAP *bullets_bmp;
ALLEGRO_BITMAP *tr_map;
ALLEGRO_BITMAP *sentries;
ALLEGRO_BITMAP *pickups;
ALLEGRO_BITMAP *miner;
ALLEGRO_BITMAP *jewel;

ALLEGRO_BITMAP *menu_bg_bmp;

//Sounds
ALLEGRO_VOICE *voice;
ALLEGRO_MIXER *mixer;
//ALLEGRO_SAMPLE_INSTANCE *sample;
//ALLEGRO_SAMPLE_INSTANCE *clunk_inst;
ALLEGRO_SAMPLE_INSTANCE *wind_inst;

ALLEGRO_SAMPLE *clunk;
ALLEGRO_SAMPLE *wind;

MapType Map;
MenuType Menu;

char map_names[MAP_NAME_LENGTH * MAX_MAPS];

/*
typedef struct
{
	char map[MAP_NAME_LENGTH];
} MapNameType;

typedef struct
{
	MapNameType Group;
	MapNameType Map[MAX_MAPS];
	int Count;	//number of valid maps
} MapGroupType;

#define MAX_GROUPS 3
*/
MapGroupType MapNames[MAX_GROUPS];


int game_over = 0;
int mapx, mapy;
int tile_width = 64, tile_height = 64;

int gpio_active = false;
int debug_on = false;
bool pressed_keys[ALLEGRO_KEY_MAX];

bool take_screenshot = false;
int screenshot_count = 0;
char screenshout_count_str[3];
char screenshot_basename[5] = {'S','c','r','_',0};
char screenshot_ext[5] = {'.','p','n','g',0};
char screenshot_name[20];

FILE* logfile;

//Local prototypes
void DoTitle(ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_EVENT event);
int  DoMenu(ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_EVENT event);
int  GameOver(void);
int  FreeGameBitmaps(void);
void FreeMenuBitmaps(void);
void FreeFonts(void);
void Exit(void);
void draw_debug(void);

int main (int argc, char *argv[]){
//int main(void) {
    ALLEGRO_TIMER *timer;
    ALLEGRO_EVENT_QUEUE *queue;
    ALLEGRO_EVENT event;
    bool redraw = true;
	int i,j,k,temp;
	int num_maps,selected_map;

	//parse command line arguments
	for (i=1 ; i<argc ; i++)
	{
		if (strncmp(argv[i],"-g",2) == 0) gpio_active = true;	//Enable GPIO joystick with -g switch
		if (strncmp(argv[i],"-d",2) == 0) debug_on = true;		//Enable debug output
	}

	/* Init Allegro 5 + addons. */
    al_init();

	//set path
    ALLEGRO_PATH *path = al_get_standard_path(ALLEGRO_RESOURCES_PATH);
	al_change_directory(al_path_cstr(path, '/'));  // change the working directory

	//open logfile in executable directory
	logfile = fopen("logfile.txt","w");
    fprintf(logfile,"Init Allegro\n");

    //init other bits of allegro
    al_init_image_addon();
    al_init_primitives_addon();
    al_init_font_addon();
    al_init_ttf_addon();
    al_install_mouse();
    al_install_keyboard();
    al_install_audio();
	al_init_acodec_addon();

    srand(time(NULL));

	//move to objects / init??
    for(i=0 ; i<NUM_ANGLES ; i++)
    {
		sinlut[i] = sin(i*ANGLE_INC_RAD);
		coslut[i] = cos(i*ANGLE_INC_RAD);
	}

    /* Create our window. */
    al_set_new_display_flags(ALLEGRO_FULLSCREEN_WINDOW);//ALLEGRO_WINDOWED);// | ALLEGRO_RESIZABLE);
    fprintf(logfile,"Creating display\n");
    display = al_create_display(SCREENX, SCREENY);

    al_set_window_title(display, "Headrush");

	//change directory to data, where all resources live (images, fonts, sounds and text files)
	al_append_path_component(path, "data");
	al_change_directory(al_path_cstr(path, '/'));  // change the working directory

    //font = al_load_font("arial.ttf", 20, 0);	//debug font
    if ((font       = al_load_font("miriam.ttf", 20, 0))          == NULL)  fprintf(logfile,"miriam.ttf load fail"); //debug font
    if ((menu_font  = al_load_font("Audiowide-Regular.ttf", 40,0))== NULL)  fprintf(logfile,"Audiowide-Regular.ttf load fail"); //*****
    if ((small_font = al_load_font("Audiowide-Regular.ttf", 30,0))== NULL)  fprintf(logfile,"Audiowide-Regular.ttf load fail"); //*****
    if ((big_font   = al_load_font("northstar.ttf", 200, 0))      == NULL)  fprintf(logfile,"northstar.ttf load fail");
    if ((race_font  = al_load_font("7seg.ttf", 20, 0))            == NULL)  fprintf(logfile,"7seg.ttf load fail");
	//if ((title_font = al_load_font("41155_WESTM.ttf", 200, 0))    == NULL)  fprintf(logfile,"41155_WESTM.ttf load fail");
	if ((title_font = al_load_font("Zebulon.otf", 130, 0))    == NULL)  fprintf(logfile,"Zebulon.otf load fail");
	fprintf(logfile,"Loaded 6 fonts\n");
	fflush(logfile);

	//al_set_new_bitmap_flags(ALLEGRO_MIN_LINEAR | ALLEGRO_MAG_LINEAR);
	//fprintf(logfile,"Creating Tilemap\n");
	//tile_map_create();
	//make_bullet_bitmap();

    /*
    voice = al_create_voice(44100, ALLEGRO_AUDIO_DEPTH_INT16, ALLEGRO_CHANNEL_CONF_2);
    mixer = al_create_mixer(44100, ALLEGRO_AUDIO_DEPTH_INT16, ALLEGRO_CHANNEL_CONF_2);
    al_set_default_mixer(mixer);
    al_attach_mixer_to_voice(mixer, voice);
    clunk = al_load_sample("clunk.wav");
    clunk_inst = al_create_sample_instance(clunk);
    al_attach_sample_instance_to_mixer(clunk_inst, mixer);
    al_set_sample_instance_playmode(clunk_inst, ALLEGRO_PLAYMODE_ONCE);
    */

	al_reserve_samples(6);
	if ((shoota = al_load_sample  ("shootA.wav"))   == NULL)  fprintf(logfile,"shootA.wav load fail");
	if ((shootb = al_load_sample  ("shootB.wav"))   == NULL)  fprintf(logfile,"shootB.wav load fail");
	if ((particle = al_load_sample("particle.wav")) == NULL)  fprintf(logfile,"particle.wav load fail");
	if ((dead = al_load_sample    ("dead.wav"))     == NULL)  fprintf(logfile,"dead.wav load fail");
	if ((clunk = al_load_sample   ("clunk.wav"))    == NULL)  fprintf(logfile,"clunk.wav load fail");
	if ((wind = al_load_sample    ("wind.wav"))     == NULL)  fprintf(logfile,"wind.wav load fail");

	fprintf(logfile,"Creating Events\n");
	timer = al_create_timer(1.0 / 30);
	queue = al_create_event_queue();

	//Allegro Joystick routines
	//move to inputs/init??
	if (al_install_joystick())	//USB joystick, hopefully
		fprintf(logfile,"Init allegro joystick\n");
	fprintf(logfile,"%d Joysticks on system\n",al_get_num_joysticks());

	al_register_event_source(queue, al_get_keyboard_event_source());
	//al_register_event_source(queue, al_get_mouse_event_source());
	al_register_event_source(queue, al_get_display_event_source(display));
	al_register_event_source(queue, al_get_timer_event_source(timer));
	al_register_event_source(queue, al_get_joystick_event_source());

	for (i=0 ; i<al_get_num_joysticks() ; i++)
	{
		USBJOY[i] = al_get_joystick(i);
		if (al_get_joystick_active(USBJOY[i]))
			fprintf(logfile,"Joystick %d active\n",i);
		else
			fprintf(logfile,"Joystick %d NOT active\n",i);
	}

	al_start_timer(timer);

	fflush(logfile);

	if (gpio_active)
	{
		fprintf(logfile,"Init GPIO joystick\n");
		init_joystick();	//Ian's GPIO hacked joystick.
	}
	else
		fprintf(logfile,"Skip GPIO joystick\n");

	init_controls();	//setup defaults for what controls which ship.

	fflush(logfile);

	Menu.map = 0;	//start on first map, but only first time. After that, remember it.

	DoTitle(queue, event);

	//loop to here
	while(1)
	{
		DoMenu(queue, event);

		fprintf(logfile,"Loading map\n");
		fflush(logfile);
		LoadMap();	//load TR map. Filenames from map struct.

        fprintf(logfile,"Making collision masks\n");
        fflush(logfile);
        make_ship_col_mask();
        make_sentry_col_mask();
		make_map_col_mask();

        fprintf(logfile,"Loading game bitmaps\n");
		fflush(logfile);
        if ((ships = al_load_bitmap("ships.png")) == NULL)  fprintf(logfile,"ships.png load fail");
        if ((pickups = al_load_bitmap("pickups.png")) == NULL)  fprintf(logfile,"pickups.png load fail");
        //if ((miner = al_load_bitmap("miner2.png")) == NULL)  fprintf(logfile,"miner.png load fail");;
        if ((miner = al_load_bitmap("astronaut.png")) == NULL)  fprintf(logfile,"astronaut.png load fail");;
        if ((jewel = al_load_bitmap("jewels.png")) == NULL)  fprintf(logfile,"jewels.png load fail");;
        if ((status_bg = al_load_bitmap("status_bg.png")) == NULL)  fprintf(logfile,"status_bg.png load fail");;
		if ((panel_bmp = al_load_bitmap("panel.png")) == NULL)  fprintf(logfile,"panel.png load fail");;
		if ((panel_pressed_bmp = al_load_bitmap("panel_pressed.png")) == NULL)  fprintf(logfile,"panel_pressed.png load fail");;

        make_bullet_bitmap();

		fprintf(logfile,"Init Ships\n");	//init ship structs
		init_ships(MAX_SHIPS);				//read stuff from map struct.
		fprintf(logfile,"Init Bullets\n");  //zeroing out the array
		init_bullets();

		fprintf(logfile,"Init done\n");
		fflush(logfile);
        display_map_text(true,30);	//this is the description text file, plus 'press fire' message

        //wait for fire(thrust) to clear text / enter map
        while(1)
        {
            al_wait_for_event(queue, &event);

            if (event.type == ALLEGRO_EVENT_TIMER)
            {
                if (gpio_active)
                	ReadGPIOJoystick();
			}

            else if (event.type == ALLEGRO_EVENT_KEY_DOWN)
            {
                if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
                    Exit();
                else
                    key_down_log[event.keyboard.keycode]=true;
            }

            else CheckUSBJoyStick(event);

            ScanInputs(num_ships);

			for (i=0 ; i<num_ships ; i++)
			{
				if (Ship[i].thrust_down || key_down_log[ALLEGRO_KEY_ENTER])
            	{
					Ship[i].thrust_down = false;
					key_down_log[ALLEGRO_KEY_ENTER] == false;
					break;			//out of the for loop
				}
			}
			if (i<num_ships) break;	//if we came out of for() loop early, must have had a button, so exit while() loop.
        }

        FreeMenuBitmaps();

        if (Map.mission)							//start timer
        {
			Map.race = true;
			Ship[i].current_lap_time = 0;
			Ship[i].racing = true;
		}

		//Main loop here
		while (1)
		{
			//al_play_sample(wind, 1, 0, 1, ALLEGRO_PLAYMODE_LOOP, NULL);

			al_wait_for_event(queue, &event);

			if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
				break;

			if (event.type == ALLEGRO_EVENT_KEY_DOWN)
			{
				//DEBUG THINGS
				if (debug_on)
				{
					if (event.keyboard.keycode == ALLEGRO_KEY_S)//DEBUG; SUICIDE
					{
						Ship[0].shield = 0;
					}
					if (event.keyboard.keycode == ALLEGRO_KEY_G)
					{
						grid++;
						if (grid > MAX_GRID) grid = 0;
					}

				}
				//END DEBUG

				if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE) break;//Exit();

				else if  (event.keyboard.keycode == ALLEGRO_KEY_PRINTSCREEN)
				{
					take_screenshot = true;
					fprintf(logfile,"Screenshot %d\n",screenshot_count);
				}
				else
				{
					pressed_keys[event.keyboard.keycode]=true;
					key_down_log[event.keyboard.keycode]=true;
				}
			}

			if (event.type == ALLEGRO_EVENT_KEY_UP)
			{
				pressed_keys[event.keyboard.keycode]=false;
				key_up_log[event.keyboard.keycode]=true;
			}

			//USB Joystick events here
			CheckUSBJoyStick(event);

			//clients post to network server???

			if (event.type == ALLEGRO_EVENT_TIMER)
				redraw = true;
			if (event.type == ALLEGRO_EVENT_DISPLAY_RESIZE) {
				al_acknowledge_resize(display);
				redraw = true;
			}

			if (redraw && al_is_event_queue_empty(queue))
			{
				if (gpio_active) ReadGPIOJoystick();	//No events, so have to read this in the timer loop.

				double t = al_get_time();

				if (take_screenshot)
				{
					take_screenshot = false;	//don't do it again

					sprintf(screenshout_count_str,"%02d",screenshot_count++);	//increment number for filename

					strcpy(screenshot_name,screenshot_basename);	//init string
					strcat(screenshot_name,screenshout_count_str);	//append number
					strcat(screenshot_name,screenshot_ext);			//append extension

					al_save_bitmap(screenshot_name, al_get_backbuffer(display));

					fprintf(logfile,"Screenshot %d saved\n",screenshot_count-1);
				}

				al_flip_display();
				fps_accum++;
				if (t - fps_time >= 1)
				{
					fps = fps_accum;
					fps_accum = 0;
					fps_time = t;
				}

				al_clear_to_color(al_map_rgb(0, 0, 0));	//clear the screen

				//Single draw_split_screen function
				//needs paramter FULL, TOP, BOTTOM, TL, TR, BL, BR
				//it gets window/screen coords from this and figures out where to put the ship etc.
				//Ship numbers passed in. This is used to centre the 'viewport' on the ship
				switch(num_ships)
				{
				case 1:
						draw_split_screen(FULL,0);
						break;
				case 2:
						draw_split_screen(TOP,1);
						draw_split_screen(BOTTOM,0);
						break;
				case 3:
				case 4:
						//draw_split_screen(TOPLEFT,0);
						//draw_split_screen(TOPRIGHT,1);
						//draw_split_screen(BOTTOMLEFT,2);
						//draw_split_screen(BOTTOMRIGHT,3);
						draw_split_screen(TOPLEFT,2);		//better for sitimus....
						draw_split_screen(TOPRIGHT,1);
						draw_split_screen(BOTTOMLEFT,0);
						draw_split_screen(BOTTOMRIGHT,3);
						break;
				}

				draw_status_bar(num_ships);	//also does dividers

				if (debug_on) draw_debug();

				ScanInputs(num_ships);		//Convert keypresses/joystick input to ship controls (for n ships)

				if (game_over)
				{
					//al_stop_samples();	//stop the audio

					if (!GameOver()) break;	//0 returned, break out of inner while(1) loop. Still in outer, so we'll re-init stuff
				}
				else
				{
					game_over = UpdateShips(num_ships);		//Calculate Ship positions based on controls
															//Also handle firing, updating fuel, ammo, lives etc.
															//if lives go down to 0, return game_over countdown.

					UpdateSentries();	//Automatic guns / volcanoes
					UpdateBullets();	//Calculate new bullet positions, expire old ones.

					//if (Map.type == 0)
					{
						//check for collisions, modify shield parameter for ships, ttl for bullets.
						CheckSSCollisions(num_ships);//Ship-to-ship collisions
						CheckBSCollisions(num_ships);//bullet-to-ship collisions
						CheckBSentryCollisions();
						CheckSWCollisions(num_ships);//Ship-to-wall collisions
						CheckBWCollisions();		 //Bullet-to-wall collisions
					}
				}

				redraw = FALSE;
			}
		}
		fprintf(logfile,"Game Over\n");
        FreeGameBitmaps();
        fflush(logfile);
	}
    return 0;
}


/****************************************************
** int GameOver()
** Draw increasing white overlay on screen.
** UpdateShips() isn't called while game_over > 0,
** so screen is effectively frozen.
** Decrement game_over timer and display GAME OVER
** text when expired, then wait for thrust key/button to exit
****************************************************/
int GameOver()
{
	int i,temp,score;

	//	draw game over screen, gradually getting whiter
	temp = 2*(100-game_over);
	al_set_clipping_rectangle(0, 0, SCREENX, SCREENY);
	al_draw_filled_rectangle(0,0,SCREENX,SCREENY,al_map_rgba(temp,temp,temp,temp));

	if (game_over > 1)					//decrement timer
		game_over--;
	else								//if timer has expired (to 1)
	{
		//draw the text
		if (Map.mission)
		{
			score = 0;
			score += Ship[0].miners*500;
			score += Ship[0].jewels*200;
			score += Ship[0].sentries*100;

			if (Ship[0].lives)
				al_draw_textf(menu_font, al_map_rgb(0,0,0),100,  100,  ALLEGRO_ALIGN_LEFT, "MISSION COMPLETE");
			else
				al_draw_textf(menu_font, al_map_rgb(0,0,0),100,  100,  ALLEGRO_ALIGN_LEFT, "MISSION FAILED");

			al_draw_textf(menu_font, al_map_rgb(0,0,0),100,  200,  ALLEGRO_ALIGN_LEFT, "RESCUED");
			al_draw_textf(menu_font, al_map_rgb(0,0,0),400,  200,  ALLEGRO_ALIGN_LEFT, "%02d x 500= ",Ship[0].miners);
			al_draw_textf(menu_font, al_map_rgb(0,0,0),800,  200,  ALLEGRO_ALIGN_RIGHT, "%d",Ship[0].miners*500);


			al_draw_textf(menu_font, al_map_rgb(0,0,0),100,  250,  ALLEGRO_ALIGN_LEFT, "JEWELS");
			al_draw_textf(menu_font, al_map_rgb(0,0,0),400,  250,  ALLEGRO_ALIGN_LEFT, "%02d x 200= ",Ship[0].jewels);
			al_draw_textf(menu_font, al_map_rgb(0,0,0),800,  250,  ALLEGRO_ALIGN_RIGHT, "%d",Ship[0].jewels*200);


			al_draw_textf(menu_font, al_map_rgb(0,0,0),100,  300,  ALLEGRO_ALIGN_LEFT, "SENTRIES");
			al_draw_textf(menu_font, al_map_rgb(0,0,0),400,  300,  ALLEGRO_ALIGN_LEFT, "%02d x 100 = ",Ship[0].sentries);
			al_draw_textf(menu_font, al_map_rgb(0,0,0),800,  300,  ALLEGRO_ALIGN_RIGHT, "%d",Ship[0].sentries*100);

			if (Ship[0].lives)
			{
				al_draw_textf(menu_font, al_map_rgb(0,0,0),100,  350,  ALLEGRO_ALIGN_LEFT, "TIME BONUS");

				temp = Map.time_limit - Ship[0].current_lap_time;
				if (temp < 0) temp = 0;

				score += temp*100;
				al_draw_textf(menu_font, al_map_rgb(0,0,0),400,  350,  ALLEGRO_ALIGN_LEFT, "%02d x 100 = ",temp);
				al_draw_textf(menu_font, al_map_rgb(0,0,0),800,  350,  ALLEGRO_ALIGN_RIGHT, "%d",temp*100);
			}

			al_draw_textf(menu_font, al_map_rgb(0,0,0),100,  400,  ALLEGRO_ALIGN_LEFT, "SCORE:");
			al_draw_textf(menu_font, al_map_rgb(0,0,0),800,  400,  ALLEGRO_ALIGN_RIGHT, "%d",score);

			/*
			score += Ship[0].miners*500;

			al_draw_textf(menu_font, al_map_rgb(0,0,0),250,  250,  ALLEGRO_ALIGN_LEFT, "%02d x 200 = %d",Ship[0].jewels,Ship[0].jewels*200);
			score += Ship[0].jewels*200;

			al_draw_textf(menu_font, al_map_rgb(0,0,0),250,  300,  ALLEGRO_ALIGN_LEFT, "%02d x 10 = %d",Ship[0].sentries,Ship[0].sentries*100);
			score += Ship[0].sentries*100;

			temp = Ship[0].current_lap_time - Map.time_limit;

			al_draw_textf(menu_font, al_map_rgb(0,0,0),250,  350,  ALLEGRO_ALIGN_LEFT, "%02d x 10 = %d",temp,temp*10);
			score += temp*10;
			al_draw_textf(menu_font, al_map_rgb(0,0,0),300,  400,  ALLEGRO_ALIGN_LEFT, "%d",score);
			*/

		}
		else
		{
			al_draw_textf(big_font, al_map_rgb(0,0,0),SCREENX/2,  SCREENY/8,  ALLEGRO_ALIGN_CENTER, "GAME");
			al_draw_textf(big_font, al_map_rgb(0,0,0),SCREENX/2, 5*SCREENY/8,  ALLEGRO_ALIGN_CENTER, "OVER");
		}
		for(i=0 ; i<num_ships ; i++)	//and enable check for each ships fire/thrust button to go back to start (menu)
		{
			if(Ship[i].thrust_down)
			{
				Ship[i].thrust_down = false;
				game_over = 0;
			}
		}
	}
	return game_over;	//return 0 when we want to restart the game
}


int read_maps(void)
{
	int i,j,group = -1,map = 0,len;
	char temp[MAP_NAME_LENGTH];

	FILE* mapfile;

	mapfile = fopen("maps.txt","r");

	if (mapfile == NULL)
		fprintf(logfile,"Failed to open file maps.txt\n");

	for (i=0 ; /*i<MAX_MAPS*/ ; i++)
	{
		//if (fscanf(mapfile,"%s",&map_names[MAP_NAME_LENGTH * i]) == EOF) break;
		//if (fscanf(mapfile,"%s",&temp) == EOF) break;
		if (fgets(temp,MAP_NAME_LENGTH,mapfile) == NULL) break;

		len = strcspn(temp, "\r\n");

		if (len == 0) break;

		temp[len] = 0;

		//temp[strlen(temp)]=0;

		//fprintf(logfile,"%s *%d\n",temp,strcspn(temp, " \r\n"));

		if (temp[0] == ':')	//group name
		{
			if (group != -1)
			{
				MapNames[group].Count = map;
				//fprintf(logfile,"Group %d Count = %d\n",group, MapNames[group].Count);
			}
			group++;
			map = 0;

			strncpy ((char*)&MapNames[group].Group, &temp[1], MAP_NAME_LENGTH-1);
			//fprintf(logfile,"Group: %s\n",&MapNames[group].Group);

		}
		else	//map name
		{
			strncpy ((char*)&MapNames[group].Map[map], &temp[0], MAP_NAME_LENGTH);
			//fprintf(logfile,"Map: %s\n",&MapNames[group].Map[map]);
			map++;
		}

		//fprintf(logfile,"%s\n",&map_names[MAP_NAME_LENGTH * i]);
	}

	MapNames[group].Count = map;
	//fprintf(logfile,"Group %d Count = %d\n",group, MapNames[group].Count);

	fclose(mapfile);

	fprintf(logfile,"%d Map groups\n",group+1);

	for (i=0 ; i<group+1 ; i++)
	{
		fprintf(logfile,"Group: %s\n",(char*)&MapNames[i].Group);
		for (j=0 ; j<MapNames[i].Count ; j++)
		{
			fprintf(logfile,"Map: %s\n",(char*)&MapNames[i].Map[j]);
		}
	}

	return group+1;
}

void DoTitle(ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_EVENT event)
{
	ALLEGRO_TRANSFORM transform;
	int count = 0;
	//float scale;

	float ht = 100;	//total height (light)
	float hc = 10;	//camera height
	float y = 0;	//how far the text has fallen
	float a = 10;	//width of text
	float shadow_scale, text_scale;
	int sound_latency = 15;

	char fade_in[200],fade_out[200],visible[200];
	int fade_in_y = 600, visible_y = 570, fade_out_y = 540;
	int fade_count=0;
	float alpha;

	FILE* credits;

	menu_bg_bmp = al_load_bitmap("menu_bg.png");
	credits = fopen ("credits.txt","r");
	fade_in[0] = 0;
	visible[0] = 0;
	fade_out[0] = 0;

    voice = al_create_voice(44100, ALLEGRO_AUDIO_DEPTH_INT16, ALLEGRO_CHANNEL_CONF_2);
    mixer = al_create_mixer(44100, ALLEGRO_AUDIO_DEPTH_INT16, ALLEGRO_CHANNEL_CONF_2);
    al_set_default_mixer(mixer);
    al_attach_mixer_to_voice(mixer, voice);
    wind_inst = al_create_sample_instance(wind);
    al_attach_sample_instance_to_mixer(wind_inst, mixer);
    al_set_sample_instance_playmode(wind_inst, ALLEGRO_PLAYMODE_LOOP);

	al_play_sample_instance(wind_inst);//, 1, 0, 1, ALLEGRO_PLAYMODE_LOOP, NULL);

	//al_play_sample(wind, 5, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);

	while(1)
	{
		al_wait_for_event(queue, &event);

		if (event.type == ALLEGRO_EVENT_TIMER)
		{
			if (y < ht) y++;

			al_set_sample_instance_gain(wind_inst,7*y/ht);
			if (y == ht-sound_latency)
				al_play_sample(clunk, 5, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);

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
				al_draw_textf(title_font, al_map_rgb(128, 128, 0),0, -1*al_get_font_ascent(title_font)/2,  ALLEGRO_ALIGN_CENTRE, "%s", NAME);
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
	FreeMenuBitmaps();
	return;
}

int DoMenu(ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_EVENT event)
{
	int i;
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

	Menu.num_groups = read_maps();	//read 'maps.txt' and store strings
	//RETURN VALUE HERE
	//MAYBE NEED CURRENT GROUP / CURRENT MAP in struct somewhere???


	//Menu.map = 0;
	Menu.col = 0;
	Menu.offset = 0;

	menu_bg_bmp = al_load_bitmap("menu_bg.png");

	fprintf(logfile,"\nSelected map %d,%d\n",Menu.group,Menu.map);

	//REDUNDANT
	//if (map_names[MAP_NAME_LENGTH * Menu.map] == ':')
	//{
	//	Menu.map++;
	//	fprintf(logfile,"Selected map %d\n",Menu.map);
	//}

	if (init_map(Menu.group, Menu.map))		//opens 'map name.txt' and copies into MapTypeStruct.
	{								//everything else (load map, make col map, init ships etc. reads from MapType struct
		fprintf(logfile,"Failed to open map file\n");
		return 0;
	}

	while(1)
	{
		al_wait_for_event(queue, &event);

		if (event.type == ALLEGRO_EVENT_TIMER && al_is_event_queue_empty(queue))
		{
			display_menu();//Menu.num_maps, Menu.map);	//show strings, highlight selected

			//sliding columns
			if (Menu.col >= 2)
			{
				if (Menu.offset > -460)
					Menu.offset -= 20;
			}
			else
			{
				if (Menu.offset < 0)
					Menu.offset += 20;
			}

			//expanding maps
			if (Menu.expand < 55)
				Menu.expand += 5;

            if (gpio_active) ReadGPIOJoystick();
		}

		else if (event.type == ALLEGRO_EVENT_KEY_DOWN)
		{
			al_play_sample(clunk, 1, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
            //al_play_sample_instance(clunk_inst);

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


						/*
						if (Menu.map < Menu.num_maps-1)
						{
							Menu.map++;

							if (map_names[MAP_NAME_LENGTH * Menu.map] == ':')
								Menu.map++;

							fprintf(logfile,"\nSelected map %d\n",Menu.map);
							init_map(Menu.map);
						}
						*/
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


						/*
						if (Menu.map > 0)
						{
							Menu.map--;

							if (map_names[MAP_NAME_LENGTH * Menu.map] == ':')
								Menu.map--;

							init_map(Menu.map);
							fprintf(logfile,"Selected map %d\n",Menu.map);
						}
						*/
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

						//if (Ship[Menu.player].controller < 0)
						//	Ship[Menu.player].controller = 0;


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

void LoadMap(void)  //different function for tilemaps? Or just a smarter one....
{
    fprintf(logfile,"tr_map = %d\n",tr_map);
    fflush(logfile);

    tr_map = al_load_bitmap(Map.display_file_name);
	if (Map.type == 0)
	{
    	mapx = 2*al_get_bitmap_width(tr_map);
    	mapy = 2*al_get_bitmap_height(tr_map);
	}
	else
	{
		mapx = map_width * tile_width;
		mapy = map_height * tile_height;
	}

	if (Map.num_sentries)
		sentries = al_load_bitmap(Map.sentry_file_name);

    fprintf(logfile,"tr_map size %d x %d\n",mapx,mapy);
    fflush(logfile);
}

int FreeGameBitmaps(void)
{
    int i=0,j=0;;

    fprintf(logfile,"Freeing game bitmaps\n");
    if (ships)
    {
        al_destroy_bitmap(ships);
        ships = NULL;
        i++;
    }
    j++;

    if (status_bg)
    {
        al_destroy_bitmap(status_bg);
        status_bg = NULL;
        i++;
    }
    j++;

    if (panel_bmp)
    {
        al_destroy_bitmap(panel_bmp);
        panel_bmp = NULL;
        i++;
    }
    j++;

    if (panel_pressed_bmp)
    {
        al_destroy_bitmap(panel_pressed_bmp);
        panel_pressed_bmp = NULL;
        i++;
    }
    j++;

    if (bullets_bmp)
    {
        al_destroy_bitmap(bullets_bmp);
        bullets_bmp = NULL;
        i++;
    }
    j++;

    if (tr_map)
    {
        al_destroy_bitmap(tr_map);
        tr_map = NULL;
        i++;
    }
    j++;

    if (sentries)
    {
        al_destroy_bitmap(sentries);
        sentries = NULL;
        i++;
    }
    j++;

    if (pickups)
    {
        al_destroy_bitmap(pickups);
        pickups = NULL;
        i++;
    }
    j++;

    if (miner)
    {
        al_destroy_bitmap(miner);
        miner = NULL;
        i++;
    }
    j++;

    if (jewel)
    {
        al_destroy_bitmap(jewel);
        jewel = NULL;
        i++;
    }
    j++;

    fprintf(logfile,"Freed %d/%d game bitmaps\n",i,j);
    fflush(logfile);

    return;
}

void FreeMenuBitmaps()
{
    int i=0;

    if (menu_bg_bmp)
    {
        al_destroy_bitmap(menu_bg_bmp);
        menu_bg_bmp = NULL;
        i++;
    }

    fprintf(logfile,"Freed %d menu bitmaps\n",i);
    fflush(logfile);
}

void FreeFonts(void)
{
	al_destroy_font(font      );
	al_destroy_font(menu_font );
	al_destroy_font(small_font);
	al_destroy_font(big_font  );
	al_destroy_font(race_font );
	al_destroy_font(title_font);
	fprintf(logfile,"Freed 6 fonts\n");
}

void Exit(void)
{
	fprintf(logfile,"Exiting\n");
    FreeGameBitmaps();
    FreeMenuBitmaps();
    FreeFonts();
    al_destroy_display(display);
	fclose(logfile);
	exit(0);
}

extern int map_idx_x, map_idx_y;
extern int map_idx, map_word1, map_word2;
extern int shift1, shift2;
extern int ship_word1, ship_word1_shift;
extern int ship_word2, ship_word2_shift;
extern int collision;
extern float x_dis, y_dis, distance, direction;

extern int tile_x, tile_y, tile_idx, tile, tl_tile;
extern int start_row,current_row,rows;


void draw_debug(void)
{
	int i;

	int level;

	if (Map.mission) level = num_ships*150;
	else             level = num_ships*120;


	al_draw_textf(font, al_map_rgb(255, 255, 255),0, level,  ALLEGRO_ALIGN_LEFT, "FPS: %d", fps);
	al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "X: %.0f", Ship[0].xpos);
	al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Y: %.0f", Ship[0].ypos);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Mapx: %d", mapx);

	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "M: %d", Ship[0].miners);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "J: %d", Ship[0].jewels);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Score: %d", Ship[0].sentries);

	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Angle: %d", ANGLE_INC*Ship[0].angle);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Landed: %d", Ship[0].landed);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Pad: %d", Ship[0].pad);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Home Pad: %d", Ship[0].home_pad);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Menu: %d", Ship[0].menu);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Recharge: %d", Ship[0].recharge_timer);

	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "A1: %d", Ship[0].ammo1_type);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "A2: %d", Ship[0].ammo2_type);

	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Mass[0]: %d", Ship[0].mass);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Race: %d", Map.race);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "approaching: %d", Ship[0].approaching);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "racing: %d", Ship[0].racing);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "FT: %f", FrameTime);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Lap: %0.3f", Ship[0].current_lap_time);

	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "BD: %d", GPIOJoystick.button_down);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "BU: %d", GPIOJoystick.button_up);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "B: %d", GPIOJoystick.button);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "ThD: %d", Ship[1].thrust_down);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "ThH: %d", Ship[1].thrust_held);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Th: %d", Ship[1].thrust);

	//targetted sentries
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "x_dis: %.1f", x_dis);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "y_dis: %.1f", y_dis);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "distance: %.1f", distance);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "direction: %.1f", direction);

	//heatseekers/black holes
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "X: %.1f", x);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Y: %.1f", y);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "R^2: %.1f", r_squared);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "R: %.1f", r);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "xg: %.1f", xg);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "yg: %.1f",yg);

	//for (i=0 ; i<al_get_num_joysticks() ; i++)
	//{
	//	al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Joy%d L %d",i,USBJoystick[i].left_down);
	//	al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Joy%d R %d",i,USBJoystick[i].right_down);
	//	al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Joy%d U %d",i,USBJoystick[i].up_down);
	//	al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Joy%d D %d",i,USBJoystick[i].down_down);
	//	al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Joy%d B %d",i,USBJoystick[i].button_down);
	//}


	//bullets
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "1stBull: %d", first_bullet);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "lastBull: %d", last_bullet);
	//if(last_bullet == END_OF_LIST)
	//	al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "LB.ttl: N/A");
	//else
	//{
	//	al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "LB.ttl: %04X", Bullet[last_bullet].ttl);
	//	al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "LB.ttl mask: %04X", Bullet[last_bullet].ttl & 0x003F);
	//	al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "LB.yv: %f", Bullet[last_bullet].yv);
	//}

	//Ship-to-wall collisions
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "MX:%d", map_idx_x);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "MY:%d", map_idx_y);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "MI:%d", map_idx);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Sh1:%d", shift1);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Sh2:%d", shift2);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "MW1: %08X", map_word1);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "SW1S:%08X", ship_word1_shift);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "MW2: %08X", map_word2);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "SW2S:%08X", ship_word2_shift);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "SW:%08X", ship_word1);

	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "S0.s:%d", Map.sentry[0].shield);

	//Tiled
	al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "TX:%d", tile_x);
	al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "TY:%d", tile_y);
	al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "TI:%d", tile_idx);
	al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Tile:%d", tl_tile);
	al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Sh1:%d", shift1);
	al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Sh2:%d", shift2);
	al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "MW1: %08X", map_word1);
	al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "SW1:%08X", ship_word1);
	al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "SW1S:%08X", ship_word1_shift);
	al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "SR: %d", start_row);
	al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "CR:%d", current_row);
	al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Rows:%d", rows);
	al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Collision:%d", collision);

	//Bullet to ship collisions
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "SW:%08X", ship_word);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "BW: %08X", bullet_word);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "YO:%d", y_offset);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "SH:%d", shift);

	//Ship to ship collisions
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "YOv:%d", y_overlap);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "YOf:%d", y_offset);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "IR:%d", i_row);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "JR:%d", j_row);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Sh:%d", shift);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "SiW: %08X", shipi_word);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "SjW:%08X", shipj_word);

	//Bullet to wall collisions
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "MX:%d", map_idx_x);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "MY:%d", map_idx_y);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "MI:%d", map_idx);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Sh:%d", shift);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "MW: %08X", map_word);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "BW:%08X", bullet_word);

	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "C:%d", collision);

	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "JDS:%d", joystick_down_state);
}
