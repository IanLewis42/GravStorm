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

ALLEGRO_JOYSTICK *USBJOY[2];

JoystickType GPIOJoystick;
JoystickType USBJoystick[2];

JoystickType TouchJoystick; //for android

bool key_down_log[ALLEGRO_KEY_MAX];
bool key_up_log[ALLEGRO_KEY_MAX];

void ScanInputs(int num_ships);
void ReadGPIOJoystick();
void CheckUSBJoyStick(ALLEGRO_EVENT event);
void NewTouch(ALLEGRO_EVENT event, int i);
void DoDPAD(ALLEGRO_EVENT event);
ButtonType FindButton(float x, float y);


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

	fprintf(logfile,"Read GPIO joystick\n");

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
void CheckUSBJoyStick(ALLEGRO_EVENT event)
{
	int JoyIdx = 0;

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
			}

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

}

//Android controls
//Read touches. Map into TouchJoystick, plus 'Command' struct
//drag dpad and thrust.

//
void CheckTouchControls(ALLEGRO_EVENT event)
{
	int i;

	if (event.type == ALLEGRO_EVENT_TOUCH_BEGIN)
	{
		//find free entry in touch array
		i = 0;
		while (Touch[i].id != NO_TOUCH) i++;

		if (i >= NUM_TOUCHES)   //protection for running out of array.....
            return;

		Touch[i].id = event.touch.id;
		Touch[i].button = FindButton(event.touch.x, event.touch.y);

		NewTouch(event,i);  //check x,y for dpad, poke structures
	}
	else if (event.type == ALLEGRO_EVENT_TOUCH_END)
    {
		for (i=0 ; Touch[i].id != event.touch.id ; i++) //find touch
        {}

        Touch[i].id = NO_TOUCH;             //reset array entry

        if (Touch[i].button == DPAD)        //release joystick
        {
            TouchJoystick.right_up = true;
            TouchJoystick.left_up = true;
            TouchJoystick.down_up = true;
            TouchJoystick.up_up = true;
        }
        else if (Touch[i].button == THRUST_BUTTON)
            TouchJoystick.button_up = true;
        //don't care about release of these?
        //else if (Touch[i].button == BACK)
        //    Command.goback = true;
        //else if (Touch[i].button == RADAR)
        //    Command.toggleradar = true;
        //else if (Touch[i].button == START || Touch[i].button == SELECT)
        //    Command.goforward = true;

        Touch[i].button = NO_BUTTON;        //probably not really needed....
    }
    else if (event.type == ALLEGRO_EVENT_TOUCH_MOVE)
    {
        for (i=0 ; Touch[i].id != event.touch.id ; i++) //find touch
        {}

        if (i >= NUM_TOUCHES)   //protection for running out of array.....
            return;

        ButtonType oldbutton = Touch[i].button;
        Touch[i].button = FindButton(event.touch.x, event.touch.y);

        if (oldbutton == DPAD)        //touch WAS on DPAD
        {
            if (Touch[i].button == DPAD)             //touch STILL on DPAD
            {
                DoDPAD(event);  //map touch event x/y to TouchJoystick struct.
            }
            else    //drag DPAD - hopefully don't need to limit, as event.touch.x/y shouldn't go off screen
            {
                if (event.touch.x < Ctrl.ctrl[DPAD].x)
                    Ctrl.ctrl[DPAD].x = event.touch.x;
                else if (event.touch.x > Ctrl.ctrl[DPAD].x + Ctrl.ctrl[DPAD].w)
                    Ctrl.ctrl[DPAD].x = event.touch.x - Ctrl.ctrl[DPAD].w;
                if (event.touch.y < Ctrl.ctrl[DPAD].y)
                    Ctrl.ctrl[DPAD].y = event.touch.y;
                else if (event.touch.y > Ctrl.ctrl[DPAD].y + Ctrl.ctrl[DPAD].h)
                    Ctrl.ctrl[DPAD].y = event.touch.y - Ctrl.ctrl[DPAD].h;
            }
        }
        else if (oldbutton == THRUST_BUTTON)        //touch was on thrust
        {
            if (Touch[i].button == THRUST_BUTTON)  //touch still on thrust
            {}                                  //nothing to do
            else //drag thrust button
            {
                if (event.touch.x < Ctrl.ctrl[THRUST_BUTTON].x)
                    Ctrl.ctrl[THRUST_BUTTON].x = event.touch.x;
                else if (event.touch.x > Ctrl.ctrl[THRUST_BUTTON].x + Ctrl.ctrl[THRUST_BUTTON].w)
                    Ctrl.ctrl[THRUST_BUTTON].x = event.touch.x - Ctrl.ctrl[THRUST_BUTTON].w;
                if (event.touch.y < Ctrl.ctrl[THRUST_BUTTON].y)
                    Ctrl.ctrl[THRUST_BUTTON].y = event.touch.y;
                else if (event.touch.y > Ctrl.ctrl[THRUST_BUTTON].y + Ctrl.ctrl[THRUST_BUTTON].h)
                    Ctrl.ctrl[THRUST_BUTTON].y = event.touch.y - Ctrl.ctrl[THRUST_BUTTON].h;
            }
        }

        else if (oldbutton == NO_BUTTON)            //were on no button
        {
            if (Touch[i].button != NO_BUTTON)       //now touching button
            {
                NewTouch(event,i);                  //treat as new touch
            }
        }
        //don't need any other cases, don't care about holding or release of other buttons.???
        return;
    }
}

void NewTouch(ALLEGRO_EVENT event, int i)
{
    if (Touch[i].button == DPAD)
    {
        DoDPAD(event);  //map touch event x/y to TouchJoystick struct.
    }
    else if (Touch[i].button == THRUST_BUTTON)
        TouchJoystick.button_down = true;
    else if (Touch[i].button == BACK)
        Command.goback = true;
    else if (Touch[i].button == RADAR)
        Command.toggleradar = true;
    else if (Touch[i].button == START || Touch[i].button == SELECT)
        Command.goforward = true;
    //bigger/smaller buttons here

    return;
}

void DoDPAD(ALLEGRO_EVENT event)
{
    int relx = event.touch.x - Ctrl.ctrl[DPAD].x;    //relative x
    int rely = event.touch.y - Ctrl.ctrl[DPAD].y;    //relative y

    //check left/right
    if (relx < Ctrl.ctrl[DPAD].w * (1/3))   //left hand side
    {
        TouchJoystick.left_down = true;
        TouchJoystick.right_up = true;
    }
    else if (relx > Ctrl.ctrl[DPAD].w * (2/3))   //right hand side
    {
        TouchJoystick.right_down = true;
        TouchJoystick.left_up = true;
    }
    else   //middle
    {
        TouchJoystick.right_up = true;
        TouchJoystick.left_up = true;
    }

    //check up/down
    if (rely < Ctrl.ctrl[DPAD].h * (1/3))   //top
    {
        TouchJoystick.up_down = true;
        TouchJoystick.down_up = true;
    }
    else if (rely > Ctrl.ctrl[DPAD].h * (2/3))   //bottom
    {
        TouchJoystick.down_down = true;
        TouchJoystick.up_up = true;
    }
    else                                    //middle
    {
        TouchJoystick.down_up = true;
        TouchJoystick.up_up = true;
    }
    return;
}

//better check this works.....
ButtonType FindButton(float x, float y)
{
    ButtonType i;

    for (i=0 ; i<NO_BUTTON ; i++)
    {
        if (Ctrl.ctrl[i].active)
            if ((x > Ctrl.ctrl[i].x) && (x < (Ctrl.ctrl[i].x + Ctrl.ctrl[i].w)))
                if ((y > Ctrl.ctrl[i].y) && (y < (Ctrl.ctrl[i].y + Ctrl.ctrl[i].h)))
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

		else if (Ship[j].controller == USB_JOYSTICK0 || Ship[j].controller == USB_JOYSTICK1 )
		{
			if (Ship[j].controller == USB_JOYSTICK0) joy_idx = 0;
			if (Ship[j].controller == USB_JOYSTICK1) joy_idx = 1;

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

	//Android - set display indexes.
	Ctrl.ctrl[DPAD].idx = 4;
	if (Ship[first_ship].left_held)  Ctrl.ctrl[DPAD].idx-=1;
	if (Ship[first_ship].right_held) Ctrl.ctrl[DPAD].idx+=1;
	if (Ship[first_ship].fire1_held) Ctrl.ctrl[DPAD].idx-=3;
	if (Ship[first_ship].fire2_held) Ctrl.ctrl[DPAD].idx+=3;

	if (Ship[first_ship].thrust_held) Ctrl.ctrl[THRUST_BUTTON].idx = 1;
	else Ctrl.ctrl[THRUST_BUTTON].idx = 0;

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
