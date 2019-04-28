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

#define KEYS  0
#define GPIO_JOYSTICK 1
#define USB_JOYSTICK0 2
#define USB_JOYSTICK1 3
#define USB_JOYSTICK2 4
#define USB_JOYSTICK3 5
#define TOUCH_JOYSTICK 6
#define NUM_CONTROLLERS 7

//GPIO Pins for joystick
#define GPIO_LEFT  22/*18*/
#define GPIO_RIGHT 24/*26*/
#define GPIO_UP    18/*22*/
#define GPIO_DOWN  26/*24*/
#define GPIO_BUT1  16

#define RELEASED 0
#define HELD 1

typedef enum
{
    BUTTON = 0,
    STICK,
}ControlType;

typedef struct
{
    ControlType Type;
    int ButIdx;
    int StickIdx;
    int AxisIdx;
    float Threshold;
    int* OnPtr;
    int* OffPtr;
    int Held;

}ControlMapType;

typedef struct
{
	//int left;
	//int right;
	//int up;
	//int down;
	//int button;

	ALLEGRO_JOYSTICK *al_joy;

	ControlMapType Map[5*2];    //5 each for menu & game

	int left_down;	//down events
	int right_down;
	int up_down;
	int down_down;
	int button_down;

	int left_up;	//up events
	int right_up;
	int up_up;
	int down_up;
	int button_up;

	float spin;		//used for variable rotation on android
    float x;
    float y;
} JoystickType;

typedef enum
{
    NO_ACTION=0,
    TOUCH,
    MOVE,
    RELEASE
}SelectActionType;

typedef struct
{
    float x;
    float y;
    float dx;
    float dy;
    float sumdx;
    float sumdy;
    float sumdxmax;
    float sumdxmin;
    float sumdymax;
    float sumdymin;
    int line;
    SelectActionType action;
}SelectType;

//extern ALLEGRO_JOYSTICK *USBJOY[4];
extern bool key_down_log[ALLEGRO_KEY_MAX];
extern bool key_up_log[ALLEGRO_KEY_MAX];
extern JoystickType USBJoystick[4];
extern JoystickType TouchJoystick;
extern SelectType Select;  //for touch menu control. Couldn't think of a better name.....
extern int relx,rely;
extern int controllers [NUM_CONTROLLERS];

void ScanInputs(int num_ships);
void ReadGPIOJoystick();
void CheckUSBJoyStick(ALLEGRO_EVENT event, bool menu);
void CheckTouchControls(ALLEGRO_EVENT event);
void UpdateTouches(void);
void ValidateTouch(int i);

