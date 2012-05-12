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
// Include only for GX hardware builds (part 1):
#ifdef GXIMP
// <<< FIX
#include "../ref_gx/gx_local.h"

#include <gccore.h>
#include <wiiuse/wpad.h>

extern GXRModeObj* sys_rmode;

extern	cvar_t*	wmotelookbinv;

extern cvar_t* in_osk;

double gximp_guide_increment;

int gximp_pal_increment;

int gximp_guide_texture;

void		GXimp_BeginFrame( float camera_separation )
{
}

void		GXimp_EndFrame( void )
{
	ir_t p;
	u32 k;
	int r;
	int w;
	int sw;
	int hsw;
	unsigned int* guide;
	double a;
	int i;
	int x;
	int y;

	WPAD_ScanPads();
	WPAD_IR(WPAD_CHAN_0, &p);
	if(p.valid)
	{
		k = WPAD_ButtonsHeld(WPAD_CHAN_0);
		if((((k & WPAD_BUTTON_A) != WPAD_BUTTON_A)&&(wmotelookbinv->value == 0))
		 ||(((k & WPAD_BUTTON_A) == WPAD_BUTTON_A)&&(wmotelookbinv->value != 0))
		 ||(in_osk->value != 0))
		{
			r = 12 * vid.width / 320;
			w = 2 * r;
			for (sw = 1 ; sw < w ; sw<<=1)
				;
			hsw = sw / 2;

			guide = Sys_BigStackAlloc(sw*sw * sizeof(unsigned), "GXimp_EndFrame");

			i = 0;
			for(y = 0; y < sw; y++)
			{
				for(x = 0; x < sw; x++)
				{
					guide[i] = d_8to24table[255];
					i++;
				};
			};

			a = 0;
			for(i = 0; i < 16; i++)
			{
				a = M_PI * i / 8;
				x = hsw - r * cos(a + gximp_guide_increment);
				y = hsw - r * sin(a + gximp_guide_increment);

				guide[y*sw + x] = d_8to24table[gximp_pal_increment];
				guide[y*sw + x + 1] = d_8to24table[gximp_pal_increment];

				gximp_pal_increment++;
				if(gximp_pal_increment > 255)
				{
					gximp_pal_increment = 0;
				};
			};

			if(!gximp_guide_texture)
				gximp_guide_texture = numgxtextures++;
			
			GX_Bind(gximp_guide_texture);
			GX_LoadAndBind(guide, sw*sw * 4, sw, sw, GX_TF_RGBA8);

			qgxBegin (GX_QUADS, gxu_cur_vertex_format, 4);
			qgxPosition3f32(p.x - hsw, p.y - hsw, 0);
			qgxColor4u8(gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
			qgxTexCoord2f32 (0, 0);
			qgxPosition3f32(p.x + hsw, p.y - hsw, 0);
			qgxColor4u8(gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
			qgxTexCoord2f32 (1, 0);
			qgxPosition3f32(p.x + hsw, p.y + hsw, 0);
			qgxColor4u8(gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
			qgxTexCoord2f32 (1, 1);
			qgxPosition3f32(p.x - hsw, p.y + hsw, 0);
			qgxColor4u8(gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
			qgxTexCoord2f32 (0, 1);
			qgxEnd ();

			Sys_BigStackFree(sw*sw * sizeof(unsigned), "GXimp_EndFrame");
		};
		gximp_guide_increment += 0.02;
		if(gximp_guide_increment > M_PI)
		{
			gximp_guide_increment -= M_PI;
		};
	};
}

int 		GXimp_Init( void *hinstance, void *hWnd )
{
	return true;
}

void		GXimp_Shutdown( void )
{
}

int     	GXimp_SetMode( int *pwidth, int *pheight, int mode, qboolean fullscreen )
{
	int width, height;

	fprintf(stderr, "GXimp_SetMode\n");

	ri.Con_Printf( PRINT_ALL, "Initializing GX hardware...\n");

	ri.Con_Printf (PRINT_ALL, "...setting mode %d:", mode );

	if ( !ri.Vid_GetModeInfo( &width, &height, mode ) )
	{
		ri.Con_Printf( PRINT_ALL, " invalid mode\n" );
		return rserr_invalid_mode;
	}

	ri.Con_Printf( PRINT_ALL, " %d %d\n", width, height );

	// destroy the existing window
	GXimp_Shutdown ();

	*pwidth = width;
	*pheight = height;

	// let the sound and input subsystems know about the new window
	ri.Vid_NewWindow (width, height);

	return rserr_ok;
}

void		GXimp_AppActivate( qboolean active )
{
}
// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// Include only for GX hardware builds (part 2):
#endif
// <<< FIX

