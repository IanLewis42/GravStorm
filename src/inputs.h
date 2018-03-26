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
#define NA 4
#define TOUCH_JOYSTICK 5

//GPIO Pins for joystick
#define GPIO_LEFT  22/*18*/
#define GPIO_RIGHT 24/*26*/
#define GPIO_UP    18/*22*/
#define GPIO_DOWN  26/*24*/
#define GPIO_BUT1  16

#define RELEASED 0
#define HELD 1

typedef struct
{
	int left;
	int right;
	int up;
	int down;
	int button;

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

extern ALLEGRO_JOYSTICK *USBJOY[2];
extern bool key_down_log[ALLEGRO_KEY_MAX];
extern bool key_up_log[ALLEGRO_KEY_MAX];
extern JoystickType USBJoystick[2];
extern JoystickType TouchJoystick;
extern SelectType Select;  //for touch menu control. Couldn't think of a better name.....
extern int relx,rely;

void ScanInputs(int num_ships);
void ReadGPIOJoystick();
void CheckUSBJoyStick(ALLEGRO_EVENT event);
void CheckTouchControls(ALLEGRO_EVENT event);
void UpdateTouches(void);
void ValidateTouch(int i);

