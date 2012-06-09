/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
#include "tr_local.h"
#include "gxutils.h"
#include <malloc.h>

backEndData_t	*backEndData[SMP_FRAMES];
backEndState_t	backEnd;


/*
static float	s_flipMatrix[16] = {
	// convert from our coordinate system (looking down X)
	// to OpenGL's coordinate system (looking down -Z)
	0, 0, -1, 0,
	-1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 0, 1
};
*/

gxtexobj_t	gxtexobjs[1024 + MAX_DRAWIMAGES];

int GX_TEXTURE0, GX_TEXTURE1;

int		gx_tex_allocated; // To track amount of memory used for textures

/*
** GX_Bind
*/
void GX_Bind( image_t *image ) {
	int texnum;

	if ( !image ) {
		ri.Printf( PRINT_WARNING, "GX_Bind: NULL image\n" );
		texnum = tr.defaultImage->texnum;
	} else {
		texnum = image->texnum;
	}

	if ( r_nobind->integer && tr.dlightImage ) {		// performance evaluation option
		texnum = tr.dlightImage->texnum;
	}

	if ( glState.currenttextures[glState.currenttmu] != texnum ) {
		image->frameUsed = tr.frameCount;
		glState.currenttextures[glState.currenttmu] = texnum;
		if(gxtexobjs[texnum].data != NULL)
			qgxLoadTexObj(&gxtexobjs[texnum].texobj, GX_TEXMAP0 + glState.currenttmu );
	}
}

/*
** GX_SelectTexture
*/
void GX_SelectTexture( int unit )
{
	if ( glState.currenttmu == unit )
	{
		return;
	}

	if (( unit != 0 )&&( unit != 1 ))
	{
		ri.Error( ERR_DROP, "GX_SelectTexture: unit = %i", unit );
	}

	glState.currenttmu = unit;
}


/*
** GX_BindMultitexture
*/
void GX_BindMultitexture( image_t *image0, u32 env0, image_t *image1, u32 env1 ) {
	int		texnum0, texnum1;

	texnum0 = image0->texnum;
	texnum1 = image1->texnum;

	if ( r_nobind->integer && tr.dlightImage ) {		// performance evaluation option
		texnum0 = texnum1 = tr.dlightImage->texnum;
	}

	if ( glState.currenttextures[1] != texnum1 ) {
		GX_SelectTexture( 1 );
		image1->frameUsed = tr.frameCount;
		glState.currenttextures[1] = texnum1;
		if(gxtexobjs[texnum1].data != NULL)
			qgxLoadTexObj(&gxtexobjs[texnum1].texobj, GX_TEXMAP1 );
	}
	if ( glState.currenttextures[0] != texnum0 ) {
		GX_SelectTexture( 0 );
		image0->frameUsed = tr.frameCount;
		glState.currenttextures[0] = texnum0;
		if(gxtexobjs[texnum0].data != NULL)
			qgxLoadTexObj(&gxtexobjs[texnum0].texobj, GX_TEXMAP0 );
	}
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
	qboolean changed = qfalse;
	int texnum = glState.currenttextures[glState.currenttmu];
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
		changed = qtrue;
	};
	gxtexobjs[texnum].width = width;
	gxtexobjs[texnum].height = height;
	return changed;
}

void GX_BindCurrentTex(qboolean changed, int format, int mipmap)
{
	int texnum = glState.currenttextures[glState.currenttmu];
	DCFlushRange(gxtexobjs[texnum].data, gxtexobjs[texnum].length);

	if(format == GX_TF_CI8)
		qgxInitTexObjCI(&gxtexobjs[texnum].texobj, gxtexobjs[texnum].data, gxtexobjs[texnum].width, gxtexobjs[texnum].height, format, GX_REPEAT, GX_REPEAT, mipmap, GX_TLUT0);
	else
		qgxInitTexObj(&gxtexobjs[texnum].texobj, gxtexobjs[texnum].data, gxtexobjs[texnum].width, gxtexobjs[texnum].height, format, GX_REPEAT, GX_REPEAT, mipmap);

	qgxLoadTexObj(&gxtexobjs[texnum].texobj, GX_TEXMAP0 + glState.currenttmu);
	if(changed)
		qgxInvalidateTexAll();
}

void GX_LoadAndBind (void* data, int length, int width, int height, int format)
{
	qboolean changed = GX_ReallocTex(length, width, height);
	int texnum = glState.currenttextures[glState.currenttmu];
	switch(format)
	{
	case GX_TF_RGBA8:
		GXU_CopyTexRGBA8((byte*)data, width, height, (byte*)(gxtexobjs[texnum].data));
		break;
	case GX_TF_RGB5A3:
		GXU_CopyTexRGB5A3((byte*)data, width, height, (byte*)(gxtexobjs[texnum].data));
		break;
	case GX_TF_CI8:
	case GX_TF_I8:
	case GX_TF_A8:
		GXU_CopyTexV8((byte*)data, width, height, (byte*)(gxtexobjs[texnum].data));
		break;
	case GX_TF_IA4:
		GXU_CopyTexIA4((byte*)data, width, height, (byte*)(gxtexobjs[texnum].data));
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
		dst = (byte*)(gxtexobjs[glState.currenttextures[glState.currenttmu]].data);
		if(dst != NULL)
		{
			tex_width = gxtexobjs[glState.currenttextures[glState.currenttmu]].width;
			tex_height = gxtexobjs[glState.currenttextures[glState.currenttmu]].height;
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
		dst = (byte*)(gxtexobjs[glState.currenttextures[glState.currenttmu]].data);
		if(dst != NULL)
		{
			tex_width = gxtexobjs[glState.currenttextures[glState.currenttmu]].width;
			tex_height = gxtexobjs[glState.currenttextures[glState.currenttmu]].height;
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
		dst = (byte*)(gxtexobjs[glState.currenttextures[glState.currenttmu]].data);
		if(dst != NULL)
		{
			tex_width = gxtexobjs[glState.currenttextures[glState.currenttmu]].width;
			tex_height = gxtexobjs[glState.currenttextures[glState.currenttmu]].height;
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
		dst = (byte*)(gxtexobjs[glState.currenttextures[glState.currenttmu]].data);
		if(dst != NULL)
		{
			tex_width = gxtexobjs[glState.currenttextures[glState.currenttmu]].width;
			tex_height = gxtexobjs[glState.currenttextures[glState.currenttmu]].height;
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


/*
** GX_Cull
*/
void GX_Cull( int cullType ) {
	if ( glState.faceCulling == cullType ) {
		return;
	}

	glState.faceCulling = cullType;

	if ( cullType == CT_TWO_SIDED ) 
	{
		gxu_cull_enabled = false;
		qgxSetCullMode(GX_CULL_NONE);
	} 
	else 
	{
		if ( cullType == CT_BACK_SIDED )
		{
			if ( backEnd.viewParms.isMirror )
			{
				gxu_cull_mode = GX_CULL_BACK;
			}
			else
			{
				gxu_cull_mode = GX_CULL_FRONT;
			}
		}
		else
		{
			if ( backEnd.viewParms.isMirror )
			{
				gxu_cull_mode = GX_CULL_FRONT;
			}
			else
			{
				gxu_cull_mode = GX_CULL_BACK;
			}
		}
		gxu_cull_enabled = true;
		qgxSetCullMode(gxu_cull_mode);
	}
}

/*
** GX_TexEnv
*/
void GX_TexEnv( int env )
{
	if ( env == glState.texEnv[glState.currenttmu] )
	{
		return;
	}

	glState.texEnv[glState.currenttmu] = env;


	switch ( env )
	{
	case GX_MODULATE:
	case GX_REPLACE:
	case GX_DECAL:
		qgxSetTevOp( GX_TEVSTAGE0 + glState.currenttmu, env );
	case GXU_TEVOP_ADD:
		qguSetTevOpAdd( GX_TEVSTAGE0 + glState.currenttmu );
		break;
	default:
		ri.Error( ERR_DROP, "GX_TexEnv: invalid env '%d' passed\n", env );
		break;
	}
}

/*
** GL_State
**
** This routine is responsible for setting the most commonly changed state
** in Q3.
*/
void GL_State( unsigned long stateBits )
{
	unsigned long diff = stateBits ^ glState.glStateBits;

	if ( !diff )
	{
		return;
	}

	//
	// check depthFunc bits
	//
	if ( diff & GLS_DEPTHFUNC_EQUAL )
	{
		if ( stateBits & GLS_DEPTHFUNC_EQUAL )
		{
			gxu_cur_z_func = GX_EQUAL;
			GX_SetZMode(gxu_z_test_enabled, gxu_cur_z_func, gxu_z_write_enabled);
		}
		else
		{
			gxu_cur_z_func = GX_LEQUAL;
			GX_SetZMode(gxu_z_test_enabled, gxu_cur_z_func, gxu_z_write_enabled);
		}
	}

	//
	// check blend bits
	//
	if ( diff & ( GLS_SRCBLEND_BITS | GLS_DSTBLEND_BITS ) )
	{
		if ( stateBits & ( GLS_SRCBLEND_BITS | GLS_DSTBLEND_BITS ) )
		{
			switch ( stateBits & GLS_SRCBLEND_BITS )
			{
			case GLS_SRCBLEND_ZERO:
				gxu_blend_src_value = GX_BL_ZERO;
				break;
			case GLS_SRCBLEND_ONE:
				gxu_blend_src_value = GX_BL_ONE;
				break;
			case GLS_SRCBLEND_DST_COLOR:
				gxu_blend_src_value = GX_BL_DSTCLR;
				break;
			case GLS_SRCBLEND_ONE_MINUS_DST_COLOR:
				gxu_blend_src_value = GX_BL_INVDSTCLR;
				break;
			case GLS_SRCBLEND_SRC_ALPHA:
				gxu_blend_src_value = GX_BL_SRCALPHA;
				break;
			case GLS_SRCBLEND_ONE_MINUS_SRC_ALPHA:
				gxu_blend_src_value = GX_BL_INVSRCALPHA;
				break;
			case GLS_SRCBLEND_DST_ALPHA:
				gxu_blend_src_value = GX_BL_DSTALPHA;
				break;
			case GLS_SRCBLEND_ONE_MINUS_DST_ALPHA:
				gxu_blend_src_value = GX_BL_INVDSTALPHA;
				break;
			case GLS_SRCBLEND_ALPHA_SATURATE:
				gxu_blend_src_value = GX_BL_SRCALPHA/*************************GL_SRC_ALPHA_SATURATE*/;
				break;
			default:
				gxu_blend_src_value = GX_BL_ONE;		// to get warning to shut up
				ri.Error( ERR_DROP, "GL_State: invalid src blend state bits\n" );
				break;
			}

			switch ( stateBits & GLS_DSTBLEND_BITS )
			{
			case GLS_DSTBLEND_ZERO:
				gxu_blend_dst_value = GX_BL_ZERO;
				break;
			case GLS_DSTBLEND_ONE:
				gxu_blend_dst_value = GX_BL_ONE;
				break;
			case GLS_DSTBLEND_SRC_COLOR:
				gxu_blend_dst_value = GX_BL_SRCCLR;
				break;
			case GLS_DSTBLEND_ONE_MINUS_SRC_COLOR:
				gxu_blend_dst_value = GX_BL_INVSRCCLR;
				break;
			case GLS_DSTBLEND_SRC_ALPHA:
				gxu_blend_dst_value = GX_BL_SRCALPHA;
				break;
			case GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA:
				gxu_blend_dst_value = GX_BL_INVSRCALPHA;
				break;
			case GLS_DSTBLEND_DST_ALPHA:
				gxu_blend_dst_value = GX_BL_DSTALPHA;
				break;
			case GLS_DSTBLEND_ONE_MINUS_DST_ALPHA:
				gxu_blend_dst_value = GX_BL_INVDSTALPHA;
				break;
			default:
				gxu_blend_dst_value = GX_BL_ONE;		// to get warning to shut up
				ri.Error( ERR_DROP, "GL_State: invalid dst blend state bits\n" );
				break;
			}

			qgxSetBlendMode(GX_BM_BLEND, gxu_blend_src_value, gxu_blend_dst_value, GX_LO_NOOP);
		}
		else
		{
			qgxSetBlendMode(GX_BM_NONE, gxu_blend_src_value, gxu_blend_dst_value, GX_LO_NOOP);
		}
	}

	//
	// check depthmask
	//
	if ( diff & GLS_DEPTHMASK_TRUE )
	{
		if ( stateBits & GLS_DEPTHMASK_TRUE )
		{
			gxu_z_write_enabled = GX_TRUE;
			qgxSetZMode(gxu_z_test_enabled, gxu_cur_z_func, gxu_z_write_enabled);
		}
		else
		{
			gxu_z_write_enabled = GX_FALSE;
			qgxSetZMode(gxu_z_test_enabled, gxu_cur_z_func, gxu_z_write_enabled);
		}
	}

	//
	// fill/line mode
	//
	if ( diff & GLS_POLYMODE_LINE )
	{
		if ( stateBits & GLS_POLYMODE_LINE )
		{
			// This is not yet available in the current platform. Removing:
			//qglPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		}
		else
		{
			// This is not yet available in the current platform. Removing:
			//qglPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		}
	}

	//
	// depthtest
	//
	if ( diff & GLS_DEPTHTEST_DISABLE )
	{
		if ( stateBits & GLS_DEPTHTEST_DISABLE )
		{
			gxu_z_test_enabled = GX_FALSE;
			qgxSetZMode(gxu_z_test_enabled, gxu_cur_z_func, gxu_z_write_enabled);
		}
		else
		{
			gxu_z_test_enabled = GX_TRUE;
			qgxSetZMode(gxu_z_test_enabled, gxu_cur_z_func, gxu_z_write_enabled);
		}
	}

	//
	// alpha test
	//
	if ( diff & GLS_ATEST_BITS )
	{
		switch ( stateBits & GLS_ATEST_BITS )
		{
		case 0:
			gxu_alpha_test_enabled = false;
			qgxSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
			break;
		case GLS_ATEST_GT_0:
			gxu_alpha_test_lower = 0;
			gxu_alpha_test_higher = 255;
			gxu_alpha_test_enabled = true;
			qgxSetAlphaCompare(GX_GREATER, gxu_alpha_test_lower, GX_AOP_AND, GX_LEQUAL, gxu_alpha_test_higher);
			break;
		case GLS_ATEST_LT_80:
			gxu_alpha_test_lower = 0;
			gxu_alpha_test_higher = 127;
			gxu_alpha_test_enabled = true;
			qgxSetAlphaCompare(GX_GEQUAL, gxu_alpha_test_lower, GX_AOP_AND, GX_LESS, gxu_alpha_test_higher);
			break;
		case GLS_ATEST_GE_80:
			gxu_alpha_test_lower = 128;
			gxu_alpha_test_higher = 255;
			gxu_alpha_test_enabled = true;
			qgxSetAlphaCompare(GX_GEQUAL, gxu_alpha_test_lower, GX_AOP_AND, GX_LEQUAL, gxu_alpha_test_higher);
			break;
		default:
			assert( 0 );
			break;
		}
	}

	glState.glStateBits = stateBits;
}



/*
================
RB_Hyperspace

A player has predicted a teleport, but hasn't arrived yet
================
*/
static void RB_Hyperspace( void ) {
	int		c;

	if ( !backEnd.isHyperspace ) {
		// do initialization shit
	}

	c = backEnd.refdef.time & 255;
	gxu_background_color.r = c;
	gxu_background_color.g = c;
	gxu_background_color.b = c;
	gxu_background_color.a = 255;
	//qglClear( GL_COLOR_BUFFER_BIT );

	backEnd.isHyperspace = qtrue;
}


static void SetViewportAndScissor( void ) {
	int i, j, k;

	k = 0;
	for(i = 0; i < 4; i++)
	{
		for(j = 0; j < 4; j++)
		{
			gxu_projection_matrices[gxu_cur_projection_matrix][i][j] = backEnd.viewParms.projectionMatrix[k];
			k++;
		}
	}

	qgxLoadProjectionMtx(gxu_projection_matrices[gxu_cur_projection_matrix], gxu_cur_projection_type);

	// set the window clipping
	gxu_viewport_x = backEnd.viewParms.viewportX;
	gxu_viewport_y = backEnd.viewParms.viewportY;
	gxu_viewport_width = backEnd.viewParms.viewportWidth;
	gxu_viewport_height = backEnd.viewParms.viewportHeight;
	qgxSetViewport (gxu_viewport_x, gxu_viewport_y, gxu_viewport_width, gxu_viewport_height, gxu_depth_min, gxu_depth_max);
	qgxSetScissor(gxu_viewport_x, gxu_viewport_y, gxu_viewport_width, gxu_viewport_height);
}

/*
=================
RB_BeginDrawingView

Any mirrored or portaled views have already been drawn, so prepare
to actually render the visible surfaces for this view
=================
*/
void RB_BeginDrawingView (void) {
	//int clearBits = 0;

	// sync with gl if needed
	if ( r_finish->integer == 1 && !glState.finishCalled ) {
		//qglFinish ();
		glState.finishCalled = qtrue;
	}
	if ( r_finish->integer == 0 ) {
		glState.finishCalled = qtrue;
	}

	// we will need to change the projection matrix before drawing
	// 2D images again
	backEnd.projection2D = qfalse;
	gxu_cur_projection_type = GX_PERSPECTIVE;

	//
	// set the modelview matrix for the viewer
	//
	SetViewportAndScissor();

	// ensures that depth writes are enabled for the depth clear
	GL_State( GLS_DEFAULT );
	// clear relevant buffers
	gxu_clear_buffers = GX_TRUE;

	if ( r_measureOverdraw->integer || r_shadows->integer == 2 )
	{
		//clearBits |= GL_STENCIL_BUFFER_BIT;
	}
	if ( r_fastsky->integer && !( backEnd.refdef.rdflags & RDF_NOWORLDMODEL ) )
	{
		gxu_clear_color_buffer = GX_TRUE;	// FIXME: only if sky shaders have been used
#ifdef _DEBUG
		qglClearColor( 0.8f, 0.7f, 0.4f, 1.0f );	// FIXME: get color of sky
#else
		gxu_background_color.r = 0;
		gxu_background_color.g = 0;
		gxu_background_color.b = 0;
		gxu_background_color.a = 255;	// FIXME: get color of sky
#endif
	}
	//qglClear( clearBits );

	if ( ( backEnd.refdef.rdflags & RDF_HYPERSPACE ) )
	{
		RB_Hyperspace();
		return;
	}
	else
	{
		backEnd.isHyperspace = qfalse;
	}

	glState.faceCulling = -1;		// force face culling to set next time

	// we will only draw a sun if there was sky rendered in this view
	backEnd.skyRenderedThisView = qfalse;

	// clip to the plane of the portal
	// ***************** Clipping planes unavailable in this platform. Removing:
	/*
	if ( backEnd.viewParms.isPortal ) {
		float	plane[4];
		double	plane2[4];

		plane[0] = backEnd.viewParms.portalPlane.normal[0];
		plane[1] = backEnd.viewParms.portalPlane.normal[1];
		plane[2] = backEnd.viewParms.portalPlane.normal[2];
		plane[3] = backEnd.viewParms.portalPlane.dist;

		plane2[0] = DotProduct (backEnd.viewParms.or.axis[0], plane);
		plane2[1] = DotProduct (backEnd.viewParms.or.axis[1], plane);
		plane2[2] = DotProduct (backEnd.viewParms.or.axis[2], plane);
		plane2[3] = DotProduct (plane, backEnd.viewParms.or.origin) - plane[3];

		qglLoadMatrixf( s_flipMatrix );
		qglClipPlane (GL_CLIP_PLANE0, plane2);
		qglEnable (GL_CLIP_PLANE0);
	} else {
		qglDisable (GL_CLIP_PLANE0);
	}
	*/
}


#define	MAC_EVENT_PUMP_MSEC		5

/*
==================
RB_RenderDrawSurfList
==================
*/
void RB_RenderDrawSurfList( drawSurf_t *drawSurfs, int numDrawSurfs ) {
	shader_t		*shader, *oldShader;
	int				fogNum, oldFogNum;
	int				entityNum, oldEntityNum;
	int				dlighted, oldDlighted;
	qboolean		depthRange, oldDepthRange;
	int				i, j, k;
	drawSurf_t		*drawSurf;
	int				oldSort;
	float			originalTime;
#ifdef __MACOS__
	int				macEventTime;

	Sys_PumpEvents();		// crutch up the mac's limited buffer queue size

	// we don't want to pump the event loop too often and waste time, so
	// we are going to check every shader change
	macEventTime = ri.Milliseconds() + MAC_EVENT_PUMP_MSEC;
#endif

	// save original time for entity shader offsets
	originalTime = backEnd.refdef.floatTime;

	// clear the z buffer, set the modelview, etc
	RB_BeginDrawingView ();

	// draw everything
	oldEntityNum = -1;
	backEnd.currentEntity = &tr.worldEntity;
	oldShader = NULL;
	oldFogNum = -1;
	oldDepthRange = qfalse;
	oldDlighted = qfalse;
	oldSort = -1;
	depthRange = qfalse;

	backEnd.pc.c_surfaces += numDrawSurfs;

	for (i = 0, drawSurf = drawSurfs ; i < numDrawSurfs ; i++, drawSurf++) {
		if ( drawSurf->sort == oldSort ) {
			// fast path, same as previous sort
			rb_surfaceTable[ *drawSurf->surface ]( drawSurf->surface );
			continue;
		}
		oldSort = drawSurf->sort;
		R_DecomposeSort( drawSurf->sort, &entityNum, &shader, &fogNum, &dlighted );

		//
		// change the tess parameters if needed
		// a "entityMergable" shader is a shader that can have surfaces from seperate
		// entities merged into a single batch, like smoke and blood puff sprites
		if (shader != oldShader || fogNum != oldFogNum || dlighted != oldDlighted 
			|| ( entityNum != oldEntityNum && !shader->entityMergable ) ) {
			if (oldShader != NULL) {
#ifdef __MACOS__	// crutch up the mac's limited buffer queue size
				int		t;

				t = ri.Milliseconds();
				if ( t > macEventTime ) {
					macEventTime = t + MAC_EVENT_PUMP_MSEC;
					Sys_PumpEvents();
				}
#endif
				RB_EndSurface();
			}
			RB_BeginSurface( shader, fogNum );
			oldShader = shader;
			oldFogNum = fogNum;
			oldDlighted = dlighted;
		}

		//
		// change the modelview matrix if needed
		//
		if ( entityNum != oldEntityNum ) {
			depthRange = qfalse;

			if ( entityNum != ENTITYNUM_WORLD ) {
				backEnd.currentEntity = &backEnd.refdef.entities[entityNum];
				backEnd.refdef.floatTime = originalTime - backEnd.currentEntity->e.shaderTime;
				// we have to reset the shaderTime as well otherwise image animations start
				// from the wrong frame
				tess.shaderTime = backEnd.refdef.floatTime - tess.shader->timeOffset;

				// set up the transformation matrix
				R_RotateForEntity( backEnd.currentEntity, &backEnd.viewParms, &backEnd.or );

				// set up the dynamic lighting if needed
				if ( backEnd.currentEntity->needDlights ) {
					R_TransformDlights( backEnd.refdef.num_dlights, backEnd.refdef.dlights, &backEnd.or );
				}

				if ( backEnd.currentEntity->e.renderfx & RF_DEPTHHACK ) {
					// hack the depth range to prevent view model from poking into walls
					depthRange = qtrue;
				}
			} else {
				backEnd.currentEntity = &tr.worldEntity;
				backEnd.refdef.floatTime = originalTime;
				backEnd.or = backEnd.viewParms.world;
				// we have to reset the shaderTime as well otherwise image animations on
				// the world (like water) continue with the wrong frame
				tess.shaderTime = backEnd.refdef.floatTime - tess.shader->timeOffset;
				R_TransformDlights( backEnd.refdef.num_dlights, backEnd.refdef.dlights, &backEnd.or );
			}

			k = 0;
			for(i = 0; i < 3; i++)
			{
				for(j = 0; j < 4; j++)
				{
					gxu_modelview_matrices[gxu_cur_modelview_matrix][i][j] = backEnd.or.modelMatrix[k];
					k++;
				}
			}

			//
			// change depthrange if needed
			//
			if ( oldDepthRange != depthRange ) {
				if ( depthRange ) {
					gxu_depth_min = 0.0;
					gxu_depth_max = 0.3;
				} else {
					gxu_depth_min = 0.0;
					gxu_depth_max = 1.0;
				}
				qgxSetViewport (gxu_viewport_x, gxu_viewport_y, gxu_viewport_width, gxu_viewport_height, gxu_depth_min, gxu_depth_max);
				oldDepthRange = depthRange;
			}

			oldEntityNum = entityNum;
		}

		// add the triangles for this surface
		rb_surfaceTable[ *drawSurf->surface ]( drawSurf->surface );
	}

	backEnd.refdef.floatTime = originalTime;

	// draw the contents of the last shader batch
	if (oldShader != NULL) {
		RB_EndSurface();
	}

	// go back to the world modelview matrix
	k = 0;
	for(i = 0; i < 3; i++)
	{
		for(j = 0; j < 4; j++)
		{
			gxu_modelview_matrices[gxu_cur_modelview_matrix][i][j] = backEnd.viewParms.world.modelMatrix[k];
			k++;
		}
	}
	if ( depthRange ) {
		gxu_depth_min = 0.0;
		gxu_depth_max = 1.0;
		qgxSetViewport (gxu_viewport_x, gxu_viewport_y, gxu_viewport_width, gxu_viewport_height, gxu_depth_min, gxu_depth_max);
	}

#if 0
	RB_DrawSun();
#endif
	// darken down any stencil shadows
	RB_ShadowFinish();		

	// add light flares on lights that aren't obscured
	RB_RenderFlares();

#ifdef __MACOS__
	Sys_PumpEvents();		// crutch up the mac's limited buffer queue size
#endif
}


/*
============================================================================

RENDER BACK END THREAD FUNCTIONS

============================================================================
*/

/*
================
RB_SetGL2D

================
*/
void	RB_SetGL2D (void) {
	backEnd.projection2D = qtrue;
	gxu_cur_projection_type = GX_ORTHOGRAPHIC;

	// set 2D virtual screen size
	gxu_viewport_x = 0;
	gxu_viewport_y = 0;
	gxu_viewport_width = glConfig.vidWidth;
	gxu_viewport_height = glConfig.vidHeight;
	qgxSetViewport (gxu_viewport_x, gxu_viewport_y, gxu_viewport_width, gxu_viewport_height, gxu_depth_min, gxu_depth_max);
	qgxSetScissor(gxu_viewport_x, gxu_viewport_y, gxu_viewport_width, gxu_viewport_height);
	qguOrtho(gxu_projection_matrices[gxu_cur_projection_matrix], 0, glConfig.vidHeight, 0, glConfig.vidWidth, 0, 1);
	gxu_cur_projection_type = GX_ORTHOGRAPHIC;
	qgxLoadProjectionMtx(gxu_projection_matrices[gxu_cur_projection_matrix], gxu_cur_projection_type);
	qguMtxIdentity(gxu_modelview_matrices[gxu_cur_modelview_matrix]);
	qgxLoadPosMtxImm(gxu_modelview_matrices[gxu_cur_modelview_matrix], GX_PNMTX0);

	GL_State( GLS_DEPTHTEST_DISABLE |
			  GLS_SRCBLEND_SRC_ALPHA |
			  GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA );

	gxu_cull_enabled = false;
	qgxSetCullMode(GX_CULL_NONE);
	//qglDisable( GL_CLIP_PLANE0 );

	// set time for 2D shaders
	backEnd.refdef.time = ri.Milliseconds();
	backEnd.refdef.floatTime = backEnd.refdef.time * 0.001f;
}


/*
=============
RE_StretchRaw

FIXME: not exactly backend
Stretches a raw 32 bit power of 2 bitmap image over the given screen rectangle.
Used for cinematics.
=============
*/
void RE_StretchRaw (int x, int y, int w, int h, int cols, int rows, const byte *data, int client, qboolean dirty) {
	int			i, j;
	int			start, end;

	if ( !tr.registered ) {
		return;
	}
	R_SyncRenderThread();

	// we definately want to sync every frame for the cinematics
	//qglFinish();

	start = end = 0;
	if ( r_speeds->integer ) {
		start = ri.Milliseconds();
	}

	// make sure rows and cols are powers of 2
	for ( i = 0 ; ( 1 << i ) < cols ; i++ ) {
	}
	for ( j = 0 ; ( 1 << j ) < rows ; j++ ) {
	}
	if ( ( 1 << i ) != cols || ( 1 << j ) != rows) {
		ri.Error (ERR_DROP, "Draw_StretchRaw: size not a power of 2: %i by %i", cols, rows);
	}

	GX_Bind( tr.scratchImage[client] );

	// if the scratchImage isn't in the format we want, specify it as a new texture
	if ( cols != tr.scratchImage[client]->width || rows != tr.scratchImage[client]->height ) {
		tr.scratchImage[client]->width = tr.scratchImage[client]->uploadWidth = cols;
		tr.scratchImage[client]->height = tr.scratchImage[client]->uploadHeight = rows;
		GX_LoadAndBind( (void*)data, cols * rows * 4, cols, rows, GX_TF_RGBA8 );
		int texnum = glState.currenttextures[glState.currenttmu];
		qgxInitTexObjFilterMode( &gxtexobjs[texnum].texobj, GX_LINEAR, GX_LINEAR );
		qgxInitTexObjWrapMode( &gxtexobjs[texnum].texobj, GX_CLAMP, GX_CLAMP );
	} else {
		if (dirty) {
			// otherwise, just subimage upload it so that drivers can tell we are going to be changing
			// it and don't try and do a texture compression
			GX_LoadSubAndBind( (void*)data, 0, 0, cols, rows, GX_TF_RGBA8 );
		}
	}

	if ( r_speeds->integer ) {
		end = ri.Milliseconds();
		ri.Printf( PRINT_ALL, "qglTexSubImage2D %i, %i: %i msec\n", cols, rows, end - start );
	}

	RB_SetGL2D();

	float l = tr.identityLight;
	if(l < 0.0) 
		l = 0.0;
	if(l > 1.0) 
		l = 1.0;
	gxu_cur_r = l * 255.0;
	gxu_cur_g = l * 255.0;
	gxu_cur_b = l * 255.0;
	gxu_cur_a = 255;

	qgxBegin (GX_QUADS, gxu_cur_vertex_format, 4);
	qgxPosition3f32 (x, y, 0);
	qgxColor4u8 (gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
	qgxTexCoord2f32 ( 0.5f / cols,  0.5f / rows );
	qgxPosition3f32 (x+w, y, 0);
	qgxColor4u8 (gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
	qgxTexCoord2f32 ( ( cols - 0.5f ) / cols ,  0.5f / rows );
	qgxPosition3f32 (x+w, y+h, 0);
	qgxColor4u8 (gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
	qgxTexCoord2f32 ( ( cols - 0.5f ) / cols, ( rows - 0.5f ) / rows );
	qgxPosition3f32 (x, y+h, 0);
	qgxColor4u8 (gxu_cur_r, gxu_cur_g, gxu_cur_b, gxu_cur_a);
	qgxTexCoord2f32 ( 0.5f / cols, ( rows - 0.5f ) / rows );
	qgxEnd ();
}

void RE_UploadCinematic (int w, int h, int cols, int rows, const byte *data, int client, qboolean dirty) {

	GX_Bind( tr.scratchImage[client] );

	// if the scratchImage isn't in the format we want, specify it as a new texture
	if ( cols != tr.scratchImage[client]->width || rows != tr.scratchImage[client]->height ) {
		tr.scratchImage[client]->width = tr.scratchImage[client]->uploadWidth = cols;
		tr.scratchImage[client]->height = tr.scratchImage[client]->uploadHeight = rows;
		GX_LoadAndBind( (void*)data, cols * rows * 4, cols, rows, GX_TF_RGBA8 );
		int texnum = glState.currenttextures[glState.currenttmu];
		qgxInitTexObjFilterMode( &gxtexobjs[texnum].texobj, GX_LINEAR, GX_LINEAR );
		qgxInitTexObjWrapMode( &gxtexobjs[texnum].texobj, GX_CLAMP, GX_CLAMP );
	} else {
		if (dirty) {
			// otherwise, just subimage upload it so that drivers can tell we are going to be changing
			// it and don't try and do a texture compression
			GX_LoadSubAndBind( (void*)data, 0, 0, cols, rows, GX_TF_RGBA8 );
		}
	}
}


/*
=============
RB_SetColor

=============
*/
const void	*RB_SetColor( const void *data ) {
	const setColorCommand_t	*cmd;

	cmd = (const setColorCommand_t *)data;

	backEnd.color2D[0] = cmd->color[0] * 255;
	backEnd.color2D[1] = cmd->color[1] * 255;
	backEnd.color2D[2] = cmd->color[2] * 255;
	backEnd.color2D[3] = cmd->color[3] * 255;

	return (const void *)(cmd + 1);
}

/*
=============
RB_StretchPic
=============
*/
const void *RB_StretchPic ( const void *data ) {
	const stretchPicCommand_t	*cmd;
	shader_t *shader;
	int		numVerts, numIndexes;

	cmd = (const stretchPicCommand_t *)data;

	if ( !backEnd.projection2D ) {
		RB_SetGL2D();
	}

	shader = cmd->shader;
	if ( shader != tess.shader ) {
		if ( tess.numIndexes ) {
			RB_EndSurface();
		}
		backEnd.currentEntity = &backEnd.entity2D;
		RB_BeginSurface( shader, 0 );
	}

	RB_CHECKOVERFLOW( 4, 6 );
	numVerts = tess.numVertexes;
	numIndexes = tess.numIndexes;

	tess.numVertexes += 4;
	tess.numIndexes += 6;

	tess.indexes[ numIndexes ] = numVerts + 3;
	tess.indexes[ numIndexes + 1 ] = numVerts + 0;
	tess.indexes[ numIndexes + 2 ] = numVerts + 2;
	tess.indexes[ numIndexes + 3 ] = numVerts + 2;
	tess.indexes[ numIndexes + 4 ] = numVerts + 0;
	tess.indexes[ numIndexes + 5 ] = numVerts + 1;

	*(int *)tess.vertexColors[ numVerts ] =
		*(int *)tess.vertexColors[ numVerts + 1 ] =
		*(int *)tess.vertexColors[ numVerts + 2 ] =
		*(int *)tess.vertexColors[ numVerts + 3 ] = *(int *)backEnd.color2D;

	tess.xyz[ numVerts ][0] = cmd->x;
	tess.xyz[ numVerts ][1] = cmd->y;
	tess.xyz[ numVerts ][2] = 0;

	tess.texCoords[ numVerts ][0][0] = cmd->s1;
	tess.texCoords[ numVerts ][0][1] = cmd->t1;

	tess.xyz[ numVerts + 1 ][0] = cmd->x + cmd->w;
	tess.xyz[ numVerts + 1 ][1] = cmd->y;
	tess.xyz[ numVerts + 1 ][2] = 0;

	tess.texCoords[ numVerts + 1 ][0][0] = cmd->s2;
	tess.texCoords[ numVerts + 1 ][0][1] = cmd->t1;

	tess.xyz[ numVerts + 2 ][0] = cmd->x + cmd->w;
	tess.xyz[ numVerts + 2 ][1] = cmd->y + cmd->h;
	tess.xyz[ numVerts + 2 ][2] = 0;

	tess.texCoords[ numVerts + 2 ][0][0] = cmd->s2;
	tess.texCoords[ numVerts + 2 ][0][1] = cmd->t2;

	tess.xyz[ numVerts + 3 ][0] = cmd->x;
	tess.xyz[ numVerts + 3 ][1] = cmd->y + cmd->h;
	tess.xyz[ numVerts + 3 ][2] = 0;

	tess.texCoords[ numVerts + 3 ][0][0] = cmd->s1;
	tess.texCoords[ numVerts + 3 ][0][1] = cmd->t2;

	return (const void *)(cmd + 1);
}


/*
=============
RB_DrawSurfs

=============
*/
const void	*RB_DrawSurfs( const void *data ) {
	const drawSurfsCommand_t	*cmd;

	// finish any 2D drawing if needed
	if ( tess.numIndexes ) {
		RB_EndSurface();
	}

	cmd = (const drawSurfsCommand_t *)data;

	backEnd.refdef = cmd->refdef;
	backEnd.viewParms = cmd->viewParms;

	RB_RenderDrawSurfList( cmd->drawSurfs, cmd->numDrawSurfs );

	return (const void *)(cmd + 1);
}


/*
=============
RB_DrawBuffer

=============
*/
const void	*RB_DrawBuffer( const void *data ) {
	const drawBufferCommand_t	*cmd;

	cmd = (const drawBufferCommand_t *)data;

	//qglDrawBuffer( cmd->buffer );

	// clear screen for debugging
	if ( r_clear->integer ) {
		gxu_background_color.r = 255;
		gxu_background_color.g = 0;
		gxu_background_color.b = 127;
		gxu_background_color.a = 255;
		//qglClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	}

	return (const void *)(cmd + 1);
}

/*
===============
RB_ShowImages

Draw all the images to the screen, on top of whatever
was there.  This is used to test for texture thrashing.

Also called by RE_EndRegistration
===============
*/
void RB_ShowImages( void ) {
	// Temporarily deactivated, until we understand how to implement qglClear or qglFinish:
	/*
	int		i;
	image_t	*image;
	float	x, y, w, h;
	int		start, end;

	if ( !backEnd.projection2D ) {
		RB_SetGL2D();
	}

	qglClear( GL_COLOR_BUFFER_BIT );

	qglFinish();

	start = ri.Milliseconds();

	for ( i=0 ; i<tr.numImages ; i++ ) {
		image = tr.images[i];

		w = glConfig.vidWidth / 20;
		h = glConfig.vidHeight / 15;
		x = i % 20 * w;
		y = i / 20 * h;

		// show in proportional size in mode 2
		if ( r_showImages->integer == 2 ) {
			w *= image->uploadWidth / 512.0f;
			h *= image->uploadHeight / 512.0f;
		}

		GX_Bind( image );
		qglBegin (GL_QUADS);
		qglTexCoord2f( 0, 0 );
		qglVertex2f( x, y );
		qglTexCoord2f( 1, 0 );
		qglVertex2f( x + w, y );
		qglTexCoord2f( 1, 1 );
		qglVertex2f( x + w, y + h );
		qglTexCoord2f( 0, 1 );
		qglVertex2f( x, y + h );
		qglEnd();
	}

	qglFinish();

	end = ri.Milliseconds();
	ri.Printf( PRINT_ALL, "%i msec to draw all images\n", end - start );
	*/

}


/*
=============
RB_SwapBuffers

=============
*/
const void	*RB_SwapBuffers( const void *data ) {
	const swapBuffersCommand_t	*cmd;

	// finish any 2D drawing if needed
	if ( tess.numIndexes ) {
		RB_EndSurface();
	}

	// texture swapping test
	if ( r_showImages->integer ) {
		RB_ShowImages();
	}

	cmd = (const swapBuffersCommand_t *)data;

	// we measure overdraw by reading back the stencil buffer and
	// counting up the number of increments that have happened
	if ( r_measureOverdraw->integer ) {
		// Stencil buffer unavailable. Deactivating:
		/*
		int i;
		long sum = 0;
		unsigned char *stencilReadback;

		stencilReadback = ri.Hunk_AllocateTempMemory( glConfig.vidWidth * glConfig.vidHeight );
		qglReadPixels( 0, 0, glConfig.vidWidth, glConfig.vidHeight, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, stencilReadback );

		for ( i = 0; i < glConfig.vidWidth * glConfig.vidHeight; i++ ) {
			sum += stencilReadback[i];
		}

		backEnd.pc.c_overDraw += sum;
		ri.Hunk_FreeTempMemory( stencilReadback );
		*/
	}


	if ( !glState.finishCalled ) {
		//qglFinish();
	}

	GLimp_LogComment( "***************** RB_SwapBuffers *****************\n\n\n" );

	GLimp_EndFrame();

	backEnd.projection2D = qfalse;

	return (const void *)(cmd + 1);
}

/*
====================
RB_ExecuteRenderCommands

This function will be called synchronously if running without
smp extensions, or asynchronously by another thread.
====================
*/
void RB_ExecuteRenderCommands( const void *data ) {
	int		t1, t2;

	t1 = ri.Milliseconds ();

	if ( !r_smp->integer || data == backEndData[0]->commands.cmds ) {
		backEnd.smpFrame = 0;
	} else {
		backEnd.smpFrame = 1;
	}

	while ( 1 ) {
		switch ( *(const int *)data ) {
		case RC_SET_COLOR:
			data = RB_SetColor( data );
			break;
		case RC_STRETCH_PIC:
			data = RB_StretchPic( data );
			break;
		case RC_DRAW_SURFS:
			data = RB_DrawSurfs( data );
			break;
		case RC_DRAW_BUFFER:
			data = RB_DrawBuffer( data );
			break;
		case RC_SWAP_BUFFERS:
			data = RB_SwapBuffers( data );
			break;
		case RC_SCREENSHOT:
			data = RB_TakeScreenshotCmd( data );
			break;

		case RC_END_OF_LIST:
		default:
			// stop rendering on this thread
			t2 = ri.Milliseconds ();
			backEnd.pc.msec = t2 - t1;
			return;
		}
	}

}


/*
================
RB_RenderThread
================
*/
void RB_RenderThread( void ) {
	const void	*data;

	// wait for either a rendering command or a quit command
	while ( 1 ) {
		// sleep until we have work to do
		data = GLimp_RendererSleep();

		if ( !data ) {
			return;	// all done, renderer is shutting down
		}

		renderThreadActive = qtrue;

		RB_ExecuteRenderCommands( data );

		renderThreadActive = qfalse;
	}
}

