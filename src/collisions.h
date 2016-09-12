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

void make_ship_col_mask(void);
void make_map_col_mask(void);
void make_sentry_col_mask(void);

void CheckSSCollisions(int num_ships);
void CheckBSCollisions(int num_ships);
void CheckSWCollisions(int num_ships);
void CheckBWCollisions(void);
void CheckBSentryCollisions(void);
void CheckBSwitchCollisions(void);

int EquivalentColour(ALLEGRO_COLOR col1, ALLEGRO_COLOR col2);

extern unsigned long int ship_col_mask[], map_col_mask[];
