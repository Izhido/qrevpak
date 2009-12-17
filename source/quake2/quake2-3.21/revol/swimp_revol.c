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
#include <gccore.h>
#include <wiiuse/wpad.h>
#define BOOL_IMPLEMENTED 1

#include "../ref_soft/r_local.h"
#include "Keyboard_img.h"
#include "KeyboardInverted_img.h"
#include "osk_revol.h"

struct y1cby2cr_palentry_t
{
	u32 color;
	u32 count;
};

extern void* sys_framebuffer[2];

extern cvar_t* in_osk;

int swimp_outerwidthborder;

int swimp_outerheightborder;

int swimp_scale;

extern GXRModeObj* sys_rmode;

u8 swimp_8to24finaltable[1024];

struct y1cby2cr_palentry_t d_8toy1cby2cr[256][256];

int swimp_palentry_increment;

double swimp_guide_increment;

int swimp_pal_increment;

u32 SWimp_CvtRGB(u8 r1, u8 g1, u8 b1, u8 r2, u8 g2, u8 b2)
{
	int y1, cb1, cr1, y2, cb2, cr2, cb, cr;

	y1 = (299 * r1 + 587 * g1 + 114 * b1) / 1000;
	cb1 = (-16874 * r1 - 33126 * g1 + 50000 * b1 + 12800000) / 100000;
	cr1 = (50000 * r1 - 41869 * g1 - 8131 * b1 + 12800000) / 100000;

	y2 = (299 * r2 + 587 * g2 + 114 * b2) / 1000;
	cb2 = (-16874 * r2 - 33126 * g2 + 50000 * b2 + 12800000) / 100000;
	cr2 = (50000 * r2 - 41869 * g2 - 8131 * b2 + 12800000) / 100000;

	cb = (cb1 + cb2) >> 1;
	cr = (cr1 + cr2) >> 1;

	return (y1 << 24) | (cb << 16) | (y2 << 8) | cr;
}

void SWimp_DrawOnScreenKeyboard(void)
{
	ir_t p;
	u32 k;
	int j;
	int i;
	int m;
	int n;
	int l;
	u32* v;
	u32 vinc;
	u8 a;
	u32 c;
	const u8* img;

	if(in_osk->value)
	{
		m = 606;
		n = 300;
		WPAD_ScanPads();
		WPAD_IR(WPAD_CHAN_0, &p);
		if(p.valid)
		{
			osk_selected = OSK_KeyAt(p.x - ((sys_rmode->viWidth - m) / 2), p.y - ((sys_rmode->viHeight - n) / 2));
		} else
		{
			osk_selected = 0;
		};
		k = WPAD_ButtonsHeld(WPAD_CHAN_0);
		v = ((u32*)(sys_framebuffer[0])) + ((sys_rmode->viHeight - n) / 4 * sys_rmode->viWidth) + ((sys_rmode->viWidth - m) / 4);
		vinc = (sys_rmode->viWidth - m) / 2;
		l = 0;
		for(j = 0; j < n; j++)
		{
			i = 0;
			while(i < m)
			{
				img = Keyboard_img;
				if(osk_selected != 0)
				{
					if((k & WPAD_BUTTON_A) != WPAD_BUTTON_A)
					{
						if((i >= osk_selected->left)&&(i <= osk_selected->right)&&(j >= osk_selected->top)&&(j <= osk_selected->bottom))
						{
							img = KeyboardInverted_img;
						};
					};
				};
				if(osk_shiftpressed != 0)
				{
					if((i >= osk_shiftpressed->left)&&(i <= osk_shiftpressed->right)&&(j >= osk_shiftpressed->top)&&(j <= osk_shiftpressed->bottom))
					{
						img = KeyboardInverted_img;
					};
				};
				if(osk_capspressed != 0)
				{
					if((i >= osk_capspressed->left)&&(i <= osk_capspressed->right)&&(j >= osk_capspressed->top)&&(j <= osk_capspressed->bottom))
					{
						img = KeyboardInverted_img;
					};
				};
				a = img[l];
				if(a == 0)
				{
					l += 5;
				} else
				{
					l++;
					c = *((u32*)(img + l));
					l += 4;
					(*v) = c;
				};
				i += 2;
				v++;
			};
			v += vinc;
		};
	};
}

void SWimp_DrawWmoteGuide(void)
{
	ir_t p;
	u32 k;
	int r;
	double a;
	int i;
	int x;
	int y;
	u32* v;
	u32 f;
	int pi;
	u8 cr;
	u8 cg;
	u8 cb;
	
	WPAD_ScanPads();
	WPAD_IR(WPAD_CHAN_0, &p);
	if(p.valid)
	{
		k = WPAD_ButtonsHeld(WPAD_CHAN_0);
		if((k & WPAD_BUTTON_A) != WPAD_BUTTON_A)
		{
			r = 12 * vid.width / 320;
			a = 0;
			for(i = 0; i < 16; i++)
			{
				a = 3.14159265358979323846 * i / 8;
				x = p.x - r * cos(a + swimp_guide_increment);
				y = p.y - r * sin(a + swimp_guide_increment);
				if((x > swimp_outerwidthborder)&&(x < (sys_rmode->viWidth - swimp_outerwidthborder))&&(y > swimp_outerheightborder)&&(y < (sys_rmode->viHeight - swimp_outerheightborder)))
				{
					v = ((u32*)(sys_framebuffer[0])) + (y * sys_rmode->viWidth / 2) + (x / 2);
					if(d_8toy1cby2cr[swimp_pal_increment][swimp_pal_increment].count == swimp_palentry_increment)
					{
						f = d_8toy1cby2cr[swimp_pal_increment][swimp_pal_increment].color;
					} else
					{
						pi = swimp_pal_increment << 2;
						cr = swimp_8to24finaltable[pi];
						pi++;
						cg = swimp_8to24finaltable[pi];
						pi++;
						cb = swimp_8to24finaltable[pi];
						f = SWimp_CvtRGB(cr, cg, cb, cr, cg, cb);
						d_8toy1cby2cr[swimp_pal_increment][swimp_pal_increment].color = f;
						d_8toy1cby2cr[swimp_pal_increment][swimp_pal_increment].count = swimp_palentry_increment;
					};
					*v = f;
				};
				swimp_pal_increment++;
				if(swimp_pal_increment > 255)
				{
					swimp_pal_increment = 0;
				};
			};
		};
		swimp_guide_increment += 0.02;
		if(swimp_guide_increment > 3.14159265358979323846)
		{
			swimp_guide_increment -= 3.14159265358979323846;
		};
	};
}

void		SWimp_BeginFrame( float camera_separation )
{
}

void		SWimp_EndFrame (void)
{
	int i;
	int j;
	int h;
	int w;
	byte* p;
	u32* v;
	int vinc;
	int ix;
	int jx;
	byte pl;
	int pli;
	byte pr;
	int pri;
	u32 f;
	u8 r1;
	u8 g1;
	u8 b1;
	u8 r2;
	u8 g2;
	u8 b2;
	
	i = 0;
	j = 0;
	h = vid.height;
	w = vid.rowbytes;
	p = vid.buffer + i * vid.height + j;
	v = ((u32*)(sys_framebuffer[0])) + ((swimp_outerheightborder + (i * swimp_scale)) * (sys_rmode->viWidth / 2)) + (swimp_outerwidthborder / 2) + (j * swimp_scale / 2);
	vinc = (sys_rmode->viWidth - (vid.width * swimp_scale)) / 2;
	i = 0;
	ix = 0;
	while(i < h)
	{
		j = 0;
		jx = 0;
		while(j < w)
		{
			pl = *p;
			jx++;
			if(jx >= swimp_scale)
			{
				jx = 0;
				p++;
				j++;
			};
			pr = *p;
			jx++;
			if(jx >= swimp_scale)
			{
				jx = 0;
				p++;
				j++;
			};
			if(d_8toy1cby2cr[pl][pr].count == swimp_palentry_increment)
			{
				f = d_8toy1cby2cr[pl][pr].color;
			} else
			{
				pli = ((int)pl) << 2;
				r1 = swimp_8to24finaltable[pli];
				pli++;
				g1 = swimp_8to24finaltable[pli];
				pli++;
				b1 = swimp_8to24finaltable[pli];
				pri = ((int)pr) << 2;
				r2 = swimp_8to24finaltable[pri];
				pri++;
				g2 = swimp_8to24finaltable[pri];
				pri++;
				b2 = swimp_8to24finaltable[pri];
				f = SWimp_CvtRGB(r1, g1, b1, r2, g2, b2);
				d_8toy1cby2cr[pl][pr].color = f;
				d_8toy1cby2cr[pl][pr].count = swimp_palentry_increment;
			};
			*v = f;
			v++;
		};
		v += vinc;
		ix++;
		if(ix >= swimp_scale)
		{
			ix = 0;
			i++;
			p += vid.rowbytes;
		};
		p -= w;
	};
	SWimp_DrawOnScreenKeyboard();
	SWimp_DrawWmoteGuide();
}

int			SWimp_Init( void *hInstance, void *wndProc )
{
	return true;
}

static qboolean SWimp_InitGraphics(int newwidth, int newheight)
{
	int i;
	int j;

	SWimp_Shutdown();

	// let the sound and input subsystems know about the new window
	ri.Vid_NewWindow (newwidth, newheight);

	vid.rowbytes = newwidth;

	vid.buffer = malloc(newwidth * newheight);
	if (!vid.buffer)
		Sys_Error("Unabled to alloc vid.buffer!\n");

	swimp_scale = 1;

	swimp_outerwidthborder = ((sys_rmode->viWidth - newwidth) / 2);
	swimp_outerheightborder = ((sys_rmode->viHeight - newheight) / 2);

	swimp_guide_increment = 0;
	swimp_pal_increment = 0;

	for(i = 0; i < 255; i++)
	{
		for(j = 0; j < 255; j++)
		{
			d_8toy1cby2cr[i][j].count = 0;
		};
	};
	swimp_palentry_increment = 1;

	return true;
}

void		SWimp_SetPalette( const unsigned char *palette)
{
	memcpy(swimp_8to24finaltable, (void*)palette, 1024);
	swimp_palentry_increment++;
}

void		SWimp_Shutdown( void )
{
}

rserr_t		SWimp_SetMode( int *pwidth, int *pheight, int mode, qboolean fullscreen )
{
	rserr_t retval = rserr_ok;

	ri.Con_Printf (PRINT_ALL, "setting mode %d:", mode );

	if ( !ri.Vid_GetModeInfo( pwidth, pheight, mode ) )
	{
		ri.Con_Printf( PRINT_ALL, " invalid mode\n" );
		return rserr_invalid_mode;
	}

	ri.Con_Printf( PRINT_ALL, " %d %d\n", *pwidth, *pheight);

	if ( !SWimp_InitGraphics(*pwidth, *pheight) ) {
		return rserr_invalid_mode;
	}

	R_GammaCorrectAndSetPalette( ( const unsigned char * ) d_8to24table );

	return retval;
}

void		SWimp_AppActivate( qboolean active )
{
}
