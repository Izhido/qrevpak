Q1Rev: A port of the original Quake engine to the Nintendo Wii, compiled using devkitPPC / libogc.


IMPORTANT: Q1Rev is now part of Quake Rev PAK.


The engine is feature-complete, with sound & network play. In that regard, it can be considered as a "unmodified" NetQuake client. Unfortunately, there are next to no servers that let you use unmodified NetQuake clients (as of this release, lab.ihoc.net is still active).



New features since Release 3 of Quake Rev PAK:

- GX-accelerated build is now available! Runs at 60fps, and looks really gorgeous. It can be loaded from the launcher, by selecting a checkbox below the game entries list.
- A few minor memory leaks, and a *big* one in the GX-accelerated build (coming from the original source code) were fixed. Also, stack memory handling was modified in many places so the engine becomes a little more stable.



Key Assignment:

Wii Remote #1            Keyboard                                      
---------------------------------------------------------------------------
D-pad                    Arrow keys (*)                                    
1                        ENTER                                         
2                        ESC
+                        1 --> 2 --> 3 --> ... --> 7 --> 1 --> ...
-                        7 --> 6 --> 5 --> ... --> 1 --> 7 --> ...
B                        Left Mouse Button (Fire, usually)
A                        Enable Wii Remote Look Mode (see below)
Home                     {Pause} / "y" key (see below)

Nunchuck #1              Keyboard                                      
---------------------------------------------------------------------------
Joystick                 Walk / Sidestep (see below)
Z                        Space Bar (Jump, usually)
C                        Enables / disables on-screen keyboard (see below)

Classic Controller #1    Keyboard                                      
---------------------------------------------------------------------------
D-pad                    Arrow keys
A                        ENTER                                         
B                        ESC
X                        1 --> 2 --> 3 --> ... --> 7 --> 1 --> ...
Y                        7 --> 6 --> 5 --> ... --> 1 --> 7 --> ...
L                        Left Mouse Button (Fire, usually)
R                        Space Bar (Jump, usually)
ZL                       Enables / disables on-screen keyboard (see below)
Home                     {Pause} / "y" key (see below)
Left Stick               Walk / Sidestep (see below)
Right Stick              Free Look Mode (see below)

Gamecube Controller #1   Keyboard                                      
---------------------------------------------------------------------------
Main analog stick        Walk / Sidestep (see below)
D-pad                    Arrow keys                                    
A                        ENTER                                         
B                        ESC
X                        1 --> 2 --> 3 --> ... --> 7 --> 1 --> ...
Y                        7 --> 6 --> 5 --> ... --> 1 --> 7 --> ...
L                        Left Mouse Button (Fire, usually)
R                        Space Bar (Jump, usually)
Start/Select             {Pause} / "y" key (see below)
C-stick                  Free Look Mode (see below)


(*) When viewed with the Wii Remote pointing to the screen

With these keys, it's possible to play the game using the default settings. 



Walk / Sidestep:

Either the joystick of the Nunchuck expansion or the Left stick of the Classic Controller connected to the Wii Remote #1, as well as the Main analog stick of the Gamecube controller, can be used to walk forward / backwards and sidestep to the left / right. This is independent of any 'bind' assignments made to the arrow keys on config.cfg.


Wii Remote Look Mode:

Point your Wii Remote to the screen. You will see a small dotted circle, a guide showing where your Wii Remote is pointing to. 

Now, enter the game, then press and hold 'A' while pointing to the screen. The guide will disappear, and the camera (the direction you're looking) will start responding to your movements with the Wii Remote, *relative* to whether the guide was just before disappearing.

Release 'A'. The camera will *stay* exactly where you left it, and then the guide will reappear.

Feel free to use this control scheme when playing the game. Again, this is independent of any bindings to the arrow keys.

If you feel the camera movement is not reaching far enough, increase the new "Wii Remote Speed" setting on the "Options" menu (or the "wmotespeed" setting on your config.cfg file). 

NOTE: If you do not want to press [A] to move the camera, go to Menu - Options, and check the "Invert Wiimote look bt" option. The Wii Remote will start responding immediately to your movements, *until* you press [A] (effectively "inverting" the look mode schema). 

IMPORTANT: The camera will *not* move as long as you have the on-screen keyboard enabled (see below).


Free Look mode:

The Left stick of the Classic Controller connected to the Wii Remote #1, or the C-stick of the Gamecube controller #1, can be used to move your camera (the direction you're looking) mostly anywhere. Just move the stick in the direction you want to look; the camera will stay there until you move the stick again.


On-Screen Keyboard:

Press [C] on the Nunchuck, or [ZL] in the Classic Controller. You will see a white, transparent keyboard covering most of the screen. Move your Wii Remote guide (the dotted circle) over it. You will see keys on the keyboard being selected. By pressing [A] on the Wiimote, the selected key will be "pressed" and sent to the engine as a normal key press. Release [A], and the key will also be "released" on screen as well as a normal key "release" from an actual keyboard. Press [C] again on the Nunchuk (or [ZL] on the Classic controller) to hide the on-screen keyboard. NOTE: if you exit your game while the on-screen keyboard is enabled, you will see it again next time you play.


Home / Start-Select button:

When pressed, both generate a PAUSE command to the engine. If, however, you press any of these buttons at the "Quit" screen on the Main Menu, it will emulate instead a press of the "y" key, ending the game. 




To run Q1Rev as a standalone application (which we do not recommend, please use Quake Rev PAK), please do the following:

1) Copy the contents of this archive into the *root* of your media card (or USB stick);
2) Copy the contents of the /ID1/ folder from your original Quake game (usually under C:\QUAKE\ for Windows machines) to the /ID1/ folder of your media card;
3) *Erase* the config.cfg from the /ID1/ folder in your media card, that was copied from the original Quake game folder. This is in order to let the engine create the original bind mappings, plus the new values added for this port.
4) Boot your Wii, then start the game using your favorite booting method. The game is in /apps/Q1Rev/boot.dol in your media card. IMPORTANT: If you use the Homebrew Channel, be sure to update it to the latest version available.

If you have not bought Quake yet... um, well, what the hell are you waiting for? Go buy it, and play it at least once in your PC, for goodness's sake! All done? Good. Now, locate the /ID1/ folder in your PC and follow the steps above.

If you don't yet have access to the game, the shareware episode can be obtained free of charge from Id software's web site at http://www.idsoftware.com/  . Install it, locate /ID1/, follow steps above.




Any comments about the engine are welcome to heribertod@gmail.com.




Original source code (C) 1996-1997 Id Software, Inc.
Modifications (c) 2009-2012 Heriberto Delgado.

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

All trademarks or registered trademarks used are properties of their respective owners.
