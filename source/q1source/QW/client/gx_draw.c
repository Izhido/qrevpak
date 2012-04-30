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

// draw.c -- this is the only file outside the refresh that touches the
// vid buffer

#ifdef GXQUAKE

#include <gccore.h>
#include <malloc.h>

#include "quakedef.h"

#include "gxutils.h"

extern cvar_t crosshair, cl_crossx, cl_crossy, crosshaircolor;

cvar_t		gl_nobind = {"gl_nobind", "0"};
cvar_t		gl_max_size = {"gl_max_size", "1024"};
cvar_t		gl_picmip = {"gl_picmip", "0"};

byte		*draw_chars;				// 8*8 graphic characters
qpic_t		*draw_disc;
qpic_t		*draw_backtile;

int			translate_texture;
int			char_texture;
int			cs_texture; // crosshair texture

static byte cs_data[64] = {
	0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xff,
	0xfe, 0xff, 0xfe, 0xff, 0xfe, 0xff, 0xfe, 0xff,
	0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};


typedef struct
{
	int		texnum;
	float	sl, tl, sh, th;
} gxpic_t;

byte		conback_buffer[sizeof(qpic_t) + sizeof(gxpic_t)];
qpic_t		*conback = (qpic_t *)&conback_buffer;

int		gx_lightmap_format = 4;
int		gx_solid_format = 3;
int		gx_alpha_format = 4;

u8		gx_filter_min = GX_LIN_MIP_NEAR;
u8		gx_filter_max = GX_LINEAR;

int		gx_tex_allocated; // To track amount of memory used for textures

int		texels;

typedef struct
{
	int		texnum;
	char	identifier[64];
	int		width, height;
	qboolean	mipmap;
} gxtexture_t;

typedef struct
{
	GXTexObj texobj;
	u16 width;
	u16 height;
	void* data;
	u32 length;
} gxtexobj_t;

#define	MAX_GXTEXTURES	1024
gxtexture_t	gxtextures[MAX_GXTEXTURES];
gxtexobj_t	gxtexobjs[MAX_GXTEXTURES];
int			numgxtextures;

static u8 oldtarget = GX_TEXMAP0;


void GX_Bind (int texnum)
{
	if (gl_nobind.value)
		texnum = char_texture;
	if (currenttexture == texnum)
		return;
	currenttexture = texnum;
	if(gxtexobjs[texnum].data != NULL)
		GX_LoadTexObj(&gxtexobjs[texnum].texobj, oldtarget - GX_TEXMAP0);
}

qboolean GX_ReallocTex(int length, int width, int height)
{
	qboolean changed = false;
	if(gxtexobjs[currenttexture].length < length)
	{
		if(gxtexobjs[currenttexture].data != NULL)
		{
			free(gxtexobjs[currenttexture].data);
			gxtexobjs[currenttexture].data = NULL;
			gx_tex_allocated -= gxtexobjs[currenttexture].length;
		};
		gxtexobjs[currenttexture].data = memalign(32, length);
		if(gxtexobjs[currenttexture].data == NULL)
		{
			Sys_Error("GX_ReallocTex: allocation failed on %i bytes", length);
		};
		gxtexobjs[currenttexture].length = length;
		gx_tex_allocated += length;
		changed = true;
	};
	gxtexobjs[currenttexture].width = width;
	gxtexobjs[currenttexture].height = height;
	return changed;
}

void GX_BindCurrentTex(qboolean changed, int format, int mipmap)
{
	DCFlushRange(gxtexobjs[currenttexture].data, gxtexobjs[currenttexture].length);
	GX_InitTexObj(&gxtexobjs[currenttexture].texobj, gxtexobjs[currenttexture].data, gxtexobjs[currenttexture].width, gxtexobjs[currenttexture].height, format, GX_REPEAT, GX_REPEAT, mipmap);
	GX_LoadTexObj(&gxtexobjs[currenttexture].texobj, oldtarget - GX_TEXMAP0);
	if(changed)
		GX_InvalidateTexAll();
}

void GX_LoadAndBind (void* data, int length, int width, int height, int format)
{
	qboolean changed = GX_ReallocTex(length, width, height);
	switch(format)
	{
	case GX_TF_RGBA8:
		GXU_CopyTexRGBA8((byte*)data, width, height, (byte*)(gxtexobjs[currenttexture].data));
		break;
	case GX_TF_RGB5A3:
		GXU_CopyTexRGB5A3((byte*)data, width, height, (byte*)(gxtexobjs[currenttexture].data));
		break;
	case GX_TF_I8:
	case GX_TF_A8:
		GXU_CopyTexV8((byte*)data, width, height, (byte*)(gxtexobjs[currenttexture].data));
		break;
	case GX_TF_IA4:
		GXU_CopyTexIA4((byte*)data, width, height, (byte*)(gxtexobjs[currenttexture].data));
		break;
	};
	GX_BindCurrentTex(changed, format, GX_FALSE);
}

void GX_LoadSubAndBind (void* data, int xoffset, int yoffset, int width, int height, int format)
{
	byte* dst;

	if(format == GX_TF_RGBA8)
	{
		dst = (byte*)(gxtexobjs[currenttexture].data);
		if(dst != NULL)
		{
			GXU_CopyTexSubRGBA8(data, width, height, dst, xoffset, yoffset, gxtexobjs[currenttexture].width, gxtexobjs[currenttexture].height);
		};
		GX_BindCurrentTex(true, format, GX_FALSE);
	} else if(format == GX_TF_RGB5A3)
	{
		dst = (byte*)(gxtexobjs[currenttexture].data);
		if(dst != NULL)
		{
			GXU_CopyTexSubRGB5A3(data, width, height, dst, xoffset, yoffset, gxtexobjs[currenttexture].width, gxtexobjs[currenttexture].height);
		};
		GX_BindCurrentTex(true, format, GX_FALSE);
	} else if((format == GX_TF_A8)||(format == GX_TF_I8))
	{
		dst = (byte*)(gxtexobjs[currenttexture].data);
		if(dst != NULL)
		{
			GXU_CopyTexSubV8(data, width, height, dst, xoffset, yoffset, gxtexobjs[currenttexture].width, gxtexobjs[currenttexture].height);
		};
		GX_BindCurrentTex(true, format, GX_FALSE);
	} else if(format == GX_TF_IA4)
	{
		dst = (byte*)(gxtexobjs[currenttexture].data);
		if(dst != NULL)
		{
			GXU_CopyTexSubIA4(data, width, height, dst, xoffset, yoffset, gxtexobjs[currenttexture].width, gxtexobjs[currenttexture].height);
		};
		GX_BindCurrentTex(true, format, GX_FALSE);
	};
}

void GX_SetMinMag (int minfilt, int magfilt)
{
	if(gxtexobjs[currenttexture].data != NULL)
	{
		GX_InitTexObjFilterMode(&gxtexobjs[currenttexture].texobj, minfilt, magfilt);
	};
}

void GX_Ortho(Mtx44 mtx, f32 top, f32 bottom, f32 left, f32 right, f32 nearP, f32 farP)
{
	f32 t;

	memset(mtx, 0, 4 * 4 * sizeof(f32));
	t = 1.0f / (right - left);
	mtx[0][0] = 2 * t;
	mtx[0][3] = (-right - left) * t;
	t = 1.0f / (top - bottom);
	mtx[1][1] = 2 * t;
	mtx[1][3] = (-top - bottom)*t;
	t = 1.0f / (farP - nearP);
	mtx[2][2] = -2 * t;
	mtx[2][3] = (-farP - nearP) * t;
	mtx[3][3] = 1;
}

/*
=============================================================================

  scrap allocation

  Allocate all the little status bar obejcts into a single texture
  to crutch up stupid hardware / drivers

=============================================================================
*/

#define	MAX_SCRAPS		1
#define	BLOCK_WIDTH		256
#define	BLOCK_HEIGHT	256

int			scrap_allocated[MAX_SCRAPS][BLOCK_WIDTH];
byte		scrap_texels[MAX_SCRAPS][BLOCK_WIDTH*BLOCK_HEIGHT*4];
qboolean	scrap_dirty;
int			scrap_texnum;

// returns a texture number and the position inside it
int Scrap_AllocBlock (int w, int h, int *x, int *y)
{
	int		i, j;
	int		best, best2;
	int		texnum;

	for (texnum=0 ; texnum<MAX_SCRAPS ; texnum++)
	{
		best = BLOCK_HEIGHT;

		for (i=0 ; i<BLOCK_WIDTH-w ; i++)
		{
			best2 = 0;

			for (j=0 ; j<w ; j++)
			{
				if (scrap_allocated[texnum][i+j] >= best)
					break;
				if (scrap_allocated[texnum][i+j] > best2)
					best2 = scrap_allocated[texnum][i+j];
			}
			if (j == w)
			{	// this is a valid spot
				*x = i;
				*y = best = best2;
			}
		}

		if (best + h > BLOCK_HEIGHT)
			continue;

		for (i=0 ; i<w ; i++)
			scrap_allocated[texnum][*x + i] = best + h;

		return texnum;
	}

	Sys_Error ("Scrap_AllocBlock: full");
	return 0;
}

int	scrap_uploads;

void Scrap_Upload (void)
{
	scrap_uploads++;
	GX_Bind(scrap_texnum);
	GX_Upload8 (scrap_texels[0], BLOCK_WIDTH, BLOCK_HEIGHT, false, true);
	scrap_dirty = false;
}

//=============================================================================
/* Support Routines */

typedef struct cachepic_s
{
	char		name[MAX_QPATH];
	qpic_t		pic;
	byte		padding[32];	// for appended gxpic
} cachepic_t;

#define	MAX_CACHED_PICS		128
cachepic_t	menu_cachepics[MAX_CACHED_PICS];
int			menu_numcachepics;

byte		menuplyr_pixels[4096];

int		pic_texels;
int		pic_count;

qpic_t *Draw_PicFromWad (char *name)
{
	qpic_t	*p;
	gxpic_t	*gx;

	p = W_GetLumpName (name);
	gx = (gxpic_t *)p->data;

	// load little ones into the scrap
	if (p->width < 64 && p->height < 64)
	{
		int		x, y;
		int		i, j, k;
		int		texnum;

		texnum = Scrap_AllocBlock (p->width, p->height, &x, &y);
		scrap_dirty = true;
		k = 0;
		for (i=0 ; i<p->height ; i++)
			for (j=0 ; j<p->width ; j++, k++)
				scrap_texels[texnum][(y+i)*BLOCK_WIDTH + x + j] = p->data[k];
		texnum += scrap_texnum;
		gx->texnum = texnum;
		gx->sl = (x+0.01)/(float)BLOCK_WIDTH;
		gx->sh = (x+p->width-0.01)/(float)BLOCK_WIDTH;
		gx->tl = (y+0.01)/(float)BLOCK_WIDTH;
		gx->th = (y+p->height-0.01)/(float)BLOCK_WIDTH;

		pic_count++;
		pic_texels += p->width*p->height;
	}
	else
	{
		gx->texnum = GX_LoadPicTexture (p);
		gx->sl = 0;
		gx->sh = 1;
		gx->tl = 0;
		gx->th = 1;
	}
	return p;
}


/*
================
Draw_CachePic
================
*/
qpic_t	*Draw_CachePic (char *path)
{
	cachepic_t	*pic;
	int			i;
	qpic_t		*dat;
	gxpic_t		*gx;

	for (pic=menu_cachepics, i=0 ; i<menu_numcachepics ; pic++, i++)
		if (!strcmp (path, pic->name))
			return &pic->pic;

	if (menu_numcachepics == MAX_CACHED_PICS)
		Sys_Error ("menu_numcachepics == MAX_CACHED_PICS");
	menu_numcachepics++;
	strcpy (pic->name, path);

//
// load the pic from disk
//
	dat = (qpic_t *)COM_LoadTempFile (path);	
	if (!dat)
		Sys_Error ("Draw_CachePic: failed to load %s", path);
	SwapPic (dat);

	// HACK HACK HACK --- we need to keep the bytes for
	// the translatable player picture just for the menu
	// configuration dialog
	if (!strcmp (path, "gfx/menuplyr.lmp"))
		memcpy (menuplyr_pixels, dat->data, dat->width*dat->height);

	pic->pic.width = dat->width;
	pic->pic.height = dat->height;

	gx = (gxpic_t *)pic->pic.data;
	gx->texnum = GX_LoadPicTexture (dat);
	gx->sl = 0;
	gx->sh = 1;
	gx->tl = 0;
	gx->th = 1;

	return &pic->pic;
}


void Draw_CharToConback (int num, byte *dest)
{
	int		row, col;
	byte	*source;
	int		drawline;
	int		x;

	row = num>>4;
	col = num&15;
	source = draw_chars + (row<<10) + (col<<3);

	drawline = 8;

	while (drawline--)
	{
		for (x=0 ; x<8 ; x++)
			if (source[x] != 255)
				dest[x] = 0x60 + source[x];
		source += 128;
		dest += 320;
	}

}

typedef struct
{
	char *name;
	u8	minimize, maximize;
} gxmode_t;

gxmode_t modes[] = {
	{"GX_NEAR", GX_NEAR, GX_NEAR},
	{"GX_LINEAR", GX_LINEAR, GX_LINEAR},
	{"GX_NEAR_MIP_NEAR", GX_NEAR_MIP_NEAR, GX_NEAR},
	{"GX_LIN_MIP_NEAR", GX_LIN_MIP_NEAR, GX_LINEAR},
	{"GX_NEAR_MIP_LIN", GX_NEAR_MIP_LIN, GX_NEAR},
	{"GX_LIN_MIP_LIN", GX_LIN_MIP_LIN, GX_LINEAR}
};

/*
===============
Draw_TextureMode_f
===============
*/
void Draw_TextureMode_f (void)
{
	int		i;
	gxtexture_t	*gxt;

	if (Cmd_Argc() == 1)
	{
		for (i=0 ; i< 6 ; i++)
			if (gx_filter_min == modes[i].minimize)
			{
				Con_Printf ("%s\n", modes[i].name);
				return;
			}
		Con_Printf ("current filter is unknown???\n");
		return;
	}

	for (i=0 ; i< 6 ; i++)
	{
		if (!Q_strcasecmp (modes[i].name, Cmd_Argv(1) ) )
			break;
	}
	if (i == 6)
	{
		Con_Printf ("bad filter name\n");
		return;
	}

	gx_filter_min = modes[i].minimize;
	gx_filter_max = modes[i].maximize;

	// change all the existing mipmap texture objects
	for (i=0, gxt=gxtextures ; i<numgxtextures ; i++, gxt++)
	{
		if (gxt->mipmap)
		{
			GX_Bind (gxt->texnum);
			GX_SetMinMag (gx_filter_min, gx_filter_max);
		}
	}
}

/*
===============
Draw_Init
===============
*/
void Draw_Init (void)
{
	int		i;
	qpic_t	*cb;
	byte	*dest;
	int		x;
	char	ver[40];
	gxpic_t	*gx;
	int start;
	byte    *ncdata;
	memset(gxtexobjs, 0, sizeof(gxtexobjs));

	Cvar_RegisterVariable (&gl_nobind);
	Cvar_RegisterVariable (&gl_max_size);
	Cvar_RegisterVariable (&gl_picmip);

	Cmd_AddCommand ("gl_texturemode", &Draw_TextureMode_f);

	// load the console background and the charset
	// by hand, because we need to write the version
	// string into the background before turning
	// it into a texture
	draw_chars = W_GetLumpName ("conchars");
	for (i=0 ; i<256*64 ; i++)
		if (draw_chars[i] == 0)
			draw_chars[i] = 255;	// proper transparent color

	// now turn them into textures
	char_texture = GX_LoadTexture ("charset", 128, 128, draw_chars, false, true);
//	Draw_CrosshairAdjust();
	cs_texture = GX_LoadTexture ("crosshair", 8, 8, cs_data, false, true);

	start = Hunk_LowMark ();

	cb = (qpic_t *)COM_LoadHunkFile ("gfx/conback.lmp");	
	if (!cb)
		Sys_Error ("Couldn't load gfx/conback.lmp");
	SwapPic (cb);

	sprintf (ver, "%4.2f", VERSION);
	dest = cb->data + 320 + 320*186 - 11 - 8*strlen(ver);
	for (x=0 ; x<strlen(ver) ; x++)
		Draw_CharToConback (ver[x], dest+(x<<3));

#if 0
	conback->width = vid.conwidth;
	conback->height = vid.conheight;

	// scale console to vid size
	dest = ncdata = Hunk_AllocName(vid.conwidth * vid.conheight, "conback");

	for (y=0 ; y<vid.conheight ; y++, dest += vid.conwidth)
	{
		src = cb->data + cb->width * (y*cb->height/vid.conheight);
		if (vid.conwidth == cb->width)
			memcpy (dest, src, vid.conwidth);
		else
		{
			f = 0;
			fstep = cb->width*0x10000/vid.conwidth;
			for (x=0 ; x<vid.conwidth ; x+=4)
			{
				dest[x] = src[f>>16];
				f += fstep;
				dest[x+1] = src[f>>16];
				f += fstep;
				dest[x+2] = src[f>>16];
				f += fstep;
				dest[x+3] = src[f>>16];
				f += fstep;
			}
		}
	}
#else
	conback->width = cb->width;
	conback->height = cb->height;
	ncdata = cb->data;
#endif

	GX_SetMinMag (GX_NEAR, GX_NEAR);

	gx = (gxpic_t *)conback->data;
	gx->texnum = GX_LoadTexture ("conback", conback->width, conback->height, ncdata, false, false);
	gx->sl = 0;
	gx->sh = 1;
	gx->tl = 0;
	gx->th = 1;
	conback->width = vid.conwidth;
	conback->height = vid.conheight;

	// free loaded console
	Hunk_FreeToLowMark (start);

	// save a texture slot for translated picture
	translate_texture = texture_extension_number++;

	// save slots for scraps
	scrap_texnum = texture_extension_number;
	texture_extension_number += MAX_SCRAPS;

	//
	// get the other pics we need
	//
	draw_disc = Draw_PicFromWad ("disc");
	draw_backtile = Draw_PicFromWad ("backtile");
}



/*
================
Draw_Character

Draws one 8*8 graphics character with 0 being transparent.
It can be clipped to the top of the screen to allow the console to be
smoothly scrolled off.
================
*/
void Draw_Character (int x, int y, int num)
{
	int				row, col;
	float			frow, fcol, size;

	if (num == 32)
		return;		// space

	num &= 255;
	
	if (y <= -8)
		return;			// totally off screen

	row = num>>4;
	col = num&15;

	frow = row*0.0625;
	fcol = col*0.0625;
	size = 0.0625;

	GX_Bind (char_texture);

	GX_Begin (GX_QUADS, gxu_cur_vertex_format, 4);
	GX_Position3f32(x, y, 0);
	GX_Color4u8(gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
	GX_TexCoord2f32 (fcol, frow);
	GX_Position3f32(x+8, y, 0);
	GX_Color4u8(gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
	GX_TexCoord2f32 (fcol + size, frow);
	GX_Position3f32(x+8, y+8, 0);
	GX_Color4u8(gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
	GX_TexCoord2f32 (fcol + size, frow + size);
	GX_Position3f32(x, y+8, 0);
	GX_Color4u8(gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
	GX_TexCoord2f32 (fcol, frow + size);
	GX_End ();
}

/*
================
Draw_String
================
*/
void Draw_String (int x, int y, char *str)
{
	while (*str)
	{
		Draw_Character (x, y, *str);
		str++;
		x += 8;
	}
}

/*
================
Draw_Alt_String
================
*/
void Draw_Alt_String (int x, int y, char *str)
{
	while (*str)
	{
		Draw_Character (x, y, (*str) | 0x80);
		str++;
		x += 8;
	}
}

void Draw_Crosshair(void)
{
	int x, y;
	extern vrect_t		scr_vrect;
	unsigned char *pColor;

	if (crosshair.value == 2) {
		x = scr_vrect.x + scr_vrect.width/2 - 3 + cl_crossx.value; 
		y = scr_vrect.y + scr_vrect.height/2 - 3 + cl_crossy.value;

		GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
		pColor = (unsigned char *) &d_8to24table[(byte) crosshaircolor.value];
		gxu_cur_r = *pColor;
		gxu_cur_g = *(pColor + 1);
		gxu_cur_b = *(pColor + 2);
		gxu_cur_a = *(pColor + 3);
		GX_Bind (cs_texture);

		GX_Begin (GX_QUADS, gxu_cur_vertex_format, 4);
		GX_Position3f32(x - 4, y - 4, 0);
		GX_Color4u8(gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
		GX_TexCoord2f32 (0, 0);
		GX_Position3f32(x+12, y-4, 0);
		GX_Color4u8(gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
		GX_TexCoord2f32 (1, 0);
		GX_Position3f32(x+12, y+12, 0);
		GX_Color4u8(gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
		GX_TexCoord2f32 (1, 1);
		GX_Position3f32(x - 4, y+12, 0);
		GX_Color4u8(gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
		GX_TexCoord2f32 (0, 1);
		GX_End ();

		GX_SetTevOp(GX_TEVSTAGE0, GX_REPLACE);
	} else if (crosshair.value)
		Draw_Character (scr_vrect.x + scr_vrect.width/2-4 + cl_crossx.value, 
			scr_vrect.y + scr_vrect.height/2-4 + cl_crossy.value, 
			'+');
}


/*
================
Draw_DebugChar

Draws a single character directly to the upper right corner of the screen.
This is for debugging lockups by drawing different chars in different parts
of the code.
================
*/
void Draw_DebugChar (char num)
{
}

/*
=============
Draw_Pic
=============
*/
void Draw_Pic (int x, int y, qpic_t *pic)
{
	gxpic_t			*gx;

	if (scrap_dirty)
		Scrap_Upload ();
	gx = (gxpic_t *)pic->data;
	gxu_cur_r = 255;
	gxu_cur_g = 255;
	gxu_cur_b = 255;
	gxu_cur_a = 255;
	GX_Bind (gx->texnum);
	GX_Begin (GX_QUADS, gxu_cur_vertex_format, 4);
	GX_Position3f32(x, y, 0);
	GX_Color4u8(gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
	GX_TexCoord2f32 (gx->sl, gx->tl);
	GX_Position3f32(x+pic->width, y, 0);
	GX_Color4u8(gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
	GX_TexCoord2f32 (gx->sh, gx->tl);
	GX_Position3f32(x+pic->width, y+pic->height, 0);
	GX_Color4u8(gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
	GX_TexCoord2f32 (gx->sh, gx->th);
	GX_Position3f32(x, y+pic->height, 0);
	GX_Color4u8(gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
	GX_TexCoord2f32 (gx->sl, gx->th);
	GX_End ();
}

/*
=============
Draw_AlphaPic
=============
*/
void Draw_AlphaPic (int x, int y, qpic_t *pic, float alpha)
{
	byte			*dest, *source;
	unsigned short	*pusdest;
	int				v, u;
	gxpic_t			*gx;

	if (scrap_dirty)
		Scrap_Upload ();
	gx = (gxpic_t *)pic->data;
	GX_SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
	GX_SetBlendMode(GX_BM_BLEND, gxu_blend_src_value, gxu_blend_dst_value, GX_LO_NOOP);
	gxu_cur_r = 255;
	gxu_cur_g = 255;
	gxu_cur_b = 255;
	if(alpha < 0.0)
		alpha = 0.0;
	if(alpha > 1.0)
		alpha = 1.0;
	gxu_cur_a = alpha * 255.0;
	GX_Bind (gx->texnum);
	GX_Begin (GX_QUADS, gxu_cur_vertex_format, 4);
	GX_Position3f32(x, y, 0);
	GX_Color4u8(gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
	GX_TexCoord2f32 (gx->sl, gx->tl);
	GX_Position3f32(x+pic->width, y, 0);
	GX_Color4u8(gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
	GX_TexCoord2f32 (gx->sh, gx->tl);
	GX_Position3f32(x+pic->width, y+pic->height, 0);
	GX_Color4u8(gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
	GX_TexCoord2f32 (gx->sh, gx->th);
	GX_Position3f32(x, y+pic->height, 0);
	GX_Color4u8(gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
	GX_TexCoord2f32 (gx->sl, gx->th);
	GX_End ();
	gxu_cur_a = 255;
	GX_SetAlphaCompare(GX_GEQUAL, gxu_alpha_test_lower, GX_AOP_AND, GX_LEQUAL, gxu_alpha_test_higher);
	GX_SetBlendMode(GX_BM_NONE, gxu_blend_src_value, gxu_blend_dst_value, GX_LO_NOOP); 
}

void Draw_SubPic(int x, int y, qpic_t *pic, int srcx, int srcy, int width, int height)
{
	gxpic_t			*gx;
	float newsl, newtl, newsh, newth;
	float oldgxwidth, oldgxheight;

	if (scrap_dirty)
		Scrap_Upload ();
	gx = (gxpic_t *)pic->data;
	
	oldgxwidth = gx->sh - gx->sl;
	oldgxheight = gx->th - gx->tl;

	newsl = gx->sl + (srcx*oldgxwidth)/pic->width;
	newsh = newsl + (width*oldgxwidth)/pic->width;

	newtl = gx->tl + (srcy*oldgxheight)/pic->height;
	newth = newtl + (height*oldgxheight)/pic->height;
	
	gxu_cur_r = 255;
	gxu_cur_g = 255;
	gxu_cur_b = 255;
	gxu_cur_a = 255;
	GX_Bind (gx->texnum);
	GX_Begin (GX_QUADS, gxu_cur_vertex_format, 4);
	GX_Position3f32(x, y, 0);
	GX_Color4u8(gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
	GX_TexCoord2f32 (newsl, newtl);
	GX_Position3f32(x+width, y, 0);
	GX_Color4u8(gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
	GX_TexCoord2f32 (newsh, newtl);
	GX_Position3f32(x+width, y+height, 0);
	GX_Color4u8(gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
	GX_TexCoord2f32 (newsh, newth);
	GX_Position3f32(x, y+height, 0);
	GX_Color4u8(gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
	GX_TexCoord2f32 (newsl, newth);
	GX_End ();
}

/*
=============
Draw_TransPic
=============
*/
void Draw_TransPic (int x, int y, qpic_t *pic)
{

	if (x < 0 || (unsigned)(x + pic->width) > vid.width || y < 0 ||
		 (unsigned)(y + pic->height) > vid.height)
	{
		Sys_Error ("Draw_TransPic: bad coordinates");
	}
		
	Draw_Pic (x, y, pic);
}


/*
=============
Draw_TransPicTranslate

Only used for the player color selection menu
=============
*/
void Draw_TransPicTranslate (int x, int y, qpic_t *pic, byte *translation)
{
	int				v, u, c;
	unsigned*		trans = Sys_BigStackAlloc(64*64 * sizeof(unsigned), "Draw_TransPicTranslate");
	unsigned *dest;
	byte			*src;
	int				p;

	GX_Bind (translate_texture);

	c = pic->width * pic->height;

	dest = trans;
	for (v=0 ; v<64 ; v++, dest += 64)
	{
		src = &menuplyr_pixels[ ((v*pic->height)>>6) *pic->width];
		for (u=0 ; u<64 ; u++)
		{
			p = src[(u*pic->width)>>6];
			if (p == 255)
				dest[u] = p;
			else
				dest[u] =  d_8to24table[translation[p]];
		}
	}

	GX_LoadAndBind (trans, 64*64 * sizeof(unsigned), 64, 64, GX_TF_RGBA8);

	GX_SetMinMag (GX_LINEAR, GX_LINEAR);

	gxu_cur_r = 255;
	gxu_cur_g = 255;
	gxu_cur_b = 255;
	gxu_cur_a = 255;
	GX_Begin (GX_QUADS, gxu_cur_vertex_format, 4);
	GX_Position3f32(x, y, 0);
	GX_Color4u8(gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
	GX_TexCoord2f32 (0, 0);
	GX_Position3f32(x+pic->width, y, 0);
	GX_Color4u8(gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
	GX_TexCoord2f32 (1, 0);
	GX_Position3f32(x+pic->width, y+pic->height, 0);
	GX_Color4u8(gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
	GX_TexCoord2f32 (1, 1);
	GX_Position3f32(x, y+pic->height, 0);
	GX_Color4u8(gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
	GX_TexCoord2f32 (0, 1);
	GX_End ();
	Sys_BigStackFree(64*64 * sizeof(unsigned), "Draw_TransPicTranslate");
}


/*
================
Draw_ConsoleBackground

================
*/
void Draw_ConsoleBackground (int lines)
{
	char ver[80];
	int x, i;
	int y;

	y = (vid.height * 3) >> 2;
	if (lines > y)
		Draw_Pic(0, lines-vid.height, conback);
	else
		Draw_AlphaPic (0, lines - vid.height, conback, (float)(1.2 * lines)/y);

	// hack the version number directly into the pic
//	y = lines-186;
	y = lines-14;
	if (!cls.download) {
#ifdef __linux__
		sprintf (ver, "LinuxGL (%4.2f) QuakeWorld", LINUX_VERSION);
#else
		sprintf (ver, "GL (%4.2f) QuakeWorld", GLQUAKE_VERSION);
#endif
		x = vid.conwidth - (strlen(ver)*8 + 11) - (vid.conwidth*8/320)*7;
		for (i=0 ; i<strlen(ver) ; i++)
			Draw_Character (x + i * 8, y, ver[i] | 0x80);
	}
}


/*
=============
Draw_TileClear

This repeats a 64*64 tile graphic to fill the screen around a sized down
refresh window.
=============
*/
void Draw_TileClear (int x, int y, int w, int h)
{
	gxu_cur_r = 255;
	gxu_cur_g = 255;
	gxu_cur_b = 255;
	gxu_cur_a = 255;
	GX_Bind (*(int *)draw_backtile->data);
	GX_Begin (GX_QUADS, gxu_cur_vertex_format, 4);
	GX_Position3f32(x, y, 0);
	GX_Color4u8(gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
	GX_TexCoord2f32 (x/64.0, y/64.0);
	GX_Position3f32(x+w, y, 0);
	GX_Color4u8(gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
	GX_TexCoord2f32 ( (x+w)/64.0, y/64.0);
	GX_Position3f32(x+w, y+h, 0);
	GX_Color4u8(gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
	GX_TexCoord2f32 ( (x+w)/64.0, (y+h)/64.0);
	GX_Position3f32(x, y+h, 0);
	GX_Color4u8(gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
	GX_TexCoord2f32 (x/64.0, (y+h)/64.0);
	GX_End ();
}


/*
=============
Draw_Fill

Fills a box of pixels with a single color
=============
*/
void Draw_Fill (int x, int y, int w, int h, int c)
{
	u8 r, g, b;

	r = host_basepal[c*3];
	g = host_basepal[c*3+1];
	b = host_basepal[c*3+2];
	GXU_DisableTexture();
	GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
	GX_Position3f32(x, y, 0);
	GX_Color4u8(r, g, b, 255);
	GX_Position3f32(x+w, y, 0);
	GX_Color4u8(r, g, b, 255);
	GX_Position3f32(x+w, y+h, 0);
	GX_Color4u8(r, g, b, 255);
	GX_Position3f32(x, y+h, 0);
	GX_Color4u8(r, g, b, 255);
	GX_End();
	GXU_EnableTexture();
}
//=============================================================================

/*
================
Draw_FadeScreen

================
*/
void Draw_FadeScreen (void)
{
	GX_SetBlendMode(GX_BM_BLEND, gxu_blend_src_value, gxu_blend_dst_value, GX_LO_NOOP); 
	GXU_DisableTexture();
	GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
	GX_Position3f32(0, 0, 0);
	GX_Color4u8(0, 0, 0, 204);
	GX_Position3f32(vid.width, 0, 0);
	GX_Color4u8(0, 0, 0, 204);
	GX_Position3f32(vid.width, vid.height, 0);
	GX_Color4u8(0, 0, 0, 204);
	GX_Position3f32(0, vid.height, 0);
	GX_Color4u8(0, 0, 0, 204);
	GX_End();
	GXU_EnableTexture();
	GX_SetBlendMode(GX_BM_NONE, gxu_blend_src_value, gxu_blend_dst_value, GX_LO_NOOP); 

	Sbar_Changed();
}

//=============================================================================

/*
================
Draw_BeginDisc

Draws the little blue disc in the corner of the screen.
Call before beginning any disc IO.
================
*/
void Draw_BeginDisc (void)
{/*
	if (!draw_disc)
		return;
	glDrawBuffer  (GL_FRONT);
	Draw_Pic (vid.width - 24, 0, draw_disc);
	glDrawBuffer  (GL_BACK);
*/}


/*
================
Draw_EndDisc

Erases the disc icon.
Call after completing any disc IO
================
*/
void Draw_EndDisc (void)
{
}

/*
================
GX_Set2D

Setup as if the screen was 320*200
================
*/
void GX_Set2D (void)
{
	gxu_viewport_x = gxx;
	gxu_viewport_y = gxy;
	gxu_viewport_width = gxwidth;
	gxu_viewport_height = gxheight;
	GX_SetViewport (gxu_viewport_x, gxu_viewport_y, gxu_viewport_width, gxu_viewport_height, gxdepthmin, gxdepthmax);

	GX_Ortho(gxu_projection_matrices[gxu_cur_projection_matrix], 0, vid.height, 0, vid.width, 0, 300); //-99999, 99999);
	GX_LoadProjectionMtx(gxu_projection_matrices[gxu_cur_projection_matrix], GX_ORTHOGRAPHIC);

	guMtxIdentity(gxu_modelview_matrices[gxu_cur_modelview_matrix]);
	GX_LoadPosMtxImm(gxu_modelview_matrices[gxu_cur_modelview_matrix], GX_PNMTX0);

	gxu_z_test_enabled = GX_FALSE;
	GX_SetZMode(gxu_z_test_enabled, gxu_cur_z_func, gxu_z_write_enabled);
	gxu_cull_enabled = false;
	GX_SetCullMode(GX_CULL_NONE);
	GX_SetBlendMode(GX_BM_NONE, gxu_blend_src_value, gxu_blend_dst_value, GX_LO_NOOP);
	gxu_alpha_test_enabled = true;
	GX_SetAlphaCompare(GX_GEQUAL, gxu_alpha_test_lower, GX_AOP_AND, GX_LEQUAL, gxu_alpha_test_higher);
//		gxu_alpha_test_enabled = false;
//		GX_SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);


	gxu_cur_r = 255;
	gxu_cur_g = 255;
	gxu_cur_b = 255;
	gxu_cur_a = 255;
}

//====================================================================

/*
================
GX_FindTexture
================
*/
int GX_FindTexture (char *identifier)
{
	int		i;
	gxtexture_t	*gxt;

	for (i=0, gxt=gxtextures ; i<numgxtextures ; i++, gxt++)
	{
		if (!strcmp (identifier, gxt->identifier))
			return gxtextures[i].texnum;
	}

	return -1;
}

/*
================
GX_ResampleTexture
================
*/
void GX_ResampleTexture (unsigned *in, int inwidth, int inheight, unsigned *out,  int outwidth, int outheight)
{
	int		i, j;
	unsigned	*inrow;
	unsigned	frac, fracstep;

	fracstep = inwidth*0x10000/outwidth;
	for (i=0 ; i<outheight ; i++, out += outwidth)
	{
		inrow = in + inwidth*(i*inheight/outheight);
		frac = fracstep >> 1;
		for (j=0 ; j<outwidth ; j+=4)
		{
			out[j] = inrow[frac>>16];
			frac += fracstep;
			out[j+1] = inrow[frac>>16];
			frac += fracstep;
			out[j+2] = inrow[frac>>16];
			frac += fracstep;
			out[j+3] = inrow[frac>>16];
			frac += fracstep;
		}
	}
}

/*
================
GX_Resample8BitTexture -- JACK
================
*/
void GX_Resample8BitTexture (unsigned char *in, int inwidth, int inheight, unsigned char *out,  int outwidth, int outheight)
{
	int		i, j;
	unsigned	char *inrow;
	unsigned	frac, fracstep;

	fracstep = inwidth*0x10000/outwidth;
	for (i=0 ; i<outheight ; i++, out += outwidth)
	{
		inrow = in + inwidth*(i*inheight/outheight);
		frac = fracstep >> 1;
		for (j=0 ; j<outwidth ; j+=4)
		{
			out[j] = inrow[frac>>16];
			frac += fracstep;
			out[j+1] = inrow[frac>>16];
			frac += fracstep;
			out[j+2] = inrow[frac>>16];
			frac += fracstep;
			out[j+3] = inrow[frac>>16];
			frac += fracstep;
		}
	}
}

/*
================
GX_MipMap

Operates in place, quartering the size of the texture
================
*/
void GX_MipMap (byte *in, int width, int height)
{
	int		i, j;
	byte	*out;

	width <<=2;
	height >>= 1;
	out = in;
	for (i=0 ; i<height ; i++, in+=width)
	{
		for (j=0 ; j<width ; j+=8, out+=4, in+=8)
		{
			out[0] = (in[0] + in[4] + in[width+0] + in[width+4])>>2;
			out[1] = (in[1] + in[5] + in[width+1] + in[width+5])>>2;
			out[2] = (in[2] + in[6] + in[width+2] + in[width+6])>>2;
			out[3] = (in[3] + in[7] + in[width+3] + in[width+7])>>2;
		}
	}
}

/*
===============
GX_Upload32
===============
*/
void GX_Upload32 (unsigned *data, int length, int width, int height,  qboolean mipmap)
{
static	unsigned	scaled[1024*512];	// [512*256];
	int			scaled_width, scaled_height;

	for (scaled_width = 1 ; scaled_width < width ; scaled_width<<=1)
		;
	for (scaled_height = 1 ; scaled_height < height ; scaled_height<<=1)
		;

	scaled_width >>= (int)gl_picmip.value;
	scaled_height >>= (int)gl_picmip.value;

	if (scaled_width > gl_max_size.value)
		scaled_width = gl_max_size.value;
	if (scaled_height > gl_max_size.value)
		scaled_height = gl_max_size.value;

	// This is required in order to make small textures compatible with GX hardware:
	if (scaled_width < 4)
		scaled_width = 4;
	if (scaled_height < 4)
		scaled_height = 4;

	if (scaled_width * scaled_height > sizeof(scaled)/4)
		Sys_Error ("GX_LoadTexture: too big");

#if 0
	if (mipmap)
		gluBuild2DMipmaps (GL_TEXTURE_2D, samples, width, height, GL_RGBA, GL_UNSIGNED_BYTE, trans);
	else if (scaled_width == width && scaled_height == height)
		glTexImage2D (GL_TEXTURE_2D, 0, samples, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, trans);
	else
	{
		gluScaleImage (GL_RGBA, width, height, GL_UNSIGNED_BYTE, trans,
			scaled_width, scaled_height, GL_UNSIGNED_BYTE, scaled);
		glTexImage2D (GL_TEXTURE_2D, 0, samples, scaled_width, scaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, scaled);
	}
#else
texels += scaled_width * scaled_height;

	if (scaled_width == width && scaled_height == height)
	{
		if (!mipmap)
		{
			GX_LoadAndBind (data, length, scaled_width, scaled_height, GX_TF_RGBA8);
			goto done;
		}
		memcpy (scaled, data, width*height*4);
	}
	else
		GX_ResampleTexture (data, width, height, scaled, scaled_width, scaled_height);

	if (mipmap)
	{
		int		miplevel;
		int sw;
		int sh;
		u32 scaledlen;
		qboolean changed;
		byte* dst;

		miplevel = 1;
		sw = scaled_width;
		sh = scaled_height;
		while (sw > 4 && sh > 4)
		{
			sw >>= 1;
			sh >>= 1;
			miplevel++;
		};
		scaledlen = GX_GetTexBufferSize(scaled_width, scaled_height, GX_TF_RGBA8, GX_TRUE, miplevel);
		changed = GX_ReallocTex(scaledlen, scaled_width, scaled_height);
		dst = GXU_CopyTexRGBA8((byte*)scaled, scaled_width, scaled_height, (byte*)(gxtexobjs[currenttexture].data));
		while (scaled_width > 4 && scaled_height > 4)
		{
			GX_MipMap ((byte *)scaled, scaled_width, scaled_height);
			scaled_width >>= 1;
			scaled_height >>= 1;
			dst = GXU_CopyTexRGBA8((byte*)scaled, scaled_width, scaled_height, dst);
		};
		GX_BindCurrentTex(changed, GX_TF_RGBA8, GX_TRUE);
	} else
		GX_LoadAndBind (scaled, length * scaled_width / width * scaled_height / height, scaled_width, scaled_height, GX_TF_RGBA8);

done: ;
#endif


	if (mipmap)
		GX_SetMinMag (gx_filter_min, gx_filter_max);
	else
		GX_SetMinMag (gx_filter_max, gx_filter_max);
}

/*
===============
GX_Upload8
===============
*/
void GX_Upload8 (byte *data, int width, int height,  qboolean mipmap, qboolean alpha)
{
static	unsigned	trans[640*480];		// FIXME, temporary
	int			i, s;
	qboolean	noalpha;
	int			p;

	s = width*height;
	// if there are no transparent pixels, make it a 3 component
	// texture even if it was specified as otherwise
	if (alpha)
	{
		noalpha = true;
		for (i=0 ; i<s ; i++)
		{
			p = data[i];
			if (p == 255)
				noalpha = false;
			trans[i] = d_8to24table[p];
		}

		if (alpha && noalpha)
			alpha = false;
	}
	else
	{
		if (s&3)
			Sys_Error ("GX_Upload8: s&3");
		for (i=0 ; i<s ; i+=4)
		{
			trans[i] = d_8to24table[data[i]];
			trans[i+1] = d_8to24table[data[i+1]];
			trans[i+2] = d_8to24table[data[i+2]];
			trans[i+3] = d_8to24table[data[i+3]];
		}
	}

	GX_Upload32 (trans, s * 4, width, height, mipmap);
}

/*
================
GX_LoadTexture
================
*/
int GX_LoadTexture (char *identifier, int width, int height, byte *data, qboolean mipmap, qboolean alpha)
{
	int			i;
	gxtexture_t	*gxt;

	// see if the texture is allready present
	if (identifier[0])
	{
		for (i=0, gxt=gxtextures ; i<numgxtextures ; i++, gxt++)
		{
			if (!strcmp (identifier, gxt->identifier))
			{
				if (width != gxt->width || height != gxt->height)
					Sys_Error ("GX_LoadTexture: cache mismatch");
				return gxtextures[i].texnum;
			}
		}
	}
	else
		gxt = &gxtextures[numgxtextures];
	numgxtextures++;
	if(numgxtextures > MAX_GXTEXTURES)
		Sys_Error ("GX_LoadTexture: Too many textures!");

	strcpy (gxt->identifier, identifier);
	gxt->texnum = texture_extension_number;
	gxt->width = width;
	gxt->height = height;
	gxt->mipmap = mipmap;

	GX_Bind(texture_extension_number );

	GX_Upload8 (data, width, height, mipmap, alpha);

	texture_extension_number++;

	return texture_extension_number-1;
}

/*
================
GX_LoadPicTexture
================
*/
int GX_LoadPicTexture (qpic_t *pic)
{
	return GX_LoadTexture ("", pic->width, pic->height, pic->data, false, true);
}

/****************************************/

void GX_SelectTexture (u8 target) 
{
	if (!gx_mtexable)
		return;
	if (target == oldtarget) 
		return;
	cnttextures[oldtarget-GX_TEXMAP0] = currenttexture;
	currenttexture = cnttextures[target-GX_TEXMAP0];
	oldtarget = target;
	if(currenttexture >= 0)
		if(gxtexobjs[currenttexture].data != NULL)
			GX_LoadTexObj(&gxtexobjs[currenttexture].texobj, oldtarget - GX_TEXMAP0);
}

#endif

