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

#define NUM_ANGLES 40
#define ANGLE_INC 360/NUM_ANGLES
#define ANGLE_INC_RAD 2*PI/NUM_ANGLES

#define EMPTY_SHIP_MASS 50
#define MINER_MASS 		20
#define JEWEL_MASS 		20

#define THRUST 50
#define G 1000

#define BLT_NORMAL		0
#define BLT_RANDOM		1
#define BLT_DOUBLE		2
#define BLT_TRIPLE		3
#define BLT_HEAVY		4
#define BLT_MINE		5
#define BLT_HEATSEEKER	6
#define BLT_SPREADER	7
#define BLT_LAVA		8
#define BLT_SENTRY		9
#define BULLET_TYPES 	10

#define DEFAULT_FUEL		75
#define DEFAULT_AMMO1		50
#define DEFAULT_AMMO1_TYPE	BLT_NORMAL
#define DEFAULT_AMMO2		4
#define DEFAULT_AMMO2_TYPE	BLT_HEAVY

#define BULLET_SPEED 10
#define MAX_BULLETS 200	//max we store/process at one time
#define END_OF_LIST 0xffff	//end of linked list of bullets

#define LEFT	0x01	//flags to NewBullet()
#define RIGHT   0x02
#define RANDOM  0x04
#define MINE	0x08

//HS_ACCN / MIN_R_SQUARED = THRUST
//HS_ACCN = THRUST * MIN_R_SQUARED

#define MIN_R 200	//
#define MIN_R_SQUARED (MIN_R*MIN_R)
#define HS_ACCN (THRUST*MIN_R_SQUARED) //500000 //70 //5000	//Heatseeker

#define PAD_FUEL   0x0010	//bit/mask defines for what a given pad will refuel
#define PAD_AMMO1  0x0020
#define PAD_AMMO2  0x0040
#define PAD_SHIELD 0x0080
#define PAD_MINER  0x0100
#define PAD_JEWEL  0x0200

#define MENU_AMMO1_LEVEL 0
#define MENU_AMMO1_TYPE  1
#define MENU_AMMO2_LEVEL 2
#define MENU_AMMO2_TYPE  3
#define MENU_FUEL_LEVEL  4
#define MENU_MAX		 4

#define SENTRY_SHIELD	15

typedef struct
{
	//key mapping
	int controller;	//keys or joystick - live
	int selected_controller; //remember what we chose to replace N/As in menu
	int up_key;
	int down_key;
	int left_key;
	int right_key;
	int thrust_key;

	//states of keys - pack into single int?
	int thrust;	//KEEP THRUST
	int right;
	int left;
	int fire1;
	int fire2;

	//one-shot
	int thrust_down;	//boolean
	int right_down;
	int left_down;
	int fire1_down;
	int fire2_down;

	//continuous
	int thrust_held;	//boolean
	int right_held;
	int left_held;
	int fire1_held;
	int fire2_held;

	//constants, but allow local variation
	float drag;
	float gravity;
	float x_force;	//extra forces, added for bouncing forcefields.
	float y_force;

	//calculated angle, position and velocity
	int angle;  //0 to NUM_ANGLES-1
	float xpos;
	float ypos;
	float xv;
	float yv;

	//need 3 values for fuel, ammo1,2 levels
	//1.default, always there when you start a game
	//2.user default - set up in ship menu, this is what you refuel to, and what a new ship starts at
	//3.current.

	//Stuff
	int lives;
	int shield;
	int fuel;
	int ammo1;		//normal - only 1 type for now
	int ammo1_type;
	int fire1_reload;//shoot every 10? frames
	int last_position; //for left/right firing
	int ammo2;		//special
	int ammo2_type;	//heavy, mine, heatseeker, spreader. others?
	int mass;		//derived from above
	int miners;		//rescuees on rescue missions
	int miner_count;//animation
	int jewels;		//bonus pickup on rescue missions
	int jewel_count;//animation
	int sentries;	//sentries destroyed

	//user defaults
	int user_fuel;
	int user_ammo1;
	int user_ammo2;

	int home_pad;

    int recharge_timer;		//set to zero on landing, counts up.
	int reincarnate_timer;	//set to non-zero on destruction, counts down to zero

	//int start_xpos;	//read from map struct
	//int start_ypos;

	int landed;		//config menu
	int pad;
	int menu;
	int menu_state;

	int	approaching;	//race
	int racing;
	int lap_complete;
	float current_lap_time;
	float last_lap_time;

	ALLEGRO_COLOR colour;	//background colour for status display
	ALLEGRO_BITMAP* status_bg;

} ShipType;

typedef struct
{
	int type;	//0 for 'normal', 1,2,3,4 for specials....
	float xpos;
	float ypos;
	float xv;
	float yv;
	int ttl;	//time-to-live. in frames. Puts a limit on how many we can have
	int damage;
	float mass;
	ALLEGRO_COLOR colour;
	int next_bullet;
} BulletType;

extern ShipType Ship[MAX_SHIPS];
extern num_ships;
extern BulletType Bullet[MAX_BULLETS];
extern int first_bullet;
extern int mapx, mapy;

//Trig LUTs
extern float sinlut[NUM_ANGLES];
extern float coslut[NUM_ANGLES];

extern ALLEGRO_SAMPLE *shoota;
extern ALLEGRO_SAMPLE *shootb;
extern ALLEGRO_SAMPLE *particle;
extern ALLEGRO_SAMPLE *dead;

int  UpdateShips(int num_ships);
void UpdateBullets(void);
void UpdateSentries(void);