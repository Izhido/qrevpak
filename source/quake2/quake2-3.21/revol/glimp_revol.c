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

// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// Include only for the GL builds (part 1):
#ifdef GXIMP
// <<< FIX
#include "../ref_gx/gx_local.h"

void		GLimp_BeginFrame( float camera_separation )
{
}

void		GLimp_EndFrame( void )
{
}

int 		GLimp_Init( void *hinstance, void *hWnd )
{
	return true;
}

void		GLimp_Shutdown( void )
{
}

int     	GLimp_SetMode( int *pwidth, int *pheight, int mode, qboolean fullscreen )
{
	int width, height;

	fprintf(stderr, "GLimp_SetMode\n");

	ri.Con_Printf( PRINT_ALL, "Initializing OpenGL display\n");

	ri.Con_Printf (PRINT_ALL, "...setting mode %d:", mode );

	if ( !ri.Vid_GetModeInfo( &width, &height, mode ) )
	{
		ri.Con_Printf( PRINT_ALL, " invalid mode\n" );
		return rserr_invalid_mode;
	}

	ri.Con_Printf( PRINT_ALL, " %d %d\n", width, height );

	// destroy the existing window
	GLimp_Shutdown ();

	*pwidth = width;
	*pheight = height;

	// let the sound and input subsystems know about the new window
	ri.Vid_NewWindow (width, height);

	return rserr_ok;
}

void		GLimp_AppActivate( qboolean active )
{
}
// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// Include only for the GL builds (part 2):
#endif
// <<< FIX

