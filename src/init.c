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

#include <string.h>
#include <stdio.h>
#include <ctype.h>

#define ALLEGRO_UNSTABLE 1  //needed for haptics.

#include "allegro5/allegro.h"
#include "allegro5/allegro_image.h"
#include "allegro5/allegro_primitives.h"
#include "allegro5/allegro_font.h"
#include "allegro5/allegro_audio.h"
#ifdef ANDROID
#include <allegro5/allegro_android.h>
#endif

#include "game.h"
#include "init.h"
#include "objects.h"
#include "inputs.h"
#include "drawing.h"
#include "network.h"

#if RPI
#include <wiringPi.h>
#endif
ALLEGRO_FILE* map_file;	//text file containing map-specific data
const char txt[5] = ".txt\0";
const char scores[7] = "_s.bin\0";

ALLEGRO_COLOR ShipColour[8];
ALLEGRO_COLOR StatusColour[8];

void load_map_file(void);
void swap(int* a, int* b);

int get_map_players(int group, int map)
{
    //int i = 0, j = 0, l = 0, m = 0, n = 0, o = 0, p = 0;    //counters for pads, special areas, blackholes, sentries, forcefields etc.
    //int length;
    char map_file_name[MAP_NAME_LENGTH+1];
    char str[100];
    char *line;
    //int players;

    strncpy(map_file_name, (char *) &MapNames[group].Map[map], MAP_NAME_LENGTH);
    strcat(map_file_name, txt);

    map_file = al_fopen(map_file_name, "r");

    if (map_file == NULL) {
        al_fprintf(logfile, "Couldn't open file %s\n", map_file_name);
        return 1;
    }
    al_fprintf(logfile, "Opened map file %s\n", map_file_name);

    Map.max_players = 1;

    //while (fgets(str, 100, map_file) != NULL)
    while (al_fgets(map_file, str, 100) != NULL)
    {
        line = str;

        while (isspace(*line)) line++;

        if (strncmp(line, "max_players", 11) == 0)
        {
            sscanf(line + 11, " %d", &Map.max_players);
            al_fclose(map_file);
            return Map.max_players;
        }
    }
    al_fclose(map_file);
    return 1;
}



//parse:
//align label first column, parameters separated by spaces, suits scanf I think.
//so read line, check label, scanf, pattern determined by label.

//for network client: get sent name of map file, as that doesn't assume identical list of maps.
//so can either scan through list, to match name, and work out group and map then call this,
//or store received map name somewhere, come here and overwrite

int init_map(int group, int map)
{
	int i=0, j=0,l=0,m=0,n=0,o=0,p=0;	//counters for pads, special areas, blackholes, sentries, forcefields etc.
    int length;
	char map_file_name[MAP_NAME_LENGTH+1];
	char str[100];
	char *line;
	strncpy(map_file_name, (char*)&MapNames[group].Map[map], MAP_NAME_LENGTH);

	if (Net.client)
        strncpy(map_file_name, Net.mapfile ,MAP_NAME_LENGTH);

	strcat(map_file_name, txt);

	map_file = al_fopen(map_file_name,"r");

	if (map_file == NULL)
	{
		al_fprintf(logfile,"Couldn't open file %s\n",map_file_name);
		return 1;
	}
	al_fprintf(logfile,"Opened map file %s\n",map_file_name);

    //defaults.
    Map.type = 0;

    Map.display_file_name[0] = 0;
    Map.background_file_name[0] = 0;
    Map.collision_file_name[0] = 0;
    Map.display_file_name[0] = 0;
    Map.ascii_map_file_name[0] = 0;
    Map.description_file_name[0] = 0;
    Map.sentry_file_name[0] = 0;
    Map.sentry_collision_file_name[0] = 0;

    Map.background_fade = 0;
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
    Map.num_switches = 0;
    Map.num_racelines = 0;

    Map.total_miners = 0;

    Map.gravity = 0.05;
    Map.drag = 2;
    Map.wrap = 0;

    Map.race = 0;

	//while (fgets(str, 100, map_file) != NULL)
	while (al_fgets(map_file, str, 100) != NULL)
	{
		line = str;

		while (isspace(*line)) line++;

		if (strncmp(line,"map_type",8) == 0)
		{
			sscanf(line+8," %d",&Map.type);
			al_fprintf(logfile,"Map type:%d\n",Map.type);
		}

		else if (strncmp(line,"display_map",11) == 0)
		{
			sscanf(line+11," %s",(char *)&Map.display_file_name);
			al_fprintf(logfile,"Display Map:%s\n",Map.display_file_name);
		}

		else if (strncmp(line,"background",10) == 0)
		{
			sscanf(line+10," %s",(char *)&Map.background_file_name);
			al_fprintf(logfile,"Map background:%s\n",Map.background_file_name);
		}

		else if (strncmp(line,"bg_fade",7) == 0)
		{
			Map.background_fade = true;
			sscanf(line+7," %d",&Map.bg_fade_thresh);
			al_fprintf(logfile,"Background fading: On\n");
			al_fprintf(logfile,"Background fade threshold:%d\n",Map.bg_fade_thresh);
		}

		else if (strncmp(line,"collision_map",13) == 0)
		{
			sscanf(line+13," %s",(char *)&Map.collision_file_name);
			al_fprintf(logfile,"Collision Map:%s\n",Map.collision_file_name);
		}

		else if (strncmp(line,"ascii_map",9) == 0)
		{
			sscanf(line+9," %s",(char *)&Map.ascii_map_file_name);
			al_fprintf(logfile,"ASCII map file:%s\n",Map.ascii_map_file_name);
			load_map_file();
		}

		else if (strncmp(line,"sentry_display",14) == 0)
		{
			sscanf(line+14," %s",(char *)&Map.sentry_file_name);
			al_fprintf(logfile,"Sentry Image file:%s\n",Map.sentry_file_name);
		}

		else if (strncmp(line,"sentry_collision",16) == 0)
		{
			sscanf(line+16," %s",(char *)&Map.sentry_collision_file_name);
			al_fprintf(logfile,"Sentry Collision file:%s\n",Map.sentry_collision_file_name);
		}

		else if (strncmp(line,"description",11) == 0)
		{
			sscanf(line+11," %s",(char *)&Map.description_file_name);
			al_fprintf(logfile,"Description file:%s\n",Map.description_file_name);
		}

		else if (strncmp(line,"ship_first",10) == 0)
		{
			sscanf(line+10," %d",&Map.ship_first);
			al_fprintf(logfile,"Ship first:%d\n",Map.ship_first);
		}

		else if (strncmp(line,"mission",7) == 0)
		{
			sscanf(line+7," %d",&Map.mission);
			al_fprintf(logfile,"Mission:%d\n",Map.mission);
		}

		else if (strncmp(line,"lives",5) == 0)
		{
			sscanf(line+5," %d",&Map.lives);
			if (Map.lives > 6)
				Map.lives = 6;

			al_fprintf(logfile,"Lives:%d\n",Map.lives);
		}

		else if (strncmp(line,"time_limit",10) == 0)
		{
			sscanf(line+10," %d",&Map.time_limit);
			al_fprintf(logfile,"Time Limit:%d\n",Map.time_limit);
		}

		else if (strncmp(line,"max_players",11) == 0)
		{
			sscanf(line+11," %d",&Map.max_players);
			al_fprintf(logfile,"Max players:%d\n",Map.max_players);
		}

		else if (strncmp(line,"gravity",7) == 0)
		{
			sscanf(line+7," %f",&Map.gravity);
			al_fprintf(logfile,"Gravity:%f\n",Map.gravity);
		}

		else if (strncmp(line,"drag",4) == 0)
		{
			sscanf(line+4," %f",&Map.drag);
			al_fprintf(logfile,"Drag:%f\n",Map.drag);
		}

		else if (strncmp(line,"wrap",4) == 0)
		{
			sscanf(line+4," %d",&Map.wrap);
			al_fprintf(logfile,"Wrap:%d\n",Map.wrap);
		}

		else if (strncmp(line,"pad",3) == 0)
		{
			Map.pad[i].miners = 0;
			Map.pad[i].jewels = 0;
			sscanf(line+3," %x %d %d %d %d %d",(unsigned int*)&Map.pad[i].type,&Map.pad[i].y,&Map.pad[i].min_x,&Map.pad[i].max_x,&Map.pad[i].miners,&Map.pad[i].jewels);
			al_fprintf(logfile,"Pad %d: type:%02x y:%d x:%d x:%d miners:%d jewels:%d\n",i,Map.pad[i].type,Map.pad[i].y,Map.pad[i].min_x,Map.pad[i].max_x,Map.pad[i].miners,Map.pad[i].jewels);

			if ((Map.pad[i].type & 0x000f) < MAX_SHIPS)
	            Ship[Map.pad[i].type & 0x000f].home_pad = i;    //bottom nibble of type gives ship which this is home pad for.

            Map.total_miners += Map.pad[i].miners;

            i++;
		}

		else if (strncmp(line,"area",4) == 0)
		{
			sscanf(line+4," %d %d %d %d %f %f",                           &Map.area[j].min_x,&Map.area[j].max_x,&Map.area[j].min_y,&Map.area[j].max_y,&Map.area[j].gravity,&Map.area[j].drag);
			al_fprintf(logfile,"area %d: x:%d x:%d y:%d y:%d g:%f drag:%f\n",j,Map.area[j].min_x, Map.area[j].max_x, Map.area[j].min_y, Map.area[j].max_y, Map.area[j].gravity, Map.area[j].drag);
			j++;
		}

		else if (strncmp(line,"blackhole",9) == 0)
		{
			Map.radial_gravity = true;
			if (l==0)
				al_fprintf(logfile,"Radial Gravity On\n");

			sscanf(line+9," %d %d %f",                        &Map.blackhole[l].x,&Map.blackhole[l].y,&Map.blackhole[l].g);
			al_fprintf(logfile,"blackhole %d: x:%d y:%d g:%f\n",l,Map.blackhole[l].x,Map.blackhole[l].y,Map.blackhole[l].g);
			l++;
		}

		//      x y type(0/1/2) gun volcano firing period probability random/targeted
		//sentry

		else if (strncmp(line,"sentry",6) == 0)
		{
			sscanf(line+6," %i %i %i %i %i %i %i %i %i %i",                                                                   &Map.sentry[m].x, &Map.sentry[m].y, &Map.sentry[m].direction, &Map.sentry[m].type, &Map.sentry[m].period, &Map.sentry[m].probability, &Map.sentry[m].random, &Map.sentry[m].range, &Map.sentry[m].alive_sprite, &Map.sentry[m].dead_sprite);
			al_fprintf(logfile,"Sentry %i: x:%i, y:%i, Direction:%i, Type:%i, Period:%i, Prob:%i, Random:%i, Range:%i, Sprite%d, Sprite%d\n",m, Map.sentry[m].x,  Map.sentry[m].y,  Map.sentry[m].direction,  Map.sentry[m].type,  Map.sentry[m].period,  Map.sentry[m].probability,  Map.sentry[m].random,  Map.sentry[m].range, Map.sentry[m].alive_sprite, Map.sentry[m].dead_sprite);
			Map.sentry[m].range = Map.sentry[m].range * Map.sentry[m].range;	//square to save square rooting later.
			Map.sentry[m].count = Map.sentry[m].period;	//init countdown timer
			Map.sentry[m].shield = SENTRY_SHIELD;		//init shield
			Map.sentry[m].alive = 1;
			m++;
		}

		else if (strncmp(line,"race",4) == 0)
		{
			sscanf(line+4," %d %d %d %d %d",&Map.raceline[p].line_minx,&Map.raceline[p].line_maxx,&Map.raceline[p].line_miny,&Map.raceline[p].line_maxy,&Map.raceline[p].reverse);
			Map.race = true;
			if (Map.raceline[p].line_minx == Map.raceline[p].line_maxx)
			{
				Map.raceline[p].before_maxx = Map.raceline[p].line_minx + 20;	//experimental
				Map.raceline[p].after_minx  = Map.raceline[p].line_minx - 20;

                Map.raceline[p].before_minx = Map.raceline[p].line_minx;
                Map.raceline[p].after_maxx  = Map.raceline[p].line_minx;

				Map.raceline[p].before_miny = Map.raceline[p].line_miny;
				Map.raceline[p].after_miny  = Map.raceline[p].line_miny;

				Map.raceline[p].before_maxy = Map.raceline[p].line_maxy;
				Map.raceline[p].after_maxy  = Map.raceline[p].line_maxy;
			}
			else if (Map.raceline[p].line_miny == Map.raceline[p].line_maxy)
			{
				Map.raceline[p].before_miny = Map.raceline[p].line_miny - 20;
				Map.raceline[p].after_maxy  = Map.raceline[p].line_miny + 20;

				Map.raceline[p].before_maxy = Map.raceline[p].line_miny;
				Map.raceline[p].after_miny  = Map.raceline[p].line_miny;

				Map.raceline[p].before_minx = Map.raceline[p].line_minx;
				Map.raceline[p].after_minx  = Map.raceline[p].line_minx;

				Map.raceline[p].before_maxx = Map.raceline[p].line_maxx;
				Map.raceline[p].after_maxx  = Map.raceline[p].line_maxx;
			}
			else al_fprintf(logfile,"X or Y values must match in race start/finish line (i.e. line must be horizontal or vertical)\n");

            if (Map.raceline[p].reverse)
            {
                swap(&Map.raceline[p].before_miny, &Map.raceline[p].after_miny);
                swap(&Map.raceline[p].before_maxy, &Map.raceline[p].after_maxy);
                swap(&Map.raceline[p].before_minx, &Map.raceline[p].after_minx);
                swap(&Map.raceline[p].before_maxx, &Map.raceline[p].after_maxx);
            }
			al_fprintf(logfile,"Race %d: x:%d x:%d y:%d y:%d\n",p,Map.raceline[p].line_minx,Map.raceline[p].line_maxx,Map.raceline[p].line_miny,Map.raceline[p].line_maxy);
			al_fprintf(logfile,"Before: x:%d x:%d y:%d y:%d\n",Map.raceline[p].before_minx,Map.raceline[p].before_maxx,Map.raceline[p].before_miny,Map.raceline[p].before_maxy);
			al_fprintf(logfile,"After:  x:%d x:%d y:%d y:%d\n",Map.raceline[p].after_minx,Map.raceline[p].after_maxx,Map.raceline[p].after_miny,Map.raceline[p].after_maxy);
			p++;
		}

		else if (strncmp(line,"forcefield",10) == 0)
		{
			sscanf(line+10," %d %d %d %d %f %d %d %d %d",&Map.forcefield[n].min_x,&Map.forcefield[n].max_x,&Map.forcefield[n].min_y,&Map.forcefield[n].max_y,&Map.forcefield[n].strength,&Map.forcefield[n].switch1,&Map.forcefield[n].switch2,&Map.forcefield[n].closed_sprite,&Map.forcefield[n].open_sprite);
			if (Map.forcefield[n].min_x == Map.forcefield[n].max_x)	//vertical line
			{
				Map.forcefield[n].half_x = Map.forcefield[n].min_x;
				Map.forcefield[n].min_x = Map.forcefield[n].half_x - 20;
				Map.forcefield[n].max_x = Map.forcefield[n].half_x + 20;

				Map.forcefield[n].half_y = Map.forcefield[n].min_y;	//don't need half_y, so set = min_y and use as test for line direction.
			}
			else if (Map.forcefield[n].min_y == Map.forcefield[n].max_y)	//horizontal line
			{
				Map.forcefield[n].half_y = Map.forcefield[n].min_y;
				Map.forcefield[n].min_y = Map.forcefield[n].half_y - 20;
				Map.forcefield[n].max_y = Map.forcefield[n].half_y + 20;

				Map.forcefield[n].half_x = Map.forcefield[n].min_x;
			}

			else al_fprintf(logfile,"X or Y values must match in forcefield (i.e. line must be horizontal or vertical)\n");

			Map.forcefield[n].alpha = 255;
			Map.forcefield[n].state = CLOSED;

			al_fprintf(logfile,"ForceField: x:%d x:%d x:%d y:%d y:%d y:%d\n",Map.forcefield[n].min_x,Map.forcefield[n].half_x,Map.forcefield[n].max_x,Map.forcefield[n].min_y,Map.forcefield[n].half_y,Map.forcefield[n].max_y);
			al_fprintf(logfile,"ForceField: strength:%0.0f switch1:%d, switch2:%d\n",Map.forcefield[n].strength,Map.forcefield[n].switch1,Map.forcefield[n].switch2);
			n++;
		}

		else if (strncmp(line,"switch",6) == 0)
		{
			sscanf(line+6," %i %i %i %i %i ",                                        &Map.switches[o].x, &Map.switches[o].y, &Map.switches[o].closed_sprite, &Map.switches[o].open_sprite, &Map.switches[o].open_time);
			al_fprintf(logfile,"Switch %i: x:%i, y:%i, Sprite%d, Sprite%d, Time:%d\n",o, Map.switches[o].x,  Map.switches[o].y,  Map.switches[o].closed_sprite,  Map.switches[o].open_sprite,  Map.switches[o].open_time);
			Map.switches[o].shield = SENTRY_SHIELD;		//init shield
			Map.switches[o].open = 0;
			o++;
		}
	}

	Map.num_pads = i;
	al_fprintf(logfile,"%d pads\n",Map.num_pads);
	Map.num_special_areas = j;
	al_fprintf(logfile,"%d areas\n",Map.num_special_areas);
	Map.num_blackholes = l;
	al_fprintf(logfile,"%d blackholes\n",Map.num_blackholes);
	Map.num_sentries = m;
	al_fprintf(logfile,"%d sentries\n",Map.num_sentries);
	Map.num_forcefields = n;
	al_fprintf(logfile,"%d forcefields\n",Map.num_forcefields);
	Map.num_switches = o;
	al_fprintf(logfile,"%d switches\n",Map.num_switches);
	Map.num_racelines = p;
	al_fprintf(logfile,"%d racelines\n",Map.num_racelines);

	if (Map.display_file_name[0] == 0)
		al_fprintf(logfile,"ERROR: No display file specified\n");
	if (Map.collision_file_name[0] == 0)
		al_fprintf(logfile,"ERROR: No collision file specified\n");
	if (Map.type == 1)
	{
		if (Map.ascii_map_file_name[0] == 0)
			al_fprintf(logfile,"ERROR: No ASCII map file specified\n");
	}
	if (Map.num_sentries != 0)
	{
		if (Map.sentry_file_name[0] == 0)
			al_fprintf(logfile,"ERROR: No sentry display file specified\n");

		if (Map.sentry_collision_file_name[0] == 0)
			al_fprintf(logfile,"Possible error: No sentry collision file specified. Sentries will be indestructible.\n");
	}
	if (Map.num_pads == 0)
		al_fprintf(logfile,"ERROR: No pads specified\n");

	al_fprintf(logfile,"--End of mapfile %s--\n",map_file_name);

	al_fflush(logfile);
	al_fclose  (map_file);

	//read hi scores
	if (Map.mission || Map.race)
    {
        strncpy(map_file_name, (char*)&MapNames[group].Map[map], MAP_NAME_LENGTH);
        strcat(map_file_name, scores);
#ifdef ANDROID
        char pathstr[100];
        char *pathptr;
        al_set_standard_file_interface();
        ALLEGRO_PATH *path = al_get_standard_path(ALLEGRO_USER_DATA_PATH);    //android
        //ALLEGRO_DEBUG(#std ": %s", al_path_cstr(path, '/'));
        sprintf(pathstr,"%s",al_path_cstr(path, '/'));
        al_change_directory(al_path_cstr(path, '/'));  // change the working directory
        pathptr = al_get_current_directory();
#endif
        map_file = al_fopen(map_file_name,"r");
        if (map_file == NULL)
        {
            al_fprintf(logfile,"Couldn't open scores/times file %s. Using default high scores/times.\n",map_file_name);
            for (i=0 ; i<MAX_SCORES ; i++)
            {
                if (Map.mission)
                {
                    Map.oldscore[i].score = 5000-500*i;
                    strncpy(Map.oldscore[i].name,"IPL",4);
                }
                else if (Map.race)
                {
                    Map.oldtime[i].time = 40+5*i;
                    strncpy(Map.oldtime[i].name,"IPL",4);
                }
            }
        }
        else
        {
            al_fprintf(logfile,"Opened scores/times file %s\n",map_file_name);
            j=0;
            //while (fgets(str, 100, map_file) != NULL)
            while (al_fgets(map_file, str, 100) != NULL)
            {
                length = strlen(str);
                for (i=0 ; i<length-1 ; i++)
                    str[i] -= 0x81+j;
                if (Map.mission)
                {
                    sscanf(str,"%d %s",&Map.oldscore[j].score,&Map.oldscore[j].name[0]);
                    //al_fprintf(logfile,"%d %s\n",Map.oldscore[j].score,Map.oldscore[j].name);
                }
                else if (Map.race)
                {
                    sscanf(str,"%f %s",&Map.oldtime[j].time,&Map.oldtime[j].name[0]);
                    al_fprintf(logfile,"%.3f %s\n",Map.oldtime[j].time,Map.oldtime[j].name);
                }
                j++;
                if (j==MAX_SCORES) break;
            }
            al_fclose(map_file);
        }
#ifdef ANDROID
        al_android_set_apk_file_interface();
#endif
    }

	return 0;
}

void swap(int* a, int*b)
{
    int temp;
    temp=*a;
    *a=*b;
    *b=temp;
}

void load_map_file(void)
{
	int i,j,found;
    ALLEGRO_FILE* map_file;
	unsigned char line[200];    //unsigned to allow values up to 256 for maps with many tiles.

	if ((map_file = al_fopen(Map.ascii_map_file_name,"r")) == NULL)  al_fprintf(logfile,"Couldn't open %s for reading.\n",Map.ascii_map_file_name);

	map_width = 0;

	j=0;
	while(1)
	{
		//if (fgets((char *)line,200,map_file) == NULL)	//get a line from the file, exit on end of file
		if (al_fgets(map_file, (char *)line,200) == NULL)	//get a line from the file, exit on end of file
			break;

		i=0;
		found = false;
		while(line[i] != 0x0a && line[i] != 0x0d)	//line ends with CR or LF
		{
			if (line[i] == ' ')							//space counts as zero
				tile_map[i+MAX_MAP_WIDTH*j] = 0;
			else if (line[i] >='0' && line [i] <= '9')	//ascii 0-9 map to integer 0-9
				tile_map[i+MAX_MAP_WIDTH*j] = line[i]-'0';
			else if (/*toupper*/(line[i]) >='A' /*&& toupper(line[i]) <= 'Z'*/)	//ascii A-Z(or a-z) map to 10-36
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

	al_fclose(map_file);

	al_fprintf(logfile,"ASCII Map: H:%d W:%d\n",map_height,map_width);
}

void init_controls(void)
{
	int i;
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

	Ship[2].controller = USB_JOYSTICK0;	//keys or joystick
	Ship[2].selected_controller = USB_JOYSTICK0;
    Ship[2].up_key     = ALLEGRO_KEY_Q;
    Ship[2].down_key   = ALLEGRO_KEY_A;
    Ship[2].left_key   = ALLEGRO_KEY_R;
    Ship[2].right_key  = ALLEGRO_KEY_T;
    Ship[2].thrust_key = ALLEGRO_KEY_ALT;

	Ship[3].controller = USB_JOYSTICK1;	//keys or joystick
	Ship[3].selected_controller = USB_JOYSTICK1;
	Ship[3].up_key     = ALLEGRO_KEY_UP;
	Ship[3].down_key   = ALLEGRO_KEY_DOWN;
	Ship[3].left_key   = ALLEGRO_KEY_LEFT;
	Ship[3].right_key  = ALLEGRO_KEY_RIGHT;
	Ship[3].thrust_key = ALLEGRO_KEY_RCTRL;

  #ifdef ANDROID

    Ship[0].controller = TOUCH_JOYSTICK;
	TouchJoystick.spin = 0;

	//On-screen controls
    if ((Ctrl.controls = al_load_bitmap("controls.png")) == NULL)  al_fprintf(logfile,"controls.png load fail");

	int dw,dh;//,bw,bh;
	dw = al_get_display_width(display);
	dh = al_get_display_height(display);

	Ctrl.reversed = false;
    Ctrl.ButtonSize = 100*scale;  //tweak for scaling????

	//init size/position of buttons
	//could read/store this in file, but I guess still need this for defaults....
	Ctrl.ctrl[DPAD].active = FALSE;
	Ctrl.ctrl[DPAD].size = 2*Ctrl.ButtonSize;//al_get_bitmap_width(Ctrl.direction.bmp);
	Ctrl.ctrl[DPAD].x = 0.98*dw-Ctrl.ctrl[DPAD].size;
	Ctrl.ctrl[DPAD].y = 0.98*dh-Ctrl.ctrl[DPAD].size;
	Ctrl.ctrl[DPAD].movex = 10;
	Ctrl.ctrl[DPAD].movey = 10;

    Ctrl.ctrl[ASTICK].active = FALSE;
    Ctrl.ctrl[ASTICK].size = 2*Ctrl.ButtonSize;//al_get_bitmap_width(Ctrl.direction.bmp);
    Ctrl.ctrl[ASTICK].border = Ctrl.ctrl[ASTICK].size*0.5;
    Ctrl.ctrl[ASTICK].x = 0.98*dw-Ctrl.ctrl[ASTICK].size;
    Ctrl.ctrl[ASTICK].y = 0.98*dh-Ctrl.ctrl[ASTICK].size;
	Ctrl.ctrl[ASTICK].movex = 10;
	Ctrl.ctrl[ASTICK].movey = 10;

    Ctrl.ctrl[ASTICK2].active = FALSE;
    Ctrl.ctrl[ASTICK2].size = 2*Ctrl.ButtonSize;//al_get_bitmap_width(Ctrl.direction.bmp);
    Ctrl.ctrl[ASTICK2].border = 0;
    Ctrl.ctrl[ASTICK2].x = 0.98*dw-Ctrl.ctrl[ASTICK2].size;
    Ctrl.ctrl[ASTICK2].y = 0.98*dh-Ctrl.ctrl[ASTICK2].size;
	Ctrl.ctrl[ASTICK2].movex = 10;
	Ctrl.ctrl[ASTICK2].movey = 10;

	Ctrl.ctrl[THRUST_BUTTON].active = FALSE;
	Ctrl.ctrl[THRUST_BUTTON].size = 1.5*Ctrl.ButtonSize;//al_get_bitmap_width(Ctrl.thrust.bmp);
    Ctrl.ctrl[THRUST_BUTTON].border = 0;//Ctrl.ctrl[THRUST_BUTTON].size*0.3;
	Ctrl.ctrl[THRUST_BUTTON].x = 0.01*dw;//-Ctrl.ctrl[THRUST_BUTTON].size;//+Ctrl.ButtonSize*0.7;
	Ctrl.ctrl[THRUST_BUTTON].y = 0.99*dh-Ctrl.ctrl[THRUST_BUTTON].size;//-Ctrl.ButtonSize*0.7;
	Ctrl.ctrl[THRUST_BUTTON].movex = 0;
	Ctrl.ctrl[THRUST_BUTTON].movey = 10;

	Ctrl.ctrl[SELECT].active = TRUE;
	Ctrl.ctrl[SELECT].size = 1.5*Ctrl.ButtonSize;//al_get_bitmap_width(Ctrl.thrust.bmp);
    Ctrl.ctrl[SELECT].border = 0;
	Ctrl.ctrl[SELECT].x = 0.99*dw-Ctrl.ButtonSize*1.7;
	Ctrl.ctrl[SELECT].y = 0.99*dh-Ctrl.ctrl[SELECT].size;//-Ctrl.ButtonSize*0.7;
	Ctrl.ctrl[SELECT].movex = 0;
	Ctrl.ctrl[SELECT].movey = 10;

    Ctrl.ctrl[FIRE1].active = FALSE;
    Ctrl.ctrl[FIRE1].size = Ctrl.ButtonSize;//al_get_bitmap_width(Ctrl.thrust.bmp);
    Ctrl.ctrl[FIRE1].border = 0;
    Ctrl.ctrl[FIRE1].x = 0.02*dw;
    Ctrl.ctrl[FIRE1].y = 0.98*dh-Ctrl.ctrl[FIRE1].size-Ctrl.ButtonSize*1.5;
	Ctrl.ctrl[FIRE1].movex = 0;
	Ctrl.ctrl[FIRE1].movey = 20;

    Ctrl.ctrl[FIRE2].active = FALSE;
    Ctrl.ctrl[FIRE2].size = Ctrl.ButtonSize;//al_get_bitmap_width(Ctrl.thrust.bmp);
    Ctrl.ctrl[FIRE2].border = 0;
    Ctrl.ctrl[FIRE2].x = 0.02*dw+Ctrl.ButtonSize*1.5;
    Ctrl.ctrl[FIRE2].y = 0.98*dh-Ctrl.ctrl[FIRE2].size;
	Ctrl.ctrl[FIRE2].movex = -10;
	Ctrl.ctrl[FIRE2].movey = 10;

    Ctrl.ctrl[BACK].active = TRUE;
	Ctrl.ctrl[BACK].size = Ctrl.ButtonSize;//al_get_bitmap_width(Ctrl.escape.bmp);
    Ctrl.ctrl[BACK].border = 0;
	Ctrl.ctrl[BACK].x = 0.5*dw - (Ctrl.ctrl[BACK].size * 2.60);
	Ctrl.ctrl[BACK].y = 0.02*dh;
	Ctrl.ctrl[BACK].movex = 20;
	Ctrl.ctrl[BACK].movey = 0;

    Ctrl.ctrl[SMALLER].active = TRUE;
    Ctrl.ctrl[SMALLER].size = Ctrl.ButtonSize;//al_get_bitmap_width(Ctrl.escape.bmp);
    Ctrl.ctrl[SMALLER].border = 0;
    Ctrl.ctrl[SMALLER].x = 0.5*dw - (Ctrl.ctrl[SMALLER].size * 1.55);
    Ctrl.ctrl[SMALLER].y = 0.02*dh;
	Ctrl.ctrl[SMALLER].movex = 10;
	Ctrl.ctrl[SMALLER].movey = 0;

    Ctrl.ctrl[BIGGER].active = TRUE;
    Ctrl.ctrl[BIGGER].size = Ctrl.ButtonSize;//al_get_bitmap_width(Ctrl.escape.bmp);
    Ctrl.ctrl[BIGGER].border = 0;
    Ctrl.ctrl[BIGGER].x = 0.5*dw - (Ctrl.ctrl[BIGGER].size * 0.5);
    Ctrl.ctrl[BIGGER].y = 0.02*dh;
	Ctrl.ctrl[BIGGER].movex = 0;
	Ctrl.ctrl[BIGGER].movey = 0;

    Ctrl.ctrl[REVERSE].active = FALSE;
    Ctrl.ctrl[REVERSE].size = Ctrl.ButtonSize;//al_get_bitmap_width(Ctrl.escape.bmp);
    Ctrl.ctrl[REVERSE].border = 0;
    Ctrl.ctrl[REVERSE].x = 0.5*dw + (Ctrl.ctrl[RADAR].size * 0.55);
    Ctrl.ctrl[REVERSE].y = 0.02*dh;
    Ctrl.ctrl[REVERSE].movex = -10;
    Ctrl.ctrl[REVERSE].movey = 0;

    Ctrl.ctrl[RADAR].active = FALSE;
    Ctrl.ctrl[RADAR].size = Ctrl.ButtonSize;//al_get_bitmap_width(Ctrl.escape.bmp);
    Ctrl.ctrl[RADAR].border = 0;
    Ctrl.ctrl[RADAR].x = 0.5*dw + (Ctrl.ctrl[RADAR].size * 1.60);
    Ctrl.ctrl[RADAR].y = 0.02*dh;
	Ctrl.ctrl[RADAR].movex = -20;
	Ctrl.ctrl[RADAR].movey = 0;


    //init touch array
    for (i=0 ; i<NUM_TOUCHES ; i++)
    {
        Touch[i].id = NO_TOUCH;
        Touch[i].button = NO_BUTTON;
        Touch[i].count = 0;
        Touch[i].x = 0;
        Touch[i].y = 0;
        Touch[i].valid = 0;
    }

#endif // ANDROID
    float line_space = 35*font_scale;

    Select.sumdymax =  2*line_space;       //start offset 2 lines from top.
    Select.sumdymin = Select.sumdymax;//-4*line_space;  //4 lines on first screen
    Select.sumdy =  Select.sumdymax;       //don't go below that.

	//Colours - redundant??
	StatusColour[0] = al_map_rgba(0, 32, 0, 20);  //used for background to status (ammo, fuel, lives etc)
    StatusColour[1] = al_map_rgba(32, 0, 0, 20);
    StatusColour[2] = al_map_rgba(0, 0, 32, 20);
    StatusColour[3] = al_map_rgba(32, 32, 0, 20);
    StatusColour[4] = al_map_rgba(32, 16, 0, 20);
    StatusColour[5] = al_map_rgba(0, 32, 32, 20);
    StatusColour[6] = al_map_rgba(32, 32,32, 20);
    StatusColour[7] = al_map_rgba(32, 0, 32, 20);

	ShipColour[0] = al_map_rgb(0, 255, 0);  //used for radar
    ShipColour[1] = al_map_rgb(255, 0, 0);
    ShipColour[2] = al_map_rgb(0, 64, 255);
    ShipColour[3] = al_map_rgb(255, 255, 0);
    ShipColour[4] = al_map_rgb(255, 128, 0);
    ShipColour[5] = al_map_rgb(0, 255, 255);
    ShipColour[6] = al_map_rgb(255, 255 ,255);
    ShipColour[7] = al_map_rgb(255, 0, 255);

    for (i=0 ; i<MAX_SHIPS ; i++)
    {
        Ship[i].image = i;
        Ship[i].colour = ShipColour[i];
        Ship[i].statuscolour = StatusColour[i];
    }
}

void init_ships(int num_ships)
{
	int i;

	for (i=0 ; i<MAX_SHIPS ; i++)   //init all, as we can have late joiner on network games.
	{
		//Ship[i].image      = i+4;
		Ship[i].lives      = Map.lives;
		Ship[i].crashed    = 0;
		Ship[i].killed     = 0;
		Ship[i].kills      = 0;
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
			Ship[i].user_ammo1 = DEFAULT_AMMO1;
			Ship[i].user_ammo2 = DEFAULT_AMMO2;
		}
		else
			Ship[i].racing = false;

		Ship[i].lap_complete = false;
		Ship[i].last_lap_time = 0;
        Ship[i].best_lap_time = 999999;

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
    Ship[i].fangle = 0;
	Ship[i].xv = 0;
	Ship[i].yv = 0;

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
	Ship[i].pad = Ship[i].home_pad;
	Ship[i].menu = FALSE;

	if (!Map.mission)	//keep timer running on mission levels
	{
		Ship[i].approaching_sf = false;
		Ship[i].approaching_next = false;
		Ship[i].racing = false;
		Ship[i].current_lap_time = 0;
        Ship[i].current_raceline = 0;
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
#if RPI
	//al_fprintf(logfile,"Init GPIO joystick\n");

	wiringPiSetupPhys();	//set up wiring pi lib. Using physical pin numbers (i.e. the numbers on the header)
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
#endif // TARGET
}

void general_init()
{

}

