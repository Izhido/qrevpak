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

int			sys_curtime;


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


void Sys_mkdir ( const char *path ) {
}

void Sys_Error (const char *error, ...) {
	va_list		argptr;

	printf ("Sys_Error: ");	
	va_start (argptr,error);
	vprintf (error,argptr);
	va_end (argptr);
	printf ("\n");

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
	return 0;
}

void	Sys_Mkdir (const char *path) {
}

char	*Sys_FindFirst (char *path, unsigned musthave, unsigned canthave) {
	return NULL;
}

char	*Sys_FindNext (unsigned musthave, unsigned canthave) {
	return NULL;
}

void	Sys_FindClose (void) {
}

void	Sys_Init (void) {
}


void	Sys_EarlyOutput( char *string ) {
	printf( "%s", string );
}


int main (int argc, char **argv) {
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

	while (1) {
		Com_Frame( );
	}
	return 0;
}


