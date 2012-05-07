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

#include "gx_local.h"
#include <malloc.h>

image_t		gxtextures[MAX_GXTEXTURES];
gxtexobj_t	gxtexobjs[TEXNUM_IMAGES + MAX_GXTEXTURES];
int			numgxtextures;
int			base_textureid;		// gxtextures[i] = base_textureid+i

static byte			 intensitytable[256];
static unsigned char gammatable[256];

cvar_t		*intensity;

unsigned	d_8to24table[256];

qboolean GX_Upload8 (byte *data, int width, int height,  qboolean mipmap, qboolean is_sky );
qboolean GX_Upload32 (unsigned *data, int width, int height,  qboolean mipmap);


int		gx_solid_format = 3;
int		gx_alpha_format = 4;

int		gx_tex_solid_format = 3;
int		gx_tex_alpha_format = 4;

int		gx_filter_min = GX_LIN_MIP_NEAR;
int		gx_filter_max = GX_LINEAR;

int		gx_tex_allocated; // To track amount of memory used for textures

u16*	gx_paldata = NULL;
GXTlutObj gx_palobj;

void GX_SetTexturePalette( unsigned palette[256] )
{
	int i;

	if ( qgxLoadTlut && gl_ext_palettedtexture->value )
	{
		if(gx_paldata == NULL)
			gx_paldata = memalign(32, 256 * sizeof(u16));

		for ( i = 0; i < 256; i++ )
		{
			gx_paldata[i] = 0x8000 
				          | ((( palette[i] >> (3 + 0 )) & 0x1f) << 10) 
			              | ((( palette[i] >> (3 + 8 )) & 0x1f) << 5) 
			              | ((( palette[i] >> (3 + 16)) & 0x1f) << 0);
		}

		DCFlushRange(gx_paldata, 256 * sizeof(u16));
		qgxInitTlutObj(&gx_palobj, gx_paldata, GX_TL_RGB565, 256);
		qgxLoadTlut(&gx_palobj, GX_TLUT0);
		qgxInvalidateTexAll();
	}
}

void GX_EnableMultitexture( qboolean enable )
{
	if ( GX_TEXTURE0 == GX_TEXTURE1 )
		return;

	if ( enable )
	{
		GX_SelectTexture( GX_TEXTURE1 );
		qgxEnableTexStage1();
		GX_TexEnv( GX_REPLACE );
	}
	else
	{
		GX_SelectTexture( GX_TEXTURE1 );
		qgxDisableTexStage1();
		GX_TexEnv( GX_REPLACE );
	}
	GX_SelectTexture( GX_TEXTURE0 );
	GX_TexEnv( GX_REPLACE );
}

void GX_SelectTexture( GLenum texture )
{
	int tmu;

	if ( GX_TEXTURE0 == GX_TEXTURE1 )
		return;

	if ( texture == GX_TEXTURE0 )
	{
		tmu = 0;
	}
	else
	{
		tmu = 1;
	}

	if ( tmu == gx_state.currenttmu )
	{
		return;
	}

	gx_state.currenttmu = tmu;
}

void GX_TexEnv( GLenum mode )
{
	static int lastmodes[2] = { -1, -1 };

	if ( mode != lastmodes[gx_state.currenttmu] )
	{
		qgxSetTevOp( GX_TEVSTAGE0 + gx_state.currenttmu, mode );
		lastmodes[gx_state.currenttmu] = mode;
	}
}

void GX_Bind (int texnum)
{
	extern	image_t	*draw_chars;

	if (gl_nobind->value && draw_chars)		// performance evaluation option
		texnum = draw_chars->texnum;
	if ( gx_state.currenttextures[gx_state.currenttmu] == texnum)
		return;
	gx_state.currenttextures[gx_state.currenttmu] = texnum;
	if(gxtexobjs[texnum].data != NULL)
		qgxLoadTexObj(&gxtexobjs[texnum].texobj, GX_TEXMAP0 + gx_state.currenttmu );
}

void GX_MBind( GLenum target, int texnum )
{
	GX_SelectTexture( target );
	if ( target == GX_TEXTURE0 )
	{
		if ( gx_state.currenttextures[0] == texnum )
			return;
	}
	else
	{
		if ( gx_state.currenttextures[1] == texnum )
			return;
	}
	GX_Bind( texnum );
}

void GX_DeleteTexData(int texnum)
{
	if(gxtexobjs[texnum].data != NULL)
	{
		free(gxtexobjs[texnum].data);
		gxtexobjs[texnum].data = NULL;
		gx_tex_allocated -= gxtexobjs[texnum].length;
		gxtexobjs[texnum].length = 0;
	};
}

qboolean GX_ReallocTex(int length, int width, int height)
{
	qboolean changed = false;
	int texnum = gx_state.currenttextures[gx_state.currenttmu];
	if(gxtexobjs[texnum].length < length)
	{
		GX_DeleteTexData(texnum);
		gxtexobjs[texnum].data = memalign(32, length);
		if(gxtexobjs[texnum].data == NULL)
		{
			Sys_Error("GX_ReallocTex: allocation failed on %i bytes", length);
		};
		gxtexobjs[texnum].length = length;
		gx_tex_allocated += length;
		changed = true;
	};
	gxtexobjs[texnum].width = width;
	gxtexobjs[texnum].height = height;
	return changed;
}

void GX_BindCurrentTex(qboolean changed, int format, int mipmap)
{
	int texnum = gx_state.currenttextures[gx_state.currenttmu];
	DCFlushRange(gxtexobjs[texnum].data, gxtexobjs[texnum].length);

	if(format == GX_TF_CI8)
		qgxInitTexObjCI(&gxtexobjs[texnum].texobj, gxtexobjs[texnum].data, gxtexobjs[texnum].width, gxtexobjs[texnum].height, format, GX_REPEAT, GX_REPEAT, mipmap, GX_TLUT0);
	else
		qgxInitTexObj(&gxtexobjs[texnum].texobj, gxtexobjs[texnum].data, gxtexobjs[texnum].width, gxtexobjs[texnum].height, format, GX_REPEAT, GX_REPEAT, mipmap);

	qgxLoadTexObj(&gxtexobjs[texnum].texobj, GX_TEXMAP0 + gx_state.currenttmu);
	if(changed)
		qgxInvalidateTexAll();
}

void GX_LoadAndBind (void* data, int length, int width, int height, int format)
{
	qboolean changed = GX_ReallocTex(length, width, height);
	int texnum = gx_state.currenttextures[gx_state.currenttmu];
	switch(format)
	{
	case GX_TF_RGBA8:
		GX_CopyTexRGBA8((byte*)data, width, height, (byte*)(gxtexobjs[texnum].data));
		break;
	case GX_TF_RGB5A3:
		GX_CopyTexRGB5A3((byte*)data, width, height, (byte*)(gxtexobjs[texnum].data));
		break;
	case GX_TF_I8:
	case GX_TF_A8:
		GX_CopyTexV8((byte*)data, width, height, (byte*)(gxtexobjs[texnum].data));
		break;
	case GX_TF_IA4:
		GX_CopyTexIA4((byte*)data, width, height, (byte*)(gxtexobjs[texnum].data));
		break;
	};
	GX_BindCurrentTex(changed, format, GX_FALSE);
}

void GX_LoadSubAndBind (void* data, int xoffset, int yoffset, int width, int height, int format)
{
	byte* dst;
	int tex_width;
	int tex_height;
	int ybegin;
	int yend;
	int x;
	int y;
	int xi;
	int yi;
	int xs;
	int ys;
	int k;
	qboolean in;
	byte s1;
	byte s2;

	if(format == GX_TF_RGBA8)
	{
		dst = (byte*)(gxtexobjs[gx_state.currenttextures[gx_state.currenttmu]].data);
		if(dst != NULL)
		{
			tex_width = gxtexobjs[gx_state.currenttextures[gx_state.currenttmu]].width;
			tex_height = gxtexobjs[gx_state.currenttextures[gx_state.currenttmu]].height;
			ybegin = (yoffset >> 2) << 2;
			if(ybegin < 0) 
				ybegin = 0;
			if(ybegin > tex_height) 
				ybegin = tex_height;
			yend = (((yoffset + height) >> 2) << 2) + 4;
			if(yend < 0) 
				yend = 0;
			if(yend > tex_height) 
				yend = tex_height;
			if(ybegin > 0) 
				dst += (4 * ybegin * tex_height);
			for(y = ybegin; y < yend; y += 4)
			{
				for(x = 0; x < tex_width; x += 4)
				{
					for(yi = 0; yi < 4; yi++)
					{
						for(xi = 0; xi < 4; xi++)
						{
							in = false;
							xs = x + xi - xoffset;
							if((xs >= 0)&&(xs < width))
							{
								ys = y + yi - yoffset;
								if((ys >= 0)&&(ys < height))
								{
									k = 4 * (ys * width + xs);
									in = true;
									*(dst++) = ((byte*)data)[k + 3];
									*(dst++) = ((byte*)data)[k];
								};
							};
							if(!in)
								dst += 2;
						};
					};
					for(yi = 0; yi < 4; yi++)
					{
						for(xi = 0; xi < 4; xi++)
						{
							in = false;
							xs = x + xi - xoffset;
							if((xs >= 0)&&(xs < width))
							{
								ys = y + yi - yoffset;
								if((ys >= 0)&&(ys < height))
								{
									k = 4 * (ys * width + xs);
									in = true;
									*(dst++) = ((byte*)data)[k + 1];
									*(dst++) = ((byte*)data)[k + 2];
								};
							};
							if(!in)
								dst += 2;
						};
					};
				};
			};
		};
		GX_BindCurrentTex(true, format, GX_FALSE);
	} else if(format == GX_TF_RGB5A3)
	{
		dst = (byte*)(gxtexobjs[gx_state.currenttextures[gx_state.currenttmu]].data);
		if(dst != NULL)
		{
			tex_width = gxtexobjs[gx_state.currenttextures[gx_state.currenttmu]].width;
			tex_height = gxtexobjs[gx_state.currenttextures[gx_state.currenttmu]].height;
			ybegin = (yoffset >> 2) << 2;
			if(ybegin < 0) 
				ybegin = 0;
			if(ybegin > tex_height) 
				ybegin = tex_height;
			yend = (((yoffset + height) >> 2) << 2) + 4;
			if(yend < 0) 
				yend = 0;
			if(yend > tex_height) 
				yend = tex_height;
			if(ybegin > 0) 
				dst += (2 * ybegin * tex_height);
			for(y = ybegin; y < yend; y += 4)
			{
				for(x = 0; x < tex_width; x += 4)
				{
					for(yi = 0; yi < 4; yi++)
					{
						for(xi = 0; xi < 4; xi++)
						{
							in = false;
							xs = x + xi - xoffset;
							if((xs >= 0)&&(xs < width))
							{
								ys = y + yi - yoffset;
								if((ys >= 0)&&(ys < height))
								{
									k = 2 * (ys * width + xs);
									in = true;
									s1 = ((byte*)data)[k];
									s2 = ((byte*)data)[k + 1];
									*(dst++) = (((s2 & 15) >> 1) << 4) | (s1 >> 4);
									*(dst++) = ((s1 & 15) << 4) | (s2 >> 4);
								};
							};
							if(!in)
								dst += 2;
						};
					};
				};
			};
		};
		GX_BindCurrentTex(true, format, GX_FALSE);
	} else if((format == GX_TF_A8)||(format == GX_TF_I8))
	{
		dst = (byte*)(gxtexobjs[gx_state.currenttextures[gx_state.currenttmu]].data);
		if(dst != NULL)
		{
			tex_width = gxtexobjs[gx_state.currenttextures[gx_state.currenttmu]].width;
			tex_height = gxtexobjs[gx_state.currenttextures[gx_state.currenttmu]].height;
			ybegin = (yoffset >> 2) << 2;
			if(ybegin < 0) 
				ybegin = 0;
			if(ybegin > tex_height) 
				ybegin = tex_height;
			yend = (((yoffset + height) >> 2) << 2) + 4;
			if(yend < 0) 
				yend = 0;
			if(yend > tex_height) 
				yend = tex_height;
			if(ybegin > 0) 
				dst += (ybegin * tex_height);
			for(y = ybegin; y < yend; y += 4)
			{
				for(x = 0; x < tex_width; x += 8)
				{
					for(yi = 0; yi < 4; yi++)
					{
						for(xi = 0; xi < 8; xi++)
						{
							in = false;
							xs = x + xi - xoffset;
							if((xs >= 0)&&(xs < width))
							{
								ys = y + yi - yoffset;
								if((ys >= 0)&&(ys < height))
								{
									k = ys * width + xs;
									in = true;
									*(dst++) = ((byte*)data)[k];
								};
							};
							if(!in)
								dst++;
						};
					};
				};
			};
		};
		GX_BindCurrentTex(true, format, GX_FALSE);
	} else if(format == GX_TF_IA4)
	{
		dst = (byte*)(gxtexobjs[gx_state.currenttextures[gx_state.currenttmu]].data);
		if(dst != NULL)
		{
			tex_width = gxtexobjs[gx_state.currenttextures[gx_state.currenttmu]].width;
			tex_height = gxtexobjs[gx_state.currenttextures[gx_state.currenttmu]].height;
			ybegin = (yoffset >> 2) << 2;
			if(ybegin < 0) 
				ybegin = 0;
			if(ybegin > tex_height) 
				ybegin = tex_height;
			yend = (((yoffset + height) >> 2) << 2) + 4;
			if(yend < 0) 
				yend = 0;
			if(yend > tex_height) 
				yend = tex_height;
			if(ybegin > 0) 
				dst += (ybegin * tex_height);
			for(y = ybegin; y < yend; y += 4)
			{
				for(x = 0; x < tex_width; x += 8)
				{
					for(yi = 0; yi < 4; yi++)
					{
						for(xi = 0; xi < 8; xi++)
						{
							in = false;
							xs = x + xi - xoffset;
							if((xs >= 0)&&(xs < width))
							{
								ys = y + yi - yoffset;
								if((ys >= 0)&&(ys < height))
								{
									k = ys * width + xs;
									in = true;
									s1 = ((byte*)data)[k];
									*(dst++) = ((s1 & 15) << 4) | (s1 >> 4);
								};
							};
							if(!in)
								dst++;
						};
					};
				};
			};
		};
		GX_BindCurrentTex(true, format, GX_FALSE);
	};
}

void GX_SetMinMag (int minfilt, int magfilt)
{
	int texnum = gx_state.currenttextures[gx_state.currenttmu];
	if(gxtexobjs[texnum].data != NULL)
	{
		qgxInitTexObjFilterMode(&gxtexobjs[texnum].texobj, minfilt, magfilt);
	};
}

typedef struct
{
	char *name;
	int	minimize, maximize;
} gxmode_t;

gxmode_t modes[] = {
	{"GX_NEAR", GX_NEAR, GX_NEAR},
	{"GX_LINEAR", GX_LINEAR, GX_LINEAR},
	{"GX_NEAR_MIP_NEAR", GX_NEAR_MIP_NEAR, GX_NEAR},
	{"GX_LIN_MIP_NEAR", GX_LIN_MIP_NEAR, GX_LINEAR},
	{"GX_NEAR_MIP_LIN", GX_NEAR_MIP_LIN, GX_NEAR},
	{"GX_LIN_MIP_LIN", GX_LIN_MIP_LIN, GX_LINEAR}
};

#define NUM_GX_MODES (sizeof(modes) / sizeof (gxmode_t))

typedef struct
{
	char *name;
	int mode;
} gxtmode_t;

gxtmode_t gx_alpha_modes[] = {
	{"default", 4}
};

#define NUM_GX_ALPHA_MODES (sizeof(gx_alpha_modes) / sizeof (gxtmode_t))

gxtmode_t gx_solid_modes[] = {
	{"default", 3}
};

#define NUM_GX_SOLID_MODES (sizeof(gx_solid_modes) / sizeof (gxtmode_t))

/*
===============
GX_TextureMode
===============
*/
void GX_TextureMode( char *string )
{
	int		i;
	image_t	*glt;

	for (i=0 ; i< NUM_GX_MODES ; i++)
	{
		if ( !Q_stricmp( modes[i].name, string ) )
			break;
	}

	if (i == NUM_GX_MODES)
	{
		ri.Con_Printf (PRINT_ALL, "bad filter name\n");
		return;
	}

	gx_filter_min = modes[i].minimize;
	gx_filter_max = modes[i].maximize;

	// change all the existing mipmap texture objects
	for (i=0, glt=gxtextures ; i<numgxtextures ; i++, glt++)
	{
		if (glt->type != it_pic && glt->type != it_sky )
		{
			GX_Bind (glt->texnum);
			GX_SetMinMag(gx_filter_min, gx_filter_max);
		}
	}
}

/*
===============
GX_TextureAlphaMode
===============
*/
void GX_TextureAlphaMode( char *string )
{
	int		i;

	for (i=0 ; i< NUM_GX_ALPHA_MODES ; i++)
	{
		if ( !Q_stricmp( gx_alpha_modes[i].name, string ) )
			break;
	}

	if (i == NUM_GX_ALPHA_MODES)
	{
		ri.Con_Printf (PRINT_ALL, "bad alpha texture mode name\n");
		return;
	}

	gx_tex_alpha_format = gx_alpha_modes[i].mode;
}

/*
===============
GX_TextureSolidMode
===============
*/
void GX_TextureSolidMode( char *string )
{
	int		i;

	for (i=0 ; i< NUM_GX_SOLID_MODES ; i++)
	{
		if ( !Q_stricmp( gx_solid_modes[i].name, string ) )
			break;
	}

	if (i == NUM_GX_SOLID_MODES)
	{
		ri.Con_Printf (PRINT_ALL, "bad solid texture mode name\n");
		return;
	}

	gx_tex_solid_format = gx_solid_modes[i].mode;
}

/*
===============
GX_ImageList_f
===============
*/
void	GX_ImageList_f (void)
{
	int		i;
	image_t	*image;
	int		texels;
	const char *palstrings[2] =
	{
		"RGB",
		"PAL"
	};

	ri.Con_Printf (PRINT_ALL, "------------------\n");
	texels = 0;

	for (i=0, image=gxtextures ; i<numgxtextures ; i++, image++)
	{
		if (image->texnum <= 0)
			continue;
		texels += image->upload_width*image->upload_height;
		switch (image->type)
		{
		case it_skin:
			ri.Con_Printf (PRINT_ALL, "M");
			break;
		case it_sprite:
			ri.Con_Printf (PRINT_ALL, "S");
			break;
		case it_wall:
			ri.Con_Printf (PRINT_ALL, "W");
			break;
		case it_pic:
			ri.Con_Printf (PRINT_ALL, "P");
			break;
		default:
			ri.Con_Printf (PRINT_ALL, " ");
			break;
		}

		ri.Con_Printf (PRINT_ALL,  " %3i %3i %s: %s\n",
			image->upload_width, image->upload_height, palstrings[image->paletted], image->name);
	}
	ri.Con_Printf (PRINT_ALL, "Total texel count (not counting mipmaps): %i\n", texels);
}


/*
=============================================================================

  scrap allocation

  Allocate all the little status bar obejcts into a single texture
  to crutch up inefficient hardware / drivers

=============================================================================
*/

#define	MAX_SCRAPS		1
#define	BLOCK_WIDTH		256
#define	BLOCK_HEIGHT	256

int			scrap_allocated[MAX_SCRAPS][BLOCK_WIDTH];
byte		scrap_texels[MAX_SCRAPS][BLOCK_WIDTH*BLOCK_HEIGHT];
qboolean	scrap_dirty;

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

	return -1;
//	Sys_Error ("Scrap_AllocBlock: full");
}

int	scrap_uploads;

void Scrap_Upload (void)
{
	scrap_uploads++;
	GX_Bind(TEXNUM_SCRAPS);
	GX_Upload8 (scrap_texels[0], BLOCK_WIDTH, BLOCK_HEIGHT, false, false );
	scrap_dirty = false;
}

/*
=================================================================

PCX LOADING

=================================================================
*/


/*
==============
LoadPCX
==============
*/
void LoadPCX (char *filename, byte **pic, byte **palette, int *width, int *height)
{
	byte	*raw;
	pcx_t	*pcx;
	int		x, y;
	int		len;
	int		dataByte, runLength;
	byte	*out, *pix;

	*pic = NULL;
	*palette = NULL;

	//
	// load the file
	//
	len = ri.FS_LoadFile (filename, (void **)&raw, true);
	if (!raw)
	{
		ri.Con_Printf (PRINT_DEVELOPER, "Bad pcx file %s\n", filename);
		return;
	}

	//
	// parse the PCX file
	//
	pcx = (pcx_t *)raw;

    pcx->xmin = LittleShort(pcx->xmin);
    pcx->ymin = LittleShort(pcx->ymin);
    pcx->xmax = LittleShort(pcx->xmax);
    pcx->ymax = LittleShort(pcx->ymax);
    pcx->hres = LittleShort(pcx->hres);
    pcx->vres = LittleShort(pcx->vres);
    pcx->bytes_per_line = LittleShort(pcx->bytes_per_line);
    pcx->palette_type = LittleShort(pcx->palette_type);

	raw = &pcx->data;

	if (pcx->manufacturer != 0x0a
		|| pcx->version != 5
		|| pcx->encoding != 1
		|| pcx->bits_per_pixel != 8
		|| pcx->xmax >= 640
		|| pcx->ymax >= 480)
	{
		ri.Con_Printf (PRINT_ALL, "Bad pcx file %s\n", filename);
		return;
	}

	out = malloc ( (pcx->ymax+1) * (pcx->xmax+1) );

	*pic = out;

	pix = out;

	if (palette)
	{
		*palette = malloc(768);
		memcpy (*palette, (byte *)pcx + len - 768, 768);
	}

	if (width)
		*width = pcx->xmax+1;
	if (height)
		*height = pcx->ymax+1;

	for (y=0 ; y<=pcx->ymax ; y++, pix += pcx->xmax+1)
	{
		for (x=0 ; x<=pcx->xmax ; )
		{
			dataByte = *raw++;

			if((dataByte & 0xC0) == 0xC0)
			{
				runLength = dataByte & 0x3F;
				dataByte = *raw++;
			}
			else
				runLength = 1;

			while(runLength-- > 0)
				pix[x++] = dataByte;
		}

	}

	if ( raw - (byte *)pcx > len)
	{
		ri.Con_Printf (PRINT_DEVELOPER, "PCX file %s was malformed", filename);
		free (*pic);
		*pic = NULL;
	}

	ri.FS_FreeFile (pcx);
}

/*
=========================================================

TARGA LOADING

=========================================================
*/

typedef struct _TargaHeader {
	unsigned char 	id_length, colormap_type, image_type;
	unsigned short	colormap_index, colormap_length;
	unsigned char	colormap_size;
	unsigned short	x_origin, y_origin, width, height;
	unsigned char	pixel_size, attributes;
} TargaHeader;


/*
=============
LoadTGA
=============
*/
void LoadTGA (char *name, byte **pic, int *width, int *height)
{
	int		columns, rows, numPixels;
	byte	*pixbuf;
	int		row, column;
	byte	*buf_p;
	byte	*buffer;
	int		length;
	TargaHeader		targa_header;
	byte			*targa_rgba;
	byte tmp[2];

	*pic = NULL;

	//
	// load the file
	//
	length = ri.FS_LoadFile (name, (void **)&buffer, true);
	if (!buffer)
	{
		ri.Con_Printf (PRINT_DEVELOPER, "Bad tga file %s\n", name);
		return;
	}

	buf_p = buffer;

	targa_header.id_length = *buf_p++;
	targa_header.colormap_type = *buf_p++;
	targa_header.image_type = *buf_p++;
	
	tmp[0] = buf_p[0];
	tmp[1] = buf_p[1];
	targa_header.colormap_index = LittleShort ( *((short *)tmp) );
	buf_p+=2;
	tmp[0] = buf_p[0];
	tmp[1] = buf_p[1];
	targa_header.colormap_length = LittleShort ( *((short *)tmp) );
	buf_p+=2;
	targa_header.colormap_size = *buf_p++;
	targa_header.x_origin = LittleShort ( *((short *)buf_p) );
	buf_p+=2;
	targa_header.y_origin = LittleShort ( *((short *)buf_p) );
	buf_p+=2;
	targa_header.width = LittleShort ( *((short *)buf_p) );
	buf_p+=2;
	targa_header.height = LittleShort ( *((short *)buf_p) );
	buf_p+=2;
	targa_header.pixel_size = *buf_p++;
	targa_header.attributes = *buf_p++;

	if (targa_header.image_type!=2 
		&& targa_header.image_type!=10) 
		ri.Sys_Error (ERR_DROP, "LoadTGA: Only type 2 and 10 targa RGB images supported\n");

	if (targa_header.colormap_type !=0 
		|| (targa_header.pixel_size!=32 && targa_header.pixel_size!=24))
		ri.Sys_Error (ERR_DROP, "LoadTGA: Only 32 or 24 bit images supported (no colormaps)\n");

	columns = targa_header.width;
	rows = targa_header.height;
	numPixels = columns * rows;

	if (width)
		*width = columns;
	if (height)
		*height = rows;

	targa_rgba = malloc (numPixels*4);
	*pic = targa_rgba;

	if (targa_header.id_length != 0)
		buf_p += targa_header.id_length;  // skip TARGA image comment
	
	if (targa_header.image_type==2) {  // Uncompressed, RGB images
		for(row=rows-1; row>=0; row--) {
			pixbuf = targa_rgba + row*columns*4;
			for(column=0; column<columns; column++) {
				unsigned char red,green,blue,alphabyte;
				switch (targa_header.pixel_size) {
					case 24:
							
							blue = *buf_p++;
							green = *buf_p++;
							red = *buf_p++;
							*pixbuf++ = red;
							*pixbuf++ = green;
							*pixbuf++ = blue;
							*pixbuf++ = 255;
							break;
					case 32:
							blue = *buf_p++;
							green = *buf_p++;
							red = *buf_p++;
							alphabyte = *buf_p++;
							*pixbuf++ = red;
							*pixbuf++ = green;
							*pixbuf++ = blue;
							*pixbuf++ = alphabyte;
							break;
				}
			}
		}
	}
	else if (targa_header.image_type==10) {   // Runlength encoded RGB images
		unsigned char red,green,blue,alphabyte,packetHeader,packetSize,j;
		for(row=rows-1; row>=0; row--) {
			pixbuf = targa_rgba + row*columns*4;
			for(column=0; column<columns; ) {
				packetHeader= *buf_p++;
				packetSize = 1 + (packetHeader & 0x7f);
				if (packetHeader & 0x80) {        // run-length packet
					switch (targa_header.pixel_size) {
						case 24:
								blue = *buf_p++;
								green = *buf_p++;
								red = *buf_p++;
								alphabyte = 255;
								break;
						case 32:
								blue = *buf_p++;
								green = *buf_p++;
								red = *buf_p++;
								alphabyte = *buf_p++;
								break;
					}
	
					for(j=0;j<packetSize;j++) {
						*pixbuf++=red;
						*pixbuf++=green;
						*pixbuf++=blue;
						*pixbuf++=alphabyte;
						column++;
						if (column==columns) { // run spans across rows
							column=0;
							if (row>0)
								row--;
							else
								goto breakOut;
							pixbuf = targa_rgba + row*columns*4;
						}
					}
				}
				else {                            // non run-length packet
					for(j=0;j<packetSize;j++) {
						switch (targa_header.pixel_size) {
							case 24:
									blue = *buf_p++;
									green = *buf_p++;
									red = *buf_p++;
									*pixbuf++ = red;
									*pixbuf++ = green;
									*pixbuf++ = blue;
									*pixbuf++ = 255;
									break;
							case 32:
									blue = *buf_p++;
									green = *buf_p++;
									red = *buf_p++;
									alphabyte = *buf_p++;
									*pixbuf++ = red;
									*pixbuf++ = green;
									*pixbuf++ = blue;
									*pixbuf++ = alphabyte;
									break;
						}
						column++;
						if (column==columns) { // pixel packet run spans across rows
							column=0;
							if (row>0)
								row--;
							else
								goto breakOut;
							pixbuf = targa_rgba + row*columns*4;
						}						
					}
				}
			}
			breakOut:;
		}
	}

	ri.FS_FreeFile (buffer);
}


/*
====================================================================

IMAGE FLOOD FILLING

====================================================================
*/


/*
=================
Mod_FloodFillSkin

Fill background pixels so mipmapping doesn't have haloes
=================
*/

typedef struct
{
	short		x, y;
} floodfill_t;

// must be a power of 2
#define FLOODFILL_FIFO_SIZE 0x1000
#define FLOODFILL_FIFO_MASK (FLOODFILL_FIFO_SIZE - 1)

#define FLOODFILL_STEP( off, dx, dy ) \
{ \
	if (pos[off] == fillcolor) \
	{ \
		pos[off] = 255; \
		fifo[inpt].x = x + (dx), fifo[inpt].y = y + (dy); \
		inpt = (inpt + 1) & FLOODFILL_FIFO_MASK; \
	} \
	else if (pos[off] != 255) fdc = pos[off]; \
}

void R_FloodFillSkin( byte *skin, int skinwidth, int skinheight )
{
	byte				fillcolor = *skin; // assume this is the pixel to fill
	floodfill_t*		fifo;
	int					inpt = 0, outpt = 0;
	int					filledcolor = -1;
	int					i;

	if (filledcolor == -1)
	{
		filledcolor = 0;
		// attempt to find opaque black
		for (i = 0; i < 256; ++i)
			if (d_8to24table[i] == (255 << 0)) // alpha 1.0
			{
				filledcolor = i;
				break;
			}
	}

	// can't fill to filled color or to transparent color (used as visited marker)
	if ((fillcolor == filledcolor) || (fillcolor == 255))
	{
		//printf( "not filling skin from %d to %d\n", fillcolor, filledcolor );
		return;
	}

	fifo = Sys_BigStackAlloc(FLOODFILL_FIFO_SIZE * sizeof(floodfill_t), "R_FloodFillSkin");

	fifo[inpt].x = 0, fifo[inpt].y = 0;
	inpt = (inpt + 1) & FLOODFILL_FIFO_MASK;

	while (outpt != inpt)
	{
		int			x = fifo[outpt].x, y = fifo[outpt].y;
		int			fdc = filledcolor;
		byte		*pos = &skin[x + skinwidth * y];

		outpt = (outpt + 1) & FLOODFILL_FIFO_MASK;

		if (x > 0)				FLOODFILL_STEP( -1, -1, 0 );
		if (x < skinwidth - 1)	FLOODFILL_STEP( 1, 1, 0 );
		if (y > 0)				FLOODFILL_STEP( -skinwidth, 0, -1 );
		if (y < skinheight - 1)	FLOODFILL_STEP( skinwidth, 0, 1 );
		skin[x + skinwidth * y] = fdc;
	};
	Sys_BigStackFree(FLOODFILL_FIFO_SIZE * sizeof(floodfill_t), "R_FloodFillSkin");
}

//=======================================================


/*
================
GX_ResampleTexture
================
*/
void GX_ResampleTexture (unsigned *in, int inwidth, int inheight, unsigned *out,  int outwidth, int outheight)
{
	int		i, j;
	unsigned	*inrow, *inrow2;
	unsigned	frac, fracstep;
	unsigned*	p1 = Sys_BigStackAlloc(1024 * sizeof(unsigned), "GX_ResampleTexture");
	unsigned*	p2 = Sys_BigStackAlloc(1024 * sizeof(unsigned), "GX_ResampleTexture");
	byte		*pix1, *pix2, *pix3, *pix4;

	fracstep = inwidth*0x10000/outwidth;

	frac = fracstep>>2;
	for (i=0 ; i<outwidth ; i++)
	{
		p1[i] = 4*(frac>>16);
		frac += fracstep;
	}
	frac = 3*(fracstep>>2);
	for (i=0 ; i<outwidth ; i++)
	{
		p2[i] = 4*(frac>>16);
		frac += fracstep;
	}

	for (i=0 ; i<outheight ; i++, out += outwidth)
	{
		inrow = in + inwidth*(int)((i+0.25)*inheight/outheight);
		inrow2 = in + inwidth*(int)((i+0.75)*inheight/outheight);
		frac = fracstep >> 1;
		for (j=0 ; j<outwidth ; j++)
		{
			pix1 = (byte *)inrow + p1[j];
			pix2 = (byte *)inrow + p2[j];
			pix3 = (byte *)inrow2 + p1[j];
			pix4 = (byte *)inrow2 + p2[j];
			((byte *)(out+j))[0] = (pix1[0] + pix2[0] + pix3[0] + pix4[0])>>2;
			((byte *)(out+j))[1] = (pix1[1] + pix2[1] + pix3[1] + pix4[1])>>2;
			((byte *)(out+j))[2] = (pix1[2] + pix2[2] + pix3[2] + pix4[2])>>2;
			((byte *)(out+j))[3] = (pix1[3] + pix2[3] + pix3[3] + pix4[3])>>2;
		}
	};

	Sys_BigStackFree(2048 * sizeof(unsigned), "GX_ResampleTexture");
}

/*
================
GX_LightScaleTexture

Scale up the pixel values in a texture to increase the
lighting range
================
*/
void GX_LightScaleTexture (unsigned *in, int inwidth, int inheight, qboolean only_gamma )
{
	if ( only_gamma )
	{
		int		i, c;
		byte	*p;

		p = (byte *)in;

		c = inwidth*inheight;
		for (i=0 ; i<c ; i++, p+=4)
		{
			p[0] = gammatable[p[0]];
			p[1] = gammatable[p[1]];
			p[2] = gammatable[p[2]];
		}
	}
	else
	{
		int		i, c;
		byte	*p;

		p = (byte *)in;

		c = inwidth*inheight;
		for (i=0 ; i<c ; i++, p+=4)
		{
			p[0] = gammatable[intensitytable[p[0]]];
			p[1] = gammatable[intensitytable[p[1]]];
			p[2] = gammatable[intensitytable[p[2]]];
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

Returns has_alpha
===============
*/
void GX_BuildPalettedTexture( unsigned char *paletted_texture, unsigned char *scaled, int scaled_width, int scaled_height )
{
	int i;

	for ( i = 0; i < scaled_width * scaled_height; i++ )
	{
		unsigned int r, g, b, c;

		r = ( scaled[0] >> 3 ) & 31;
		g = ( scaled[1] >> 2 ) & 63;
		b = ( scaled[2] >> 3 ) & 31;

		c = r | ( g << 5 ) | ( b << 11 );

		paletted_texture[i] = gx_state.d_16to8table[c];

		scaled += 4;
	}
}

int		upload_width, upload_height;
qboolean uploaded_paletted;

qboolean GX_Upload32 (unsigned *data, int width, int height,  qboolean mipmap)
{
	int			samples;
	unsigned*	scaled = Sys_BigStackAlloc(256*256 * sizeof(unsigned), "GX_Upload32");
	unsigned char* paletted_texture = Sys_BigStackAlloc(256*256, "GX_Upload32");
	int			scaled_width, scaled_height;
	int			i, c;
	byte		*scan;
	int comp;

	uploaded_paletted = false;

	for (scaled_width = 1 ; scaled_width < width ; scaled_width<<=1)
		;
	if (gl_round_down->value && scaled_width > width && mipmap)
		scaled_width >>= 1;
	for (scaled_height = 1 ; scaled_height < height ; scaled_height<<=1)
		;
	if (gl_round_down->value && scaled_height > height && mipmap)
		scaled_height >>= 1;

	// let people sample down the world textures for speed
	if (mipmap)
	{
		scaled_width >>= (int)gl_picmip->value;
		scaled_height >>= (int)gl_picmip->value;
	}

	// don't ever bother with >256 textures
	if (scaled_width > 256)
		scaled_width = 256;
	if (scaled_height > 256)
		scaled_height = 256;

	// This is required in order to make small textures compatible with GX hardware:
	if (scaled_width < 4)
		scaled_width = 4;
	if (scaled_height < 4)
		scaled_height = 4;

	upload_width = scaled_width;
	upload_height = scaled_height;

	if (scaled_width * scaled_height > (256*256 * sizeof(unsigned))/4)
		ri.Sys_Error (ERR_DROP, "GX_Upload32: too big");

	// scan the texture for any non-255 alpha
	c = width*height;
	scan = ((byte *)data) + 3;
	samples = gx_solid_format;
	for (i=0 ; i<c ; i++, scan += 4)
	{
		if ( *scan != 255 )
		{
			samples = gx_alpha_format;
			break;
		}
	}

	if (samples == gx_solid_format)
	    comp = gx_tex_solid_format;
	else if (samples == gx_alpha_format)
	    comp = gx_tex_alpha_format;
	else {
	    ri.Con_Printf (PRINT_ALL,
			   "Unknown number of texture components %i\n",
			   samples);
	    comp = samples;
	}

#if 0
	if (mipmap)
		gluBuild2DMipmaps (GL_TEXTURE_2D, samples, width, height, GL_RGBA, GL_UNSIGNED_BYTE, trans);
	else if (scaled_width == width && scaled_height == height)
		qglTexImage2D (GL_TEXTURE_2D, 0, comp, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, trans);
	else
	{
		gluScaleImage (GL_RGBA, width, height, GL_UNSIGNED_BYTE, trans,
			scaled_width, scaled_height, GL_UNSIGNED_BYTE, scaled);
		qglTexImage2D (GL_TEXTURE_2D, 0, comp, scaled_width, scaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, scaled);
	}
#else

	if (scaled_width == width && scaled_height == height)
	{
		if (!mipmap)
		{
			if ( qgxLoadTlut && gl_ext_palettedtexture->value && samples == gx_solid_format )
			{
				uploaded_paletted = true;
				GX_BuildPalettedTexture( paletted_texture, ( unsigned char * ) data, scaled_width, scaled_height );
				GX_LoadAndBind(paletted_texture, scaled_width * scaled_height, scaled_width, scaled_height, GX_TF_CI8);
			}
			else
			{
				GX_LoadAndBind(data, scaled_width * scaled_height * 4, scaled_width, scaled_height, GX_TF_RGBA8);
			}
			goto done;
		}
		memcpy (scaled, data, width*height*4);
	}
	else
		GX_ResampleTexture (data, width, height, scaled, scaled_width, scaled_height);

	GX_LightScaleTexture (scaled, scaled_width, scaled_height, !mipmap );

	if ( qgxLoadTlut && gl_ext_palettedtexture->value && ( samples == gx_solid_format ) )
	{
		uploaded_paletted = true;
		GX_BuildPalettedTexture( paletted_texture, ( unsigned char * ) scaled, scaled_width, scaled_height );
		GX_LoadAndBind(paletted_texture, scaled_width * scaled_height, scaled_width, scaled_height, GX_TF_CI8);
	}
	else
	{
		GX_LoadAndBind(scaled, scaled_width * scaled_height * sizeof(unsigned), scaled_width, scaled_height, GX_TF_RGBA8);
	}

	if (mipmap)
	{
		int texnum;
		int	miplevel;
		int sw;
		int sh;
		u32 scaledlen;
		qboolean changed;
		byte* dst;

		texnum = gx_state.currenttextures[gx_state.currenttmu];
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
		if ( qgxLoadTlut && gl_ext_palettedtexture->value && samples == gx_solid_format )
		{
			uploaded_paletted = true;
			GX_BuildPalettedTexture( paletted_texture, ( unsigned char * ) scaled, scaled_width, scaled_height );
			dst = GX_CopyTexV8(paletted_texture, scaled_width, scaled_height, (byte*)(gxtexobjs[texnum].data));
		}
		else
		{
			dst = GX_CopyTexRGBA8((byte*)scaled, scaled_width, scaled_height, (byte*)(gxtexobjs[texnum].data));
		};
		while (scaled_width > 4 && scaled_height > 4)
		{
			GX_MipMap ((byte *)scaled, scaled_width, scaled_height);
			scaled_width >>= 1;
			scaled_height >>= 1;
			if ( qgxLoadTlut && gl_ext_palettedtexture->value && samples == gx_solid_format )
			{
				uploaded_paletted = true;
				GX_BuildPalettedTexture( paletted_texture, ( unsigned char * ) scaled, scaled_width, scaled_height );
				dst = GX_CopyTexV8(paletted_texture, scaled_width, scaled_height, dst);
			}
			else
			{
				dst = GX_CopyTexRGBA8((byte*)scaled, scaled_width, scaled_height, dst);
			}
		};
		GX_BindCurrentTex(changed, GX_TF_RGBA8, GX_TRUE);
	}
done: ;
#endif

	Sys_BigStackFree(256*256 * sizeof(unsigned) + 256*256, "GX_Upload32");

	if (mipmap)
	{
		GX_SetMinMag(gx_filter_min, gx_filter_max);
	}
	else
	{
		GX_SetMinMag(gx_filter_max, gx_filter_max);
	}

	return (samples == gx_alpha_format);
}

/*
===============
GX_Upload8

Returns has_alpha
===============
*/
/*
static qboolean IsPowerOf2( int value )
{
	int i = 1;


	while ( 1 )
	{
		if ( value == i )
			return true;
		if ( i > value )
			return false;
		i <<= 1;
	}
}
*/

qboolean GX_Upload8 (byte *data, int width, int height,  qboolean mipmap, qboolean is_sky )
{
	unsigned*	trans;
	int			i, s;
	int			p;
	qboolean	ret;

	s = width*height;

	if (s > (512*256 * sizeof(unsigned))/4)
		ri.Sys_Error (ERR_DROP, "GX_Upload8: too large");

	if ( qgxLoadTlut && 
		 gl_ext_palettedtexture->value && 
		 is_sky )
	{
		GX_LoadAndBind(data, width * height, width, height, GX_TF_CI8);

		GX_SetMinMag(gx_filter_max, gx_filter_max);
	}
	else
	{
		trans = Sys_BigStackAlloc(512*256 * sizeof(unsigned),"GX_Upload8");
		for (i=0 ; i<s ; i++)
		{
			p = data[i];
			trans[i] = d_8to24table[p];

			if (p == 255)
			{	// transparent, so scan around for another color
				// to avoid alpha fringes
				// FIXME: do a full flood fill so mips work...
				if (i > width && data[i-width] != 255)
					p = data[i-width];
				else if (i < s-width && data[i+width] != 255)
					p = data[i+width];
				else if (i > 0 && data[i-1] != 255)
					p = data[i-1];
				else if (i < s-1 && data[i+1] != 255)
					p = data[i+1];
				else
					p = 0;
				// copy rgb components
				((byte *)&trans[i])[0] = ((byte *)&d_8to24table[p])[0];
				((byte *)&trans[i])[1] = ((byte *)&d_8to24table[p])[1];
				((byte *)&trans[i])[2] = ((byte *)&d_8to24table[p])[2];
			}
		}

		ret = GX_Upload32 (trans, width, height, mipmap);
		Sys_BigStackFree(512*256 * sizeof(unsigned),"GX_Upload8");
		return ret;
	};
}


/*
================
GX_LoadPic

This is also used as an entry point for the generated r_notexture
================
*/
image_t *GX_LoadPic (char *name, byte *pic, int width, int height, imagetype_t type, int bits)
{
	image_t		*image;
	int			i;

	// find a free image_t
	for (i=0, image=gxtextures ; i<numgxtextures ; i++,image++)
	{
		if (!image->texnum)
			break;
	}
	if (i == numgxtextures)
	{
		if (numgxtextures == MAX_GXTEXTURES)
			ri.Sys_Error (ERR_DROP, "MAX_GXTEXTURES");
		numgxtextures++;
	}
	image = &gxtextures[i];

	if (strlen(name) >= sizeof(image->name))
		ri.Sys_Error (ERR_DROP, "Draw_LoadPic: \"%s\" is too long", name);
	strcpy (image->name, name);
	image->registration_sequence = registration_sequence;

	image->width = width;
	image->height = height;
	image->type = type;

	if (type == it_skin && bits == 8)
		R_FloodFillSkin(pic, width, height);

	// load little pics into the scrap
	if (image->type == it_pic && bits == 8
		&& image->width < 64 && image->height < 64)
	{
		int		x, y;
		int		i, j, k;
		int		texnum;

		texnum = Scrap_AllocBlock (image->width, image->height, &x, &y);
		if (texnum == -1)
			goto nonscrap;
		scrap_dirty = true;

		// copy the texels into the scrap block
		k = 0;
		for (i=0 ; i<image->height ; i++)
			for (j=0 ; j<image->width ; j++, k++)
				scrap_texels[texnum][(y+i)*BLOCK_WIDTH + x + j] = pic[k];
		image->texnum = TEXNUM_SCRAPS + texnum;
		image->scrap = true;
		image->has_alpha = true;
		image->sl = (x+0.01)/(float)BLOCK_WIDTH;
		image->sh = (x+image->width-0.01)/(float)BLOCK_WIDTH;
		image->tl = (y+0.01)/(float)BLOCK_WIDTH;
		image->th = (y+image->height-0.01)/(float)BLOCK_WIDTH;
	}
	else
	{
nonscrap:
		image->scrap = false;
		image->texnum = TEXNUM_IMAGES + (image - gxtextures);
		GX_Bind(image->texnum);
		// Mipmaps disabled; pointless for this engine in GX hardware:
		if (bits == 8)
			image->has_alpha = GX_Upload8 (pic, width, height, false/*(image->type != it_pic && image->type != it_sky)*/, image->type == it_sky );
		else
			image->has_alpha = GX_Upload32 ((unsigned *)pic, width, height, false/*(image->type != it_pic && image->type != it_sky)*/ );
		image->upload_width = upload_width;		// after power of 2 and scales
		image->upload_height = upload_height;
		image->paletted = uploaded_paletted;
		image->sl = 0;
		image->sh = 1;
		image->tl = 0;
		image->th = 1;
	}

	return image;
}


/*
================
GX_LoadWal
================
*/
image_t *GX_LoadWal (char *name)
{
	miptex_t	*mt;
	int			width, height, ofs;
	image_t		*image;

	ri.FS_LoadFile (name, (void **)&mt, true);
	if (!mt)
	{
		ri.Con_Printf (PRINT_ALL, "GX_FindImage: can't load %s\n", name);
		return r_notexture;
	}

	width = LittleLong (mt->width);
	height = LittleLong (mt->height);
	ofs = LittleLong (mt->offsets[0]);

	image = GX_LoadPic (name, (byte *)mt + ofs, width, height, it_wall, 8);

	ri.FS_FreeFile ((void *)mt);

	return image;
}

/*
===============
GX_FindImage

Finds or loads the given image
===============
*/
image_t	*GX_FindImage (char *name, imagetype_t type)
{
	image_t	*image;
	int		i, len;
	byte	*pic, *palette;
	int		width, height;

	if (!name)
		return NULL;	//	ri.Sys_Error (ERR_DROP, "GX_FindImage: NULL name");
	len = strlen(name);
	if (len<5)
		return NULL;	//	ri.Sys_Error (ERR_DROP, "GX_FindImage: bad name: %s", name);

	// look for it
	for (i=0, image=gxtextures ; i<numgxtextures ; i++,image++)
	{
		if (!strcmp(name, image->name))
		{
			image->registration_sequence = registration_sequence;
			return image;
		}
	}

	//
	// load the pic from disk
	//
	pic = NULL;
	palette = NULL;
	if (!strcmp(name+len-4, ".pcx"))
	{
		LoadPCX (name, &pic, &palette, &width, &height);
		if (!pic)
			return NULL; // ri.Sys_Error (ERR_DROP, "GX_FindImage: can't load %s", name);
		image = GX_LoadPic (name, pic, width, height, type, 8);
	}
	else if (!strcmp(name+len-4, ".wal"))
	{
		image = GX_LoadWal (name);
	}
	else if (!strcmp(name+len-4, ".tga"))
	{
		LoadTGA (name, &pic, &width, &height);
		if (!pic)
			return NULL; // ri.Sys_Error (ERR_DROP, "GX_FindImage: can't load %s", name);
		image = GX_LoadPic (name, pic, width, height, type, 32);
	}
	else
		return NULL;	//	ri.Sys_Error (ERR_DROP, "GX_FindImage: bad extension on: %s", name);


	if (pic)
		free(pic);
	if (palette)
		free(palette);

	return image;
}



/*
===============
R_RegisterSkin
===============
*/
struct image_s *R_RegisterSkin (char *name)
{
	return GX_FindImage (name, it_skin);
}


/*
================
GX_FreeUnusedImages

Any image that was not touched on this registration sequence
will be freed.
================
*/
void GX_FreeUnusedImages (void)
{
	int		i;
	image_t	*image;

	// never free r_notexture or particle texture
	r_notexture->registration_sequence = registration_sequence;
	r_particletexture->registration_sequence = registration_sequence;

	for (i=0, image=gxtextures ; i<numgxtextures ; i++, image++)
	{
		if (image->registration_sequence == registration_sequence)
			continue;		// used this sequence
		if (!image->registration_sequence)
			continue;		// free image_t slot
		if (image->type == it_pic)
			continue;		// don't free pics
		// free it
		GX_DeleteTexData(image->texnum);
		memset (image, 0, sizeof(*image));
	}
}


/*
===============
Draw_GetPalette
===============
*/
int Draw_GetPalette (void)
{
	int		i;
	int		r, g, b;
	unsigned	v;
	byte	*pic, *pal;
	int		width, height;

	// get the palette

	LoadPCX ("pics/colormap.pcx", &pic, &pal, &width, &height);
	if (!pal)
		ri.Sys_Error (ERR_FATAL, "Couldn't load pics/colormap.pcx");

	for (i=0 ; i<256 ; i++)
	{
		r = pal[i*3+0];
		g = pal[i*3+1];
		b = pal[i*3+2];
		
		v = (255<<24) + (r<<0) + (g<<8) + (b<<16);
		d_8to24table[i] = LittleLong(v);
	}

	d_8to24table[255] &= LittleLong(0xffffff);	// 255 is transparent

	free (pic);
	free (pal);

	return 0;
}


/*
===============
GX_InitImages
===============
*/
void	GX_InitImages (void)
{
	int		i, j;
	float	g = vid_gamma->value;

	registration_sequence = 1;

	// init intensity conversions
	intensity = ri.Cvar_Get ("intensity", "2", 0);

	if ( intensity->value <= 1 )
		ri.Cvar_Set( "intensity", "1" );

	gx_state.inverse_intensity = 1 / intensity->value;

	Draw_GetPalette ();

	if ( qgxLoadTlut )
	{
		ri.FS_LoadFile( "pics/16to8.dat", &gx_state.d_16to8table, true);
		if ( !gx_state.d_16to8table )
			ri.Sys_Error( ERR_FATAL, "Couldn't load pics/16to8.pcx");
	}

	for ( i = 0; i < 256; i++ )
	{
		if ( g == 1 )
		{
			gammatable[i] = i;
		}
		else
		{
			float inf;

			inf = 255 * pow ( (i+0.5)/255.5 , g ) + 0.5;
			if (inf < 0)
				inf = 0;
			if (inf > 255)
				inf = 255;
			gammatable[i] = inf;
		}
	}

	for (i=0 ; i<256 ; i++)
	{
		j = i*intensity->value;
		if (j > 255)
			j = 255;
		intensitytable[i] = j;
	}
}

/*
===============
GX_ShutdownImages
===============
*/
void	GX_ShutdownImages (void)
{
	int		i;
	image_t	*image;

	for (i=0, image=gxtextures ; i<numgxtextures ; i++, image++)
	{
		if (!image->registration_sequence)
			continue;		// free image_t slot
		// free it
		GX_DeleteTexData(image->texnum);
		memset (image, 0, sizeof(*image));
	}
}

