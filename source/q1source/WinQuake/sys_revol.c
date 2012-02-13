/*
Copyright (C) 1996-1997 Id Software, Inc.

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

#include "quakedef.h"
#include "errno.h"

#include "osk_revol.h"
#include "Keys_dat.h"

#define SYS_FIFO_SIZE (256*1024)

qboolean isDedicated;

void* sys_framebuffer[2] = {NULL, NULL};

GXRModeObj* sys_rmode;

void* sys_gpfifo = NULL;

f32 sys_yscale;

u32 sys_xfbHeight;

GXColor sys_background_color = {0xFF, 0, 0, 0}; // Believe it or not, the "clear color" for GLQuake is red, not black...

u32 sys_currentframebuf;

int sys_current_weapon;

extern qboolean	scr_drawdialog;

extern qboolean m_state_is_quit;

extern cvar_t in_osk;

extern cvar_t in_wlook;

u32 sys_previous_keys;

u32 sys_previous_pad_keys;

int sys_previous_unaliased_key;

double sys_previous_time;

int sys_netinit_error;

char sys_ipaddress_text[16];

float sys_previous_char_time;

qboolean sys_current_char_count;

// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// New data structures for big stack handling:
#define BIGSTACK_SIZE 2 * 1024 * 1024

byte sys_bigstack[BIGSTACK_SIZE];

int sys_bigstack_cursize;
// <<< FIX

s32 sys_mouse_valid;

mouse_event sys_mouse_event;

u8 sys_previous_mouse_buttons;

float sys_frame_length;

/*
===============================================================================

FILE IO

===============================================================================
*/

#define MAX_HANDLES             10
FILE    *sys_handles[MAX_HANDLES];

int             findhandle (void)
{
	int             i;
	
	for (i=1 ; i<MAX_HANDLES ; i++)
		if (!sys_handles[i])
			return i;
	Sys_Error ("out of handles");
	return -1;
}

/*
================
filelength
================
*/
int filelength (FILE *f)
{
	int             pos;
	int             end;

	pos = ftell (f);
	fseek (f, 0, SEEK_END);
	end = ftell (f);
	fseek (f, pos, SEEK_SET);

	return end;
}

int Sys_FileOpenRead (char *path, int *hndl)
{
	FILE    *f;
	int             i;
	
	i = findhandle ();

	f = fopen(path, "rb");
	if (!f)
	{
		*hndl = -1;
		return -1;
	}
	sys_handles[i] = f;
	*hndl = i;
	
	return filelength(f);
}

int Sys_FileOpenWrite (char *path)
{
	FILE    *f;
	int             i;
	
	i = findhandle ();

	f = fopen(path, "wb");
	if (!f)
		Sys_Error ("Error opening %s: %s", path,strerror(errno));
	sys_handles[i] = f;
	
	return i;
}

void Sys_FileClose (int handle)
{
	fclose (sys_handles[handle]);
	sys_handles[handle] = NULL;
}

void Sys_FileSeek (int handle, int position)
{
	fseek (sys_handles[handle], position, SEEK_SET);
}

void Sys_FileSeekCur (int handle, int offset)
{
	fseek (sys_handles[handle], offset, SEEK_CUR);
}

int Sys_FileRead (int handle, void *dest, int count)
{
	return fread (dest, 1, count, sys_handles[handle]);
}

int Sys_FileWrite (int handle, void *data, int count)
{
	return fwrite (data, 1, count, sys_handles[handle]);
}

int     Sys_FileTime (char *path)
{
	struct	stat	buf;
	
	if (stat (path,&buf) == -1)
		return -1;
	
	return buf.st_mtime;
}

void Sys_mkdir (char *path)
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

/*
===============================================================================

SYSTEM IO

===============================================================================
*/

void Sys_MakeCodeWriteable (unsigned long startaddr, unsigned long length)
{
}


void Sys_Error (char *error, ...)
{
	va_list         argptr;
	FILE* f;
	time_t rawtime;
	struct tm * timeinfo;
	char stime[32];

	f= fopen("QRevPAK.err", "ab");
	if(f != NULL)
	{
		time(&rawtime);
		timeinfo = localtime (&rawtime);
		strftime(stime, 32, "%Y/%m/%d %H:%M:%S",timeinfo);
		fprintf(f, "%s : Sys_Error: ", stime);
		va_start(argptr, error);
		vfprintf(f, error, argptr);
		va_end(argptr);
		fprintf(f, "\n\n");
		fclose(f);
	};
	printf ("Sys_Error: ");   
	va_start (argptr,error);
	vprintf (error,argptr);
	va_end (argptr);
	printf ("\n");

	VIDEO_SetNextFramebuffer(sys_framebuffer[0]);
	VIDEO_WaitVSync();

    Host_Shutdown();

	exit (1);
}

void Sys_Printf (char *fmt, ...)
{
	va_list         argptr;
	
	va_start (argptr,fmt);
	vprintf (fmt,argptr);
	va_end (argptr);
}

void Sys_Quit (void)
{
	vrect_t	vr;

	// These were copied from the quit message of the Windows version:
	M_DrawTextBox (0, 0, 38, 27);
	M_PrintWhite (16, 12,  "  Quake version 1.09 by id Software\n\n");
	M_PrintWhite (16, 28,  "Programming        Art \n");
	M_Print (16, 36,  " John Carmack       Adrian Carmack\n");
	M_Print (16, 44,  " Michael Abrash     Kevin Cloud\n");
	M_Print (16, 52,  " John Cash          Paul Steed\n");
	M_Print (16, 60,  " Dave 'Zoid' Kirsch\n");
	M_PrintWhite (16, 68,  "Design             Biz\n");
	M_Print (16, 76,  " John Romero        Jay Wilbur\n");
	M_Print (16, 84,  " Sandy Petersen     Mike Wilson\n");
	M_Print (16, 92,  " American McGee     Donna Jackson\n");
	M_Print (16, 100,  " Tim Willits        Todd Hollenshead\n");
	M_PrintWhite (16, 108, "Support            Projects\n");
	M_Print (16, 116, " Barrett Alexander  Shawn Green\n");
	M_PrintWhite (16, 124, "Sound Effects\n");
	M_Print (16, 132, " Trent Reznor and Nine Inch Nails\n\n");
	M_PrintWhite (16, 140, "Quake is a trademark of Id Software,\n");
	M_PrintWhite (16, 148, "inc., (c)1996 Id Software, inc. All\n");
	M_PrintWhite (16, 156, "rights reserved. NIN logo is a\n");
	M_PrintWhite (16, 164, "registered trademark licensed to\n");
	M_PrintWhite (16, 172, "Nothing Interactive, Inc. All rights\n");
	M_PrintWhite (16, 180, "reserved.\n");
	M_PrintWhite (16, 196, "Modifications for Nintendo Wii using\n");
	M_PrintWhite (16, 204, "devkitPPC / libogc:\n");
	M_PrintWhite (16, 212, "(c) 2009, 2012 Heriberto Delgado.");
	vr.x = vr.y = 0;
	vr.width = vid.width;
	vr.height = vid.height;
	vr.pnext = NULL;
	VID_Update (&vr);

    Host_Shutdown();

	exit (0);
}

double Sys_FloatTime (void)
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
	return ((double)(ms - base)) / 1000.0;
}

char *Sys_ConsoleInput (void)
{
	return NULL;
}

void Sys_Sleep (void)
{
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
	} else if((symbol == KS_Return)||(symbol == KS_KP_Enter))
	{
		return K_ENTER;
	} else if(symbol == KS_Escape)
	{
		return K_ESCAPE;
	} else if(symbol == KS_BackSpace)
	{
		return K_BACKSPACE;
	} else if((symbol == KS_Up)||(symbol == KS_KP_Up))
	{
		return K_UPARROW;
	} else if((symbol == KS_Down)||(symbol == KS_KP_Down))
	{
		return K_DOWNARROW;
	} else if((symbol == KS_Left)||(symbol == KS_KP_Left))
	{
		return K_LEFTARROW;
	} else if((symbol == KS_Right)||(symbol == KS_KP_Right))
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
	} else if((symbol == KS_Insert)||(symbol == KS_KP_Insert))
	{
		return K_INS;
	} else if((symbol == KS_Delete)||(symbol == KS_KP_Delete))
	{
		return K_DEL;
	} else if((symbol == KS_Next)||(symbol == KS_KP_Next))
	{
		return K_PGDN;
	} else if((symbol == KS_Prior)||(symbol == KS_KP_Prior))
	{
		return K_PGUP;
	} else if((symbol == KS_Home)||(symbol == KS_KP_Home))
	{
		return K_HOME;
	} else if((symbol == KS_End)||(symbol == KS_KP_End))
	{
		return K_END;
	} else if(symbol == KS_KP_Divide)
	{
		return '/';
	} else if(symbol == KS_KP_Subtract)
	{
		return '-';
	} else if(symbol == KS_KP_Add)
	{
		return '+';
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
	Cbuf_AddText("keyalias WMOTE_HOME PAUSE_Y\n");
	Cbuf_AddText("keyalias NUNCHUK_Z SPACE\n");
	Cbuf_AddText("keyalias NUNCHUK_C OSK\n");
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
	Cbuf_AddText("keyalias CLASSIC_HOME PAUSE_Y\n");
	Cbuf_AddText("keyalias GCUBE_UPARROW UPARROW\n");
	Cbuf_AddText("keyalias GCUBE_LEFTARROW LEFTARROW\n");
	Cbuf_AddText("keyalias GCUBE_RIGHTARROW RIGHTARROW\n");
	Cbuf_AddText("keyalias GCUBE_DOWNARROW DOWNARROW\n");
	Cbuf_AddText("keyalias GCUBE_A ENTER\n");
	Cbuf_AddText("keyalias GCUBE_B ESCAPE\n");
	Cbuf_AddText("keyalias GCUBE_X INVNEXT\n");
	Cbuf_AddText("keyalias GCUBE_Y INVPREV\n");
	Cbuf_AddText("keyalias GCUBE_L MOUSE1\n");
	Cbuf_AddText("keyalias GCUBE_R SPACE\n");
	Cbuf_AddText("keyalias GCUBE_START PAUSE_Y\n");
	key_alias_invoked = true;
}

void Sys_HandleKey(int k, qboolean pressed)
{
	sys_previous_unaliased_key = k;
	k = keyaliases[k];
	if(k == K_INVNEXT)
	{
		if(pressed && (sys_current_weapon < 8))
		{
			sys_current_weapon++;
		};
		Key_Event('0' + sys_current_weapon, pressed);
	} else if(k == K_INVPREV)
	{
		if(pressed && (sys_current_weapon > 1))
		{
			sys_current_weapon--;
		};
		Key_Event('0' + sys_current_weapon, pressed);
	} else if(k == K_WLOOK)
	{
		if(pressed)
		{
			Cvar_SetValue("in_wlook", 1);
		} else
		{
			Cvar_SetValue("in_wlook", 0);
		};
	} else if(k == K_OSK)
	{
		if(pressed)
		{
			if(in_osk.value == 0)
			{
				Cvar_SetValue("in_osk", 1);
			} else
			{
				Cvar_SetValue("in_osk", 0);
			};
		};
	} else if(k == K_PAUSE_Y)
	{
		if(scr_drawdialog || m_state_is_quit)
		{
			Key_Event('y', pressed);
		} else if(m_state_is_quit)
		{
			Key_Event(K_PAUSE, pressed);
		};
	} else
	{
		Key_Event(k, pressed);
	};
}

void Sys_SendKeyEvents (void)
{
	ir_t p;
	u32 k;
	int osk_key;
	u32 g;
	keyboard_event e;
	expansion_t ex;

	if(!key_alias_invoked)
	{
		Sys_DefaultAliases();
	};
	if(in_osk.value != 0)
	{
		WPAD_IR(WPAD_CHAN_0, &p);
		if(p.valid)
		{
			osk_selected = OSK_KeyAt(p.x - ((sys_rmode->viWidth - OSK_WIDTH) / 2),
				                     p.y - ((sys_rmode->viHeight - OSK_HEIGHT) / 2));
		} else
		{
			osk_selected = 0;
		};
	};
	WPAD_ScanPads();
	k = WPAD_ButtonsHeld(WPAD_CHAN_0);
	if((sys_previous_keys & WPAD_BUTTON_A) != (k & WPAD_BUTTON_A))
	{
		if(in_osk.value != 0)
		{
			osk_key = OSK_HandleKeys((k & WPAD_BUTTON_A) == WPAD_BUTTON_A);
			if(osk_key >= 0)
			{
				Sys_HandleKey(osk_key, ((k & WPAD_BUTTON_A) == WPAD_BUTTON_A));
			};
		} else
		{
			Sys_HandleKey(K_WMOTE_A, ((k & WPAD_BUTTON_A) == WPAD_BUTTON_A));
		};
	};
	if((sys_previous_keys & WPAD_BUTTON_B) != (k & WPAD_BUTTON_B))
	{
		Sys_HandleKey(K_WMOTE_B, ((k & WPAD_BUTTON_B) == WPAD_BUTTON_B));
	};
	if((sys_previous_keys & WPAD_BUTTON_1) != (k & WPAD_BUTTON_1))
	{
		Sys_HandleKey(K_WMOTE_1, ((k & WPAD_BUTTON_1) == WPAD_BUTTON_1));
	};
	if((sys_previous_keys & WPAD_BUTTON_2) != (k & WPAD_BUTTON_2))
	{
		Sys_HandleKey(K_WMOTE_2, ((k & WPAD_BUTTON_2) == WPAD_BUTTON_2));
	};
	if((sys_previous_keys & WPAD_BUTTON_PLUS) != (k & WPAD_BUTTON_PLUS))
	{
		Sys_HandleKey(K_WMOTE_PLUS, ((k & WPAD_BUTTON_PLUS) == WPAD_BUTTON_PLUS));
	};
	if((sys_previous_keys & WPAD_BUTTON_MINUS) != (k & WPAD_BUTTON_MINUS))
	{
		Sys_HandleKey(K_WMOTE_MINUS, ((k & WPAD_BUTTON_MINUS) == WPAD_BUTTON_MINUS));
	};
	if((sys_previous_keys & WPAD_BUTTON_UP) != (k & WPAD_BUTTON_UP))
	{
		Sys_HandleKey(K_WMOTE_UPARROW, ((k & WPAD_BUTTON_UP) == WPAD_BUTTON_UP));
	};
	if((sys_previous_keys & WPAD_BUTTON_DOWN) != (k & WPAD_BUTTON_DOWN))
	{
		Sys_HandleKey(K_WMOTE_DOWNARROW, ((k & WPAD_BUTTON_DOWN) == WPAD_BUTTON_DOWN));
	};
	if((sys_previous_keys & WPAD_BUTTON_LEFT) != (k & WPAD_BUTTON_LEFT))
	{
		Sys_HandleKey(K_WMOTE_LEFTARROW, ((k & WPAD_BUTTON_LEFT) == WPAD_BUTTON_LEFT));
	};
	if((sys_previous_keys & WPAD_BUTTON_RIGHT) != (k & WPAD_BUTTON_RIGHT))
	{
		Sys_HandleKey(K_WMOTE_RIGHTARROW, ((k & WPAD_BUTTON_RIGHT) == WPAD_BUTTON_RIGHT));
	};
	if((sys_previous_keys & WPAD_BUTTON_HOME) != (k & WPAD_BUTTON_HOME))
	{
		Sys_HandleKey(K_WMOTE_HOME, ((k & WPAD_BUTTON_HOME) == WPAD_BUTTON_HOME));
	};
	WPAD_Expansion(WPAD_CHAN_0, &ex);
	if(ex.type == WPAD_EXP_NUNCHUK)
	{
		if((sys_previous_keys & WPAD_NUNCHUK_BUTTON_Z) != (k & WPAD_NUNCHUK_BUTTON_Z))
		{
			Sys_HandleKey(K_NUNCHUK_Z, ((k & WPAD_NUNCHUK_BUTTON_Z) == WPAD_NUNCHUK_BUTTON_Z));
		};
		if((sys_previous_keys & WPAD_NUNCHUK_BUTTON_C) != (k & WPAD_NUNCHUK_BUTTON_C))
		{
			Sys_HandleKey(K_NUNCHUK_C, ((k & WPAD_NUNCHUK_BUTTON_C) == WPAD_NUNCHUK_BUTTON_C));
		};
	} else if(ex.type == WPAD_EXP_CLASSIC)
	{
		if((sys_previous_keys & WPAD_CLASSIC_BUTTON_A) != (k & WPAD_CLASSIC_BUTTON_A))
		{
			Sys_HandleKey(K_CLASSIC_A, ((k & WPAD_CLASSIC_BUTTON_A) == WPAD_CLASSIC_BUTTON_A));
		};
		if((sys_previous_keys & WPAD_CLASSIC_BUTTON_B) != (k & WPAD_CLASSIC_BUTTON_B))
		{
			Sys_HandleKey(K_CLASSIC_B, ((k & WPAD_CLASSIC_BUTTON_B) == WPAD_CLASSIC_BUTTON_B));
		};
		if((sys_previous_keys & WPAD_CLASSIC_BUTTON_X) != (k & WPAD_CLASSIC_BUTTON_X))
		{
			Sys_HandleKey(K_CLASSIC_X, ((k & WPAD_CLASSIC_BUTTON_X) == WPAD_CLASSIC_BUTTON_X));
		};
		if((sys_previous_keys & WPAD_CLASSIC_BUTTON_Y) != (k & WPAD_CLASSIC_BUTTON_Y))
		{
			Sys_HandleKey(K_CLASSIC_Y, ((k & WPAD_CLASSIC_BUTTON_Y) == WPAD_CLASSIC_BUTTON_Y));
		};
		if((sys_previous_keys & WPAD_CLASSIC_BUTTON_FULL_L) != (k & WPAD_CLASSIC_BUTTON_FULL_L))
		{
			Sys_HandleKey(K_CLASSIC_L, ((k & WPAD_CLASSIC_BUTTON_FULL_L) == WPAD_CLASSIC_BUTTON_FULL_L));
		};
		if((sys_previous_keys & WPAD_CLASSIC_BUTTON_FULL_R) != (k & WPAD_CLASSIC_BUTTON_FULL_R))
		{
			Sys_HandleKey(K_CLASSIC_R, ((k & WPAD_CLASSIC_BUTTON_FULL_R) == WPAD_CLASSIC_BUTTON_FULL_R));
		};
		if((sys_previous_keys & WPAD_CLASSIC_BUTTON_ZL) != (k & WPAD_CLASSIC_BUTTON_ZL))
		{
			Sys_HandleKey(K_CLASSIC_ZL, ((k & WPAD_CLASSIC_BUTTON_ZL) == WPAD_CLASSIC_BUTTON_ZL));
		};
		if((sys_previous_keys & WPAD_CLASSIC_BUTTON_ZR) != (k & WPAD_CLASSIC_BUTTON_ZR))
		{
			Sys_HandleKey(K_CLASSIC_ZR, ((k & WPAD_CLASSIC_BUTTON_ZR) == WPAD_CLASSIC_BUTTON_ZR));
		};
		if((sys_previous_keys & WPAD_CLASSIC_BUTTON_PLUS) != (k & WPAD_CLASSIC_BUTTON_PLUS))
		{
			Sys_HandleKey(K_CLASSIC_PLUS, ((k & WPAD_CLASSIC_BUTTON_PLUS) == WPAD_CLASSIC_BUTTON_PLUS));
		};
		if((sys_previous_keys & WPAD_CLASSIC_BUTTON_MINUS) != (k & WPAD_CLASSIC_BUTTON_MINUS))
		{
			Sys_HandleKey(K_CLASSIC_MINUS, ((k & WPAD_CLASSIC_BUTTON_MINUS) == WPAD_CLASSIC_BUTTON_MINUS));
		};
		if((sys_previous_keys & WPAD_CLASSIC_BUTTON_UP) != (k & WPAD_CLASSIC_BUTTON_UP))
		{
			Sys_HandleKey(K_CLASSIC_UPARROW, ((k & WPAD_CLASSIC_BUTTON_UP) == WPAD_CLASSIC_BUTTON_UP));
		};
		if((sys_previous_keys & WPAD_CLASSIC_BUTTON_DOWN) != (k & WPAD_CLASSIC_BUTTON_DOWN))
		{
			Sys_HandleKey(K_CLASSIC_DOWNARROW, ((k & WPAD_CLASSIC_BUTTON_DOWN) == WPAD_CLASSIC_BUTTON_DOWN));
		};
		if((sys_previous_keys & WPAD_CLASSIC_BUTTON_LEFT) != (k & WPAD_CLASSIC_BUTTON_LEFT))
		{
			Sys_HandleKey(K_CLASSIC_LEFTARROW, ((k & WPAD_CLASSIC_BUTTON_LEFT) == WPAD_CLASSIC_BUTTON_LEFT));
		};
		if((sys_previous_keys & WPAD_CLASSIC_BUTTON_RIGHT) != (k & WPAD_CLASSIC_BUTTON_RIGHT))
		{
			Sys_HandleKey(K_CLASSIC_RIGHTARROW, ((k & WPAD_CLASSIC_BUTTON_RIGHT) == WPAD_CLASSIC_BUTTON_RIGHT));
		};
		if((sys_previous_keys & WPAD_CLASSIC_BUTTON_HOME) != (k & WPAD_CLASSIC_BUTTON_HOME))
		{
			Sys_HandleKey(K_CLASSIC_HOME, ((k & WPAD_CLASSIC_BUTTON_HOME) == WPAD_CLASSIC_BUTTON_HOME));
		};
	};
	sys_previous_keys = k;
	PAD_ScanPads();
	g = PAD_ButtonsHeld(0);
	if((sys_previous_pad_keys & PAD_BUTTON_A) != (g & PAD_BUTTON_A))
	{
		Sys_HandleKey(K_GCUBE_A, ((g & PAD_BUTTON_A) == PAD_BUTTON_A));
	};
	if((sys_previous_pad_keys & PAD_BUTTON_B) != (g & PAD_BUTTON_B))
	{
		Sys_HandleKey(K_GCUBE_B, ((g & PAD_BUTTON_B) == PAD_BUTTON_B));
	};
	if((sys_previous_pad_keys & PAD_BUTTON_X) != (g & PAD_BUTTON_X))
	{
		Sys_HandleKey(K_GCUBE_X, ((g & PAD_BUTTON_X) == PAD_BUTTON_X));
	};
	if((sys_previous_pad_keys & PAD_BUTTON_Y) != (g & PAD_BUTTON_Y))
	{
		Sys_HandleKey(K_GCUBE_Y, ((g & PAD_BUTTON_Y) == PAD_BUTTON_Y));
	};
	if((sys_previous_pad_keys & PAD_TRIGGER_L) != (g & PAD_TRIGGER_L))
	{
		Sys_HandleKey(K_GCUBE_L, ((g & PAD_TRIGGER_L) == PAD_TRIGGER_L));
	};
	if((sys_previous_pad_keys & PAD_TRIGGER_R) != (g & PAD_TRIGGER_R))
	{
		Sys_HandleKey(K_GCUBE_R, ((g & PAD_TRIGGER_R) == PAD_TRIGGER_R));
	};
	if((sys_previous_pad_keys & PAD_BUTTON_UP) != (g & PAD_BUTTON_UP))
	{
		Sys_HandleKey(K_GCUBE_UPARROW, ((g & PAD_BUTTON_UP) == PAD_BUTTON_UP));
	};
	if((sys_previous_pad_keys & PAD_BUTTON_DOWN) != (g & PAD_BUTTON_DOWN))
	{
		Sys_HandleKey(K_GCUBE_DOWNARROW, ((g & PAD_BUTTON_DOWN) == PAD_BUTTON_DOWN));
	};
	if((sys_previous_pad_keys & PAD_BUTTON_LEFT) != (g & PAD_BUTTON_LEFT))
	{
		Sys_HandleKey(K_GCUBE_LEFTARROW, ((g & PAD_BUTTON_LEFT) == PAD_BUTTON_LEFT));
	};
	if((sys_previous_pad_keys & PAD_BUTTON_RIGHT) != (g & PAD_BUTTON_RIGHT))
	{
		Sys_HandleKey(K_GCUBE_RIGHTARROW, ((g & PAD_BUTTON_RIGHT) == PAD_BUTTON_RIGHT));
	};
	if((sys_previous_pad_keys & PAD_BUTTON_START) != (g & PAD_BUTTON_START))
	{
		Sys_HandleKey(K_GCUBE_START, ((g & PAD_BUTTON_START) == PAD_BUTTON_START));
	};
	sys_previous_pad_keys = g;
	while(KEYBOARD_GetEvent(&e))
    {
        if(e.type == KEYBOARD_PRESSED)
        {
			Sys_HandleKey(Sys_FindKey(e.symbol), true);
        } else if(e.type == KEYBOARD_RELEASED)
        {
			Sys_HandleKey(Sys_FindKey(e.symbol), false);
        };
    };
	if((sys_previous_mouse_buttons & 0x01) != (sys_mouse_event.button & 0x01))
	{
		Sys_HandleKey(K_MOUSE1, ((sys_mouse_event.button & 0x01) == 0x01));
	};
	if((sys_previous_mouse_buttons & 0x02) != (sys_mouse_event.button & 0x02))
	{
		Sys_HandleKey(K_MOUSE2, ((sys_mouse_event.button & 0x02) == 0x02));
	};
	if((sys_previous_mouse_buttons & 0x04) != (sys_mouse_event.button & 0x04))
	{
		Sys_HandleKey(K_MOUSE3, ((sys_mouse_event.button & 0x04) == 0x04));
	};
	sys_previous_mouse_buttons = sys_mouse_event.button;
}

void Sys_HighFPPrecision (void)
{
}

void Sys_LowFPPrecision (void)
{
}

// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// New function to help local -> heap allocation issues
void* Sys_Malloc(int size, char* purpose)
{
	void* m;
	
	m = malloc(size);
	if(m == 0)
	{
		Sys_Error ("Sys_Malloc: %s - failed on %i bytes", purpose, size);
	};
	return m;
}
// <<< FIX

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

//=============================================================================

void Sys_PowerOff(s32 chan)
{
	SYS_ResetSystem(SYS_POWEROFF, 0, 0);
}

int main (int argc, char* argv[])
{
	isDedicated = false;

	sys_current_weapon = 1;

	VIDEO_Init();
	WPAD_Init();
	PAD_Init();

	sys_rmode = VIDEO_GetPreferredMode(NULL);

	sys_framebuffer[0] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(sys_rmode));
	sys_framebuffer[1] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(sys_rmode));

	sys_currentframebuf = 0;

	CON_Init(sys_framebuffer[0], 20, 20, sys_rmode->fbWidth, sys_rmode->xfbHeight, sys_rmode->fbWidth * VI_DISPLAY_PIX_SZ);

	VIDEO_Configure(sys_rmode);
	VIDEO_SetNextFramebuffer(sys_framebuffer[sys_currentframebuf]);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if(sys_rmode->viTVMode & VI_NON_INTERLACE)
	{
		VIDEO_WaitVSync();
	};

#ifdef GXQUAKE
	sys_gpfifo = memalign(32,SYS_FIFO_SIZE);
	memset(sys_gpfifo,0,SYS_FIFO_SIZE);
 
	GX_Init(sys_gpfifo,SYS_FIFO_SIZE);
 
	GX_SetCopyClear(sys_background_color, GX_MAX_Z24);
 
	sys_yscale = GX_GetYScaleFactor(sys_rmode->efbHeight,sys_rmode->xfbHeight);
	sys_xfbHeight = GX_SetDispCopyYScale(sys_yscale);
	GX_SetScissor(0,0,sys_rmode->fbWidth,sys_rmode->efbHeight);
	GX_SetDispCopySrc(0,0,sys_rmode->fbWidth,sys_rmode->efbHeight);
	GX_SetDispCopyDst(sys_rmode->fbWidth,sys_xfbHeight);
	GX_SetCopyFilter(sys_rmode->aa,sys_rmode->sample_pattern,GX_TRUE,sys_rmode->vfilter);
	GX_SetFieldMode(sys_rmode->field_rendering,((sys_rmode->viHeight==2*sys_rmode->xfbHeight)?GX_ENABLE:GX_DISABLE));
 
	GX_CopyDisp(sys_framebuffer[sys_currentframebuf],GX_TRUE);
	GX_SetPixelFmt(GX_PF_RGBA6_Z24, GX_ZC_LINEAR);
	GX_SetDispCopyGamma(GX_GM_1_0);

	GX_ClearVtxDesc();
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
 	GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
 
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
 
	GX_SetVtxAttrFmt(GX_VTXFMT1, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT1, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT1, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
 
	GX_SetNumChans(1);
	GX_SetNumTexGens(0);
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLOR0A0);
	GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);

	sys_frame_length = 1.0 / 60.0;

#else
	VIDEO_ClearFrameBuffer(sys_rmode, sys_framebuffer[1], COLOR_BLACK);
	VIDEO_SetNextFramebuffer(sys_framebuffer[1]);

	sys_frame_length = 1.0 / 30.0;

#endif

	WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS_ACC_IR);
	WPAD_SetVRes(WPAD_CHAN_0, sys_rmode->fbWidth, sys_rmode->xfbHeight);

	if(!fatInitDefault())
	{
		Sys_Error("Filesystem not enabled");
	};

	sys_previous_time = Sys_FloatTime();
	do 
	{
		sys_netinit_error = if_config(sys_ipaddress_text, NULL, NULL, true);
	} while((sys_netinit_error == -EAGAIN)&&((Sys_FloatTime() - sys_previous_time) < 3));
	if(sys_netinit_error < 0)
	{
		Sys_Printf("Network not enabled\n");
	};

	if (KEYBOARD_Init(Sys_KeyPress) != 0)
	{
		Sys_Printf("Keyboard not found\n");
	};

	OSK_LoadKeys(Keys_dat, Keys_dat_size);

	static quakeparms_t    parms;

	parms.memsize = 8*1024*1024;
	parms.membase = malloc (parms.memsize);
	parms.basedir = ".";

	COM_InitArgv (argc, argv);

	parms.argc = com_argc;
	parms.argv = com_argv;

	printf ("Host_Init\n");
	Host_Init (&parms);

	WPAD_SetPowerButtonCallback(Sys_PowerOff);

	while (1)
	{
		sys_previous_time = Sys_FloatTime();
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
		Host_Frame (sys_frame_length);
#ifdef GXQUAKE
		sys_currentframebuf ^= 1;
		GX_SetColorUpdate(GX_TRUE);
		GX_SetAlphaUpdate(GX_TRUE);
		GX_CopyDisp(sys_framebuffer[sys_currentframebuf], GX_TRUE);
		VIDEO_SetNextFramebuffer(sys_framebuffer[sys_currentframebuf]);
#endif
		KEYBOARD_FlushEvents();
		VIDEO_Flush();
		VIDEO_WaitVSync();
		while((Sys_FloatTime() - sys_previous_time) < sys_frame_length)
		{
			LWP_YieldThread();
		};
	};

	return 0;
}
