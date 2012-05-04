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

// draw.c

#include "gx_local.h"

#include "gxutils.h"

image_t		*draw_chars;

extern	qboolean	scrap_dirty;
void Scrap_Upload (void);


/*
===============
Draw_InitLocal
===============
*/
void Draw_InitLocal (void)
{
	// load console characters (don't bilerp characters)
	draw_chars = GL_FindImage ("pics/conchars.pcx", it_pic);
	GX_Bind( draw_chars->texnum );
	GX_SetMinMag(GX_NEAR, GX_NEAR);
}



/*
================
Draw_Char

Draws one 8*8 graphics character with 0 being transparent.
It can be clipped to the top of the screen to allow the console to be
smoothly scrolled off.
================
*/
void Draw_Char (int x, int y, int num)
{
	int				row, col;
	float			frow, fcol, size;

	num &= 255;
	
	if ( (num&127) == 32 )
		return;		// space

	if (y <= -8)
		return;			// totally off screen

	row = num>>4;
	col = num&15;

	frow = row*0.0625;
	fcol = col*0.0625;
	size = 0.0625;

	GX_Bind (draw_chars->texnum);

	qgxBegin (GX_QUADS, gxu_cur_vertex_format, 4);
	qgxPosition3f32 (x, y, 0);
	qgxColor4u8 (gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
	qgxTexCoord2f32 (fcol, frow);
	qgxPosition3f32 (x+8, y, 0);
	qgxColor4u8 (gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
	qgxTexCoord2f32 (fcol + size, frow);
	qgxPosition3f32 (x+8, y+8, 0);
	qgxColor4u8 (gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
	qgxTexCoord2f32 (fcol + size, frow + size);
	qgxPosition3f32 (x, y+8, 0);
	qgxColor4u8 (gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
	qgxTexCoord2f32 (fcol, frow + size);
	qgxEnd ();
}

/*
=============
Draw_FindPic
=============
*/
image_t	*Draw_FindPic (char *name)
{
	image_t *gl;
	char	fullname[MAX_QPATH];

	if (name[0] != '/' && name[0] != '\\')
	{
		Com_sprintf (fullname, sizeof(fullname), "pics/%s.pcx", name);
		gl = GL_FindImage (fullname, it_pic);
	}
	else
		gl = GL_FindImage (name+1, it_pic);

	return gl;
}

/*
=============
Draw_GetPicSize
=============
*/
void Draw_GetPicSize (int *w, int *h, char *pic)
{
	image_t *gl;

	gl = Draw_FindPic (pic);
	if (!gl)
	{
		*w = *h = -1;
		return;
	}
	*w = gl->width;
	*h = gl->height;
}

/*
=============
Draw_StretchPic
=============
*/
void Draw_StretchPic (int x, int y, int w, int h, char *pic)
{
	image_t *gl;

	gl = Draw_FindPic (pic);
	if (!gl)
	{
		ri.Con_Printf (PRINT_ALL, "Can't find pic: %s\n", pic);
		return;
	}

	if (scrap_dirty)
		Scrap_Upload ();

	GX_Bind (gl->texnum);
	qgxBegin (GX_QUADS, gxu_cur_vertex_format, 4);
	qgxPosition3f32 (x, y, 0);
	qgxColor4u8 (gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
	qgxTexCoord2f32 (gl->sl, gl->tl);
	qgxPosition3f32 (x+w, y, 0);
	qgxColor4u8 (gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
	qgxTexCoord2f32 (gl->sh, gl->tl);
	qgxPosition3f32 (x+w, y+h, 0);
	qgxColor4u8 (gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
	qgxTexCoord2f32 (gl->sh, gl->th);
	qgxPosition3f32 (x, y+h, 0);
	qgxColor4u8 (gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
	qgxTexCoord2f32 (gl->sl, gl->th);
	qgxEnd ();
}


/*
=============
Draw_Pic
=============
*/
void Draw_Pic (int x, int y, char *pic)
{
	image_t *gl;

	gl = Draw_FindPic (pic);
	if (!gl)
	{
		ri.Con_Printf (PRINT_ALL, "Can't find pic: %s\n", pic);
		return;
	}
	if (scrap_dirty)
		Scrap_Upload ();

	GX_Bind (gl->texnum);
	qgxBegin (GX_QUADS, gxu_cur_vertex_format, 4);
	qgxPosition3f32 (x, y, 0);
	qgxColor4u8 (gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
	qgxTexCoord2f32 (gl->sl, gl->tl);
	qgxPosition3f32 (x+gl->width, y, 0);
	qgxColor4u8 (gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
	qgxTexCoord2f32 (gl->sh, gl->tl);
	qgxPosition3f32 (x+gl->width, y+gl->height, 0);
	qgxColor4u8 (gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
	qgxTexCoord2f32 (gl->sh, gl->th);
	qgxPosition3f32 (x, y+gl->height, 0);
	qgxColor4u8 (gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
	qgxTexCoord2f32 (gl->sl, gl->th);
	qgxEnd ();
}

/*
=============
Draw_TileClear

This repeats a 64*64 tile graphic to fill the screen around a sized down
refresh window.
=============
*/
void Draw_TileClear (int x, int y, int w, int h, char *pic)
{
	image_t	*image;

	image = Draw_FindPic (pic);
	if (!image)
	{
		ri.Con_Printf (PRINT_ALL, "Can't find pic: %s\n", pic);
		return;
	}

	GX_Bind (image->texnum);
	qgxBegin (GX_QUADS, gxu_cur_vertex_format, 4);
	qgxPosition3f32 (x, y, 0);
	qgxColor4u8 (gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
	qgxTexCoord2f32 (x/64.0, y/64.0);
	qgxPosition3f32 (x+w, y, 0);
	qgxColor4u8 (gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
	qgxTexCoord2f32 ( (x+w)/64.0, y/64.0);
	qgxPosition3f32 (x+w, y+h, 0);
	qgxColor4u8 (gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
	qgxTexCoord2f32 ( (x+w)/64.0, (y+h)/64.0);
	qgxPosition3f32 (x, y+h, 0);
	qgxColor4u8 (gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
	qgxTexCoord2f32 ( x/64.0, (y+h)/64.0 );
	qgxEnd ();
}


/*
=============
Draw_Fill

Fills a box of pixels with a single color
=============
*/
void Draw_Fill (int x, int y, int w, int h, int c)
{
	union
	{
		unsigned	c;
		byte		v[4];
	} color;

	if ( (unsigned)c > 255)
		ri.Sys_Error (ERR_FATAL, "Draw_Fill: bad color");

	qgxDisableTexture();

	color.c = d_8to24table[c];

	qgxBegin (GX_QUADS, GX_VTXFMT0, 4);

	qgxPosition3f32 (x, y, 0);
	qgxColor4u8 (color.v[0], color.v[1], color.v[2], 255);
	qgxPosition3f32 (x+w, y, 0);
	qgxColor4u8 (color.v[0], color.v[1], color.v[2], 255);
	qgxPosition3f32 (x+w, y+h, 0);
	qgxColor4u8 (color.v[0], color.v[1], color.v[2], 255);
	qgxPosition3f32 (x, y+h, 0);
	qgxColor4u8 (color.v[0], color.v[1], color.v[2], 255);

	qgxEnd ();
	qgxEnableTexture();
}

//=============================================================================

/*
================
Draw_FadeScreen

================
*/
void Draw_FadeScreen (void)
{
	qgxSetBlendMode(GX_BM_BLEND, gxu_blend_src_value, gxu_blend_dst_value, GX_LO_NOOP);
	qgxDisableTexture();
	qgxBegin (GX_QUADS, GX_VTXFMT0, 4);

	qgxPosition3f32 (0, 0, 0);
	qgxColor4u8 (0, 0, 0, 205);
	qgxPosition3f32 (vid.width, 0, 0);
	qgxColor4u8 (0, 0, 0, 205);
	qgxPosition3f32 (vid.width, vid.height, 0);
	qgxColor4u8 (0, 0, 0, 205);
	qgxPosition3f32 (0, vid.height, 0);
	qgxColor4u8 (0, 0, 0, 205);

	qgxEnd ();
	qgxEnableTexture();
	qgxSetBlendMode(GX_BM_NONE, gxu_blend_src_value, gxu_blend_dst_value, GX_LO_NOOP);
}


//====================================================================


/*
=============
Draw_StretchRaw
=============
*/
extern unsigned	r_rawpalette[256];

void Draw_StretchRaw (int x, int y, int w, int h, int cols, int rows, byte *data)
{
	unsigned*	image32;
	unsigned char* image8;
	int			i, j, trows;
	byte		*source;
	int			frac, fracstep;
	float		hscale;
	int			row;
	float		t;

	GX_Bind (0);

	if (rows<=256)
	{
		hscale = 1;
		trows = rows;
	}
	else
	{
		hscale = rows/256.0;
		trows = 256;
	}
	t = rows*hscale / 256;

	if ( !qglColorTableEXT )
	{
		image32 = Sys_BigStackAlloc(256*256 * sizeof(unsigned), "Draw_StretchRaw");

		unsigned *dest;

		for (i=0 ; i<trows ; i++)
		{
			row = (int)(i*hscale);
			if (row > rows)
				break;
			source = data + cols*row;
			dest = &image32[i*256];
			fracstep = cols*0x10000/256;
			frac = fracstep >> 1;
			for (j=0 ; j<256 ; j++)
			{
				dest[j] = r_rawpalette[source[frac>>16]];
				frac += fracstep;
			}
		}

		GX_LoadAndBind(image32, 256 * 256 * sizeof(unsigned), 256, 256, GX_TF_RGBA8);

		Sys_BigStackFree(256*256 * sizeof(unsigned), "Draw_StretchRaw");
	}
	else
	{
		image8 = Sys_BigStackAlloc(256*256, "Draw_StretchRaw");

		unsigned char *dest;

		for (i=0 ; i<trows ; i++)
		{
			row = (int)(i*hscale);
			if (row > rows)
				break;
			source = data + cols*row;
			dest = &image8[i*256];
			fracstep = cols*0x10000/256;
			frac = fracstep >> 1;
			for (j=0 ; j<256 ; j++)
			{
				dest[j] = source[frac>>16];
				frac += fracstep;
			}
		}

		GX_LoadAndBind(image8, 256 * 256, 256, 256, GX_TF_CI8);

		Sys_BigStackFree(256*256, "Draw_StretchRaw");
	}
	GX_SetMinMag(GX_LINEAR, GX_LINEAR);

	qgxBegin (GX_QUADS, gxu_cur_vertex_format, 4);
	qgxPosition3f32 (x, y, 0);
	qgxColor4u8 (gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
	qgxTexCoord2f32 (0, 0);
	qgxPosition3f32 (x+w, y, 0);
	qgxColor4u8 (gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
	qgxTexCoord2f32 (1, 0);
	qgxPosition3f32 (x+w, y+h, 0);
	qgxColor4u8 (gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
	qgxTexCoord2f32 (1, t);
	qgxPosition3f32 (x, y+h, 0);
	qgxColor4u8 (gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
	qgxTexCoord2f32 (0, t);
	qgxEnd ();
}

