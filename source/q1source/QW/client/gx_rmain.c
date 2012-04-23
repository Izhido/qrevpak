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
// r_main.c

#ifdef GXQUAKE

#include <gccore.h>

#include "quakedef.h"

#include "gxutils.h"

extern int gx_tex_allocated;

entity_t	r_worldentity;

qboolean	r_cache_thrash;		// compatability

vec3_t		modelorg, r_entorigin;
entity_t	*currententity;

int			r_visframecount;	// bumped when going to a new PVS
int			r_framecount;		// used for dlight push checking

mplane_t	frustum[4];

int			c_brush_polys, c_alias_polys;

qboolean	envmap;				// true during envmap command capture 

int			currenttexture = -1;		// to avoid unnecessary texture sets

int			cnttextures[2] = {-1, -1};     // cached

int			particletexture;	// little dot for particles
int			playertextures;		// up to 16 color translated skins

int			mirrortexturenum;	// quake texturenum, not gxtexturenum
qboolean	mirror;
mplane_t	*mirror_plane;

//
// view origin
//
vec3_t	vup;
vec3_t	vpn;
vec3_t	vright;
vec3_t	r_origin;

Mtx		r_world_matrix;
Mtx		r_base_world_matrix;

//
// screen size info
//
refdef_t	r_refdef;

mleaf_t		*r_viewleaf, *r_oldviewleaf;

texture_t	*r_notexture_mip;

int		d_lightstylevalue[256];	// 8.8 fraction of base light value


void R_MarkLeaves (void);

cvar_t	r_norefresh = {"r_norefresh","0"};
cvar_t	r_drawentities = {"r_drawentities","1"};
cvar_t	r_drawviewmodel = {"r_drawviewmodel","1"};
cvar_t	r_speeds = {"r_speeds","0"};
cvar_t	r_fullbright = {"r_fullbright","0"};
cvar_t	r_lightmap = {"r_lightmap","0"};
cvar_t	r_shadows = {"r_shadows","0"};
cvar_t	r_mirroralpha = {"r_mirroralpha","1"};
cvar_t	r_wateralpha = {"r_wateralpha","1"};
cvar_t	r_dynamic = {"r_dynamic","1"};
cvar_t	r_novis = {"r_novis","0"};
cvar_t	r_netgraph = {"r_netgraph","0"};

cvar_t	gx_clear = {"gx_clear","0"};
cvar_t	gx_cull = {"gx_cull","1"};
cvar_t	gx_texsort = {"gx_texsort","1"};
cvar_t	gx_smoothmodels = {"gx_smoothmodels","1"};
cvar_t	gx_affinemodels = {"gx_affinemodels","0"};
cvar_t	gx_polyblend = {"gx_polyblend","1"};
cvar_t	gx_flashblend = {"gx_flashblend","1"};
cvar_t	gx_playermip = {"gx_playermip","0"};
cvar_t	gx_nocolors = {"gx_nocolors","0"};
cvar_t	gx_keeptjunctions = {"gx_keeptjunctions","1"};
cvar_t	gx_reporttjunctions = {"gx_reporttjunctions","0"};
cvar_t	gx_finish = {"gl_finish","0"};


extern	cvar_t	gx_ztrick;
extern	cvar_t	scr_fov;
/*
=================
R_CullBox

Returns true if the box is completely outside the frustom
=================
*/
qboolean R_CullBox (vec3_t mins, vec3_t maxs)
{
	int		i;

	for (i=0 ; i<4 ; i++)
		if (BoxOnPlaneSide (mins, maxs, &frustum[i]) == 2)
			return true;
	return false;
}


void R_RotateForEntity (entity_t *e)
{
	Mtx m;
	guVector a;

	guMtxTrans(m, e->origin[0],  e->origin[1],  e->origin[2]);
	guMtxConcat(gxu_modelview_matrices[gxu_cur_modelview_matrix], m, gxu_modelview_matrices[gxu_cur_modelview_matrix]);
	a.x = 0;
	a.y = 0;
	a.z = 1;
	guMtxRotAxisDeg(m, &a, e->angles[1]);
	guMtxConcat(gxu_modelview_matrices[gxu_cur_modelview_matrix], m, gxu_modelview_matrices[gxu_cur_modelview_matrix]);
	a.y = 1;
	a.z = 0;
	guMtxRotAxisDeg(m, &a, -e->angles[0]);
	guMtxConcat(gxu_modelview_matrices[gxu_cur_modelview_matrix], m, gxu_modelview_matrices[gxu_cur_modelview_matrix]);
	//ZOID: fixed z angle
	a.x = 1;
	a.y = 0;
	guMtxRotAxisDeg(m, &a, e->angles[2]);
	guMtxConcat(gxu_modelview_matrices[gxu_cur_modelview_matrix], m, gxu_modelview_matrices[gxu_cur_modelview_matrix]);
}

/*
=============================================================

  SPRITE MODELS

=============================================================
*/

/*
================
R_GetSpriteFrame
================
*/
mspriteframe_t *R_GetSpriteFrame (entity_t *currententity)
{
	msprite_t		*psprite;
	mspritegroup_t	*pspritegroup;
	mspriteframe_t	*pspriteframe;
	int				i, numframes, frame;
	float			*pintervals, fullinterval, targettime, time;

	psprite = currententity->model->cache.data;
	frame = currententity->frame;

	if ((frame >= psprite->numframes) || (frame < 0))
	{
		Con_Printf ("R_DrawSprite: no such frame %d\n", frame);
		frame = 0;
	}

	if (psprite->frames[frame].type == SPR_SINGLE)
	{
		pspriteframe = psprite->frames[frame].frameptr;
	}
	else
	{
		pspritegroup = (mspritegroup_t *)psprite->frames[frame].frameptr;
		pintervals = pspritegroup->intervals;
		numframes = pspritegroup->numframes;
		fullinterval = pintervals[numframes-1];

		time = cl.time + currententity->syncbase;

	// when loading in Mod_LoadSpriteGroup, we guaranteed all interval values
	// are positive, so we don't have to worry about division by 0
		targettime = time - ((int)(time / fullinterval)) * fullinterval;

		for (i=0 ; i<(numframes-1) ; i++)
		{
			if (pintervals[i] > targettime)
				break;
		}

		pspriteframe = pspritegroup->frames[i];
	}

	return pspriteframe;
}


/*
=================
R_DrawSpriteModel

=================
*/
void R_DrawSpriteModel (entity_t *e)
{
	vec3_t	point;
	mspriteframe_t	*frame;
	float		*up, *right;
	vec3_t		v_forward, v_right, v_up;
	msprite_t		*psprite;

	// don't even bother culling, because it's just a single
	// polygon without a surface cache
	frame = R_GetSpriteFrame (e);
	psprite = currententity->model->cache.data;

	if (psprite->type == SPR_ORIENTED)
	{	// bullet marks on walls
		AngleVectors (currententity->angles, v_forward, v_right, v_up);
		up = v_up;
		right = v_right;
	}
	else
	{	// normal sprite
		up = vup;
		right = vright;
	}

	gxu_cur_r = 255;
	gxu_cur_g = 255;
	gxu_cur_b = 255;
	gxu_cur_a = 255;

	GX_DisableMultitexture();

    GX_Bind(frame->gx_texturenum);

	GX_SetAlphaCompare(GX_GEQUAL, gxu_alpha_test_lower, GX_AOP_AND, GX_LEQUAL, gxu_alpha_test_higher);
	GX_Begin (GX_QUADS, gxu_cur_vertex_format, 4);

	VectorMA (e->origin, frame->down, up, point);
	VectorMA (point, frame->left, right, point);
	GX_Position3f32(*(point), *(point + 1), *(point + 2));
	GX_Color4u8(gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
	GX_TexCoord2f32 (0, 1);

	VectorMA (e->origin, frame->up, up, point);
	VectorMA (point, frame->left, right, point);
	GX_Position3f32(*(point), *(point + 1), *(point + 2));
	GX_Color4u8(gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
	GX_TexCoord2f32 (0, 0);

	VectorMA (e->origin, frame->up, up, point);
	VectorMA (point, frame->right, right, point);
	GX_Position3f32(*(point), *(point + 1), *(point + 2));
	GX_Color4u8(gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
	GX_TexCoord2f32 (1, 0);

	VectorMA (e->origin, frame->down, up, point);
	VectorMA (point, frame->right, right, point);
	GX_Position3f32(*(point), *(point + 1), *(point + 2));
	GX_Color4u8(gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
	GX_TexCoord2f32 (1, 1);
	
	GX_End ();

	GX_SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
}

/*
=============================================================

  ALIAS MODELS

=============================================================
*/


#define NUMVERTEXNORMALS	162

float	r_avertexnormals[NUMVERTEXNORMALS][3] = {
#include "anorms.h"
};

vec3_t	shadevector;
float	shadelight, ambientlight;

// precalculated dot products for quantized angles
#define SHADEDOT_QUANT 16
float	r_avertexnormal_dots[SHADEDOT_QUANT][256] =
#include "anorm_dots.h"
;

float	*shadedots = r_avertexnormal_dots[0];

int	lastposenum;

/*
=============
GX_DrawAliasFrame
=============
*/
void GX_DrawAliasFrame (aliashdr_t *paliashdr, int posenum)
{
	float 	l;
	trivertx_t	*verts;
	int		*order;
	int		count;

lastposenum = posenum;

	verts = (trivertx_t *)((byte *)paliashdr + paliashdr->posedata);
	verts += posenum * paliashdr->poseverts;
	order = (int *)((byte *)paliashdr + paliashdr->commands);

	gxu_cur_a = 255;

	while (1)
	{
		// get the vertex count and primitive type
		count = *order++;
		if (!count)
			break;		// done
		if (count < 0)
		{
			count = -count;
			GX_Begin(GX_TRIANGLEFAN, gxu_cur_vertex_format, count);
		}
		else
			GX_Begin(GX_TRIANGLESTRIP, gxu_cur_vertex_format, count);
		do
		{
			// normals and vertexes come from the frame list
			GX_Position3f32(verts->v[0], verts->v[1], verts->v[2]);
			verts++;
			l = shadedots[verts->lightnormalindex] * shadelight;
			if(l < 0.0)
				l = 0.0;
			if(l > 1.0)
				l = 1.0;
			gxu_cur_r = gxu_cur_g = gxu_cur_b = (l * 255.0);
			GX_Color4u8(gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
			// texture coordinates come from the draw list
			GX_TexCoord2f32 (((float *)order)[0], ((float *)order)[1]);
			order += 2;
		} while (--count);

		GX_End ();
	}
}


/*
=============
GX_DrawAliasShadow
=============
*/
extern	vec3_t			lightspot;

void GX_DrawAliasShadow (aliashdr_t *paliashdr, int posenum)
{
	trivertx_t	*verts;
	int		*order;
	vec3_t	point;
	float	height, lheight;
	int		count;

	lheight = currententity->origin[2] - lightspot[2];

	height = 0;
	verts = (trivertx_t *)((byte *)paliashdr + paliashdr->posedata);
	verts += posenum * paliashdr->poseverts;
	order = (int *)((byte *)paliashdr + paliashdr->commands);

	height = -lheight + 1.0;

	while (1)
	{
		// get the vertex count and primitive type
		count = *order++;
		if (!count)
			break;		// done
		if (count < 0)
		{
			count = -count;
			GX_Begin (GX_TRIANGLEFAN, GX_VTXFMT0, count);
		}
		else
			GX_Begin (GX_TRIANGLESTRIP, GX_VTXFMT0, count);

		do
		{
			// texture coordinates come from the draw list
			// (skipped for shadows) GX_TexCoord2f32 (*((float *)order), *(((float *)order) + 1));
			order += 2;

			// normals and vertexes come from the frame list
			point[0] = verts->v[0] * paliashdr->scale[0] + paliashdr->scale_origin[0];
			point[1] = verts->v[1] * paliashdr->scale[1] + paliashdr->scale_origin[1];
			point[2] = verts->v[2] * paliashdr->scale[2] + paliashdr->scale_origin[2];

			point[0] -= shadevector[0]*(point[2]+lheight);
			point[1] -= shadevector[1]*(point[2]+lheight);
			point[2] = height;
//			height -= 0.001;
			GX_Position3f32(point[0], point[1], point[2]);
			GX_Color4u8(gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);

			verts++;
		} while (--count);

		GX_End ();
	};

}



/*
=================
R_SetupAliasFrame

=================
*/
void R_SetupAliasFrame (int frame, aliashdr_t *paliashdr)
{
	int				pose, numposes;
	float			interval;

	if ((frame >= paliashdr->numframes) || (frame < 0))
	{
		Con_DPrintf ("R_AliasSetupFrame: no such frame %d\n", frame);
		frame = 0;
	}

	pose = paliashdr->frames[frame].firstpose;
	numposes = paliashdr->frames[frame].numposes;

	if (numposes > 1)
	{
		interval = paliashdr->frames[frame].interval;
		pose += (int)(cl.time / interval) % numposes;
	}

	GX_DrawAliasFrame (paliashdr, pose);
}



/*
=================
R_DrawAliasModel

=================
*/
void R_DrawAliasModel (entity_t *e)
{
	int			i;
	int			lnum;
	vec3_t		dist;
	float		add;
	model_t		*clmodel;
	vec3_t		mins, maxs;
	aliashdr_t	*paliashdr;
	float		an;
	int			anim;
	Mtx			m;

	clmodel = currententity->model;

	VectorAdd (currententity->origin, clmodel->mins, mins);
	VectorAdd (currententity->origin, clmodel->maxs, maxs);

	if (R_CullBox (mins, maxs))
		return;


	VectorCopy (currententity->origin, r_entorigin);
	VectorSubtract (r_origin, r_entorigin, modelorg);

	//
	// get lighting information
	//

	ambientlight = shadelight = R_LightPoint (currententity->origin);

	// allways give the gun some light
	if (e == &cl.viewent && ambientlight < 24)
		ambientlight = shadelight = 24;

	for (lnum=0 ; lnum<MAX_DLIGHTS ; lnum++)
	{
		if (cl_dlights[lnum].die >= cl.time)
		{
			VectorSubtract (currententity->origin,
							cl_dlights[lnum].origin,
							dist);
			add = cl_dlights[lnum].radius - Length(dist);

			if (add > 0) {
				ambientlight += add;
				//ZOID models should be affected by dlights as well
				shadelight += add;
			}
		}
	}

	// clamp lighting so it doesn't overbright as much
	if (ambientlight > 128)
		ambientlight = 128;
	if (ambientlight + shadelight > 192)
		shadelight = 192 - ambientlight;

	// ZOID: never allow players to go totally black
	if (!strcmp(clmodel->name, "progs/player.mdl")) {
		if (ambientlight < 8)
			ambientlight = shadelight = 8;

	} else if (!strcmp (clmodel->name, "progs/flame2.mdl")
		|| !strcmp (clmodel->name, "progs/flame.mdl") )
		// HACK HACK HACK -- no fullbright colors, so make torches full light
		ambientlight = shadelight = 256;

	shadedots = r_avertexnormal_dots[((int)(e->angles[1] * (SHADEDOT_QUANT / 360.0))) & (SHADEDOT_QUANT - 1)];
	shadelight = shadelight / 200.0;
	
	an = e->angles[1]/180*M_PI;
	shadevector[0] = cos(-an);
	shadevector[1] = sin(-an);
	shadevector[2] = 1;
	VectorNormalize (shadevector);

	//
	// locate the proper data
	//
	paliashdr = (aliashdr_t *)Mod_Extradata (currententity->model);

	c_alias_polys += paliashdr->numtris;

	//
	// draw all the triangles
	//

	GX_DisableMultitexture();

	guMtxCopy(gxu_modelview_matrices[gxu_cur_modelview_matrix], gxu_modelview_matrices[gxu_cur_modelview_matrix + 1]);
	gxu_cur_modelview_matrix++;

	R_RotateForEntity (e);

	if (!strcmp (clmodel->name, "progs/eyes.mdl") ) {
		guMtxTrans(m, paliashdr->scale_origin[0], paliashdr->scale_origin[1], paliashdr->scale_origin[2] - (22 + 8));
		guMtxConcat(gxu_modelview_matrices[gxu_cur_modelview_matrix], m, gxu_modelview_matrices[gxu_cur_modelview_matrix]);

// double size of eyes, since they are really hard to see while hardware rendering
		guMtxScale(m, paliashdr->scale[0]*2, paliashdr->scale[1]*2, paliashdr->scale[2]*2);
		guMtxConcat(gxu_modelview_matrices[gxu_cur_modelview_matrix], m, gxu_modelview_matrices[gxu_cur_modelview_matrix]);
	} else {
		guMtxTrans(m, paliashdr->scale_origin[0], paliashdr->scale_origin[1], paliashdr->scale_origin[2]);
		guMtxConcat(gxu_modelview_matrices[gxu_cur_modelview_matrix], m, gxu_modelview_matrices[gxu_cur_modelview_matrix]);
		guMtxScale(m, paliashdr->scale[0], paliashdr->scale[1], paliashdr->scale[2]);
		guMtxConcat(gxu_modelview_matrices[gxu_cur_modelview_matrix], m, gxu_modelview_matrices[gxu_cur_modelview_matrix]);
	}

	GX_LoadPosMtxImm(gxu_modelview_matrices[gxu_cur_modelview_matrix], GX_PNMTX0);

	anim = (int)(cl.time*10) & 3;
    GX_Bind(paliashdr->gx_texturenum[currententity->skinnum][anim]);

	// we can't dynamically colormap textures, so they are cached
	// seperately for the players.  Heads are just uncolored.
	if (currententity->scoreboard && !gx_nocolors.value)
	{
		i = currententity->scoreboard - cl.players;
		if (!currententity->scoreboard->skin) {
			Skin_Find(currententity->scoreboard);
			R_TranslatePlayerSkin(i);
		}
		if (i >= 0 && i<MAX_CLIENTS)
		    GX_Bind(playertextures + i);
	}

	if (gx_smoothmodels.value)
		glShadeModel (GL_SMOOTH);
	GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);

	if (gx_affinemodels.value)
		glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);

	R_SetupAliasFrame (currententity->frame, paliashdr);

	GX_SetTevOp(GX_TEVSTAGE0, GX_REPLACE);

	glShadeModel (GL_FLAT);
	if (gx_affinemodels.value)
		glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	gxu_cur_modelview_matrix--;
	GX_LoadPosMtxImm(gxu_modelview_matrices[gxu_cur_modelview_matrix], GX_PNMTX0);

	if (r_shadows.value)
	{
		guMtxCopy(gxu_modelview_matrices[gxu_cur_modelview_matrix], gxu_modelview_matrices[gxu_cur_modelview_matrix + 1]);
		gxu_cur_modelview_matrix++;
		R_RotateForEntity (e);
		GX_LoadPosMtxImm(gxu_modelview_matrices[gxu_cur_modelview_matrix], GX_PNMTX0);
		gxu_cur_vertex_format = GX_VTXFMT0;
 		GX_SetVtxDesc(GX_VA_TEX0, GX_NONE);
		GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLOR0A0);
		GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
		GX_SetBlendMode(GX_BM_BLEND, gxu_blend_src_value, gxu_blend_dst_value, GX_LO_NOOP); 
		gxu_cur_r = 0;
		gxu_cur_g = 0;
		gxu_cur_b = 0;
		gxu_cur_a = 127;
		GX_DrawAliasShadow (paliashdr, lastposenum);
		gxu_cur_vertex_format = GX_VTXFMT1;
 		GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);
 		GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
		GX_SetTevOp(GX_TEVSTAGE0, GX_REPLACE);
		GX_SetBlendMode(GX_BM_NONE, gxu_blend_src_value, gxu_blend_dst_value, GX_LO_NOOP); 
		gxu_cur_r = 255;
		gxu_cur_g = 255;
		gxu_cur_b = 255;
		gxu_cur_a = 255;
		gxu_cur_modelview_matrix--;
		GX_LoadPosMtxImm(gxu_modelview_matrices[gxu_cur_modelview_matrix], GX_PNMTX0);
	}

}

//==================================================================================

/*
=============
R_DrawEntitiesOnList
=============
*/
void R_DrawEntitiesOnList (void)
{
	int		i;

	if (!r_drawentities.value)
		return;

	// draw sprites seperately, because of alpha blending
	for (i=0 ; i<cl_numvisedicts ; i++)
	{
		currententity = &cl_visedicts[i];

		switch (currententity->model->type)
		{
		case mod_alias:
			R_DrawAliasModel (currententity);
			break;

		case mod_brush:
			R_DrawBrushModel (currententity);
			break;

		default:
			break;
		}
	}

	for (i=0 ; i<cl_numvisedicts ; i++)
	{
		currententity = &cl_visedicts[i];

		switch (currententity->model->type)
		{
		case mod_sprite:
			R_DrawSpriteModel (currententity);
			break;

		default :
			break;
		}
	}
}

/*
=============
R_DrawViewModel
=============
*/
void R_DrawViewModel (void)
{
	float		ambient[4], diffuse[4];
	int			j;
	int			lnum;
	vec3_t		dist;
	float		add;
	dlight_t	*dl;
	int			ambientlight, shadelight;

	if (!r_drawviewmodel.value || !Cam_DrawViewModel())
		return;

	if (envmap)
		return;

	if (!r_drawentities.value)
		return;

	if (cl.stats[STAT_ITEMS] & IT_INVISIBILITY)
		return;

	if (cl.stats[STAT_HEALTH] <= 0)
		return;

	currententity = &cl.viewent;
	if (!currententity->model)
		return;

	j = R_LightPoint (currententity->origin);

	if (j < 24)
		j = 24;		// allways give some light on gun
	ambientlight = j;
	shadelight = j;

// add dynamic lights		
	for (lnum=0 ; lnum<MAX_DLIGHTS ; lnum++)
	{
		dl = &cl_dlights[lnum];
		if (!dl->radius)
			continue;
		if (!dl->radius)
			continue;
		if (dl->die < cl.time)
			continue;

		VectorSubtract (currententity->origin, dl->origin, dist);
		add = dl->radius - Length(dist);
		if (add > 0)
			ambientlight += add;
	}

	ambient[0] = ambient[1] = ambient[2] = ambient[3] = (float)ambientlight / 128;
	diffuse[0] = diffuse[1] = diffuse[2] = diffuse[3] = (float)shadelight / 128;

	// hack the depth range to prevent view model from poking into walls
	GX_SetViewport (gxu_viewport_x, gxu_viewport_y, gxu_viewport_width, gxu_viewport_height, gxdepthmin, gxdepthmin + 0.3*(gxdepthmax-gxdepthmin));
	R_DrawAliasModel (currententity);
	GX_SetViewport (gxu_viewport_x, gxu_viewport_y, gxu_viewport_width, gxu_viewport_height, gxdepthmin, gxdepthmax);
}


/*
============
R_PolyBlend
============
*/
void R_PolyBlend (void)
{
	guVector v;
	Mtx m;
	u8 r, g, b, a;

	if (!gx_polyblend.value)
		return;
	if (!v_blend[3])
		return;

//Con_Printf("R_PolyBlend(): %4.2f %4.2f %4.2f %4.2f\n",v_blend[0], v_blend[1],	v_blend[2],	v_blend[3]);

 	GX_DisableMultitexture();

	GX_SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
	GX_SetBlendMode(GX_BM_BLEND, gxu_blend_src_value, gxu_blend_dst_value, GX_LO_NOOP); 
	gxu_z_test_enabled = GX_FALSE;
	GX_SetZMode(gxu_z_test_enabled, GX_LEQUAL, gxu_z_write_enabled);
 	GX_SetVtxDesc(GX_VA_TEX0, GX_NONE);
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLOR0A0);
	GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);

	v.x = 1;
	v.y = 0;
	v.z = 0;
	guMtxRotAxisDeg(gxu_modelview_matrices[gxu_cur_modelview_matrix], &v, -90); // put Z going up
	v.x = 0;
	v.z = 1;
	guMtxRotAxisDeg(m, &v, 90); 
	guMtxConcat(gxu_modelview_matrices[gxu_cur_modelview_matrix], m, gxu_modelview_matrices[gxu_cur_modelview_matrix]); // put Z going up

	GX_LoadPosMtxImm(gxu_modelview_matrices[gxu_cur_modelview_matrix], GX_PNMTX0);

	r = v_blend[0] * 255.0;
	g = v_blend[1] * 255.0;
	b = v_blend[2] * 255.0;
	a = v_blend[3] * 255.0;

	GX_Begin (GX_QUADS, GX_VTXFMT0, 4);
	GX_Position3f32(10, 100, 100);
	GX_Color4u8(r, g, b, a);
	GX_Position3f32(10, -100, 100);
	GX_Color4u8(r, g, b, a);
	GX_Position3f32(10, -100, -100);
	GX_Color4u8(r, g, b, a);
	GX_Position3f32(10, 100, -100);
	GX_Color4u8(r, g, b, a);
	GX_End ();

	GX_SetBlendMode(GX_BM_NONE, gxu_blend_src_value, gxu_blend_dst_value, GX_LO_NOOP); 
 	GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);
 	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
	GX_SetTevOp(GX_TEVSTAGE0, GX_REPLACE);
	GX_SetAlphaCompare(GX_GEQUAL, gxu_alpha_test_lower, GX_AOP_AND, GX_LEQUAL, gxu_alpha_test_higher);
}


int SignbitsForPlane (mplane_t *out)
{
	int	bits, j;

	// for fast box on planeside test

	bits = 0;
	for (j=0 ; j<3 ; j++)
	{
		if (out->normal[j] < 0)
			bits |= 1<<j;
	}
	return bits;
}


void R_SetFrustum (void)
{
	int		i;

	if (r_refdef.fov_x == 90) 
	{
		// front side is visible

		VectorAdd (vpn, vright, frustum[0].normal);
		VectorSubtract (vpn, vright, frustum[1].normal);

		VectorAdd (vpn, vup, frustum[2].normal);
		VectorSubtract (vpn, vup, frustum[3].normal);
	}
	else
	{
		// rotate VPN right by FOV_X/2 degrees
		RotatePointAroundVector( frustum[0].normal, vup, vpn, -(90-r_refdef.fov_x / 2 ) );
		// rotate VPN left by FOV_X/2 degrees
		RotatePointAroundVector( frustum[1].normal, vup, vpn, 90-r_refdef.fov_x / 2 );
		// rotate VPN up by FOV_X/2 degrees
		RotatePointAroundVector( frustum[2].normal, vright, vpn, 90-r_refdef.fov_y / 2 );
		// rotate VPN down by FOV_X/2 degrees
		RotatePointAroundVector( frustum[3].normal, vright, vpn, -( 90 - r_refdef.fov_y / 2 ) );
	}

	for (i=0 ; i<4 ; i++)
	{
		frustum[i].type = PLANE_ANYZ;
		frustum[i].dist = DotProduct (r_origin, frustum[i].normal);
		frustum[i].signbits = SignbitsForPlane (&frustum[i]);
	}
}



/*
===============
R_SetupFrame
===============
*/
void R_SetupFrame (void)
{
// don't allow cheats in multiplayer
	r_fullbright.value = 0;
	r_lightmap.value = 0;
	if (!atoi(Info_ValueForKey(cl.serverinfo, "watervis")))
		r_wateralpha.value = 1;

	R_AnimateLight ();

	r_framecount++;

// build the transformation matrix for the given view angles
	VectorCopy (r_refdef.vieworg, r_origin);

	AngleVectors (r_refdef.viewangles, vpn, vright, vup);

// current viewleaf
	r_oldviewleaf = r_viewleaf;
	r_viewleaf = Mod_PointInLeaf (r_origin, cl.worldmodel);

	V_SetContentsColor (r_viewleaf->contents);
	V_CalcBlend ();

	r_cache_thrash = false;

	c_brush_polys = 0;
	c_alias_polys = 0;

}


/*
=============
R_SetupGX
=============
*/
void R_SetupGX (void)
{
	//float	screenaspect;
	extern	int gxwidth, gxheight;
	int		x, x2, y2, y, w, h;
	Mtx44		pm;
	Mtx			mm;
	guVector	a;

	//
	// set up viewpoint
	//
	x = r_refdef.vrect.x * gxwidth/vid.width;
	x2 = (r_refdef.vrect.x + r_refdef.vrect.width) * gxwidth/vid.width;
	y = (vid.height-r_refdef.vrect.y) * gxheight/vid.height;
	y2 = (vid.height - (r_refdef.vrect.y + r_refdef.vrect.height)) * gxheight/vid.height;

	// fudge around because of frac screen scale
	if (x > 0)
		x--;
	if (x2 < gxwidth)
		x2++;
	if (y2 < 0)
		y2--;
	if (y < gxheight)
		y++;

	w = x2 - x;
	h = y - y2;

	if (envmap)
	{
		x = y2 = 0;
		w = h = 256;
	}

	gxu_viewport_x = gxx + x;
	gxu_viewport_y = gxy/* + y2*/;
	gxu_viewport_width = w;
	gxu_viewport_height = h;
	GX_SetViewport (gxu_viewport_x, gxu_viewport_y, gxu_viewport_width, gxu_viewport_height, gxdepthmin, gxdepthmax);
	
	guPerspective(gxu_projection_matrix, r_refdef.fov_y, (float)r_refdef.vrect.width/r_refdef.vrect.height, 4, 4096);

	if (mirror)
	{
		if (mirror_plane->normal[2])
			guMtxScale(pm, 1, -1, 1);
		else
			guMtxScale(pm, -1, 1, 1);

		guMtxConcat(gxu_projection_matrix, pm, gxu_projection_matrix);
		gxu_cull_mode = GX_CULL_FRONT;
	}
	else
		gxu_cull_mode = GX_CULL_BACK;

	if(gxu_cull_enabled)
	{
		GX_SetCullMode(gxu_cull_mode);
	};

	GX_LoadProjectionMtx(gxu_projection_matrix, GX_PERSPECTIVE);

	a.x = 1;
	a.y = 0;
	a.z = 0;
	guMtxRotAxisDeg(gxu_modelview_matrices[gxu_cur_modelview_matrix], &a, -90);
	a.x = 0;
	a.z = 1;
	guMtxRotAxisDeg(mm, &a, 90);
	guMtxConcat(gxu_modelview_matrices[gxu_cur_modelview_matrix], mm, gxu_modelview_matrices[gxu_cur_modelview_matrix]); // put Z going up
	a.x = 1;
	a.z = 0;
	guMtxRotAxisDeg(mm, &a, -r_refdef.viewangles[2]);
	guMtxConcat(gxu_modelview_matrices[gxu_cur_modelview_matrix], mm, gxu_modelview_matrices[gxu_cur_modelview_matrix]);
	a.x = 0;
	a.y = 1;
	guMtxRotAxisDeg(mm, &a, -r_refdef.viewangles[0]);
	guMtxConcat(gxu_modelview_matrices[gxu_cur_modelview_matrix], mm, gxu_modelview_matrices[gxu_cur_modelview_matrix]);
	a.y = 0;
	a.z = 1;
	guMtxRotAxisDeg(mm, &a, -r_refdef.viewangles[1]);
	guMtxConcat(gxu_modelview_matrices[gxu_cur_modelview_matrix], mm, gxu_modelview_matrices[gxu_cur_modelview_matrix]);
	guMtxTrans(mm, -r_refdef.vieworg[0], -r_refdef.vieworg[1], -r_refdef.vieworg[2]);
	guMtxConcat(gxu_modelview_matrices[gxu_cur_modelview_matrix], mm, gxu_modelview_matrices[gxu_cur_modelview_matrix]);

	GX_LoadPosMtxImm(gxu_modelview_matrices[gxu_cur_modelview_matrix], GX_PNMTX0);

	guMtxCopy(gxu_modelview_matrices[gxu_cur_modelview_matrix], r_world_matrix);

	//
	// set drawing parms
	//
	if (gx_cull.value)
	{
		gxu_cull_enabled = true;
		GX_SetCullMode(gxu_cull_mode);
	} else
	{
		gxu_cull_enabled = false;
		GX_SetCullMode(GX_CULL_NONE);
	};

	gxu_blend_enabled = false;
	GX_SetBlendMode(GX_BM_NONE, gxu_blend_src_value, gxu_blend_dst_value, GX_LO_NOOP); 
	gxu_alpha_test_enabled = false;
	GX_SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
	gxu_z_test_enabled = GX_TRUE;
	GX_SetZMode(gxu_z_test_enabled, GX_LEQUAL, gxu_z_write_enabled);
}

/*
================
R_RenderScene

r_refdef must be set before the first call
================
*/
void R_RenderScene (void)
{
	R_SetupFrame ();

	R_SetFrustum ();

	R_SetupGX ();

	R_MarkLeaves ();	// done here so we know if we're in water

	R_DrawWorld ();		// adds static entities to the list

	S_ExtraUpdate ();	// don't let sound get messed up if going slow

	R_DrawEntitiesOnList ();

	GX_DisableMultitexture();

	R_RenderDlights ();

	R_DrawParticles ();

#ifdef GXTEST
	Test_Draw ();
#endif

}


/*
=============
R_Clear
=============
*/
void R_Clear (void)
{
	Cvar_SetValue("gx_ztrick", 0.0); // ONCE WE FULLY UNDERSTAND HOW zNear AND zFar WORKS ON GX, REMOVE THIS ASAP
	Sbar_Changed();                  // THIS TOO, REMOVE IT ASAP
	if (r_mirroralpha.value != 1.0)
	{
		if (gx_clear.value)
		{
			gxu_clear_color_buffer = GX_TRUE;
			Sbar_Changed();
		} else
			gxu_clear_color_buffer = GX_FALSE;
		gxu_clear_buffers = GX_TRUE;
		gxdepthmin = 0;
		gxdepthmax = 0.5;
		GX_SetZMode(gxu_z_test_enabled, GX_LEQUAL, gxu_z_write_enabled);
	}
	else if (gx_ztrick.value)
	{
		static int trickframe;

		gxu_clear_buffers = GX_FALSE;

		trickframe++;
		if (trickframe & 1)
		{
			gxdepthmin = 0;
			gxdepthmax = 0.49999;
			GX_SetZMode(gxu_z_test_enabled, GX_LEQUAL, gxu_z_write_enabled);
		}
		else
		{
			gxdepthmin = 1;
			gxdepthmax = 0.5;
			GX_SetZMode(gxu_z_test_enabled, GX_GEQUAL, gxu_z_write_enabled);
		}
	}
	else
	{
		if (gx_clear.value)
		{
			gxu_clear_color_buffer = GX_TRUE;
			Sbar_Changed();
		} else
			gxu_clear_color_buffer = GX_FALSE;
		gxu_clear_buffers = GX_TRUE;
		gxdepthmin = 0;
		gxdepthmax = 1;
		GX_SetZMode(gxu_z_test_enabled, GX_LEQUAL, gxu_z_write_enabled);
	}

	GX_SetViewport (gxu_viewport_x, gxu_viewport_y, gxu_viewport_width, gxu_viewport_height, gxdepthmin, gxdepthmax);
}

#if 0 //!!! FIXME, Zoid, mirror is disabled for now
/*
=============
R_Mirror
=============
*/
void R_Mirror (void)
{
	float		d;
	msurface_t	*s;
	entity_t	*ent;

	if (!mirror)
		return;

	memcpy (r_base_world_matrix, r_world_matrix, sizeof(r_base_world_matrix));

	d = DotProduct (r_refdef.vieworg, mirror_plane->normal) - mirror_plane->dist;
	VectorMA (r_refdef.vieworg, -2*d, mirror_plane->normal, r_refdef.vieworg);

	d = DotProduct (vpn, mirror_plane->normal);
	VectorMA (vpn, -2*d, mirror_plane->normal, vpn);

	r_refdef.viewangles[0] = -asin (vpn[2])/M_PI*180;
	r_refdef.viewangles[1] = atan2 (vpn[1], vpn[0])/M_PI*180;
	r_refdef.viewangles[2] = -r_refdef.viewangles[2];

	ent = &cl_entities[cl.viewentity];
	if (cl_numvisedicts < MAX_VISEDICTS)
	{
		cl_visedicts[cl_numvisedicts] = ent;
		cl_numvisedicts++;
	}

	gldepthmin = 0.5;
	gldepthmax = 1;
	glDepthRange (gldepthmin, gldepthmax);
	glDepthFunc (GL_LEQUAL);

	R_RenderScene ();
	R_DrawWaterSurfaces ();


	gldepthmin = 0;
	gldepthmax = 0.5;
	glDepthRange (gldepthmin, gldepthmax);
	glDepthFunc (GL_LEQUAL);

	// blend on top
	glEnable (GL_BLEND);
	glMatrixMode(GL_PROJECTION);
	if (mirror_plane->normal[2])
		glScalef (1,-1,1);
	else
		glScalef (-1,1,1);
	glCullFace(GL_FRONT);
	glMatrixMode(GL_MODELVIEW);

	glLoadMatrixf (r_base_world_matrix);

	glColor4f (1,1,1,r_mirroralpha.value);
	s = cl.worldmodel->textures[mirrortexturenum]->texturechain;
	for ( ; s ; s=s->texturechain)
		R_RenderBrushPoly (s);
	cl.worldmodel->textures[mirrortexturenum]->texturechain = NULL;
	glDisable (GL_BLEND);
	glColor4f (1,1,1,1);
}
#endif

/*
================
R_RenderView

r_refdef must be set before the first call
================
*/
void R_RenderView (void)
{
	double	time1 = 0, time2;

	if (r_norefresh.value)
		return;

	if (!r_worldentity.model || !cl.worldmodel)
		Sys_Error ("R_RenderView: NULL worldmodel");

	if (r_speeds.value)
	{
		time1 = Sys_DoubleTime ();
		c_brush_polys = 0;
		c_alias_polys = 0;
	}

	mirror = false;

	R_Clear ();

	// render normal view
	R_RenderScene ();
	R_DrawViewModel ();
	R_DrawWaterSurfaces ();

	// render mirror view
//	R_Mirror ();

	R_PolyBlend ();

	if (r_speeds.value)
	{
		time2 = Sys_DoubleTime ();
		Con_Printf ("%3i ms  %4i wpoly %4i epoly %9i bytes\n", (int)((time2-time1)*1000), c_brush_polys, c_alias_polys, gx_tex_allocated); 
	}
}

#endif
