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
// gl_mesh.c: triangle model functions

#include "gx_local.h"

#include <malloc.h>

/*
=============================================================

  ALIAS MODELS

=============================================================
*/

#define NUMVERTEXNORMALS	162

float	r_avertexnormals[NUMVERTEXNORMALS][3] = {
#include "anorms.h"
};

typedef float vec4_t[4];

static	vec4_t*	s_lerped;
//static	vec3_t	lerped[MAX_VERTS];
static u8* colorArray;

vec3_t	shadevector;
float	shadelight[3];

// precalculated dot products for quantized angles
#define SHADEDOT_QUANT 16
float	r_avertexnormal_dots[SHADEDOT_QUANT][256] =
#include "anormtab.h"
;

float	*shadedots = r_avertexnormal_dots[0];

void GX_LerpVerts( int nverts, dtrivertx_t *v, dtrivertx_t *ov, dtrivertx_t *verts, float *lerp, float move[3], float frontv[3], float backv[3] )
{
	int i;

	//PMM -- added RF_SHELL_DOUBLE, RF_SHELL_HALF_DAM
	if ( currententity->flags & ( RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE | RF_SHELL_DOUBLE | RF_SHELL_HALF_DAM) )
	{
		for (i=0 ; i < nverts; i++, v++, ov++, lerp+=4 )
		{
			float *normal = r_avertexnormals[verts[i].lightnormalindex];

			lerp[0] = move[0] + ov->v[0]*backv[0] + v->v[0]*frontv[0] + normal[0] * POWERSUIT_SCALE;
			lerp[1] = move[1] + ov->v[1]*backv[1] + v->v[1]*frontv[1] + normal[1] * POWERSUIT_SCALE;
			lerp[2] = move[2] + ov->v[2]*backv[2] + v->v[2]*frontv[2] + normal[2] * POWERSUIT_SCALE; 
		}
	}
	else
	{
		for (i=0 ; i < nverts; i++, v++, ov++, lerp+=4)
		{
			lerp[0] = move[0] + ov->v[0]*backv[0] + v->v[0]*frontv[0];
			lerp[1] = move[1] + ov->v[1]*backv[1] + v->v[1]*frontv[1];
			lerp[2] = move[2] + ov->v[2]*backv[2] + v->v[2]*frontv[2];
		}
	}

}

/*
=============
GX_DrawAliasFrameLerp

interpolates between two frames and origins
FIXME: batch lerp all vertexes
=============
*/
void GX_DrawAliasFrameLerp (dmdl_t *paliashdr, float backlerp)
{
	float 	l;
	daliasframe_t	*frame, *oldframe;
	dtrivertx_t	*v, *ov, *verts;
	int		*order;
	int		count;
	float	frontlerp;
	float	alpha;
	vec3_t	move, delta, vectors[3];
	vec3_t	frontv, backv;
	int		i;
	u16		index_xyz;
	float	*lerp;
	float	ccomp;

	frame = (daliasframe_t *)((byte *)paliashdr + paliashdr->ofs_frames 
		+ currententity->frame * paliashdr->framesize);
	verts = v = frame->verts;

	oldframe = (daliasframe_t *)((byte *)paliashdr + paliashdr->ofs_frames 
		+ currententity->oldframe * paliashdr->framesize);
	ov = oldframe->verts;

	order = (int *)((byte *)paliashdr + paliashdr->ofs_glcmds);

//	glTranslatef (frame->translate[0], frame->translate[1], frame->translate[2]);
//	glScalef (frame->scale[0], frame->scale[1], frame->scale[2]);

	if (currententity->flags & RF_TRANSLUCENT)
		alpha = currententity->alpha;
	else
		alpha = 1.0;

	// PMM - added double shell
	if ( currententity->flags & ( RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE | RF_SHELL_DOUBLE | RF_SHELL_HALF_DAM) )
		qgxDisableTexture();

	frontlerp = 1.0 - backlerp;

	// move should be the delta back to the previous frame * backlerp
	VectorSubtract (currententity->oldorigin, currententity->origin, delta);
	AngleVectors (currententity->angles, vectors[0], vectors[1], vectors[2]);

	move[0] = DotProduct (delta, vectors[0]);	// forward
	move[1] = -DotProduct (delta, vectors[1]);	// left
	move[2] = DotProduct (delta, vectors[2]);	// up

	VectorAdd (move, oldframe->translate, move);

	for (i=0 ; i<3 ; i++)
	{
		move[i] = backlerp*move[i] + frontlerp*frame->translate[i];
	}

	for (i=0 ; i<3 ; i++)
	{
		frontv[i] = frontlerp*frame->scale[i];
		backv[i] = backlerp*oldframe->scale[i];
	}

	if(s_lerped == NULL)
		s_lerped = memalign(32, MAX_VERTS * sizeof(vec4_t));

	lerp = (float*)s_lerped;

	GX_LerpVerts( paliashdr->num_xyz, v, ov, verts, lerp, move, frontv, backv );

	if ( gl_vertex_arrays->value )
	{
		if(colorArray == NULL)
			colorArray = memalign(32, MAX_VERTS*4 * sizeof(u8));

//		if ( currententity->flags & ( RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE ) )
		// PMM - added double damage shell
		if ( currententity->flags & ( RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE | RF_SHELL_DOUBLE | RF_SHELL_HALF_DAM) )
		{
			ccomp = shadelight[0] * 255;
			if(ccomp < 0.0)
				gxu_cur_r = 0;
			else if(ccomp > 255.0)
				gxu_cur_r = 255;
			else
				gxu_cur_r = ccomp;
			ccomp = shadelight[1] * 255;
			if(ccomp < 0.0)
				gxu_cur_g = 0;
			else if(ccomp > 255.0)
				gxu_cur_g = 255;
			else
				gxu_cur_g = ccomp;
			ccomp = shadelight[2] * 255;
			if(ccomp < 0.0)
				gxu_cur_b = 0;
			else if(ccomp > 255.0)
				gxu_cur_b = 255;
			else
				gxu_cur_b = ccomp;
			ccomp = alpha * 255;
			if(ccomp < 0.0)
				gxu_cur_a = 0;
			else if(ccomp > 255.0)
				gxu_cur_a = 255;
			else
				gxu_cur_a = ccomp;
		}
		else
		{
			//
			// pre light everything
			//
			for ( i = 0; i < paliashdr->num_xyz; i++ )
			{
				float l = shadedots[verts[i].lightnormalindex];

				ccomp = l * shadelight[0];
				if(ccomp < 0.0)
					colorArray[i*3+0] = 0;
				else if(ccomp > 1.0)
					colorArray[i*3+0] = 255;
				else
					colorArray[i*3+0] = ccomp * 255;
				ccomp = l * shadelight[1];
				if(ccomp < 0.0)
					colorArray[i*3+1] = 0;
				else if(ccomp > 1.0)
					colorArray[i*3+1] = 255;
				else
					colorArray[i*3+1] = ccomp * 255;
				ccomp = l * shadelight[2];
				if(ccomp < 0.0)
					colorArray[i*3+2] = 0;
				else if(ccomp > 1.0)
					colorArray[i*3+2] = 255;
				else
					colorArray[i*3+2] = ccomp * 255;
				colorArray[i*3+3] = 255;
			}
		}

		DCFlushRange(s_lerped, MAX_VERTS * sizeof(vec4_t));
		qgxSetVtxDesc(GX_VA_POS, GX_INDEX16);
		qgxSetArray(GX_VA_POS, s_lerped, sizeof(vec4_t));	// padded for SIMD

		if (!( currententity->flags & ( RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE | RF_SHELL_DOUBLE | RF_SHELL_HALF_DAM) ))
		{
			DCFlushRange(colorArray, MAX_VERTS*4 * sizeof(u8));
			qgxSetVtxDesc(GX_VA_CLR0, GX_INDEX16);
			qgxSetArray(GX_VA_CLR0, colorArray, 4 * sizeof(u8));
		};

		qgxInvVtxCache();

		while (1)
		{
			// get the vertex count and primitive type
			count = *order++;
			if (!count)
				break;		// done

			// PMM - added double damage shell
			if ( currententity->flags & ( RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE | RF_SHELL_DOUBLE | RF_SHELL_HALF_DAM) )
			{
				if (count < 0)
				{
					count = -count;
					qgxBegin (GX_TRIANGLEFAN, GX_VTXFMT0, count);
				}
				else
				{
					qgxBegin (GX_TRIANGLESTRIP, GX_VTXFMT0, count);
				};
				do
				{
					index_xyz = order[2];
					order += 3;

					GX_Position1x16(index_xyz);
					qgxColor4u8 ( gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a );

				} while (--count);
			}
			else
			{
				if (count < 0)
				{
					count = -count;
					qgxBegin (GX_TRIANGLEFAN, gxu_cur_vertex_format, count);
				}
				else
				{
					qgxBegin (GX_TRIANGLESTRIP, gxu_cur_vertex_format, count);
				};
				do
				{
					index_xyz = order[2];

					// normals and vertexes come from the frame list
//					l = shadedots[verts[index_xyz].lightnormalindex];
					
//					qglColor4f (l* shadelight[0], l*shadelight[1], l*shadelight[2], alpha);
					GX_Position1x16(index_xyz);
					GX_Color1x16(index_xyz);

					// texture coordinates come from the draw list
					qgxTexCoord2f32 (((float *)order)[0], ((float *)order)[1]);

					order += 3;

				} while (--count);
			}
			qgxEnd ();
		};

		if (!( currententity->flags & ( RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE | RF_SHELL_DOUBLE | RF_SHELL_HALF_DAM) ))
			qgxSetVtxDesc(GX_VA_CLR0, GX_DIRECT);

		qgxSetVtxDesc(GX_VA_POS, GX_DIRECT);
	}
	else
	{
		while (1)
		{
			// get the vertex count and primitive type
			count = *order++;
			if (!count)
				break;		// done

			if ( currententity->flags & ( RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE ) )
			{
				if (count < 0)
				{
					count = -count;
					qgxBegin (GX_TRIANGLEFAN, GX_VTXFMT0, count);
				}
				else
				{
					qgxBegin (GX_TRIANGLESTRIP, GX_VTXFMT0, count);
				};
				do
				{
					index_xyz = order[2];
					order += 3;

					ccomp = shadelight[0] * 255;
					if(ccomp < 0.0)
						gxu_cur_r = 0;
					else if(ccomp > 255.0)
						gxu_cur_r = 255;
					else
						gxu_cur_r = ccomp;
					ccomp = shadelight[1] * 255;
					if(ccomp < 0.0)
						gxu_cur_g = 0;
					else if(ccomp > 255.0)
						gxu_cur_g = 255;
					else
						gxu_cur_g = ccomp;
					ccomp = shadelight[2] * 255;
					if(ccomp < 0.0)
						gxu_cur_b = 0;
					else if(ccomp > 255.0)
						gxu_cur_b = 255;
					else
						gxu_cur_b = ccomp;
					ccomp = alpha * 255;
					if(ccomp < 0.0)
						gxu_cur_a = 0;
					else if(ccomp > 255.0)
						gxu_cur_a = 255;
					else
						gxu_cur_a = ccomp;

					qgxPosition3f32 (s_lerped[index_xyz][0], s_lerped[index_xyz][1], s_lerped[index_xyz][2]);
					qgxColor4u8 ( gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a );

				} while (--count);
			}
			else
			{
				if (count < 0)
				{
					count = -count;
					qgxBegin (GX_TRIANGLEFAN, gxu_cur_vertex_format, count);
				}
				else
				{
					qgxBegin (GX_TRIANGLESTRIP, gxu_cur_vertex_format, count);
				};
				do
				{
					index_xyz = order[2];

					// normals and vertexes come from the frame list
					l = shadedots[verts[index_xyz].lightnormalindex];
					
					ccomp = l * shadelight[0] * 255;
					if(ccomp < 0.0)
						gxu_cur_r = 0;
					else if(ccomp > 255.0)
						gxu_cur_r = 255;
					else
						gxu_cur_r = ccomp;
					ccomp = l * shadelight[1] * 255;
					if(ccomp < 0.0)
						gxu_cur_g = 0;
					else if(ccomp > 255.0)
						gxu_cur_g = 255;
					else
						gxu_cur_g = ccomp;
					ccomp = l * shadelight[2] * 255;
					if(ccomp < 0.0)
						gxu_cur_b = 0;
					else if(ccomp > 255.0)
						gxu_cur_b = 255;
					else
						gxu_cur_b = ccomp;
					ccomp = alpha * 255;
					if(ccomp < 0.0)
						gxu_cur_a = 0;
					else if(ccomp > 255.0)
						gxu_cur_a = 255;
					else
						gxu_cur_a = ccomp;

					qgxPosition3f32 (s_lerped[index_xyz][0], s_lerped[index_xyz][1], s_lerped[index_xyz][2]);
					qgxColor4u8 ( gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a );

					// texture coordinates come from the draw list
					qgxTexCoord2f32 (((float *)order)[0], ((float *)order)[1]);
					order += 3;

				} while (--count);
			}

			qgxEnd ();
		}
	}

//	if ( currententity->flags & ( RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE ) )
	// PMM - added double damage shell
	if ( currententity->flags & ( RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE | RF_SHELL_DOUBLE | RF_SHELL_HALF_DAM) )
		qgxEnableTexture();
}


#if 1
/*
=============
GX_DrawAliasShadow
=============
*/
extern	vec3_t			lightspot;

void GX_DrawAliasShadow (dmdl_t *paliashdr, int posenum)
{
	dtrivertx_t	*verts;
	int		*order;
	vec3_t	point;
	float	height, lheight;
	int		count;
	daliasframe_t	*frame;

	lheight = currententity->origin[2] - lightspot[2];

	frame = (daliasframe_t *)((byte *)paliashdr + paliashdr->ofs_frames 
		+ currententity->frame * paliashdr->framesize);
	verts = frame->verts;

	height = 0;

	order = (int *)((byte *)paliashdr + paliashdr->ofs_glcmds);

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
			qgxBegin (GX_TRIANGLEFAN, GX_VTXFMT0, count);
		}
		else
			qgxBegin (GX_TRIANGLESTRIP, GX_VTXFMT0, count);

		do
		{
			// normals and vertexes come from the frame list
/*
			point[0] = verts[order[2]].v[0] * frame->scale[0] + frame->translate[0];
			point[1] = verts[order[2]].v[1] * frame->scale[1] + frame->translate[1];
			point[2] = verts[order[2]].v[2] * frame->scale[2] + frame->translate[2];
*/

			memcpy( point, s_lerped[order[2]], sizeof( point )  );

			point[0] -= shadevector[0]*(point[2]+lheight);
			point[1] -= shadevector[1]*(point[2]+lheight);
			point[2] = height;
//			height -= 0.001;
			qgxPosition3f32 (point[0], point[1], point[2]);
			qgxColor4u8 (gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);

			order += 3;

//			verts++;

		} while (--count);

		qgxEnd ();
	}	
}

#endif

/*
** R_CullAliasModel
*/
static qboolean R_CullAliasModel( vec3_t bbox[8], entity_t *e )
{
	int i;
	vec3_t		mins, maxs;
	dmdl_t		*paliashdr;
	vec3_t		vectors[3];
	vec3_t		thismins, oldmins, thismaxs, oldmaxs;
	daliasframe_t *pframe, *poldframe;
	vec3_t angles;

	paliashdr = (dmdl_t *)currentmodel->extradata;

	if ( ( e->frame >= paliashdr->num_frames ) || ( e->frame < 0 ) )
	{
		ri.Con_Printf (PRINT_ALL, "R_CullAliasModel %s: no such frame %d\n", 
			currentmodel->name, e->frame);
		e->frame = 0;
	}
	if ( ( e->oldframe >= paliashdr->num_frames ) || ( e->oldframe < 0 ) )
	{
		ri.Con_Printf (PRINT_ALL, "R_CullAliasModel %s: no such oldframe %d\n", 
			currentmodel->name, e->oldframe);
		e->oldframe = 0;
	}

	pframe = ( daliasframe_t * ) ( ( byte * ) paliashdr + 
		                              paliashdr->ofs_frames +
									  e->frame * paliashdr->framesize);

	poldframe = ( daliasframe_t * ) ( ( byte * ) paliashdr + 
		                              paliashdr->ofs_frames +
									  e->oldframe * paliashdr->framesize);

	/*
	** compute axially aligned mins and maxs
	*/
	if ( pframe == poldframe )
	{
		for ( i = 0; i < 3; i++ )
		{
			mins[i] = pframe->translate[i];
			maxs[i] = mins[i] + pframe->scale[i]*255;
		}
	}
	else
	{
		for ( i = 0; i < 3; i++ )
		{
			thismins[i] = pframe->translate[i];
			thismaxs[i] = thismins[i] + pframe->scale[i]*255;

			oldmins[i]  = poldframe->translate[i];
			oldmaxs[i]  = oldmins[i] + poldframe->scale[i]*255;

			if ( thismins[i] < oldmins[i] )
				mins[i] = thismins[i];
			else
				mins[i] = oldmins[i];

			if ( thismaxs[i] > oldmaxs[i] )
				maxs[i] = thismaxs[i];
			else
				maxs[i] = oldmaxs[i];
		}
	}

	/*
	** compute a full bounding box
	*/
	for ( i = 0; i < 8; i++ )
	{
		vec3_t   tmp;

		if ( i & 1 )
			tmp[0] = mins[0];
		else
			tmp[0] = maxs[0];

		if ( i & 2 )
			tmp[1] = mins[1];
		else
			tmp[1] = maxs[1];

		if ( i & 4 )
			tmp[2] = mins[2];
		else
			tmp[2] = maxs[2];

		VectorCopy( tmp, bbox[i] );
	}

	/*
	** rotate the bounding box
	*/
	VectorCopy( e->angles, angles );
	angles[YAW] = -angles[YAW];
	AngleVectors( angles, vectors[0], vectors[1], vectors[2] );

	for ( i = 0; i < 8; i++ )
	{
		vec3_t tmp;

		VectorCopy( bbox[i], tmp );

		bbox[i][0] = DotProduct( vectors[0], tmp );
		bbox[i][1] = -DotProduct( vectors[1], tmp );
		bbox[i][2] = DotProduct( vectors[2], tmp );

		VectorAdd( e->origin, bbox[i], bbox[i] );
	}

	{
		int p, f, aggregatemask = ~0;

		for ( p = 0; p < 8; p++ )
		{
			int mask = 0;

			for ( f = 0; f < 4; f++ )
			{
				float dp = DotProduct( frustum[f].normal, bbox[p] );

				if ( ( dp - frustum[f].dist ) < 0 )
				{
					mask |= ( 1 << f );
				}
			}

			aggregatemask &= mask;
		}

		if ( aggregatemask )
		{
			return true;
		}

		return false;
	}
}

/*
=================
R_DrawAliasModel

=================
*/
void R_DrawAliasModel (entity_t *e)
{
	int			i;
	dmdl_t		*paliashdr;
	float		an;
	vec3_t		bbox[8];
	image_t		*skin;

	if ( !( e->flags & RF_WEAPONMODEL ) )
	{
		if ( R_CullAliasModel( bbox, e ) )
			return;
	}

	if ( e->flags & RF_WEAPONMODEL )
	{
		if ( r_lefthand->value == 2 )
			return;
	}

	paliashdr = (dmdl_t *)currentmodel->extradata;

	//
	// get lighting information
	//
	// PMM - rewrote, reordered to handle new shells & mixing
	// PMM - 3.20 code .. replaced with original way of doing it to keep mod authors happy
	//
	if ( currententity->flags & ( RF_SHELL_HALF_DAM | RF_SHELL_GREEN | RF_SHELL_RED | RF_SHELL_BLUE | RF_SHELL_DOUBLE ) )
	{
		VectorClear (shadelight);
		if (currententity->flags & RF_SHELL_HALF_DAM)
		{
				shadelight[0] = 0.56;
				shadelight[1] = 0.59;
				shadelight[2] = 0.45;
		}
		if ( currententity->flags & RF_SHELL_DOUBLE )
		{
			shadelight[0] = 0.9;
			shadelight[1] = 0.7;
		}
		if ( currententity->flags & RF_SHELL_RED )
			shadelight[0] = 1.0;
		if ( currententity->flags & RF_SHELL_GREEN )
			shadelight[1] = 1.0;
		if ( currententity->flags & RF_SHELL_BLUE )
			shadelight[2] = 1.0;
	}
/*
		// PMM -special case for godmode
		if ( (currententity->flags & RF_SHELL_RED) &&
			(currententity->flags & RF_SHELL_BLUE) &&
			(currententity->flags & RF_SHELL_GREEN) )
		{
			for (i=0 ; i<3 ; i++)
				shadelight[i] = 1.0;
		}
		else if ( currententity->flags & ( RF_SHELL_RED | RF_SHELL_BLUE | RF_SHELL_DOUBLE ) )
		{
			VectorClear (shadelight);

			if ( currententity->flags & RF_SHELL_RED )
			{
				shadelight[0] = 1.0;
				if (currententity->flags & (RF_SHELL_BLUE|RF_SHELL_DOUBLE) )
					shadelight[2] = 1.0;
			}
			else if ( currententity->flags & RF_SHELL_BLUE )
			{
				if ( currententity->flags & RF_SHELL_DOUBLE )
				{
					shadelight[1] = 1.0;
					shadelight[2] = 1.0;
				}
				else
				{
					shadelight[2] = 1.0;
				}
			}
			else if ( currententity->flags & RF_SHELL_DOUBLE )
			{
				shadelight[0] = 0.9;
				shadelight[1] = 0.7;
			}
		}
		else if ( currententity->flags & ( RF_SHELL_HALF_DAM | RF_SHELL_GREEN ) )
		{
			VectorClear (shadelight);
			// PMM - new colors
			if ( currententity->flags & RF_SHELL_HALF_DAM )
			{
				shadelight[0] = 0.56;
				shadelight[1] = 0.59;
				shadelight[2] = 0.45;
			}
			if ( currententity->flags & RF_SHELL_GREEN )
			{
				shadelight[1] = 1.0;
			}
		}
	}
			//PMM - ok, now flatten these down to range from 0 to 1.0.
	//		max_shell_val = max(shadelight[0], max(shadelight[1], shadelight[2]));
	//		if (max_shell_val > 0)
	//		{
	//			for (i=0; i<3; i++)
	//			{
	//				shadelight[i] = shadelight[i] / max_shell_val;
	//			}
	//		}
	// pmm
*/
	else if ( currententity->flags & RF_FULLBRIGHT )
	{
		for (i=0 ; i<3 ; i++)
			shadelight[i] = 1.0;
	}
	else
	{
		R_LightPoint (currententity->origin, shadelight);

		// player lighting hack for communication back to server
		// big hack!
		if ( currententity->flags & RF_WEAPONMODEL )
		{
			// pick the greatest component, which should be the same
			// as the mono value returned by software
			if (shadelight[0] > shadelight[1])
			{
				if (shadelight[0] > shadelight[2])
					r_lightlevel->value = 150*shadelight[0];
				else
					r_lightlevel->value = 150*shadelight[2];
			}
			else
			{
				if (shadelight[1] > shadelight[2])
					r_lightlevel->value = 150*shadelight[1];
				else
					r_lightlevel->value = 150*shadelight[2];
			}

		}
		
		if ( gl_monolightmap->string[0] != '0' )
		{
			float s = shadelight[0];

			if ( s < shadelight[1] )
				s = shadelight[1];
			if ( s < shadelight[2] )
				s = shadelight[2];

			shadelight[0] = s;
			shadelight[1] = s;
			shadelight[2] = s;
		}
	}

	if ( currententity->flags & RF_MINLIGHT )
	{
		for (i=0 ; i<3 ; i++)
			if (shadelight[i] > 0.1)
				break;
		if (i == 3)
		{
			shadelight[0] = 0.1;
			shadelight[1] = 0.1;
			shadelight[2] = 0.1;
		}
	}

	if ( currententity->flags & RF_GLOW )
	{	// bonus items will pulse with time
		float	scale;
		float	min;

		scale = 0.1 * sin(r_newrefdef.time*7);
		for (i=0 ; i<3 ; i++)
		{
			min = shadelight[i] * 0.8;
			shadelight[i] += scale;
			if (shadelight[i] < min)
				shadelight[i] = min;
		}
	}

// =================
// PGM	ir goggles color override
	if ( r_newrefdef.rdflags & RDF_IRGOGGLES && currententity->flags & RF_IR_VISIBLE)
	{
		shadelight[0] = 1.0;
		shadelight[1] = 0.0;
		shadelight[2] = 0.0;
	}
// PGM	
// =================

	shadedots = r_avertexnormal_dots[((int)(currententity->angles[1] * (SHADEDOT_QUANT / 360.0))) & (SHADEDOT_QUANT - 1)];
	
	an = currententity->angles[1]/180*M_PI;
	shadevector[0] = cos(-an);
	shadevector[1] = sin(-an);
	shadevector[2] = 1;
	VectorNormalize (shadevector);

	//
	// locate the proper data
	//

	c_alias_polys += paliashdr->num_tris;

	//
	// draw all the triangles
	//
	if (currententity->flags & RF_DEPTHHACK) // hack the depth range to prevent view model from poking into walls
		qgxSetViewport (gxu_viewport_x, gxu_viewport_y, gxu_viewport_width, gxu_viewport_height, gxu_depth_min, gxu_depth_min + 0.3*(gxu_depth_max - gxu_depth_min));

	if ( ( currententity->flags & RF_WEAPONMODEL ) && ( r_lefthand->value == 1.0F ) )
	{
		Mtx44 m;

		gxu_cur_projection_matrix++;
		qguMtxIdentity(gxu_projection_matrices[gxu_cur_projection_matrix]);
		qguMtxScale(m, -1, 1, 1);
		qguMtxConcat(gxu_projection_matrices[gxu_cur_projection_matrix], m, gxu_projection_matrices[gxu_cur_projection_matrix]);
		qguPerspective(m, r_newrefdef.fov_y, ( float ) r_newrefdef.width / r_newrefdef.height,  4,  4096);
		qguMtxConcat(gxu_projection_matrices[gxu_cur_projection_matrix], m, gxu_projection_matrices[gxu_cur_projection_matrix]);
		qgxLoadProjectionMtx(gxu_projection_matrices[gxu_cur_projection_matrix], GX_PERSPECTIVE);

		gxu_cull_mode = GX_CULL_FRONT;
		if(gxu_cull_enabled)
			qgxSetCullMode(gxu_cull_mode);
	}

	qguMtxCopy(gxu_modelview_matrices[gxu_cur_modelview_matrix], gxu_modelview_matrices[gxu_cur_modelview_matrix + 1]);
	gxu_cur_modelview_matrix++;
	e->angles[PITCH] = -e->angles[PITCH];	// sigh.
	R_RotateForEntity (e);
	e->angles[PITCH] = -e->angles[PITCH];	// sigh.

	// select skin
	if (currententity->skin)
		skin = currententity->skin;	// custom player skin
	else
	{
		if (currententity->skinnum >= MAX_MD2SKINS)
			skin = currentmodel->skins[0];
		else
		{
			skin = currentmodel->skins[currententity->skinnum];
			if (!skin)
				skin = currentmodel->skins[0];
		}
	}
	if (!skin)
		skin = r_notexture;	// fallback...
	GX_Bind(skin->texnum);

	// draw it

	// Implement this ASAP:
	//qglShadeModel (GL_SMOOTH);

	GX_TexEnv( GX_MODULATE );
	if ( currententity->flags & RF_TRANSLUCENT )
	{
		qgxSetBlendMode(GX_BM_BLEND, gxu_blend_src_value, gxu_blend_dst_value, GX_LO_NOOP);
	}


	if ( (currententity->frame >= paliashdr->num_frames) 
		|| (currententity->frame < 0) )
	{
		ri.Con_Printf (PRINT_ALL, "R_DrawAliasModel %s: no such frame %d\n",
			currentmodel->name, currententity->frame);
		currententity->frame = 0;
		currententity->oldframe = 0;
	}

	if ( (currententity->oldframe >= paliashdr->num_frames)
		|| (currententity->oldframe < 0))
	{
		ri.Con_Printf (PRINT_ALL, "R_DrawAliasModel %s: no such oldframe %d\n",
			currentmodel->name, currententity->oldframe);
		currententity->frame = 0;
		currententity->oldframe = 0;
	}

	if ( !r_lerpmodels->value )
		currententity->backlerp = 0;
	GX_DrawAliasFrameLerp (paliashdr, currententity->backlerp);

	GX_TexEnv( GX_REPLACE );
	// Implement this ASAP:
	//qglShadeModel (GL_FLAT);

	gxu_cur_modelview_matrix--;
	qgxLoadPosMtxImm(gxu_modelview_matrices[gxu_cur_modelview_matrix], GX_PNMTX0);

#if 0
	qglDisable( GL_CULL_FACE );
	qglPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	qglDisable( GL_TEXTURE_2D );
	qglBegin( GL_TRIANGLE_STRIP );
	for ( i = 0; i < 8; i++ )
	{
		qglVertex3fv( bbox[i] );
	}
	qglEnd();
	qglEnable( GL_TEXTURE_2D );
	qglPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	qglEnable( GL_CULL_FACE );
#endif

	if ( ( currententity->flags & RF_WEAPONMODEL ) && ( r_lefthand->value == 1.0F ) )
	{
		gxu_cur_projection_matrix--;
		qgxLoadProjectionMtx(gxu_projection_matrices[gxu_cur_projection_matrix], gxu_cur_projection_type);
		gxu_cull_mode = GX_CULL_BACK;
		if(gxu_cull_enabled)
			qgxSetCullMode(gxu_cull_mode);
	}

	if ( currententity->flags & RF_TRANSLUCENT )
	{
		qgxSetBlendMode(GX_BM_NONE, gxu_blend_src_value, gxu_blend_dst_value, GX_LO_NOOP);
	}

	if (currententity->flags & RF_DEPTHHACK)
		qgxSetViewport (gxu_viewport_x, gxu_viewport_y, gxu_viewport_width, gxu_viewport_height, gxu_depth_min, gxu_depth_max);

#if 1
	if (gl_shadows->value && !(currententity->flags & (RF_TRANSLUCENT | RF_WEAPONMODEL)))
	{
		qguMtxCopy(gxu_modelview_matrices[gxu_cur_modelview_matrix], gxu_modelview_matrices[gxu_cur_modelview_matrix + 1]);
		gxu_cur_modelview_matrix++;
		R_RotateForEntity (e);
		qgxDisableTexture();
		qgxSetBlendMode(GX_BM_BLEND, gxu_blend_src_value, gxu_blend_dst_value, GX_LO_NOOP);
		gxu_cur_r = 0;
		gxu_cur_g = 0;
		gxu_cur_b = 0;
		gxu_cur_a = 127;
		GX_DrawAliasShadow (paliashdr, currententity->frame );
		gxu_cur_vertex_format = GX_VTXFMT1;
		qgxEnableTexture();
		qgxSetBlendMode(GX_BM_NONE, gxu_blend_src_value, gxu_blend_dst_value, GX_LO_NOOP);
		gxu_cur_modelview_matrix--;
		qgxLoadPosMtxImm(gxu_modelview_matrices[gxu_cur_modelview_matrix], GX_PNMTX0);
	}
#endif
	gxu_cur_r = 255;
	gxu_cur_g = 255;
	gxu_cur_b = 255;
	gxu_cur_a = 255;
}


