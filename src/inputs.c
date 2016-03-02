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

ALLEGRO_JOYSTICK *USBJOY[2];
//GPIOJoystickType GPIOJoystick;

JoystickType GPIOJoystick;
JoystickType USBJoystick[2];


bool key_down_log[ALLEGRO_KEY_MAX];
bool key_up_log[ALLEGRO_KEY_MAX];

void ScanInputs(int num_ships);
void ReadGPIOJoystick();
void CheckUSBJoyStick(ALLEGRO_EVENT event);


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

void CheckUSBJoyStick(ALLEGRO_EVENT event)
{
	//return;

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

/****************************************************
** void ScanInputs(int num_ships)
** Check event logs for keys and joysticks,
** Map events into Ship xxx_down and xxx_held variables.
****************************************************/
void ScanInputs(int num_ships)
{

	int i, joy_idx;
	//static int joystick_down_state = RELEASED;

	for (i=0 ; i<num_ships ; i++)
	{
		if (Ship[i].controller == KEYS)
		{
			//Up key, fire1
			if (key_down_log[Ship[i].up_key])
			{
				Ship[i].fire1_down = true;
				Ship[i].fire1_held = true;
				key_down_log[Ship[i].up_key] = false;
			}
			if (key_up_log[Ship[i].up_key])
			{
				Ship[i].fire1_held = false;
				key_up_log[Ship[i].up_key] = false;
			}

			//Down key, fire2
			if (key_down_log[Ship[i].down_key])
			{
				Ship[i].fire2_down = true;
				Ship[i].fire2_held = true;
				key_down_log[Ship[i].down_key] = false;

			}
			if (key_up_log[Ship[i].down_key])
			{
				Ship[i].fire2_held = false;
				key_up_log[Ship[i].down_key] = false;
			}

			//Left key, left
			if (key_down_log[Ship[i].left_key])
			{
				Ship[i].left_down = true;
				Ship[i].left_held = true;
				key_down_log[Ship[i].left_key] = false;

			}
			if (key_up_log[Ship[i].left_key])
			{
				Ship[i].left_held = false;
				key_up_log[Ship[i].left_key] = false;
			}

			//Right key, right
			if (key_down_log[Ship[i].right_key])
			{
				Ship[i].right_down = true;
				Ship[i].right_held = true;
				key_down_log[Ship[i].right_key] = false;
			}
			if (key_up_log[Ship[i].right_key])
			{
				Ship[i].right_held = false;
				key_up_log[Ship[i].right_key] = false;
			}

			//Thrust key, thrust
			if (key_down_log[Ship[i].thrust_key])
			{
				Ship[i].thrust_down = true;
				Ship[i].thrust_held = true;
				key_down_log[Ship[i].thrust_key] = false;
			}
			if (key_up_log[Ship[i].thrust_key])
			{
				Ship[i].thrust_down = false;
				Ship[i].thrust_held = false;
				key_up_log[Ship[i].thrust_key] = false;
			}
		}

		else if (Ship[i].controller == GPIO_JOYSTICK)
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

		else if (Ship[i].controller == USB_JOYSTICK0 || Ship[i].controller == USB_JOYSTICK1 )
		{
			if (Ship[i].controller == USB_JOYSTICK0) joy_idx = 0;
			if (Ship[i].controller == USB_JOYSTICK1) joy_idx = 1;

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
	}

}
