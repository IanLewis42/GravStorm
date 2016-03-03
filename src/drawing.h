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

void LoadMap(void);
void FreeMap(void);
void draw_status_bar(int num_ships);
void display_menu(void);//int num_maps, int selected);
void display_map_text(int done, int timer);
void make_bullet_bitmap(void);
void draw_split_screen(ViewportType viewport, int ship_num);

extern ALLEGRO_BITMAP *tiles;
extern ALLEGRO_BITMAP *bullets_bmp;
extern ALLEGRO_BITMAP *tr_map;
extern ALLEGRO_BITMAP *tr_col_map;//Collision map. Might not need this to be an allegro bitmap, just a made-up binary format.
extern ALLEGRO_BITMAP *tr_map_section;

extern int grid;
#define MAX_GRID 3

