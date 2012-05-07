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
// gx_vidrevol.c -- video driver for the GX hardware of the Nintendo Wii 
// (based on vid_null.c)

#ifdef GXQUAKE

#include <gccore.h>
#include <wiiuse/wpad.h>
#include <malloc.h>

#include "quakedef.h"

#include "gxutils.h"

unsigned	d_8to24table[256];

int		texture_mode = GX_LINEAR;

int		texture_extension_number = 1;

float		gxdepthmin, gxdepthmax;

cvar_t	gl_ztrick = {"gl_ztrick","1"};

static float vid_gamma = 1.0;

qboolean gx_mtexable = false;

extern GXRModeObj* sys_rmode;

extern cvar_t in_osk;

double vid_guide_increment;

int vid_pal_increment;

int vid_guide_texture;

void CheckMultiTextureExtensions(void) 
{
	if (!COM_CheckParm("-nomtex")) {
		Con_Printf("Multitexture extensions found.\n");
		gx_mtexable = true;
	}
}

/*
===============
QGX_Init
===============
*/
void QGX_Init (void)
{
	CheckMultiTextureExtensions();

	gxu_background_color.r = 255;
	gxu_cull_mode = GX_CULL_BACK;
	if(gxu_cull_enabled)
	{
		GX_SetCullMode(gxu_cull_mode);
	};
	gxu_cur_vertex_format = GX_VTXFMT1;
	GXU_EnableTexture();

	gxu_alpha_test_lower = 170;
	gxu_alpha_test_higher = 255;
	gxu_alpha_test_enabled = true;
	GX_SetAlphaCompare(GX_GEQUAL, gxu_alpha_test_lower, GX_AOP_AND, GX_LEQUAL, gxu_alpha_test_higher);

	// Implement this ASAP:
	//glShadeModel (GL_FLAT);

	GX_SetMinMag (GX_NEAR, GX_NEAR);

	gxu_blend_src_value = GX_BL_SRCALPHA;
	gxu_blend_dst_value = GX_BL_INVSRCALPHA;
	GX_SetBlendMode(GX_BM_NONE, gxu_blend_src_value, gxu_blend_dst_value, GX_LO_NOOP); 

	GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
}

void GX_BeginRendering (int *x, int *y, int *width, int *height)
{
	*x = 0;
	*y = 0;
	*width = sys_rmode->fbWidth;
	*height = sys_rmode->efbHeight;
}

void GX_EndRendering (void)
{
	GX_DrawDone();
}

void VID_DrawWmoteGuide(void)
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
		if((((k & WPAD_BUTTON_A) != WPAD_BUTTON_A)&&(wmotelookbinv.value == 0))
		 ||(((k & WPAD_BUTTON_A) == WPAD_BUTTON_A)&&(wmotelookbinv.value != 0))
		 ||(in_osk.value != 0))
		{
			r = 12 * vid.width / 320;
			w = 2 * r;
			for (sw = 1 ; sw < w ; sw<<=1)
				;
			hsw = sw / 2;

			guide = Sys_BigStackAlloc(sw*sw * sizeof(unsigned), "VID_DrawWmoteGuide");

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
				x = hsw - r * cos(a + vid_guide_increment);
				y = hsw - r * sin(a + vid_guide_increment);

				guide[y*sw + x] = d_8to24table[vid_pal_increment];
				guide[y*sw + x + 1] = d_8to24table[vid_pal_increment];

				vid_pal_increment++;
				if(vid_pal_increment > 255)
				{
					vid_pal_increment = 0;
				};
			};

			if(!vid_guide_texture)
				vid_guide_texture = texture_extension_number++;
			
			GX_Bind(vid_guide_texture);
			GX_LoadAndBind(guide, sw*sw * 4, sw, sw, GX_TF_RGBA8);

			GX_Begin (GX_QUADS, gxu_cur_vertex_format, 4);
			GX_Position3f32(p.x - hsw, p.y - hsw, 0);
			GX_Color4u8(gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
			GX_TexCoord2f32 (0, 0);
			GX_Position3f32(p.x + hsw, p.y - hsw, 0);
			GX_Color4u8(gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
			GX_TexCoord2f32 (1, 0);
			GX_Position3f32(p.x + hsw, p.y + hsw, 0);
			GX_Color4u8(gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
			GX_TexCoord2f32 (1, 1);
			GX_Position3f32(p.x - hsw, p.y + hsw, 0);
			GX_Color4u8(gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
			GX_TexCoord2f32 (0, 1);
			GX_End ();

			Sys_BigStackFree(sw*sw * sizeof(unsigned), "VID_DrawWmoteGuide");
		};
		vid_guide_increment += 0.02;
		if(vid_guide_increment > M_PI)
		{
			vid_guide_increment -= M_PI;
		};
	};
}

void	VID_SetPalette (unsigned char *palette)
{
	byte	*pal;
	unsigned r,g,b;
	unsigned v;
	unsigned short i;
	unsigned	*table;

//
// 8 8 8 encoding
//
	pal = palette;
	table = d_8to24table;
	for (i=0 ; i<256 ; i++)
	{
		r = pal[0];
		g = pal[1];
		b = pal[2];
		pal += 3;
		
		v = (r<<24) | (g<<16) | (b<<8) | 255;
		*table++ = v;
	}
	d_8to24table[255] &= 0xffffff00;	// 255 is transparent
}

void	VID_ShiftPalette (unsigned char *palette)
{
	VID_SetPalette(palette);
}

static void Check_Gamma (unsigned char *pal)
{
	float	f, inf;
	unsigned char*	palette = Sys_BigStackAlloc(768, "Check_Gamma");
	int		i;

	if ((i = COM_CheckParm("-gamma")) == 0) {
		vid_gamma = 0.7;
	} else
		vid_gamma = Q_atof(com_argv[i+1]);

	for (i=0 ; i<768 ; i++)
	{
		f = pow ( (pal[i]+1)/256.0 , vid_gamma );
		inf = f*255 + 0.5;
		if (inf < 0)
			inf = 0;
		if (inf > 255)
			inf = 255;
		palette[i] = inf;
	}

	memcpy (pal, palette, sizeof(palette));
	Sys_BigStackFree(768, "Check_Gamma");
}

void	VID_Init (unsigned char *palette)
{
	int i;
	char	gxdir[MAX_OSPATH];
	int width = sys_rmode->fbWidth;
	int height = sys_rmode->efbHeight;

	Cvar_RegisterVariable (&gl_ztrick);
	
	vid.maxwarpwidth = width;
	vid.maxwarpheight = height;
	vid.colormap = host_colormap;
	vid.fullbright = 256 - LittleLong (*((int *)vid.colormap + 2048));

// set vid parameters
	if ((i = COM_CheckParm("-width")) != 0)
		width = atoi(com_argv[i+1]);
	if ((i = COM_CheckParm("-height")) != 0)
		height = atoi(com_argv[i+1]);

	if ((i = COM_CheckParm("-conwidth")) != 0)
		vid.conwidth = Q_atoi(com_argv[i+1]);
	else
		vid.conwidth = 640;

	vid.conwidth &= 0xfff8; // make it a multiple of eight

	if (vid.conwidth < 320)
		vid.conwidth = 320;

	// pick a conheight that matches with correct aspect
	vid.conheight = vid.conwidth*3 / 4;

	if ((i = COM_CheckParm("-conheight")) != 0)
		vid.conheight = Q_atoi(com_argv[i+1]);
	if (vid.conheight < 200)
		vid.conheight = 200;

	if (vid.conheight > height)
		vid.conheight = height;
	if (vid.conwidth > width)
		vid.conwidth = width;
	vid.width = vid.conwidth;
	vid.height = vid.conheight;

	vid.aspect = ((float)vid.height / (float)vid.width) *
				(320.0 / 240.0);
	vid.numpages = 2;

	QGX_Init();

	sprintf (gxdir, "%s/gxquake", com_gamedir);
	Sys_mkdir (gxdir);

	Check_Gamma(palette);
	VID_SetPalette(palette);

	Con_SafePrintf ("Video mode %dx%d initialized.\n", width, height);

	vid.recalc_refdef = 1;				// force a surface cache flush
}

void	VID_Shutdown (void)
{
}

void	VID_Update (vrect_t *rects)
{
}

#endif

