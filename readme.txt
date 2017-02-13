README.TXT
----------

INTRODUCTION 
------------
Welcome to GravStorm.

This is a game in the Gravity War / Cave Flyer genre, heavily based on the early 90's Amiga game 'TurboRaketti' (possibly my favourite game ever...) and to a lesser extent the 8-bit classic 'Thrust'.

If you've never played TurboRaketti, then it can be described as a 2 player version of 'Thrust' where your objective is to shoot the other player before he shoots you. If you've never played 'Thrust' then think of 'Asteroids' with gravity and walls. If you've never played Asteroids..... ah, I give up :-)

If you're curious as to what it looks like, there should be some screenshots in the same directory as this file.

REQUIREMENTS
------------
o Raspberry Pi
  I built/ran it under Raspbian. If you just want to run it, you'll need Raspbian too. I've tested it under both Wheezy and Jessie versions of Raspbian and it seems OK. If you want to build it yourself, feel free to try under any OS you want ;-)
  ~30MB free disk/SD card space.
  TV / Monitor plugged into the Pi. It only runs full-screen (not in a window) and VNC doesn't display anything. I've also only run it on a 16:9 TV at a resolution of 1280x720, over HDMI. I don't know what happens if your display isn't big enough. Feedback welcome :-)
  There don't seem to be any dependencies that aren't in the standard Raspbian Jessie image (as of 23/02/16).
    
o Windows
  Should run on any modern version of windows. I test on Windows7 32-bit ('cos that's what I have :-)
  Reported running on Windows XP and Windows 10.
  If you run it on any other version, please let me know how you get on.
  
o Android  
  Development is under consideration (If you've ever made anything for Android, and want to help, please get in touch)

o iOS
  No chance. Unless you want to do it :-)

INSTALLATION
------------
Go to https://github.com/IanLewis42/GravStorm 
Click the button labelled 'Clone or Download' and then 'Download Zip'.

Save the zip file, and unzip the contents of this into the directory of your choice. 

o Raspberry Pi
  On Raspberry Pi, your home directory is a good choice (probably /home/pi). You should end up with the following directory structure:

  ~/gravstorm
  ~/gravstorm/data
  ~/gravstorm/src

  (Github seems to make the directory name 'gravstorm-master'. You can leave it, or change it. I'm going to refer to it as 'gravstorm').

  You may then need to change the file permissions to make the game executable. Start up a terminal (e.g. LXterminal) 
    - change into the gravstorm directory:
      cd gravstorm
    - set file permissions:
      chmod +x gravstorm
      
  One level (neutron star) requires the Raspberry Pi to be configured with at least 128MB of Graphics RAM. The default is 64MB. This can be changed by running the config utiliy. In LXterminal type:
    sudo raspi-config
    select 'Advanced Options', then 'Memory Split' and type '128'. You probably need to reboot for this to take effect. 
  
  More on raspi-config here:
  https://www.raspberrypi.org/documentation/configuration/raspi-config.md
      
o Windows
  Unzip wherever you like. You should get a folder named 'gravstorm' (or gravstorm-master if GitHub changes it) and inside that, folders called 'data' and 'src'. 
  
  Note that the Windows executable is called 'gravstorm.exe' ; the Raspberry Pi executabe is just 'gravstorm'. You can delete whichever one you don't need.  

STARTING THE GAME
------------------
o Raspberry Pi
  Change into the game directory (if you're not there already), and run the executable 'gravstorm'

  cd gravstorm
  ./gravstorm

  NOTE: My Pi boots straight into the GUI, and I normally run Gravstorm via LXTerminal. It also works if you run it direct from the command line, without the desktop GUI running. If you try to run it from the desktop/gui and get an error message (perhaps similar to the one shown below) then try it without the GUI.
  ***
  XIO:  fatal IO error 11 (Resource temporarily unavailable) on X server ":0.0"
  after 17 requests (17 known processed) with 0 events remaining.
  ***
  
o Windows
  In the gravstorm folder, there is a file called 'gravstorm.exe'.
  Double click this to run it. Feel free to make a shortcut, pin to the start menu or whatever.
  
MENU
-----
It's supposed to be intuitive / self explanatory, but just in case....

Press any key to skip the title screen and go to the menu.

In general, use up and down cursor keys to navigate the menu, left and right to change a highlighted item.
'Enter' will take you forward to the next menu level, 'Escape' will take you back to the previous level.

o MODE
  Use cursor 'up' and 'down' to select game mode.
    Local Game        - Select this for single player, or for up to 4 players in split-screen mode on the same device.
    Host Network Game - Select this to allow other players to join 'your' game. As the host, you get to select the level.
    Join Network Game - Select this to join a game hosted by another player.
  
    NOTE: Network support is ONLY for a local network, not over the internet.
  Enter:  Next menu
  Escape: Exit 

o LEVEL
  Use cursor 'up' and 'down' to select level.
  Enter:  Next menu
  Escape: Previous menu 
  
o PLAYERS  
  Use cursor 'up' and 'down' to select item.
  Use left and right cursor keys to select number of players, ship type, and controller type when highlighted.
    If you press 'right' when 'define keys->' is highlighted, the next five keypresses will be used as your controls.
  Enter:  Start Game
  Escape: Previous menu 

NOTE: Any set of player defined keys, or joystick can also be used to navigate the menu. 

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
Current lap time, last completed lap time and best lap time for this game are displayed in the status panel.
If you complete a lap, the best times (and players who set them) will be displayed at the end of the game.

In network play only, F10 will toggle a 'radar' display at the top-right of the screen, showing the terrain, and all ships.

o Single player 'Mission' levels
  These are largely based on the 'Thrust' maps. Here, the objective is to rescue the miners who are stuck at the bottom of the cave system. Land on the 'blue' pads to rescue them. Land on the 'red' pads to collect jewels for bonus points. Beware that your ship will be heavier with extra passengers and cargo! Fly off away from the surface to complete the level.
  
STATUS PANEL
------------
At the top-left of each player's window there is a status panel for the ship. 
The layout is as follows:

Green  - Shield 
Red    - Normal Ammo
Cyan   - Special Ammo
Yellow - Fuel

White triangles are lives remaining.
Current, last completed and best lap times are displayed at the bottom of the panel.

For 'Mission' levels, miners rescued and jewels collected are shown, along with the elapsed time.

SHIP CONFIG
-----------
If you press Down/Fire2 when on your landing pad, a config menu will appear. This lets you choose how much fuel you carry, and how much and what types of ammunition. More fuel and ammo make your ship heavier, and therefore less manouverable.

EXITING
-------
If the game is completed (i.e. one player loses all lives, or mission completed) press 'thust' to return to the menu. 
If you want to exit the level before completion, press 'escape' to go back to the menu. 
Press 'escape' to exit from the menu back to the command prompt.

JOYSTICKS
---------
Joysticks (only 2 at the moment - could be extended) can be connected via USB. I've tested with a Speedlink Competition Pro USB, but I don't see why any joystick won't work. I don't know how well it will work with an analogue stick.

My wireless keyboard / trackpad (Perixx Periboard-716) seems to be detected as a USB joystick, but doesn't then work as one! This makes it difficult to detect how many 'real' joysticks are plugged in. So if you have a USB joystick, try both USB Joy 1 and USB Joy 2 in the menu.

There is also support in the Raspberry Pi version for an old-fashioned microswitch joystick connected to the Pi's GPIO header. To enable this, use the command line:

sudo ./gravstorm -g

sudo is required for access to the GPIO hardware. If you want to know which pins do what, have a look in the source, or ask me.

TROUBLESHOOTING
----------------
Gravstorm makes a logfile every time it runs. This is called 'logfile.txt' and is in the 'gravstorm' directory. If gravstorm crashes, or won't start, then have a look in here and see if there's anything helpful. If it doesn't make any sense, then feel free to send it to me.
For network games, a file called 'host.txt' or 'client.txt' will also be produced. Likewise, have a look in it, or send it to me.

KNOWN ISSUES
------------
On Raspberry Pi, there's a significant lag between events that cause sounds and the sounds themselves. I've spent a while trying to fix it. If anyone knows all about ALSA, please get in touch!

MAKING MAPS
-----------
*** Only read this bit if you want to make your own maps / levels (or modify the existing ones)***

The game has been designed to make creating your own levels as easy as possible. It's still quite complicated though :-)

I tend to use the terms 'level' and 'map' interchangably. Sorry for any confusion this causes :-)

- The file 'data/maps.txt' lists all the maps(/levels) that can be played. A : at the start of an entry indicates a group. Each entry in maps.txt (apart from the groups) must have a corresponding .txt file in the /data directory. So if the entry in maps.txt is my_level, you must have a file called data/my_level.txt
  
- This .txt file contains all the information about the level. Each parameter should be typed on its own line. 
  Pretty much anything (other than whitespace) can be used as a single-line comment, but I tend to use a semicolon ;
  The easiest thing to do is take a look at some of the existing  files, but for reference, I'll describe all the parameters here. Most are optional, or have default values. Those that are always required are marked accordingly. The file should be edited in a plain ASCII text editor (e.g. nano or leafpad on Raspberry Pi, notepad on windows.)
  
o map_type <0|1|2>
  0 - Single image file for map, displayed double size (default)
  1 - Tiled map, displayed normal size.
  2 - Single image file for map, displayed normal size
    
o display_map <filename> only png format supported. REQUIRED
  Image to display, or images of all the tiles, arranged with eight tiles per row. 
  If this is a single image, it must be an exact multiple of 32 pixels wide (e.g. 32*20 = 640. 32*25=800). 
  Maximum size is 1440x512 pixels.
  If tiles, each tile must be 64x64 pixels, so the whole image must be (8*64) x (n*64) pixels, where n=number of tiles/8. The first (i.e. top-left) tile must be entirely empty space, i.e. contain no objects that the ship could collide with.

o background <filename> png or jpg format supported.
  For a type 1 (tiled) map, this should specify a 128x128 image which will be tiled and used as a scrolling 'parallax' background.
  For a type 2 map, the background can be any size.
  
o bg_fade <value>
  Specifying this enables background fading. This means that the background will be invisible at the top of the map, and become more visible as you go down. The value is the distance from the top (in pixels) where the background starts to appear.
  
o collision_map <filename> only png format supported. REQUIRED
  The same as display_map, but all empty space MUST be 'magic pink' (i.e. R=255, G=0, B=255). 
  For map type 0, this should be the same size as the display map.
  For map type 1 (tiled), this should be HALF-SIZE, so each tile must be 32x32 pixels, and the whole image must be (8*32) x (n*32) pixels.
  For map type 2, this should be HALF-SIZE (i.e. half the size of the display map)
  
o ascii_map <filename> 
  Shows the arrangement of tiles to make the map. ASCII format, 0-9 for first 10 tiles, A-Z for next 26. ' ' (space) doubles as 0. Only applies if map_type = 1.
  
o sentry_display <filename> only png format supported.
  Images of all the sentries and forcefields, arranged in a single horizontal line. Each image must be 64x64 pixels, so the whole image must be (n*64) x 64 pixels.

o sentry_collision <filename> only png format supported.
  The same as sentry_display, but all empty space MUST be 'magic pink' (i.e. R=255, G=0, B=255). This should be HALF-SIZE, so each tile must be 32x32 pixels, and the whole image must be (n*32) x 32 pixels.
  
o description <filename> 
  Text displayed after selecting map.
  
o ship_first <0|1>
  0 - Map gets drawn first, then ship (also sentries, forcefields etc.) on top (default)
  1 - Ship (also sentries, forcefields etc.) get drawn first, then map on top.
  
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

o gravity <value>
  Default = 0.05
  
o drag <value>
  Air resistance. Default = 2

o pad <type> <y> <min x> <max x> <miners> <jewels>  REQUIRED (at least one)
  This describes a landing pad. Note that this does NOT draw graphics for the pad. These must be part of the display_map file. Maximum 12 pads allowed.
  <type> is a 16-bit *hexadecimal* number.
    Lowest nibble (digit) is the ship that this pad is 'home' for (0-3). Each ship always appears on its home pad, and will get shield, fuel and both types of ammo recharged when on it. If this digit is >3 then this pad will be a general pad, not home for any ship.
    Next nibble up determines which attributes will be recharged for *any* ship landing on this pad, mapped as follows:
    FUEL   0x0010
    AMMO1  0x0020
    AMMO2  0x0040
    SHIELD 0x0080
    e.g. if pad type is 10 then it is home for ship 0, and will recharge fuel for any ship that lands on it. If pad type is 9F then it's not home for any ship (F = 15, which is > 3) but will recharge fuel and shield for any ship (8+1 = 9)
  <y> is the y-coordinate of the pad. 0 is the top of the map, and it becomes more positive as you go down. The easiest way to find the y-coordinate is to start the game with the debug switch -d, try to land on the pad, and note the y-cordinate displayed in the status bar when you crash on the pad.
  <min x> and <max x> are the x coordinates of the left and right hand edges of the pad. If this is the home pad for a ship, it will appear half way in between these.
  <miners> and <jewels> are the number of miners stranded on this pad for the ship to rescue / number of jewels to be collected (both only applicable on 'mission' levels)
    
o blackhole <x> <y> <g>
  This describes a gravity source somewhere on the map. Ships will be pulled towards it from any direction. The force is stronger the closer you get. Again, this does NOT draw anything. Maximum 4 blackholes allowed.
  <x> <y> position of the blackhole
  <g> gravity. Typically 5

o sentry <x> <y> <direction> <type>(0/1/2) <period> <probability> <random> <range> <alive_sprite> <dead_sprite>
  Describes a sentry; a stationary object which fires bullets. This DOES draw the sentry, using a sprite taken from the sentry_display file defined earlier. Maximum 30 sentries allowed.
  
  <x> <y> position of the sentry. 
  
  <direction> direction of fire. 0-39 (9 degree increments). 0 is straight up, 10 is right, 20 is straight down etc. Doesn't apply to 'targeted' sentries.
  
  <type>(0/1/2)> 0 is 'normal' damage = 50
                 1 is 'targeted' (i.e. aims at nearest ship) damage = 50
                 2 is 'volcano' (lava coloured bullets) damage = 10
  
  <period> <probability> every <period> frames, the sentry has <probability> of firing. Each frame is 1/30th of a second.
    e.g. if period is 30 and probability is 50, then each secondd, the sentry has a 50% chance of firing.
  
  <random> amount of randomness applied to each shot. Typically 0-20.
  
  <range> distance at which targeted sentries will start firing.
  
  <alive_sprite> <dead_sprite> index into sentry_display file to define image to be drawn for when the sentry is active / destroyed. 0 for first 64x64 image, 1 for next image etc.

o forcefield <min_x> <max_x> <min_y> <max_y> <strength> <switch> <alive_sprite> <dead_sprite>
  <min_x> <max_x> <min_y> <max_y> define endpoints of forcefield. Forcefield must be either horizontal or vertical, i.e. EITHER min_x = max_x (vertical) OR min_y = max_y (horizontal). Maximum 5 forcefields allowed.
    
  <strength> force of repulsion. Typically 1000
  
  <switch> If this switch is hit with a bullet, the forcefield is deactivated. Switches are numbered in the order they are defined in this file.
  
  <alive_sprite> <dead_sprite> index into sentry_display file to define image to be drawn for when the forcefield is active / deactivated. 0 for first 64x64 image, 1 for next image etc.

o switch <x> <y> <closed sprite> <open sprite> <open time>
  creates a switch to open a forcefield.

  <x> <y> position

  <closed sprite> <open sprite> index into sentry_display file to define image to be drawn for when the switch is closed / open. 0 for first 64x64 image, 1 for next image etc.
  
  <open time> forcefield will stay open for this many frames before closing again. Each frame is 1/30th of a second. 

o area <min_x> <max_x> <min_y> <max_y> <gravity> <drag>
  defines a rectangular area where gravity and/or drag are different from the rest of the map. Maximum 4 areas allowed.
  
  <min_x> <max_x> <min_y> <max_y> define opposite corners of the rectangle
  
  <gravity> <drag> as per normal definitions for the level.

o race <min_x> <max_x> <min_y> <max_y> <reverse>
  defines the start/finish and progress lines of the race.
  
  <min_x> <max_x> <min_y> <max_y> define endpoints of line. Line must be either horizontal or vertical, i.e. EITHER min_x = max_x (vertical) OR min_y = max_y (horizontal).
  
  'normal' horizontal lines are traversed right-left; 'normal' vertical lines are traversed top-bottom. If 'reverse' is set to 1, then this direction is reversed. The lines can be viewed in mapmaker; press G to turn grid on; racelines are shown as red/green lines - traversed red-green, and in the order they're declared in the map file. 

There is a separate map maker program. This is unimaginatively called 'mapmaker'. It reads the same .txt file, which should be passed on the command line, e.g. 

mapmaker "mission 1.txt"
  
Quotes are required if there's a space in the filename. If you run it without a filename, it will immediately exit, so double clicking from Windows explorer won't work. In Windows, open a command window by clicking on the start button and typing cmd <return>, then navigate to the 'gravstorm' directory, and type the command line as above. 

Note that the Windows executable is called 'mapmaker.exe' ; the Raspberry Pi executable is just 'mapmaker'. You can delete whichever one you don't need.

Keyboard controls are:

- Cursor keys:       scroll left, right, up, down.
- PageUp / PageDown: zoom in and out
- Home:              return to centre of map, zoom level 1.0
- Q/A:               scroll tile preview on left
- Return:            reload .txt file
- S:		     save modified ASCII map file. 
- G:                 toggle grid off/white/grey/black. 
- Escape:            exit 

N.B. Grid is only shown for tiled maps (i.e. type 1), However, for all map types, 'G' also turns on colouring for pads (red bar), black holes (cyan circle), racelines (red/green) and altered gravity/drag areas (yellow).

The mouse can be used to edit tiled maps as follows:
- Left click and release to 'pick up' a tile (either from the map, or the preview area on the left)
  When a tile is picked up, the mouse cursor changes from an arrow to a hand.
- Once a tile is picked up, left click and release to place a new copy of it in the map, or left click and drag to 'paint' a series of tiles. 
- Right click and release to 'drop' the tile. The cursor will change back to an arrow.
- To erase, either right click and release while no tile is picked up, or just paint over with 'empty space' (tile zero) N.B. You can tell whether you have an empty tile picked up, or no tile by seeing if the mouse cursor is an arrow or a hand.
  
Alternatively, you can edit the ascii map file in a text editor, and reload it using <return>.

LEGAL STUFF
-----------
GravStorm Copyright (C) 2015-2017  Ian Lewis
This program and its source code are released under the GPL.
This program comes with ABSOLUTELY NO WARRANTY.
This is free software, and you are welcome to redistribute it
under certain conditions. See gpl.txt for details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

IPL 10/02/17
gravstorm9@gmail.com
