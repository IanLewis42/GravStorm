
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "allegro5/allegro.h"
#include "allegro5/allegro_image.h"
#include "allegro5/allegro_primitives.h"
#include "allegro5/allegro_font.h"
#include "allegro5/allegro_audio.h"

#include "game.h"
#include "init.h"
#include "objects.h"
#include "inputs.h"
#include "drawing.h"
#include "gameover.h"
#include "network.h"

int game_over = 0;
void SaveScores(void);

#define PPMINER  500    //points per miner rescued
#define PPJEWEL  200
#define PPSENTRY 100
#define PPSECOND 50

/****************************************************
** int GameOver()
** Draw increasing white overlay on screen.
** UpdateShips() isn't called while game_over > 0,
** so screen is effectively frozen.
** Decrement game_over timer and display GAME OVER
** text when expired, then wait for thrust key/button to exit
****************************************************/
int GameOver()
{
	int i,j,temp;
	static int score = 0,time_left = 0, timer = 0,position,ship,show_times=false;
	int line_space,y;
	int col1,col2,col3,col4;
	char cursor,display_name[50];
	int w,h;
	static int num_ships_latched = 0;

    w = al_get_display_width(display);
    h = al_get_display_height(display);

	//	draw game over screen, gradually getting whiter
	temp = (192/GO_TIMER)*(GO_TIMER-game_over);
	al_set_clipping_rectangle(0, 0, w, h);
	al_draw_filled_rectangle(0,0,w,h,al_map_rgba(temp,temp,temp,temp));

    switch (game_over)
    {
    case GO_TIMER:  //1st time in.

        num_ships_latched = num_ships;  //use to maintain all ships displayed on network server

        if (!Net.client)
        {
            //calculate nominal score - used to sort
            for (i=0 ; i<num_ships ; i++)
                Ship[i].score = Ship[i].lives + 10*Ship[i].kills;   //score just used to rank - this makes rank based on kills, with lives as 'tie breaker'

            int max_score = -1;

            //sort
            for (i=0 ; i<num_ships ; i++)
            {
                max_score = -1;

                for (j=0 ; j<num_ships ; j++)
                {
                    if (Ship[j].score > max_score)
                    {
                        max_score = Ship[j].score;
                        Map.score[i].score = max_score;
                        Map.score[i].player = j;
                        Map.score[i].kills = Ship[j].kills;
                        Map.score[i].lives = Ship[j].lives;
                    }
                }
                Ship[Map.score[i].player].score = -1;
            }
            for (i=0 ; i<num_ships ; i++)       //restore, but do I need to??
            {
                int ship = Map.score[i].player;
                Ship[ship].score = Map.score[i].score;
            }
        }

        if (Map.mission)
        {
            score = 0;                          //calculate score (only applicable for mission levels, but doesn't hurt.
            score += Ship[0].miners*500;
            score += Ship[0].jewels*200;
            score += Ship[0].sentries*100;

            if (Ship[0].lives && Ship[0].miners == Map.total_miners)    //TIME BONUS ONLY ON SUCCESS.
            {
                time_left = Map.time_limit - Ship[0].current_lap_time;
                if (time_left < 0) time_left = 0;
                score += time_left*PPSECOND;
            }

            position = MAX_SCORES+1;            //position in high score table - init to 'off the bottom'

            for (i=0 ; i<MAX_SCORES ; i++)              //copy oldscores -> newscores, inserting 'current score' where it fits.
            {
                if (score > Map.oldscore[i].score)
                {
                    Map.newscore[i].score = score;
                    strncpy(Map.newscore[i].name,"",50);
                    position = i;
                    break;                              //break when you've insetred new score
                }
                else
                {
                    Map.newscore[i].score = Map.oldscore[i].score;
                    strncpy(Map.newscore[i].name,Map.oldscore[i].name,50);
                }
            }
            i++;
            for ( ; i<MAX_SCORES ; i++)     //finish off the copy
            {
                Map.newscore[i].score = Map.oldscore[i-1].score;
                strncpy(Map.newscore[i].name,Map.oldscore[i-1].name,50);
            }
        }
        //Not mission  level, so might have a race.
        else if (Map.race)
        {
            show_times = false;
            ship = 0;                               //used when entering times

            for (j=0 ; j<num_ships ; j++)
            {
                if (Ship[j].lap_complete) show_times = true;    //only show times if a lap was completed
                Ship[j].lap_table_pos = MAX_SCORES+1;           //init each ship's position to 'off the table'

                for (i=0 ; i<MAX_SCORES ; i++)                  //now work out each ship's correct position in table
                {                                               //and copy into newtime, just like the scores
                    if (Ship[j].best_lap_time < Map.oldtime[i].time)
                    {
                        Map.newtime[i].time = Ship[j].best_lap_time;
                        strncpy(Map.newtime[i].name,"",50);
                        Ship[j].lap_table_pos = i;
                        break;
                    }
                    else
                    {
                        Map.newtime[i].time = Map.oldtime[i].time;
                        strncpy(Map.newtime[i].name,Map.oldtime[i].name,50);
                    }
                }
                i++;
                for ( ; i<MAX_SCORES ; i++)
                {
                    Map.newtime[i].time = Map.oldtime[i-1].time;
                    strncpy(Map.newtime[i].name,Map.oldtime[i-1].name,50);
                }
                //now copy new back to old for next ship.....
                for (i=0 ; i<MAX_SCORES ; i++)
                {
                    Map.oldtime[i].time = Map.newtime[i].time;
                    strncpy(Map.oldtime[i].name,Map.newtime[i].name,50);
                }
            }
            Ship[j].lap_table_pos = MAX_SCORES+1;
        }
        game_over--;    //next state...
    break;

    case 3:
        Command.goforward = false;
        game_over--;    //next state...
    break;

    case 2: //game_over = 2 i.e. nearly finished. display score for mission levels
        if (Map.mission)
        {
            line_space = 50*font_scale;

            y = 2*line_space;
            col1 = 100*font_scale;
            col2 = 400*font_scale;
            col3 = 800*font_scale;

            if (Ship[0].lives && Ship[0].miners == Map.total_miners)
                al_draw_textf(menu_font, al_map_rgb(0,0,0),col1,  y,  ALLEGRO_ALIGN_LEFT, "MISSION COMPLETE");
            else
                al_draw_textf(menu_font, al_map_rgb(0,0,0),col1,  y,  ALLEGRO_ALIGN_LEFT, "MISSION FAILED");
            y+=line_space;

            al_draw_textf(menu_font, al_map_rgb(0,0,0),col1,  y,  ALLEGRO_ALIGN_LEFT, "RESCUED");
            al_draw_textf(menu_font, al_map_rgb(0,0,0),col2,  y,  ALLEGRO_ALIGN_LEFT, "%02d x %d= ",Ship[0].miners,PPMINER);
            al_draw_textf(menu_font, al_map_rgb(0,0,0),col3,  y,  ALLEGRO_ALIGN_RIGHT, "%d",Ship[0].miners*PPMINER);
            y+=line_space;

            al_draw_textf(menu_font, al_map_rgb(0,0,0),col1,  y,  ALLEGRO_ALIGN_LEFT, "JEWELS");
            al_draw_textf(menu_font, al_map_rgb(0,0,0),col2,  y,  ALLEGRO_ALIGN_LEFT, "%02d x %d= ",Ship[0].jewels,PPJEWEL);
            al_draw_textf(menu_font, al_map_rgb(0,0,0),col3,  y,  ALLEGRO_ALIGN_RIGHT, "%d",Ship[0].jewels*PPJEWEL);
            y+=line_space;

            al_draw_textf(menu_font, al_map_rgb(0,0,0),col1,  y,  ALLEGRO_ALIGN_LEFT, "SENTRIES");
            al_draw_textf(menu_font, al_map_rgb(0,0,0),col2,  y,  ALLEGRO_ALIGN_LEFT, "%02d x %d = ",Ship[0].sentries,PPSENTRY);
            al_draw_textf(menu_font, al_map_rgb(0,0,0),col3,  y,  ALLEGRO_ALIGN_RIGHT, "%d",Ship[0].sentries*PPSENTRY);
            y+=line_space;

            if (Ship[0].lives && Ship[0].miners == Map.total_miners)
            {
                al_draw_textf(menu_font, al_map_rgb(0,0,0),col1,  y,  ALLEGRO_ALIGN_LEFT, "TIME BONUS");
                al_draw_textf(menu_font, al_map_rgb(0,0,0),col2,  y,  ALLEGRO_ALIGN_LEFT, "%02d x  %d = ",time_left,PPSECOND);
                al_draw_textf(menu_font, al_map_rgb(0,0,0),col3,  y,  ALLEGRO_ALIGN_RIGHT, "%d",time_left*PPSECOND);
                y+=line_space;
            }

            al_draw_textf(menu_font, al_map_rgb(0,0,0),col1,  y,  ALLEGRO_ALIGN_LEFT, "SCORE:");
            al_draw_textf(menu_font, al_map_rgb(0,0,0),col3,  y,  ALLEGRO_ALIGN_RIGHT, "%d",score);
        }
        else    //not mission, so display 'game over'
        {
            line_space = 60*font_scale;

            y = line_space/2;
            col1 = 120*font_scale;
            col2 = 260*font_scale;
            col3 = 460*font_scale;
            col4 = 600*font_scale;

            al_draw_textf(title_font, al_map_rgb(0,0,0),w/2,  y,  ALLEGRO_ALIGN_CENTER, "gAme over");    //capital A looks cooler....

            y = 200*font_scale;

            //column headings
            al_draw_textf(menu_font, al_map_rgb(0,0,0), col1,   y,  ALLEGRO_ALIGN_LEFT, "Rank");
            al_draw_textf(menu_font, al_map_rgb(0,0,0), col2,   y,  ALLEGRO_ALIGN_LEFT, "Player");
            al_draw_textf(menu_font, al_map_rgb(0,0,0), col3,   y,  ALLEGRO_ALIGN_LEFT, "Kills");
            al_draw_textf(menu_font, al_map_rgb(0,0,0), col4,   y,  ALLEGRO_ALIGN_LEFT, "Lives");
            y+=line_space;

            for (i=0 ; i<num_ships_latched ; i++)
            {
                ALLEGRO_COLOR colour;

                colour = al_map_rgb(0,0,0);

                if (Net.server || Net.client)
                {
                    if (Map.score[i].player == Net.id)  //highlight yourself
                        colour = al_map_rgb(128,0,0);
                }

                //equals sign if 2 players are equal
                if (Map.score[i].kills == Map.score[i-1].kills &&
                    Map.score[i].lives == Map.score[i-1].lives)
                    al_draw_textf(menu_font, colour, col1,   y+line_space*i,  ALLEGRO_ALIGN_LEFT, "=");
                else    //otherwise rank
                    al_draw_textf(menu_font, colour, col1,   y+line_space*i,  ALLEGRO_ALIGN_LEFT, "%d",i+1);

                //draw ship and player number
                al_draw_bitmap_region(ships,Ship[Map.score[i].player].angle*SHIP_SIZE_X,Ship[Map.score[i].player].image*SHIP_SIZE_Y*2,SHIP_SIZE_X,SHIP_SIZE_Y,col2, y+line_space*i,0);
                al_draw_textf(menu_font, colour, col2+SHIP_SIZE_X,   y+line_space*i,  ALLEGRO_ALIGN_LEFT, "(P%d)",Map.score[i].player+1);

                //kills and lives
                al_draw_textf(menu_font, colour, col3,   y+line_space*i,  ALLEGRO_ALIGN_LEFT, "%d",Map.score[i].kills);
                al_draw_textf(menu_font, colour, col4,   y+line_space*i,  ALLEGRO_ALIGN_LEFT, "%d",Map.score[i].lives);

            }
        }

        if (Command.goforward)      //thrust, or OK(?) button
        {
            Command.goforward = false;
            game_over--;
        }

        /*
        for(i=0 ; i<num_ships ; i++)	//check for each ships fire/thrust button to go to next state
        {
            if(Ship[i].thrust_down)
            {
                Ship[i].thrust_down = false;
                game_over --;
            }
        }
        */
    break;
    case 1: //game_over = 1, final state, hi score table for missions, times table for race, straight out otherwise.
        if (Map.mission)
        {
            line_space = 50*font_scale;

            y = line_space;
            col1 = 100*font_scale;
            col2 = 300*font_scale;

            timer++;            //blinking cursor
            if (timer & 0x10)
                cursor = ' ';
            else
                cursor = '_';

            if (keypress)
            {
                if (current_key == 0x0D) //return
                {
                    SaveScores();
                    game_over --;   //exit back to menu
                }
                else if (current_key == 0x08)   //backspace
                {
                    if (strlen(Map.newscore[position].name) > 0)
                        Map.newscore[position].name[strlen(Map.newscore[position].name)-1] = 0;
                }
                else if (strlen(Map.newscore[position].name) < 30)
                    strncat(Map.newscore[position].name,&current_key,50);

                keypress = false;
            }

            strncpy(display_name,Map.newscore[position].name,50);
            strcat(display_name,&cursor);

            al_draw_textf(menu_font, al_map_rgb(0,0,0),col1,  y,  ALLEGRO_ALIGN_LEFT, "HIGH SCORES");
            y+=line_space;

            for (i=0 ; i<MAX_SCORES ; i++)
            {
                al_draw_textf(menu_font, al_map_rgb(0,0,0),col1,  y+i*line_space,  ALLEGRO_ALIGN_LEFT, "%d",Map.newscore[i].score);
                if (i==position)
                    al_draw_textf(menu_font, al_map_rgb(0,0,0),col2,  y+i*line_space,  ALLEGRO_ALIGN_LEFT, "%s",display_name);
                else
                    al_draw_textf(menu_font, al_map_rgb(0,0,0),col2,  y+i*line_space,  ALLEGRO_ALIGN_LEFT, "%s",Map.newscore[i].name);
            }
            //if we are not in the high score table allow thrust to exit
            //don't otherwise, as thrust key might be a character that is typed in to high score table!
            if (position > MAX_SCORES)
            {
                for(i=0 ; i<num_ships ; i++)	//enable check for each ships fire/thrust button to go back to start (menu)
                {
                    /*
                    if(Ship[i].thrust_down)
                    {
                        Ship[i].thrust_down = false;
                        game_over --;
                        score = 0;
                    }
                    */
                    if (Command.goforward)      //thrust, or OK(?) button
                    {
                        Command.goforward = false;
                        game_over--;
                        score = 0;
                    }
                }
            }
        }
        else if (Map.race && show_times)    //race, and someone completed a lap.
        {
            line_space = 50*font_scale;

            y = line_space;
            col1 = 100*font_scale;
            col2 = 300*font_scale;
            col3 = 500*font_scale;

            while (Ship[ship].lap_table_pos > MAX_SCORES)  //skip to next ship with a lap table entry
            {
                if (ship >= num_ships)
                    break;
                ship++;
            }

            timer++;            //blinking cursor
            if (timer & 0x10)
                cursor = ' ';
            else
                cursor = '_';

            if (keypress)
            {
                if (current_key == 0x0D) //return
                {
                    SaveScores();
                    ship++;
                    while (Ship[ship].lap_table_pos > MAX_SCORES)  //skip to next ship with a lap table entry
                    {
                        ship++;
                        if (ship >= num_ships)
                        {
                            game_over--;
                            break;
                        }
                    }
                }
                else if (current_key == 0x08)   //backspace
                {
                    if (strlen(Map.newtime[Ship[ship].lap_table_pos].name) > 0)
                        Map.newtime[Ship[ship].lap_table_pos].name[strlen(Map.newtime[Ship[ship].lap_table_pos].name)-1] = 0;
                }
                else if (strlen(Map.newtime[Ship[ship].lap_table_pos].name) < 30)
                    strncat(Map.newtime[Ship[ship].lap_table_pos].name,&current_key,50);

                keypress = false;
            }

            strncpy(display_name,Map.newtime[Ship[ship].lap_table_pos].name,50);
            strcat(display_name,&cursor);

            al_draw_textf(menu_font, al_map_rgb(0,0,0),col1,  y,  ALLEGRO_ALIGN_LEFT, "BEST TIMES");
            y+=line_space;

            for (i=0 ; i<MAX_SCORES ; i++)
            {
                al_draw_textf(menu_font, al_map_rgb(0,0,0),col1,  y+i*line_space,  ALLEGRO_ALIGN_LEFT, "%.3f",Map.newtime[i].time);
                if (i==Ship[ship].lap_table_pos)
                {
                    al_draw_textf(menu_font, al_map_rgb(0,0,0),col2,  y+i*line_space,  ALLEGRO_ALIGN_LEFT, "(P%d)",ship+1);
                    al_draw_textf(menu_font, al_map_rgb(0,0,0),col3,  y+i*line_space,  ALLEGRO_ALIGN_LEFT, "%s",display_name);
                }
                else
                    al_draw_textf(menu_font, al_map_rgb(0,0,0),col3,  y+i*line_space,  ALLEGRO_ALIGN_LEFT, "%s",Map.newtime[i].name);
            }

            //if we are not in the high score table allow thrust to exit
            if (ship >= num_ships)
            {
                for(i=0 ; i<num_ships ; i++)	//and enable check for each ships fire/thrust button to go back to start (menu)
                {
                    /*
                    if(Ship[i].thrust_down)
                    {
                        Ship[i].thrust_down = false;
                        game_over --;
                        score = 0;
                    }
                    */
                    if (Command.goforward)      //thrust, or OK(?) button
                    {
                        Command.goforward = false;
                        game_over--;
                        score = 0;
                    }
                }
            }
        }
        else
            game_over--;
    break;
    default:
        game_over--;					//decrement timer
    break;
    }
	return game_over;	//return 0 when we want to restart the game
}

void SaveScores(void)   //scores/times.
{
    FILE *score_file;
    char score_file_name[50];
    char temp[100];
    const char scores[7] = "_s.bin\0";
    int i,j;

    strncpy(score_file_name, (char*)&MapNames[Menu.group].Map[Menu.map], MAP_NAME_LENGTH);

    strcat(score_file_name, scores);

    score_file = fopen(score_file_name,"w");

    if (score_file == NULL)
    {
        fprintf(logfile,"Couldn't open scores/times file %s. High scores/times can't be saved.\n",score_file_name);
    }
    else
    {
        fprintf(logfile,"Opened scores/times file %s\n",score_file_name);

        for (i=0 ; i<MAX_SCORES ; i++)
        {
            if (Map.mission)
                sprintf(temp,"%d %s",Map.newscore[i].score,Map.newscore[i].name);
            else if (Map.race)
                sprintf(temp,"%.3f %s",Map.newtime[i].time,Map.newtime[i].name);

            for (j=0 ; temp[j] != 0 ; j++)
                temp[j]+=0x81+i;

            fprintf(score_file,"%s\n",temp);
        }

        fclose(score_file);
    }
}
