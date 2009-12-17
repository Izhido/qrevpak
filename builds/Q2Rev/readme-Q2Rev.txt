Q2Rev: A port of the original id Tech 2 engine (Quake II) to the Nintendo Wii, compiled using devkitPPC / libogc.
Release 1



The engine is almost feature-complete, with sound & network play. The only missing functionality is the CTF (capture-the-flag) module that was added later into the engine.



All of the improvements applied to Q1Rev Release 2 were included in Q2Rev. These include the following:

- Improved Wii Remote handling (since Q1Rev Release 1). More stable, and more sensitive. A small wrist movement goes a long way. Adjust the sensitivity level with the new "Wii Remote speed" option, next to "Mouse speed" in Options in the Main Menu.
- New On-Screen Keyboard, with access to most (if not all) keys of the US keyboard.
- Support for the standard Gamecube controller (see below).
- Switching to "big stack" in key points in the code, allow for more stable, more smooth movement in the engine.
- The engine can be started from either the SD card slot in front of the Wii, or an USB memory stick plugged in any of the USB ports in the back of the console (an improvement since Q1Rev R1).
- Experimental, untested support for USB keyboard & mouse.



Key Assignment:

Wii Remote #1        Keyboard                                      
---------------------------------------------------
D-pad                Arrow keys (*)                                    
1                    ENTER                                         
2                    ESC
+                    1 --> 2 --> 3 --> ... --> 8 --> 1 --> ...
-                    8 --> 7 --> 6 --> ... --> 1 --> 8 --> ...
B                    Left Mouse Button (Fire, usually)
A                    Enable Wii Remote Look Mode (see below)
Home                 "F1" / "y" keys (see below)

Nunchuck #1          Keyboard                                      
---------------------------------------------------
Joystick             Walk / Sidestep (see below)
Z                    Space Bar (Jump, usually)
C                    "c" key / Enables or disables on-screen keyboard (see below)

Gamecube Controller #1      Keyboard                                      
---------------------------------------------------
Main analog stick    Walk / Sidestep (see below)
D-pad                Arrow keys (*)                                    
A                    ENTER                                         
B                    ESC
X                    1 --> 2 --> 3 --> ... --> 8 --> 1 --> ...
Y                    8 --> 7 --> 6 --> ... --> 1 --> 8 --> ...
Z                    "c" key (see below)
L                    Left Mouse Button (Fire, usually)
R                    Space Bar (Jump, usually)
Start/Select         "F1" / "y" keys (see below)
C-stick              Free Look Mode (see below)


(*) When viewed with the Wii Remote pointing to the screen

With these keys, it's possible to play the game using the default settings. 



Walk / Sidestep:

Both the joystick of the Nunchuck expansion connected to the Wii Remote #1, and the Main analog stick of the Gamecube controller, can be used to walk forward / backwards and sidestep to the left / right. This is independent of any 'bind' assignments made to the arrow keys on config.cfg.


Wii Remote Look Mode:

Point your Wii Remote to the screen. You will see a small dotted circle, a guide showing where your Wii Remote is pointing to. 

Now, enter the game, then press and hold 'A' while pointing to the screen. The guide will disappear, and the camera (the direction you're looking) will start responding to your movements with the Wii Remote, *relative* to whether the guide was just before disappearing.

Release 'A'. The camera will *stay* exactly where you left it, and then the guide will reappear.

Feel free to use this control scheme when playing the game. Again, this is independent of any bindings to the arrow keys.

If you feel the camera movement is not reaching far enough, increase the new "Wii Remote Speed" setting on the "Options" menu (or the "wmotespeed" setting on your config.cfg file). 


Free Look mode:

The C-stick of the Gamecube controller can be used to move your camera (the direction you're looking) mostly anywhere. Just move the stick in the direction you want to look; the camera will stay there until you move the stick again.


On-Screen Keyboard:

Press 'C' on the Nunchuck. You will see a white, transparent keyboard covering most of the screen. Move your Wii Remote guide (the dotted circle) over it. You will see keys on the keyboard being selected. By pressing 'A' on the Wiimote, the selected key will be "pressed" and sent to the engine as a normal key press. Release 'A', and the key will also be "released" on screen as well as a normal key "release" from an actual keyboard. Press 'C' again on the Nunchuk to hide the on-screen keyboard. NOTE: if you exit your game while the on-screen keyboard is enabled, you will see it again next time you play.

'c' key:

Press 'C' on the Nunchuk (yes, it's the same key as the On-Screen Keyboard), or the 'Z' button on the Gamecube controller to make the player 'crouch'. As long as any of these keys are held, the player will be down; release it and it will go up again. If you pressed 'C' on the Nunchuk, releasing the key will also bring the on-screen keyboard up. Just press 'C' again to hide the keyboard & continue playing. 

Home / Start-Select button:

When pressed, any of them will generate simultaneously "F1" and "y" keys for the engine. F1 brings up the mission brief window during gameplay; if you're on the "Quit" screen of the Main Menu, pressing these keys will end the game (because of the "y" key).




To run Q2Rev, please do the following:

1) Copy the contents of this archive into the *root* of your media card (or USB stick);
2) Copy the contents of the /baseq2/ folder AND subfolders from your original Quake II game to the /baseq2/ folder of your media card;
3) *Erase* the config.cfg from the /baseq2/ folder in your media card, that was copied from the original Quake II game folder. This is in order to let the engine create the original bind mappings, plus the new values added for this port.
4) Boot your Wii, then start the game using your favorite booting method. The game is in /apps/Q2Rev/boot.dol in your media card.

If you don't yet have access to the game, the demo can be obtained free of charge from Id software's web site at http://www.idsoftware.com/  . Install it, locate /baseq2/, follow steps above.




Any comments about the engine are welcome to heribertod@gmail.com.




Original source code (C) 1997-2001 Id Software, Inc.
Modifications (c) 2009 Heriberto Delgado.

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

All trademarks or registered trademarks used are properties of their respective owners.
