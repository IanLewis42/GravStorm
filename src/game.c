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
#include <math.h>

#define ALLEGRO_UNSTABLE 1  //needed for haptics.

#include "allegro5/allegro.h"
#include "allegro5/allegro_image.h"
#include "allegro5/allegro_primitives.h"
#include "allegro5/allegro_font.h"
#include "allegro5/allegro_ttf.h"
#include "allegro5/allegro_audio.h"
#include "allegro5/allegro_acodec.h"
#ifdef ANDROID
  #include <allegro5/allegro_android.h>
#endif

#include "game.h"
#include "drawing.h"
#include "init.h"
#include "collisions.h"
#include "objects.h"
#include "inputs.h"
#include "gameover.h"
#include "network.h"
#include "auto.h"

#ifdef RPI
#include <wiringPi.h>
#endif


ALLEGRO_DISPLAY *display;
ALLEGRO_HAPTIC *hap;
ALLEGRO_HAPTIC_EFFECT_ID *hapID;

ALLEGRO_TIMER *timer;

ALLEGRO_FONT *font;         //debug
ALLEGRO_FONT *menu_font;
ALLEGRO_FONT *glow_font;
ALLEGRO_FONT *small_font;
ALLEGRO_FONT *small_glow_font;
ALLEGRO_FONT *big_font;
ALLEGRO_FONT *race_font;
ALLEGRO_FONT *title_font;
float font_scale;
//float font_scale_y;

int fps, fps_accum;
double fps_time;
int fpsnet, fpsnet_acc;
int missed_packets = 0;

//Bitmaps
ALLEGRO_BITMAP *logo;
ALLEGRO_BITMAP *ships;
ALLEGRO_BITMAP *grey_ships;
ALLEGRO_BITMAP *status_bg;
ALLEGRO_BITMAP *panel_bmp;
ALLEGRO_BITMAP *panel_pressed_bmp;
ALLEGRO_BITMAP *bullets_bmp;
ALLEGRO_BITMAP *tr_map;
ALLEGRO_BITMAP *background;
ALLEGRO_BITMAP *sentries;
ALLEGRO_BITMAP *ui;
ALLEGRO_BITMAP *pickups;
ALLEGRO_BITMAP *miner;
ALLEGRO_BITMAP *jewel;
ALLEGRO_BITMAP *icon;
ALLEGRO_BITMAP *menu_bg_bmp;
//ALLEGRO_BITMAP *ctrl_direction;
//ALLEGRO_BITMAP *ctrl_thrust;
//ALLEGRO_BITMAP *ctrl_escape;

//Sounds
ALLEGRO_VOICE *voice;
//ALLEGRO_VOICE dummy_voice;
ALLEGRO_MIXER *mixer;

ALLEGRO_SAMPLE *slam;
ALLEGRO_SAMPLE *clunk;
ALLEGRO_SAMPLE *wind;
ALLEGRO_SAMPLE *shoota;
ALLEGRO_SAMPLE *shootb;
ALLEGRO_SAMPLE *dead;
ALLEGRO_SAMPLE *particle;
ALLEGRO_SAMPLE *yippee;
ALLEGRO_SAMPLE *loop;

ALLEGRO_SAMPLE_INSTANCE *clunk_inst;
ALLEGRO_SAMPLE_INSTANCE *wind_inst[MAX_SHIPS];
ALLEGRO_SAMPLE_INSTANCE *shoota_inst[MAX_SHIPS];
ALLEGRO_SAMPLE_INSTANCE *shootb_inst[MAX_SHIPS];
ALLEGRO_SAMPLE_INSTANCE *dead_inst[MAX_SHIPS];
ALLEGRO_SAMPLE_INSTANCE *particle_inst[MAX_SHIPS];
ALLEGRO_SAMPLE_INSTANCE *sentry_particle_inst;
ALLEGRO_SAMPLE_INSTANCE *yippee_inst;

MapType Map;
MenuType Menu;
RadarType Radar;
CtrlsType Ctrl;
CommandType Command;
ScaleType Scale;

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

int mapx, mapy;
//int tile_width = 64, tile_height = 64;

int gpio_active = false;
int debug_on = false;
int tracking = false;
int d=0;    //index for ship array for debug
bool pressed_keys[ALLEGRO_KEY_MAX];

bool take_screenshot = false;
int screenshot_count = 0;
char screenshout_count_str[3];
char screenshot_basename[5] = {'S','c','r','_',0};
char screenshot_ext[5] = {'.','p','n','g',0};
char screenshot_name[20];

int debug_key = 0;
int keypress = false;
char current_key = 0;
float volume, v_squared;
bool redraw = false;
float scale,invscale;
int halted = 0;
int vibrate_time = 0;
int vibrate_timer = 0;

ALLEGRO_FILE* logfile;



//Local prototypes
int  DoTitle(ALLEGRO_EVENT_QUEUE *queue);//, ALLEGRO_EVENT event);
int  DoMenu(ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_EVENT event);
int  GameOver(void);
//int  FireOrEscape(ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_EVENT event);
void ForwardOrBack(ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_EVENT event);
void FreeGameBitmaps(void);
void FreeFonts(void);
void StopNetwork(void);
void draw_debug(void);

#define FIRE 1
#define ESCAPE 2

#ifndef ANDROID
int game(int argc, char **argv );
int main (int argc, char *argv[])
{
	game (argc, argv);
	return 0;
}

#endif // ANDROID


int game(int argc, char **argv )
{


#if 0   //check haptic installation

    /* Init Allegro 5 + addons. */
    al_init();

    //init other bits of allegro
    al_init_image_addon();
    al_init_primitives_addon();
    al_init_font_addon();
    al_init_ttf_addon();
    al_install_mouse();
    al_install_keyboard();
    al_install_audio();
    al_init_acodec_addon();
    al_android_set_apk_fs_interface();

    al_set_new_display_option(ALLEGRO_SUPPORTED_ORIENTATIONS,ALLEGRO_DISPLAY_ORIENTATION_LANDSCAPE,ALLEGRO_REQUIRE);

    display = al_create_display(960, 540);  //Moto E resolution

    ALLEGRO_TOUCH_INPUT *touch;
    int temp = al_install_haptic();
    if (al_is_haptic_installed())
    {
        if (al_is_display_haptic(display))
            hap = al_get_haptic_from_display(display);

        else if (al_is_touch_input_haptic(touch))
            al_get_haptic_from_touch_input(touch);
    }

    while(1);

#endif

    ALLEGRO_EVENT_SOURCE *EVSRC_KEYBOARD;
    ALLEGRO_EVENT_SOURCE *EVSRC_DISPLAY;
    ALLEGRO_EVENT_SOURCE *EVSRC_JOYSTICK;
    ALLEGRO_EVENT_SOURCE *EVSRC_TOUCH;
    ALLEGRO_EVENT_SOURCE *EVSRC_TIMER;

//int main (int argc, char *argv[]){
//int main(void) {
    ALLEGRO_EVENT_QUEUE *queue;
    ALLEGRO_EVENT event;

	int i;//,j,k,temp;
	//int num_maps,selected_map;
	int exit, back_to_menu = false;
    float scalex,scaley;//scale;

	//parse command line arguments
	for (i=1 ; i<argc ; i++)
	{
		if (strncmp(argv[i],"-g",2) == 0) gpio_active = true;	//Enable GPIO joystick with -g switch
		if (strncmp(argv[i],"-d",2) == 0) debug_on = true;		//Enable debug output
	}

    //debug_on = true;		//Enable debug output

	/* Init Allegro 5 + addons. */
    al_init();

    char pathstr[100];
    //char *pathptr;

    al_set_standard_file_interface();
    ALLEGRO_PATH *path = al_get_standard_path(ALLEGRO_RESOURCES_PATH);    //win/linux
    //ALLEGRO_PATH *path = al_get_standard_path(ALLEGRO_USER_DATA_PATH);    //android
    //ALLEGRO_DEBUG(#std ": %s", al_path_cstr(path, '/'));
    sprintf(pathstr,"%s",al_path_cstr(path, '/'));
	al_change_directory(al_path_cstr(path, '/'));  // change the working directory
    //pathptr = al_get_current_directory();

    logfile = al_fopen("logfile.txt","w");

    al_fprintf(logfile,"log open\n");
    al_fflush(logfile);

    //init other bits of allegro
    al_init_image_addon();
    al_init_primitives_addon();
    al_init_font_addon();
    al_init_ttf_addon();
    al_install_mouse();
    al_install_keyboard();
    al_install_audio();
	al_init_acodec_addon();
#ifdef ANDROID
    al_android_set_apk_fs_interface();
#endif
	//al_fprintf(logfile,"Init Allegro done\n");

    /* Create our window. */
    #if RPI
    al_set_new_display_flags(ALLEGRO_FULLSCREEN_WINDOW);//ALLEGRO_WINDOWED);// | ALLEGRO_RESIZABLE);
    #else
    #ifdef _WIN32
    al_set_new_display_flags(ALLEGRO_RESIZABLE);//ALLEGRO_WINDOWED);// | ALLEGRO_RESIZABLE);
    #endif
    #endif // _WIN32
#ifdef ANDROID
    al_set_new_display_option(ALLEGRO_SUPPORTED_ORIENTATIONS,ALLEGRO_DISPLAY_ORIENTATION_LANDSCAPE,ALLEGRO_REQUIRE);
#endif

    //al_fprintf(logfile,"Creating display\n");
    display = al_create_display(SCREENX, SCREENY);
    //display = al_create_display(960, 540);  //Moto E resolution
    //display = al_create_display(800, 480);  //Cheap android phone resolution

	//change directory to data, where all resources live (images, fonts, sounds and text files)

    al_append_path_component(path, "data");
	al_change_directory(al_path_cstr(path, '/'));  // change the working directory
    sprintf(pathstr,"%s",al_path_cstr(path, '/'));

    //_jni_callVoidMethodV(_al_android_get_jnienv(), _al_android_activity_object(), "HideNavBar", "()V");

#ifdef ANDROID
    ALLEGRO_TOUCH_INPUT *touch;
    int temp = al_install_haptic();
    if (al_is_haptic_installed())
    {
        if (al_is_display_haptic(display))
            hap = al_get_haptic_from_display(display);

        else if (al_is_touch_input_haptic(touch))
            al_get_haptic_from_touch_input(touch);
    }
#endif

    al_set_window_title(display, NAME);

    al_set_new_bitmap_flags(ALLEGRO_VIDEO_BITMAP);// | ALLEGRO_MAG_LINEAR | ALLEGRO_MIN_LINEAR);

    scalex = (float)al_get_display_width(display)/SCREENX;
    scaley = (float)al_get_display_height(display)/SCREENY;

    if (scalex < scaley) scale = scalex;
    else scale = scaley;

    invscale = 1/scale;
    CalcScales();

    al_fprintf(logfile,"Display Created. Scale = %f, i/scale = %f\n",scale,invscale);
    al_fflush(logfile);
#ifdef ANDROID
    al_android_set_apk_file_interface();
#endif
    if ((icon = al_load_bitmap("gs_icon.png")) == NULL)
        al_fprintf(logfile,"gs_icon.png load fail\n");

    al_fflush(logfile);
    if (icon) al_set_display_icon(display, icon);

    LoadFonts(scale);
    if (al_is_audio_installed())
    {
        voice = al_create_voice(44100, ALLEGRO_AUDIO_DEPTH_INT16, ALLEGRO_CHANNEL_CONF_2);
        mixer = al_create_mixer(44100, ALLEGRO_AUDIO_DEPTH_INT16, ALLEGRO_CHANNEL_CONF_2);
        al_set_default_mixer(mixer);
        al_attach_mixer_to_voice(mixer, voice);
        al_fprintf(logfile,"Setup audio voice and mixer\n");
    }
    else    //get here if no audio available - create mixer to stop rest of code whinging.
        mixer = al_create_mixer(44100, ALLEGRO_AUDIO_DEPTH_INT16, ALLEGRO_CHANNEL_CONF_2);

	al_fflush(logfile);

	al_fprintf(logfile,"Creating Events\n");
	timer = al_create_timer(1.0 / 30);
	queue = al_create_event_queue();
	al_fflush(logfile);

	//Allegro Joystick routines
	//move to inputs/init??
	if (al_install_joystick())	//USB joystick, hopefully
		al_fprintf(logfile,"Init allegro joystick\n");
	al_fprintf(logfile,"%d Joysticks on system\n",al_get_num_joysticks());
	al_fflush(logfile);

	if (al_install_touch_input())
        al_fprintf(logfile,"Init allegro touch input\n");

    EVSRC_KEYBOARD = al_get_keyboard_event_source();
    al_register_event_source(queue,EVSRC_KEYBOARD);

    EVSRC_DISPLAY = al_get_display_event_source(display);
	al_register_event_source(queue, EVSRC_DISPLAY);

    EVSRC_TIMER = al_get_timer_event_source(timer);
	al_register_event_source(queue, EVSRC_TIMER);

    EVSRC_JOYSTICK = al_get_joystick_event_source();
	al_register_event_source(queue,EVSRC_JOYSTICK);

    if (al_is_touch_input_installed())
    {
        EVSRC_TOUCH = al_get_touch_input_event_source();
        al_register_event_source(queue, EVSRC_TOUCH);
    }

	for (i=0 ; i<al_get_num_joysticks() ; i++)
	{
		USBJOY[i] = al_get_joystick(i);
		if (al_get_joystick_active(USBJOY[i]))
			al_fprintf(logfile,"Joystick %d active\n",i);
		else
			al_fprintf(logfile,"Joystick %d NOT active\n",i);
	}

	al_start_timer(timer);
	int timer_running = true;

    init_controls();	//setup defaults for what controls which ship.

    exit = DoTitle(queue);//, event);
    if (exit) return 0;

	al_fflush(logfile);

    init_controls();	//do again now nav bar has gone to get dpad in correct place

    clunk_inst = al_create_sample_instance(clunk);
    al_attach_sample_instance_to_mixer(clunk_inst, mixer);


    //move to objects / init??
    for(i=0 ; i<NUM_ANGLES ; i++)
    {
        sinlut[i] = sin(i*ANGLE_INC_RAD);
        coslut[i] = cos(i*ANGLE_INC_RAD);
    }

    srand(time(NULL));

    if (gpio_active)
	{
		al_fprintf(logfile,"Init GPIO joystick\n");
		init_joystick();	//Ian's GPIO hacked joystick.
	}
	else
		al_fprintf(logfile,"Skip GPIO joystick\n");


    init_ships(MAX_SHIPS);
	al_fflush(logfile);

	Menu.map = 0;	//start on first map, but only first time. After that, remember it.


    //back to here when exiting game
	while(1)
	{
		//LoadFonts();
		Command.goforward = false;

		exit = DoMenu(queue, event);

		if (exit) break;

		al_fprintf(logfile,"Loading map\n");
		al_fflush(logfile);
		LoadMap();	//load TR map. Filenames from map struct.

        al_fprintf(logfile,"Making collision masks\n");
        al_fflush(logfile);
        make_ship_col_mask();
        make_sentry_col_mask();
		make_map_col_mask();

		make_radar_bitmap();
		//if (num_ships > 1)
        Radar.on = true;

        al_fprintf(logfile,"Loading game bitmaps\n");
		al_fflush(logfile);
        if ((ships = al_load_bitmap("ships.png")) == NULL)  al_fprintf(logfile,"ships.png load fail");
        if ((pickups = al_load_bitmap("pickups.png")) == NULL)  al_fprintf(logfile,"pickups.png load fail");
        if ((miner = al_load_bitmap("astronaut.png")) == NULL)  al_fprintf(logfile,"astronaut.png load fail");
        if ((jewel = al_load_bitmap("jewels.png")) == NULL)  al_fprintf(logfile,"jewels.png load fail");
        if ((status_bg = al_load_bitmap("status_bg.png")) == NULL)  al_fprintf(logfile,"status_bg.png load fail");
		if ((panel_bmp = al_load_bitmap("panel.png")) == NULL)  al_fprintf(logfile,"panel.png load fail");
		if ((panel_pressed_bmp = al_load_bitmap("panel_pressed.png")) == NULL)  al_fprintf(logfile,"panel_pressed.png load fail");
        if ((ui = al_load_bitmap("ui.png")) == NULL)  al_fprintf(logfile,"ui.png load fail");
        make_bullet_bitmap();

        CalcScales();   //do again for panel.

		al_fprintf(logfile,"Init Ships\n");	//init ship structs
		init_ships(MAX_SHIPS);				//read stuff from map struct.

		al_fprintf(logfile,"Init Bullets\n");  //zeroing out the array
		init_bullets();

		al_fprintf(logfile,"Init done\n");
		al_fflush(logfile);

        //wait for fire(thrust) to clear text / enter map
        make_map_text_bitmap();
        display_map_text(true,30);	//this is the description text file, plus 'press fire' message

        while(1)
        {
            ServiceNetwork();

            ForwardOrBack(queue, event);    //check input devices for forward/back keys/buttons/touches

            if (Command.goforward)
            {
                Command.goforward = false;
                break;
            }

            else if (Command.goback)
            {
                Command.goback = false;
                back_to_menu = true;
                StopNetwork();          //exit back to menu
                break;
            }
            else if (redraw && al_is_event_queue_empty(queue))
            {
                redraw = false;
                display_map_text(true,30);	//this is the description text file, plus 'press fire' message
            }
        }

        //if client, wait for host
        if (Net.client)
        {
            NetSendReady();

            while(1)
            {
                display_wait_text();
                ServiceNetwork();

                if (Net.started == true)
                {
                    Net.client_state = RUNNING;
                    break;
                }

                ForwardOrBack(queue, event);

                if (Command.goback)
                 {
                    Command.goback = false;
                    back_to_menu = true;
                    StopNetwork();          //exit back to menu
                    break;
                }
            }
        }

        //if server, just go!
        else if (Net.server)
        {
            NetStartGame();
            ServiceNetwork();
        }
        missed_packets = 0;

        FreeMenuBitmaps();

        al_fprintf(logfile,"Game Start\n");

        GameControls();

        //into game here
        for (i=0 ; i<MAX_SHIPS ; i++)
        {
            //Ship[i].thrust = 0; //just in case....

            wind_inst[i] = al_create_sample_instance(wind);
            al_attach_sample_instance_to_mixer(wind_inst[i], mixer);
            al_set_sample_instance_playmode(wind_inst[i], ALLEGRO_PLAYMODE_LOOP);

            if (num_ships > 1)  //if only 1 ship, leave sound centred
            {
                if(i&1) //if more than 1 ship, set alternate ships in opposite L/R channels
                    al_set_sample_instance_pan(wind_inst[i],  1.0);
                else
                    al_set_sample_instance_pan(wind_inst[i], -1.0);
            }
            al_play_sample_instance(wind_inst[i]);

            shoota_inst[i] = al_create_sample_instance(shoota);
            al_attach_sample_instance_to_mixer(shoota_inst[i], mixer);
            shootb_inst[i] = al_create_sample_instance(shootb);
            al_attach_sample_instance_to_mixer(shootb_inst[i], mixer);
            dead_inst[i] = al_create_sample_instance(dead);
            al_attach_sample_instance_to_mixer(dead_inst[i], mixer);
            particle_inst[i] = al_create_sample_instance(particle);
            al_attach_sample_instance_to_mixer(particle_inst[i], mixer);
        }
        yippee_inst = al_create_sample_instance(yippee);
        al_attach_sample_instance_to_mixer(yippee_inst, mixer);
        sentry_particle_inst = al_create_sample_instance(particle);
        al_attach_sample_instance_to_mixer(sentry_particle_inst, mixer);

        if (Map.mission)							//start timer
        {
			Map.race = true;
			Ship[0].current_lap_time = 0;
			Ship[0].racing = true;
		}
		Map.timer = 0;  //timer for auto ships.

        for (i=0 ; i<Menu.ai_ships ; i++)
        {
            GotoTakeoff(num_ships+i);
        }
        num_ships += Menu.ai_ships;


#define DISPLAY 0
#define KEYBOARD 1
#define TOUCH 2
#define TIMER 3
#define JOYSTICK 4

		//Main game loop here
		while (1)
		{
			int evsrc;
            if (back_to_menu)
            {
                back_to_menu = false;
                break;
            }

			al_wait_for_event(queue, &event);

            if (event.any.source == EVSRC_DISPLAY)
                evsrc = DISPLAY;
            if (event.any.source == EVSRC_KEYBOARD)
                evsrc = KEYBOARD;
            if (al_is_touch_input_installed())
                if (event.any.source == EVSRC_TOUCH)
                    evsrc = TOUCH;
            if (event.any.source == EVSRC_TIMER)
                evsrc = TIMER;
            if (event.any.source == EVSRC_JOYSTICK)
                evsrc = JOYSTICK;
            switch(evsrc)
            {
            case  DISPLAY:
                    if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
                    {
                        exit = true;
                        break;
                    }
                    else if (event.type == ALLEGRO_EVENT_DISPLAY_HALT_DRAWING)//  || event.type == ALLEGRO_EVENT_DISPLAY_SWITCH_OUT)    //we've been sidelined by the user/os
                    {
                        al_acknowledge_drawing_halt(display);   //acknowledge
                        halted = true;                          //flag to drawing routines to do nothing
                        al_stop_timer(timer);                   //no more timer events, so we should do nothing, saving battery
                        al_destroy_voice(voice);
                        //break;
                    }
                    else if (event.type == ALLEGRO_EVENT_DISPLAY_RESUME_DRAWING )//|| event.type == ALLEGRO_EVENT_DISPLAY_SWITCH_IN) //we've been restored
                    {
                        al_acknowledge_drawing_resume(display); //ack
                        halted = false;                         //remove flag
                        al_start_timer(timer);                  //restart timer
                        voice = al_create_voice(44100, ALLEGRO_AUDIO_DEPTH_INT16, ALLEGRO_CHANNEL_CONF_2);  //restart audio
                        al_attach_mixer_to_voice(mixer, voice);
                        //break;
                    }
                    else if (event.type == ALLEGRO_EVENT_DISPLAY_RESIZE) {
                        al_acknowledge_resize(display);
                        LoadFonts(0);
                        redraw = true;
                    }
                    else
                    {
                        break;
                    }
            break;
            case KEYBOARD:
                if (event.type == ALLEGRO_EVENT_KEY_DOWN)
                {
                    debug_key = event.keyboard.keycode;

                    //DEBUG THINGS
                    if (debug_on) {
                        if (event.keyboard.keycode == ALLEGRO_KEY_S)//DEBUG; SUICIDE
                        {
                            Ship[0].shield = 0;
                        }
                        if (event.keyboard.keycode == ALLEGRO_KEY_G) {
                            grid++;
                            if (grid > MAX_GRID) grid = 0;
                        }
                        if (event.keyboard.keycode == ALLEGRO_KEY_D) {
                            d++;
                            if (d >= MAX_SHIPS) d = 0;
                        }
                        if (event.keyboard.keycode == ALLEGRO_KEY_SPACE) {

                            if (timer_running)
                            {
                                al_stop_timer(timer);
                                timer_running = false;
                            }
                            else
                            {
                                al_start_timer(timer);
                                timer_running = true;
                            }
                        }
                        //Auto modes
                        if (event.keyboard.keycode == ALLEGRO_KEY_0) {
                            Ship[d].automode = MANUAL;
                        }
                        if (event.keyboard.keycode == ALLEGRO_KEY_1) {
                            GotoTakeoff(d);
                        }
                        if (event.keyboard.keycode == ALLEGRO_KEY_2) {
                            GotoCruise(d);
                        }
                        if (event.keyboard.keycode == ALLEGRO_KEY_3) {
                            GotoHunt(d);
                        }
                        if (event.keyboard.keycode == ALLEGRO_KEY_4) {
                            GotoReturn(d);
                        }
                        if (event.keyboard.keycode == ALLEGRO_KEY_Y) {
                            tracking = !tracking;
                        }

                        //ZOOM
                        if (event.keyboard.keycode == ALLEGRO_KEY_PGUP) {
                            scale += 0.025;
                            invscale = 1 / scale;
                        }
                        if (event.keyboard.keycode == ALLEGRO_KEY_PGDN) {
                            scale -= 0.025;
                            invscale = 1 / scale;
                        }
                    }
                    //END DEBUG

                    if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
                    {
                        Command.goback = true;
                        Command.goforward = false;
                    }

                    else if  (event.keyboard.keycode == ALLEGRO_KEY_PRINTSCREEN ||
                              event.keyboard.keycode == ALLEGRO_KEY_F12)
                    {
                        take_screenshot = true;
                        al_fprintf(logfile,"Screenshot %d\n",screenshot_count);
                    }
                    else if (event.keyboard.keycode == ALLEGRO_KEY_F10) //radar
                    {
                        Command.toggleradar = true;
                    }
                    else
                    {
                        pressed_keys[event.keyboard.keycode]=true;
                        key_down_log[event.keyboard.keycode]=true;
                    }
                }
                else if (event.type == ALLEGRO_EVENT_KEY_UP)
                {
                    pressed_keys[event.keyboard.keycode]=false;
                    key_up_log[event.keyboard.keycode]=true;
                }
                else if (event.type == ALLEGRO_EVENT_KEY_CHAR)   //used for high score entry
                {
                    if (game_over==2)
                    {
                        keypress = true;
                        current_key = event.keyboard.unichar;
                    }
                }
            break;
            case JOYSTICK:
                CheckUSBJoyStick(event);
            break;
            case TOUCH:
                CheckTouchControls(event);
            break;
            case TIMER:
                if (event.type == ALLEGRO_EVENT_TIMER)
                    redraw = true;
            break;
            default:
            break;
            }
            if (exit) break;
			if (Command.goback)
            {
                Command.goback = false;

                game_over = 0;//1;

                if (Net.server)
                {
                    NetSendAbort();         //tell clients to abort

                    while (num_ships > 1)   //wait for them all to disconnect
                        ServiceNetwork();
                }
                StopNetwork();          //exit back to menu
                break;
            }
            else if (Command.toggleradar)
            {
                Command.toggleradar = false;
#ifndef ANDROID
                //if (Net.client || Net.server || num_ships==1)
#endif
                {
                    Radar.on = !Radar.on;
                }
            }

            //Stuff to do on timer tick
			if (redraw && al_is_event_queue_empty(queue))
			{
				if (gpio_active) ReadGPIOJoystick();	//No events, so have to read this in the timer loop.

                UpdateTouches();

				double t = al_get_time();

				if (take_screenshot)
				{
					take_screenshot = false;	//don't do it again

					sprintf(screenshout_count_str,"%02d",screenshot_count++);	//increment number for filename

					strcpy(screenshot_name,screenshot_basename);	//init string
					strcat(screenshot_name,screenshout_count_str);	//append number
					strcat(screenshot_name,screenshot_ext);			//append extension

					al_save_bitmap(screenshot_name, al_get_backbuffer(display));

					al_fprintf(logfile,"Screenshot %d saved\n",screenshot_count-1);
				}

				al_flip_display();
				fps_accum++;
				if (t - fps_time >= 1)
				{
					fps = fps_accum;
					fps_accum = 0;
					fps_time = t;

					fpsnet = fpsnet_acc;
					fpsnet_acc = 0;
				}

                if (Net.server)
                {
                    NetSendGameState();
                    ServiceNetwork();
                }

				al_clear_to_color(al_map_rgb(0, 0, 0));	//clear the screen

				//Single draw_split_screen function
				//needs paramter FULL, TOP, BOTTOM, TL, TR, BL, BR
				//it gets window/screen coords from this and figures out where to put the ship etc.
				//Ship numbers passed in. This is used to centre the 'viewport' on the ship

//#ifdef ANDROID
//                draw_split_screen(FULL,0);
//#else
                /*if (Net.client || Net.server)
				{
                    draw_split_screen(FULL,Net.id);
                }
				else*/
                {
                    //switch(num_ships)
                    switch(Menu.ships)
                    {
                    case 1:
                            draw_split_screen(FULL,Net.id);
                            break;
                    case 2:
                            draw_split_screen(TOP,1);
                            draw_split_screen(BOTTOM,0);
                            break;
                    case 3:
                    case 4:
                            draw_split_screen(TOPLEFT,2);		//better for sitimus....
                            draw_split_screen(TOPRIGHT,1);
                            draw_split_screen(BOTTOMLEFT,0);
                            draw_split_screen(BOTTOMRIGHT,3);
                            break;
                    }
                }

                draw_dividers(Menu.ships);
//#endif
				draw_radar();

				if (debug_on) draw_debug();

				//ok unless we allow AI in net games.....
				if (Menu.netmode == LOCAL)
                    ScanInputs(Menu.ships+Menu.ai_ships);		//Convert keypresses/joystick input to ship controls (for n ships)
                else
                    ScanInputs(1);

				if (Net.client)
                {
                    char buffer[256];
                    strcpy(buffer,NAME);
                    sprintf(buffer+strlen(NAME)," (Client: %s ; %d players : Q:%.1f)",Net.myaddress,num_ships,Net.quality);
                    al_set_window_title(display, buffer);

                    if (!game_over)
                    {
                        //NetSendKeys();  //send keypresses to server

                        if (Net.client_state == IDLE)   //must have disconnected after receiving an abort
                        {
                            StopNetwork();
                            if (Net.aborted)
                            {
                                Net.client_state = ABORTED;
                                Net.aborted = false;
                            }

                            break;
                        }
                    }
                    //ServiceNetwork();                   //must do this regularly....
                    //if (!Net.updated)                   //if we didn't get a game state packet
                    //{
                    //    UpdateBullets();                //update positions locally
                    //    for (i=0 ; i<num_ships ; i++)
                    //    {
                    //        Ship[i].xpos += Ship[i].xv;
                    //        Ship[i].ypos -= Ship[i].yv;
                    //    }
                    //    missed_packets++;
                    //}
                }

				if (game_over)
				{
					if (game_over == GO_TIMER-1)    //wait 1 tick to calculate / sort scores.
                    {
                        if (Net.server) NetSendGameOver();

                        for (i=0 ; i<MAX_SHIPS ; i++)
                        {
                            al_stop_sample_instance(wind_inst[i]);
                            //Ship[i].automode = MANUAL;
                            Ship[i].fire1_held = false;
                            Ship[i].fire2_down = false;
                            Ship[i].thrust_held = false;
                        }
                    }

					if (!GameOver())
                    {
                        //0 returned, break out of inner while(1) loop. Still in outer, so we'll re-init stuff
                        StopNetwork();
                        break;
                    }
				}
				else
				{
                    draw_controls(al_map_rgba_f(0.5,0.5,0.5,0.5));
                    game_over = UpdateShips(num_ships);		//Calculate Ship positions based on controls
                                                                //Also handle firing, updating fuel, ammo, lives etc.
					CheckSWCollisions(num_ships);//Ship-to-wall collisions

					if (!Net.client)    //i.e. local or host
					{                                                                //if lives go down to 0, return game_over countdown.
                        UpdateSwitches();
                        UpdateForcefields();
                        UpdateSentries();	//Automatic guns / volcanoes
                        UpdateBullets();	//Calculate new bullet positions, expire old ones.

                        //check for collisions, modify shield parameter for ships, ttl for bullets.
                        CheckSSCollisions(num_ships);//Ship-to-ship collisions
                        CheckBSCollisions(num_ships);//bullet-to-ship collisions
                        CheckBSentryCollisions();
                        CheckBSwitchCollisions();
                        //CheckSWCollisions(num_ships);//Ship-to-wall collisions
                        CheckBWCollisions();		 //Bullet-to-wall collisions
					}
					else //client;
                    {
                        NetSendShipState();
                        ServiceNetwork();

                        if (!Net.updated)                   //if we didn't get a game state packet
                        {
                            UpdateBullets();                //do a local best-guees update.
                            UpdateRemoteShips();            //next recieved packet will correct any inaccuracies.
                            missed_packets++;
                        }
                    }

                    for (i=0 ; i<MAX_SHIPS ; i++)
                    {
                        if (Ship[i].thrust)
                        {
                            volume = 2.0;
                        }
                        else
                        {
                            //max_v_squared = (THRUST / Map.drag)^2;
                            //              = (50/2)^2
                            //              =  625
                            v_squared = Ship[i].xv*Ship[i].xv + Ship[i].yv*Ship[i].yv;
                            volume = 1+(v_squared/625)*2;
                        }

                        al_set_sample_instance_gain (wind_inst[i],volume);
                        al_set_sample_instance_speed(wind_inst[i],volume);
                    }
				}
				redraw = FALSE;
                if (vibrate_timer)
                {
                    vibrate_timer--;
                    if (vibrate_timer==0) {
                        Vibrate(vibrate_time);
                        vibrate_time = 0;
                    }
                }
			}
		}
		al_fprintf(logfile,"Game Over\n");
        FreeGameBitmaps();

        MenuControls();

        for (i=0 ; i<MAX_SHIPS ; i++)
        {
            al_destroy_sample_instance(wind_inst[i]);
            al_destroy_sample_instance(shoota_inst[i]);
            al_destroy_sample_instance(shootb_inst[i]);
            al_destroy_sample_instance(dead_inst[i]);
            al_destroy_sample_instance(particle_inst[i]);
            Ship[i].automode = MANUAL;
            Ship[i].thrust_held = false;                    //for auto ships.
        }
        al_destroy_sample_instance(yippee_inst);
        al_destroy_sample_instance(sentry_particle_inst);

        al_fflush(logfile);
        if (exit) break;
	}
    return 0;
}

void MenuControls(void)
{
    Ctrl.ctrl[RADAR].active = FALSE;
    Ctrl.ctrl[ASTICK].active = FALSE;
    Ctrl.ctrl[ASTICK2].active = FALSE;
    Ctrl.ctrl[FIRE1].active = FALSE;
    Ctrl.ctrl[FIRE2].active = FALSE;
    Ctrl.ctrl[THRUST_BUTTON].active = FALSE;
    Ctrl.ctrl[SELECT].active = TRUE;
    Ctrl.ctrl[REVERSE].active = FALSE;
}

void GameControls(void)
{
    Ctrl.ctrl[RADAR].active = TRUE;
    Ctrl.ctrl[ASTICK].active = TRUE;
    Ctrl.ctrl[ASTICK2].active = TRUE;
    Ctrl.ctrl[FIRE1].active = TRUE;
    Ctrl.ctrl[FIRE2].active = TRUE;
    Ctrl.ctrl[THRUST_BUTTON].active = TRUE;
    Ctrl.ctrl[SELECT].active = FALSE;
    Ctrl.ctrl[REVERSE].active = TRUE;
}

void SystemBackPressed(void)
{
    Command.goback = true;
}

void ForwardOrBack(ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_EVENT event)
{
    int i;//,redraw=TRUE;

    al_wait_for_event(queue, &event);

    if (event.type == ALLEGRO_EVENT_DISPLAY_RESIZE)
    {
        al_acknowledge_resize(display);
        LoadFonts(0);
    }

    else if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
        Exit();

    else if (event.type == ALLEGRO_EVENT_TIMER)
    {
        if (gpio_active)
            ReadGPIOJoystick();
        UpdateTouches();
        redraw = true;
#ifdef ANDROID
        //if (_jni_callVoidMethodV(_al_android_get_jnienv(), _al_android_activity_object(), "WasBackPressed", "()I"))
        //    Command.goback = true;
#endif
    }
    else if (event.type == ALLEGRO_EVENT_DISPLAY_HALT_DRAWING)   //we've been sidelined by the user/os
    {
        al_acknowledge_drawing_halt(display);   //acknowledge
        halted = true;                          //flag to drawing routines to do nothing
        al_stop_timer(timer);                   //no more timer events, so we should do nothing, saving battery
        #ifdef ANDROID
            al_set_default_voice(NULL);             //destroy voice, so no more sound events, ditto.
        #endif // ANDROID
        //break;
    }
    else if (event.type == ALLEGRO_EVENT_DISPLAY_RESUME_DRAWING) //we've been restored
    {
        al_acknowledge_drawing_resume(display); //ack
        halted = false;                         //remove flag
        al_start_timer(timer);                  //restart timer
        voice = al_create_voice(44100, ALLEGRO_AUDIO_DEPTH_INT16, ALLEGRO_CHANNEL_CONF_2);  //restart audio
        al_attach_mixer_to_voice(mixer, voice);
        //break;
    }
    else if (event.type == ALLEGRO_EVENT_KEY_DOWN)
    {
        if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
            Command.goback = true;
        else
            key_down_log[event.keyboard.keycode]=true;
    }

    else if (event.type == ALLEGRO_EVENT_KEY_UP)
        key_up_log[event.keyboard.keycode]=true;

    else
    {
        CheckUSBJoyStick(event);
        CheckTouchControls(event);  //will set command.goforward etc.
    }

    ScanInputs(num_ships);

    for (i=0 ; i<num_ships ; i++)
    {
        if (Ship[i].thrust_down || key_down_log[ALLEGRO_KEY_ENTER])
        {
            Command.goforward = true;
            Ship[i].thrust_down = false;
            key_down_log[ALLEGRO_KEY_ENTER] = false;
            break;			//out of the for loop
        }
    }

    return;
}

void CalcScales(void)
{
    float scalex,scaley,w,h,bmw,bmh;

    w = (float)al_get_display_width(display);
    h = (float)al_get_display_height(display);
    scalex = w/SCREENX;
    scaley = h/SCREENY;

    if (scalex < scaley)
        Scale.scale = scalex;
    else
        Scale.scale = scaley;

    Scale.invscale = 1/Scale.scale;

    Scale.fontscale = Scale.scale*0.8;

    if (panel_bmp)
    {
        bmw = al_get_bitmap_width(panel_bmp);
        bmh = al_get_bitmap_height(panel_bmp);

        scalex = (float)w/bmw;
        scaley = (float)h/bmh;

        if (scalex < scaley)
            Scale.menuscale = scalex;
        else
            Scale.menuscale = scaley;

        Scale.xmin = 0.1*w + 32*0.8*Scale.menuscale;
        Scale.xmax = 0.1*w + 415*0.8*Scale.menuscale;
        Scale.xdiff = Scale.xmax - Scale.xmin;

        Scale.ammo1level = 0.1*h + 41*0.8*Scale.menuscale;
        Scale.ammo1type  = 0.1*h + 80*0.8*Scale.menuscale;
        Scale.ammo2level = 0.1*h + 151*0.8*Scale.menuscale;
        Scale.ammo2type  = 0.1*h + 198*0.8*Scale.menuscale;
        Scale.fuellevel  = 0.1*h + 272*0.8*Scale.menuscale;
        Scale.ymax       = 0.1*h + 323*0.8*Scale.menuscale;


    }
}

void LoadFonts(float scale)
{
    FreeFonts();

    //font_scale = (float)al_get_display_width(display)/(SCREENX);    //font sizes chosen to suit SCREENX, so scale according to what we actually have!
    if (scale) font_scale = scale*0.8;

    if ((font       = al_load_font("miriam.ttf", 20*font_scale, 0))          == NULL)  al_fprintf(logfile,"miriam.ttf load fail\n"); //debug font

    if ((big_font   = al_load_font("Zebulon.otf", 200*font_scale, 0))      == NULL)  al_fprintf(logfile,"Zebulon.otf load fail\n");
    if ((title_font = al_load_font("Zebulon.otf", 125*font_scale, 0))    == NULL)  al_fprintf(logfile,"Zebulon.otf load fail\n");

    if ((menu_font  = al_load_font("Audiowide-Regular.ttf", 40*font_scale,0))== NULL)  al_fprintf(logfile,"Audiowide-Regular.ttf load fail\n"); //*****
    if ((glow_font  = al_load_font("Audiowide-500.ttf", 40*font_scale,0))== NULL)      al_fprintf(logfile,"Audiowide-500.ttf load fail\n"); //*****
    if ((small_font = al_load_font("Audiowide-Regular.ttf", 30*font_scale,0))== NULL)  al_fprintf(logfile,"Audiowide-Regular.ttf load fail\n"); //*****
    if ((small_glow_font = al_load_font("Audiowide-500.ttf", 30*font_scale,0))== NULL) al_fprintf(logfile,"Audiowide-500.ttf load fail\n"); //*****

    //don't scale this one....
    if ((race_font  = al_load_font("7seg.ttf", 18, 0))            == NULL)  al_fprintf(logfile,"7seg.ttf load fail\n");

	al_fprintf(logfile,"Loaded fonts (scaled by %.2f)\n",font_scale);
	al_fflush(logfile);
}

void LoadSamples(void)
{
    if ((slam = al_load_sample   ("slam.wav"))    == NULL)  al_fprintf(logfile,"slam.wav load fail");
    if ((shoota = al_load_sample  ("shootA.wav"))   == NULL)  al_fprintf(logfile,"shootA.wav load fail");
    if ((shootb = al_load_sample  ("shootB.wav"))   == NULL)  al_fprintf(logfile,"shootB.wav load fail");
    if ((particle = al_load_sample("particle.wav")) == NULL)  al_fprintf(logfile,"particle.wav load fail");
    if ((dead = al_load_sample    ("dead.wav"))     == NULL)  al_fprintf(logfile,"dead.wav load fail");
    if ((clunk = al_load_sample   ("clunk.wav"))    == NULL)  al_fprintf(logfile,"clunk.wav load fail");
    if ((wind = al_load_sample    ("wind.ogg"))     == NULL)  al_fprintf(logfile,"wind.ogg load fail");
    if ((yippee = al_load_sample  ("yippee.wav"))   == NULL)  al_fprintf(logfile,"yippee.wav load fail");
    if ((loop = al_load_sample    ("gsloop.ogg"))   == NULL)  al_fprintf(logfile,"gsloop.ogg load fail");
    al_fprintf(logfile,"Loaded Audio Samples\n");
}
int read_maps(void)
{
	int i,j,group = -1,map = 0,len;
	char temp[MAP_NAME_LENGTH];

	ALLEGRO_FILE* mapfile;

	mapfile = al_fopen("maps.txt","r");

	if (mapfile == NULL)
		al_fprintf(logfile,"Failed to open file maps.txt\n");

	for (i=0 ; /*i<MAX_MAPS*/ ; i++)
	{
		//if (fscanf(mapfile,"%s",&map_names[MAP_NAME_LENGTH * i]) == EOF) break;
		//if (fscanf(mapfile,"%s",&temp) == EOF) break;
		//if (fgets(temp,MAP_NAME_LENGTH,mapfile) == NULL) break;
        if (al_fgets(mapfile,temp,MAP_NAME_LENGTH) == NULL) break;

		len = strcspn(temp, "\r\n");

		if (len == 0) break;

		temp[len] = 0;

		//temp[strlen(temp)]=0;

		//al_fprintf(logfile,"%s *%d\n",temp,strcspn(temp, " \r\n"));

		if (temp[0] == ':')	//group name
		{
			if (group != -1)
			{
				MapNames[group].Count = map;
				//al_fprintf(logfile,"Group %d Count = %d\n",group, MapNames[group].Count);
			}
			group++;
			map = 0;

			strncpy ((char*)&MapNames[group].Group, &temp[1], MAP_NAME_LENGTH-1);
			//al_fprintf(logfile,"Group: %s\n",&MapNames[group].Group);

		}
		else	//map name
		{
			strncpy ((char*)&MapNames[group].Map[map], &temp[0], MAP_NAME_LENGTH);
			//al_fprintf(logfile,"Map: %s\n",&MapNames[group].Map[map]);
			map++;
		}

		//al_fprintf(logfile,"%s\n",&map_names[MAP_NAME_LENGTH * i]);
	}

	MapNames[group].Count = map;
	//al_fprintf(logfile,"Group %d Count = %d\n",group, MapNames[group].Count);

	al_fclose(mapfile);

	al_fprintf(logfile,"%d Map groups\n",group+1);

	for (i=0 ; i<group+1 ; i++)
	{
		al_fprintf(logfile,"Group: %s\n",(char*)&MapNames[i].Group);
		for (j=0 ; j<MapNames[i].Count ; j++)
		{
			al_fprintf(logfile,"Map: %s\n",(char*)&MapNames[i].Map[j]);
		}
	}

	return group+1;
}

void LoadMap(void)  //different function for tilemaps? Or just a smarter one....
{
    //al_fprintf(logfile,"tr_map = %d\n",tr_map);
    //al_fflush(logfile);

    tr_map = al_load_bitmap(Map.display_file_name);
	if (tr_map == NULL)
    {
        al_fprintf(logfile,"Display Map load error\n");
        return;
    }

	if (Map.type == 1)  //tiled
	{
		mapx = map_width * TILE_WIDTH;
		mapy = map_height * TILE_HEIGHT;
	}
	else if (Map.type == 2) //single image, not scaled
	{
    	mapx = al_get_bitmap_width(tr_map);
    	mapy = al_get_bitmap_height(tr_map);
        map_width = mapx/64;
        map_height = mapy/64;
	}
	else    //0, tr style, single image, x2
	{
    	mapx = 2*al_get_bitmap_width(tr_map);
    	mapy = 2*al_get_bitmap_height(tr_map);
        map_width = mapx/64;
        map_height = mapy/64;
	}

	if (Map.background_file_name[0] != 0)
    {
        background = al_load_bitmap(Map.background_file_name);
        if (background == NULL)
            al_fprintf(logfile,"Background bitmap load fail.\n");
    }

	if (Map.sentry_file_name[0] != 0)
    {
		sentries = al_load_bitmap(Map.sentry_file_name);
		if (sentries == NULL)
            al_fprintf(logfile,"Sentries bitmap load fail.\n");
    }

    al_fprintf(logfile,"tr_map size %d x %d\n",mapx,mapy);
    al_fflush(logfile);
}

void FreeGameBitmaps(void)
{
    int i=0,j=0;;

    al_fprintf(logfile,"Freeing game bitmaps\n");
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

    if (ui)
    {
        al_destroy_bitmap(ui);
        ui = NULL;
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

    if (background)
    {
        al_destroy_bitmap(background);
        background = NULL;
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

    if (map_text_bmp)
    {
        al_destroy_bitmap(map_text_bmp);
        map_text_bmp = NULL;
        i++;
    }
    j++;

    if (Radar.mask)
    {
        if (Radar.display == Radar.mask)
        {
            al_destroy_bitmap(Radar.mask);
            Radar.mask = NULL;
            Radar.display = NULL;
            i++;
            j++;
        }
        else
        {
            al_destroy_bitmap(Radar.mask);
            Radar.mask = NULL;
            al_destroy_bitmap(Radar.display);
            Radar.display = NULL;
            i+=2;
            j+=2;
        }
    }

    al_fprintf(logfile,"Freed %d/%d game bitmaps\n",i,j);
    al_fflush(logfile);

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

    if (logo)
    {
        al_destroy_bitmap(logo);
        logo = NULL;
        i++;
    }

    if (grey_ships)
    {
        al_destroy_bitmap(grey_ships);
        grey_ships = NULL;
        i++;
    }

    if (inst_bmp)
    {
        al_destroy_bitmap(inst_bmp);
        inst_bmp = NULL;
        i++;
    }

    al_fprintf(logfile,"Freed %d menu/title bitmaps\n",i);
    al_fflush(logfile);
}

void FreeFonts(void)
{
	al_destroy_font(font      );
	al_destroy_font(menu_font );
	al_destroy_font(glow_font );
	al_destroy_font(small_font);
	al_destroy_font(small_glow_font);
	al_destroy_font(big_font  );
	al_destroy_font(race_font );
	al_destroy_font(title_font);
	al_fprintf(logfile,"Freed fonts\n");
}

void StopNetwork(void)
{
    if (Net.server)
    {
        NetStopServer();    //stop the server
    }
    if (Net.client)
    {
        if (Net.client_state == CONNECTED || Net.client_state == RUNNING)
        {
            NetDisconnectClient();
        }

        while (Net.client_state != IDLE)
        {
            ServiceNetwork();
        }
        NetStopClient();

       // NetStopClient();
    }
    Net.started = false;
}


void Exit(void)
{
	al_fprintf(logfile,"Exiting\n");
    FreeGameBitmaps();
    FreeMenuBitmaps();
    FreeFonts();
    al_destroy_display(display);
	al_fclose(logfile);
	if (hostfile) al_fclose (hostfile);
	if (clientfile) al_fclose (clientfile);
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
extern float soverm;

void draw_debug(void)
{
	int level;

    level = 100;

	al_draw_textf(font, al_map_rgb(255, 255, 255),0, level,  ALLEGRO_ALIGN_LEFT, "FPS: %d", fps);
	al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30,  ALLEGRO_ALIGN_LEFT, "Menu.ships: %d", Menu.ships);
	al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30,  ALLEGRO_ALIGN_LEFT, "Menu.ai_ships: %d", Menu.ai_ships);
	al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30,  ALLEGRO_ALIGN_LEFT, "num_ships: %d", num_ships);

	al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30,  ALLEGRO_ALIGN_LEFT, "Ship: %d", d);
	al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30,  ALLEGRO_ALIGN_LEFT, "Algorithm: %i", Ship[d].autoalgorithm);
	al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30,  ALLEGRO_ALIGN_LEFT, "Automode: %i", Ship[d].automode);
	al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30,  ALLEGRO_ALIGN_LEFT, "Autotimer: %i", Map.timer);
    al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30,  ALLEGRO_ALIGN_LEFT, "Af1TT: %i", Ship[d].autofire1toggletime);
	al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30,  ALLEGRO_ALIGN_LEFT, "Autofire1: %i", Ship[d].autofire1);
	al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30,  ALLEGRO_ALIGN_LEFT, "fire_state: %i", fire_state);

    //for (i=0 ; i<8 ; i++)
    //    al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30,  ALLEGRO_ALIGN_LEFT, "Wall[%d] = %d",i,walls[i]);

	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "target: %d", target_angle);
    //al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "avoid: %d", avoid_angle);
    al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "ratio: %0.2f", ratio);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "avoidx: %0.2f", avoidx);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "avoidy: %0.2f", avoidy);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "sumx: %0.2f", sumx);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "sumy: %0.2f", sumy);
	al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "AVI: %0.2f", Ship[d].autovectintegrator);
	al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "VSquint: %0.2f", Ship[d].autovsqint);



    //al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Scale:%0.3f",scale);
	//for (i=0 ; Touch[i].id != NO_TOUCH ; i++)
    //for (i=0 ; i<NUM_TOUCHES ; i++)
    //{
    //    al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Touch:%d, But:%d",Touch[i].id,Touch[i].button);
    //}

    //al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "NumTouches:%d",i);


    //al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Select.x:%0.3f",Select.x);
    //al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Select.y:%0.3f",Select.y);
    //al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Select.dx:%0.3f",Select.dx);
    //al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Select.dx:%0.3f",Select.dy);

    //al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Spin:%0.3f",TouchJoystick.spin);

    //al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30,  ALLEGRO_ALIGN_LEFT, "Net: %d", fpsnet);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30,  ALLEGRO_ALIGN_LEFT, "Missed: %d", missed_packets);
    /*
	al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "NetQual: %.1f", Net.quality);
	al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Ship: %d", d);
    al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Image: %d", Ship[d].image);
	al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "X: %.0f", Ship[d].xpos);
	al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Y: %.0f", Ship[d].ypos);
    */
    al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Xv: %.2f", Ship[d].xv);
    /*
    al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Yv: %.0f", Ship[d].yv);
    al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Relx: %d", relx);
    al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Rely: %d", rely);
    */
    al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "FAngle: %0.2f", Ship[d].fangle);
    al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Angle: %d", Ship[d].angle);
    /*
    al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Shield: %d", Ship[d].shield);
    //al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "G: %.2f", Ship[d].gravity);
*/
    al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30,  ALLEGRO_ALIGN_LEFT, "Kills:%d",Ship[d].kills);
    /*al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30,  ALLEGRO_ALIGN_LEFT, "Killed:%d",Ship[d].killed);
    al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30,  ALLEGRO_ALIGN_LEFT, "Crashes:%d",Ship[d].crashed);
    al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30,  ALLEGRO_ALIGN_LEFT, "Ammo2:%d",(Ship[d].ammo2*25)/2);
     */

    //al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30,  ALLEGRO_ALIGN_LEFT, "Clients:%d",Net.clients );
    //al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30,  ALLEGRO_ALIGN_LEFT, "Clients Ready:%d",Net.clients_ready);
    //al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30,  ALLEGRO_ALIGN_LEFT, "Connected:%d",Net.connected);


    //al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "R: %d", random);
    //al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "R1:%d", random100);
    //al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "S: %d", shots);
    //al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "C: %d", count);

	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Key: %d",debug_key);

    //al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "XV: %.2f", Ship[d].xv);
    //al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "YV: %.2f", Ship[d].yv);

	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Vol: %.2f", volume);
    //al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "V^2: %.2f", v_squared);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Mapx: %d", mapx);

	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "M: %d", Ship[d].miners);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "J: %d", Ship[d].jewels);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Score: %d", Ship[d].sentries);

	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Angle: %d", ANGLE_INC*Ship[d].angle);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Landed: %d", Ship[d].landed);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Pad: %d", Ship[d].pad);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Home Pad: %d", Ship[d].home_pad);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Menu: %d", Ship[d].menu);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Recharge: %d", Ship[d].recharge_timer);

	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "A1: %d", Ship[d].ammo1_type);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "A2: %d", Ship[d].ammo2_type);

	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Mass[d]: %d", Ship[0].mass);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Race: %d", Map.race);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "approaching: %d", Ship[d].approaching);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "racing: %d", Ship[d].racing);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "FT: %f", FrameTime);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Lap: %0.3f", Ship[d].current_lap_time);

	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "BD: %d", GPIOJoystick.button_down);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "BU: %d", GPIOJoystick.button_up);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "B: %d", GPIOJoystick.button);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "ThD: %d", Ship[d].thrust_down);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "ThH: %d", Ship[d].thrust_held);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Th: %d", Ship[d].thrust);

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

	//int i;
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
    //al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "Coll:%d", collision);
	//al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "S0.s:%d", Map.sentry[0].shield);

	//Tiled
	/*al_draw_textf(font, al_map_rgb(255, 255, 255),0, level+=30, ALLEGRO_ALIGN_LEFT, "TX:%d", tile_x);
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
    */
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

//#if 0
#ifdef RPI
/* Function: al_vfprintf
 */
int al_vfprintf(ALLEGRO_FILE *pfile, const char *format, va_list args)
{
   int rv = -1;
   ALLEGRO_USTR *ustr = 0;
   size_t size = 0;
   bool success;

   if (pfile != 0 && format != 0)
   {
      ustr = al_ustr_new("");
      if (ustr)
      {
         success = al_ustr_vappendf(ustr, format, args);
         if (success)
         {
            size = al_ustr_size(ustr);
            if (size > 0)
            {
               rv = al_fwrite(pfile, (const void*)(al_cstr(ustr)), size);
               if (rv != (int)size) {
                  rv = -1;
               }
            }
         }
         al_ustr_free(ustr);
      }
   }
   return rv;
}


/* Function: al_fprintf
 */
int al_fprintf(ALLEGRO_FILE *pfile, const char *format, ...)
{
   int rv = -1;
   va_list args;

   if (pfile != 0 && format != 0)
   {
      va_start(args, format);
      rv = al_vfprintf(pfile, format, args);
      va_end(args);
   }
   return rv;
}


/* vim: set sts=3 sw=3 et: */
#endif
