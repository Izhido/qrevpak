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
/*
** REVOL_QGX.C
**
** This file, originally based on linux_qgl.c, implements the bindings 
** of GX/gu/GXU functions to the new qgx/qgu (originally qgl) function 
** pointers, mapping the GX hardware functions of the Wii hardware
** to the engine, while at the same time supporting logging as it was
** defined in the original code.
** When doing a port of Quake3, using this particular code, you must 
** implement the following two functions:
**
** QGL_Init() - loads libraries, assigns function pointers, etc.
** QGL_Shutdown() - unloads libraries, NULLs function pointers
*/

#include "../renderer_gx/tr_local.h"
#include "revol_glw.h"
#include "gxutils.h"

void ( APIENTRY * qguMtxConcat )(Mtx a, Mtx b, Mtx ab);
void ( APIENTRY * qguMtxCopy )(Mtx src, Mtx dst);
void ( APIENTRY * qguMtxIdentity )(Mtx mt);
void ( APIENTRY * qguMtxRotAxisDeg )(Mtx mt, guVector* axis, f32 deg);
void ( APIENTRY * qguMtxScale) (Mtx mt, f32 xS, f32 yS, f32 zS);
void ( APIENTRY * qguMtxTrans )(Mtx mt, f32 xT, f32 yT, f32 zT);
void ( APIENTRY * qguOrtho )(Mtx44 mt, f32 t, f32 b, f32 l, f32 r, f32 n, f32 f);
void ( APIENTRY * qguPerspective )(Mtx44 mt, f32 fovy, f32 aspect, f32 n, f32 f);
void ( APIENTRY * qguSetTevOpAdd )(u8 stage);
void ( APIENTRY * qgxBegin )(u8 primitve, u8 vtxfmt, u16 vtxcnt);
void ( APIENTRY * qgxColor4u8 )(u8 r, u8 g, u8 b, u8 a);
void ( APIENTRY * qgxDisableTexStage1 )(void);
void ( APIENTRY * qgxDisableTexture )(void);
void ( APIENTRY * qgxEnableTexStage1 )(void);
void ( APIENTRY * qgxEnableTexture )(void);
void ( APIENTRY * qgxEnd )(void);
void ( APIENTRY * qgxInitTexObj )(GXTexObj *obj, void *img_ptr, u16 wd, u16 ht, u8 fmt, u8 wrap_s, u8 wrap_t, u8 mipmap);
void ( APIENTRY * qgxInitTexObjCI )(GXTexObj *obj, void *img_ptr, u16 wd, u16 ht, u8 fmt, u8 wrap_s, u8 wrap_t, u8 mipmap, u32 tlut_name);
void ( APIENTRY * qgxInitTexObjFilterMode )(GXTexObj *obj, u8 minfilt, u8 magfilt);
void ( APIENTRY * qgxInvalidateTexAll )(void);
void ( APIENTRY * qgxInvVtxCache )(void);
void ( APIENTRY * qgxLoadPosMtxImm )(Mtx mt, u32 pnidx);
void ( APIENTRY * qgxLoadProjectionMtx )(Mtx44 mt, u8 type);
void ( APIENTRY * qgxLoadTexObj )(GXTexObj *obj, u8 mapid);
void ( APIENTRY * qgxPosition3f32 )(f32 x, f32 y, f32 z);
void ( APIENTRY * qgxSetAlphaCompare )(u8 comp0, u8 ref0, u8 aop, u8 comp1, u8 ref1);
void ( APIENTRY * qgxSetArray )(u32 attr, void *ptr, u8 stride);
void ( APIENTRY * qgxSetBlendMode )(u8 type, u8 src_fact, u8 dst_fact, u8 op);
void ( APIENTRY * qgxSetCullMode )(u8 mode);
void ( APIENTRY * qgxSetScissor )(u32 xOrigin, u32 yOrigin, u32 wd, u32 ht);
void ( APIENTRY * qgxSetTevOp )(u8 tevstage, u8 mode);
void ( APIENTRY * qgxSetViewport )(f32 xOrig, f32 yOrig, f32 wd, f32 ht, f32 nearZ, f32 farZ);
void ( APIENTRY * qgxSetVtxDesc )(u8 attr, u8 type);
void ( APIENTRY * qgxSetZMode )(u8 enable, u8 func, u8 update_enable);
void ( APIENTRY * qgxTexCoord2f32 )(f32 s, f32 t);

void ( APIENTRY * qgxInitTlutObj )(GXTlutObj *obj, void *lut, u8 fmt, u16 entries);
void ( APIENTRY * qgxLoadTlut)(GXTlutObj *obj, u32 tlut_name);

void ( APIENTRY * qgxSetPointSize)(u8 width, u8 fmt);

static void ( APIENTRY * dllBegin )(u8 primitve, u8 vtxfmt, u16 vtxcnt);
static void ( APIENTRY * dllColor4u8 )(u8 r, u8 g, u8 b, u8 a);
static void ( APIENTRY * dllDisableTexStage1 )(void);
static void ( APIENTRY * dllDisableTexture )(void);
static void ( APIENTRY * dllEnableTexStage1 )(void);
static void ( APIENTRY * dllEnableTexture )(void);
static void ( APIENTRY * dllEnd )(void);
static void ( APIENTRY * dllInitTexObj )(GXTexObj *obj, void *img_ptr, u16 wd, u16 ht, u8 fmt, u8 wrap_s, u8 wrap_t, u8 mipmap);
static void ( APIENTRY * dllInitTexObjCI )(GXTexObj *obj, void *img_ptr, u16 wd, u16 ht, u8 fmt, u8 wrap_s, u8 wrap_t, u8 mipmap, u32 tlut_name);
static void ( APIENTRY * dllInitTexObjFilterMode )(GXTexObj *obj, u8 minfilt, u8 magfilt);
static void ( APIENTRY * dllInvalidateTexAll )(void);
static void ( APIENTRY * dllInvVtxCache )(void);
static void ( APIENTRY * dllLoadPosMtxImm )(Mtx mt, u32 pnidx);
static void ( APIENTRY * dllLoadProjectionMtx )(Mtx44 mt, u8 type);
static void ( APIENTRY * dllLoadTexObj )(GXTexObj *obj, u8 mapid);
static void ( APIENTRY * dllMtxConcat )(Mtx a, Mtx b, Mtx ab);
static void ( APIENTRY * dllMtxCopy )(Mtx src, Mtx dst);
static void ( APIENTRY * dllMtxIdentity )(Mtx mt);
static void ( APIENTRY * dllMtxRotAxisDeg )(Mtx mt, guVector* axis, f32 deg);
static void ( APIENTRY * dllMtxScale) (Mtx mt, f32 xS, f32 yS, f32 zS);
static void ( APIENTRY * dllMtxTrans )(Mtx mt, f32 xT, f32 yT, f32 zT);
static void ( APIENTRY * dllOrtho )(Mtx44 mt, f32 t, f32 b, f32 l, f32 r, f32 n, f32 f);
static void ( APIENTRY * dllPerspective )(Mtx44 mt, f32 fovy, f32 aspect, f32 n, f32 f);
static void ( APIENTRY * dllPosition3f32 )(f32 x, f32 y, f32 z);
static void ( APIENTRY * dllSetAlphaCompare )(u8 comp0, u8 ref0, u8 aop, u8 comp1, u8 ref1);
static void ( APIENTRY * dllSetArray )(u32 attr, void *ptr, u8 stride);
static void ( APIENTRY * dllSetBlendMode )(u8 type, u8 src_fact, u8 dst_fact, u8 op);
static void ( APIENTRY * dllSetCullMode )(u8 mode);
static void ( APIENTRY * dllSetScissor )(u32 xOrigin, u32 yOrigin, u32 wd, u32 ht);
static void ( APIENTRY * dllSetTevOp )(u8 tevstage, u8 mode);
static void ( APIENTRY * dllSetTevOpAdd )(u8 stage);
static void ( APIENTRY * dllSetViewport )(f32 xOrig, f32 yOrig, f32 wd, f32 ht, f32 nearZ, f32 farZ);
static void ( APIENTRY * dllSetVtxDesc )(u8 attr, u8 type);
static void ( APIENTRY * dllSetZMode )(u8 enable, u8 func, u8 update_enable);
static void ( APIENTRY * dllTexCoord2f32 )(f32 s, f32 t);

static void APIENTRY logBegin(u8 primitve, u8 vtxfmt, u16 vtxcnt)
{
	fprintf( glw_state.log_fp, "GX_Begin( 0x%x, 0x%x, %u)\n", primitve, vtxfmt, vtxcnt );
	dllBegin( primitve, vtxfmt, vtxcnt );
}

#define SIG( x ) fprintf( glw_state.log_fp, x "\n" )

static void APIENTRY logColor4u8(u8 r, u8 g, u8 b, u8 a)
{
	SIG( "GX_Color4u8" );
	dllColor4u8( r, g, b, a );
}

static void APIENTRY logDisableTexStage1(void)
{
	SIG( "GXU_DisableTexStage1" );
	dllDisableTexStage1();
}

static void APIENTRY logDisableTexture(void)
{
	SIG( "GXU_DisableTexture" );
	dllDisableTexture();
}

static void APIENTRY logEnableTexStage1(void)
{
	SIG( "GXU_EnableTexStage1" );
	dllEnableTexStage1();
}

static void APIENTRY logEnableTexture(void)
{
	SIG( "GXU_EnableTexture" );
	dllEnableTexture();
}

static void APIENTRY logEnd(void)
{
	SIG( "GX_End" );
	dllEnd();
}

static void APIENTRY logInitTexObj(GXTexObj *obj, void *img_ptr, u16 wd, u16 ht, u8 fmt, u8 wrap_s, u8 wrap_t, u8 mipmap)
{
	SIG( "GX_InitTexObj" );
	dllInitTexObj(obj, img_ptr, wd, ht, fmt, wrap_s, wrap_t, mipmap);
}

static void APIENTRY logInitTexObjCI(GXTexObj *obj, void *img_ptr, u16 wd, u16 ht, u8 fmt, u8 wrap_s, u8 wrap_t, u8 mipmap, u32 tlut_name)
{
	SIG( "GX_InitTexObjCI" );
	dllInitTexObjCI(obj, img_ptr, wd, ht, fmt, wrap_s, wrap_t, mipmap, tlut_name);
}

static void APIENTRY logInitTexObjFilterMode(GXTexObj *obj,u8 minfilt,u8 magfilt)
{
	SIG( "GX_InitTexObjFilterMode" );
	dllInitTexObjFilterMode(obj, minfilt, magfilt);
}

static void APIENTRY logInvalidateTexAll(void)
{
	SIG( "GX_InvalidateTexAll" );
	dllInvalidateTexAll();
}

static void APIENTRY logInvVtxCache(void)
{
	SIG( "GX_InvVtxCache" );
	dllInvVtxCache();
}

static void APIENTRY logLoadPosMtxImm(Mtx mt, u32 pnidx)
{
	SIG( "GX_LoadPosMtxImm" );
	dllLoadPosMtxImm( mt, pnidx );
}

static void logLoadProjectionMtx(Mtx44 mt, u8 type)
{
	SIG( "GX_LoadProjectionMtx" );
	dllLoadProjectionMtx( mt, type );
}

static void APIENTRY logLoadTexObj(GXTexObj *obj, u8 mapid)
{
	fprintf( glw_state.log_fp, "GX_LoadTexObj( 0x%x, %u )\n", (unsigned int)obj, mapid );
	dllLoadTexObj( obj, mapid );
}

static void APIENTRY logMtxConcat(Mtx a, Mtx b, Mtx ab)
{
	SIG( "guMtxConcat" );
	dllMtxConcat( a, b, ab );
}

static void APIENTRY logMtxCopy(Mtx src, Mtx dst)
{
	SIG( "guMtxCopy" );
	dllMtxCopy( src, dst );
}

static void APIENTRY logMtxIdentity(Mtx mt)
{
	SIG( "guMtxIdentity" );
	dllMtxIdentity( mt );
}

static void APIENTRY logMtxScale(Mtx mt, f32 xS, f32 yS, f32 zS)
{
	SIG( "guMtxScale" );
	dllMtxScale( mt, xS, yS, zS );
}

static void APIENTRY logMtxTrans(Mtx mt, f32 xT, f32 yT, f32 zT)
{
	SIG( "guMtxTrans" );
	dllMtxTrans( mt, xT, yT, zT );
}

static void APIENTRY logMtxRotAxisDeg(Mtx mt, guVector* axis, f32 deg)
{
	SIG( "guMtxRotAxisDeg" );
	dllMtxRotAxisDeg( mt, axis, deg );
}

static void APIENTRY logOrtho(Mtx44 mt, f32 t, f32 b, f32 l, f32 r, f32 n, f32 f)
{
	SIG( "guOrtho" );
	dllOrtho( mt, t, b, l, r, n, f );
}

static void APIENTRY logPerspective(Mtx44 mt, f32 fovy, f32 aspect, f32 n, f32 f)
{
	SIG( "guPerspective" );
	dllPerspective( mt, fovy, aspect, n, f );
}

static void APIENTRY logPosition3f32(f32 x, f32 y, f32 z)
{
	SIG( "GX_Position3f32" );
	dllPosition3f32( x, y, z );
}

static void APIENTRY logSetArray(u32 attr, void *ptr, u8 stride)
{
	fprintf( glw_state.log_fp, "GX_SetArray( 0x%x, 0x%x, 0x%x )\n", attr, (int)ptr, stride );
	dllSetArray( attr, ptr, stride );
}
static void APIENTRY logSetAlphaCompare(u8 comp0, u8 ref0, u8 aop, u8 comp1, u8 ref1)
{
	fprintf( glw_state.log_fp, "GX_SetAlphaCompare( 0x%x, 0x%x, 0x%x, 0x%x, 0x%x )\n", comp0, ref0, aop, comp1, ref1 );
	dllSetAlphaCompare( comp0, ref0, aop, comp1, ref1 );
}
static void APIENTRY logSetBlendMode(u8 type, u8 src_fact, u8 dst_fact, u8 op)
{
	SIG( "GX_SetBlendMode" );
	dllSetBlendMode( type, src_fact, dst_fact, op );
}
static void APIENTRY logSetCullMode(u8 mode)
{
	SIG( "GX_SetCullMode" );
	dllSetCullMode( mode );
}
static void APIENTRY logSetScissor(u32 xOrigin, u32 yOrigin, u32 wd, u32 ht)
{
	SIG( "GX_SetScissor" );
	dllSetScissor( xOrigin, yOrigin, wd, ht );
}
static void APIENTRY logSetTevOp(u8 tevstage, u8 mode)
{
	fprintf( glw_state.log_fp, "GX_SetTevOp( 0x%x, 0x%x )\n", tevstage, mode );
	dllSetTevOp( tevstage, mode );
}
static void APIENTRY logSetTevOpAdd(u8 stage)
{
	fprintf( glw_state.log_fp, "GXU_SetTevOpAdd( 0x%x )\n", stage );
	dllSetTevOpAdd( stage );
}
static void APIENTRY logSetViewport(f32 xOrig, f32 yOrig, f32 wd, f32 ht, f32 nearZ, f32 farZ)
{
	SIG( "GX_SetViewport" );
	dllSetViewport( xOrig, yOrig, wd, ht, nearZ, farZ );
}
static void APIENTRY logSetVtxDesc(u8 attr, u8 type)
{
	SIG( "GX_SetVtxDesc" );
	dllSetVtxDesc( attr, type );
}
static void APIENTRY logSetZMode(u8 enable, u8 func, u8 update_enable)
{
	SIG( "GX_SetZMode" );
	dllSetZMode( enable, func, update_enable );
}

static void APIENTRY logTexCoord2f32(f32 s, f32 t)
{
	SIG( "GX_TexCoord2f32" );
	dllTexCoord2f32( s, t );
}

/*
** QGL_Shutdown
**
** Nulls out all the proc pointers, signaling the end of usage of this driver.
*/
void QGL_Shutdown( void )
{
	qguMtxConcat                 = NULL;
	qguMtxCopy                   = NULL;
	qguMtxIdentity               = NULL;
	qguMtxRotAxisDeg             = NULL;
	qguMtxScale                  = NULL;
	qguMtxTrans                  = NULL;
	qguOrtho                     = NULL;
	qguPerspective               = NULL;
	qguSetTevOpAdd               = NULL;
	qgxBegin                     = NULL;
	qgxColor4u8                  = NULL;
	qgxDisableTexStage1          = NULL;
	qgxDisableTexture            = NULL;
	qgxEnableTexStage1           = NULL;
	qgxEnableTexture             = NULL;
	qgxEnd                       = NULL;
	qgxInitTexObj                = NULL;
	qgxInitTexObjCI              = NULL;
	qgxInitTexObjFilterMode      = NULL;
	qgxInvalidateTexAll          = NULL;
	qgxInvVtxCache               = NULL;
	qgxLoadPosMtxImm             = NULL;
	qgxLoadProjectionMtx         = NULL;
	qgxLoadTexObj                = NULL;
	qgxPosition3f32              = NULL;
	qgxSetAlphaCompare           = NULL;
	qgxSetArray                  = NULL;
	qgxSetBlendMode              = NULL;
	qgxSetCullMode               = NULL;
	qgxSetScissor                = NULL;
	qgxSetTevOp                  = NULL;
	qgxSetViewport               = NULL;
	qgxSetVtxDesc                = NULL;
	qgxSetZMode                  = NULL;
	qgxTexCoord2f32              = NULL;

	qgxInitTlutObj               = NULL;
	qgxLoadTlut                  = NULL;
}

/*
** QGL_Init
**
** This is responsible for binding our qgx/qgu function pointers to 
** the GX layer.
** 
*/

qboolean QGL_Init( const char *dllname )
{
	qguMtxConcat                 = dllMtxConcat = guMtxConcat;
	qguMtxCopy                   = dllMtxCopy = guMtxCopy;
	qguMtxIdentity               = dllMtxIdentity = guMtxIdentity;
	qguMtxRotAxisDeg             = dllMtxRotAxisDeg = GXU_CallguMtxRotAxisDeg;
	qguMtxScale                  = dllMtxScale = guMtxScale;
	qguOrtho                     = dllOrtho = guOrtho;
	qguPerspective               = dllPerspective = guPerspective;
	qguSetTevOpAdd               = dllSetTevOpAdd = GXU_SetTevOpAdd;
	qguMtxTrans                  = dllMtxTrans = guMtxTrans;
	qgxBegin                     = dllBegin = GX_Begin;
	qgxColor4u8                  = dllColor4u8 = GX_Color4u8;
	qgxDisableTexStage1          = dllDisableTexStage1 = GXU_DisableTexStage1;
	qgxDisableTexture            = dllDisableTexture = GXU_DisableTexture;
	qgxEnableTexStage1           = 	dllEnableTexStage1           = GXU_EnableTexStage1;
	qgxEnableTexture             = 	dllEnableTexture             = GXU_EnableTexture;
	qgxEnd                       = 	dllEnd                       = GX_End;
	qgxInitTexObj                =  dllInitTexObj                = GX_InitTexObj;
	qgxInitTexObjCI              =  dllInitTexObjCI              = GX_InitTexObjCI;
	qgxInitTexObjFilterMode      =  dllInitTexObjFilterMode      = GX_InitTexObjFilterMode;
	qgxInvalidateTexAll          =  dllInvalidateTexAll          = GX_InvalidateTexAll;
	qgxInvVtxCache               =  dllInvVtxCache               = GX_InvVtxCache;
	qgxLoadPosMtxImm             =  dllLoadPosMtxImm             = GX_LoadPosMtxImm;
	qgxLoadProjectionMtx         =  dllLoadProjectionMtx         = GX_LoadProjectionMtx;
	qgxLoadTexObj                =  dllLoadTexObj                = GX_LoadTexObj;
	qgxPosition3f32              = 	dllPosition3f32              = GX_Position3f32;
	qgxSetAlphaCompare           =  dllSetAlphaCompare           = GX_SetAlphaCompare;
	qgxSetArray                  =  dllSetArray                  = GX_SetArray;
	qgxSetBlendMode              = 	dllSetBlendMode              = GX_SetBlendMode;
	qgxSetCullMode               =  dllSetCullMode               = GX_SetCullMode;
	qgxSetScissor                =  dllSetScissor                = GX_SetScissor;
	qgxSetTevOp                  = 	dllSetTevOp                  = GX_SetTevOp;
	qgxSetViewport               = 	dllSetViewport               = GX_SetViewport;
	qgxSetVtxDesc                = 	dllSetVtxDesc                = GX_SetVtxDesc;
	qgxSetZMode                  = 	dllSetZMode                  = GX_SetZMode;
	qgxTexCoord2f32              = 	dllTexCoord2f32              = GX_TexCoord2f32;

	qgxSetPointSize = 0;
	qgxInitTlutObj = 0;
	qgxLoadTlut = 0;

	return qtrue;
}

void QGL_EnableLogging( qboolean enable ) {
  // bk001205 - fixed for new countdown
  static qboolean isEnabled = qfalse; // init
  
  // return if we're already active
  if ( isEnabled && enable ) {
    // decrement log counter and stop if it has reached 0
    ri.Cvar_Set( "r_logFile", va("%d", r_logFile->integer - 1 ) );
    if ( r_logFile->integer ) {
      return;
    }
    enable = qfalse;
  }

  // return if we're already disabled
  if ( !enable && !isEnabled )
    return;

  isEnabled = enable;

  // bk001205 - old code starts here
  if ( enable ) {
    if ( !glw_state.log_fp ) {
      struct tm *newtime;
      time_t aclock;
      char buffer[1024];
      cvar_t	*basedir;
      
      time( &aclock );
      newtime = localtime( &aclock );
      
      asctime( newtime );
      
      basedir = ri.Cvar_Get( "fs_basepath", "", 0 ); // FIXME: userdir?
      assert(basedir);
      Com_sprintf( buffer, sizeof(buffer), "%s/gl.log", basedir->string ); 
      glw_state.log_fp = fopen( buffer, "wt" );
      assert(glw_state.log_fp);
      ri.Printf(PRINT_ALL, "QGL_EnableLogging(%d): writing %s\n", r_logFile->integer, buffer );

      fprintf( glw_state.log_fp, "%s\n", asctime( newtime ) );
    }

		qguMtxConcat                 = logMtxConcat;
		qguMtxCopy                   = logMtxCopy;
		qguMtxIdentity               = logMtxIdentity;
		qguMtxRotAxisDeg             = logMtxRotAxisDeg;
		qguMtxScale                  = logMtxScale;
		qguMtxTrans                  = logMtxTrans;
		qguOrtho                     = logOrtho;
		qguPerspective               = logPerspective;
		qguSetTevOpAdd               = logSetTevOpAdd;
		qgxBegin                     = logBegin;
		qgxColor4u8                  = logColor4u8;
		qgxDisableTexStage1          = logDisableTexStage1 ;
		qgxDisableTexture            = logDisableTexture ;
		qgxEnableTexStage1           = 	logEnableTexStage1           ;
		qgxEnableTexture             = 	logEnableTexture             ;
		qgxEnd                       = 	logEnd                       ;
		qgxInitTexObj                =  logInitTexObj                ;
		qgxInitTexObjCI              =  logInitTexObjCI              ;
		qgxInitTexObjFilterMode      =  logInitTexObjFilterMode      ;
		qgxInvalidateTexAll          =  logInvalidateTexAll          ;
		qgxInvVtxCache               =  logInvVtxCache               ;
		qgxLoadPosMtxImm             =  logLoadPosMtxImm             ;
		qgxLoadProjectionMtx         =  logLoadProjectionMtx         ;
		qgxLoadTexObj                =  logLoadTexObj                ;
		qgxPosition3f32              = 	logPosition3f32              ;
		qgxSetAlphaCompare           =  logSetAlphaCompare           ;
		qgxSetArray                  =  logSetArray                  ;
		qgxSetBlendMode              = 	logSetBlendMode              ;
		qgxSetCullMode               = 	logSetCullMode               ;
		qgxSetScissor                = 	logSetScissor                ;
		qgxSetTevOp                  = 	logSetTevOp                  ;
		qgxSetViewport               = 	logSetViewport               ;
		qgxSetVtxDesc                = 	logSetVtxDesc                ;
		qgxSetZMode                  = 	logSetZMode                  ;
		qgxTexCoord2f32              = 	logTexCoord2f32              ;
	}
	else
	{
		qguMtxConcat                 = dllMtxConcat;
		qguMtxCopy                   = dllMtxCopy;
		qguMtxIdentity               = dllMtxIdentity;
		qguMtxRotAxisDeg             = dllMtxRotAxisDeg;
		qguMtxScale                  = dllMtxScale;
		qguMtxTrans                  = dllMtxTrans;
		qguOrtho                     = dllOrtho;
		qguPerspective               = dllPerspective;
		qguSetTevOpAdd               = dllSetTevOpAdd;
		qgxBegin                     = dllBegin;
		qgxColor4u8                  = dllColor4u8;
		qgxDisableTexStage1          = dllDisableTexStage1 ;
		qgxDisableTexture            = dllDisableTexture ;
		qgxEnableTexStage1           = 	dllEnableTexStage1           ;
		qgxEnd                       = 	dllEnd                       ;
		qgxInitTexObj                =  dllInitTexObj                ;
		qgxInitTexObjCI              =  dllInitTexObjCI              ;
		qgxInitTexObjFilterMode      =  dllInitTexObjFilterMode      ;
		qgxInvalidateTexAll          =  dllInvalidateTexAll          ;
		qgxInvVtxCache               =  dllInvVtxCache               ;
		qgxLoadPosMtxImm             =  dllLoadPosMtxImm             ;
		qgxLoadProjectionMtx         =  dllLoadProjectionMtx         ;
		qgxLoadTexObj                =  dllLoadTexObj                ;
		qgxPosition3f32              = 	dllPosition3f32              ;
		qgxSetAlphaCompare           =  dllSetAlphaCompare           ;
		qgxSetArray                  =  dllSetArray                  ;
		qgxSetBlendMode              = 	dllSetBlendMode              ;
		qgxSetCullMode               =  dllSetCullMode               ;
		qgxSetScissor                =  dllSetScissor                ;
		qgxSetTevOp                  = 	dllSetTevOp                  ;
		qgxSetViewport               = 	dllSetViewport               ;
		qgxSetVtxDesc                = 	dllSetVtxDesc                ;
		qgxSetZMode                  = 	dllSetZMode                  ;
		qgxTexCoord2f32              = 	dllTexCoord2f32              ;
	}
}


void GLimp_LogNewFrame( void )
{
	fprintf( glw_state.log_fp, "*** R_BeginFrame ***\n" );
}


