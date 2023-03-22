/*
	GravStorm
    Copyright (C) 2015-2018 Ian Lewis

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

void AutoShip(int i);
void GotoReturn(int i);
void GotoTakeoff(int i);
void GotoCruise(int i);
void GotoHunt(int i);

void find_obstructions(int i);
void find_walls8(int i);
void find_walls(int i);
void find_ships(int i);

extern WallType walls[NUM_ANGLES];
extern float sumx,sumy,ratio;
extern int avoid_angle,target_angle;
extern float targetx,targety,avoidx,avoidy;
extern int fire_state;
