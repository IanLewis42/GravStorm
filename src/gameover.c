
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define ALLEGRO_UNSTABLE 1  //needed for haptics.

#include "allegro5/allegro.h"
#include "allegro5/allegro_image.h"
#include "allegro5/allegro_primitives.h"
#include "allegro5/allegro_font.h"
#include "allegro5/allegro_audio.h"
#ifdef ANDROID
#include <allegro5/allegro_android.h>
#endif

#include "game.h"
#include "init.h"
#include "objects.h"
#include "inputs.h"
#include "drawing.h"
#include "gameover.h"
#include "network.h"

int game_over = 0;
void SaveScores(void);
void DisplayScores(int position, int start);
void DisplayTimes(int position, int ship);


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
	static int score = 0,time_left = 0,position,ship,show_times=false;
	int line_space,y;
	int col1,col2,col3,col4;
	int w,h;
	static int num_ships_latched = 0;

    if (halted) return game_over;

    w = al_get_display_width(display);
    h = al_get_display_height(display);

	//	draw game over screen, gradually getting whiter
	temp = 1-((192/GO_TIMER)*(GO_TIMER-game_over));
	al_set_clipping_rectangle(0, 0, w, h);
	al_draw_filled_rectangle(0,0,w,h,al_map_rgba(temp,temp,temp,temp));

    //for (i=0 ; i<NO_BUTTON ; i++)
    //    if (Ctrl.ctrl[i].active)
    //        al_draw_scaled_bitmap(Ctrl.controls, Ctrl.ctrl[i].idx*200, i*200, 200,200, Ctrl.ctrl[i].x, Ctrl.ctrl[i].y, Ctrl.ctrl[i].size, Ctrl.ctrl[i].size, 0);
    draw_controls(al_map_rgba_f(0.5,0.5,0.5,0.5));

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
        for (i=0 ; i<num_ships ; i++)                       //needed for mission and race levels
            Ship[i].lap_table_pos = MAX_SCORES+1;           //init each ship's position to 'off the table'

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

            //DEBUG
            //score = 4000;

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
                //Ship[j].lap_table_pos = MAX_SCORES+1;           //init each ship's position to 'off the table'

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

    case 4:
        Command.goforward = false;
        game_over--;    //next state...
    break;

    case 3: //game_over = 2 i.e. nearly finished. display score for mission levels
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
                al_draw_scaled_bitmap(ships,Ship[Map.score[i].player].angle*SHIP_SIZE_X,Ship[Map.score[i].player].image*SHIP_SIZE_Y*2,SHIP_SIZE_X,SHIP_SIZE_Y,col2, y+line_space*i,SHIP_SIZE_X*scale,SHIP_SIZE_Y*scale,0);
                al_draw_textf(menu_font, colour, col2+SHIP_SIZE_X*scale,   y+line_space*i,  ALLEGRO_ALIGN_LEFT, "(P%d)",Map.score[i].player+1);

                //kills and lives
                al_draw_textf(menu_font, colour, col3,   y+line_space*i,  ALLEGRO_ALIGN_LEFT, "%d",Map.score[i].kills);
                al_draw_textf(menu_font, colour, col4,   y+line_space*i,  ALLEGRO_ALIGN_LEFT, "%d",Map.score[i].lives);

            }
        }

        if (Command.goforward)      //thrust, or OK(?) button
        {
            Command.goforward = false;
            game_over--;
#ifdef ANDROID
            //_jni_callVoidMethodV(_al_android_get_jnienv(), _al_android_activity_object(), "UiChangeListener", "()V");
            if (Map.mission && (position < MAX_SCORES))
                _jni_callVoidMethodV(_al_android_get_jnienv(), _al_android_activity_object(),"OpenKeyBoard", "()V");
            if (Map.race && (Ship[0].lap_table_pos < MAX_SCORES))
                _jni_callVoidMethodV(_al_android_get_jnienv(), _al_android_activity_object(),"OpenKeyBoard", "()V");
#endif
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
    case 2: //game_over = 1, final state, hi score table for missions, times table for race, straight out otherwise.
        if (Map.mission)
        {
            if (keypress)
            {
                if (current_key == 0x0D) //return
                {
                    SaveScores();
                    game_over --;   //exit back to menu
#ifdef ANDROID
                    _jni_callVoidMethodV(_al_android_get_jnienv(), _al_android_activity_object(), "CloseKeyBoard", "()V");
#endif
                }
                else if (current_key == 0x08)   //backspace
                {
                    if (strlen(Map.newscore[position].name) > 0)
                        Map.newscore[position].name[strlen(Map.newscore[position].name)-1] = 0;
                }
                else if (strlen(Map.newscore[position].name) < 30)
                    strncat(Map.newscore[position].name,&current_key,1);

                keypress = false;
            }

            temp=0;
#ifdef ANDROID
            if (position < MAX_SCORES)
                temp = position;
#endif
            DisplayScores(position, temp);

            //if we are not in the high score table allow thrust to exit
            //don't otherwise, as thrust key might be a character that is typed in to high score table!
            if (position > MAX_SCORES)
            {
                for(i=0 ; i<num_ships ; i++)	//enable check for each ships fire/thrust button to go back to start (menu)
                {
                    if (Command.goforward)      //thrust, or OK(?) button
                    {
                        Command.goforward = false;
                        game_over-=2;
                        score = 0;
                    }
                }
            }
        }
        else if (Map.race && show_times)    //race, and someone completed a lap.
        {

            while (Ship[ship].lap_table_pos > MAX_SCORES)  //skip to next ship with a lap table entry
            {
                if (ship >= num_ships)
                    break;
                ship++;
            }

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
#ifdef ANDROID
                            _jni_callVoidMethodV(_al_android_get_jnienv(), _al_android_activity_object(), "CloseKeyBoard", "()V");
#endif
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
                    strncat(Map.newtime[Ship[ship].lap_table_pos].name,&current_key,1);

                keypress = false;
            }

            temp = 0;
#ifdef ANDROID
            if (Ship[0].lap_table_pos < MAX_SCORES)
                temp = Ship[0].lap_table_pos-2;
            if (temp < 0)
                temp = 0;
#endif
            DisplayTimes(temp, ship);

            //if we are not in the high score table allow thrust to exit
            if (ship >= num_ships)
            {
                for(i=0 ; i<num_ships ; i++)	//and enable check for each ships fire/thrust button to go back to start (menu)
                {
                    if (Command.goforward)      //thrust, or OK(?) button
                    {
                        Command.goforward = false;
                        game_over-=2;
                        score = 0;
                    }
                }
            }
        }
        else
            game_over-=2;
    break;
    case 1:
#ifdef ANDROID
            if (Map.mission)
                DisplayScores(11, 0);
            else if (Map.race && show_times)    //race, and someone completed a lap.
                DisplayTimes(0, MAX_SHIPS);

            if (Command.goforward)      //thrust, or OK(?) button
            {
                Command.goforward = false;
                game_over--;
                score = 0;
            }
#else
            game_over--;
#endif
    break;
    default:
        game_over--;					//decrement timer
    break;
    }
	return game_over;	//return 0 when we want to restart the game
}

void DisplayScores(int position, int start)
{
    int line_space = 50*font_scale;
    int i;
    int y = line_space;
    int col1 = 100*font_scale;
    int col2 = 300*font_scale;
    int col3 = 350*font_scale;

    char cursor[2];
    char blank[] = " ",underscore[] = "_";
    char display_name[50];

    static int timer = 0;

    timer++;            //blinking cursor
    if (timer & 0x10)
        strcpy(cursor,blank);
    else
        strcpy(cursor,underscore);

    if (position < MAX_SCORES) {
        strncpy(display_name, Map.newscore[position].name, 49);
        strcat(display_name, cursor);
    }

    al_draw_textf(menu_font, al_map_rgb(0,0,0),col1,  y,  ALLEGRO_ALIGN_LEFT, "HIGH SCORES");
    y+=line_space;

    for (i=start ; i<MAX_SCORES ; i++)
    {
        al_draw_textf(menu_font, al_map_rgb(0,0,0),col1,  y,  ALLEGRO_ALIGN_LEFT, "%d",i+1);
        al_draw_textf(menu_font, al_map_rgb(0,0,0),col2,  y,  ALLEGRO_ALIGN_RIGHT, "%d",Map.newscore[i].score);

        if (i==position)
            al_draw_textf(menu_font, al_map_rgb(0,0,0),col3, y,  ALLEGRO_ALIGN_LEFT, "%s",display_name);
        else
            al_draw_textf(menu_font, al_map_rgb(0,0,0),col3, y,  ALLEGRO_ALIGN_LEFT, "%s",Map.newscore[i].name);
        y+=line_space;
    }
}

void DisplayTimes(int start, int ship)
{
    int line_space = 50*font_scale;
    int i;
    int y = line_space;
    int col1 = 100*font_scale;
    int col2 = 200*font_scale;
    int col3 = 400*font_scale;
    int col4 = 500*font_scale;

    char cursor[2],blank[] = " ",underscore[] = "_";
    char display_name[50];

    static int timer = 0;

    timer++;            //blinking cursor
    if (timer & 0x10)
        strcpy(cursor,blank);
    else
        strcpy(cursor,underscore);

    strncpy(display_name,Map.newtime[Ship[ship].lap_table_pos].name,49);
    strcat(display_name,cursor);

    al_draw_textf(menu_font, al_map_rgb(0,0,0),col1,  y,  ALLEGRO_ALIGN_LEFT, "BEST TIMES");
    y+=line_space;

    for (i=start ; i<MAX_SCORES ; i++)
    {
        al_draw_textf(menu_font, al_map_rgb(0,0,0),col1,  y,  ALLEGRO_ALIGN_LEFT, "%d",i+1);
        al_draw_textf(menu_font, al_map_rgb(0,0,0),col2,  y,  ALLEGRO_ALIGN_LEFT, "%.3f",Map.newtime[i].time);
        if (ship < MAX_SHIPS && i==Ship[ship].lap_table_pos)
        {
            al_draw_textf(menu_font, al_map_rgb(0,0,0),col3,  y,  ALLEGRO_ALIGN_LEFT, "(P%d)",ship+1);
            al_draw_textf(menu_font, al_map_rgb(0,0,0),col4,  y,  ALLEGRO_ALIGN_LEFT, "%s",display_name);
        }
        else
            al_draw_textf(menu_font, al_map_rgb(0,0,0),col4,  y,  ALLEGRO_ALIGN_LEFT, "%s",Map.newtime[i].name);
        y+=line_space;
    }

}

void SaveScores(void)   //scores/times.
{
    ALLEGRO_FILE *score_file;
    char score_file_name[MAP_NAME_LENGTH];
    char temp[100];
    const char scores[7] = "_s.bin\0";
    int i,j;

    strncpy(score_file_name, (char*)&MapNames[Menu.group].Map[Menu.map], MAP_NAME_LENGTH-1);

    strcat(score_file_name, scores);
#ifdef ANDROID
    char pathstr[100];
    char *pathptr;
    al_set_standard_file_interface();
    ALLEGRO_PATH *path = al_get_standard_path(ALLEGRO_USER_DATA_PATH);    //android
    //ALLEGRO_DEBUG(#std ": %s", al_path_cstr(path, '/'));
    sprintf(pathstr,"%s",al_path_cstr(path, '/'));
    al_change_directory(al_path_cstr(path, '/'));  // change the working directory
    pathptr = al_get_current_directory();
#endif
    score_file = al_fopen(score_file_name,"w");

    if (score_file == NULL)
    {
        al_fprintf(logfile,"Couldn't open scores/times file %s. High scores/times can't be saved.\n",score_file_name);
    }
    else
    {
        al_fprintf(logfile,"Opened scores/times file %s\n",score_file_name);

        for (i=0 ; i<MAX_SCORES ; i++)
        {
            if (Map.mission)
                sprintf(temp,"%d %s\n",Map.newscore[i].score,Map.newscore[i].name);
            else if (Map.race)
                sprintf(temp,"%.3f %s\n",Map.newtime[i].time,Map.newtime[i].name);

            for (j=0 ; temp[j] != 0 ; j++)
                temp[j]+=0x81+i;

            al_fprintf(score_file,"%s\n",temp);
#if 0
            al_fputs(score_file,temp);

#ifdef _WIN32
            al_fputc(score_file,0x0D);  //terminate line
#endif
            al_fputc(score_file,0x0A);
#endif
        }

        al_fclose(score_file);
#ifdef ANDROID
        al_android_set_apk_file_interface();
#endif
    }
}
