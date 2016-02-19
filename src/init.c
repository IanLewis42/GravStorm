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

#include <string.h>
#include <stdio.h>

#include "allegro5/allegro.h"
#include "allegro5/allegro_image.h"
#include "allegro5/allegro_primitives.h"
#include "allegro5/allegro_font.h"
#include "allegro5/allegro_audio.h"

#include <wiringPi.h>

#include "game.h"
#include "init.h"
#include "objects.h"
#include "inputs.h"
#include "drawing.h"

FILE* map_file;	//text file containing map-specific data
const char txt[5] = ".txt\0";

void load_map_file(void);

//parse:
//align label first column, parameters separated by spaces, suits scanf I think.
//so read line, check label, scanf, pattern determined by label.
int init_map(int group, int map)
{
	int i=0, j=0,k=0,l=0,m=0,n=0;	//counters for pads, special areas, blackholes, sentries, forcefields etc.
	//float ff_strength;
	char map_file_name[MAP_NAME_LENGTH];
	char str[100];
	char *line;
	//char* name;

	//name = (char*)&MapNames[group].Map[map];

	//for (i=0 ; i<MAP_NAME_LENGTH ; i++)


	//strncpy(map_file_name, (map_names + map*MAP_NAME_LENGTH), MAP_NAME_LENGTH);
	strncpy(map_file_name, (char*)&MapNames[group].Map[map], MAP_NAME_LENGTH);

	strcat(map_file_name, txt);

	map_file = fopen(map_file_name,"r");

	if (map_file == NULL)
	{
		fprintf(logfile,"Couldn't open file %s\n",map_file_name);
		return 1;
	}

    //defaults.
    Map.type = 0;

    Map.display_file_name[0] = 0;
    Map.collision_file_name[0] = 0;
    Map.display_file_name[0] = 0;
    Map.ascii_map_file_name[0] = 0;
    Map.description_file_name[0] = 0;
    Map.sentry_file_name[0] = 0;
    Map.sentry_collision_file_name[0] = 0;

  	Map.ship_first = 0;
    Map.max_players=1;
    Map.mission = 0;
   	Map.lives = 6;
   	Map.time_limit = 60;
    Map.radial_gravity = 0;

   	Map.num_pads = 0;
    Map.num_blackholes = 0;
    Map.num_sentries = 0;
    Map.num_special_areas = 0;
    Map.num_forcefields = 0;

    Map.gravity = 0.05;
    Map.drag = 2;
    Map.wrap = 0;

    Map.race = 0;

	while (fgets(str, 100, map_file) != NULL)
	{
		line = str;

		while (isspace(*line)) line++;

		if (strncmp(line,"map_type",8) == 0)
		{
			sscanf(line+8," %d",&Map.type);
			fprintf(logfile,"Map type:%d\n",Map.type);
		}

		else if (strncmp(line,"display_map",11) == 0)
		{
			sscanf(line+11," %s",&Map.display_file_name);
			fprintf(logfile,"Display Map:%s\n",Map.display_file_name);
		}

		else if (strncmp(line,"collision_map",13) == 0)
		{
			sscanf(line+13," %s",&Map.collision_file_name);
			fprintf(logfile,"Collision Map:%s\n",Map.collision_file_name);
		}

		else if (strncmp(line,"ascii_map",9) == 0)
		{
			sscanf(line+9," %s",&Map.ascii_map_file_name);
			fprintf(logfile,"ASCII map file:%s\n",Map.ascii_map_file_name);
			load_map_file();
		}

		else if (strncmp(line,"sentry_display",14) == 0)
		{
			sscanf(line+14," %s",&Map.sentry_file_name);
			fprintf(logfile,"Sentry Image file:%s\n",Map.sentry_file_name);
		}

		else if (strncmp(line,"sentry_collision",16) == 0)
		{
			sscanf(line+16," %s",&Map.sentry_collision_file_name);
			fprintf(logfile,"Sentry Collision file:%s\n",Map.sentry_collision_file_name);
		}

		else if (strncmp(line,"description",11) == 0)
		{
			sscanf(line+11," %s",&Map.description_file_name);
			fprintf(logfile,"Description file:%s\n",Map.description_file_name);
		}

		else if (strncmp(line,"ship_first",10) == 0)
		{
			sscanf(line+10," %d",&Map.ship_first);
			fprintf(logfile,"Ship first:%d\n",Map.ship_first);
		}

		else if (strncmp(line,"mission",7) == 0)
		{
			sscanf(line+7," %d",&Map.mission);
			fprintf(logfile,"Mission:%d\n",Map.mission);
		}

		else if (strncmp(line,"lives",5) == 0)
		{
			sscanf(line+5," %d",&Map.lives);
			if (Map.lives > 6)
				Map.lives = 6;

			fprintf(logfile,"Lives:%d\n",Map.lives);
		}

		else if (strncmp(line,"time_limit",10) == 0)
		{
			sscanf(line+10," %d",&Map.time_limit);
			fprintf(logfile,"Time Limit:%d\n",Map.time_limit);
		}

		else if (strncmp(line,"max_players",11) == 0)
		{
			sscanf(line+11," %d",&Map.max_players);
			fprintf(logfile,"Max players:%d\n",Map.max_players);
			//for (k=Map.max_players ; k<MAX_SHIPS ; k++)
			for (k=0 ; k<MAX_SHIPS ; k++)
			{
				if (k >= Map.max_players)
					Ship[k].controller = NA;
				else
					Ship[k].controller = Ship[k].selected_controller;
			}
		}

		else if (strncmp(line,"gravity",7) == 0)
		{
			sscanf(line+7," %f",&Map.gravity);
			fprintf(logfile,"Gravity:%f\n",Map.gravity);
		}

		else if (strncmp(line,"drag",4) == 0)
		{
			sscanf(line+4," %f",&Map.drag);
			fprintf(logfile,"Drag:%f\n",Map.drag);
		}

		else if (strncmp(line,"wrap",4) == 0)
		{
			sscanf(line+4," %d",&Map.wrap);
			fprintf(logfile,"Wrap:%d\n",Map.wrap);
		}

		else if (strncmp(line,"pad",3) == 0)
		{
			Map.pad[i].miners = 0;
			Map.pad[i].jewels = 0;
			sscanf(line+3," %x %d %d %d %d %d",&Map.pad[i].type,&Map.pad[i].y,&Map.pad[i].min_x,&Map.pad[i].max_x,&Map.pad[i].miners,&Map.pad[i].jewels);
			fprintf(logfile,"Pad %d: type:%02x y:%d x:%d x:%d miners:%d jewels:%d\n",i,Map.pad[i].type,Map.pad[i].y,Map.pad[i].min_x,Map.pad[i].max_x,Map.pad[i].miners,Map.pad[i].jewels);

			if ((Map.pad[i].type & 0x000f) < MAX_SHIPS)
	            Ship[Map.pad[i].type & 0x000f].home_pad = i;    //bottom nibble of type gives ship which this is home pad for.

            i++;
		}

		else if (strncmp(line,"area",4) == 0)
		{
			sscanf(line+4," %d %d %d %d %f %f",                           &Map.area[j].min_x,&Map.area[j].max_x,&Map.area[j].min_y,&Map.area[j].max_y,&Map.area[j].gravity,&Map.area[j].drag);
			fprintf(logfile,"area %d: x:%d x:%d y:%d y:%d g:%f drag:%f\n",j,Map.area[j].min_x, Map.area[j].max_x, Map.area[j].min_y, Map.area[j].max_y, Map.area[j].gravity, Map.area[j].drag);
			j++;
		}

		else if (strncmp(line,"blackhole",9) == 0)
		{
			Map.radial_gravity = true;
			if (l==0)
				fprintf(logfile,"Radial Gravity On\n");

			sscanf(line+9," %d %d %f",                        &Map.blackhole[l].x,&Map.blackhole[l].y,&Map.blackhole[l].g);
			fprintf(logfile,"blackhole %d: x:%d y:%d g:%f\n",l,Map.blackhole[l].x,Map.blackhole[l].y,Map.blackhole[l].g);
			l++;
		}

		//      x y type(0/1/2) gun volcano firing period probability random/targeted
		//sentry

		else if (strncmp(line,"sentry",6) == 0)
		{
			sscanf(line+6," %i %i %i %i %i %i %i %i %i %i",                                                                   &Map.sentry[m].x, &Map.sentry[m].y, &Map.sentry[m].direction, &Map.sentry[m].type, &Map.sentry[m].period, &Map.sentry[m].probability, &Map.sentry[m].random, &Map.sentry[m].range, &Map.sentry[m].alive_sprite, &Map.sentry[m].dead_sprite);
			fprintf(logfile,"Sentry %i: x:%i, y:%i, Direction:%i, Type:%i, Period:%i, Prob:%i, Random:%i, Range:%i, Sprite%d, Sprite%d\n",m, Map.sentry[m].x,  Map.sentry[m].y,  Map.sentry[m].direction,  Map.sentry[m].type,  Map.sentry[m].period,  Map.sentry[m].probability,  Map.sentry[m].random,  Map.sentry[m].range, Map.sentry[m].alive_sprite, Map.sentry[m].dead_sprite);
			Map.sentry[m].range = Map.sentry[m].range * Map.sentry[m].range;	//square to save square rooting later.
			Map.sentry[m].count = Map.sentry[m].period;	//init countdown timer
			Map.sentry[m].shield = SENTRY_SHIELD;		//init shield
			Map.sentry[m].alive = 1;
			m++;
		}

		else if (strncmp(line,"race",4) == 0)
		{
			//fprintf(logfile,"RACE\n");
			sscanf(line+4," %d %d %d %d",&Map.raceline_minx,&Map.raceline_maxx,&Map.raceline_miny,&Map.raceline_maxy);
			//fprintf(logfile,"Race start/finish line: min x:%d max x:%d min y:%d max y:%d\n",Map.raceline_minx,Map.raceline_maxx,Map.raceline_miny,Map.raceline_maxy);
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

			else fprintf(logfile,"X or Y values must match in race start/finish line (i.e. line must be horizontal or vertical)\n");

			fprintf(logfile,"Race:   x:%d x:%d y:%d y:%d\n",Map.raceline_minx,Map.raceline_maxx,Map.raceline_miny,Map.raceline_maxy);
			fprintf(logfile,"Before: x:%d x:%d y:%d y:%d\n",Map.before_minx,Map.before_maxx,Map.before_miny,Map.before_maxy);
			fprintf(logfile,"After:  x:%d x:%d y:%d y:%d\n",Map.after_minx,Map.after_maxx,Map.after_miny,Map.after_maxy);
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

			else fprintf(logfile,"X or Y values must match in forcefield (i.e. line must be horizontal or vertical)\n");

			fprintf(logfile,"ForceField: x:%d x:%d x:%d y:%d y:%d y:%d\n",Map.forcefield[n].min_x,Map.forcefield[n].half_x,Map.forcefield[n].max_x,Map.forcefield[n].min_y,Map.forcefield[n].half_y,Map.forcefield[n].max_y);
			fprintf(logfile,"ForceField: strength:%0.0f sentry:%d\n",Map.forcefield[n].strength,Map.forcefield[n].sentry);
			n++;
		}
	}

	Map.num_pads = i;
	Map.num_special_areas = j;
	Map.num_blackholes = l;
	Map.num_sentries = m;
	Map.num_forcefields = n;

	if (Map.display_file_name[0] == 0)
		fprintf(logfile,"ERROR: No display file specified\n");
	if (Map.collision_file_name[0] == 0)
		fprintf(logfile,"ERROR: No collision file specified\n");
	if (Map.type == 1)
	{
		if (Map.ascii_map_file_name[0] == 0)
			fprintf(logfile,"ERROR: No ASCII map file specified\n");
	}
	if (Map.num_sentries != 0)
	{
		if (Map.sentry_file_name[0] == 0)
			fprintf(logfile,"ERROR: No sentry display file specified\n");

		if (Map.sentry_collision_file_name[0] == 0)
			fprintf(logfile,"Possible error: No sentry collision file specified. Sentries will be indestructible.\n");
	}
	if (Map.num_pads == 0)
		fprintf(logfile,"ERROR: No pads specified\n");

	fprintf(logfile,"--End of mapfile %s--\n",map_file_name);

	fflush(logfile);
	fclose  (map_file);
	return 0;
}

void load_map_file(void)
{
	int i,j,found;
    FILE* map_file;
	char line[200];

	if ((map_file = fopen(Map.ascii_map_file_name,"r")) == NULL)  fprintf(logfile,"Couldn't open %s for reading.\n",Map.ascii_map_file_name);

	map_width = 0;

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
				if(i >= map_width) map_width = i; //width is last valid char
			}
			i++;

		}
		j++;
		if (found) 	map_height = j;	//height is number of lines read
	}
	map_width++;

	fclose(map_file);

	fprintf(logfile,"ASCII Map: H:%d W:%d\n",map_height,map_width);
}

void init_controls(void)
{
	//key mapping
	Ship[0].controller = KEYS;	//keys or joystick
	Ship[0].selected_controller = KEYS;	//keys or joystick
	Ship[0].up_key     = ALLEGRO_KEY_UP;
	Ship[0].down_key   = ALLEGRO_KEY_DOWN;
	Ship[0].left_key   = ALLEGRO_KEY_LEFT;
	Ship[0].right_key  = ALLEGRO_KEY_RIGHT;
	Ship[0].thrust_key = ALLEGRO_KEY_RCTRL;

	//key mapping
	Ship[1].controller = KEYS;//USB_JOYSTICK0;	//keys or joystick
	Ship[1].selected_controller = KEYS;
	Ship[1].up_key     = ALLEGRO_KEY_Q;
	Ship[1].down_key   = ALLEGRO_KEY_A;
	Ship[1].left_key   = ALLEGRO_KEY_R;
	Ship[1].right_key  = ALLEGRO_KEY_T;
	Ship[1].thrust_key = ALLEGRO_KEY_ALT;

	Ship[2].controller = NA;	//keys or joystick
	Ship[2].selected_controller = USB_JOYSTICK0;
    Ship[2].up_key     = ALLEGRO_KEY_Q;
    Ship[2].down_key   = ALLEGRO_KEY_A;
    Ship[2].left_key   = ALLEGRO_KEY_R;
    Ship[2].right_key  = ALLEGRO_KEY_T;
    Ship[2].thrust_key = ALLEGRO_KEY_ALT;

	Ship[3].controller = NA;	//keys or joystick
	Ship[3].selected_controller = USB_JOYSTICK1;
	Ship[3].up_key     = ALLEGRO_KEY_UP;
	Ship[3].down_key   = ALLEGRO_KEY_DOWN;
	Ship[3].left_key   = ALLEGRO_KEY_LEFT;
	Ship[3].right_key  = ALLEGRO_KEY_RIGHT;
	Ship[3].thrust_key = ALLEGRO_KEY_LCTRL;
}

void init_ships(int num_ships)
{
	int i;
	Ship[0].colour = 	al_map_rgba(0, 32, 0, 20);  //used for background to status (ammo, fuel, lives etc)
	Ship[1].colour = 	al_map_rgba(32, 0, 0, 20);
	Ship[2].colour = 	al_map_rgba(0, 0, 32, 20);
	Ship[3].colour = 	al_map_rgba(32, 32, 0, 20);

	for (i=0 ; i<num_ships ; i++)
	{
		Ship[i].lives      = Map.lives;
		Ship[i].user_fuel  = DEFAULT_FUEL;
		Ship[i].user_ammo1 = DEFAULT_AMMO1;
		Ship[i].ammo1_type = DEFAULT_AMMO1_TYPE;
		Ship[i].user_ammo2 = DEFAULT_AMMO2;
		Ship[i].ammo2_type = DEFAULT_AMMO2_TYPE;
		Ship[i].menu_state = MENU_AMMO1_LEVEL;
		if (Map.mission)
		{
			Ship[i].racing = true;
			Ship[i].current_lap_time = 0;
			Ship[i].user_ammo1 = 0;
			Ship[i].user_ammo2 = 8;
		}
		else
			Ship[i].racing = false;

		Ship[i].lap_complete = false;
		Ship[i].last_lap_time = 0;

		Ship[i].miners = 0;
		Ship[i].jewels = 0;
		Ship[i].sentries = 0;
		Ship[i].home_pad = i;

		reinit_ship(i);
	}
}


void reinit_ship(int i)
{
	Ship[i].thrust = FALSE;
	Ship[i].right = FALSE;
	Ship[i].left = FALSE;
	Ship[i].fire1 = 0;
	Ship[i].fire2 = 0;

	Ship[i].thrust_down = FALSE;
	Ship[i].right_down = FALSE;
	Ship[i].left_down = FALSE;
	Ship[i].fire1_down = 0;
	Ship[i].fire2_down = 0;

	Ship[i].thrust_held = FALSE;
	Ship[i].right_held = FALSE;
	Ship[i].left_held = FALSE;
	Ship[i].fire1_held = 0;
	Ship[i].fire2_held = 0;

	Ship[i].drag = Map.drag;
	Ship[i].gravity = Map.gravity;//9.81;

	Ship[i].angle = 0;
	Ship[i].xv = 0;
	Ship[i].yv = 0;

	//Ship[i].xpos = (Map.pad[i].min_x + Map.pad[i].max_x)>>1;
	//Ship[i].ypos = Map.pad[i].y-2;

	Ship[i].xpos = (Map.pad[Ship[i].home_pad].min_x + Map.pad[Ship[i].home_pad].max_x)>>1;
	Ship[i].ypos = Map.pad[Ship[i].home_pad].y-2;

	Ship[i].shield 		= 100;	//need to have user defaults for these.
	Ship[i].fuel 		= Ship[i].user_fuel<<4;
	Ship[i].ammo1 		= Ship[i].user_ammo1;
	Ship[i].ammo2 		= Ship[i].user_ammo2;
	Ship[i].mass = ShipMass(i);			//call function to work this out

	Ship[i].reincarnate_timer = 0;
	Ship[i].recharge_timer = 0;
	Ship[i].landed = TRUE;
	//Ship[i].pad = i;
	Ship[i].pad = Ship[i].home_pad;
	Ship[i].menu = FALSE;

	if (!Map.mission)	//keep timer running on mission levels
	{
		Ship[i].approaching = false;
		Ship[i].racing = false;
		Ship[i].current_lap_time = 0;
	}
}

void init_bullets(void)
{
	int i;

	for (i=0 ; i<MAX_BULLETS ; i++)
		Bullet[i].ttl = 0;
}

void init_keys(bool* pressed_keys)
{
	int i;

	for (i=0 ; i<ALLEGRO_KEY_MAX ; i++)
	{
		pressed_keys[i] = false;
	}
}

void init_joystick(void)
{
	fprintf(logfile,"Init GPIO joystick\n");

	wiringPiSetupPhys();	//set up wiring pi lib. Using phsical piun numbers (ie the numbers on th eheader)
	pinMode (26, INPUT) ;	//using pins 26,24,22,18,16. All input and pull up.
	pullUpDnControl(26,PUD_UP);
	pinMode (24, INPUT) ;
	pullUpDnControl(24,PUD_UP);
	pinMode (22, INPUT) ;
	pullUpDnControl(22,PUD_UP);
	pinMode (18, INPUT) ;
	pullUpDnControl(18,PUD_UP);
	pinMode (16, INPUT) ;
	pullUpDnControl(16,PUD_UP);
}

void general_init()
{

}
