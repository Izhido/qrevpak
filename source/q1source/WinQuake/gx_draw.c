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

#define GL_COLOR_INDEX8_EXT     0x80E5

extern unsigned char d_15to8table[65536];

extern Mtx44 gx_projection_matrix;

extern Mtx gx_modelview_matrices[32];

extern int gx_cur_modelview_matrix;

extern qboolean gx_cull_enabled;

extern u8 gx_z_test_enabled;

extern u8 gx_z_write_enabled;

extern qboolean gx_blend_enabled;

extern u8 gx_blend_src_value;

extern u8 gx_blend_dst_value;


cvar_t		gx_nobind = {"gx_nobind", "0"};
cvar_t		gx_max_size = {"gx_max_size", "1024"};
cvar_t		gx_picmip = {"gx_picmip", "0"};

#define		GX_MAX_MIPMAPS 10			// This is related to gx_max_size defined above

byte		*draw_chars;				// 8*8 graphic characters
qpic_t		*draw_disc;
qpic_t		*draw_backtile;

int			translate_texture;
int			char_texture;

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

int		gx_filter_min = GL_LINEAR_MIPMAP_NEAREST;
int		gx_filter_max = GL_LINEAR;


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
	void* data[GX_MAX_MIPMAPS];
	int length[GX_MAX_MIPMAPS];
} gxtexobj_t;

#define	MAX_GXTEXTURES	1024
gxtexture_t	gxtextures[MAX_GXTEXTURES];
gxtexobj_t	gxtexobjs[MAX_GXTEXTURES];
int			numgxtextures;


void GX_Bind (int texnum)
{
	if (gx_nobind.value)
		texnum = char_texture;
	if (currenttexture == texnum)
		return;
	currenttexture = texnum;
	if(gxtexobjs[texnum].data[0] != NULL)
		GX_LoadTexObj(&gxtexobjs[texnum].texobj, GX_TEXMAP0);
}

void GX_LoadAndBind (void* data, int length, int width, int height, int format, int mipmap)
{
	qboolean changed;

	changed = false;
	if(gxtexobjs[currenttexture].length[mipmap] < length)
	{
		if(gxtexobjs[currenttexture].data[mipmap] != NULL)
		{
			free(gxtexobjs[currenttexture].data[mipmap]);
			gxtexobjs[currenttexture].data[mipmap] = NULL;
			changed = true;
		};
		gxtexobjs[currenttexture].data[mipmap] = memalign(32, length);
		if(gxtexobjs[currenttexture].data[mipmap] == NULL)
		{
			Sys_Error("GX_LoadAndBind: allocation failed on %i bytes", length);
		};
		gxtexobjs[currenttexture].length[mipmap] = length;
	};
	memcpy(gxtexobjs[currenttexture].data[mipmap], data, length);
	if(mipmap == 0)
	{
		GX_InitTexObj(&gxtexobjs[currenttexture].texobj, gxtexobjs[currenttexture].data[mipmap], width, height, format, GX_REPEAT, GX_REPEAT, GX_FALSE);
		GX_LoadTexObj(&gxtexobjs[currenttexture].texobj, GX_TEXMAP0);
		if(changed)
			GX_InvalidateTexAll();
	};
}

/*
=============================================================================

  scrap allocation

  Allocate all the little status bar obejcts into a single texture
  to crutch up stupid hardware / drivers

=============================================================================
*/

#define	MAX_SCRAPS		2
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
	int		bestx;
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
}

int	scrap_uploads;

void Scrap_Upload (void)
{
	int		texnum;

	scrap_uploads++;

	for (texnum=0 ; texnum<MAX_SCRAPS ; texnum++) {
		GX_Bind(scrap_texnum + texnum);
		GX_Upload8 (scrap_texels[texnum], BLOCK_WIDTH, BLOCK_HEIGHT, false, true);
	}
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
	int	minimize, maximize;
} gxmode_t;

gxmode_t modes[] = {
	{"GL_NEAREST", GL_NEAREST, GL_NEAREST},
	{"GL_LINEAR", GL_LINEAR, GL_LINEAR},
	{"GL_NEAREST_MIPMAP_NEAREST", GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST},
	{"GL_LINEAR_MIPMAP_NEAREST", GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR},
	{"GL_NEAREST_MIPMAP_LINEAR", GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST},
	{"GL_LINEAR_MIPMAP_LINEAR", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR}
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
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gx_filter_min);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gx_filter_max);
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
	byte	*dest, *src;
	int		x, y;
	char	ver[40];
	gxpic_t	*gx;
	int		start;
	byte	*ncdata;
	int		f, fstep;

	memset(gxtexobjs, 0, sizeof(gxtexobjs));

	Cvar_RegisterVariable (&gx_nobind);
	Cvar_RegisterVariable (&gx_max_size);
	Cvar_RegisterVariable (&gx_picmip);

	Cmd_AddCommand ("gx_texturemode", &Draw_TextureMode_f);

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

	start = Hunk_LowMark();

	cb = (qpic_t *)COM_LoadTempFile ("gfx/conback.lmp");	
	if (!cb)
		Sys_Error ("Couldn't load gfx/conback.lmp");
	SwapPic (cb);

	// hack the version number directly into the pic
	sprintf (ver, "(gx %4.2f) %4.2f", (float)GXQUAKE_VERSION, (float)VERSION);
	dest = cb->data + 320*186 + 320 - 11 - 8*strlen(ver);
	y = strlen(ver);
	for (x=0 ; x<y ; x++)
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

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	gx = (gxpic_t *)conback->data;
	gx->texnum = GX_LoadTexture ("conback", conback->width, conback->height, ncdata, false, false);
	gx->sl = 0;
	gx->sh = 1;
	gx->tl = 0;
	gx->th = 1;
	conback->width = vid.width;
	conback->height = vid.height;

	// free loaded console
	Hunk_FreeToLowMark(start);

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
	byte			*dest;
	byte			*source;
	unsigned short	*pusdest;
	int				drawline;	
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

	glBegin (GL_QUADS);
	glTexCoord2f (fcol, frow);
	glVertex2f (x, y);
	glTexCoord2f (fcol + size, frow);
	glVertex2f (x+8, y);
	glTexCoord2f (fcol + size, frow + size);
	glVertex2f (x+8, y+8);
	glTexCoord2f (fcol, frow + size);
	glVertex2f (x, y+8);
	glEnd ();
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
	GX_SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 1);
	gx_blend_enabled = true;
	GX_SetBlendMode(GX_BM_BLEND, gx_blend_src_value, gx_blend_dst_value, GX_LO_NOOP); 
	glColor4f (1,1,1,alpha);
	GX_Bind (gx->texnum);
	glBegin (GL_QUADS);
	glTexCoord2f (gx->sl, gx->tl);
	glVertex2f (x, y);
	glTexCoord2f (gx->sh, gx->tl);
	glVertex2f (x+pic->width, y);
	glTexCoord2f (gx->sh, gx->th);
	glVertex2f (x+pic->width, y+pic->height);
	glTexCoord2f (gx->sl, gx->th);
	glVertex2f (x, y+pic->height);
	glEnd ();
	glColor4f (1,1,1,1);
	GX_SetAlphaCompare(GX_GREATER, 0.666, GX_AOP_AND, GX_ALWAYS, 1);
	gx_blend_enabled = false;
	GX_SetBlendMode(GX_BM_NONE, gx_blend_src_value, gx_blend_dst_value, GX_LO_NOOP); 
}


/*
=============
Draw_Pic
=============
*/
void Draw_Pic (int x, int y, qpic_t *pic)
{
	byte			*dest, *source;
	unsigned short	*pusdest;
	int				v, u;
	gxpic_t			*gx;

	if (scrap_dirty)
		Scrap_Upload ();
	gx = (gxpic_t *)pic->data;
	glColor4f (1,1,1,1);
	GX_Bind (gx->texnum);
	glBegin (GL_QUADS);
	glTexCoord2f (gx->sl, gx->tl);
	glVertex2f (x, y);
	glTexCoord2f (gx->sh, gx->tl);
	glVertex2f (x+pic->width, y);
	glTexCoord2f (gx->sh, gx->th);
	glVertex2f (x+pic->width, y+pic->height);
	glTexCoord2f (gx->sl, gx->th);
	glVertex2f (x, y+pic->height);
	glEnd ();
}


/*
=============
Draw_TransPic
=============
*/
void Draw_TransPic (int x, int y, qpic_t *pic)
{
	byte	*dest, *source, tbyte;
	unsigned short	*pusdest;
	int				v, u;

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

	GX_LoadAndBind (trans, 64*64 * sizeof(unsigned), 64, 64, GX_TF_RGBA8, 0);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glColor3f (1,1,1);
	glBegin (GL_QUADS);
	glTexCoord2f (0, 0);
	glVertex2f (x, y);
	glTexCoord2f (1, 0);
	glVertex2f (x+pic->width, y);
	glTexCoord2f (1, 1);
	glVertex2f (x+pic->width, y+pic->height);
	glTexCoord2f (0, 1);
	glVertex2f (x, y+pic->height);
	glEnd ();
	Sys_BigStackFree(64*64 * sizeof(unsigned), "Draw_TransPicTranslate");
}


/*
================
Draw_ConsoleBackground

================
*/
void Draw_ConsoleBackground (int lines)
{
	int y = (vid.height * 3) >> 2;

	if (lines > y)
		Draw_Pic(0, lines - vid.height, conback);
	else
		Draw_AlphaPic (0, lines - vid.height, conback, (float)(1.2 * lines)/y);
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
	glColor3f (1,1,1);
	GX_Bind (*(int *)draw_backtile->data);
	glBegin (GL_QUADS);
	glTexCoord2f (x/64.0, y/64.0);
	glVertex2f (x, y);
	glTexCoord2f ( (x+w)/64.0, y/64.0);
	glVertex2f (x+w, y);
	glTexCoord2f ( (x+w)/64.0, (y+h)/64.0);
	glVertex2f (x+w, y+h);
	glTexCoord2f ( x/64.0, (y+h)/64.0 );
	glVertex2f (x, y+h);
	glEnd ();
}


/*
=============
Draw_Fill

Fills a box of pixels with a single color
=============
*/
void Draw_Fill (int x, int y, int w, int h, int c)
{
	glDisable (GL_TEXTURE_2D);
	glColor3f (host_basepal[c*3]/255.0,
		host_basepal[c*3+1]/255.0,
		host_basepal[c*3+2]/255.0);

	glBegin (GL_QUADS);

	glVertex2f (x,y);
	glVertex2f (x+w, y);
	glVertex2f (x+w, y+h);
	glVertex2f (x, y+h);

	glEnd ();
	glColor3f (1,1,1);
	glEnable (GL_TEXTURE_2D);
}
//=============================================================================

/*
================
Draw_FadeScreen

================
*/
void Draw_FadeScreen (void)
{
	gx_blend_enabled = true;
	GX_SetBlendMode(GX_BM_BLEND, gx_blend_src_value, gx_blend_dst_value, GX_LO_NOOP); 
	glDisable (GL_TEXTURE_2D);
	glColor4f (0, 0, 0, 0.8);
	glBegin (GL_QUADS);

	glVertex2f (0,0);
	glVertex2f (vid.width, 0);
	glVertex2f (vid.width, vid.height);
	glVertex2f (0, vid.height);

	glEnd ();
	glColor4f (1,1,1,1);
	glEnable (GL_TEXTURE_2D);
	gx_blend_enabled = false;
	GX_SetBlendMode(GX_BM_NONE, gx_blend_src_value, gx_blend_dst_value, GX_LO_NOOP); 

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
	GX_SetViewport(gxx, gxy, gxwidth, gxheight, 0, 1);

	guOrtho(gx_projection_matrix, 0, vid.height, 0, vid.width, -99999, 99999);
	GX_LoadProjectionMtx(gx_projection_matrix, GX_PERSPECTIVE);

	guMtxIdentity(gx_modelview_matrices[gx_cur_modelview_matrix]);
	GX_LoadPosMtxImm(gx_modelview_matrices[gx_cur_modelview_matrix], GX_PNMTX0);

	gx_z_test_enabled = GX_FALSE;
	GX_SetZMode(gx_z_test_enabled, GX_LEQUAL, gx_z_write_enabled);
	gx_cull_enabled = false;
	GX_SetCullMode(GX_CULL_NONE);
	gx_blend_enabled = false;
	GX_SetBlendMode(GX_BM_NONE, gx_blend_src_value, gx_blend_dst_value, GX_LO_NOOP); 
	GX_SetAlphaCompare(GX_GREATER, 0.666, GX_AOP_AND, GX_ALWAYS, 1);
//		GX_SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 1);


	glColor4f (1,1,1,1);
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
================
GX_MipMap8Bit

Mipping for 8 bit textures
================
*/
void GX_MipMap8Bit (byte *in, int width, int height)
{
	int		i, j;
	unsigned short     r,g,b;
	byte	*out, *at1, *at2, *at3, *at4;

//	width <<=2;
	height >>= 1;
	out = in;
	for (i=0 ; i<height ; i++, in+=width)
	{
		for (j=0 ; j<width ; j+=2, out+=1, in+=2)
		{
			at1 = (byte *) (d_8to24table + in[0]);
			at2 = (byte *) (d_8to24table + in[1]);
			at3 = (byte *) (d_8to24table + in[width+0]);
			at4 = (byte *) (d_8to24table + in[width+1]);

 			r = (at1[0]+at2[0]+at3[0]+at4[0]); r>>=5;
 			g = (at1[1]+at2[1]+at3[1]+at4[1]); g>>=5;
 			b = (at1[2]+at2[2]+at3[2]+at4[2]); b>>=5;

			out[0] = d_15to8table[(r<<0) + (g<<5) + (b<<10)];
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

	scaled_width >>= (int)gx_picmip.value;
	scaled_height >>= (int)gx_picmip.value;

	if (scaled_width > gx_max_size.value)
		scaled_width = gx_max_size.value;
	if (scaled_height > gx_max_size.value)
		scaled_height = gx_max_size.value;

	if (scaled_width * scaled_height > sizeof(scaled)/4)
		Sys_Error ("GX_LoadTexture: too big");

#if 0
	if (mipmap)
		gluBuild2DMipmaps (GL_TEXTURE_2D, samples, width, height, GL_RGBA, GL_UNSIGNED_BYTE, trans);
	else if (scaled_width == width && scaled_height == height)
		GX_LoadAndBind (trans, length, width, height, GX_TF_RGBA8, 0);
	else
	{
		gluScaleImage (GL_RGBA, width, height, GL_UNSIGNED_BYTE, trans,
			scaled_width, scaled_height, GL_UNSIGNED_BYTE, scaled);
		GX_LoadAndBind (scaled, length * scaled_width / width * scaled_height / height, scaled_width, scaled_height, GX_TF_RGBA8, 0);
	}
#else
texels += scaled_width * scaled_height;

	if (scaled_width == width && scaled_height == height)
	{
		if (!mipmap)
		{
			GX_LoadAndBind (data, length, scaled_width, scaled_height, GX_TF_RGBA8, 0);
			goto done;
		}
		memcpy (scaled, data, width*height*4);
	}
	else
		GX_ResampleTexture (data, width, height, scaled, scaled_width, scaled_height);

	GX_LoadAndBind (scaled, length * scaled_width / width * scaled_height / height, scaled_width, scaled_height, GX_TF_RGBA8, 0);
	if (mipmap)
	{
		int		miplevel;

		miplevel = 0;
		while (scaled_width > 1 || scaled_height > 1)
		{
			GX_MipMap ((byte *)scaled, scaled_width, scaled_height);
			scaled_width >>= 1;
			scaled_height >>= 1;
			if (scaled_width < 1)
				scaled_width = 1;
			if (scaled_height < 1)
				scaled_height = 1;
			miplevel++;
			GX_LoadAndBind (scaled, length * scaled_width / width * scaled_height / height, scaled_width, scaled_height, GX_TF_RGBA8, miplevel);
		}
	}
done: ;
#endif


	if (mipmap)
	{
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gx_filter_min);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gx_filter_max);
	}
	else
	{
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gx_filter_max);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gx_filter_max);
	}
}

void GX_Upload8_EXT (byte *data, int width, int height,  qboolean mipmap, qboolean alpha) 
{
	int			i, s;
	qboolean	noalpha;
	int			p;
	static unsigned j;
	int			samples;
    static	unsigned char scaled[1024*512];	// [512*256];
	int			scaled_width, scaled_height;

	s = width*height;
	// if there are no transparent pixels, make it a 3 component
	// texture even if it was specified as otherwise
	if (alpha)
	{
		noalpha = true;
		for (i=0 ; i<s ; i++)
		{
			if (data[i] == 255)
				noalpha = false;
		}

		if (alpha && noalpha)
			alpha = false;
	}
	for (scaled_width = 1 ; scaled_width < width ; scaled_width<<=1)
		;
	for (scaled_height = 1 ; scaled_height < height ; scaled_height<<=1)
		;

	scaled_width >>= (int)gx_picmip.value;
	scaled_height >>= (int)gx_picmip.value;

	if (scaled_width > gx_max_size.value)
		scaled_width = gx_max_size.value;
	if (scaled_height > gx_max_size.value)
		scaled_height = gx_max_size.value;

	if (scaled_width * scaled_height > sizeof(scaled))
		Sys_Error ("GX_LoadTexture: too big");

	samples = 1; // alpha ? gx_alpha_format : gx_solid_format;

	texels += scaled_width * scaled_height;

	if (scaled_width == width && scaled_height == height)
	{
		if (!mipmap)
		{
			glTexImage2D (GL_TEXTURE_2D, 0, GL_COLOR_INDEX8_EXT, scaled_width, scaled_height, 0, GL_COLOR_INDEX , GL_UNSIGNED_BYTE, data);
			goto done;
		}
		memcpy (scaled, data, width*height);
	}
	else
		GX_Resample8BitTexture (data, width, height, scaled, scaled_width, scaled_height);

	glTexImage2D (GL_TEXTURE_2D, 0, GL_COLOR_INDEX8_EXT, scaled_width, scaled_height, 0, GL_COLOR_INDEX, GL_UNSIGNED_BYTE, scaled);
	if (mipmap)
	{
		int		miplevel;

		miplevel = 0;
		while (scaled_width > 1 || scaled_height > 1)
		{
			GX_MipMap8Bit ((byte *)scaled, scaled_width, scaled_height);
			scaled_width >>= 1;
			scaled_height >>= 1;
			if (scaled_width < 1)
				scaled_width = 1;
			if (scaled_height < 1)
				scaled_height = 1;
			miplevel++;
			glTexImage2D (GL_TEXTURE_2D, miplevel, GL_COLOR_INDEX8_EXT, scaled_width, scaled_height, 0, GL_COLOR_INDEX, GL_UNSIGNED_BYTE, scaled);
		}
	}
done: ;


	if (mipmap)
	{
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gx_filter_min);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gx_filter_max);
	}
	else
	{
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gx_filter_max);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gx_filter_max);
	}
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

 	if (VID_Is8bit() && !alpha && (data!=scrap_texels[0])) {
 		GX_Upload8_EXT (data, width, height, mipmap, alpha);
 		return;
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
	qboolean	noalpha;
	int			i, p, s;
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
	else {
		gxt = &gxtextures[numgxtextures];
		numgxtextures++;
	}

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

static GLenum oldtarget = TEXTURE0_SGIS;

void GX_SelectTexture (GLenum target) 
{
	if (!gx_mtexable)
		return;
	qgxSelectTextureSGIS(target);
	if (target == oldtarget) 
		return;
	cnttextures[oldtarget-TEXTURE0_SGIS] = currenttexture;
	currenttexture = cnttextures[target-TEXTURE0_SGIS];
	oldtarget = target;
}

#endif

