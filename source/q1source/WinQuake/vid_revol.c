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
// vid_revol.c -- video driver for the Nintendo Wii using devkitPPC / libogc
// (based on vid_null.c)

// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// Include only for the software renderer builds (part 1):
#ifndef GLQUAKE
// <<< FIX

#include <gccore.h>
#include <wiiuse/wpad.h>

#include "quakedef.h"
#include "d_local.h"
#include "Keyboard_img.h"
#include "KeyboardInverted_img.h"
#include "osk_revol.h"

struct y1cby2cr_palentry_t
{
	u32 color;
	u32 count;
};

viddef_t	vid;				// global video state

int vid_width;

int vid_height;

extern GXRModeObj* sys_rmode;

extern cvar_t in_osk;

extern cvar_t in_wlook;

byte*	vid_buffer;

short*	zbuffer;

byte*	surfcache;

unsigned short	d_8to16table[256];

unsigned	d_8to24table[256];

struct y1cby2cr_palentry_t d_8toy1cby2cr[256][256];

int vid_palentry_increment;

extern void* sys_framebuffer[3];

int vid_scale;

int vid_finalwidth;

int vid_finalheight;

int vid_outerwidthborder;

int vid_outerheightborder;

double vid_guide_increment;

int vid_pal_increment;

u32 VID_CvtRGB(u8 r1, u8 g1, u8 b1, u8 r2, u8 g2, u8 b2)
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

void VID_DrawOnScreenKeyboard(void)
{
	int j;
	int i;
	u32* v;
	u32 vinc;
	u8 a;
	u32 c;
	const u8* img;
	int m;
	int n;
	int imginc;

	if(in_osk.value)
	{
		v = ((u32*)(sys_framebuffer[1])) + ((sys_rmode->viHeight - OSK_HEIGHT) / 4 * sys_rmode->viWidth) 
		                                 + ((sys_rmode->viWidth - OSK_WIDTH) / 4);
		vinc = (sys_rmode->viWidth - OSK_WIDTH) / 2;
		img = Keyboard_img;
		for(j = 0; j < OSK_HEIGHT; j++)
		{
			i = 0;
			while(i < OSK_WIDTH)
			{
				a = (*img);
				if(a == 0)
				{
					img += 5;
				} else
				{
					img++;
					c = *((u32*)img);
					img += 4;
					*v = c;
				};
				i += 2;
				v++;
			};
			v += vinc;
		};
		if(osk_selected != 0)
		{
			i = osk_selected->left & ~1;
			j = osk_selected->top & ~1;
			m = (osk_selected->right & ~1) - i;
			n = (osk_selected->bottom & ~1) - j;
			v = ((u32*)(sys_framebuffer[1])) + ((sys_rmode->viHeight - OSK_HEIGHT + j + j) / 4 * sys_rmode->viWidth) 
			                                 + ((sys_rmode->viWidth - OSK_WIDTH + i + i) / 4);
			vinc = (sys_rmode->viWidth - m) / 2;
			img = KeyboardInverted_img + (j * OSK_WIDTH * 5) + (i * 5);
			imginc = (OSK_WIDTH - m) * 5;
			for(j = 0; j < n; j++)
			{
				i = 0;
				while(i < m)
				{
					a = (*img);
					if(a == 0)
					{
						img += 5;
					} else
					{
						img++;
						c = *((u32*)img);
						img += 4;
						*v = c;
					};
					i += 2;
					v++;
				};
				v += vinc;
				img += imginc;
			};
		};
		if(osk_shiftpressed != 0)
		{
			i = osk_shiftpressed->left & ~1;
			j = osk_shiftpressed->top & ~1;
			m = (osk_shiftpressed->right & ~1) - i;
			n = (osk_shiftpressed->bottom & ~1) - j;
			v = ((u32*)(sys_framebuffer[1])) + ((sys_rmode->viHeight - OSK_HEIGHT + j + j) / 4 * sys_rmode->viWidth) 
			                                 + ((sys_rmode->viWidth - OSK_WIDTH + i + i) / 4);
			vinc = (sys_rmode->viWidth - m) / 2;
			img = KeyboardInverted_img + (j * OSK_WIDTH * 5) + (i * 5);
			imginc = (OSK_WIDTH - m) * 5;
			for(j = 0; j < n; j++)
			{
				i = 0;
				while(i < m)
				{
					a = (*img);
					if(a == 0)
					{
						img += 5;
					} else
					{
						img++;
						c = *((u32*)img);
						img += 4;
						*v = c;
					};
					i += 2;
					v++;
				};
				v += vinc;
				img += imginc;
			};
		};
		if(osk_capspressed != 0)
		{
			i = osk_capspressed->left & ~1;
			j = osk_capspressed->top & ~1;
			m = (osk_capspressed->right & ~1) - i;
			n = (osk_capspressed->bottom & ~1) - j;
			v = ((u32*)(sys_framebuffer[1])) + ((sys_rmode->viHeight - OSK_HEIGHT + j + j) / 4 * sys_rmode->viWidth) 
			                                 + ((sys_rmode->viWidth - OSK_WIDTH + i + i) / 4);
			vinc = (sys_rmode->viWidth - m) / 2;
			img = KeyboardInverted_img + (j * OSK_WIDTH * 5) + (i * 5);
			imginc = (OSK_WIDTH - m) * 5;
			for(j = 0; j < n; j++)
			{
				i = 0;
				while(i < m)
				{
					a = (*img);
					if(a == 0)
					{
						img += 5;
					} else
					{
						img++;
						c = *((u32*)img);
						img += 4;
						*v = c;
					};
					i += 2;
					v++;
				};
				v += vinc;
				img += imginc;
			};
		};
	};
}

void VID_DrawWmoteGuide(void)
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
	u32 c;
	u8 cr;
	u8 cg;
	u8 cb;
	
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
			a = 0;
			for(i = 0; i < 16; i++)
			{
				a = 3.14159265358979323846 * i / 8;
				x = p.x - r * cos(a + vid_guide_increment);
				y = p.y - r * sin(a + vid_guide_increment);
				if((x > vid_outerwidthborder)&&(x < (sys_rmode->viWidth - vid_outerwidthborder))&&(y > vid_outerheightborder)&&(y < (sys_rmode->viHeight - vid_outerheightborder)))
				{
					v = ((u32*)(sys_framebuffer[1])) + (y * sys_rmode->viWidth / 2) + (x / 2);
					if(d_8toy1cby2cr[vid_pal_increment][vid_pal_increment].count == vid_palentry_increment)
					{
						*v = d_8toy1cby2cr[vid_pal_increment][vid_pal_increment].color;
					} else
					{
						c = d_8to24table[vid_pal_increment];
						cr = c & 0xFF;
						cg = (c >> 8) & 0xFF;
						cb = (c >> 16) & 0xFF;
						f = VID_CvtRGB(cr, cg, cb, cr, cg, cb);
						*v = f;
						d_8toy1cby2cr[vid_pal_increment][vid_pal_increment].color = f;
						d_8toy1cby2cr[vid_pal_increment][vid_pal_increment].count = vid_palentry_increment;
					};
				};
				vid_pal_increment++;
				if(vid_pal_increment > 255)
				{
					vid_pal_increment = 0;
				};
			};
		};
		vid_guide_increment += 0.02;
		if(vid_guide_increment > 3.14159265358979323846)
		{
			vid_guide_increment -= 3.14159265358979323846;
		};
	};
}

void	VID_SetPalette (unsigned char *palette)
{
	int i;
	int j;
	unsigned char r;
	unsigned char g;
	unsigned char b;

	j = 0;
	for(i = 0; i < 256; i++)
	{
		r = palette[j];
		j++;
		g = palette[j];
		j++;
		b = palette[j];
		j++;
		d_8to16table[i] = 0x8000 | (((b >> 3) & 0x1F) << 10) | (((g >> 3) & 0x1F) << 5) | ((r >> 3) & 0x1F);
		d_8to24table[i] = 0xFF000000 | (b << 16) | (g << 8) | r;
	};
	vid_palentry_increment++;
}

void	VID_ShiftPalette (unsigned char *palette)
{
	VID_SetPalette(palette);
}

void	VID_Init (unsigned char *palette)
{
	float ws;
	float hs;
	int i;
	int j;
	int surfcache_len;

	ws = floor(((double)(sys_rmode->viWidth + 1)) / ((double)MAXWIDTH)) + 1.0;
	hs = floor(((double)(sys_rmode->viHeight + 1)) / ((double)MAXHEIGHT)) + 1.0;
	if(ws > hs)
	{
		vid_scale = ws;
	} else
	{
		vid_scale = hs;
	};
	vid_width = sys_rmode->viWidth / vid_scale;
	if(vid_width > MAXWIDTH)
	{
		vid_width = MAXWIDTH;
	};
	vid_height = sys_rmode->viHeight / vid_scale;
	if(vid_height > MAXHEIGHT)
	{
		vid_height = MAXHEIGHT;
	};
	
	// The following line applies a rough approximation of the Kell factor to the height of the screen.
	// Seems like the width doesn't really need it:
	vid_height = vid_height - (vid_height >> 3);

	vid_buffer = Sys_Malloc(sizeof(byte) * vid_width * vid_height, "VID_Init");
	zbuffer = Sys_Malloc(sizeof(short) * vid_width * vid_height, "VID_Init");
	vid.maxwarpwidth = vid.width = vid.conwidth = vid_width;
	vid.maxwarpheight = vid.height = vid.conheight = vid_height;
	vid.aspect = (((double)vid.width) / ((double)vid.height)) / (320.0 / 200.0);
	vid.numpages = 1;
	vid.colormap = host_colormap;
	vid.fullbright = 256 - LittleLong (*((int *)vid.colormap + 2048));
	vid.buffer = vid.conbuffer = vid_buffer;
	vid.rowbytes = vid.conrowbytes = vid_width;
	
	VID_SetPalette(palette);

	d_pzbuffer = zbuffer;
	surfcache_len = D_SurfaceCacheForRes(vid.width, vid.height);
	surfcache = Sys_Malloc(surfcache_len, "VID_Init");
	D_InitCaches (surfcache, surfcache_len);

	vid_finalwidth = vid_width * vid_scale;
	vid_finalheight = vid_height * vid_scale;

	vid_outerwidthborder = ((sys_rmode->viWidth - vid_finalwidth) / 2);
	vid_outerheightborder = ((sys_rmode->viHeight - vid_finalheight) / 2);

	vid_guide_increment = 0;
	vid_pal_increment = 0;

	for(i = 0; i < 255; i++)
	{
		for(j = 0; j < 255; j++)
		{
			d_8toy1cby2cr[i][j].count = 0;
		};
	};
	vid_palentry_increment = 1;
}

void	VID_Shutdown (void)
{
}

void	VID_Update (vrect_t *rects)
{
	vrect_t* r;
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
	byte pr;
	unsigned int c;
	u32 f;
	u8 r1;
	u8 g1;
	u8 b1;
	u8 r2;
	u8 g2;
	u8 b2;
	
	r = rects;
	while(r != 0)
	{
		i = r->y;
		j = r->x;
		h = r->height;
		w = r->width;
		p = vid_buffer + i * vid_height + j;
		v = ((u32*)(sys_framebuffer[1])) + ((vid_outerheightborder + (i * vid_scale)) * (sys_rmode->viWidth / 2)) + (vid_outerwidthborder / 2) + (j * vid_scale / 2);
		vinc = (sys_rmode->viWidth - (r->width * vid_scale)) / 2;
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
				if(jx >= vid_scale)
				{
					jx = 0;
					p++;
					j++;
				};
				pr = *p;
				jx++;
				if(jx >= vid_scale)
				{
					jx = 0;
					p++;
					j++;
				};
				if(d_8toy1cby2cr[pl][pr].count == vid_palentry_increment)
				{
					*v = d_8toy1cby2cr[pl][pr].color;
				} else
				{
					c = d_8to24table[pl];
					r1 = c & 0xFF;
					g1 = (c >> 8) & 0xFF;
					b1 = (c >> 16) & 0xFF;
					c = d_8to24table[pr];
					r2 = c & 0xFF;
					g2 = (c >> 8) & 0xFF;
					b2 = (c >> 16) & 0xFF;
					f = VID_CvtRGB(r1, g1, b1, r2, g2, b2);
					*v = f;
					d_8toy1cby2cr[pl][pr].color = f;
					d_8toy1cby2cr[pl][pr].count = vid_palentry_increment;
				};
				v++;
			};
			v += vinc;
			ix++;
			if(ix >= vid_scale)
			{
				ix = 0;
				i++;
				p += vid_width;
			};
			p -= w;
		};
		r = r->pnext;
	};
	VID_DrawOnScreenKeyboard();
	VID_DrawWmoteGuide();
}

/*
================
D_BeginDirectRect
================
*/
void D_BeginDirectRect (int x, int y, byte *pbitmap, int width, int height)
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
	byte pr;
	unsigned int c;
	u32 f;
	u8 r1;
	u8 g1;
	u8 b1;
	u8 r2;
	u8 g2;
	u8 b2;

	i = y;
	j = x;
	h = height;
	w = width;
	p = pbitmap;
	v = ((u32*)(sys_framebuffer[1])) + ((vid_outerheightborder + (i * vid_scale)) * (sys_rmode->viWidth / 2)) + (vid_outerwidthborder / 2) + (j * vid_scale / 2);
	vinc = (sys_rmode->viWidth - (w * vid_scale)) / 2;
	i = 0;
	ix = 0;
	while(i < h)
	{
		jx = 0;
		while(j < w)
		{
			pl = *p;
			jx++;
			if(jx >= vid_scale)
			{
				jx = 0;
				p++;
				j++;
			};
			pr = *p;
			jx++;
			if(jx >= vid_scale)
			{
				jx = 0;
				p++;
				j++;
			};
			if(d_8toy1cby2cr[pl][pr].count == vid_palentry_increment)
			{
				f = d_8toy1cby2cr[pl][pr].color;
			} else
			{
				c = d_8to24table[pl];
				r1 = c & 0xFF;
				g1 = (c >> 8) & 0xFF;
				b1 = (c >> 16) & 0xFF;
				c = d_8to24table[pr];
				r2 = c & 0xFF;
				g2 = (c >> 8) & 0xFF;
				b2 = (c >> 16) & 0xFF;
				f = VID_CvtRGB(r1, g1, b1, r2, g2, b2);
				d_8toy1cby2cr[pl][pr].color = f;
				d_8toy1cby2cr[pl][pr].count = vid_palentry_increment;
			};
			*v = f;
			v++;
		};
		v += vinc;
		ix++;
		if(ix >= vid_scale)
		{
			ix = 0;
			i++;
		} else
		{
			p -= w;
		};
	};
}


/*
================
D_EndDirectRect
================
*/
void D_EndDirectRect (int x, int y, int width, int height)
{
}

/*
================
VID_UpdateFullScreenStatus
================
*/
void VID_UpdateFullScreenStatus(void)
{
	ir_t p;

	WPAD_IR(WPAD_CHAN_0, &p);
	scr_copyeverything = (p.valid) || (in_osk.value);
}
// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// Include only for the software renderer builds (part 2):
#endif
// <<< FIX
