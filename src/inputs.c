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

//#define ALLEGRO_UNSTABLE 1  //needed for haptics.

#include "allegro5/allegro.h"
#include "allegro5/allegro_image.h"
#include "allegro5/allegro_primitives.h"
#include "allegro5/allegro_font.h"
#include "allegro5/allegro_audio.h"
#include "allegro5/allegro_acodec.h"

#include "game.h"
#include "inputs.h"
#include "objects.h"
#include "network.h"
#include "auto.h"
#ifdef RPI
#include <wiringPi.h>
#endif

//ALLEGRO_JOYSTICK *USBJOY[4];

JoystickType GPIOJoystick;
JoystickType USBJoystick[4];
JoystickType TouchJoystick; //for android

int controllers [NUM_CONTROLLERS];

TouchType Touch[NUM_TOUCHES];

SelectType Select;  //for touch menu control. Couldn't think of a better name.....

bool key_down_log[ALLEGRO_KEY_MAX];
bool key_up_log[ALLEGRO_KEY_MAX];

void ScanInputs(int num_ships);
void ReadGPIOJoystick();
void NewTouch(float x, float y, int i);
void DoDPAD(float x, float y);
void DoAStick(float x, float y);
ButtonType FindButton(float x, float y);
void swapx(ButtonType i, ButtonType j);



/****************************************************
** void ReadGPIOJoystick()
** Read GPIO pins, create xx_up and xx_down events,
** similar to allegro key_up and key_down events.
****************************************************/
void ReadGPIOJoystick()
{
#if RPI
	static int joystick_left_state = RELEASED;
	static int joystick_right_state = RELEASED;
	static int joystick_up_state = RELEASED;
	static int joystick_down_state = RELEASED;
	static int joystick_button_state = RELEASED;

	al_fprintf(logfile,"Read GPIO joystick\n");

	//state machine to set/clear variables only once per stick push.
	//This is so that subsequent code can clear the variable to get a one-shot effect.
	//LEFT
	if (joystick_left_state == RELEASED)	//if stick was not held
	{
		if (!digitalRead(GPIO_LEFT))		//read it
		{
			GPIOJoystick.left_down = true;
			joystick_left_state = HELD;		//and remember that it was held
		}
	}
	else  //must have been HELD
	{
		if (!!digitalRead(GPIO_LEFT))		//Read it. If it's not held,
		{
			GPIOJoystick.left_up = true;
			joystick_left_state = RELEASED;	//flip state back
		}
	}
	//RIGHT
	if (joystick_right_state == RELEASED)	//if stick was not held
	{
		if (!digitalRead(GPIO_RIGHT))		//read it
		{
			GPIOJoystick.right_down = true;
			joystick_right_state = HELD;		//and remember that it was held
		}
	}
	else  //must have been HELD
	{
		if (!!digitalRead(GPIO_RIGHT))		//Read it. If it's not held,
		{
			GPIOJoystick.right_up = true;
			joystick_right_state = RELEASED;	//flip state back
		}
	}
	//UP
	if (joystick_up_state == RELEASED)	//if stick was not held
	{
		if (!digitalRead(GPIO_UP))		//read it
		{
			GPIOJoystick.up_down = true;
			joystick_up_state = HELD;		//and remember that it was held
		}
	}
	else  //must have been HELD
	{
		if (!!digitalRead(GPIO_UP))		//Read it. If it's not held,
		{
			GPIOJoystick.up_up = true;
			joystick_up_state = RELEASED;	//flip state back
		}
	}
	//DOWN
	if (joystick_down_state == RELEASED)	//if stick was not held
	{
		if (!digitalRead(GPIO_DOWN))		//read it
		{
			GPIOJoystick.down_down = true;
			joystick_down_state = HELD;		//and remember that it was held
		}
	}
	else  //must have been HELD
	{
		if (!!digitalRead(GPIO_DOWN))		//Read it. If it's not held,
		{
			GPIOJoystick.down_up = true;
			joystick_down_state = RELEASED;	//flip state back
		}
	}

	//BUTTON
	if (joystick_button_state == RELEASED)	//if stick was not held
	{
		if (!digitalRead(GPIO_BUT1))		//read it
		{
			GPIOJoystick.button_down = true;
			joystick_button_state = HELD;		//and remember that it was held
		}
	}
	else  //must have been HELD
	{
		if (!!digitalRead(GPIO_BUT1))		//Read it. If it's not held,
		{
			GPIOJoystick.button_up = true;
			joystick_button_state = RELEASED;	//flip state back
		}
	}

	//GPIOJoystick.button = !digitalRead(GPIO_BUT1);
#endif
}



//maps allegro events into JoystickType struct.
void CheckUSBJoyStick(ALLEGRO_EVENT event, bool menu)
{
	int i,start,JoyIdx = 0;
	float hysteresis = 0.2;
#if 1

    if (menu)
        start = 5;
    else
        start = 0;

    if (!(event.type == ALLEGRO_EVENT_JOYSTICK_AXIS ||
          event.type == ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN ||
          event.type == ALLEGRO_EVENT_JOYSTICK_BUTTON_UP))
        return;

    while(event.joystick.id != USBJoystick[JoyIdx].al_joy) JoyIdx++;    //find the right controller

    for (i=start ; i<start+5 ; i++)   //spin through mapping
    {
        if (event.type == ALLEGRO_EVENT_JOYSTICK_AXIS && USBJoystick[JoyIdx].Map[i].Type == STICK)  //stick, not button
        {
            if (event.joystick.stick == USBJoystick[JoyIdx].Map[i].StickIdx)            //does the stick in the event match the control map we're looking at?
            {
                if (event.joystick.axis == USBJoystick[JoyIdx].Map[i].AxisIdx)          //and the axis
                {
 /*
 if (pos*threshold>0)	//i.e. both +ve or both -ve
   if (_held)
     if (abs(pos)> abs(threshold + hysteresis)) //abs??
       _down = true
     else if (pos<threshold - hysteresis)
       _up = true
 */
                    //if((event.joystick.pos * USBJoystick[JoyIdx].Map[i].Threshold) > 0) //i.e. +ve value & +ve threshold or -ve value & -ve threshold
                    {
                        if (USBJoystick[JoyIdx].Map[i].Held)    //if the control is held, only check for release
                        {
                            if (fabsf(event.joystick.pos) < (fabsf(USBJoystick[JoyIdx].Map[i].Threshold) - hysteresis))
                            {
                                *USBJoystick[JoyIdx].Map[i].OffPtr = true;
                                USBJoystick[JoyIdx].Map[i].Held = false;
                            }
                        }
                        else
                        {
                            if((event.joystick.pos * USBJoystick[JoyIdx].Map[i].Threshold) > 0) //i.e. +ve value & +ve threshold or -ve value & -ve threshold
                            {
                                if (fabsf(event.joystick.pos) > (fabsf(USBJoystick[JoyIdx].Map[i].Threshold) + hysteresis))
                                {
                                    *USBJoystick[JoyIdx].Map[i].OnPtr = true;
                                    USBJoystick[JoyIdx].Map[i].Held = true;
                                }
                            }
                        }
                    }


                    /*
                    if (USBJoystick[JoyIdx].Map[i].Threshold < 0)                       //is the threshold negative?
                    {
                        if (event.joystick.pos < USBJoystick[JoyIdx].Map[i].Threshold-hysteresis)  //yes, so check for < threshold
                        {
                            *USBJoystick[JoyIdx].Map[i].OnPtr  = true;                    //assert
                            // *USBJoystick[JoyIdx].Map[i].OffPtr = false;
                        }
                        else if (event.joystick.pos > USBJoystick[JoyIdx].Map[i].Threshold+hysteresis)
                        {
                            // *USBJoystick[JoyIdx].Map[i].OnPtr  = false;                   //deassert
                            *USBJoystick[JoyIdx].Map[i].OffPtr = true;
                        }
                    }
                    else
                    {
                        if (event.joystick.pos > USBJoystick[JoyIdx].Map[i].Threshold+hysteresis)  //no, so check for > threshold
                        {
                            *USBJoystick[JoyIdx].Map[i].OnPtr  = true;                    //assert
                            // *USBJoystick[JoyIdx].Map[i].OffPtr = false;
                        }
                        else if (event.joystick.pos < USBJoystick[JoyIdx].Map[i].Threshold-hysteresis)
                        {
                            // *USBJoystick[JoyIdx].Map[i].OnPtr  = false;                   //deassert
                            *USBJoystick[JoyIdx].Map[i].OffPtr = true;
                        }
                    }
                    */
                }
            }
        }
        else if (event.type == ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN && USBJoystick[JoyIdx].Map[i].Type == BUTTON)  //stick, not button
        {
            if (event.joystick.button == USBJoystick[JoyIdx].Map[i].ButIdx)  //does the button match
                *USBJoystick[JoyIdx].Map[i].OnPtr  = true;                    //assert
        }
        else if (event.type == ALLEGRO_EVENT_JOYSTICK_BUTTON_UP && USBJoystick[JoyIdx].Map[i].Type == BUTTON)  //stick, not button
        {
            if (event.joystick.button == USBJoystick[JoyIdx].Map[i].ButIdx)  //does the button match
                *USBJoystick[JoyIdx].Map[i].OffPtr  = true;                    //assert
        }
    }


        //event.type == ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN ||
        //event.type == ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN


#else
	if (event.type == ALLEGRO_EVENT_JOYSTICK_AXIS)
	{

		if (event.joystick.id == USBJOY[0])
			JoyIdx = 0;
		else if (event.joystick.id == USBJOY[1])
			JoyIdx = 1;


		{
			if (event.joystick.axis == 0)
			{
				if (event.joystick.pos < -0.5)
				{
					USBJoystick[JoyIdx].left_down = true;
					//USBJoystick[JoyIdx].right_down = false;
					USBJoystick[JoyIdx].right_up = true;
				}
				else if (event.joystick.pos > 0.5)
				{
					//USBJoystick[JoyIdx].left_down = false;
					USBJoystick[JoyIdx].left_up = true;
					USBJoystick[JoyIdx].right_down = true;
				}
				else
				{
					//USBJoystick[JoyIdx].left_down = false;
					//USBJoystick[JoyIdx].right_down = false;
					USBJoystick[JoyIdx].left_up = true;
					USBJoystick[JoyIdx].right_up = true;
				}
                //TouchJoystick.spin = 2*(event.joystick.pos);
                TouchJoystick.y = event.joystick.pos;
            }
			else if (event.joystick.axis == 1)
			{
				if (event.joystick.pos < -0.5)
				{
					USBJoystick[JoyIdx].up_down = true;
					//USBJoystick[JoyIdx].down_down = false;
					USBJoystick[JoyIdx].down_up = true;
				}
				else if (event.joystick.pos > 0.5)
				{
					//USBJoystick[JoyIdx].up_down = false;
					USBJoystick[JoyIdx].up_up = true;
					USBJoystick[JoyIdx].down_down = true;
				}
				else
				{
					//USBJoystick[JoyIdx].up_down = false;
					//USBJoystick[JoyIdx].down_down = false;
					USBJoystick[JoyIdx].up_up = true;
					USBJoystick[JoyIdx].down_up = true;
				}
                if (fabs(event.joystick.pos) > 0.1)
                    TouchJoystick.spin = -3*(event.joystick.pos);
                else
                    TouchJoystick.spin = 0;
                TouchJoystick.x = event.joystick.pos;

			}
            //Ship[0].fangle = atan2(TouchJoystick.x,TouchJoystick.y);
            //Ship[0].fangle = (Ship[0].fangle * 180)/PI;

		}
		//USBJOY[1] here
	}

	if (event.type ==  ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN)
	{
		//if (event.joystick.id == USBJOY[0])
		if (event.joystick.id == USBJOY[0])
			JoyIdx = 0;
		else if (event.joystick.id == USBJOY[1])
			JoyIdx = 1;
		if (event.joystick.button == 0 || event.joystick.button == 1)
		{
			USBJoystick[JoyIdx].button_down = true;
		}
	}

	if (event.type ==  ALLEGRO_EVENT_JOYSTICK_BUTTON_UP)
	{
		//if (event.joystick.id == USBJOY[0])
		if (event.joystick.id == USBJOY[0])
			JoyIdx = 0;
		else if (event.joystick.id == USBJOY[1])
			JoyIdx = 1;

		if (event.joystick.button == 0 || event.joystick.button == 1)
		{
			USBJoystick[JoyIdx].button_up = true;
		}
	}
#endif
}

//Android controls
//Read touches. Map into TouchJoystick, plus 'Command' struct
//drag dpad and thrust.

//

void CheckTouchControls(ALLEGRO_EVENT event)
{
	int i;
    //int w,h;

	if (event.type == ALLEGRO_EVENT_TOUCH_BEGIN) {
		//find free entry in touch array
		i = 0;
		while (Touch[i].id != NO_TOUCH && Touch[i].id != event.touch.id) {
            i++;
            if (i >= NUM_TOUCHES - 1)   //protection for running out of array.....
                return;
        }

		Touch[i].id = event.touch.id;
		Touch[i].count = 1;                 //start debounce
        Touch[i].x = event.touch.x;
        Touch[i].y = event.touch.y;
	}

	else if (event.type == ALLEGRO_EVENT_TOUCH_END)
	{
		for (i = 0; Touch[i].id != event.touch.id; i++) //find touch
		{}

		Touch[i].id = NO_TOUCH;             //reset array entry
		Touch[i].count = 0;
        Touch[i].x = 0;
        Touch[i].y = 0;
        Touch[i].valid = 0;

		switch (Touch[i].button) {
/*			case DPAD:        //release joystick
				TouchJoystick.right_up = true;
				TouchJoystick.left_up = true;
				TouchJoystick.down_up = true;
				TouchJoystick.up_up = true;
				TouchJoystick.spin = 0;
				break;*/
			case THRUST_BUTTON:
            case SELECT:
				TouchJoystick.button_up = true;
                Ctrl.ctrl[THRUST_BUTTON].idx = 0;
                Ctrl.ctrl[SELECT].idx = 0;
				break;
			case FIRE1:
				TouchJoystick.up_up = TRUE;
				Ctrl.ctrl[FIRE1].idx = 0;
				break;
			case FIRE2:
				TouchJoystick.down_up = TRUE;
				Ctrl.ctrl[FIRE2].idx = 0;
				break;

            case CW:
                TouchJoystick.right_up = true;
                Ctrl.ctrl[CW].idx = 0;
                break;
            case ACW:
                TouchJoystick.left_up = true;
                Ctrl.ctrl[ACW].idx = 0;
                break;

			case BACK:
				Ctrl.ctrl[BACK].idx = 0;
				break;
			case RADAR:
				Ctrl.ctrl[RADAR].idx = 0;
				break;
			case BIGGER:
				Ctrl.ctrl[BIGGER].idx = 0;
				break;
			case SMALLER:
				Ctrl.ctrl[SMALLER].idx = 0;
				break;
            case NO_BUTTON:
                Select.x = event.touch.x;
                Select.y = event.touch.y;
                Select.action = RELEASE;
                break;
            case ASTICK:
            case ASTICK2:
            default:
            break;
		}
		Touch[i].button = NO_BUTTON;        //probably not really needed....
	}
	else if (event.type == ALLEGRO_EVENT_TOUCH_MOVE)
	{
		for (i=0 ; Touch[i].id != event.touch.id ; i++) //find touch
		{}

		if (i >= NUM_TOUCHES)   //protection for running out of array.....
			return;

        Touch[i].x = event.touch.x;
        Touch[i].y = event.touch.y;

        //if (Touch[i].valid == 0)
        //    return;

        ButtonType oldbutton = Touch[i].button;
		Touch[i].button = FindButton(event.touch.x, event.touch.y);

        switch (oldbutton) {
/*            case DPAD: {
                if (Touch[i].button == DPAD)             //touch STILL on DPAD
                {
                    DoDPAD(Touch[i].x, Touch[i].y);  //map touch event x/y to TouchJoystick struct.
                } else    //drag DPAD - hopefully don't need to limit, as event.touch.x/y shouldn't go off screen
                {
                    if (event.touch.x < Ctrl.ctrl[DPAD].x) {
                        Ctrl.ctrl[DPAD].x = event.touch.x;
                        Ctrl.ctrl[ASTICK].x = event.touch.x;
                    } else if (event.touch.x > Ctrl.ctrl[DPAD].x + Ctrl.ctrl[DPAD].size) {
                        Ctrl.ctrl[DPAD].x = event.touch.x - Ctrl.ctrl[DPAD].size;
                        Ctrl.ctrl[ASTICK].x = event.touch.x - Ctrl.ctrl[DPAD].size;
                    }
                    if (event.touch.y < Ctrl.ctrl[DPAD].y) {
                        Ctrl.ctrl[DPAD].y = event.touch.y;
                        Ctrl.ctrl[ASTICK].y = event.touch.y;
                    } else if (event.touch.y > Ctrl.ctrl[DPAD].y + Ctrl.ctrl[DPAD].size) {
                        Ctrl.ctrl[DPAD].y = event.touch.y - Ctrl.ctrl[DPAD].size;
                        Ctrl.ctrl[ASTICK].y = event.touch.y - Ctrl.ctrl[DPAD].size;
                    }
                    Touch[i].button = DPAD;
                }
            }
            break;*/
            case ASTICK: {
                //don't drag - but keep updating.
                Touch[i].button = ASTICK;
                DoAStick(Touch[i].x, Touch[i].y);  //map touch event x/y to TouchJoystick struct.
            }
            break;
            case THRUST_BUTTON: {
                if (Touch[i].button == THRUST_BUTTON)  //touch still on thrust
                {}                                  //nothing to do

                else //drag thrust button - NO!
                {
                    TouchJoystick.button_up = true;
                    Ctrl.ctrl[THRUST_BUTTON].idx = 0;
                }
            }
            break;
            case CW:
                if (Touch[i].button != CW)  //touch moved off button
                {
                    Touch[i].button = NO_BUTTON;
                    TouchJoystick.right_up = true;  //so cancel
                    Ctrl.ctrl[CW].idx = 0;
                }
            break;
            case ACW:
                if (Touch[i].button != ACW)  //touch moved off button
                {
                    Touch[i].button = NO_BUTTON;
                    TouchJoystick.left_up = true; //so cancel
                    Ctrl.ctrl[ACW].idx = 0;
                }
            break;
            case FIRE1:
                if (Touch[i].button != FIRE1) {
                    TouchJoystick.up_up = true;
                    Ctrl.ctrl[FIRE1].idx = 0;
                }
            break;
            case FIRE2:
                if (Touch[i].button != FIRE2) {
                    TouchJoystick.down_up = true;
                    Ctrl.ctrl[FIRE2].idx = 0;
                }
            break;
            case NO_BUTTON: {
                if (Touch[i].button != NO_BUTTON)       //now touching button
                {
                    NewTouch(Touch[i].x, Touch[i].y, i);                  //treat as new touch
                }
                else
                {
                    //w = al_get_display_width(display);
                    //h = al_get_display_height(display);
                    Select.sumdx += event.touch.dx;
                    if (Select.sumdx > Select.sumdxmax) Select.sumdx = Select.sumdxmax;
                    if (Select.sumdx < Select.sumdxmin) Select.sumdx = Select.sumdxmin;
                    Select.sumdy += event.touch.dy;
                    if (Select.sumdy > Select.sumdymax) Select.sumdy = Select.sumdymax;
                    if (Select.sumdy < Select.sumdymin) Select.sumdy = Select.sumdymin;
                    Select.x = event.touch.x;
                    Select.y = event.touch.y;
                    Select.action = MOVE;
                }
            }
            break;
            //other buttons, just un-press (only affects drawing)
            default :
               if (Touch[i].button != oldbutton)
                Ctrl.ctrl[oldbutton].idx = 0;       //unpress
        }
		return;
	}
}

//debounce
void UpdateTouches(void)
{
    int i;
    for (i=0 ; i<NUM_TOUCHES ; i++)
    {
        if (Touch[i].count > 0)
            if (Touch[i].count < TOUCH_THRESHOLD)
                Touch[i].count++;
        if (Touch[i].count == TOUCH_THRESHOLD)
        {
            Touch[i].count = 0;
            ValidateTouch(i);
        }
    }
    return;
}

void ValidateTouch(int i) {
	Touch[i].button = FindButton(Touch[i].x, Touch[i].y);
    Touch[i].valid = 1;
	NewTouch(Touch[i].x, Touch[i].y, i);  //check x,y for dpad, poke structures
	return;
}
void flip(ButtonType i );
int relx,rely;

void NewTouch(float x, float y, int i)
{
    int j;//,w,h;

    switch (Touch[i].button) {
/*		case DPAD:
			DoDPAD(x, y);  //map touch event x/y to TouchJoystick struct.
			Touch[i].valid = 0;
			Touch[i].count = 0;
			break;*/
		case ASTICK:
			DoAStick(x, y);
			break;
		case THRUST_BUTTON:
            TouchJoystick.button_down = true;
            Ctrl.ctrl[THRUST_BUTTON].idx = 1;
            Touch[i].valid = 0;
            Touch[i].count = 0;
            break;

        case CW:
            TouchJoystick.right_down = true;
            Ctrl.ctrl[CW].idx = 1;
            Touch[i].valid = 0;
            Touch[i].count = 0;
            break;
        case ACW:
            TouchJoystick.left_down = true;
            Ctrl.ctrl[ACW].idx = 1;
            Touch[i].valid = 0;
            Touch[i].count = 0;
            break;

        case SELECT:
            Command.goforward = true;
			Ctrl.ctrl[SELECT].idx = 1;
			Touch[i].valid = 0;
			Touch[i].count = 0;
			break;
		case BACK:
			Command.goback = true;
			Ctrl.ctrl[BACK].idx = 1;
			Touch[i].valid = 0;
			Touch[i].count = 0;
			break;
		case RADAR:
			Command.toggleradar = true;
			Ctrl.ctrl[RADAR].idx = 1;
			Touch[i].valid = 0;
			Touch[i].count = 0;
			break;
		case FIRE1:
			TouchJoystick.up_down = TRUE;
			Ctrl.ctrl[FIRE1].idx = 1;
			Touch[i].valid = 0;
			Touch[i].count = 0;
			break;
		case FIRE2:
			TouchJoystick.down_down = TRUE;
			Ctrl.ctrl[FIRE2].idx = 1;
			Touch[i].valid = 0;
			Touch[i].count = 0;
			break;
			//break;
		case BIGGER:
			Ctrl.ctrl[BIGGER].idx = 1;
			for (j = 0; j < NO_BUTTON; j++)
			{
				Ctrl.ctrl[j].size += 10;
				Ctrl.ctrl[j].x -= Ctrl.ctrl[j].movex;
				Ctrl.ctrl[j].y -= Ctrl.ctrl[j].movey;
			}
            Touch[i].valid = 0;
            Touch[i].count = 0;
        break;
        case SMALLER:
			Ctrl.ctrl[SMALLER].idx = 1;
            for (j=0 ; j<NO_BUTTON ; j++)
			{
				Ctrl.ctrl[j].size -= 10;
				Ctrl.ctrl[j].x += Ctrl.ctrl[j].movex;
				Ctrl.ctrl[j].y += Ctrl.ctrl[j].movey;
			}
            Touch[i].valid = 0;
            Touch[i].count = 0;
            break;
        case REVERSE:
            //flip(DPAD);
            //flip(SELECT);
            Ctrl.mode++;
            Ctrl.mode &= 0x0003;
            GameControls();
            flip(ASTICK);
            flip(ASTICK2);
            flip(THRUST_BUTTON);
            flip(FIRE1);
            flip(FIRE2);
            flip(CW);
            flip(ACW);
            swapx(CW,ACW);
        break;
        case NO_BUTTON:
            //w = al_get_display_width(display);
            //h = al_get_display_height(display);
            //Select.sumdx = 0;
            //Select.sumdy = 0;
            Select.x = x;
            Select.y = y;
            Select.action = TOUCH;
            break;
        case ASTICK2:
        default:
        break;
    }
    return;
}

void flip(ButtonType i )
{
    int w = al_get_display_width(display);

    Ctrl.ctrl[i].x = w-(Ctrl.ctrl[i].x + Ctrl.ctrl[i].size);    //flip position

    Ctrl.ctrl[i].movex = -1 * (Ctrl.ctrl[i].movex - 10);        //change move tweak for resizing.

    return;
}

void swapx(ButtonType i, ButtonType j)
{
    int temp;
    temp = Ctrl.ctrl[i].x;
    Ctrl.ctrl[i].x = Ctrl.ctrl[j].x;
    Ctrl.ctrl[j].x = temp;
}
/*
void DoDPAD(float x, float y)
{
    relx = x - Ctrl.ctrl[DPAD].x;    //relative x               //relative to top-left corner
    rely = y - Ctrl.ctrl[DPAD].y;    //relative y

    //TouchJoystick.spin = 4*((float)relx/Ctrl.ctrl[DPAD].size-0.5);

	//int lthresh = Ctrl.ctrl[DPAD].size/3;

    //check left/right
    if (relx < (Ctrl.ctrl[DPAD].size/3))   //left hand side
    {
        TouchJoystick.left_down = true;
        TouchJoystick.right_up = true;
    }
    else if (relx > (2*Ctrl.ctrl[DPAD].size/3))   //right hand side
    {
        TouchJoystick.right_down = true;
        TouchJoystick.left_up = true;
    }
    else   //middle
    {
        TouchJoystick.right_down = false;
        TouchJoystick.right_up = true;
        TouchJoystick.left_down = false;
        TouchJoystick.left_up = true;
    }

    //check up/down
    if (rely < (Ctrl.ctrl[DPAD].size/3))   //top
    {
        TouchJoystick.up_down = true;
        TouchJoystick.down_up = true;
    }
    else if (rely > (2*Ctrl.ctrl[DPAD].size/3))   //bottom
    {
        TouchJoystick.down_down = true;
        TouchJoystick.up_up = true;
    }
    else                                    //middle
    {
        TouchJoystick.down_down = false;
        TouchJoystick.down_up = true;
        TouchJoystick.up_down = false;
        TouchJoystick.up_up = true;
    }
    return;
}
*/
void DoAStick(float x, float y)
{
    int centre_x = (Ctrl.ctrl[ASTICK].x + Ctrl.ctrl[ASTICK].size/2);
    int centre_y = (Ctrl.ctrl[ASTICK].y + Ctrl.ctrl[ASTICK].size/2);

    relx = x - centre_x;    //relative to centre
    rely =y - centre_y;

    Ship[Net.id].fangle = atan2(relx,-1*rely);                            //find angle

    Ctrl.ctrl[ASTICK2].x = centre_x + 0.4*Ctrl.ctrl[ASTICK].size*sin(Ship[Net.id].fangle);//event.touch.x;  //plot
    Ctrl.ctrl[ASTICK2].x -= Ctrl.ctrl[ASTICK2].size/2;
    Ctrl.ctrl[ASTICK2].y = centre_y - 0.4*Ctrl.ctrl[ASTICK].size*cos(Ship[Net.id].fangle);//event.touch.y;
    Ctrl.ctrl[ASTICK2].y -= Ctrl.ctrl[ASTICK2].size/2;

    Ship[Net.id].fangle = Ship[Net.id].fangle*180/PI;                                 //to degrees
    if (Ship[Net.id].fangle < 0) Ship[Net.id].fangle += 360;                          //convert +/- 180 to 0-360

    return;
}

//better check this works.....
ButtonType FindButton(float x, float y)
{
    ButtonType i;

    for (i=0 ; i<NO_BUTTON ; i++)
    {
        if (Ctrl.ctrl[i].active)
            if ((x > (Ctrl.ctrl[i].x - Ctrl.ctrl[i].border)) && (x < (Ctrl.ctrl[i].x + Ctrl.ctrl[i].size + Ctrl.ctrl[i].border)))
                if ((y > (Ctrl.ctrl[i].y - Ctrl.ctrl[i].border)) && (y < (Ctrl.ctrl[i].y + Ctrl.ctrl[i].size + Ctrl.ctrl[i].border)))
                    break;
    }

    return i;
}

/****************************************************
** void ScanInputs(int num_ships)
** Check event logs for keys and joysticks,
** Map events into Ship xxx_down and xxx_held variables.
****************************************************/
void ScanInputs(int num_ships)
{

	int i, j, first_ship, joy_idx;
	//static int joystick_down_state = RELEASED;

	if (Net.client)
    {
        first_ship = Net.id;
        num_ships = first_ship+1;
    }
    else
        first_ship = 0;

	for (i=first_ship ; i<num_ships ; i++)
	{
		if (Net.client)
            j=0;    //take inputs from ship 0
        else
            j=i;

        if (Ship[i].automode != MANUAL)
            AutoShip(i);
        else
        {
            if (Ship[j].controller == KEYS)
            {
                //Up key, fire1
                if (key_down_log[Ship[j].up_key])
                {
                    Ship[i].fire1_down = true;
                    Ship[i].fire1_held = true;
                    key_down_log[Ship[j].up_key] = false;
                }
                if (key_up_log[Ship[j].up_key])
                {
                    Ship[i].fire1_held = false;
                    key_up_log[Ship[j].up_key] = false;
                }

                //Down key, fire2
                if (key_down_log[Ship[j].down_key])
                {
                    Ship[i].fire2_down = true;
                    Ship[i].fire2_held = true;
                    key_down_log[Ship[j].down_key] = false;

                }
                if (key_up_log[Ship[j].down_key])
                {
                    Ship[i].fire2_held = false;
                    key_up_log[Ship[j].down_key] = false;
                }

                //Left key, left
                if (key_down_log[Ship[j].left_key])
                {
                    Ship[i].left_down = true;
                    Ship[i].left_held = true;
                    key_down_log[Ship[j].left_key] = false;

                }
                if (key_up_log[Ship[j].left_key])
                {
                    Ship[i].left_held = false;
                    key_up_log[Ship[j].left_key] = false;
                }

                //Right key, right
                if (key_down_log[Ship[j].right_key])
                {
                    Ship[i].right_down = true;
                    Ship[i].right_held = true;
                    key_down_log[Ship[j].right_key] = false;
                }
                if (key_up_log[Ship[j].right_key])
                {
                    Ship[i].right_held = false;
                    key_up_log[Ship[j].right_key] = false;
                }

                //Thrust key, thrust
                if (key_down_log[Ship[j].thrust_key])
                {
                    Ship[i].thrust_down = true;
                    Ship[i].thrust_held = true;
                    key_down_log[Ship[j].thrust_key] = false;
                }
                if (key_up_log[Ship[j].thrust_key])
                {
                    Ship[i].thrust_down = false;
                    Ship[i].thrust_held = false;
                    key_up_log[Ship[j].thrust_key] = false;
                }
            }

            else if (Ship[j].controller == GPIO_JOYSTICK)
            {
                if (GPIOJoystick.left_down)
                {
                    Ship[i].left_down   = true;
                    Ship[i].left_held   = true;
                    GPIOJoystick.left_down = false;
                }
                if (GPIOJoystick.left_up)
                {
                    Ship[i].left_held   = false;
                    GPIOJoystick.left_up = false;
                }

                if (GPIOJoystick.right_down)
                {
                    Ship[i].right_down   = true;
                    Ship[i].right_held   = true;
                    GPIOJoystick.right_down = false;
                }
                if (GPIOJoystick.right_up)
                {
                    Ship[i].right_held   = false;
                    GPIOJoystick.right_up = false;
                }

                if (GPIOJoystick.up_down)
                {
                    Ship[i].fire1_down   = true;
                    Ship[i].fire1_held   = true;
                    GPIOJoystick.up_down = false;
                }
                if (GPIOJoystick.up_up)
                {
                    Ship[i].fire1_held   = false;
                    GPIOJoystick.up_up = false;
                }

                if (GPIOJoystick.down_down)
                {
                    Ship[i].fire2_down   = true;
                    Ship[i].fire2_held   = true;
                    GPIOJoystick.down_down = false;
                }
                if (GPIOJoystick.down_up)
                {
                    Ship[i].fire2_held   = false;
                    GPIOJoystick.down_up = false;
                }

                if (GPIOJoystick.button_down)
                {
                    Ship[i].thrust_down   = true;
                    Ship[i].thrust_held   = true;
                    GPIOJoystick.button_down = false;
                }
                if (GPIOJoystick.button_up)
                {
                    Ship[i].thrust_down   = false;
                    Ship[i].thrust_held   = false;
                    GPIOJoystick.button_up = false;
                }
                //Ship[i].thrust_held = GPIOJoystick.button;
            }

            else if (Ship[j].controller >= USB_JOYSTICK0 && Ship[j].controller <= USB_JOYSTICK3)
            {
                //if (Ship[j].controller == USB_JOYSTICK0) joy_idx = 0;
                //if (Ship[j].controller == USB_JOYSTICK1) joy_idx = 1;

                joy_idx = Ship[j].controller - USB_JOYSTICK0;

                if (USBJoystick[joy_idx].left_down)
                {
                    Ship[i].left_down   = true;
                    Ship[i].left_held   = true;
                    USBJoystick[joy_idx].left_down = false;
                }
                if (USBJoystick[joy_idx].left_up)
                {
                    Ship[i].left_held   = false;
                    USBJoystick[joy_idx].left_up = false;
                }

                if (USBJoystick[joy_idx].right_down)
                {
                    Ship[i].right_down   = true;
                    Ship[i].right_held   = true;
                    USBJoystick[joy_idx].right_down = false;
                }
                if (USBJoystick[joy_idx].right_up)
                {
                    Ship[i].right_held   = false;
                    USBJoystick[joy_idx].right_up = false;
                }

                if (USBJoystick[joy_idx].up_down)
                {
                    Ship[i].fire1_down   = true;
                    Ship[i].fire1_held   = true;
                    USBJoystick[joy_idx].up_down = false;
                }
                if (USBJoystick[joy_idx].up_up)
                {
                    Ship[i].fire1_held   = false;
                    USBJoystick[joy_idx].up_up = false;
                }

                if (USBJoystick[joy_idx].down_down)
                {
                    Ship[i].fire2_down   = true;
                    Ship[i].fire2_held   = true;
                    USBJoystick[joy_idx].down_down = false;
                }
                if (USBJoystick[joy_idx].down_up)
                {
                    Ship[i].fire2_held   = false;
                    USBJoystick[joy_idx].down_up = false;
                }

                if (USBJoystick[joy_idx].button_down)
                {
                    Ship[i].thrust_down   = true;
                    Ship[i].thrust_held   = true;
                    USBJoystick[joy_idx].button_down = false;
                }
                if (USBJoystick[joy_idx].button_up)
                {
                    Ship[i].thrust_down   = false;
                    Ship[i].thrust_held   = false;
                    USBJoystick[joy_idx].button_up = false;
                }
                //Ship[i].thrust_held = USBJoystick[0].button;
            }

            else if (Ship[j].controller == TOUCH_JOYSTICK)
            {
                if (TouchJoystick.left_down)
                {
                    Ship[i].left_down   = true;
                    Ship[i].left_held   = true;
                    TouchJoystick.left_down = false;
                }
                if (TouchJoystick.left_up)
                {
                    Ship[i].left_held   = false;
                    TouchJoystick.left_up = false;
                }

                if (TouchJoystick.right_down)
                {
                    Ship[i].right_down   = true;
                    Ship[i].right_held   = true;
                    TouchJoystick.right_down = false;
                }
                if (TouchJoystick.right_up)
                {
                    Ship[i].right_held   = false;
                    TouchJoystick.right_up = false;
                }

                if (TouchJoystick.up_down)
                {
                    Ship[i].fire1_down   = true;
                    Ship[i].fire1_held   = true;
                    TouchJoystick.up_down = false;
                }
                if (TouchJoystick.up_up)
                {
                    Ship[i].fire1_held   = false;
                    TouchJoystick.up_up = false;
                }

                if (TouchJoystick.down_down)
                {
                    Ship[i].fire2_down   = true;
                    Ship[i].fire2_held   = true;
                    TouchJoystick.down_down = false;
                }
                if (TouchJoystick.down_up)
                {
                    Ship[i].fire2_held   = false;
                    TouchJoystick.down_up = false;
                }

                if (TouchJoystick.button_down)
                {
                    Ship[i].thrust_down   = true;
                    Ship[i].thrust_held   = true;
                    TouchJoystick.button_down = false;
                }
                if (TouchJoystick.button_up)
                {
                    Ship[i].thrust_down   = false;
                    Ship[i].thrust_held   = false;
                    TouchJoystick.button_up = false;
                }
                //Ship[i].thrust_held = USBJoystick[0].button;
            }

            if (Ship[i].thrust_down)
            {
                Ship[i].thrust_down = false;
                Command.goforward = true;
            }
        }
	}
/*
	//Android - set display indexes.
	Ctrl.ctrl[DPAD].idx = 4;
	if (Ship[first_ship].left_held)  Ctrl.ctrl[DPAD].idx-=1;
	if (Ship[first_ship].right_held) Ctrl.ctrl[DPAD].idx+=1;
	if (Ship[first_ship].fire1_held) Ctrl.ctrl[DPAD].idx-=3;
	if (Ship[first_ship].fire2_held) Ctrl.ctrl[DPAD].idx+=3;

	if (Ship[first_ship].thrust_held)
	{
		Ctrl.ctrl[THRUST_BUTTON].idx = 1;
		Ctrl.ctrl[SELECT].idx = 1;
	}
	else
	{
		Ctrl.ctrl[THRUST_BUTTON].idx = 0;
		Ctrl.ctrl[SELECT].idx = 0;
	}
*/
	//Ctrl.select.idx = Ctrl.thrust.idx;
	//Ctrl.start.idx  = Ctrl.thrust.idx;

	/*
	if (Net.client) //copy inputs from Ship[0] to Ship[Net.id]
    {
        Ship[Net.id].fire1_down  = Ship[0].fire1_down;
        Ship[Net.id].fire1_held  = Ship[0].fire1_held;
        Ship[Net.id].fire2_down  = Ship[0].fire2_down;
        Ship[Net.id].fire2_held  = Ship[0].fire2_held;
        Ship[Net.id].left_down   = Ship[0].left_down;
        Ship[Net.id].left_held   = Ship[0].left_held;
        Ship[Net.id].right_down  = Ship[0].right_down;
        Ship[Net.id].right_held  = Ship[0].right_held;
        Ship[Net.id].thrust_down = Ship[0].thrust_down;
        Ship[Net.id].thrust_held = Ship[0].thrust_held;

        //Ship[0].fire1_down  = false;
        //Ship[0].fire1_held  = false;
        //Ship[0].fire2_down  = false;
        //Ship[0].fire2_held  = false;
        //Ship[0].left_down   = false;
        //Ship[0].left_held   = false;
        //Ship[0].right_down  = false;
        //Ship[0].right_held  = false;
        //Ship[0].thrust_down = false;
        //Ship[0].thrust_held = false;
    }
    */
}
