/*Copyright (C) 1997-2001 Id Software, Inc.

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
// sys_revol.c -- system driver for the Nintendo Wii using devkitPPC / libogc
// (based on sys_null.c)

#include <gccore.h>
#include <fat.h>
#include <unistd.h>
#include <wiiuse/wpad.h>
#include <ogc/lwp_watchdog.h>
#include <network.h>
#include <wiikeyboard/keyboard.h>
#include <ogc/usbmouse.h>
#include <sys/stat.h>
#include <dirent.h>
#include <malloc.h>
#define BOOL_IMPLEMENTED 1

#include "../qcommon/qcommon.h"
#include "../game/game.h"
#include "errno.h"
#include "../client/keys.h"
#include "osk_revol.h"
#include "Keys_dat.h"


#define SYS_FIFO_SIZE (256*1024)


int	curtime;

unsigned	sys_frame_time;


void* sys_framebuffer[3] = {NULL, NULL, NULL};

GXRModeObj* sys_rmode;

void* sys_gpfifo = NULL;

f32 sys_yscale;

u32 sys_xfbHeight;

u32 sys_currentframebuf;

int sys_previous_time;

int sys_netinit_error;

char sys_ipaddress_text[16];

char sys_basedir[MAX_QPATH + 1];

char sys_resolution_in_brackets[MAX_STRING_CHARS + 1];

char sys_resolution_description[MAX_STRING_CHARS + 1];

extern cvar_t* in_osk;

extern cvar_t* in_wlook;

u32 sys_previous_keys;

u32 sys_previous_pad_keys;

int sys_current_weapon;

byte *membase;

int maxhunksize;

int curhunksize;

// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// New data structures for big stack handling:
#define BIGSTACK_SIZE 2 * 1024 * 1024

byte sys_bigstack[BIGSTACK_SIZE];

int sys_bigstack_cursize;
// <<< FIX

s32 sys_mouse_valid;

mouse_event sys_mouse_event;

u8 sys_previous_mouse_buttons;


game_export_t *GetGameAPI (game_import_t *import);


void Sys_GetBaseDir(int argc, char** argv)
{
	int i;
	char* p;
	qboolean FirstFound;
	qboolean AllFound;

	if(argc == 0)
	{
		strcpy(sys_basedir, "/");
	} else 
	{
		p = argv[0];
		i = 0;
		FirstFound = false;
		AllFound = false;
		while(i < MAX_QPATH)
		{
			if(p[i] == 0)
			{
				break;
			} else if(p[i] == ':')
			{
				if(FirstFound)
				{
					break;
				} else
				{
					FirstFound = true;
				};
			} else if(p[i] == '/')
			{
				if(FirstFound)
				{
					AllFound = true;
				};
				break;
			};
			i++;
		};
		if(AllFound)
		{
			strncpy(sys_basedir, p, i + 1);
			sys_basedir[i + 1] = 0;
		} else
		{
			strcpy(sys_basedir, "/");
		};
	};
}

void Sys_GetResolutionTexts()
{
	sprintf(sys_resolution_in_brackets, "[%i %i  ]", sys_rmode->viWidth, 9 * sys_rmode->viHeight / 10);
	sprintf(sys_resolution_description, "Detected Mode: %ix%i", sys_rmode->viWidth, 9 * sys_rmode->viHeight / 10); 
}

void Sys_Error (char *error, ...)
{
	va_list		argptr;

	printf ("Sys_Error: ");	
	va_start (argptr,error);
	vprintf (error,argptr);
	va_end (argptr);
	printf ("\n");
	printf ("\n");
	printf ("\n");
	printf ("\n");
	printf ("\n");
	printf ("\n");
	printf ("\n");
	printf ("\n");

	exit (1);
}

void Sys_Quit (void)
{
	exit (0);
}

void	Sys_UnloadGame (void)
{
}

void	*Sys_GetGameAPI (void *parms)
{
	return GetGameAPI(parms);
}

char *Sys_ConsoleInput (void)
{
	return NULL;
}

void	Sys_ConsoleOutput (char *string)
{
	printf(string);
}

void Sys_KeyPress(char c)
{
	// Do nothing; keys are being polled straight from the keyboard
}

char Sys_FindKey(u16 symbol)
{
	if((symbol >= 32)&&(symbol <= 127))
	{
		return (char)symbol;
	} else if(symbol == KS_Tab)
	{
		return K_TAB;
	} else if(symbol == KS_Return)
	{
		return K_ENTER;
	} else if(symbol == KS_Escape)
	{
		return K_ESCAPE;
	} else if(symbol == KS_BackSpace)
	{
		return K_BACKSPACE;
	} else if(symbol == KS_Up)
	{
		return K_UPARROW;
	} else if(symbol == KS_Down)
	{
		return K_DOWNARROW;
	} else if(symbol == KS_Left)
	{
		return K_LEFTARROW;
	} else if(symbol == KS_Right)
	{
		return K_RIGHTARROW;
	} else if((symbol == KS_Alt_L)||(symbol == KS_Alt_R))
	{
		return K_ALT;
	} else if((symbol == KS_Control_L)||(symbol == KS_Control_R))
	{
		return K_CTRL;
	} else if((symbol == KS_Shift_L)||(symbol == KS_Shift_R))
	{
		return K_SHIFT;
	} else if((symbol == KS_f1)||(symbol == KS_F1))
	{
		return K_F1;
	} else if((symbol == KS_f2)||(symbol == KS_F2))
	{
		return K_F2;
	} else if((symbol == KS_f3)||(symbol == KS_F3))
	{
		return K_F3;
	} else if((symbol == KS_f4)||(symbol == KS_F4))
	{
		return K_F4;
	} else if((symbol == KS_f5)||(symbol == KS_F5))
	{
		return K_F5;
	} else if((symbol == KS_f6)||(symbol == KS_F6))
	{
		return K_F6;
	} else if((symbol == KS_f7)||(symbol == KS_F7))
	{
		return K_F7;
	} else if((symbol == KS_f8)||(symbol == KS_F8))
	{
		return K_F8;
	} else if((symbol == KS_f9)||(symbol == KS_F9))
	{
		return K_F9;
	} else if((symbol == KS_f10)||(symbol == KS_F10))
	{
		return K_F10;
	} else if((symbol == KS_f11)||(symbol == KS_F11))
	{
		return K_F11;
	} else if((symbol == KS_f12)||(symbol == KS_F12))
	{
		return K_F12;
	} else if(symbol == KS_Insert)
	{
		return K_INS;
	} else if(symbol == KS_Delete)
	{
		return K_DEL;
	} else if(symbol == KS_Next)
	{
		return K_PGDN;
	} else if(symbol == KS_Prior)
	{
		return K_PGUP;
	} else if(symbol == KS_Home)
	{
		return K_HOME;
	} else if(symbol == KS_End)
	{
		return K_END;
	} else if(symbol == KS_KP_Home)
	{
		return K_KP_HOME;
	} else if(symbol == KS_KP_Up)
	{
		return K_KP_UPARROW;
	} else if(symbol == KS_KP_Prior)
	{
		return K_KP_PGUP;
	} else if(symbol == KS_KP_Left)
	{
		return K_KP_LEFTARROW;
	} else if(symbol == KS_KP_Right)
	{
		return K_KP_RIGHTARROW;
	} else if(symbol == KS_KP_End)
	{
		return K_KP_END;
	} else if(symbol == KS_KP_Down)
	{
		return K_KP_DOWNARROW;
	} else if(symbol == KS_KP_Next)
	{
		return K_KP_PGDN;
	} else if(symbol == KS_KP_Enter)
	{
		return K_KP_ENTER;
	} else if(symbol == KS_KP_Insert)
	{
		return K_KP_INS;
	} else if(symbol == KS_KP_Delete)
	{
		return K_KP_DEL;
	} else if(symbol == KS_KP_Divide)
	{
		return K_KP_SLASH;
	} else if(symbol == KS_KP_Subtract)
	{
		return K_KP_MINUS;
	} else if(symbol == KS_KP_Add)
	{
		return K_KP_PLUS;
	} else if((symbol >= KS_KP_0)&&(symbol <= KS_KP_9))
	{
		return '0' + (symbol - KS_KP_0);
	} else if(symbol == KS_Pause)
	{
		return K_PAUSE;
	} else
	{
		return 0;
	};
}

void Sys_DefaultAliases (void)
{
	Cbuf_AddText("bind WLOOK wlook\n");
	Cbuf_AddText("bind OSK osk\n");
	Cbuf_AddText("bind INVPREV invprev\n");
	Cbuf_AddText("bind INVNEXT invnext\n");
	Cbuf_AddText("keyalias WMOTE_A WLOOK\n");
	Cbuf_AddText("keyalias WMOTE_UPARROW UPARROW\n");
	Cbuf_AddText("keyalias WMOTE_LEFTARROW LEFTARROW\n");
	Cbuf_AddText("keyalias WMOTE_RIGHTARROW RIGHTARROW\n");
	Cbuf_AddText("keyalias WMOTE_DOWNARROW DOWNARROW\n");
	Cbuf_AddText("keyalias WMOTE_1 ENTER\n");
	Cbuf_AddText("keyalias WMOTE_2 ESCAPE\n");
	Cbuf_AddText("keyalias WMOTE_PLUS INVNEXT\n");
	Cbuf_AddText("keyalias WMOTE_MINUS INVPREV\n");
	Cbuf_AddText("keyalias WMOTE_B MOUSE1\n");
	Cbuf_AddText("keyalias WMOTE_HOME F1_Y\n");
	Cbuf_AddText("keyalias NUNCHUK_Z SPACE\n");
	Cbuf_AddText("keyalias NUNCHUK_C OSK_C\n");
	Cbuf_AddText("keyalias CLASSIC_UPARROW UPARROW\n");
	Cbuf_AddText("keyalias CLASSIC_LEFTARROW LEFTARROW\n");
	Cbuf_AddText("keyalias CLASSIC_RIGHTARROW RIGHTARROW\n");
	Cbuf_AddText("keyalias CLASSIC_DOWNARROW DOWNARROW\n");
	Cbuf_AddText("keyalias CLASSIC_A ENTER\n");
	Cbuf_AddText("keyalias CLASSIC_B ESCAPE\n");
	Cbuf_AddText("keyalias CLASSIC_X INVNEXT\n");
	Cbuf_AddText("keyalias CLASSIC_Y INVPREV\n");
	Cbuf_AddText("keyalias CLASSIC_L MOUSE1\n");
	Cbuf_AddText("keyalias CLASSIC_R SPACE\n");
	Cbuf_AddText("keyalias CLASSIC_ZL OSK\n");
	Cbuf_AddText("keyalias CLASSIC_ZR c\n");
	Cbuf_AddText("keyalias CLASSIC_HOME F1_Y\n");
	Cbuf_AddText("keyalias GCUBE_UPARROW UPARROW\n");
	Cbuf_AddText("keyalias GCUBE_LEFTARROW LEFTARROW\n");
	Cbuf_AddText("keyalias GCUBE_RIGHTARROW RIGHTARROW\n");
	Cbuf_AddText("keyalias GCUBE_DOWNARROW DOWNARROW\n");
	Cbuf_AddText("keyalias GCUBE_A ENTER\n");
	Cbuf_AddText("keyalias GCUBE_B ESCAPE\n");
	Cbuf_AddText("keyalias GCUBE_X INVNEXT\n");
	Cbuf_AddText("keyalias GCUBE_Y INVPREV\n");
	Cbuf_AddText("keyalias GCUBE_Z c\n");
	Cbuf_AddText("keyalias GCUBE_L MOUSE1\n");
	Cbuf_AddText("keyalias GCUBE_R SPACE\n");
	Cbuf_AddText("keyalias GCUBE_START F1_Y\n");
	key_alias_invoked = true;
}

void Sys_SendKeyEvents (void)
{
	u32 k;
	u32 g;
	keyboard_event e;
	expansion_t ex;
	int osk_key;

	if(!key_alias_invoked)
		Sys_DefaultAliases();
	if((sys_previous_mouse_buttons & 0x01) != (sys_mouse_event.button & 0x01))
	{
		Key_Event(K_MOUSE1, ((sys_mouse_event.button & 0x01) == 0x01), Sys_Milliseconds());
	};
	if((sys_previous_mouse_buttons & 0x02) != (sys_mouse_event.button & 0x02))
	{
		Key_Event(K_MOUSE2, ((sys_mouse_event.button & 0x02) == 0x02), Sys_Milliseconds());
	};
	if((sys_previous_mouse_buttons & 0x04) != (sys_mouse_event.button & 0x04))
	{
		Key_Event(K_MOUSE3, ((sys_mouse_event.button & 0x04) == 0x04), Sys_Milliseconds());
	};
	sys_previous_mouse_buttons = sys_mouse_event.button;
	WPAD_ScanPads();
	k = WPAD_ButtonsHeld(WPAD_CHAN_0);
	WPAD_Expansion(WPAD_CHAN_0, &ex);
	if((sys_previous_keys & WPAD_BUTTON_1) != (k & WPAD_BUTTON_1))
	{
		Key_Event(K_ENTER, ((k & WPAD_BUTTON_1) == WPAD_BUTTON_1), Sys_Milliseconds());
	};
	if((sys_previous_keys & WPAD_BUTTON_2) != (k & WPAD_BUTTON_2))
	{
		Key_Event(K_ESCAPE, ((k & WPAD_BUTTON_2) == WPAD_BUTTON_2), Sys_Milliseconds());
	};
	if((sys_previous_keys & WPAD_BUTTON_PLUS) != (k & WPAD_BUTTON_PLUS))
	{
		if(((k & WPAD_BUTTON_PLUS) == WPAD_BUTTON_PLUS)&&(sys_current_weapon < 9))
		{
			sys_current_weapon++;
		};
		Key_Event('0' + sys_current_weapon, ((k & WPAD_BUTTON_PLUS) == WPAD_BUTTON_PLUS), Sys_Milliseconds());
	};
	if((sys_previous_keys & WPAD_BUTTON_MINUS) != (k & WPAD_BUTTON_MINUS))
	{
		if(((k & WPAD_BUTTON_MINUS) == WPAD_BUTTON_MINUS)&&(sys_current_weapon > 0))
		{
			sys_current_weapon--;
		};
		Key_Event('0' + sys_current_weapon, ((k & WPAD_BUTTON_MINUS) == WPAD_BUTTON_MINUS), Sys_Milliseconds());
	};
	if((sys_previous_keys & WPAD_BUTTON_A) != (k & WPAD_BUTTON_A))
	{
		if((k & WPAD_BUTTON_A) == WPAD_BUTTON_A)
		{
			Cvar_SetValue("in_wlook", 1);
		} else
		{
			Cvar_SetValue("in_wlook", 0);
		};
	};
	if((sys_previous_keys & WPAD_BUTTON_B) != (k & WPAD_BUTTON_B))
	{
		Key_Event(K_MOUSE1, ((k & WPAD_BUTTON_B) == WPAD_BUTTON_B), Sys_Milliseconds());
	};
	if((sys_previous_keys & WPAD_BUTTON_UP) != (k & WPAD_BUTTON_UP))
	{
		Key_Event(K_UPARROW, ((k & WPAD_BUTTON_UP) == WPAD_BUTTON_UP), Sys_Milliseconds());
	};
	if((sys_previous_keys & WPAD_BUTTON_DOWN) != (k & WPAD_BUTTON_DOWN))
	{
		Key_Event(K_DOWNARROW, ((k & WPAD_BUTTON_DOWN) == WPAD_BUTTON_DOWN), Sys_Milliseconds());
	};
	if((sys_previous_keys & WPAD_BUTTON_LEFT) != (k & WPAD_BUTTON_LEFT))
	{
		Key_Event(K_LEFTARROW, ((k & WPAD_BUTTON_LEFT) == WPAD_BUTTON_LEFT), Sys_Milliseconds());
	};
	if((sys_previous_keys & WPAD_BUTTON_RIGHT) != (k & WPAD_BUTTON_RIGHT))
	{
		Key_Event(K_RIGHTARROW, ((k & WPAD_BUTTON_RIGHT) == WPAD_BUTTON_RIGHT), Sys_Milliseconds());
	};
	if((sys_previous_keys & WPAD_BUTTON_HOME) != (k & WPAD_BUTTON_HOME))
	{
		Key_Event('y', ((k & WPAD_BUTTON_HOME) == WPAD_BUTTON_HOME), Sys_Milliseconds());
		Key_Event(K_F1, ((k & WPAD_BUTTON_HOME) == WPAD_BUTTON_HOME), Sys_Milliseconds());
	};
	if((sys_previous_keys & WPAD_BUTTON_A) != (k & WPAD_BUTTON_A))
	{
		if(in_osk->value)
		{
			osk_key = OSK_HandleKeys((k & WPAD_BUTTON_A) == WPAD_BUTTON_A);
			if(osk_key >= 0)
			{
				Key_Event(osk_key, ((k & WPAD_BUTTON_A) == WPAD_BUTTON_A), Sys_Milliseconds());
			};
		};
	};
	if(ex.type == WPAD_EXP_NUNCHUK)
	{
		if((sys_previous_keys & WPAD_NUNCHUK_BUTTON_Z) != (k & WPAD_NUNCHUK_BUTTON_Z))
		{
			Key_Event(' ', ((k & WPAD_NUNCHUK_BUTTON_Z) == WPAD_NUNCHUK_BUTTON_Z), Sys_Milliseconds());
		};
		if((sys_previous_keys & WPAD_NUNCHUK_BUTTON_C) != (k & WPAD_NUNCHUK_BUTTON_C))
		{
			Key_Event('c', ((k & WPAD_NUNCHUK_BUTTON_C) == WPAD_NUNCHUK_BUTTON_C), Sys_Milliseconds());
			if((sys_previous_keys & WPAD_NUNCHUK_BUTTON_C) == WPAD_NUNCHUK_BUTTON_C)
			{
				if(in_osk->value)
				{
					Cvar_SetValue("in_osk", 0);
				} else
				{
					Cvar_SetValue("in_osk", 1);
				};
			};
		};
	};
	if(ex.type == WPAD_EXP_CLASSIC)
	{
		if((sys_previous_keys & WPAD_CLASSIC_BUTTON_A) != (k & WPAD_CLASSIC_BUTTON_A))
		{
			Key_Event(K_ENTER, ((k & WPAD_CLASSIC_BUTTON_A) == WPAD_CLASSIC_BUTTON_A), Sys_Milliseconds());
		};
		if((sys_previous_keys & WPAD_CLASSIC_BUTTON_B) != (k & WPAD_CLASSIC_BUTTON_B))
		{
			Key_Event(K_ESCAPE, ((k & WPAD_CLASSIC_BUTTON_B) == WPAD_CLASSIC_BUTTON_B), Sys_Milliseconds());
		};
		if((sys_previous_keys & WPAD_CLASSIC_BUTTON_X) != (k & WPAD_CLASSIC_BUTTON_X))
		{
			if(((k & WPAD_CLASSIC_BUTTON_X) == WPAD_CLASSIC_BUTTON_X)&&(sys_current_weapon < 8))
			{
				sys_current_weapon++;
			};
			Key_Event('0' + sys_current_weapon, ((k & WPAD_CLASSIC_BUTTON_X) == WPAD_CLASSIC_BUTTON_X), Sys_Milliseconds());
		};
		if((sys_previous_keys & WPAD_CLASSIC_BUTTON_Y) != (k & WPAD_CLASSIC_BUTTON_Y))
		{
			if(((k & WPAD_CLASSIC_BUTTON_Y) == WPAD_CLASSIC_BUTTON_Y)&&(sys_current_weapon > 0))
			{
				sys_current_weapon--;
			};
			Key_Event('0' + sys_current_weapon, ((k & WPAD_CLASSIC_BUTTON_Y) == WPAD_CLASSIC_BUTTON_Y), Sys_Milliseconds());
		};
		if((sys_previous_keys & WPAD_CLASSIC_BUTTON_FULL_L) != (k & WPAD_CLASSIC_BUTTON_FULL_L))
		{
			Key_Event(K_MOUSE1, ((k & WPAD_CLASSIC_BUTTON_FULL_L) == WPAD_CLASSIC_BUTTON_FULL_L), Sys_Milliseconds());
		};
		if((sys_previous_keys & WPAD_CLASSIC_BUTTON_FULL_R) != (k & WPAD_CLASSIC_BUTTON_FULL_R))
		{
			Key_Event(' ', ((k & WPAD_CLASSIC_BUTTON_FULL_R) == WPAD_CLASSIC_BUTTON_FULL_R), Sys_Milliseconds());
		};
		if((sys_previous_keys & WPAD_CLASSIC_BUTTON_ZL) != (k & WPAD_CLASSIC_BUTTON_ZL))
		{
			if((k & WPAD_CLASSIC_BUTTON_ZL) == WPAD_CLASSIC_BUTTON_ZL)
			{
				if(in_osk->value)
				{
					Cvar_SetValue("in_osk", 0);
				} else
				{
					Cvar_SetValue("in_osk", 1);
				};
			};
		};
		if((sys_previous_keys & WPAD_CLASSIC_BUTTON_ZR) != (k & WPAD_CLASSIC_BUTTON_ZR))
		{
			Key_Event('c', ((k & WPAD_CLASSIC_BUTTON_ZR) == WPAD_CLASSIC_BUTTON_ZR), Sys_Milliseconds());
		};
		if((sys_previous_keys & WPAD_CLASSIC_BUTTON_UP) != (k & WPAD_CLASSIC_BUTTON_UP))
		{
			Key_Event(K_UPARROW, ((k & WPAD_CLASSIC_BUTTON_UP) == WPAD_CLASSIC_BUTTON_UP), Sys_Milliseconds());
		};
		if((sys_previous_keys & WPAD_CLASSIC_BUTTON_DOWN) != (k & WPAD_CLASSIC_BUTTON_DOWN))
		{
			Key_Event(K_DOWNARROW, ((k & WPAD_CLASSIC_BUTTON_DOWN) == WPAD_CLASSIC_BUTTON_DOWN), Sys_Milliseconds());
		};
		if((sys_previous_keys & WPAD_CLASSIC_BUTTON_LEFT) != (k & WPAD_CLASSIC_BUTTON_LEFT))
		{
			Key_Event(K_LEFTARROW, ((k & WPAD_CLASSIC_BUTTON_LEFT) == WPAD_CLASSIC_BUTTON_LEFT), Sys_Milliseconds());
		};
		if((sys_previous_keys & WPAD_CLASSIC_BUTTON_RIGHT) != (k & WPAD_CLASSIC_BUTTON_RIGHT))
		{
			Key_Event(K_RIGHTARROW, ((k & WPAD_CLASSIC_BUTTON_RIGHT) == WPAD_CLASSIC_BUTTON_RIGHT), Sys_Milliseconds());
		};
		if((sys_previous_keys & WPAD_CLASSIC_BUTTON_HOME) != (k & WPAD_CLASSIC_BUTTON_HOME))
		{
			Key_Event('y', ((k & WPAD_CLASSIC_BUTTON_HOME) == WPAD_CLASSIC_BUTTON_HOME), Sys_Milliseconds());
			Key_Event(K_F1, ((k & WPAD_CLASSIC_BUTTON_HOME) == WPAD_CLASSIC_BUTTON_HOME), Sys_Milliseconds());
		};
	};
	sys_previous_keys = k;
	PAD_ScanPads();
	g = PAD_ButtonsHeld(0);
	if((sys_previous_pad_keys & PAD_BUTTON_A) != (g & PAD_BUTTON_A))
	{
		Key_Event(K_ENTER, ((g & PAD_BUTTON_A) == PAD_BUTTON_A), Sys_Milliseconds());
	};
	if((sys_previous_pad_keys & PAD_BUTTON_B) != (g & PAD_BUTTON_B))
	{
		Key_Event(K_ESCAPE, ((g & PAD_BUTTON_B) == PAD_BUTTON_B), Sys_Milliseconds());
	};
	if((sys_previous_pad_keys & PAD_BUTTON_X) != (g & PAD_BUTTON_X))
	{
		if(((g & PAD_BUTTON_X) == PAD_BUTTON_X)&&(sys_current_weapon < 9))
		{
			sys_current_weapon++;
		};
		Key_Event('0' + sys_current_weapon, ((g & PAD_BUTTON_X) == PAD_BUTTON_X), Sys_Milliseconds());
	};
	if((sys_previous_pad_keys & PAD_BUTTON_Y) != (g & PAD_BUTTON_Y))
	{
		if(((g & PAD_BUTTON_Y) == PAD_BUTTON_Y)&&(sys_current_weapon > 0))
		{
			sys_current_weapon--;
		};
		Key_Event('0' + sys_current_weapon, ((g & PAD_BUTTON_Y) == PAD_BUTTON_Y), Sys_Milliseconds());
	};
	if((sys_previous_pad_keys & PAD_TRIGGER_Z) != (g & PAD_TRIGGER_Z))
	{
		Key_Event('c', ((g & PAD_TRIGGER_Z) == PAD_TRIGGER_Z), Sys_Milliseconds());
	};
	if((sys_previous_pad_keys & PAD_TRIGGER_L) != (g & PAD_TRIGGER_L))
	{
		Key_Event(K_MOUSE1, ((g & PAD_TRIGGER_L) == PAD_TRIGGER_L), Sys_Milliseconds());
	};
	if((sys_previous_pad_keys & PAD_TRIGGER_R) != (g & PAD_TRIGGER_R))
	{
		Key_Event(' ', ((g & PAD_TRIGGER_R) == PAD_TRIGGER_R), Sys_Milliseconds());
	};
	if((sys_previous_pad_keys & PAD_BUTTON_UP) != (g & PAD_BUTTON_UP))
	{
		Key_Event(K_UPARROW, ((g & PAD_BUTTON_UP) == PAD_BUTTON_UP), Sys_Milliseconds());
	};
	if((sys_previous_pad_keys & PAD_BUTTON_DOWN) != (g & PAD_BUTTON_DOWN))
	{
		Key_Event(K_DOWNARROW, ((g & PAD_BUTTON_DOWN) == PAD_BUTTON_DOWN), Sys_Milliseconds());
	};
	if((sys_previous_pad_keys & PAD_BUTTON_LEFT) != (g & PAD_BUTTON_LEFT))
	{
		Key_Event(K_LEFTARROW, ((g & PAD_BUTTON_LEFT) == PAD_BUTTON_LEFT), Sys_Milliseconds());
	};
	if((sys_previous_pad_keys & PAD_BUTTON_RIGHT) != (g & PAD_BUTTON_RIGHT))
	{
		Key_Event(K_RIGHTARROW, ((g & PAD_BUTTON_RIGHT) == PAD_BUTTON_RIGHT), Sys_Milliseconds());
	};
	if((sys_previous_pad_keys & PAD_BUTTON_START) != (g & PAD_BUTTON_START))
	{
		Key_Event('y', ((g & PAD_BUTTON_START) == PAD_BUTTON_START), Sys_Milliseconds());
		Key_Event(K_F1, ((g & PAD_BUTTON_START) == PAD_BUTTON_START), Sys_Milliseconds());
	};
	sys_previous_pad_keys = g;
	while(KEYBOARD_GetEvent(&e))
    {
        if(e.type == KEYBOARD_PRESSED)
        {
			Key_Event(Sys_FindKey(e.symbol), true, Sys_Milliseconds());
        } else if(e.type == KEYBOARD_RELEASED)
        {
			Key_Event(Sys_FindKey(e.symbol), false, Sys_Milliseconds());
        };
    };
	// grab frame time 
	sys_frame_time = Sys_Milliseconds();
}

void Sys_AppActivate (void)
{
}

void Sys_CopyProtect (void)
{
}

char *Sys_GetClipboardData( void )
{
	return NULL;
}

void *Hunk_Begin (int maxsize)
{
	// reserve a huge chunk of memory, but don't commit any yet
	maxhunksize = maxsize;
	curhunksize = 0;
	membase = malloc(maxhunksize);
	if (membase == NULL)
		Sys_Error("unable to allocate %d bytes", maxsize);
	memset (membase, 0, maxsize);
	return membase;
}

void *Hunk_Alloc (int size)
{
	byte *buf;

	// round to cacheline
	size = (size+31)&~31;
	if (curhunksize + size > maxhunksize)
		Sys_Error("Hunk_Alloc overflow");
	buf = membase + curhunksize;
	curhunksize += size;
	return buf;
}

int Hunk_End (void)
{
	byte *n;

	n = realloc(membase, curhunksize);
	if (n != membase)
		Sys_Error("Hunk_End:  Could not remap virtual block (%d)", errno);
	
	return curhunksize;
}

void Hunk_Free (void *base)
{
	if (base)
		free(base);
}

int Sys_Milliseconds (void)
{
	static int		base;
	static qboolean	initialized = false;
	u64 t;
	int ms;

	t = gettime();
	ms = ticks_to_millisecs(t);
	if (!initialized)
	{
		base = ms;
		initialized = true;
	}
	curtime = ms - base;

	return curtime;
}

void	Sys_Mkdir (char *path)
{
	mkdir(path, 0777);
}

//============================================

static	char	findbase[MAX_OSPATH];
static	char	findpath[MAX_OSPATH];
static	char	findpattern[MAX_OSPATH];
static	DIR		*fdir;

int glob_match(char *pattern, char *text);

int glob_chars_caseless_match(char p, char t)
{
	if((p >= 'a')&&(p <= 'z'))
	{
		p = p - ('a' - 'A');
	};
	if((t >= 'a')&&(t <= 'z'))
	{
		t = t - ('a' - 'A');
	};
	return(p == t);
}

/* Like glob_match, but match PATTERN against any final segment of TEXT.  */
static int glob_match_after_star(char *pattern, char *text)
{
	register char *p = pattern, *t = text;
	register char c, c1;

	while ((c = *p++) == '?' || c == '*')
		if (c == '?' && *t++ == '\0')
			return 0;

	if (c == '\0')
		return 1;

	if (c == '\\')
		c1 = *p;
	else
		c1 = c;

	while (1) {
		if ((c == '[' || *t == c1) && glob_match(p - 1, t))
			return 1;
		if (*t++ == '\0')
			return 0;
	}
}

/* Match the pattern PATTERN against the string TEXT;
   return 1 if it matches, 0 otherwise.

   A match means the entire string TEXT is used up in matching.

   In the pattern string, `*' matches any sequence of characters,
   `?' matches any character, [SET] matches any character in the specified set,
   [!SET] matches any character not in the specified set.

   A set is composed of characters or ranges; a range looks like
   character hyphen character (as in 0-9 or A-Z).
   [0-9a-zA-Z_] is the set of characters allowed in C identifiers.
   Any other character in the pattern must be matched exactly.

   To suppress the special syntactic significance of any of `[]*?!-\',
   and match the character exactly, precede it with a `\'.
*/

int glob_match(char *pattern, char *text)
{
	register char *p = pattern, *t = text;
	register char c;

	while ((c = *p++) != '\0')
		switch (c) {
		case '?':
			if (*t == '\0')
				return 0;
			else
				++t;
			break;

		case '\\':
			if (!glob_chars_caseless_match(*p++,*t++))
				return 0;
			break;

		case '*':
			return glob_match_after_star(p, t);

		case '[':
			{
				register char c1 = *t++;
				int invert;

				if (!c1)
					return (0);

				invert = ((*p == '!') || (*p == '^'));
				if (invert)
					p++;

				c = *p++;
				while (1) {
					register char cstart = c, cend = c;

					if (c == '\\') {
						cstart = *p++;
						cend = cstart;
					}
					if (c == '\0')
						return 0;

					c = *p++;
					if (c == '-' && *p != ']') {
						cend = *p++;
						if (cend == '\\')
							cend = *p++;
						if (cend == '\0')
							return 0;
						c = *p++;
					}
					if (c1 >= cstart && c1 <= cend)
						goto match;
					if (c == ']')
						break;
				}
				if (!invert)
					return 0;
				break;

			  match:
				/* Skip the rest of the [...] construct that already matched.  */
				while (c != ']') {
					if (c == '\0')
						return 0;
					c = *p++;
					if (c == '\0')
						return 0;
					else if (c == '\\')
						++p;
				}
				if (invert)
					return 0;
				break;
			}

		default:
			if (!glob_chars_caseless_match(c, *t++))
				return 0;
		}

	return *t == '\0';
}

static qboolean CompareAttributes(char *path, char *name,
	unsigned musthave, unsigned canthave )
{
	struct stat st;
	char fn[MAX_OSPATH];

// . and .. never match
	if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
		return false;

	sprintf(fn, "%s/%s", path, name);
	if (stat(fn, &st) == -1)
		return false; // shouldn't happen

	if ( ( S_ISDIR(st.st_mode) ) && ( canthave & SFF_SUBDIR ) )
		return false;

	if ( ( musthave & SFF_SUBDIR ) && !( S_ISDIR(st.st_mode) ) )
		return false;

	return true;
}

char *Sys_FindFirst (char *path, unsigned musthave, unsigned canhave)
{
	struct dirent *d;
	char *p;

	if (fdir)
		Sys_Error ("Sys_BeginFind without close");

//	COM_FilePath (path, findbase);
	strcpy(findbase, path);

	if ((p = strrchr(findbase, '/')) != NULL) {
		*p = 0;
		strcpy(findpattern, p + 1);
	} else
		strcpy(findpattern, "*");

	if (strcmp(findpattern, "*.*") == 0)
		strcpy(findpattern, "*");
	
	if ((fdir = opendir(findbase)) == NULL)
		return NULL;
	while ((d = readdir(fdir)) != NULL) {
		if (!*findpattern || glob_match(findpattern, d->d_name)) {
//			if (*findpattern)
//				printf("%s matched %s\n", findpattern, d->d_name);
			if (CompareAttributes(findbase, d->d_name, musthave, canhave)) {
				sprintf (findpath, "%s/%s", findbase, d->d_name);
				return findpath;
			}
		}
	}
	return NULL;
}

char *Sys_FindNext (unsigned musthave, unsigned canhave)
{
	struct dirent *d;

	if (fdir == NULL)
		return NULL;
	while ((d = readdir(fdir)) != NULL) {
		if (!*findpattern || glob_match(findpattern, d->d_name)) {
//			if (*findpattern)
//				printf("%s matched %s\n", findpattern, d->d_name);
			if (CompareAttributes(findbase, d->d_name, musthave, canhave)) {
				sprintf (findpath, "%s/%s", findbase, d->d_name);
				return findpath;
			}
		}
	}
	return NULL;
}

void Sys_FindClose (void)
{
	if (fdir != NULL)
		closedir(fdir);
	fdir = NULL;
}


//============================================

void	Sys_Init (void)
{
}


//=============================================================================

// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// New functions for big stack handling:
void Sys_BigStackRewind(void)
{
	sys_bigstack_cursize = 0;
}

void* Sys_BigStackAlloc(int size, char* purpose)
{
	void* p;

	p = 0;
	if(sys_bigstack_cursize + size < BIGSTACK_SIZE)
	{
		p = sys_bigstack + sys_bigstack_cursize;
		sys_bigstack_cursize = sys_bigstack_cursize + size;
	} else
	{
		Sys_Error ("Sys_BigStackAlloc: %s - failed on %i bytes", purpose, size);
	};
	return p;
}

void Sys_BigStackFree(int size, char* purpose)
{
	if(sys_bigstack_cursize - size >= 0)
	{
		sys_bigstack_cursize = sys_bigstack_cursize - size;
	} else
	{
		Sys_Error ("Sys_BigStackFree: %s - underflow on %i bytes", purpose, sys_bigstack_cursize - size);
	};
}
// <<< FIX

void Sys_PowerOff(s32 chan)
{
	if(chan == WPAD_CHAN_0)
	{
		SYS_ResetSystem(SYS_POWEROFF, 0, 0);
	};
}

int main (int argc, char **argv)
{
	VIDEO_Init();
	WPAD_Init();
	PAD_Init();

	sys_rmode = VIDEO_GetPreferredMode(NULL);
	VIDEO_Configure(sys_rmode);

	sys_framebuffer[0] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(sys_rmode));
	sys_framebuffer[1] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(sys_rmode));
#ifdef GLIMP
	sys_framebuffer[2] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(sys_rmode));
#endif

	VIDEO_ClearFrameBuffer(sys_rmode, sys_framebuffer[0], COLOR_BLACK);

	CON_Init(sys_framebuffer[0], 20, 20, sys_rmode->fbWidth, sys_rmode->xfbHeight, sys_rmode->fbWidth * VI_DISPLAY_PIX_SZ);

#ifndef GLIMP
	VIDEO_ClearFrameBuffer(sys_rmode, sys_framebuffer[1], COLOR_BLACK);
#endif

	VIDEO_SetNextFramebuffer(sys_framebuffer[0]);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if(sys_rmode->viTVMode & VI_NON_INTERLACE)
	{
		VIDEO_WaitVSync();
	};

#ifdef GLIMP
	GXColor background = {0, 0, 0, 0xff};
	sys_currentframebuf = 1;
	sys_gpfifo = memalign(32, SYS_FIFO_SIZE);
	memset(sys_gpfifo, 0, SYS_FIFO_SIZE);
	GX_Init(sys_gpfifo, SYS_FIFO_SIZE);
	GX_SetCopyClear(background, 0x00ffffff);
	GX_SetViewport(0, 0, sys_rmode->fbWidth, sys_rmode->efbHeight, 0, 1);
	sys_yscale = GX_GetYScaleFactor(sys_rmode->efbHeight, sys_rmode->xfbHeight);
	sys_xfbHeight = GX_SetDispCopyYScale(sys_yscale);
	GX_SetScissor(0, 0, sys_rmode->fbWidth, sys_rmode->efbHeight);
	GX_SetDispCopySrc(0, 0, sys_rmode->fbWidth, sys_rmode->efbHeight);
	GX_SetDispCopyDst(sys_rmode->fbWidth, sys_xfbHeight);
	GX_SetCopyFilter(sys_rmode->aa, sys_rmode->sample_pattern, GX_TRUE, sys_rmode->vfilter);
	if(sys_rmode->viHeight == 2 * sys_rmode->xfbHeight)
	{
		GX_SetFieldMode(sys_rmode->field_rendering, GX_ENABLE);
	} else
	{
		GX_SetFieldMode(sys_rmode->field_rendering, GX_DISABLE);
	};
	if(sys_rmode->aa)
	{
        GX_SetPixelFmt(GX_PF_RGB565_Z16, GX_ZC_LINEAR);
	} else
	{
        GX_SetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);
	};
	GX_SetCullMode(GX_CULL_NONE);
	GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
	GX_SetAlphaUpdate(GX_ENABLE);
	GX_CopyDisp(sys_framebuffer[sys_currentframebuf], GX_TRUE);
	GX_SetDispCopyGamma(GX_GM_1_0);
	GX_ClearVtxDesc();
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
 	GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GX_SetVtxAttrFmt (GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
	GX_SetNumChans(1);
	GX_SetNumTexGens(0);
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLOR0A0);
	GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
#endif

	WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS_ACC_IR);
	WPAD_SetVRes(WPAD_CHAN_0, sys_rmode->fbWidth, sys_rmode->xfbHeight);

	if(!fatInitDefault())
	{
		Sys_Error("Filesystem not enabled");
	};

	Sys_GetBaseDir(argc, argv);

	Sys_GetResolutionTexts();

	sys_previous_time = Sys_Milliseconds();
	do 
	{
		sys_netinit_error = if_config(sys_ipaddress_text, NULL, NULL, true);
	} while((sys_netinit_error == -EAGAIN)&&((Sys_Milliseconds() - sys_previous_time) < 3000));
	if(sys_netinit_error < 0)
	{
		Sys_ConsoleOutput("Network not enabled\n");
	};

	if (KEYBOARD_Init(Sys_KeyPress) != 0)
	{
		Sys_ConsoleOutput("Keyboard not found\n");
	};

	OSK_LoadKeys(Keys_dat, Keys_dat_size);

	sys_current_weapon = 1;

	Qcommon_Init (argc, argv);

	WPAD_SetPowerButtonCallback(Sys_PowerOff);

#ifdef GLIMP
	VIDEO_SetNextFramebuffer(sys_framebuffer[sys_currentframebuf]);
#else
	VIDEO_SetNextFramebuffer(sys_framebuffer[1]);
#endif

	while (1)
	{
		if(MOUSE_IsConnected())
		{
			sys_mouse_valid = MOUSE_GetEvent(&sys_mouse_event);
			if(sys_mouse_valid)	MOUSE_FlushEvents();
		}
		else
		{
			sys_mouse_valid = 0;
			sys_mouse_event.button = 0;
		};
		Qcommon_Frame (1000 / 30);
#ifdef GLIMP
		GX_DrawDone();
		if(sys_currentframebuf == 1)
		{
			sys_currentframebuf = 2;
		} else
		{
			sys_currentframebuf = 1;
		};
		GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
		GX_SetColorUpdate(GX_TRUE);
		GX_CopyDisp(sys_framebuffer[sys_currentframebuf], GX_TRUE);
		VIDEO_SetNextFramebuffer(sys_framebuffer[sys_currentframebuf]);
#endif
		KEYBOARD_FlushEvents();
		VIDEO_Flush();
		VIDEO_WaitVSync();
	}

	return 0;
}


