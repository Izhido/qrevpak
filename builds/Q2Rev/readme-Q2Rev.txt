Q2Rev: A port of the original id Tech 2 engine (Quake II) to the Nintendo Wii, compiled using devkitPPC / libogc.


IMPORTANT: Q2Rev is now part of Quake Rev PAK.


The engine is feature-complete, with sound, network play and CD Music track playback. For technical reasons, the Capture-The-Flag module is not included in this engine; you will need to play Q2CTFRev for that purpose.



New features since Release 2 of Quake Rev PAK:

- CD Music track playback is now available! 
- Key aliases. With the new "keyalias" command, you can map any key in your controller to any other one, letting you customize your controls any way you like.
- If a game crashes, and the engine is able to tell, it will create a "QRevPAK.err" file containing the date/time and the cause of the crash.
- The [+] and [-] buttons now move through your inventory, instead of going with fixed keys. Remember to press [1] ( = Enter key, if unaliased) to activate the selected item.
- The "c" / On-screen Keyboard combo button now behaves differently. The "c" key command (Crouch) is sent immediately; the On-Screen keyboard is shown upon key release. However, if the key is held for more than 0.5 seconds, the On-Screen keyboard won't appear at all. 



Key Assignment:

Wii Remote #1        Keyboard                                      
--------------------------------------------------------------------------------------
D-pad                    Arrow keys (*)                                    
1                        ENTER                                         
2                        ESC
+                        Next item in inventory
-                        Previous item in inventory
B                        Left Mouse Button (Fire, usually)
A                        Enable Wii Remote Look Mode (see below)
Home                     "F1" / "y" keys (see below)

Nunchuck #1              Keyboard                                      
--------------------------------------------------------------------------------------
Joystick                 Walk / Sidestep (see below)
Z                        Space Bar (Jump, usually)
C                        "c" key / Enables or disables on-screen keyboard, if held 
                                   less than 0.5 seconds

Classic Controller #1    Keyboard                                      
--------------------------------------------------------------------------------------
D-pad                    Arrow keys (*)                                    
A                        ENTER                                         
B                        ESC
X                        Next item in inventory
Y                        Previous item in inventory
L                        Left Mouse Button (Fire, usually)
R                        Space Bar (Jump, usually)
ZL                       Enables / disables on-screen keyboard (see below)
ZR                       "c" key (see below)
Start/Select             "F1" / "y" keys (see below)
Left Stick               Walk / Sidestep (see below)
Right Stick              Free Look Mode (see below)

Gamecube Controller #1   Keyboard                                      
--------------------------------------------------------------------------------------
Main analog stick        Walk / Sidestep (see below)
D-pad                    Arrow keys (*)                                    
A                        ENTER                                         
B                        ESC
X                        Next item in inventory
Y                        Previous item in inventory
Z                        "c" key (see below)
L                        Left Mouse Button (Fire, usually)
R                        Space Bar (Jump, usually)
Start/Select             "F1" / "y" keys (see below)
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

Press and release [C] on the Nunchuck, for less than 0.5 seconds, or [ZL] in the Classic Controller. You will see a white, transparent keyboard covering most of the screen. Move your Wii Remote guide (the dotted circle) over it. You will see keys on the keyboard being selected. By pressing [A] on the Wiimote, the selected key will be "pressed" and sent to the engine as a normal key press. Release [A], and the key will also be "released" on screen as well as a normal key "release" from an actual keyboard. Press [C] again on the Nunchuk (or [ZL] on the Classic controller) to hide the on-screen keyboard. NOTE: if you exit your game while the on-screen keyboard is enabled, you will see it again next time you play.


'c' key:

Press [C] on the Nunchuk (yes, it's the same key as the On-Screen Keyboard), the [Z] button on the Gamecube controller, or the [ZR] button on the Classic controller to make the player 'crouch'. As long as any of these keys are held, the player will be down; release it and it will go up again. If you pressed [C] on the Nunchuk, releasing the key (if done in less than 0.5 seconds) will also bring the on-screen keyboard up. Just press [C] again to hide the keyboard & continue playing. 


Home / Start-Select button:

When pressed, any of them will generate simultaneously "F1" and "y" keys for the engine. F1 brings up the mission brief window during gameplay; if you're on the "Quit" screen of the Main Menu, pressing these keys will end the game (because of the "y" key).




To run Q2Rev as a standalone application (which we do not recommend, please use Quake Rev PAK), please do the following:

1) Copy the contents of this archive into the *root* of your media card (or USB stick);
2) Copy the contents of the /baseq2/ folder AND subfolders from your original Quake II game to the /baseq2/ folder of your media card;
3) *Erase* the config.cfg from the /baseq2/ folder in your media card, that was copied from the original Quake II game folder. This is in order to let the engine create the original bind mappings, plus the new values added for this port.
4) Boot your Wii, then start the game using your favorite booting method. The game is in /apps/Q2Rev/boot.dol in your media card. IMPORTANT: If you use the Homebrew Channel, be sure to update it to the latest version available.

If you don't yet have access to the game, the demo can be obtained free of charge from Id software's web site at http://www.idsoftware.com/  . Install it, locate /baseq2/, follow steps above.




Any comments about the engine are welcome to heribertod@gmail.com.




Original source code (C) 1997-2001 Id Software, Inc.
Modifications (c) 2009-2012 Heriberto Delgado.

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

All trademarks or registered trademarks used are properties of their respective owners.
