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
** QGX.H
** (Based on QGL.H)
*/

#ifndef __QGX_H__
#define __QGX_H__

#include <gccore.h>

#ifndef APIENTRY
#define APIENTRY
#endif

extern  void ( APIENTRY * qguMtxConcat )(Mtx a, Mtx b, Mtx ab);
extern  void ( APIENTRY * qguMtxCopy )(Mtx src, Mtx dst);
extern	void ( APIENTRY * qguMtxIdentity )(Mtx mt);
extern	void ( APIENTRY * qguMtxRotAxisDeg )(Mtx mt, guVector* axis, f32 deg);
extern	void ( APIENTRY * qguMtxScale) (Mtx mt, f32 xS, f32 yS, f32 zS);
extern  void ( APIENTRY * qguMtxTrans )(Mtx mt, f32 xT, f32 yT, f32 zT);
extern  void ( APIENTRY * qguOrtho )(Mtx44 mt, f32 t, f32 b, f32 l, f32 r, f32 n, f32 f);
extern  void ( APIENTRY * qguPerspective )(Mtx44 mt, f32 fovy, f32 aspect, f32 n, f32 f);
extern  void ( APIENTRY * qguSetTevOpAdd )(u8 stage);
extern  void ( APIENTRY * qgxBegin )(u8 primitve, u8 vtxfmt, u16 vtxcnt);
extern  void ( APIENTRY * qgxColor4u8 )(u8 r, u8 g, u8 b, u8 a);
extern  void ( APIENTRY * qgxDisableTexStage1 )(void);
extern  void ( APIENTRY * qgxDisableTexture )(void);
extern  void ( APIENTRY * qgxEnableTexStage1 )(void);
extern  void ( APIENTRY * qgxEnableTexture )(void);
extern  void ( APIENTRY * qgxEnd )(void);
extern  void ( APIENTRY * qgxInitTexObj )(GXTexObj *obj, void *img_ptr, u16 wd, u16 ht, u8 fmt, u8 wrap_s, u8 wrap_t, u8 mipmap);
extern  void ( APIENTRY * qgxInitTexObjCI )(GXTexObj *obj, void *img_ptr, u16 wd, u16 ht, u8 fmt, u8 wrap_s, u8 wrap_t, u8 mipmap, u32 tlut_name);
extern  void ( APIENTRY * qgxInitTexObjFilterMode )(GXTexObj *obj, u8 minfilt, u8 magfilt);
extern  void ( APIENTRY * qgxInitTexObjWrapMode )(GXTexObj *obj,u8 wrap_s,u8 wrap_t);
extern  void ( APIENTRY * qgxInvalidateTexAll )(void);
extern  void ( APIENTRY * qgxInvVtxCache )(void);
extern	void ( APIENTRY * qgxLoadPosMtxImm )(Mtx mt, u32 pnidx);
extern	void ( APIENTRY * qgxLoadProjectionMtx )(Mtx44 mt, u8 type);
extern  void ( APIENTRY * qgxLoadTexObj )(GXTexObj *obj, u8 mapid);
extern  void ( APIENTRY * qgxPosition3f32 )(f32 x, f32 y, f32 z);
extern  void ( APIENTRY * qgxSetAlphaCompare )(u8 comp0, u8 ref0, u8 aop, u8 comp1, u8 ref1);
extern  void ( APIENTRY * qgxSetArray )(u32 attr, void *ptr, u8 stride);
extern  void ( APIENTRY * qgxSetBlendMode )(u8 type, u8 src_fact, u8 dst_fact, u8 op);
extern  void ( APIENTRY * qgxSetCullMode )(u8 mode);
extern  void ( APIENTRY * qgxSetScissor )(u32 xOrigin, u32 yOrigin, u32 wd, u32 ht);
extern  void ( APIENTRY * qgxSetTevOp )(u8 tevstage, u8 mode);
extern  void ( APIENTRY * qgxSetViewport )(f32 xOrig, f32 yOrig, f32 wd, f32 ht, f32 nearZ, f32 farZ);
extern  void ( APIENTRY * qgxSetVtxDesc )(u8 attr, u8 type);
extern  void ( APIENTRY * qgxSetZMode )(u8 enable, u8 func, u8 update_enable);
extern  void ( APIENTRY * qgxTexCoord2f32 )(f32 s, f32 t);

extern	void ( APIENTRY * qgxSetPointSize)(u8 width, u8 fmt);

extern  void ( APIENTRY * qgxInitTlutObj )(GXTlutObj *obj, void *lut, u8 fmt, u16 entries);
extern	void ( APIENTRY * qgxLoadTlut)(GXTlutObj *obj, u32 tlut_name);

extern int GX_TEXTURE0, GX_TEXTURE1;

#endif
