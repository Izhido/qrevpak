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
// r_surf.c: surface-related refresh code

#ifdef GXQUAKE

#include <gccore.h>

#include "quakedef.h"

extern Mtx gx_modelview_matrices[32];

extern int gx_cur_modelview_matrix;

extern u8 gx_z_test_enabled;

extern u8 gx_z_write_enabled;

extern qboolean gx_blend_enabled;

extern u8 gx_blend_src_value;

extern u8 gx_blend_dst_value;

extern u8 gx_cur_vertex_format;

extern u8 gx_cur_r;

extern u8 gx_cur_g;

extern u8 gx_cur_b;

extern u8 gx_cur_a;


int			skytexturenum;

#ifndef GL_RGBA4
#define	GL_RGBA4	0
#endif


int		lightmap_bytes;		// 1, 2, or 4

int		lightmap_textures;

unsigned		blocklights[18*18];

#define	BLOCK_WIDTH		128
#define	BLOCK_HEIGHT	128

#define	MAX_LIGHTMAPS	64
int			active_lightmaps;

typedef struct gxRect_s {
	unsigned char l,t,w,h;
} gxRect_t;

gxpoly_t	*lightmap_polys[MAX_LIGHTMAPS];
qboolean	lightmap_modified[MAX_LIGHTMAPS];
gxRect_t	lightmap_rectchange[MAX_LIGHTMAPS];

int			allocated[MAX_LIGHTMAPS][BLOCK_WIDTH];

// the lightmap texture data needs to be kept in
// main memory so texsubimage can update properly
byte		lightmaps[4*MAX_LIGHTMAPS*BLOCK_WIDTH*BLOCK_HEIGHT];

// For gx_texsort 0
msurface_t  *skychain = NULL;
msurface_t  *waterchain = NULL;

void R_RenderDynamicLightmaps (msurface_t *fa);

/*
===============
R_AddDynamicLights
===============
*/
void R_AddDynamicLights (msurface_t *surf)
{
	int			lnum;
	int			sd, td;
	float		dist, rad, minlight;
	vec3_t		impact, local;
	int			s, t;
	int			i;
	int			smax, tmax;
	mtexinfo_t	*tex;

	smax = (surf->extents[0]>>4)+1;
	tmax = (surf->extents[1]>>4)+1;
	tex = surf->texinfo;

	for (lnum=0 ; lnum<MAX_DLIGHTS ; lnum++)
	{
		if ( !(surf->dlightbits & (1<<lnum) ) )
			continue;		// not lit by this light

		rad = cl_dlights[lnum].radius;
		dist = DotProduct (cl_dlights[lnum].origin, surf->plane->normal) -
				surf->plane->dist;
		rad -= fabs(dist);
		minlight = cl_dlights[lnum].minlight;
		if (rad < minlight)
			continue;
		minlight = rad - minlight;

		for (i=0 ; i<3 ; i++)
		{
			impact[i] = cl_dlights[lnum].origin[i] -
					surf->plane->normal[i]*dist;
		}

		local[0] = DotProduct (impact, tex->vecs[0]) + tex->vecs[0][3];
		local[1] = DotProduct (impact, tex->vecs[1]) + tex->vecs[1][3];

		local[0] -= surf->texturemins[0];
		local[1] -= surf->texturemins[1];
		
		for (t = 0 ; t<tmax ; t++)
		{
			td = local[1] - t*16;
			if (td < 0)
				td = -td;
			for (s=0 ; s<smax ; s++)
			{
				sd = local[0] - s*16;
				if (sd < 0)
					sd = -sd;
				if (sd > td)
					dist = sd + (td>>1);
				else
					dist = td + (sd>>1);
				if (dist < minlight)
					blocklights[t*smax + s] += (rad - dist)*256;
			}
		}
	}
}


/*
===============
R_BuildLightMap

Combine and scale multiple lightmaps into the 8.8 format in blocklights
===============
*/
void R_BuildLightMap (msurface_t *surf, byte *dest, int stride)
{
	int			smax, tmax;
	int			t;
	int			i, j, size;
	byte		*lightmap;
	unsigned	scale;
	int			maps;
	int			lightadj[4];
	unsigned	*bl;

	surf->cached_dlight = (surf->dlightframe == r_framecount);

	smax = (surf->extents[0]>>4)+1;
	tmax = (surf->extents[1]>>4)+1;
	size = smax*tmax;
	lightmap = surf->samples;

// set to full bright if no light data
	if (r_fullbright.value || !cl.worldmodel->lightdata)
	{
		for (i=0 ; i<size ; i++)
			blocklights[i] = 255*256;
		goto store;
	}

// clear to no light
	for (i=0 ; i<size ; i++)
		blocklights[i] = 0;

// add all the lightmaps
	if (lightmap)
		for (maps = 0 ; maps < MAXLIGHTMAPS && surf->styles[maps] != 255 ;
			 maps++)
		{
			scale = d_lightstylevalue[surf->styles[maps]];
			surf->cached_light[maps] = scale;	// 8.8 fraction
			for (i=0 ; i<size ; i++)
				blocklights[i] += lightmap[i] * scale;
			lightmap += size;	// skip to next lightmap
		}

// add all the dynamic lights
	if (surf->dlightframe == r_framecount)
		R_AddDynamicLights (surf);

// bound, invert, and shift
store:
	switch (gx_lightmap_format)
	{
	case GX_TF_RGBA8:
		stride -= (smax<<2);
		bl = blocklights;
		for (i=0 ; i<tmax ; i++, dest += stride)
		{
			for (j=0 ; j<smax ; j++)
			{
				t = *bl++;
				t >>= 7;
				if (t > 255)
					t = 255;
				dest[3] = 255-t;
				dest += 4;
			}
		}
		break;
	case GL_ALPHA:
	case GL_LUMINANCE:
	case GL_INTENSITY:
		bl = blocklights;
		for (i=0 ; i<tmax ; i++, dest += stride)
		{
			for (j=0 ; j<smax ; j++)
			{
				t = *bl++;
				t >>= 7;
				if (t > 255)
					t = 255;
				dest[j] = 255-t;
			}
		}
		break;
	default:
		Sys_Error ("Bad lightmap format");
	}
}


/*
===============
R_TextureAnimation

Returns the proper texture for a given time and base texture
===============
*/
texture_t *R_TextureAnimation (texture_t *base)
{
	int		reletive;
	int		count;

	if (currententity->frame)
	{
		if (base->alternate_anims)
			base = base->alternate_anims;
	}
	
	if (!base->anim_total)
		return base;

	reletive = (int)(cl.time*10) % base->anim_total;

	count = 0;	
	while (base->anim_min > reletive || base->anim_max <= reletive)
	{
		base = base->anim_next;
		if (!base)
			Sys_Error ("R_TextureAnimation: broken cycle");
		if (++count > 100)
			Sys_Error ("R_TextureAnimation: infinite cycle");
	}

	return base;
}


/*
=============================================================

	BRUSH MODELS

=============================================================
*/


extern	int		solidskytexture;
extern	int		alphaskytexture;
extern	float	speedscale;		// for top sky and bottom sky

void DrawGXWaterPoly (gxpoly_t *p);
void DrawGXWaterPolyLightmap (gxpoly_t *p);

lpMTexFUNC qgxMTexCoord2fSGIS = NULL;
lpSelTexFUNC qgxSelectTextureSGIS = NULL;

qboolean mtexenabled = false;

void GX_SelectTexture (GLenum target);

void GX_DisableMultitexture(void) 
{
	if (mtexenabled) {
		gx_cur_vertex_format = GX_VTXFMT0;
 		GX_SetVtxDesc(GX_VA_TEX0, GX_NONE);
		GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLOR0A0);
		GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
		GX_SelectTexture(TEXTURE0_SGIS);
		mtexenabled = false;
	}
}

void GX_EnableMultitexture(void) 
{
	if (gx_mtexable) {
		GX_SelectTexture(TEXTURE1_SGIS);
		gx_cur_vertex_format = GX_VTXFMT1;
 		GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);
 		GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
		GX_SetTevOp(GX_TEVSTAGE0, GX_REPLACE);
		mtexenabled = true;
	}
}

#if 0
/*
================
R_DrawSequentialPoly

Systems that have fast state and texture changes can
just do everything as it passes with no need to sort
================
*/
void R_DrawSequentialPoly (msurface_t *s)
{
	gxpoly_t	*p;
	float		*v;
	int			i;
	texture_t	*t;

	//
	// normal lightmaped poly
	//
	if (! (s->flags & (SURF_DRAWSKY|SURF_DRAWTURB|SURF_UNDERWATER) ) )
	{
		p = s->polys;

		t = R_TextureAnimation (s->texinfo->texture);
		GX_Bind (t->gx_texturenum);
		GX_Begin (GX_TRIANGLEFAN, gx_cur_vertex_format, p->numverts);
		v = p->verts[0];
		for (i=0 ; i<p->numverts ; i++, v+= VERTEXSIZE)
		{
			GX_Position3f32(v[0], v[1], v[2]);
			GX_Color4u8(gx_cur_r, gx_cur_g, gx_cur_b, gx_cur_a);
			GX_TexCoord2f32 (v[3], v[4]);
		}
		GX_End ();

		GX_Bind (lightmap_textures + s->lightmaptexturenum);
		GX_SetBlendMode(GX_BM_BLEND, gx_blend_src_value, gx_blend_dst_value, GX_LO_NOOP); 
		GX_Begin (GX_TRIANGLEFAN, gx_cur_vertex_format, p->numverts);
		v = p->verts[0];
		for (i=0 ; i<p->numverts ; i++, v+= VERTEXSIZE)
		{
			GX_Position3f32(v[0], v[1], v[2]);
			GX_Color4u8(gx_cur_r, gx_cur_g, gx_cur_b, gx_cur_a);
			GX_TexCoord2f32 (v[5], v[6]);
		}
		GX_End ();

		GX_SetBlendMode(GX_BM_NONE, gx_blend_src_value, gx_blend_dst_value, GX_LO_NOOP); 

		return;
	}

	//
	// subdivided water surface warp
	//
	if (s->flags & SURF_DRAWTURB)
	{
		GX_Bind (s->texinfo->texture->gx_texturenum);
		EmitWaterPolys (s);
		return;
	}

	//
	// subdivided sky warp
	//
	if (s->flags & SURF_DRAWSKY)
	{
		GX_Bind (solidskytexture);
		speedscale = realtime*8;
		speedscale -= (int)speedscale;

		EmitSkyPolys (s);

		gx_blend_src_value = GX_BL_SRCALPHA;
		gx_blend_dst_value = GX_BL_INVSRCALPHA;
		GX_SetBlendMode(GX_BM_BLEND, gx_blend_src_value, gx_blend_dst_value, GX_LO_NOOP); 
		GX_Bind (alphaskytexture);
		speedscale = realtime*16;
		speedscale -= (int)speedscale;
		EmitSkyPolys (s);
		if (gx_lightmap_format == GL_LUMINANCE)
		{
			gx_blend_src_value = GX_BL_ZERO;
			gx_blend_dst_value = GX_BL_INVSRCCLR;
		};

		GX_SetBlendMode(GX_BM_NONE, gx_blend_src_value, gx_blend_dst_value, GX_LO_NOOP); 
	}

	//
	// underwater warped with lightmap
	//
	p = s->polys;

	t = R_TextureAnimation (s->texinfo->texture);
	GX_Bind (t->gx_texturenum);
	DrawGXWaterPoly (p);

	GX_Bind (lightmap_textures + s->lightmaptexturenum);
	GX_SetBlendMode(GX_BM_BLEND, gx_blend_src_value, gx_blend_dst_value, GX_LO_NOOP); 
	DrawGLWaterPolyLightmap (p);
	GX_SetBlendMode(GX_BM_NONE, gx_blend_src_value, gx_blend_dst_value, GX_LO_NOOP); 
}
#else
/*
================
R_DrawSequentialPoly

Systems that have fast state and texture changes can
just do everything as it passes with no need to sort
================
*/
void R_DrawSequentialPoly (msurface_t *s)
{
	gxpoly_t	*p;
	float		*v;
	int			i;
	texture_t	*t;
	vec3_t		nv, dir;
	float		ss, ss2, length;
	float		s1, t1;
	gxRect_t	*theRect;

	//
	// normal lightmaped poly
	//

	if (! (s->flags & (SURF_DRAWSKY|SURF_DRAWTURB|SURF_UNDERWATER) ) )
	{
		R_RenderDynamicLightmaps (s);
		if (gx_mtexable) {
			p = s->polys;

			t = R_TextureAnimation (s->texinfo->texture);
			// Binds world to texture env 0
			GX_SelectTexture(TEXTURE0_SGIS);
			GX_Bind (t->gx_texturenum);
			GX_SetTevOp(GX_TEVSTAGE0, GX_REPLACE);
			// Binds lightmap to texenv 1
			GX_EnableMultitexture(); // Same as SelectTexture (TEXTURE1)
			GX_Bind (lightmap_textures + s->lightmaptexturenum);
			i = s->lightmaptexturenum;
			if (lightmap_modified[i])
			{
				lightmap_modified[i] = false;
				theRect = &lightmap_rectchange[i];
				GX_LoadSubAndBind (lightmaps+(i* BLOCK_HEIGHT + theRect->t) *BLOCK_WIDTH*lightmap_bytes, 0, theRect->t, BLOCK_WIDTH, theRect->h, gx_lightmap_format, 0);
				theRect->l = BLOCK_WIDTH;
				theRect->t = BLOCK_HEIGHT;
				theRect->h = 0;
				theRect->w = 0;
			}
			GX_SetTevOp(GX_TEVSTAGE0, GX_BLEND);
			/**************************************** THIS SHOULD BE POSSIBLE IN GX. REIMPLEMENT ASAP:
			glBegin(GL_POLYGON);
			v = p->verts[0];
			for (i=0 ; i<p->numverts ; i++, v+= VERTEXSIZE)
			{
				qgxMTexCoord2fSGIS (TEXTURE0_SGIS, v[3], v[4]);
				qgxMTexCoord2fSGIS (TEXTURE1_SGIS, v[5], v[6]);
				glVertex3fv (v);
			}
			glEnd ();
			*********************************************************************************/
			return;
		} else {
			p = s->polys;

			t = R_TextureAnimation (s->texinfo->texture);
			GX_Bind (t->gx_texturenum);
			GX_Begin(GX_TRIANGLEFAN, gx_cur_vertex_format, p->numverts);
			v = p->verts[0];
			for (i=0 ; i<p->numverts ; i++, v+= VERTEXSIZE)
			{
				GX_Position3f32(v[0], v[1], v[2]);
				GX_Color4u8(gx_cur_r, gx_cur_g, gx_cur_b, gx_cur_a);
				GX_TexCoord2f32 (v[3], v[4]);
			}
			GX_End();

			GX_Bind (lightmap_textures + s->lightmaptexturenum);
			GX_SetBlendMode(GX_BM_BLEND, gx_blend_src_value, gx_blend_dst_value, GX_LO_NOOP); 
			GX_Begin(GX_TRIANGLEFAN, gx_cur_vertex_format, p->numverts);
			v = p->verts[0];
			for (i=0 ; i<p->numverts ; i++, v+= VERTEXSIZE)
			{
				GX_Position3f32(v[0], v[1], v[2]);
				GX_Color4u8(gx_cur_r, gx_cur_g, gx_cur_b, gx_cur_a);
				GX_TexCoord2f32 (v[5], v[6]);
			}
			GX_End();

			GX_SetBlendMode(GX_BM_NONE, gx_blend_src_value, gx_blend_dst_value, GX_LO_NOOP); 
		}

		return;
	}

	//
	// subdivided water surface warp
	//

	if (s->flags & SURF_DRAWTURB)
	{
		GX_DisableMultitexture();
		GX_Bind (s->texinfo->texture->gx_texturenum);
		EmitWaterPolys (s);
		return;
	}

	//
	// subdivided sky warp
	//
	if (s->flags & SURF_DRAWSKY)
	{
		GX_DisableMultitexture();
		GX_Bind (solidskytexture);
		speedscale = realtime*8;
		speedscale -= (int)speedscale & ~127;

		EmitSkyPolys (s);

		GX_SetBlendMode(GX_BM_BLEND, gx_blend_src_value, gx_blend_dst_value, GX_LO_NOOP); 
		GX_Bind (alphaskytexture);
		speedscale = realtime*16;
		speedscale -= (int)speedscale & ~127;
		EmitSkyPolys (s);

		GX_SetBlendMode(GX_BM_NONE, gx_blend_src_value, gx_blend_dst_value, GX_LO_NOOP); 
		return;
	}

	//
	// underwater warped with lightmap
	//
	R_RenderDynamicLightmaps (s);
	if (gx_mtexable) {
		p = s->polys;

		t = R_TextureAnimation (s->texinfo->texture);
		GX_SelectTexture(TEXTURE0_SGIS);
		GX_Bind (t->gx_texturenum);
		GX_SetTevOp(GX_TEVSTAGE0, GX_REPLACE);
		GX_EnableMultitexture();
		GX_Bind (lightmap_textures + s->lightmaptexturenum);
		i = s->lightmaptexturenum;
		if (lightmap_modified[i])
		{
			lightmap_modified[i] = false;
			theRect = &lightmap_rectchange[i];
			GX_LoadSubAndBind (lightmaps+(i* BLOCK_HEIGHT + theRect->t) *BLOCK_WIDTH*lightmap_bytes, 0, theRect->t, BLOCK_WIDTH, theRect->h, gx_lightmap_format, 0);
			theRect->l = BLOCK_WIDTH;
			theRect->t = BLOCK_HEIGHT;
			theRect->h = 0;
			theRect->w = 0;
		}
		GX_SetTevOp(GX_TEVSTAGE0, GX_BLEND);
		/**************************************** THIS SHOULD BE POSSIBLE IN GX. REIMPLEMENT ASAP:
		glBegin (GL_TRIANGLE_FAN);
		v = p->verts[0];
		for (i=0 ; i<p->numverts ; i++, v+= VERTEXSIZE)
		{
			qgxMTexCoord2fSGIS (TEXTURE0_SGIS, v[3], v[4]);
			qgxMTexCoord2fSGIS (TEXTURE1_SGIS, v[5], v[6]);

			nv[0] = v[0] + 8*sin(v[1]*0.05+realtime)*sin(v[2]*0.05+realtime);
			nv[1] = v[1] + 8*sin(v[0]*0.05+realtime)*sin(v[2]*0.05+realtime);
			nv[2] = v[2];

			glVertex3fv (nv);
		}
		glEnd ();
		*********************************************************************************/

	} else {
		p = s->polys;

		t = R_TextureAnimation (s->texinfo->texture);
		GX_Bind (t->gx_texturenum);
		DrawGXWaterPoly (p);

		GX_Bind (lightmap_textures + s->lightmaptexturenum);
		GX_SetBlendMode(GX_BM_BLEND, gx_blend_src_value, gx_blend_dst_value, GX_LO_NOOP); 
		DrawGXWaterPolyLightmap (p);
		GX_SetBlendMode(GX_BM_NONE, gx_blend_src_value, gx_blend_dst_value, GX_LO_NOOP); 
	}
}
#endif


/*
================
DrawGXWaterPoly

Warp the vertex coordinates
================
*/
void DrawGXWaterPoly (gxpoly_t *p)
{
	int		i;
	float	*v;
	float	s, t, os, ot;
	vec3_t	nv;

	GX_DisableMultitexture();

	GX_Begin (GX_TRIANGLEFAN, gx_cur_vertex_format, p->numverts);
	v = p->verts[0];
	for (i=0 ; i<p->numverts ; i++, v+= VERTEXSIZE)
	{
		nv[0] = v[0] + 8*sin(v[1]*0.05+realtime)*sin(v[2]*0.05+realtime);
		nv[1] = v[1] + 8*sin(v[0]*0.05+realtime)*sin(v[2]*0.05+realtime);
		nv[2] = v[2];

		GX_Position3f32(nv[0], nv[1], nv[2]);
		GX_Color4u8(gx_cur_r, gx_cur_g, gx_cur_b, gx_cur_a);
		GX_TexCoord2f32 (v[3], v[4]);
	}
	GX_End ();
}

void DrawGXWaterPolyLightmap (gxpoly_t *p)
{
	int		i;
	float	*v;
	float	s, t, os, ot;
	vec3_t	nv;

	GX_DisableMultitexture();

	GX_Begin (GX_TRIANGLEFAN, gx_cur_vertex_format, p->numverts);
	v = p->verts[0];
	for (i=0 ; i<p->numverts ; i++, v+= VERTEXSIZE)
	{
		nv[0] = v[0] + 8*sin(v[1]*0.05+realtime)*sin(v[2]*0.05+realtime);
		nv[1] = v[1] + 8*sin(v[0]*0.05+realtime)*sin(v[2]*0.05+realtime);
		nv[2] = v[2];

		GX_Position3f32(nv[0], nv[1], nv[2]);
		GX_Color4u8(gx_cur_r, gx_cur_g, gx_cur_b, gx_cur_a);
		GX_TexCoord2f32 (v[5], v[6]);
	}
	GX_End ();
}

/*
================
DrawGXPoly
================
*/
void DrawGXPoly (gxpoly_t *p)
{
	int		i;
	float	*v;

	GX_Begin(GX_TRIANGLEFAN, gx_cur_vertex_format, p->numverts);
	v = p->verts[0];
	for (i=0 ; i<p->numverts ; i++, v+= VERTEXSIZE)
	{
		GX_Position3f32(v[0], v[1], v[2]);
		GX_Color4u8(gx_cur_r, gx_cur_g, gx_cur_b, gx_cur_a);
		GX_TexCoord2f32 (v[3], v[4]);
	}
	GX_End ();
}


/*
================
R_BlendLightmaps
================
*/
void R_BlendLightmaps (void)
{
	int			i, j;
	gxpoly_t	*p;
	float		*v;
	gxRect_t	*theRect;

	if (r_fullbright.value)
		return;
	if (!gx_texsort.value)
		return;

	gx_z_write_enabled = GX_FALSE;
	GX_SetZMode(gx_z_test_enabled, GX_LEQUAL, gx_z_write_enabled);		// don't bother writing Z

	if (gx_lightmap_format == GL_LUMINANCE)
	{
		gx_blend_src_value = GX_BL_ZERO;
		gx_blend_dst_value = GX_BL_INVSRCCLR;
	}
	else if (gx_lightmap_format == GL_INTENSITY)
	{
		GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
		gx_cur_r = 0;
		gx_cur_g = 0;
		gx_cur_b = 0;
		gx_cur_a = 255;
		gx_blend_src_value = GX_BL_SRCALPHA;
		gx_blend_dst_value = GX_BL_INVSRCALPHA;
	}

	if (!r_lightmap.value)
	{
		gx_blend_enabled = true;
	}

	if(gx_blend_enabled)
	{
		GX_SetBlendMode(GX_BM_BLEND, gx_blend_src_value, gx_blend_dst_value, GX_LO_NOOP); 
	};

	for (i=0 ; i<MAX_LIGHTMAPS ; i++)
	{
		p = lightmap_polys[i];
		if (!p)
			continue;
		GX_Bind(lightmap_textures+i);
		if (lightmap_modified[i])
		{
			lightmap_modified[i] = false;
			theRect = &lightmap_rectchange[i];
//			GX_LoadAndBind (lightmaps+i*BLOCK_WIDTH*BLOCK_HEIGHT*lightmap_bytes, BLOCK_WIDTH*BLOCK_HEIGHT*lightmap_bytes, BLOCK_WIDTH, BLOCK_HEIGHT, gx_lightmap_format, 0);
//			GX_LoadAndBind (lightmaps+(i*BLOCK_HEIGHT+theRect->t)*BLOCK_WIDTH*lightmap_bytes, (BLOCK_HEIGHT+theRect->t)*BLOCK_WIDTH*lightmap_bytes, BLOCK_WIDTH, theRect->h, gx_lightmap_format, 0);
			GX_LoadSubAndBind (lightmaps+(i* BLOCK_HEIGHT + theRect->t) *BLOCK_WIDTH*lightmap_bytes, 0, theRect->t, BLOCK_WIDTH, theRect->h, gx_lightmap_format, 0);
			theRect->l = BLOCK_WIDTH;
			theRect->t = BLOCK_HEIGHT;
			theRect->h = 0;
			theRect->w = 0;
		}
		for ( ; p ; p=p->chain)
		{
			if (p->flags & SURF_UNDERWATER)
				DrawGXWaterPolyLightmap (p);
			else
			{
				GX_Begin (GX_TRIANGLEFAN, gx_cur_vertex_format, p->numverts);
				v = p->verts[0];
				for (j=0 ; j<p->numverts ; j++, v+= VERTEXSIZE)
				{
					GX_Position3f32(v[0], v[1], v[2]);
					GX_Color4u8(gx_cur_r, gx_cur_g, gx_cur_b, gx_cur_a);
					GX_TexCoord2f32 (v[5], v[6]);
				}
				GX_End ();
			}
		}
	}

	gx_blend_enabled = false;
	GX_SetBlendMode(GX_BM_NONE, gx_blend_src_value, gx_blend_dst_value, GX_LO_NOOP); 
	if (gx_lightmap_format == GL_LUMINANCE)
	{
		gx_blend_src_value = GX_BL_SRCALPHA;
		gx_blend_dst_value = GX_BL_INVSRCALPHA;
	}
	else if (gx_lightmap_format == GL_INTENSITY)
	{
		GX_SetTevOp(GX_TEVSTAGE0, GX_REPLACE);
		gx_cur_r = 255;
		gx_cur_g = 255;
		gx_cur_b = 255;
		gx_cur_a = 255;
	}

	gx_z_write_enabled = GX_TRUE;
	GX_SetZMode(gx_z_test_enabled, GX_LEQUAL, gx_z_write_enabled);		// back to normal Z buffering
}

/*
================
R_RenderBrushPoly
================
*/
void R_RenderBrushPoly (msurface_t *fa)
{
	texture_t	*t;
	byte		*base;
	int			maps;
	gxRect_t    *theRect;
	int smax, tmax;

	c_brush_polys++;

	if (fa->flags & SURF_DRAWSKY)
	{	// warp texture, no lightmaps
		EmitBothSkyLayers (fa);
		return;
	}
		
	t = R_TextureAnimation (fa->texinfo->texture);
	GX_Bind (t->gx_texturenum);

	if (fa->flags & SURF_DRAWTURB)
	{	// warp texture, no lightmaps
		EmitWaterPolys (fa);
		return;
	}

	if (fa->flags & SURF_UNDERWATER)
		DrawGXWaterPoly (fa->polys);
	else
		DrawGXPoly (fa->polys);

	// add the poly to the proper lightmap chain

	fa->polys->chain = lightmap_polys[fa->lightmaptexturenum];
	lightmap_polys[fa->lightmaptexturenum] = fa->polys;

	// check for lightmap modification
	for (maps = 0 ; maps < MAXLIGHTMAPS && fa->styles[maps] != 255 ;
		 maps++)
		if (d_lightstylevalue[fa->styles[maps]] != fa->cached_light[maps])
			goto dynamic;

	if (fa->dlightframe == r_framecount	// dynamic this frame
		|| fa->cached_dlight)			// dynamic previously
	{
dynamic:
		if (r_dynamic.value)
		{
			lightmap_modified[fa->lightmaptexturenum] = true;
			theRect = &lightmap_rectchange[fa->lightmaptexturenum];
			if (fa->light_t < theRect->t) {
				if (theRect->h)
					theRect->h += theRect->t - fa->light_t;
				theRect->t = fa->light_t;
			}
			if (fa->light_s < theRect->l) {
				if (theRect->w)
					theRect->w += theRect->l - fa->light_s;
				theRect->l = fa->light_s;
			}
			smax = (fa->extents[0]>>4)+1;
			tmax = (fa->extents[1]>>4)+1;
			if ((theRect->w + theRect->l) < (fa->light_s + smax))
				theRect->w = (fa->light_s-theRect->l)+smax;
			if ((theRect->h + theRect->t) < (fa->light_t + tmax))
				theRect->h = (fa->light_t-theRect->t)+tmax;
			base = lightmaps + fa->lightmaptexturenum*lightmap_bytes*BLOCK_WIDTH*BLOCK_HEIGHT;
			base += fa->light_t * BLOCK_WIDTH * lightmap_bytes + fa->light_s * lightmap_bytes;
			R_BuildLightMap (fa, base, BLOCK_WIDTH*lightmap_bytes);
		}
	}
}

/*
================
R_RenderDynamicLightmaps
Multitexture
================
*/
void R_RenderDynamicLightmaps (msurface_t *fa)
{
	texture_t	*t;
	byte		*base;
	int			maps;
	gxRect_t    *theRect;
	int smax, tmax;

	c_brush_polys++;

	if (fa->flags & ( SURF_DRAWSKY | SURF_DRAWTURB) )
		return;
		
	fa->polys->chain = lightmap_polys[fa->lightmaptexturenum];
	lightmap_polys[fa->lightmaptexturenum] = fa->polys;

	// check for lightmap modification
	for (maps = 0 ; maps < MAXLIGHTMAPS && fa->styles[maps] != 255 ;
		 maps++)
		if (d_lightstylevalue[fa->styles[maps]] != fa->cached_light[maps])
			goto dynamic;

	if (fa->dlightframe == r_framecount	// dynamic this frame
		|| fa->cached_dlight)			// dynamic previously
	{
dynamic:
		if (r_dynamic.value)
		{
			lightmap_modified[fa->lightmaptexturenum] = true;
			theRect = &lightmap_rectchange[fa->lightmaptexturenum];
			if (fa->light_t < theRect->t) {
				if (theRect->h)
					theRect->h += theRect->t - fa->light_t;
				theRect->t = fa->light_t;
			}
			if (fa->light_s < theRect->l) {
				if (theRect->w)
					theRect->w += theRect->l - fa->light_s;
				theRect->l = fa->light_s;
			}
			smax = (fa->extents[0]>>4)+1;
			tmax = (fa->extents[1]>>4)+1;
			if ((theRect->w + theRect->l) < (fa->light_s + smax))
				theRect->w = (fa->light_s-theRect->l)+smax;
			if ((theRect->h + theRect->t) < (fa->light_t + tmax))
				theRect->h = (fa->light_t-theRect->t)+tmax;
			base = lightmaps + fa->lightmaptexturenum*lightmap_bytes*BLOCK_WIDTH*BLOCK_HEIGHT;
			base += fa->light_t * BLOCK_WIDTH * lightmap_bytes + fa->light_s * lightmap_bytes;
			R_BuildLightMap (fa, base, BLOCK_WIDTH*lightmap_bytes);
		}
	}
}

/*
================
R_MirrorChain
================
*/
void R_MirrorChain (msurface_t *s)
{
	if (mirror)
		return;
	mirror = true;
	mirror_plane = s->plane;
}


#if 0
/*
================
R_DrawWaterSurfaces
================
*/
void R_DrawWaterSurfaces (void)
{
	int			i;
	msurface_t	*s;
	texture_t	*t;

	if (r_wateralpha.value == 1.0)
		return;

	//
	// go back to the world matrix
	//
	gx_modelview_matrices[gx_cur_modelview_matrix][0][0] = r_world_matrix[0];
	gx_modelview_matrices[gx_cur_modelview_matrix][0][1] = r_world_matrix[1];
	gx_modelview_matrices[gx_cur_modelview_matrix][0][2] = r_world_matrix[2];
	gx_modelview_matrices[gx_cur_modelview_matrix][0][3] = r_world_matrix[3];
	gx_modelview_matrices[gx_cur_modelview_matrix][1][0] = r_world_matrix[4];
	gx_modelview_matrices[gx_cur_modelview_matrix][1][1] = r_world_matrix[5];
	gx_modelview_matrices[gx_cur_modelview_matrix][1][2] = r_world_matrix[6];
	gx_modelview_matrices[gx_cur_modelview_matrix][1][3] = r_world_matrix[7];
	gx_modelview_matrices[gx_cur_modelview_matrix][2][0] = r_world_matrix[8];
	gx_modelview_matrices[gx_cur_modelview_matrix][2][1] = r_world_matrix[9];
	gx_modelview_matrices[gx_cur_modelview_matrix][2][2] = r_world_matrix[10];
	gx_modelview_matrices[gx_cur_modelview_matrix][2][3] = r_world_matrix[11];
	GX_LoadPosMtxImm(gx_modelview_matrices[gx_cur_modelview_matrix], GX_PNMTX0);

	GX_SetBlendMode(GX_BM_BLEND, gx_blend_src_value, gx_blend_dst_value, GX_LO_NOOP); 
	gx_cur_r = 255;
	gx_cur_g = 255;
	gx_cur_b = 255;
	gx_cur_a = r_wateralpha.value * 255.0;
	GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);

	for (i=0 ; i<cl.worldmodel->numtextures ; i++)
	{
		t = cl.worldmodel->textures[i];
		if (!t)
			continue;
		s = t->texturechain;
		if (!s)
			continue;
		if ( !(s->flags & SURF_DRAWTURB) )
			continue;

		// set modulate mode explicitly
		GX_Bind (t->gx_texturenum);

		for ( ; s ; s=s->texturechain)
			R_RenderBrushPoly (s);

		t->texturechain = NULL;
	}

	GX_SetTevOp(GX_TEVSTAGE0, GX_REPLACE);

	gx_cur_r = 255;
	gx_cur_g = 255;
	gx_cur_b = 255;
	gx_cur_a = 255;
	GX_SetBlendMode(GX_BM_NONE, gx_blend_src_value, gx_blend_dst_value, GX_LO_NOOP); 
}
#else
/*
================
R_DrawWaterSurfaces
================
*/
void R_DrawWaterSurfaces (void)
{
	int			i;
	msurface_t	*s;
	texture_t	*t;

	if (r_wateralpha.value == 1.0 && gx_texsort.value)
		return;

	//
	// go back to the world matrix
	//

	gx_modelview_matrices[gx_cur_modelview_matrix][0][0] = r_world_matrix[0];
	gx_modelview_matrices[gx_cur_modelview_matrix][0][1] = r_world_matrix[1];
	gx_modelview_matrices[gx_cur_modelview_matrix][0][2] = r_world_matrix[2];
	gx_modelview_matrices[gx_cur_modelview_matrix][0][3] = r_world_matrix[3];
	gx_modelview_matrices[gx_cur_modelview_matrix][1][0] = r_world_matrix[4];
	gx_modelview_matrices[gx_cur_modelview_matrix][1][1] = r_world_matrix[5];
	gx_modelview_matrices[gx_cur_modelview_matrix][1][2] = r_world_matrix[6];
	gx_modelview_matrices[gx_cur_modelview_matrix][1][3] = r_world_matrix[7];
	gx_modelview_matrices[gx_cur_modelview_matrix][2][0] = r_world_matrix[8];
	gx_modelview_matrices[gx_cur_modelview_matrix][2][1] = r_world_matrix[9];
	gx_modelview_matrices[gx_cur_modelview_matrix][2][2] = r_world_matrix[10];
	gx_modelview_matrices[gx_cur_modelview_matrix][2][3] = r_world_matrix[11];
	GX_LoadPosMtxImm(gx_modelview_matrices[gx_cur_modelview_matrix], GX_PNMTX0);

	if (r_wateralpha.value < 1.0) {
		GX_SetBlendMode(GX_BM_BLEND, gx_blend_src_value, gx_blend_dst_value, GX_LO_NOOP); 
		gx_cur_r = 255;
		gx_cur_g = 255;
		gx_cur_b = 255;
		gx_cur_a = r_wateralpha.value * 255,0;
		GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
	}

	if (!gx_texsort.value) {
		if (!waterchain)
			return;

		for ( s = waterchain ; s ; s=s->texturechain) {
			GX_Bind (s->texinfo->texture->gx_texturenum);
			EmitWaterPolys (s);
		}
		
		waterchain = NULL;
	} else {

		for (i=0 ; i<cl.worldmodel->numtextures ; i++)
		{
			t = cl.worldmodel->textures[i];
			if (!t)
				continue;
			s = t->texturechain;
			if (!s)
				continue;
			if ( !(s->flags & SURF_DRAWTURB ) )
				continue;

			// set modulate mode explicitly
			
			GX_Bind (t->gx_texturenum);

			for ( ; s ; s=s->texturechain)
				EmitWaterPolys (s);
			
			t->texturechain = NULL;
		}

	}

	if (r_wateralpha.value < 1.0) {
		GX_SetTevOp(GX_TEVSTAGE0, GX_REPLACE);

		gx_cur_r = 255;
		gx_cur_g = 255;
		gx_cur_b = 255;
		gx_cur_a = 255;
		GX_SetBlendMode(GX_BM_NONE, gx_blend_src_value, gx_blend_dst_value, GX_LO_NOOP); 
	}

}

#endif

/*
================
DrawTextureChains
================
*/
void DrawTextureChains (void)
{
	int		i;
	msurface_t	*s;
	texture_t	*t;

	if (!gx_texsort.value) {
		GX_DisableMultitexture();

		if (skychain) {
			R_DrawSkyChain(skychain);
			skychain = NULL;
		}

		return;
	} 

	for (i=0 ; i<cl.worldmodel->numtextures ; i++)
	{
		t = cl.worldmodel->textures[i];
		if (!t)
			continue;
		s = t->texturechain;
		if (!s)
			continue;
		if (i == skytexturenum)
			R_DrawSkyChain (s);
		else if (i == mirrortexturenum && r_mirroralpha.value != 1.0)
		{
			R_MirrorChain (s);
			continue;
		}
		else
		{
			if ((s->flags & SURF_DRAWTURB) && r_wateralpha.value != 1.0)
				continue;	// draw translucent water later
			for ( ; s ; s=s->texturechain)
				R_RenderBrushPoly (s);
		}

		t->texturechain = NULL;
	}
}

/*
=================
R_DrawBrushModel
=================
*/
void R_DrawBrushModel (entity_t *e)
{
	int			j, k;
	vec3_t		mins, maxs;
	int			i, numsurfaces;
	msurface_t	*psurf;
	float		dot;
	mplane_t	*pplane;
	model_t		*clmodel;
	qboolean	rotated;

	currententity = e;
	currenttexture = -1;

	clmodel = e->model;

	if (e->angles[0] || e->angles[1] || e->angles[2])
	{
		rotated = true;
		for (i=0 ; i<3 ; i++)
		{
			mins[i] = e->origin[i] - clmodel->radius;
			maxs[i] = e->origin[i] + clmodel->radius;
		}
	}
	else
	{
		rotated = false;
		VectorAdd (e->origin, clmodel->mins, mins);
		VectorAdd (e->origin, clmodel->maxs, maxs);
	}

	if (R_CullBox (mins, maxs))
		return;

	gx_cur_r = 255;
	gx_cur_g = 255;
	gx_cur_b = 255;
	gx_cur_a = 255;
	memset (lightmap_polys, 0, sizeof(lightmap_polys));

	VectorSubtract (r_refdef.vieworg, e->origin, modelorg);
	if (rotated)
	{
		vec3_t	temp;
		vec3_t	forward, right, up;

		VectorCopy (modelorg, temp);
		AngleVectors (e->angles, forward, right, up);
		modelorg[0] = DotProduct (temp, forward);
		modelorg[1] = -DotProduct (temp, right);
		modelorg[2] = DotProduct (temp, up);
	}

	psurf = &clmodel->surfaces[clmodel->firstmodelsurface];

// calculate dynamic lighting for bmodel if it's not an
// instanced model
	if (clmodel->firstmodelsurface != 0 && !gx_flashblend.value)
	{
		for (k=0 ; k<MAX_DLIGHTS ; k++)
		{
			if ((cl_dlights[k].die < cl.time) ||
				(!cl_dlights[k].radius))
				continue;

			R_MarkLights (&cl_dlights[k], 1<<k,
				clmodel->nodes + clmodel->hulls[0].firstclipnode);
		}
	}

	guMtxCopy(gx_modelview_matrices[gx_cur_modelview_matrix], gx_modelview_matrices[gx_cur_modelview_matrix + 1]);
	gx_cur_modelview_matrix++;
e->angles[0] = -e->angles[0];	// stupid quake bug
	R_RotateForEntity (e);
	GX_LoadPosMtxImm(gx_modelview_matrices[gx_cur_modelview_matrix], GX_PNMTX0);
e->angles[0] = -e->angles[0];	// stupid quake bug

	//
	// draw texture
	//
	for (i=0 ; i<clmodel->nummodelsurfaces ; i++, psurf++)
	{
	// find which side of the node we are on
		pplane = psurf->plane;

		dot = DotProduct (modelorg, pplane->normal) - pplane->dist;

	// draw the polygon
		if (((psurf->flags & SURF_PLANEBACK) && (dot < -BACKFACE_EPSILON)) ||
			(!(psurf->flags & SURF_PLANEBACK) && (dot > BACKFACE_EPSILON)))
		{
			if (gx_texsort.value)
				R_RenderBrushPoly (psurf);
			else
				R_DrawSequentialPoly (psurf);
		}
	}

	R_BlendLightmaps ();

	gx_cur_modelview_matrix--;
	GX_LoadPosMtxImm(gx_modelview_matrices[gx_cur_modelview_matrix], GX_PNMTX0);
}

/*
=============================================================

	WORLD MODEL

=============================================================
*/

/*
================
R_RecursiveWorldNode
================
*/
void R_RecursiveWorldNode (mnode_t *node)
{
	int			i, c, side, *pindex;
	vec3_t		acceptpt, rejectpt;
	mplane_t	*plane;
	msurface_t	*surf, **mark;
	mleaf_t		*pleaf;
	double		d, dot;
	vec3_t		mins, maxs;

	if (node->contents == CONTENTS_SOLID)
		return;		// solid

	if (node->visframe != r_visframecount)
		return;
	if (R_CullBox (node->minmaxs, node->minmaxs+3))
		return;
	
// if a leaf node, draw stuff
	if (node->contents < 0)
	{
		pleaf = (mleaf_t *)node;

		mark = pleaf->firstmarksurface;
		c = pleaf->nummarksurfaces;

		if (c)
		{
			do
			{
				(*mark)->visframe = r_framecount;
				mark++;
			} while (--c);
		}

	// deal with model fragments in this leaf
		if (pleaf->efrags)
			R_StoreEfrags (&pleaf->efrags);

		return;
	}

// node is just a decision point, so go down the apropriate sides

// find which side of the node we are on
	plane = node->plane;

	switch (plane->type)
	{
	case PLANE_X:
		dot = modelorg[0] - plane->dist;
		break;
	case PLANE_Y:
		dot = modelorg[1] - plane->dist;
		break;
	case PLANE_Z:
		dot = modelorg[2] - plane->dist;
		break;
	default:
		dot = DotProduct (modelorg, plane->normal) - plane->dist;
		break;
	}

	if (dot >= 0)
		side = 0;
	else
		side = 1;

// recurse down the children, front side first
	R_RecursiveWorldNode (node->children[side]);

// draw stuff
	c = node->numsurfaces;

	if (c)
	{
		surf = cl.worldmodel->surfaces + node->firstsurface;

		if (dot < 0 -BACKFACE_EPSILON)
			side = SURF_PLANEBACK;
		else if (dot > BACKFACE_EPSILON)
			side = 0;
		{
			for ( ; c ; c--, surf++)
			{
				if (surf->visframe != r_framecount)
					continue;

				// don't backface underwater surfaces, because they warp
				if ( !(surf->flags & SURF_UNDERWATER) && ( (dot < 0) ^ !!(surf->flags & SURF_PLANEBACK)) )
					continue;		// wrong side

				// if sorting by texture, just store it out
				if (gx_texsort.value)
				{
					if (!mirror
					|| surf->texinfo->texture != cl.worldmodel->textures[mirrortexturenum])
					{
						surf->texturechain = surf->texinfo->texture->texturechain;
						surf->texinfo->texture->texturechain = surf;
					}
				} else if (surf->flags & SURF_DRAWSKY) {
					surf->texturechain = skychain;
					skychain = surf;
				} else if (surf->flags & SURF_DRAWTURB) {
					surf->texturechain = waterchain;
					waterchain = surf;
				} else
					R_DrawSequentialPoly (surf);

			}
		}

	}

// recurse down the back side
	R_RecursiveWorldNode (node->children[!side]);
}



/*
=============
R_DrawWorld
=============
*/
void R_DrawWorld (void)
{
	entity_t	ent;
	int			i;

	memset (&ent, 0, sizeof(ent));
	ent.model = cl.worldmodel;

	VectorCopy (r_refdef.vieworg, modelorg);

	currententity = &ent;
	currenttexture = -1;

	gx_cur_r = 255;
	gx_cur_g = 255;
	gx_cur_b = 255;
	gx_cur_a = 255;
	memset (lightmap_polys, 0, sizeof(lightmap_polys));
#ifdef QUAKE2
	R_ClearSkyBox ();
#endif

	R_RecursiveWorldNode (cl.worldmodel->nodes);

	DrawTextureChains ();

	R_BlendLightmaps ();

#ifdef QUAKE2
	R_DrawSkyBox ();
#endif
}


/*
===============
R_MarkLeaves
===============
*/
void R_MarkLeaves (void)
{
	byte	*vis;
	mnode_t	*node;
	int		i;
	byte*	solid;

	if (r_oldviewleaf == r_viewleaf && !r_novis.value)
		return;
	
	if (mirror)
		return;

	r_visframecount++;
	r_oldviewleaf = r_viewleaf;

	solid = Sys_BigStackAlloc(4096 * sizeof(byte), "R_MarkLeaves");

	if (r_novis.value)
	{
		vis = solid;
		memset (solid, 0xff, (cl.worldmodel->numleafs+7)>>3);
	}
	else
		vis = Mod_LeafPVS (r_viewleaf, cl.worldmodel);
		
	for (i=0 ; i<cl.worldmodel->numleafs ; i++)
	{
		if (vis[i>>3] & (1<<(i&7)))
		{
			node = (mnode_t *)&cl.worldmodel->leafs[i+1];
			do
			{
				if (node->visframe == r_visframecount)
					break;
				node->visframe = r_visframecount;
				node = node->parent;
			} while (node);
		}
	}
	Sys_BigStackFree(4096 * sizeof(byte), "R_MarkLeaves");
}



/*
=============================================================================

  LIGHTMAP ALLOCATION

=============================================================================
*/

// returns a texture number and the position inside it
int AllocBlock (int w, int h, int *x, int *y)
{
	int		i, j;
	int		best, best2;
	int		bestx;
	int		texnum;

	for (texnum=0 ; texnum<MAX_LIGHTMAPS ; texnum++)
	{
		best = BLOCK_HEIGHT;

		for (i=0 ; i<BLOCK_WIDTH-w ; i++)
		{
			best2 = 0;

			for (j=0 ; j<w ; j++)
			{
				if (allocated[texnum][i+j] >= best)
					break;
				if (allocated[texnum][i+j] > best2)
					best2 = allocated[texnum][i+j];
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
			allocated[texnum][*x + i] = best + h;

		return texnum;
	}

	Sys_Error ("AllocBlock: full");
}


mvertex_t	*r_pcurrentvertbase;
model_t		*currentmodel;

int	nColinElim;

/*
================
BuildSurfaceDisplayList
================
*/
void BuildSurfaceDisplayList (msurface_t *fa)
{
	int			i, lindex, lnumverts, s_axis, t_axis;
	float		dist, lastdist, lzi, scale, u, v, frac;
	unsigned	mask;
	vec3_t		local, transformed;
	medge_t		*pedges, *r_pedge;
	mplane_t	*pplane;
	int			vertpage, newverts, newpage, lastvert;
	qboolean	visible;
	float		*vec;
	float		s, t;
	gxpoly_t	*poly;

// reconstruct the polygon
	pedges = currentmodel->edges;
	lnumverts = fa->numedges;
	vertpage = 0;

	//
	// draw texture
	//
	poly = Hunk_Alloc (sizeof(gxpoly_t) + (lnumverts-4) * VERTEXSIZE*sizeof(float));
	poly->next = fa->polys;
	poly->flags = fa->flags;
	fa->polys = poly;
	poly->numverts = lnumverts;

	for (i=0 ; i<lnumverts ; i++)
	{
		lindex = currentmodel->surfedges[fa->firstedge + i];

		if (lindex > 0)
		{
			r_pedge = &pedges[lindex];
			vec = r_pcurrentvertbase[r_pedge->v[0]].position;
		}
		else
		{
			r_pedge = &pedges[-lindex];
			vec = r_pcurrentvertbase[r_pedge->v[1]].position;
		}
		s = DotProduct (vec, fa->texinfo->vecs[0]) + fa->texinfo->vecs[0][3];
		s /= fa->texinfo->texture->width;

		t = DotProduct (vec, fa->texinfo->vecs[1]) + fa->texinfo->vecs[1][3];
		t /= fa->texinfo->texture->height;

		VectorCopy (vec, poly->verts[i]);
		poly->verts[i][3] = s;
		poly->verts[i][4] = t;

		//
		// lightmap texture coordinates
		//
		s = DotProduct (vec, fa->texinfo->vecs[0]) + fa->texinfo->vecs[0][3];
		s -= fa->texturemins[0];
		s += fa->light_s*16;
		s += 8;
		s /= BLOCK_WIDTH*16; //fa->texinfo->texture->width;

		t = DotProduct (vec, fa->texinfo->vecs[1]) + fa->texinfo->vecs[1][3];
		t -= fa->texturemins[1];
		t += fa->light_t*16;
		t += 8;
		t /= BLOCK_HEIGHT*16; //fa->texinfo->texture->height;

		poly->verts[i][5] = s;
		poly->verts[i][6] = t;
	}

	//
	// remove co-linear points - Ed
	//
	if (!gx_keeptjunctions.value && !(fa->flags & SURF_UNDERWATER) )
	{
		for (i = 0 ; i < lnumverts ; ++i)
		{
			vec3_t v1, v2;
			float *prev, *this, *next;
			float f;

			prev = poly->verts[(i + lnumverts - 1) % lnumverts];
			this = poly->verts[i];
			next = poly->verts[(i + 1) % lnumverts];

			VectorSubtract( this, prev, v1 );
			VectorNormalize( v1 );
			VectorSubtract( next, prev, v2 );
			VectorNormalize( v2 );

			// skip co-linear points
			#define COLINEAR_EPSILON 0.001
			if ((fabs( v1[0] - v2[0] ) <= COLINEAR_EPSILON) &&
				(fabs( v1[1] - v2[1] ) <= COLINEAR_EPSILON) && 
				(fabs( v1[2] - v2[2] ) <= COLINEAR_EPSILON))
			{
				int j;
				for (j = i + 1; j < lnumverts; ++j)
				{
					int k;
					for (k = 0; k < VERTEXSIZE; ++k)
						poly->verts[j - 1][k] = poly->verts[j][k];
				}
				--lnumverts;
				++nColinElim;
				// retry next vertex next time, which is now current vertex
				--i;
			}
		}
	}
	poly->numverts = lnumverts;

}

/*
========================
GX_CreateSurfaceLightmap
========================
*/
void GX_CreateSurfaceLightmap (msurface_t *surf)
{
	int		smax, tmax, s, t, l, i;
	byte	*base;

	if (surf->flags & (SURF_DRAWSKY|SURF_DRAWTURB))
		return;

	smax = (surf->extents[0]>>4)+1;
	tmax = (surf->extents[1]>>4)+1;

	surf->lightmaptexturenum = AllocBlock (smax, tmax, &surf->light_s, &surf->light_t);
	base = lightmaps + surf->lightmaptexturenum*lightmap_bytes*BLOCK_WIDTH*BLOCK_HEIGHT;
	base += (surf->light_t * BLOCK_WIDTH + surf->light_s) * lightmap_bytes;
	R_BuildLightMap (surf, base, BLOCK_WIDTH*lightmap_bytes);
}


/*
==================
GX_BuildLightmaps

Builds the lightmap texture
with all the surfaces from all brush models
==================
*/
void GX_BuildLightmaps (void)
{
	int		i, j;
	model_t	*m;
	extern qboolean isPermedia;

	memset (allocated, 0, sizeof(allocated));

	r_framecount = 1;		// no dlightcache

	if (!lightmap_textures)
	{
		lightmap_textures = texture_extension_number;
		texture_extension_number += MAX_LIGHTMAPS;
	}

	gx_lightmap_format = GX_TF_RGBA8; // Default changed for GX hardware
	// default differently on the Permedia
	if (isPermedia)
		gx_lightmap_format = GX_TF_RGBA8;

	if (COM_CheckParm ("-lm_1"))
		gx_lightmap_format = GL_LUMINANCE;
	if (COM_CheckParm ("-lm_a"))
		gx_lightmap_format = GL_ALPHA;
	if (COM_CheckParm ("-lm_i"))
		gx_lightmap_format = GL_INTENSITY;
	if (COM_CheckParm ("-lm_2"))
		gx_lightmap_format = GL_RGBA4;
	if (COM_CheckParm ("-lm_4"))
		gx_lightmap_format = GX_TF_RGBA8;

	switch (gx_lightmap_format)
	{
	case GX_TF_RGBA8:
		lightmap_bytes = 4;
		break;
	case GL_RGBA4:
		lightmap_bytes = 2;
		break;
	case GL_LUMINANCE:
	case GL_INTENSITY:
	case GL_ALPHA:
		lightmap_bytes = 1;
		break;
	}

	for (j=1 ; j<MAX_MODELS ; j++)
	{
		m = cl.model_precache[j];
		if (!m)
			break;
		if (m->name[0] == '*')
			continue;
		r_pcurrentvertbase = m->vertexes;
		currentmodel = m;
		for (i=0 ; i<m->numsurfaces ; i++)
		{
			GX_CreateSurfaceLightmap (m->surfaces + i);
			if ( m->surfaces[i].flags & SURF_DRAWTURB )
				continue;
#ifndef QUAKE2
			if ( m->surfaces[i].flags & SURF_DRAWSKY )
				continue;
#endif
			BuildSurfaceDisplayList (m->surfaces + i);
		}
	}

 	if (!gx_texsort.value)
 		GX_SelectTexture(TEXTURE1_SGIS);

	//
	// upload all lightmaps that were filled
	//
	for (i=0 ; i<MAX_LIGHTMAPS ; i++)
	{
		if (!allocated[i][0])
			break;		// no more used
		lightmap_modified[i] = false;
		lightmap_rectchange[i].l = BLOCK_WIDTH;
		lightmap_rectchange[i].t = BLOCK_HEIGHT;
		lightmap_rectchange[i].w = 0;
		lightmap_rectchange[i].h = 0;
		GX_Bind(lightmap_textures + i);
		GX_LoadAndBind (lightmaps+i*BLOCK_WIDTH*BLOCK_HEIGHT*lightmap_bytes, BLOCK_WIDTH*BLOCK_HEIGHT*lightmap_bytes, BLOCK_WIDTH, BLOCK_HEIGHT, gx_lightmap_format, 0);
		GX_SetMinMag (GX_LINEAR, GX_LINEAR);
	}

 	if (!gx_texsort.value)
 		GX_SelectTexture(TEXTURE0_SGIS);

}

#endif
