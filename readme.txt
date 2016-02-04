README.TXT
----------

INTRODUCTION
------------
Welcome to GravStorm.

This is a game in the Gravity War / Cave Flyer genre, heavily based on the early 90's Amiga game 'TurboRaketti '
(possibly my favourite game ever...) and to a lesser extent the 8-bit classic 'Thrust'.

If you've never played TurboRaketti, then it can be described as a 2 player version of 'Thrust' where your objective
is to shoot the other player before he shoots you. If you've never played 'Thrust' then think of 'Asteroids'
with gravity and walls. If you've never played Asteroids..... ah, I give up :-)

If you're curious as to what it looks like, there should be some screenshots in the same directory as this file.

REQUIREMENTS
------------
o Raspberry Pi
  ~6MB free disk/SD card space
  TV / Monitor plugged into the Pi. It only runs full-screen (not in a window) and VNC doesn't display anything.
  I've also only run it on a 16:9 TV at a resolution of 1280x720, over HDMI. I don't know what happens if your
  display isn't big enough. Feedback welcome :-)
  There are probably some software / library dependencies. Let me know if you figure out what they are :-)
  Oh yes, I built/ran it under Raspbian. I assume you'll need Raspbian too. 
  
o Windows
  Coming soon....
  
o Android  
  Possible (If you've ever made anything for Android, please get in touch)

o iOS
  No chance. Unless you want to do it :-)

INSTALLATION
------------
Unzip the contents of this into the home directory on your Pi (probably /home/pi). You should end up with the following 
directory structure:

~/game
~/game/data
~/game/src

STARTING THE GAME
------------------
Change into the game directory, and run the executable 'game' (I told you it needs a name...)

cd game
./game

MENU
-----
It's supposed to be intuitive / self explanatory, but just in case....

Navigate the menu with the cursor keys or any of the selected control options.
First column is the map/level you'll be playing on. Some levels allow up to 2 players, some allow up to 4.
Next column selects the player.
Next column selects the control method for the selected player (Keys, joystick, N/A). 
  The FIRST instance of N/A in this column sets the number of active players.
Final column allows you to define keys.
Press Return or Fire/Thrust to display the level description, then again to start the game.
Escape to exit.

PLAYING
-------
Default controls are:
 Player 1: Cursors and Right Ctrl
 Player 2: Q A R T and Left Alt
 
Right and Left rotate the ship clockwise and anti-clockwise.
Up/Forwards fires your 'normal' weapon (rapid fire, low damage)
Down/Back fires your 'special' weapon (limited ammo, more damage, various types)
Thrust/Joystick Button fires your engine.

Some levels have a race track. This should be described in the level text.
Current lap time and last completed lap time are displayed in the status panel.

o Single player 'Mission' levels
  These are largely based on the 'Thrust' maps. Here, the objective is to rescue the miners who are stuck at the 
  bottom of the cave system. Land on the 'blue' pads to rescue them. Land on the 'red' pads to collect jewels for 
  bonus points. Beware that your ship will be heavier with extra passengers and cargo! Fly off away from the surface
  to complete the level.


STATUS PANEL
------------
The bar on the left hand side of the screen has a status panel for each ship. 
The background is the same colour as the ship. The layout is as follows:

Green  - Shield 
Red    - Normal Ammo
Cyan   - Special Ammo
Yellow - Fuel

White triangles are lives remaining.
Current and last completed lap times are displayed at the bottom of the panel.

For 'Mission' levels, miners rescued and jewels collected are shown, along with the elapsed time.

SHIP CONFIG
-----------
If you press Down/Fire2 when on your landing pad, a config menu will appear.
This lets you choose how much fuel you carry, and how much and what types of ammunition.
More fuel and ammo make your ship heavier, and therefore less manouverable.

JOYSTICKS
---------
Joysticks (only 2 at the moment - could be extended) can be connected via USB. 
I've tested with a Speedlink Competition Pro USB, but I don't see why any joystick won't work.
I don't know how well it will work with an analogue stick.
My wireless keyboard / trackpad (Perixx Periboard-716) seems to be detected as a USB joystick,
but doesn't then work as one! This makes it difficult to detect how many 'real' joysticks are plugged in.
So if you have a USB joystick, try both USB Joy 1 and USB Joy 2 in the menu.
There is also support for an old-fashioned microswitch joystick connected to the Pi's
GPIO header. To enable this, use the command line:

sudo ./game -g

sudo is required for access to the GPIO hardware. If you want to know which pins do what, 
have a look in the source, or ask me.

KNOWN ISSUES
------------
There's a significant lag between events that cause sounds and the sounds themselves.
I've spent a while trying to fix it. If anyone knows all about ALSA, please get in touch!

MAKING MAPS
-----------
The game has been designed to make creating your own levels as easy as possible. It's still quite complicated though :-)

I tend to use the terms 'level' and 'map' interchangably. Sorry for any confusion this causes :-)

- The file 'data/maps.txt' lists all the maps(/levels) that can be played. A : at the start of an entry indicates a group.
  Each entry in maps.txt (apart from the groups) must have a corresponding .txt file in the /data directory. So if
  the entry in maps.txt is my_level, you must have a file called data/my_level.txt
  
- This .txt file contains all the information about the level. Each parameter should be typed on its own line. 
  

o map_type <0|1>
  0 - Single image file for map (default)
  1 - Tiled map
  
o display_map <filename> only png format supported. REQUIRED
  Image to display, or images of all the tiles, arranged in a single horizontal line
  
o collision_map <filename> only png format supported. REQUIRED
  Similar to display_map, but all empty space MUST be 'magic pink' (i.e. R=255, G=0, B=255)
  
o ascii_map <filename> 
  Shows the arrangement of tiles to make the map. ASCII format, 0-9 for first 10 tiles, A-Z for next 26. ' ' (space) doubles as 0.
  Only applies if map_type = 1
  
o sentry_display <filename> only png format supported.
  Images of all the sentries and forcefields, arranged in a single horizontal line

o sentry_collision <filename> only png format supported.
  Similar to sentry_display, but all empty space MUST be 'magic pink' (i.e. R=255, G=0, B=255)
  
o description <filename> 
  Text displayed after selecting map
  
o ship_first <0|1>
  0 - Map gets drawn first, then ship (also sentries, forcefields etc.) on top (default)
  1 - ship (also sentries, forcefields etc.) get drawn first, then Map on top
  
o wrap <0|1>
  0 - Ship stops at edge of map (if there's no wall)  (default)
  1 - Ship wraps around to opposite edge
  
o mission <0|1>
  0 - Use for multiplayer levels  (default)
  1 - Use for single player levels
  
o lives <1-6>
  default = 6;
  
o time_limit <number of seconds> 
  only applies if 'mission' = 1. Default = 60

o max_players <1-4>
  Default = 1

o gravity
  Default = 0.05
  
o drag
  Air resistance. Default = 2

o pad <type> <y> <min x> <max x> <miners> <jewels>  REQUIRED (at least one)
  <type> is a hexadecimal number.
    Lowest nibble (digit) is the ship that this pad is 'home' for (0-3). Each ship always appears on its home pad, 
      and will get shield, fuel and both types of ammo recharged when on it. If this digit is > 3 then this pad will
      be a general pad, not home for any ship.
    Next nibble up determines which attributes will be recharged for any ship landing on this pad, mapped as follows:
    FUEL   0x0010
    AMMO1  0x0020
    AMMO2  0x0040
    SHIELD 0x0080
    e.g. if pad type is 10 then it is home for ship 0, and will recharge fuel for any ship that lands on it
         if pad type is 9F then it's not home for any ship (F = 15, which is > 3) but will recharge fuel and shield for any ship (8+1 = 9)
  <y> is the y-coordinate of the pad. 0 is the top of the map, and it becomes more positive as you go down.
    The easiest way to find the y-coordinate is to start the game with the debug switch -d, try to land on the pad, 
      and note the y-cordintae displayed in the status bar when you crash on the pad.
    
    
&Map.pad[i].type,&Map.pad[i].y,&Map.pad[i].min_x,&Map.pad[i].max_x,&Map.pad[i].miners,&Map.pad[i].jewels);
Pad %d: type:%02x y:%d x:%d x:%d miners:%d jewels:%d\n",i,Map.pad[i].type,Map.pad[i].y,Map.pad[i].min_x,Map.pad[i].max_x,Map.pad[i].miners,Map.pad[i].jewels);

Ship[Map.pad[i].type & 0x000f].home_pad = i;    //bottom nibble of type gives ship which this is home pad for.

area
&Map.area[j].min_x,&Map.area[j].max_x,&Map.area[j].min_y,&Map.area[j].max_y,&Map.area[j].gravity,&Map.area[j].drag);
area %d: x:%d x:%d y:%d y:%d g:%f drag:%f\n",j,Map.area[j].min_x, Map.area[j].max_x, Map.area[j].min_y, Map.area[j].max_y, Map.area[j].gravity, Map.area[j].drag);

blackhole
&Map.blackhole[l].x,&Map.blackhole[l].y,&Map.blackhole[l].g);
blackhole %d: x:%d y:%d g:%f\n",l,Map.blackhole[l].x,Map.blackhole[l].y,Map.blackhole[l].g);

//      x y type(0/1/2) gun volcano firing period probability random/targeted
sentry
&Map.sentry[m].x, &Map.sentry[m].y, &Map.sentry[m].direction, &Map.sentry[m].type, &Map.sentry[m].period, &Map.sentry[m].probability, &Map.sentry[m].random, &Map.sentry[m].range, &Map.sentry[m].alive_sprite, &Map.sentry[m].dead_sprite);
Sentry %i: x:%i, y:%i, Direction:%i, Type:%i, Period:%i, Prob:%i, Random:%i, Range:%i, Sprite%d, Sprite%d\n",m, Map.sentry[m].x,  Map.sentry[m].y,  Map.sentry[m].direction,  Map.sentry[m].type,  Map.sentry[m].period,  Map.sentry[m].probability,  Map.sentry[m].random,  Map.sentry[m].range, Map.sentry[m].alive_sprite, Map.sentry[m].dead_sprite);
Map.sentry[m].range = Map.sentry[m].range * Map.sentry[m].range;	//square to save square rooting later.

race
&Map.raceline_minx,&Map.raceline_maxx,&Map.raceline_miny,&Map.raceline_maxy);
else fprintf(logfile,"X or Y values must match in race start/finish line (i.e. line must be horizontal or vertical)\n");
fprintf(logfile,"Race:   x:%d x:%d y:%d y:%d\n",Map.raceline_minx,Map.raceline_maxx,Map.raceline_miny,Map.raceline_maxy);
fprintf(logfile,"Before: x:%d x:%d y:%d y:%d\n",Map.before_minx,Map.before_maxx,Map.before_miny,Map.before_maxy);
fprintf(logfile,"After:  x:%d x:%d y:%d y:%d\n",Map.after_minx,Map.after_maxx,Map.after_miny,Map.after_maxy);

forcefield
&Map.forcefield[n].min_x,&Map.forcefield[n].max_x,&Map.forcefield[n].min_y,&Map.forcefield[n].max_y,&Map.forcefield[n].strength,&Map.forcefield[n].sentry,&Map.forcefield[n].alive_sprite,&Map.forcefield[n].dead_sprite);
else fprintf(logfile,"X or Y values must match in forcefield (i.e. line must be horizontal or vertical)\n");

fprintf(logfile,"ForceField: x:%d x:%d x:%d y:%d y:%d y:%d\n",Map.forcefield[n].min_x,Map.forcefield[n].half_x,Map.forcefield[n].max_x,Map.forcefield[n].min_y,Map.forcefield[n].half_y,Map.forcefield[n].max_y);
fprintf(logfile,"ForceField: strength:%0.0f sentry:%d\n",Map.forcefield[n].strength,Map.forcefield[n].sentry);


LEGAL STUFF
-----------
Ian's Thrust Game  Copyright (C) 2015-2016  Ian Lewis
This program and it's source code are released under the GPL.
This program comes with ABSOLUTELY NO WARRANTY.
This is free software, and you are welcome to redistribute it
under certain conditions. See gpl.txt for details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

IPL 18/01/16
iansthrustgame@gmail.com
