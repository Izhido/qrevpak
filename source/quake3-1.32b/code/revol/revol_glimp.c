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
#include "../renderer_gx/tr_local.h"


static void GLW_InitExtensions( void )
{
	if ( !r_allowExtensions->integer )
	{
		ri.Printf( PRINT_ALL, "*** IGNORING EXTENSIONS ***\n" );
		return;
	}

	ri.Printf( PRINT_ALL, "Initializing extensions\n" );

	// S3 texture compression
	glConfig.textureCompression = TC_NONE;

	// Texture environment add operation
	if ( r_ext_texture_env_add->integer )
	{
		glConfig.textureEnvAddAvailable = qtrue;
		ri.Printf( PRINT_ALL, "...using texture environment add operation\n" );
	}
	else
	{
		glConfig.textureEnvAddAvailable = qfalse;
		ri.Printf( PRINT_ALL, "...ignoring texture environment add operation\n" );
	}

	// Multitexture
	if ( r_ext_multitexture->integer )
	{
		ri.Printf( PRINT_ALL, "...using multitexture\n" );
		GX_TEXTURE0 = GX_TEXMAP0;
		GX_TEXTURE1 = GX_TEXMAP1;
	}
	else
	{
		ri.Printf( PRINT_ALL, "...ignoring multitexture\n" );
		GX_TEXTURE0 = GX_TEXMAP0;
		GX_TEXTURE1 = GX_TEXMAP0;
	}
}


void		GLimp_EndFrame( void ) {
}

void 		GLimp_Init( void )
{
	GLW_InitExtensions();
}

void		GLimp_Shutdown( void ) {
}

void		GLimp_EnableLogging( qboolean enable ) {
}

void GLimp_LogComment( char *comment ) {
}

qboolean QGL_Init( const char *dllname ) {
	return qtrue;
}

void		QGL_Shutdown( void ) {
}
