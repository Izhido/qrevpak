/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
// revol_main.c -- system driver for the Nintendo Wii using devkitPPC / libogc
// (based on null_main.c)

#include <errno.h>
#include <stdio.h>
#include "../game/q_shared.h"
#include "../qcommon/qcommon.h"

#include <gccore.h>
#include <dirent.h>
#include <ogc/lwp_watchdog.h>
#include <wiiuse/wpad.h>
#include "gxutils.h"
#include <fat.h>
#include <network.h>
#include <wiikeyboard/keyboard.h>
#include "osk_revol.h"
#include "Keys_dat.h"
#include <ogc/usbmouse.h>

extern cvar_t* in_osk;

int			sys_curtime;

void* sys_framebuffer[2] = {NULL, NULL};

GXRModeObj* sys_rmode;

int sys_previous_time;

int sys_netinit_error;

u32 sys_frame_count;

char sys_ipaddress_text[16];

s32 sys_mouse_valid;

mouse_event sys_mouse_event;


//===================================================================

inline short Sys_LittleShort(short l)
{ 
	return ShortSwap(l);
}

inline int Sys_LittleLong (int l)
{ 
	return LongSwap(l); 
}

inline float Sys_LittleFloat (const float l) 
{ 
	return FloatSwap(&l);
}

//===================================================================

void Sys_BeginStreamedFile( fileHandle_t f, int readAhead ) {
}

void Sys_EndStreamedFile( fileHandle_t f ) {
}

int Sys_StreamedRead( void *buffer, int size, int count, fileHandle_t f ) {
	return fread( buffer, size, count, (FILE*)f );
}

void Sys_StreamSeek( fileHandle_t f, int offset, int origin ) {
	fseek( (FILE*)f, offset, origin );
}


//===================================================================


void Sys_Error (const char *error, ...) {
	va_list		argptr;
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

	exit (1);
}

void Sys_Quit (void) {
	exit (0);
}

void	Sys_UnloadGame (void) {
}

void	*Sys_GetGameAPI (void *parms) {
	return NULL;
}

char *Sys_GetClipboardData( void ) {
	return NULL;
}

int		Sys_Milliseconds (void) {
	static int		base;
	static qboolean	initialized = qfalse;
	u64 t;
	int ms;

	t = gettime();
	ms = ticks_to_millisecs(t);
	if (!initialized)
	{
		base = ms;
		initialized = qtrue;
	}
	sys_curtime = ms - base;

	return sys_curtime;
}

void	Sys_Mkdir (const char *path) {
	mkdir(path, 0777);
}

void	Sys_Init (void) {
}

void Sys_KeyPress(char c)
{
	// Do nothing; keys are being polled straight from the keyboard
}

void Sys_PowerOff(s32 chan)
{
	if(chan == WPAD_CHAN_0)
	{
		SYS_ResetSystem(SYS_POWEROFF, 0, 0);
	};
}


int main (int argc, char **argv) {

	VIDEO_Init();
	WPAD_Init();
	PAD_Init();

	sys_rmode = VIDEO_GetPreferredMode(NULL);

	sys_framebuffer[0] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(sys_rmode));
	sys_framebuffer[1] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(sys_rmode));

	sys_frame_count = 0;

	CON_Init(sys_framebuffer[sys_frame_count & 1], 20, 20, sys_rmode->fbWidth, sys_rmode->xfbHeight, sys_rmode->fbWidth * VI_DISPLAY_PIX_SZ);

	VIDEO_Configure(sys_rmode);
	VIDEO_SetNextFramebuffer(sys_framebuffer[sys_frame_count & 1]);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if(sys_rmode->viTVMode & VI_NON_INTERLACE)
	{
		VIDEO_WaitVSync();
	};

	sys_frame_count++;

	GXU_Init(sys_rmode, sys_framebuffer[sys_frame_count & 1]);

	WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS_ACC_IR);
	WPAD_SetVRes(WPAD_CHAN_0, sys_rmode->fbWidth, sys_rmode->xfbHeight);

	if(!fatInitDefault())
	{
		Sys_Error("Filesystem not enabled");
	};

	sys_previous_time = Sys_Milliseconds();
	do 
	{
		sys_netinit_error = if_config(sys_ipaddress_text, NULL, NULL, true);
	} while((sys_netinit_error == -EAGAIN)&&((Sys_Milliseconds() - sys_previous_time) < 3000));
	if(sys_netinit_error < 0)
	{
		printf("Network not enabled\n");
	};

	if (KEYBOARD_Init(Sys_KeyPress) != 0)
	{
		printf("Keyboard not found\n");
	};

	OSK_LoadKeys(Keys_dat, Keys_dat_size);

	int   len, i;
	char  *cmdline;

	// merge the command line, this is kinda silly
	for (len = 1, i = 1; i < argc; i++)
		len += strlen(argv[i]) + 1;
	cmdline = malloc(len);
	*cmdline = 0;
	for (i = 1; i < argc; i++)
	{
		if (i > 1)
			strcat(cmdline, " ");
		strcat(cmdline, argv[i]);
	}

	Com_Init(cmdline);

	WPAD_SetPowerButtonCallback(Sys_PowerOff);

	while (1) {

		sys_previous_time = Sys_Milliseconds();
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

		Com_Frame( );

		if(in_osk->value)
		{
			OSK_Draw(sys_rmode, sys_framebuffer[sys_frame_count & 1]);
		};
		sys_frame_count++;
		GXU_EndFrame(sys_framebuffer[sys_frame_count & 1]);

		KEYBOARD_FlushEvents();
		VIDEO_Flush();
		VIDEO_WaitVSync();

	}
	return 0;
}


