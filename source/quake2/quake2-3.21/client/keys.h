/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

//
// these are the key numbers that should be passed to Key_Event
//
#define	K_TAB			9
#define	K_ENTER			13
#define	K_ESCAPE		27
#define	K_SPACE			32

// normal keys should be passed as lowercased ascii

#define	K_BACKSPACE		127
#define	K_UPARROW		128
#define	K_DOWNARROW		129
#define	K_LEFTARROW		130
#define	K_RIGHTARROW	131

#define	K_ALT			132
#define	K_CTRL			133
#define	K_SHIFT			134
#define	K_F1			135
#define	K_F2			136
#define	K_F3			137
#define	K_F4			138
#define	K_F5			139
#define	K_F6			140
#define	K_F7			141
#define	K_F8			142
#define	K_F9			143
#define	K_F10			144
#define	K_F11			145
#define	K_F12			146
#define	K_INS			147
#define	K_DEL			148
#define	K_PGDN			149
#define	K_PGUP			150
#define	K_HOME			151
#define	K_END			152

// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// New keys for controllers in the platform (part 1):
#define K_WMOTE_A				153
#define K_WMOTE_B				154
#define K_WMOTE_1				155
#define K_WMOTE_2				156
#define K_WMOTE_PLUS			157
#define K_WMOTE_MINUS			158
#define K_WMOTE_HOME			159
// <<< FIX

#define K_KP_HOME		160
#define K_KP_UPARROW	161
#define K_KP_PGUP		162
#define	K_KP_LEFTARROW	163
#define K_KP_5			164
#define K_KP_RIGHTARROW	165
#define K_KP_END		166
#define K_KP_DOWNARROW	167
#define K_KP_PGDN		168
#define	K_KP_ENTER		169
#define K_KP_INS   		170
#define	K_KP_DEL		171
#define K_KP_SLASH		172
#define K_KP_MINUS		173
#define K_KP_PLUS		174

// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// New keys for controllers in the platform (part 2):
#define K_WMOTE_UPARROW			175
#define K_WMOTE_LEFTARROW		176
#define K_WMOTE_RIGHTARROW		177
#define K_WMOTE_DOWNARROW		178
#define K_NUNCHUK_Z				179
#define K_NUNCHUK_C				180
#define K_CLASSIC_A				181
#define K_CLASSIC_B				182
#define K_CLASSIC_X				183
#define K_CLASSIC_Y				184
#define K_CLASSIC_L				185
#define K_CLASSIC_R				186
#define K_CLASSIC_ZL			187
#define K_CLASSIC_ZR			188
#define K_CLASSIC_PLUS			189
#define K_CLASSIC_MINUS			190
#define K_CLASSIC_HOME			191
#define K_CLASSIC_UPARROW		192
#define K_CLASSIC_LEFTARROW		193
#define K_CLASSIC_RIGHTARROW	194
#define K_CLASSIC_DOWNARROW		195
#define K_GCUBE_A				196
#define K_GCUBE_B				197
#define K_GCUBE_X				198
#define K_GCUBE_Y				199
// <<< FIX

#define K_PAUSE			255

//
// mouse buttons generate virtual keys
//
#define	K_MOUSE1		200
#define	K_MOUSE2		201
#define	K_MOUSE3		202

//
// joystick buttons
//
#define	K_JOY1			203
#define	K_JOY2			204
#define	K_JOY3			205
#define	K_JOY4			206

//
// aux keys are for multi-buttoned joysticks to generate so they can use
// the normal binding process
//
#define	K_AUX1			207
#define	K_AUX2			208
#define	K_AUX3			209
#define	K_AUX4			210
#define	K_AUX5			211
#define	K_AUX6			212
#define	K_AUX7			213
#define	K_AUX8			214
#define	K_AUX9			215
#define	K_AUX10			216
#define	K_AUX11			217
#define	K_AUX12			218
#define	K_AUX13			219
#define	K_AUX14			220
#define	K_AUX15			221
#define	K_AUX16			222
#define	K_AUX17			223
#define	K_AUX18			224
#define	K_AUX19			225
#define	K_AUX20			226
#define	K_AUX21			227
#define	K_AUX22			228
#define	K_AUX23			229
#define	K_AUX24			230
#define	K_AUX25			231
#define	K_AUX26			232
#define	K_AUX27			233
#define	K_AUX28			234
#define	K_AUX29			235
#define	K_AUX30			236
#define	K_AUX31			237
#define	K_AUX32			238

#define K_MWHEELDOWN	239
#define K_MWHEELUP		240

// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// New keys for controllers in the platform (part 3):
#define K_GCUBE_Z				241
#define K_GCUBE_L				242
#define K_GCUBE_R				243
#define K_GCUBE_START			244
#define K_GCUBE_UPARROW			245
#define K_GCUBE_LEFTARROW		246
#define K_GCUBE_RIGHTARROW		247
#define K_GCUBE_DOWNARROW		248
// <<< FIX

extern char		*keybindings[256];
extern	int		key_repeats[256];
// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// New key aliases list for the new controller keys for the platform:
extern	int		keyaliases[256];
// <<< FIX

extern	int	anykeydown;
extern char chat_buffer[];
extern	int chat_bufferlen;
extern	qboolean	chat_team;

void Key_Event (int key, qboolean down, unsigned time);
void Key_Init (void);
void Key_WriteBindings (FILE *f);
// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// New function for persisting defined key aliases:
void Key_WriteAliases (FILE *f);
// <<< FIX
void Key_SetBinding (int keynum, char *binding);
void Key_ClearStates (void);
int Key_GetKey (void);

