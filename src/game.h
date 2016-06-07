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

//Target
#define  RPI 0
#define  WINDOWS 1

#define NAME "GravStorm"
#define VERSION "V0.3"

#define TRUE 1
#define FALSE 0

#define PI 3.141592654

#define MAX_SHIPS 4		//max number of players

#define SCREENX 1280//1920 //1024
#define SCREENY 720//1080 //768

#define STATUS_BAR_WIDTH 150
#define STATUS_BAR_HEIGHT 720

#define MAX_MAP_WIDTH 100
#define MAX_MAP_HEIGHT 100
extern int tile_map[MAX_MAP_WIDTH * MAX_MAP_HEIGHT];
extern int map_height, map_width;

#define MAP_X 100	//tiles
#define MAP_Y 100

#define TILE_SIZE	64
#define TILE_SHIFTS 6      //2^6 = 64
#define TILE_WIDTH  TILE_SIZE
#define TILE_HEIGHT TILE_SIZE

#define SHIP_SIZE_X 48
#define SHIP_SIZE_Y 48

#define GO_TIMER    50

typedef enum
{
	FULL = 0,
	TOP,
	BOTTOM,
	TOPLEFT,
	TOPRIGHT,
	BOTTOMLEFT,
	BOTTOMRIGHT
} ViewportType;

typedef struct
{
	int y;
	int min_x;
	int max_x;
	int type;	//0,1,2,3 for home bases, 4=fuel,5=ammo1,6=ammo2, 7shield
	int miners;
	int jewels;
} PadType;

typedef struct
{
	int min_x;	//boundaries of area - has to be rectangular
	int max_x;
	int min_y;
	int max_y;
	float gravity;	//gravity within area
	float drag;		//drag within area
} AreaType;

typedef struct
{
	int x;
	int y;
	float g;
} BlackHoleType;

//x y type(0/1) gun volcano firing period probability random/targeted
//Sentry guns and volcanoes
typedef struct
{
  int x;			//position bullet appears
  int y;
  int direction; 	//0-39
  int type;			//0=gun, 1=volcano, 2=targeted gun
  int period; 		//firing interval in frames
  int probability;	//percentage prob of firing when period expires
  int random; 		//max amount to add/subtract to x/y velocity
  int range;		//distance to a ship for targetted gun to start firing (note: squared!)
  int alive;
  int alive_sprite; //index into spite sheet for display
  int dead_sprite;
  int count;		//running count; reload to period, count down, fire on zero
  int shield;
} SentryType;

typedef struct
{
	int min_x;	//boundaries of area - has to be rectangular
	int half_x;
	int max_x;
	int min_y;
	int half_y;
	int max_y;
	float strength;
	int sentry;			//key to forcefield - when this dies, ff also dies
	int alive_sprite;	//index to sprite sheet - sprites from sentry sheet....
	int dead_sprite;
} ForcefieldType;

#define MAX_SENTRIES 30
#define MAX_PADS 12
#define MAX_AREAS 4
#define MAX_BLACKHOLES 4
#define MAX_FORCEFIELDS 5

typedef struct
{
	int type;					//0 for TR-style whole map, 1 for tiled
	char display_file_name[50];		//whole map for TR, or tiles for new style
	char background_file_name[50];
	char collision_file_name[50];	//whole map for TR, or tiles for new style
	char ascii_map_file_name[50];	//ascii file for tilemap
	char description_file_name[50]; //descriptive text for level.
	char sentry_file_name[50];
	char sentry_collision_file_name[50];
	int background_fade;
	int bg_fade_thresh;
	int ship_first;
	int max_players;
	int mission;
	int lives;
	int time_limit;
	PadType pad[MAX_PADS];
	int num_pads;
	int radial_gravity;
	BlackHoleType blackhole[MAX_BLACKHOLES];
	int num_blackholes;
	SentryType sentry[MAX_SENTRIES];
	int num_sentries;
	float gravity;
	float drag;
	int wrap;
	AreaType area[MAX_AREAS];
	int num_special_areas;	//with different g or drag
	ForcefieldType forcefield[MAX_FORCEFIELDS];
	int num_forcefields;
	int race;
	int raceline_minx;	//race start/finish line
	int raceline_maxx;
	int raceline_miny;
	int raceline_maxy;
	int before_minx;	//areas before/after start finish line to spot crossing
	int before_maxx;
	int before_miny;
	int before_maxy;
	int after_minx;
	int after_maxx;
	int after_miny;
	int after_maxy;

} MapType;

typedef struct
{
	int col;		//0,1,2
	int group;		//0-n
	int map;		//0-n
	int num_groups;	//
	int player;		//0-4
	int controls;	//0,1 keys, 2 GPIO joystick? 3 onwards USB joysticks.
	int current_key;
	int offset;		//to make columns slide
	int x_origin;
	int y_origin;
	int expand;
} MenuType;

extern ALLEGRO_DISPLAY *display;
extern ALLEGRO_FONT *menu_font;
extern ALLEGRO_FONT *small_font;
extern ALLEGRO_FONT *race_font;
extern ALLEGRO_FONT *font;
extern ALLEGRO_FONT *title_font;

//extern int num_ships;

extern ALLEGRO_BITMAP *ships;
extern ALLEGRO_BITMAP *sentries;
extern ALLEGRO_BITMAP *pickups;
extern ALLEGRO_BITMAP *miner;
extern ALLEGRO_BITMAP *jewel;
extern ALLEGRO_BITMAP *status_bg;
extern ALLEGRO_BITMAP *panel_bmp;
extern ALLEGRO_BITMAP *panel_pressed_bmp;
extern ALLEGRO_BITMAP *menu_bg_bmp;
extern ALLEGRO_BITMAP *bullets_bmp;
extern ALLEGRO_BITMAP *tr_map;
extern ALLEGRO_BITMAP *background;

#define MAP_NAME_LENGTH 50
#define MAX_MAPS 20	//per group
extern char map_names[MAP_NAME_LENGTH * MAX_MAPS];

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

#define MAX_GROUPS 5

extern MapGroupType MapNames[MAX_GROUPS];

extern MapType Map;
extern MenuType Menu;

//extern int tile_width, tile_height;

extern FILE* logfile;

ALLEGRO_VOICE *voice;
ALLEGRO_MIXER *mixer;

extern ALLEGRO_SAMPLE *wind;

extern ALLEGRO_SAMPLE_INSTANCE *clunk_inst;
extern ALLEGRO_SAMPLE_INSTANCE *wind_inst[MAX_SHIPS];
extern ALLEGRO_SAMPLE_INSTANCE *shoota_inst[MAX_SHIPS];
extern ALLEGRO_SAMPLE_INSTANCE *shootb_inst[MAX_SHIPS];
extern ALLEGRO_SAMPLE_INSTANCE *dead_inst[MAX_SHIPS];
extern ALLEGRO_SAMPLE_INSTANCE *particle_inst[MAX_SHIPS];
extern ALLEGRO_SAMPLE_INSTANCE *sentry_particle_inst;
extern ALLEGRO_SAMPLE_INSTANCE *yippee_inst;

extern bool pressed_keys[ALLEGRO_KEY_MAX];
extern int gpio_active;

int  ShipMass(int ship_num);
void FreeMenuBitmaps(void);
int read_maps(void);
void Exit(void);
