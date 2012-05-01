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
/*
** qgl_revol.c
**
** This file implements the operating system binding of GL to QGL function
** pointers.  When doing a port of Quake2 you must implement the following
** two functions:
**
** QGL_Init() - loads libraries, assigns function pointers, etc.
** QGL_Shutdown() - unloads libraries, NULLs function pointers
*/
// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// Include only for the GL builds (part 1):
#ifdef GXIMP
// <<< FIX
#define QGL
#include "../ref_gx/gx_local.h"

static FILE *log_fp = NULL;

void ( APIENTRY * qguMtxConcat )(Mtx a, Mtx b, Mtx ab);
void ( APIENTRY * qguMtxCopy )(Mtx src, Mtx dst);
void ( APIENTRY * qguMtxIdentity )(Mtx mt);
void ( APIENTRY * qguMtxRotAxisDeg )(Mtx mt, guVector* axis, f32 deg);
void ( APIENTRY * qguMtxScale) (Mtx mt, f32 xS, f32 yS, f32 zS);
void ( APIENTRY * qguMtxTrans )(Mtx mt, f32 xT, f32 yT, f32 zT);
void ( APIENTRY * qguOrtho )(Mtx44 mt, f32 t, f32 b, f32 l, f32 r, f32 n, f32 f);
void ( APIENTRY * qguPerspective )(Mtx44 mt, f32 fovy, f32 aspect, f32 n, f32 f);
void ( APIENTRY * qglAccum )(GLenum op, GLfloat value);
GLboolean ( APIENTRY * qglAreTexturesResident )(GLsizei n, const GLuint *textures, GLboolean *residences);
void ( APIENTRY * qglArrayElement )(GLint i);
void ( APIENTRY * qgxBegin )(u8 primitve, u8 vtxfmt, u16 vtxcnt);
void ( APIENTRY * qglBitmap )(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap);
void ( APIENTRY * qglCallList )(GLuint list);
void ( APIENTRY * qglCallLists )(GLsizei n, GLenum type, const GLvoid *lists);
void ( APIENTRY * qglClear )(GLbitfield mask);
void ( APIENTRY * qglClearAccum )(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
void ( APIENTRY * qglClearColor )(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
void ( APIENTRY * qglClearDepth )(GLclampd depth);
void ( APIENTRY * qglClearIndex )(GLfloat c);
void ( APIENTRY * qglClearStencil )(GLint s);
void ( APIENTRY * qglClipPlane )(GLenum plane, const GLdouble *equation);
void ( APIENTRY * qgxColor4u8 )(u8 r, u8 g, u8 b, u8 a);
void ( APIENTRY * qglColorMask )(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
void ( APIENTRY * qglColorMaterial )(GLenum face, GLenum mode);
void ( APIENTRY * qglColorPointer )(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
void ( APIENTRY * qglCopyPixels )(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);
void ( APIENTRY * qglCopyTexImage1D )(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border);
void ( APIENTRY * qglCopyTexImage2D )(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
void ( APIENTRY * qglCopyTexSubImage1D )(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
void ( APIENTRY * qglCopyTexSubImage2D )(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
void ( APIENTRY * qglDeleteLists )(GLuint list, GLsizei range);
void ( APIENTRY * qglDeleteTextures )(GLsizei n, const GLuint *textures);
void ( APIENTRY * qglDisable )(GLenum cap);
void ( APIENTRY * qglDisableClientState )(GLenum array);
void ( APIENTRY * qgxDisableTexture )(void);
void ( APIENTRY * qglDrawArrays )(GLenum mode, GLint first, GLsizei count);
void ( APIENTRY * qglDrawBuffer )(GLenum mode);
void ( APIENTRY * qglDrawElements )(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
void ( APIENTRY * qglDrawPixels )(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
void ( APIENTRY * qglEdgeFlag )(GLboolean flag);
void ( APIENTRY * qglEdgeFlagPointer )(GLsizei stride, const GLvoid *pointer);
void ( APIENTRY * qglEdgeFlagv )(const GLboolean *flag);
void ( APIENTRY * qglEnable )(GLenum cap);
void ( APIENTRY * qglEnableClientState )(GLenum array);
void ( APIENTRY * qgxEnableTexture )(void);
void ( APIENTRY * qgxEnd )(void);
void ( APIENTRY * qglEndList )(void);
void ( APIENTRY * qglEvalCoord1d )(GLdouble u);
void ( APIENTRY * qglEvalCoord1dv )(const GLdouble *u);
void ( APIENTRY * qglEvalCoord1f )(GLfloat u);
void ( APIENTRY * qglEvalCoord1fv )(const GLfloat *u);
void ( APIENTRY * qglEvalCoord2d )(GLdouble u, GLdouble v);
void ( APIENTRY * qglEvalCoord2dv )(const GLdouble *u);
void ( APIENTRY * qglEvalCoord2f )(GLfloat u, GLfloat v);
void ( APIENTRY * qglEvalCoord2fv )(const GLfloat *u);
void ( APIENTRY * qglEvalMesh1 )(GLenum mode, GLint i1, GLint i2);
void ( APIENTRY * qglEvalMesh2 )(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2);
void ( APIENTRY * qglEvalPoint1 )(GLint i);
void ( APIENTRY * qglEvalPoint2 )(GLint i, GLint j);
void ( APIENTRY * qglFeedbackBuffer )(GLsizei size, GLenum type, GLfloat *buffer);
void ( APIENTRY * qglFinish )(void);
void ( APIENTRY * qglFlush )(void);
void ( APIENTRY * qglFogf )(GLenum pname, GLfloat param);
void ( APIENTRY * qglFogfv )(GLenum pname, const GLfloat *params);
void ( APIENTRY * qglFogi )(GLenum pname, GLint param);
void ( APIENTRY * qglFogiv )(GLenum pname, const GLint *params);
void ( APIENTRY * qglFrontFace )(GLenum mode);
GLuint ( APIENTRY * qglGenLists )(GLsizei range);
void ( APIENTRY * qglGenTextures )(GLsizei n, GLuint *textures);
void ( APIENTRY * qglGetBooleanv )(GLenum pname, GLboolean *params);
void ( APIENTRY * qglGetClipPlane )(GLenum plane, GLdouble *equation);
void ( APIENTRY * qglGetDoublev )(GLenum pname, GLdouble *params);
GLenum ( APIENTRY * qglGetError )(void);
void ( APIENTRY * qglGetIntegerv )(GLenum pname, GLint *params);
void ( APIENTRY * qglGetLightfv )(GLenum light, GLenum pname, GLfloat *params);
void ( APIENTRY * qglGetLightiv )(GLenum light, GLenum pname, GLint *params);
void ( APIENTRY * qglGetMapdv )(GLenum target, GLenum query, GLdouble *v);
void ( APIENTRY * qglGetMapfv )(GLenum target, GLenum query, GLfloat *v);
void ( APIENTRY * qglGetMapiv )(GLenum target, GLenum query, GLint *v);
void ( APIENTRY * qglGetMaterialfv )(GLenum face, GLenum pname, GLfloat *params);
void ( APIENTRY * qglGetMaterialiv )(GLenum face, GLenum pname, GLint *params);
void ( APIENTRY * qglGetPixelMapfv )(GLenum map, GLfloat *values);
void ( APIENTRY * qglGetPixelMapuiv )(GLenum map, GLuint *values);
void ( APIENTRY * qglGetPixelMapusv )(GLenum map, GLushort *values);
void ( APIENTRY * qglGetPointerv )(GLenum pname, GLvoid* *params);
void ( APIENTRY * qglGetPolygonStipple )(GLubyte *mask);
const GLubyte * ( APIENTRY * qglGetString )(GLenum name);
void ( APIENTRY * qglGetTexEnvfv )(GLenum target, GLenum pname, GLfloat *params);
void ( APIENTRY * qglGetTexEnviv )(GLenum target, GLenum pname, GLint *params);
void ( APIENTRY * qglGetTexGendv )(GLenum coord, GLenum pname, GLdouble *params);
void ( APIENTRY * qglGetTexGenfv )(GLenum coord, GLenum pname, GLfloat *params);
void ( APIENTRY * qglGetTexGeniv )(GLenum coord, GLenum pname, GLint *params);
void ( APIENTRY * qglGetTexImage )(GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels);
void ( APIENTRY * qglGetTexLevelParameterfv )(GLenum target, GLint level, GLenum pname, GLfloat *params);
void ( APIENTRY * qglGetTexLevelParameteriv )(GLenum target, GLint level, GLenum pname, GLint *params);
void ( APIENTRY * qglGetTexParameterfv )(GLenum target, GLenum pname, GLfloat *params);
void ( APIENTRY * qglGetTexParameteriv )(GLenum target, GLenum pname, GLint *params);
void ( APIENTRY * qglHint )(GLenum target, GLenum mode);
void ( APIENTRY * qglIndexMask )(GLuint mask);
void ( APIENTRY * qglIndexPointer )(GLenum type, GLsizei stride, const GLvoid *pointer);
void ( APIENTRY * qglIndexd )(GLdouble c);
void ( APIENTRY * qglIndexdv )(const GLdouble *c);
void ( APIENTRY * qglIndexf )(GLfloat c);
void ( APIENTRY * qglIndexfv )(const GLfloat *c);
void ( APIENTRY * qglIndexi )(GLint c);
void ( APIENTRY * qglIndexiv )(const GLint *c);
void ( APIENTRY * qglIndexs )(GLshort c);
void ( APIENTRY * qglIndexsv )(const GLshort *c);
void ( APIENTRY * qglIndexub )(GLubyte c);
void ( APIENTRY * qglIndexubv )(const GLubyte *c);
void ( APIENTRY * qglInitNames )(void);
void ( APIENTRY * qgxInitTexObj )(GXTexObj *obj, void *img_ptr, u16 wd, u16 ht, u8 fmt, u8 wrap_s, u8 wrap_t, u8 mipmap);
void ( APIENTRY * qgxInitTexObjFilterMode )(GXTexObj *obj, u8 minfilt, u8 magfilt);
void ( APIENTRY * qglInterleavedArrays )(GLenum format, GLsizei stride, const GLvoid *pointer);
void ( APIENTRY * qgxInvalidateTexAll )(void);
GLboolean ( APIENTRY * qglIsEnabled )(GLenum cap);
GLboolean ( APIENTRY * qglIsList )(GLuint list);
GLboolean ( APIENTRY * qglIsTexture )(GLuint texture);
void ( APIENTRY * qglLightModelf )(GLenum pname, GLfloat param);
void ( APIENTRY * qglLightModelfv )(GLenum pname, const GLfloat *params);
void ( APIENTRY * qglLightModeli )(GLenum pname, GLint param);
void ( APIENTRY * qglLightModeliv )(GLenum pname, const GLint *params);
void ( APIENTRY * qglLightf )(GLenum light, GLenum pname, GLfloat param);
void ( APIENTRY * qglLightfv )(GLenum light, GLenum pname, const GLfloat *params);
void ( APIENTRY * qglLighti )(GLenum light, GLenum pname, GLint param);
void ( APIENTRY * qglLightiv )(GLenum light, GLenum pname, const GLint *params);
void ( APIENTRY * qglLineStipple )(GLint factor, GLushort pattern);
void ( APIENTRY * qglLineWidth )(GLfloat width);
void ( APIENTRY * qglListBase )(GLuint base);
void ( APIENTRY * qglLoadName )(GLuint name);
void ( APIENTRY * qgxLoadPosMtxImm )(Mtx mt, u32 pnidx);
void ( APIENTRY * qgxLoadProjectionMtx )(Mtx44 mt, u8 type);
void ( APIENTRY * qgxLoadTexObj )(GXTexObj *obj, u8 mapid);
void ( APIENTRY * qglLogicOp )(GLenum opcode);
void ( APIENTRY * qglMap1d )(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points);
void ( APIENTRY * qglMap1f )(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points);
void ( APIENTRY * qglMap2d )(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points);
void ( APIENTRY * qglMap2f )(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points);
void ( APIENTRY * qglMapGrid1d )(GLint un, GLdouble u1, GLdouble u2);
void ( APIENTRY * qglMapGrid1f )(GLint un, GLfloat u1, GLfloat u2);
void ( APIENTRY * qglMapGrid2d )(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2);
void ( APIENTRY * qglMapGrid2f )(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2);
void ( APIENTRY * qglMaterialf )(GLenum face, GLenum pname, GLfloat param);
void ( APIENTRY * qglMaterialfv )(GLenum face, GLenum pname, const GLfloat *params);
void ( APIENTRY * qglMateriali )(GLenum face, GLenum pname, GLint param);
void ( APIENTRY * qglMaterialiv )(GLenum face, GLenum pname, const GLint *params);
void ( APIENTRY * qglNewList )(GLuint list, GLenum mode);
void ( APIENTRY * qglNormal3b )(GLbyte nx, GLbyte ny, GLbyte nz);
void ( APIENTRY * qglNormal3bv )(const GLbyte *v);
void ( APIENTRY * qglNormal3d )(GLdouble nx, GLdouble ny, GLdouble nz);
void ( APIENTRY * qglNormal3dv )(const GLdouble *v);
void ( APIENTRY * qglNormal3f )(GLfloat nx, GLfloat ny, GLfloat nz);
void ( APIENTRY * qglNormal3fv )(const GLfloat *v);
void ( APIENTRY * qglNormal3i )(GLint nx, GLint ny, GLint nz);
void ( APIENTRY * qglNormal3iv )(const GLint *v);
void ( APIENTRY * qglNormal3s )(GLshort nx, GLshort ny, GLshort nz);
void ( APIENTRY * qglNormal3sv )(const GLshort *v);
void ( APIENTRY * qglNormalPointer )(GLenum type, GLsizei stride, const GLvoid *pointer);
void ( APIENTRY * qglPassThrough )(GLfloat token);
void ( APIENTRY * qglPixelMapfv )(GLenum map, GLsizei mapsize, const GLfloat *values);
void ( APIENTRY * qglPixelMapuiv )(GLenum map, GLsizei mapsize, const GLuint *values);
void ( APIENTRY * qglPixelMapusv )(GLenum map, GLsizei mapsize, const GLushort *values);
void ( APIENTRY * qglPixelStoref )(GLenum pname, GLfloat param);
void ( APIENTRY * qglPixelStorei )(GLenum pname, GLint param);
void ( APIENTRY * qglPixelTransferf )(GLenum pname, GLfloat param);
void ( APIENTRY * qglPixelTransferi )(GLenum pname, GLint param);
void ( APIENTRY * qglPixelZoom )(GLfloat xfactor, GLfloat yfactor);
void ( APIENTRY * qglPointSize )(GLfloat size);
void ( APIENTRY * qglPolygonMode )(GLenum face, GLenum mode);
void ( APIENTRY * qglPolygonOffset )(GLfloat factor, GLfloat units);
void ( APIENTRY * qglPolygonStipple )(const GLubyte *mask);
void ( APIENTRY * qglPopAttrib )(void);
void ( APIENTRY * qglPopClientAttrib )(void);
void ( APIENTRY * qglPopName )(void);
void ( APIENTRY * qgxPosition3f32 )(f32 x, f32 y, f32 z);
void ( APIENTRY * qglPrioritizeTextures )(GLsizei n, const GLuint *textures, const GLclampf *priorities);
void ( APIENTRY * qglPushAttrib )(GLbitfield mask);
void ( APIENTRY * qglPushClientAttrib )(GLbitfield mask);
void ( APIENTRY * qglPushName )(GLuint name);
void ( APIENTRY * qglRasterPos2d )(GLdouble x, GLdouble y);
void ( APIENTRY * qglRasterPos2dv )(const GLdouble *v);
void ( APIENTRY * qglRasterPos2f )(GLfloat x, GLfloat y);
void ( APIENTRY * qglRasterPos2fv )(const GLfloat *v);
void ( APIENTRY * qglRasterPos2i )(GLint x, GLint y);
void ( APIENTRY * qglRasterPos2iv )(const GLint *v);
void ( APIENTRY * qglRasterPos2s )(GLshort x, GLshort y);
void ( APIENTRY * qglRasterPos2sv )(const GLshort *v);
void ( APIENTRY * qglRasterPos3d )(GLdouble x, GLdouble y, GLdouble z);
void ( APIENTRY * qglRasterPos3dv )(const GLdouble *v);
void ( APIENTRY * qglRasterPos3f )(GLfloat x, GLfloat y, GLfloat z);
void ( APIENTRY * qglRasterPos3fv )(const GLfloat *v);
void ( APIENTRY * qglRasterPos3i )(GLint x, GLint y, GLint z);
void ( APIENTRY * qglRasterPos3iv )(const GLint *v);
void ( APIENTRY * qglRasterPos3s )(GLshort x, GLshort y, GLshort z);
void ( APIENTRY * qglRasterPos3sv )(const GLshort *v);
void ( APIENTRY * qglRasterPos4d )(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
void ( APIENTRY * qglRasterPos4dv )(const GLdouble *v);
void ( APIENTRY * qglRasterPos4f )(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
void ( APIENTRY * qglRasterPos4fv )(const GLfloat *v);
void ( APIENTRY * qglRasterPos4i )(GLint x, GLint y, GLint z, GLint w);
void ( APIENTRY * qglRasterPos4iv )(const GLint *v);
void ( APIENTRY * qglRasterPos4s )(GLshort x, GLshort y, GLshort z, GLshort w);
void ( APIENTRY * qglRasterPos4sv )(const GLshort *v);
void ( APIENTRY * qglReadBuffer )(GLenum mode);
void ( APIENTRY * qglReadPixels )(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);
void ( APIENTRY * qglRectd )(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2);
void ( APIENTRY * qglRectdv )(const GLdouble *v1, const GLdouble *v2);
void ( APIENTRY * qglRectf )(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
void ( APIENTRY * qglRectfv )(const GLfloat *v1, const GLfloat *v2);
void ( APIENTRY * qglRecti )(GLint x1, GLint y1, GLint x2, GLint y2);
void ( APIENTRY * qglRectiv )(const GLint *v1, const GLint *v2);
void ( APIENTRY * qglRects )(GLshort x1, GLshort y1, GLshort x2, GLshort y2);
void ( APIENTRY * qglRectsv )(const GLshort *v1, const GLshort *v2);
GLint ( APIENTRY * qglRenderMode )(GLenum mode);
void ( APIENTRY * qglScissor )(GLint x, GLint y, GLsizei width, GLsizei height);
void ( APIENTRY * qglSelectBuffer )(GLsizei size, GLuint *buffer);
void ( APIENTRY * qgxSetAlphaCompare )(u8 comp0, u8 ref0, u8 aop, u8 comp1, u8 ref1);
void ( APIENTRY * qgxSetBlendMode )(u8 type, u8 src_fact, u8 dst_fact, u8 op);
void ( APIENTRY * qgxSetCullMode )(u8 mode);
void ( APIENTRY * qgxSetTevOp )(u8 tevstage, u8 mode);
void ( APIENTRY * qgxSetViewport )(f32 xOrig, f32 yOrig, f32 wd, f32 ht, f32 nearZ, f32 farZ);
void ( APIENTRY * qgxSetZMode )(u8 enable, u8 func, u8 update_enable);
void ( APIENTRY * qglShadeModel )(GLenum mode);
void ( APIENTRY * qglStencilFunc )(GLenum func, GLint ref, GLuint mask);
void ( APIENTRY * qglStencilMask )(GLuint mask);
void ( APIENTRY * qglStencilOp )(GLenum fail, GLenum zfail, GLenum zpass);
void ( APIENTRY * qgxTexCoord2f32 )(f32 s, f32 t);
void ( APIENTRY * qglTexCoordPointer )(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
void ( APIENTRY * qglTexGend )(GLenum coord, GLenum pname, GLdouble param);
void ( APIENTRY * qglTexGendv )(GLenum coord, GLenum pname, const GLdouble *params);
void ( APIENTRY * qglTexGenf )(GLenum coord, GLenum pname, GLfloat param);
void ( APIENTRY * qglTexGenfv )(GLenum coord, GLenum pname, const GLfloat *params);
void ( APIENTRY * qglTexGeni )(GLenum coord, GLenum pname, GLint param);
void ( APIENTRY * qglTexGeniv )(GLenum coord, GLenum pname, const GLint *params);
void ( APIENTRY * qglVertexPointer )(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);

void ( APIENTRY * qglLockArraysEXT)( int, int);
void ( APIENTRY * qglUnlockArraysEXT) ( void );

void ( APIENTRY * qglPointParameterfEXT)( GLenum param, GLfloat value );
void ( APIENTRY * qglPointParameterfvEXT)( GLenum param, const GLfloat *value );
void ( APIENTRY * qglColorTableEXT)( int, int, int, int, int, const void * );
void ( APIENTRY * qglSelectTextureSGIS)( GLenum );
void ( APIENTRY * qglMTexCoord2fSGIS)( GLenum, GLfloat, GLfloat );
void ( APIENTRY * qglActiveTextureARB) ( GLenum );
void ( APIENTRY * qglClientActiveTextureARB) ( GLenum );

static void ( APIENTRY * dllAccum )(GLenum op, GLfloat value);
GLboolean ( APIENTRY * dllAreTexturesResident )(GLsizei n, const GLuint *textures, GLboolean *residences);
static void ( APIENTRY * dllArrayElement )(GLint i);
static void ( APIENTRY * dllBegin )(u8 primitve, u8 vtxfmt, u16 vtxcnt);
static void ( APIENTRY * dllBitmap )(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap);
static void ( APIENTRY * dllCallList )(GLuint list);
static void ( APIENTRY * dllCallLists )(GLsizei n, GLenum type, const GLvoid *lists);
static void ( APIENTRY * dllClear )(GLbitfield mask);
static void ( APIENTRY * dllClearAccum )(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
static void ( APIENTRY * dllClearColor )(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
static void ( APIENTRY * dllClearDepth )(GLclampd depth);
static void ( APIENTRY * dllClearIndex )(GLfloat c);
static void ( APIENTRY * dllClearStencil )(GLint s);
static void ( APIENTRY * dllClipPlane )(GLenum plane, const GLdouble *equation);
static void ( APIENTRY * dllColor4u8 )(u8 r, u8 g, u8 b, u8 a);
static void ( APIENTRY * dllColorMask )(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
static void ( APIENTRY * dllColorMaterial )(GLenum face, GLenum mode);
static void ( APIENTRY * dllColorPointer )(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
static void ( APIENTRY * dllCopyPixels )(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);
static void ( APIENTRY * dllCopyTexImage1D )(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border);
static void ( APIENTRY * dllCopyTexImage2D )(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
static void ( APIENTRY * dllCopyTexSubImage1D )(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
static void ( APIENTRY * dllCopyTexSubImage2D )(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
static void ( APIENTRY * dllDeleteLists )(GLuint list, GLsizei range);
static void ( APIENTRY * dllDeleteTextures )(GLsizei n, const GLuint *textures);
static void ( APIENTRY * dllDisable )(GLenum cap);
static void ( APIENTRY * dllDisableClientState )(GLenum array);
static void ( APIENTRY * dllDisableTexture )(void);
static void ( APIENTRY * dllDrawArrays )(GLenum mode, GLint first, GLsizei count);
static void ( APIENTRY * dllDrawBuffer )(GLenum mode);
static void ( APIENTRY * dllDrawElements )(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
static void ( APIENTRY * dllDrawPixels )(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
static void ( APIENTRY * dllEdgeFlag )(GLboolean flag);
static void ( APIENTRY * dllEdgeFlagPointer )(GLsizei stride, const GLvoid *pointer);
static void ( APIENTRY * dllEdgeFlagv )(const GLboolean *flag);
static void ( APIENTRY * dllEnable )(GLenum cap);
static void ( APIENTRY * dllEnableClientState )(GLenum array);
static void ( APIENTRY * dllEnableTexture )(void);
static void ( APIENTRY * dllEnd )(void);
static void ( APIENTRY * dllEndList )(void);
static void ( APIENTRY * dllEvalCoord1d )(GLdouble u);
static void ( APIENTRY * dllEvalCoord1dv )(const GLdouble *u);
static void ( APIENTRY * dllEvalCoord1f )(GLfloat u);
static void ( APIENTRY * dllEvalCoord1fv )(const GLfloat *u);
static void ( APIENTRY * dllEvalCoord2d )(GLdouble u, GLdouble v);
static void ( APIENTRY * dllEvalCoord2dv )(const GLdouble *u);
static void ( APIENTRY * dllEvalCoord2f )(GLfloat u, GLfloat v);
static void ( APIENTRY * dllEvalCoord2fv )(const GLfloat *u);
static void ( APIENTRY * dllEvalMesh1 )(GLenum mode, GLint i1, GLint i2);
static void ( APIENTRY * dllEvalMesh2 )(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2);
static void ( APIENTRY * dllEvalPoint1 )(GLint i);
static void ( APIENTRY * dllEvalPoint2 )(GLint i, GLint j);
static void ( APIENTRY * dllFeedbackBuffer )(GLsizei size, GLenum type, GLfloat *buffer);
static void ( APIENTRY * dllFinish )(void);
static void ( APIENTRY * dllFlush )(void);
static void ( APIENTRY * dllFogf )(GLenum pname, GLfloat param);
static void ( APIENTRY * dllFogfv )(GLenum pname, const GLfloat *params);
static void ( APIENTRY * dllFogi )(GLenum pname, GLint param);
static void ( APIENTRY * dllFogiv )(GLenum pname, const GLint *params);
static void ( APIENTRY * dllFrontFace )(GLenum mode);
GLuint ( APIENTRY * dllGenLists )(GLsizei range);
static void ( APIENTRY * dllGenTextures )(GLsizei n, GLuint *textures);
static void ( APIENTRY * dllGetBooleanv )(GLenum pname, GLboolean *params);
static void ( APIENTRY * dllGetClipPlane )(GLenum plane, GLdouble *equation);
static void ( APIENTRY * dllGetDoublev )(GLenum pname, GLdouble *params);
GLenum ( APIENTRY * dllGetError )(void);
static void ( APIENTRY * dllGetIntegerv )(GLenum pname, GLint *params);
static void ( APIENTRY * dllGetLightfv )(GLenum light, GLenum pname, GLfloat *params);
static void ( APIENTRY * dllGetLightiv )(GLenum light, GLenum pname, GLint *params);
static void ( APIENTRY * dllGetMapdv )(GLenum target, GLenum query, GLdouble *v);
static void ( APIENTRY * dllGetMapfv )(GLenum target, GLenum query, GLfloat *v);
static void ( APIENTRY * dllGetMapiv )(GLenum target, GLenum query, GLint *v);
static void ( APIENTRY * dllGetMaterialfv )(GLenum face, GLenum pname, GLfloat *params);
static void ( APIENTRY * dllGetMaterialiv )(GLenum face, GLenum pname, GLint *params);
static void ( APIENTRY * dllGetPixelMapfv )(GLenum map, GLfloat *values);
static void ( APIENTRY * dllGetPixelMapuiv )(GLenum map, GLuint *values);
static void ( APIENTRY * dllGetPixelMapusv )(GLenum map, GLushort *values);
static void ( APIENTRY * dllGetPointerv )(GLenum pname, GLvoid* *params);
static void ( APIENTRY * dllGetPolygonStipple )(GLubyte *mask);
const GLubyte * ( APIENTRY * dllGetString )(GLenum name);
static void ( APIENTRY * dllGetTexEnvfv )(GLenum target, GLenum pname, GLfloat *params);
static void ( APIENTRY * dllGetTexEnviv )(GLenum target, GLenum pname, GLint *params);
static void ( APIENTRY * dllGetTexGendv )(GLenum coord, GLenum pname, GLdouble *params);
static void ( APIENTRY * dllGetTexGenfv )(GLenum coord, GLenum pname, GLfloat *params);
static void ( APIENTRY * dllGetTexGeniv )(GLenum coord, GLenum pname, GLint *params);
static void ( APIENTRY * dllGetTexImage )(GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels);
static void ( APIENTRY * dllGetTexLevelParameterfv )(GLenum target, GLint level, GLenum pname, GLfloat *params);
static void ( APIENTRY * dllGetTexLevelParameteriv )(GLenum target, GLint level, GLenum pname, GLint *params);
static void ( APIENTRY * dllGetTexParameterfv )(GLenum target, GLenum pname, GLfloat *params);
static void ( APIENTRY * dllGetTexParameteriv )(GLenum target, GLenum pname, GLint *params);
static void ( APIENTRY * dllHint )(GLenum target, GLenum mode);
static void ( APIENTRY * dllIndexMask )(GLuint mask);
static void ( APIENTRY * dllIndexPointer )(GLenum type, GLsizei stride, const GLvoid *pointer);
static void ( APIENTRY * dllIndexd )(GLdouble c);
static void ( APIENTRY * dllIndexdv )(const GLdouble *c);
static void ( APIENTRY * dllIndexf )(GLfloat c);
static void ( APIENTRY * dllIndexfv )(const GLfloat *c);
static void ( APIENTRY * dllIndexi )(GLint c);
static void ( APIENTRY * dllIndexiv )(const GLint *c);
static void ( APIENTRY * dllIndexs )(GLshort c);
static void ( APIENTRY * dllIndexsv )(const GLshort *c);
static void ( APIENTRY * dllIndexub )(GLubyte c);
static void ( APIENTRY * dllIndexubv )(const GLubyte *c);
static void ( APIENTRY * dllInitNames )(void);
static void ( APIENTRY * dllInitTexObj )(GXTexObj *obj, void *img_ptr, u16 wd, u16 ht, u8 fmt, u8 wrap_s, u8 wrap_t, u8 mipmap);
static void ( APIENTRY * dllInitTexObjFilterMode )(GXTexObj *obj, u8 minfilt, u8 magfilt);
static void ( APIENTRY * dllInterleavedArrays )(GLenum format, GLsizei stride, const GLvoid *pointer);
static void ( APIENTRY * dllInvalidateTexAll )(void);
GLboolean ( APIENTRY * dllIsEnabled )(GLenum cap);
GLboolean ( APIENTRY * dllIsList )(GLuint list);
GLboolean ( APIENTRY * dllIsTexture )(GLuint texture);
static void ( APIENTRY * dllLightModelf )(GLenum pname, GLfloat param);
static void ( APIENTRY * dllLightModelfv )(GLenum pname, const GLfloat *params);
static void ( APIENTRY * dllLightModeli )(GLenum pname, GLint param);
static void ( APIENTRY * dllLightModeliv )(GLenum pname, const GLint *params);
static void ( APIENTRY * dllLightf )(GLenum light, GLenum pname, GLfloat param);
static void ( APIENTRY * dllLightfv )(GLenum light, GLenum pname, const GLfloat *params);
static void ( APIENTRY * dllLighti )(GLenum light, GLenum pname, GLint param);
static void ( APIENTRY * dllLightiv )(GLenum light, GLenum pname, const GLint *params);
static void ( APIENTRY * dllLineStipple )(GLint factor, GLushort pattern);
static void ( APIENTRY * dllLineWidth )(GLfloat width);
static void ( APIENTRY * dllListBase )(GLuint base);
static void ( APIENTRY * dllLoadName )(GLuint name);
static void ( APIENTRY * dllLoadPosMtxImm )(Mtx mt, u32 pnidx);
static void ( APIENTRY * dllLoadProjectionMtx )(Mtx44 mt, u8 type);
static void ( APIENTRY * dllLoadTexObj )(GXTexObj *obj, u8 mapid);
static void ( APIENTRY * dllLogicOp )(GLenum opcode);
static void ( APIENTRY * dllMap1d )(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points);
static void ( APIENTRY * dllMap1f )(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points);
static void ( APIENTRY * dllMap2d )(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points);
static void ( APIENTRY * dllMap2f )(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points);
static void ( APIENTRY * dllMapGrid1d )(GLint un, GLdouble u1, GLdouble u2);
static void ( APIENTRY * dllMapGrid1f )(GLint un, GLfloat u1, GLfloat u2);
static void ( APIENTRY * dllMapGrid2d )(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2);
static void ( APIENTRY * dllMapGrid2f )(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2);
static void ( APIENTRY * dllMaterialf )(GLenum face, GLenum pname, GLfloat param);
static void ( APIENTRY * dllMaterialfv )(GLenum face, GLenum pname, const GLfloat *params);
static void ( APIENTRY * dllMateriali )(GLenum face, GLenum pname, GLint param);
static void ( APIENTRY * dllMaterialiv )(GLenum face, GLenum pname, const GLint *params);
static void ( APIENTRY * dllMtxConcat )(Mtx a, Mtx b, Mtx ab);
static void ( APIENTRY * dllMtxCopy )(Mtx src, Mtx dst);
static void ( APIENTRY * dllMtxIdentity )(Mtx mt);
static void ( APIENTRY * dllMtxRotAxisDeg )(Mtx mt, guVector* axis, f32 deg);
static void ( APIENTRY * dllMtxScale) (Mtx mt, f32 xS, f32 yS, f32 zS);
static void ( APIENTRY * dllMtxTrans )(Mtx mt, f32 xT, f32 yT, f32 zT);
static void ( APIENTRY * dllNewList )(GLuint list, GLenum mode);
static void ( APIENTRY * dllNormal3b )(GLbyte nx, GLbyte ny, GLbyte nz);
static void ( APIENTRY * dllNormal3bv )(const GLbyte *v);
static void ( APIENTRY * dllNormal3d )(GLdouble nx, GLdouble ny, GLdouble nz);
static void ( APIENTRY * dllNormal3dv )(const GLdouble *v);
static void ( APIENTRY * dllNormal3f )(GLfloat nx, GLfloat ny, GLfloat nz);
static void ( APIENTRY * dllNormal3fv )(const GLfloat *v);
static void ( APIENTRY * dllNormal3i )(GLint nx, GLint ny, GLint nz);
static void ( APIENTRY * dllNormal3iv )(const GLint *v);
static void ( APIENTRY * dllNormal3s )(GLshort nx, GLshort ny, GLshort nz);
static void ( APIENTRY * dllNormal3sv )(const GLshort *v);
static void ( APIENTRY * dllNormalPointer )(GLenum type, GLsizei stride, const GLvoid *pointer);
static void ( APIENTRY * dllOrtho )(Mtx44 mt, f32 t, f32 b, f32 l, f32 r, f32 n, f32 f);
static void ( APIENTRY * dllPassThrough )(GLfloat token);
static void ( APIENTRY * dllPerspective )(Mtx44 mt, f32 fovy, f32 aspect, f32 n, f32 f);
static void ( APIENTRY * dllPixelMapfv )(GLenum map, GLsizei mapsize, const GLfloat *values);
static void ( APIENTRY * dllPixelMapuiv )(GLenum map, GLsizei mapsize, const GLuint *values);
static void ( APIENTRY * dllPixelMapusv )(GLenum map, GLsizei mapsize, const GLushort *values);
static void ( APIENTRY * dllPixelStoref )(GLenum pname, GLfloat param);
static void ( APIENTRY * dllPixelStorei )(GLenum pname, GLint param);
static void ( APIENTRY * dllPixelTransferf )(GLenum pname, GLfloat param);
static void ( APIENTRY * dllPixelTransferi )(GLenum pname, GLint param);
static void ( APIENTRY * dllPixelZoom )(GLfloat xfactor, GLfloat yfactor);
static void ( APIENTRY * dllPointSize )(GLfloat size);
static void ( APIENTRY * dllPolygonMode )(GLenum face, GLenum mode);
static void ( APIENTRY * dllPolygonOffset )(GLfloat factor, GLfloat units);
static void ( APIENTRY * dllPolygonStipple )(const GLubyte *mask);
static void ( APIENTRY * dllPopAttrib )(void);
static void ( APIENTRY * dllPopClientAttrib )(void);
static void ( APIENTRY * dllPopName )(void);
static void ( APIENTRY * dllPosition3f32 )(f32 x, f32 y, f32 z);
static void ( APIENTRY * dllPrioritizeTextures )(GLsizei n, const GLuint *textures, const GLclampf *priorities);
static void ( APIENTRY * dllPushAttrib )(GLbitfield mask);
static void ( APIENTRY * dllPushClientAttrib )(GLbitfield mask);
static void ( APIENTRY * dllPushName )(GLuint name);
static void ( APIENTRY * dllRasterPos2d )(GLdouble x, GLdouble y);
static void ( APIENTRY * dllRasterPos2dv )(const GLdouble *v);
static void ( APIENTRY * dllRasterPos2f )(GLfloat x, GLfloat y);
static void ( APIENTRY * dllRasterPos2fv )(const GLfloat *v);
static void ( APIENTRY * dllRasterPos2i )(GLint x, GLint y);
static void ( APIENTRY * dllRasterPos2iv )(const GLint *v);
static void ( APIENTRY * dllRasterPos2s )(GLshort x, GLshort y);
static void ( APIENTRY * dllRasterPos2sv )(const GLshort *v);
static void ( APIENTRY * dllRasterPos3d )(GLdouble x, GLdouble y, GLdouble z);
static void ( APIENTRY * dllRasterPos3dv )(const GLdouble *v);
static void ( APIENTRY * dllRasterPos3f )(GLfloat x, GLfloat y, GLfloat z);
static void ( APIENTRY * dllRasterPos3fv )(const GLfloat *v);
static void ( APIENTRY * dllRasterPos3i )(GLint x, GLint y, GLint z);
static void ( APIENTRY * dllRasterPos3iv )(const GLint *v);
static void ( APIENTRY * dllRasterPos3s )(GLshort x, GLshort y, GLshort z);
static void ( APIENTRY * dllRasterPos3sv )(const GLshort *v);
static void ( APIENTRY * dllRasterPos4d )(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
static void ( APIENTRY * dllRasterPos4dv )(const GLdouble *v);
static void ( APIENTRY * dllRasterPos4f )(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
static void ( APIENTRY * dllRasterPos4fv )(const GLfloat *v);
static void ( APIENTRY * dllRasterPos4i )(GLint x, GLint y, GLint z, GLint w);
static void ( APIENTRY * dllRasterPos4iv )(const GLint *v);
static void ( APIENTRY * dllRasterPos4s )(GLshort x, GLshort y, GLshort z, GLshort w);
static void ( APIENTRY * dllRasterPos4sv )(const GLshort *v);
static void ( APIENTRY * dllReadBuffer )(GLenum mode);
static void ( APIENTRY * dllReadPixels )(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);
static void ( APIENTRY * dllRectd )(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2);
static void ( APIENTRY * dllRectdv )(const GLdouble *v1, const GLdouble *v2);
static void ( APIENTRY * dllRectf )(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
static void ( APIENTRY * dllRectfv )(const GLfloat *v1, const GLfloat *v2);
static void ( APIENTRY * dllRecti )(GLint x1, GLint y1, GLint x2, GLint y2);
static void ( APIENTRY * dllRectiv )(const GLint *v1, const GLint *v2);
static void ( APIENTRY * dllRects )(GLshort x1, GLshort y1, GLshort x2, GLshort y2);
static void ( APIENTRY * dllRectsv )(const GLshort *v1, const GLshort *v2);
GLint ( APIENTRY * dllRenderMode )(GLenum mode);
static void ( APIENTRY * dllScissor )(GLint x, GLint y, GLsizei width, GLsizei height);
static void ( APIENTRY * dllSelectBuffer )(GLsizei size, GLuint *buffer);
static void ( APIENTRY * dllSetAlphaCompare )(u8 comp0, u8 ref0, u8 aop, u8 comp1, u8 ref1);
static void ( APIENTRY * dllSetBlendMode )(u8 type, u8 src_fact, u8 dst_fact, u8 op);
static void ( APIENTRY * dllSetCullMode )(u8 mode);
static void ( APIENTRY * dllSetTevOp )(u8 tevstage, u8 mode);
static void ( APIENTRY * dllSetViewport )(f32 xOrig, f32 yOrig, f32 wd, f32 ht, f32 nearZ, f32 farZ);
static void ( APIENTRY * dllSetZMode )(u8 enable, u8 func, u8 update_enable);
static void ( APIENTRY * dllShadeModel )(GLenum mode);
static void ( APIENTRY * dllStencilFunc )(GLenum func, GLint ref, GLuint mask);
static void ( APIENTRY * dllStencilMask )(GLuint mask);
static void ( APIENTRY * dllStencilOp )(GLenum fail, GLenum zfail, GLenum zpass);
static void ( APIENTRY * dllTexCoord2f32 )(f32 s, f32 t);
static void ( APIENTRY * dllTexCoordPointer )(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
static void ( APIENTRY * dllTexGend )(GLenum coord, GLenum pname, GLdouble param);
static void ( APIENTRY * dllTexGendv )(GLenum coord, GLenum pname, const GLdouble *params);
static void ( APIENTRY * dllTexGenf )(GLenum coord, GLenum pname, GLfloat param);
static void ( APIENTRY * dllTexGenfv )(GLenum coord, GLenum pname, const GLfloat *params);
static void ( APIENTRY * dllTexGeni )(GLenum coord, GLenum pname, GLint param);
static void ( APIENTRY * dllTexGeniv )(GLenum coord, GLenum pname, const GLint *params);
static void ( APIENTRY * dllVertexPointer )(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);

static void APIENTRY logAccum(GLenum op, GLfloat value)
{
	fprintf( log_fp, "glAccum\n" );
	dllAccum( op, value );
}

static GLboolean APIENTRY logAreTexturesResident(GLsizei n, const GLuint *textures, GLboolean *residences)
{
	fprintf( log_fp, "glAreTexturesResident\n" );
	return dllAreTexturesResident( n, textures, residences );
}

static void APIENTRY logArrayElement(GLint i)
{
	fprintf( log_fp, "glArrayElement\n" );
	dllArrayElement( i );
}

static void APIENTRY logBegin(u8 primitve, u8 vtxfmt, u16 vtxcnt)
{
	fprintf( log_fp, "GX_Begin( 0x%x, 0x%x, %u)\n", primitve, vtxfmt, vtxcnt );
	dllBegin( primitve, vtxfmt, vtxcnt );
}

static void APIENTRY logBitmap(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap)
{
	fprintf( log_fp, "glBitmap\n" );
	dllBitmap( width, height, xorig, yorig, xmove, ymove, bitmap );
}

static void APIENTRY logCallList(GLuint list)
{
	fprintf( log_fp, "glCallList( %u )\n", list );
	dllCallList( list );
}

static void APIENTRY logCallLists(GLsizei n, GLenum type, const void *lists)
{
	fprintf( log_fp, "glCallLists\n" );
	dllCallLists( n, type, lists );
}

static void APIENTRY logClear(GLbitfield mask)
{
	fprintf( log_fp, "glClear\n" );
	dllClear( mask );
}

static void APIENTRY logClearAccum(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
	fprintf( log_fp, "glClearAccum\n" );
	dllClearAccum( red, green, blue, alpha );
}

static void APIENTRY logClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
	fprintf( log_fp, "glClearColor\n" );
	dllClearColor( red, green, blue, alpha );
}

static void APIENTRY logClearDepth(GLclampd depth)
{
	fprintf( log_fp, "glClearDepth\n" );
	dllClearDepth( depth );
}

static void APIENTRY logClearIndex(GLfloat c)
{
	fprintf( log_fp, "glClearIndex\n" );
	dllClearIndex( c );
}

static void APIENTRY logClearStencil(GLint s)
{
	fprintf( log_fp, "glClearStencil\n" );
	dllClearStencil( s );
}

static void APIENTRY logClipPlane(GLenum plane, const GLdouble *equation)
{
	fprintf( log_fp, "glClipPlane\n" );
	dllClipPlane( plane, equation );
}

#define SIG( x ) fprintf( log_fp, x "\n" )

static void APIENTRY logColor4u8(u8 r, u8 g, u8 b, u8 a)
{
	SIG( "GX_Color4u8" );
	dllColor4u8( r, g, b, a );
}
static void APIENTRY logColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
	SIG( "glColorMask" );
	dllColorMask( red, green, blue, alpha );
}
static void APIENTRY logColorMaterial(GLenum face, GLenum mode)
{
	SIG( "glColorMaterial" );
	dllColorMaterial( face, mode );
}

static void APIENTRY logColorPointer(GLint size, GLenum type, GLsizei stride, const void *pointer)
{
	SIG( "glColorPointer" );
	dllColorPointer( size, type, stride, pointer );
}

static void APIENTRY logCopyPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type)
{
	SIG( "glCopyPixels" );
	dllCopyPixels( x, y, width, height, type );
}

static void APIENTRY logCopyTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)
{
	SIG( "glCopyTexSubImage1D" );
	dllCopyTexSubImage1D( target, level, xoffset, x, y, width );
}

static void APIENTRY logCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
	SIG( "glCopyTexSubImage2D" );
	dllCopyTexSubImage2D( target, level, xoffset, yoffset, x, y, width, height );
}

static void APIENTRY logCopyTexImage1D(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border)
{
	SIG( "glCopyTexImage1D" );
	dllCopyTexImage1D( target, level, internalFormat, x, y, width, border );
}

static void APIENTRY logCopyTexImage2D(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
	SIG( "glCopyTexImage2D" );
	dllCopyTexImage2D( target, level, internalFormat, x, y, width, height, border );
}

static void APIENTRY logDeleteLists(GLuint list, GLsizei range)
{
	SIG( "glDeleteLists" );
	dllDeleteLists( list, range );
}

static void APIENTRY logDeleteTextures(GLsizei n, const GLuint *textures)
{
	SIG( "glDeleteTextures" );
	dllDeleteTextures( n, textures );
}

static void APIENTRY logDisable(GLenum cap)
{
	fprintf( log_fp, "glDisable( 0x%x )\n", cap );
	dllDisable( cap );
}

static void APIENTRY logDisableClientState(GLenum array)
{
	SIG( "glDisableClientState" );
	dllDisableClientState( array );
}

static void APIENTRY logDisableTexture(void)
{
	SIG( "GXU_DisableTexture" );
	dllDisableTexture();
}

static void APIENTRY logDrawArrays(GLenum mode, GLint first, GLsizei count)
{
	SIG( "glDrawArrays" );
	dllDrawArrays( mode, first, count );
}

static void APIENTRY logDrawBuffer(GLenum mode)
{
	SIG( "glDrawBuffer" );
	dllDrawBuffer( mode );
}

static void APIENTRY logDrawElements(GLenum mode, GLsizei count, GLenum type, const void *indices)
{
	SIG( "glDrawElements" );
	dllDrawElements( mode, count, type, indices );
}

static void APIENTRY logDrawPixels(GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels)
{
	SIG( "glDrawPixels" );
	dllDrawPixels( width, height, format, type, pixels );
}

static void APIENTRY logEdgeFlag(GLboolean flag)
{
	SIG( "glEdgeFlag" );
	dllEdgeFlag( flag );
}

static void APIENTRY logEdgeFlagPointer(GLsizei stride, const void *pointer)
{
	SIG( "glEdgeFlagPointer" );
	dllEdgeFlagPointer( stride, pointer );
}

static void APIENTRY logEdgeFlagv(const GLboolean *flag)
{
	SIG( "glEdgeFlagv" );
	dllEdgeFlagv( flag );
}

static void APIENTRY logEnable(GLenum cap)
{
	fprintf( log_fp, "glEnable( 0x%x )\n", cap );
	dllEnable( cap );
}

static void APIENTRY logEnableClientState(GLenum array)
{
	SIG( "glEnableClientState" );
	dllEnableClientState( array );
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

static void APIENTRY logEndList(void)
{
	SIG( "glEndList" );
	dllEndList();
}

static void APIENTRY logEvalCoord1d(GLdouble u)
{
	SIG( "glEvalCoord1d" );
	dllEvalCoord1d( u );
}

static void APIENTRY logEvalCoord1dv(const GLdouble *u)
{
	SIG( "glEvalCoord1dv" );
	dllEvalCoord1dv( u );
}

static void APIENTRY logEvalCoord1f(GLfloat u)
{
	SIG( "glEvalCoord1f" );
	dllEvalCoord1f( u );
}

static void APIENTRY logEvalCoord1fv(const GLfloat *u)
{
	SIG( "glEvalCoord1fv" );
	dllEvalCoord1fv( u );
}
static void APIENTRY logEvalCoord2d(GLdouble u, GLdouble v)
{
	SIG( "glEvalCoord2d" );
	dllEvalCoord2d( u, v );
}
static void APIENTRY logEvalCoord2dv(const GLdouble *u)
{
	SIG( "glEvalCoord2dv" );
	dllEvalCoord2dv( u );
}
static void APIENTRY logEvalCoord2f(GLfloat u, GLfloat v)
{
	SIG( "glEvalCoord2f" );
	dllEvalCoord2f( u, v );
}
static void APIENTRY logEvalCoord2fv(const GLfloat *u)
{
	SIG( "glEvalCoord2fv" );
	dllEvalCoord2fv( u );
}

static void APIENTRY logEvalMesh1(GLenum mode, GLint i1, GLint i2)
{
	SIG( "glEvalMesh1" );
	dllEvalMesh1( mode, i1, i2 );
}
static void APIENTRY logEvalMesh2(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2)
{
	SIG( "glEvalMesh2" );
	dllEvalMesh2( mode, i1, i2, j1, j2 );
}
static void APIENTRY logEvalPoint1(GLint i)
{
	SIG( "glEvalPoint1" );
	dllEvalPoint1( i );
}
static void APIENTRY logEvalPoint2(GLint i, GLint j)
{
	SIG( "glEvalPoint2" );
	dllEvalPoint2( i, j );
}

static void APIENTRY logFeedbackBuffer(GLsizei size, GLenum type, GLfloat *buffer)
{
	SIG( "glFeedbackBuffer" );
	dllFeedbackBuffer( size, type, buffer );
}

static void APIENTRY logFinish(void)
{
	SIG( "glFinish" );
	dllFinish();
}

static void APIENTRY logFlush(void)
{
	SIG( "glFlush" );
	dllFlush();
}

static void APIENTRY logFogf(GLenum pname, GLfloat param)
{
	SIG( "glFogf" );
	dllFogf( pname, param );
}

static void APIENTRY logFogfv(GLenum pname, const GLfloat *params)
{
	SIG( "glFogfv" );
	dllFogfv( pname, params );
}

static void APIENTRY logFogi(GLenum pname, GLint param)
{
	SIG( "glFogi" );
	dllFogi( pname, param );
}

static void APIENTRY logFogiv(GLenum pname, const GLint *params)
{
	SIG( "glFogiv" );
	dllFogiv( pname, params );
}

static void APIENTRY logFrontFace(GLenum mode)
{
	SIG( "glFrontFace" );
	dllFrontFace( mode );
}

static GLuint APIENTRY logGenLists(GLsizei range)
{
	SIG( "glGenLists" );
	return dllGenLists( range );
}

static void APIENTRY logGenTextures(GLsizei n, GLuint *textures)
{
	SIG( "glGenTextures" );
	dllGenTextures( n, textures );
}

static void APIENTRY logGetBooleanv(GLenum pname, GLboolean *params)
{
	SIG( "glGetBooleanv" );
	dllGetBooleanv( pname, params );
}

static void APIENTRY logGetClipPlane(GLenum plane, GLdouble *equation)
{
	SIG( "glGetClipPlane" );
	dllGetClipPlane( plane, equation );
}

static void APIENTRY logGetDoublev(GLenum pname, GLdouble *params)
{
	SIG( "glGetDoublev" );
	dllGetDoublev( pname, params );
}

static GLenum APIENTRY logGetError(void)
{
	SIG( "glGetError" );
	return dllGetError();
}

static void APIENTRY logGetIntegerv(GLenum pname, GLint *params)
{
	SIG( "glGetIntegerv" );
	dllGetIntegerv( pname, params );
}

static void APIENTRY logGetLightfv(GLenum light, GLenum pname, GLfloat *params)
{
	SIG( "glGetLightfv" );
	dllGetLightfv( light, pname, params );
}

static void APIENTRY logGetLightiv(GLenum light, GLenum pname, GLint *params)
{
	SIG( "glGetLightiv" );
	dllGetLightiv( light, pname, params );
}

static void APIENTRY logGetMapdv(GLenum target, GLenum query, GLdouble *v)
{
	SIG( "glGetMapdv" );
	dllGetMapdv( target, query, v );
}

static void APIENTRY logGetMapfv(GLenum target, GLenum query, GLfloat *v)
{
	SIG( "glGetMapfv" );
	dllGetMapfv( target, query, v );
}

static void APIENTRY logGetMapiv(GLenum target, GLenum query, GLint *v)
{
	SIG( "glGetMapiv" );
	dllGetMapiv( target, query, v );
}

static void APIENTRY logGetMaterialfv(GLenum face, GLenum pname, GLfloat *params)
{
	SIG( "glGetMaterialfv" );
	dllGetMaterialfv( face, pname, params );
}

static void APIENTRY logGetMaterialiv(GLenum face, GLenum pname, GLint *params)
{
	SIG( "glGetMaterialiv" );
	dllGetMaterialiv( face, pname, params );
}

static void APIENTRY logGetPixelMapfv(GLenum map, GLfloat *values)
{
	SIG( "glGetPixelMapfv" );
	dllGetPixelMapfv( map, values );
}

static void APIENTRY logGetPixelMapuiv(GLenum map, GLuint *values)
{
	SIG( "glGetPixelMapuiv" );
	dllGetPixelMapuiv( map, values );
}

static void APIENTRY logGetPixelMapusv(GLenum map, GLushort *values)
{
	SIG( "glGetPixelMapusv" );
	dllGetPixelMapusv( map, values );
}

static void APIENTRY logGetPointerv(GLenum pname, GLvoid* *params)
{
	SIG( "glGetPointerv" );
	dllGetPointerv( pname, params );
}

static void APIENTRY logGetPolygonStipple(GLubyte *mask)
{
	SIG( "glGetPolygonStipple" );
	dllGetPolygonStipple( mask );
}

static const GLubyte * APIENTRY logGetString(GLenum name)
{
	SIG( "glGetString" );
	return dllGetString( name );
}

static void APIENTRY logGetTexEnvfv(GLenum target, GLenum pname, GLfloat *params)
{
	SIG( "glGetTexEnvfv" );
	dllGetTexEnvfv( target, pname, params );
}

static void APIENTRY logGetTexEnviv(GLenum target, GLenum pname, GLint *params)
{
	SIG( "glGetTexEnviv" );
	dllGetTexEnviv( target, pname, params );
}

static void APIENTRY logGetTexGendv(GLenum coord, GLenum pname, GLdouble *params)
{
	SIG( "glGetTexGendv" );
	dllGetTexGendv( coord, pname, params );
}

static void APIENTRY logGetTexGenfv(GLenum coord, GLenum pname, GLfloat *params)
{
	SIG( "glGetTexGenfv" );
	dllGetTexGenfv( coord, pname, params );
}

static void APIENTRY logGetTexGeniv(GLenum coord, GLenum pname, GLint *params)
{
	SIG( "glGetTexGeniv" );
	dllGetTexGeniv( coord, pname, params );
}

static void APIENTRY logGetTexImage(GLenum target, GLint level, GLenum format, GLenum type, void *pixels)
{
	SIG( "glGetTexImage" );
	dllGetTexImage( target, level, format, type, pixels );
}
static void APIENTRY logGetTexLevelParameterfv(GLenum target, GLint level, GLenum pname, GLfloat *params )
{
	SIG( "glGetTexLevelParameterfv" );
	dllGetTexLevelParameterfv( target, level, pname, params );
}

static void APIENTRY logGetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint *params)
{
	SIG( "glGetTexLevelParameteriv" );
	dllGetTexLevelParameteriv( target, level, pname, params );
}

static void APIENTRY logGetTexParameterfv(GLenum target, GLenum pname, GLfloat *params)
{
	SIG( "glGetTexParameterfv" );
	dllGetTexParameterfv( target, pname, params );
}

static void APIENTRY logGetTexParameteriv(GLenum target, GLenum pname, GLint *params)
{
	SIG( "glGetTexParameteriv" );
	dllGetTexParameteriv( target, pname, params );
}

static void APIENTRY logHint(GLenum target, GLenum mode)
{
	fprintf( log_fp, "glHint( 0x%x, 0x%x )\n", target, mode );
	dllHint( target, mode );
}

static void APIENTRY logIndexMask(GLuint mask)
{
	SIG( "glIndexMask" );
	dllIndexMask( mask );
}

static void APIENTRY logIndexPointer(GLenum type, GLsizei stride, const void *pointer)
{
	SIG( "glIndexPointer" );
	dllIndexPointer( type, stride, pointer );
}

static void APIENTRY logIndexd(GLdouble c)
{
	SIG( "glIndexd" );
	dllIndexd( c );
}

static void APIENTRY logIndexdv(const GLdouble *c)
{
	SIG( "glIndexdv" );
	dllIndexdv( c );
}

static void APIENTRY logIndexf(GLfloat c)
{
	SIG( "glIndexf" );
	dllIndexf( c );
}

static void APIENTRY logIndexfv(const GLfloat *c)
{
	SIG( "glIndexfv" );
	dllIndexfv( c );
}

static void APIENTRY logIndexi(GLint c)
{
	SIG( "glIndexi" );
	dllIndexi( c );
}

static void APIENTRY logIndexiv(const GLint *c)
{
	SIG( "glIndexiv" );
	dllIndexiv( c );
}

static void APIENTRY logIndexs(GLshort c)
{
	SIG( "glIndexs" );
	dllIndexs( c );
}

static void APIENTRY logIndexsv(const GLshort *c)
{
	SIG( "glIndexsv" );
	dllIndexsv( c );
}

static void APIENTRY logIndexub(GLubyte c)
{
	SIG( "glIndexub" );
	dllIndexub( c );
}

static void APIENTRY logIndexubv(const GLubyte *c)
{
	SIG( "glIndexubv" );
	dllIndexubv( c );
}

static void APIENTRY logInitNames(void)
{
	SIG( "glInitNames" );
	dllInitNames();
}

static void APIENTRY logInitTexObj(GXTexObj *obj, void *img_ptr, u16 wd, u16 ht, u8 fmt, u8 wrap_s, u8 wrap_t, u8 mipmap)
{
	SIG( "GX_InitTexObj" );
	dllInitTexObj(obj, img_ptr, wd, ht, fmt, wrap_s, wrap_t, mipmap);
}

static void APIENTRY logInitTexObjFilterMode(GXTexObj *obj,u8 minfilt,u8 magfilt)
{
	SIG( "GX_InitTexObjFilterMode" );
	dllInitTexObjFilterMode(obj, minfilt, magfilt);
}

static void APIENTRY logInterleavedArrays(GLenum format, GLsizei stride, const void *pointer)
{
	SIG( "glInterleavedArrays" );
	dllInterleavedArrays( format, stride, pointer );
}

static void APIENTRY logInvalidateTexAll(void)
{
	SIG( "GX_InvalidateTexAll" );
	dllInvalidateTexAll();
}

static GLboolean APIENTRY logIsEnabled(GLenum cap)
{
	SIG( "glIsEnabled" );
	return dllIsEnabled( cap );
}
static GLboolean APIENTRY logIsList(GLuint list)
{
	SIG( "glIsList" );
	return dllIsList( list );
}
static GLboolean APIENTRY logIsTexture(GLuint texture)
{
	SIG( "glIsTexture" );
	return dllIsTexture( texture );
}

static void APIENTRY logLightModelf(GLenum pname, GLfloat param)
{
	SIG( "glLightModelf" );
	dllLightModelf( pname, param );
}

static void APIENTRY logLightModelfv(GLenum pname, const GLfloat *params)
{
	SIG( "glLightModelfv" );
	dllLightModelfv( pname, params );
}

static void APIENTRY logLightModeli(GLenum pname, GLint param)
{
	SIG( "glLightModeli" );
	dllLightModeli( pname, param );

}

static void APIENTRY logLightModeliv(GLenum pname, const GLint *params)
{
	SIG( "glLightModeliv" );
	dllLightModeliv( pname, params );
}

static void APIENTRY logLightf(GLenum light, GLenum pname, GLfloat param)
{
	SIG( "glLightf" );
	dllLightf( light, pname, param );
}

static void APIENTRY logLightfv(GLenum light, GLenum pname, const GLfloat *params)
{
	SIG( "glLightfv" );
	dllLightfv( light, pname, params );
}

static void APIENTRY logLighti(GLenum light, GLenum pname, GLint param)
{
	SIG( "glLighti" );
	dllLighti( light, pname, param );
}

static void APIENTRY logLightiv(GLenum light, GLenum pname, const GLint *params)
{
	SIG( "glLightiv" );
	dllLightiv( light, pname, params );
}

static void APIENTRY logLineStipple(GLint factor, GLushort pattern)
{
	SIG( "glLineStipple" );
	dllLineStipple( factor, pattern );
}

static void APIENTRY logLineWidth(GLfloat width)
{
	SIG( "glLineWidth" );
	dllLineWidth( width );
}

static void APIENTRY logListBase(GLuint base)
{
	SIG( "glListBase" );
	dllListBase( base );
}

static void APIENTRY logLoadName(GLuint name)
{
	SIG( "glLoadName" );
	dllLoadName( name );
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
	fprintf( log_fp, "GX_LoadTexObj( 0x%x, %u )\n", (unsigned int)obj, mapid );
	dllLoadTexObj( obj, mapid );
}

static void APIENTRY logLogicOp(GLenum opcode)
{
	SIG( "glLogicOp" );
	dllLogicOp( opcode );
}

static void APIENTRY logMap1d(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points)
{
	SIG( "glMap1d" );
	dllMap1d( target, u1, u2, stride, order, points );
}

static void APIENTRY logMap1f(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points)
{
	SIG( "glMap1f" );
	dllMap1f( target, u1, u2, stride, order, points );
}

static void APIENTRY logMap2d(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points)
{
	SIG( "glMap2d" );
	dllMap2d( target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points );
}

static void APIENTRY logMap2f(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points)
{
	SIG( "glMap2f" );
	dllMap2f( target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points );
}

static void APIENTRY logMapGrid1d(GLint un, GLdouble u1, GLdouble u2)
{
	SIG( "glMapGrid1d" );
	dllMapGrid1d( un, u1, u2 );
}

static void APIENTRY logMapGrid1f(GLint un, GLfloat u1, GLfloat u2)
{
	SIG( "glMapGrid1f" );
	dllMapGrid1f( un, u1, u2 );
}

static void APIENTRY logMapGrid2d(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2)
{
	SIG( "glMapGrid2d" );
	dllMapGrid2d( un, u1, u2, vn, v1, v2 );
}
static void APIENTRY logMapGrid2f(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2)
{
	SIG( "glMapGrid2f" );
	dllMapGrid2f( un, u1, u2, vn, v1, v2 );
}
static void APIENTRY logMaterialf(GLenum face, GLenum pname, GLfloat param)
{
	SIG( "glMaterialf" );
	dllMaterialf( face, pname, param );
}
static void APIENTRY logMaterialfv(GLenum face, GLenum pname, const GLfloat *params)
{
	SIG( "glMaterialfv" );
	dllMaterialfv( face, pname, params );
}

static void APIENTRY logMateriali(GLenum face, GLenum pname, GLint param)
{
	SIG( "glMateriali" );
	dllMateriali( face, pname, param );
}

static void APIENTRY logMaterialiv(GLenum face, GLenum pname, const GLint *params)
{
	SIG( "glMaterialiv" );
	dllMaterialiv( face, pname, params );
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

static void APIENTRY logNewList(GLuint list, GLenum mode)
{
	SIG( "glNewList" );
	dllNewList( list, mode );
}

static void APIENTRY logNormal3b(GLbyte nx, GLbyte ny, GLbyte nz)
{
	SIG ("glNormal3b" );
	dllNormal3b( nx, ny, nz );
}

static void APIENTRY logNormal3bv(const GLbyte *v)
{
	SIG( "glNormal3bv" );
	dllNormal3bv( v );
}

static void APIENTRY logNormal3d(GLdouble nx, GLdouble ny, GLdouble nz)
{
	SIG( "glNormal3d" );
	dllNormal3d( nx, ny, nz );
}

static void APIENTRY logNormal3dv(const GLdouble *v)
{
	SIG( "glNormal3dv" );
	dllNormal3dv( v );
}

static void APIENTRY logNormal3f(GLfloat nx, GLfloat ny, GLfloat nz)
{
	SIG( "glNormal3f" );
	dllNormal3f( nx, ny, nz );
}

static void APIENTRY logNormal3fv(const GLfloat *v)
{
	SIG( "glNormal3fv" );
	dllNormal3fv( v );
}
static void APIENTRY logNormal3i(GLint nx, GLint ny, GLint nz)
{
	SIG( "glNormal3i" );
	dllNormal3i( nx, ny, nz );
}
static void APIENTRY logNormal3iv(const GLint *v)
{
	SIG( "glNormal3iv" );
	dllNormal3iv( v );
}
static void APIENTRY logNormal3s(GLshort nx, GLshort ny, GLshort nz)
{
	SIG( "glNormal3s" );
	dllNormal3s( nx, ny, nz );
}
static void APIENTRY logNormal3sv(const GLshort *v)
{
	SIG( "glNormal3sv" );
	dllNormal3sv( v );
}
static void APIENTRY logNormalPointer(GLenum type, GLsizei stride, const void *pointer)
{
	SIG( "glNormalPointer" );
	dllNormalPointer( type, stride, pointer );
}
static void APIENTRY logOrtho(Mtx44 mt, f32 t, f32 b, f32 l, f32 r, f32 n, f32 f)
{
	SIG( "guOrtho" );
	dllOrtho( mt, t, b, l, r, n, f );
}

static void APIENTRY logPassThrough(GLfloat token)
{
	SIG( "glPassThrough" );
	dllPassThrough( token );
}

static void APIENTRY logPerspective(Mtx44 mt, f32 fovy, f32 aspect, f32 n, f32 f)
{
	SIG( "guPerspective" );
	dllPerspective( mt, fovy, aspect, n, f );
}

static void APIENTRY logPixelMapfv(GLenum map, GLsizei mapsize, const GLfloat *values)
{
	SIG( "glPixelMapfv" );
	dllPixelMapfv( map, mapsize, values );
}

static void APIENTRY logPixelMapuiv(GLenum map, GLsizei mapsize, const GLuint *values)
{
	SIG( "glPixelMapuiv" );
	dllPixelMapuiv( map, mapsize, values );
}

static void APIENTRY logPixelMapusv(GLenum map, GLsizei mapsize, const GLushort *values)
{
	SIG( "glPixelMapusv" );
	dllPixelMapusv( map, mapsize, values );
}
static void APIENTRY logPixelStoref(GLenum pname, GLfloat param)
{
	SIG( "glPixelStoref" );
	dllPixelStoref( pname, param );
}
static void APIENTRY logPixelStorei(GLenum pname, GLint param)
{
	SIG( "glPixelStorei" );
	dllPixelStorei( pname, param );
}
static void APIENTRY logPixelTransferf(GLenum pname, GLfloat param)
{
	SIG( "glPixelTransferf" );
	dllPixelTransferf( pname, param );
}

static void APIENTRY logPixelTransferi(GLenum pname, GLint param)
{
	SIG( "glPixelTransferi" );
	dllPixelTransferi( pname, param );
}

static void APIENTRY logPixelZoom(GLfloat xfactor, GLfloat yfactor)
{
	SIG( "glPixelZoom" );
	dllPixelZoom( xfactor, yfactor );
}

static void APIENTRY logPointSize(GLfloat size)
{
	SIG( "glPointSize" );
	dllPointSize( size );
}

static void APIENTRY logPolygonMode(GLenum face, GLenum mode)
{
	fprintf( log_fp, "glPolygonMode( 0x%x, 0x%x )\n", face, mode );
	dllPolygonMode( face, mode );
}

static void APIENTRY logPolygonOffset(GLfloat factor, GLfloat units)
{
	SIG( "glPolygonOffset" );
	dllPolygonOffset( factor, units );
}
static void APIENTRY logPolygonStipple(const GLubyte *mask )
{
	SIG( "glPolygonStipple" );
	dllPolygonStipple( mask );
}
static void APIENTRY logPopAttrib(void)
{
	SIG( "glPopAttrib" );
	dllPopAttrib();
}

static void APIENTRY logPopClientAttrib(void)
{
	SIG( "glPopClientAttrib" );
	dllPopClientAttrib();
}

static void APIENTRY logPopName(void)
{
	SIG( "glPopName" );
	dllPopName();
}

static void APIENTRY logPosition3f32(f32 x, f32 y, f32 z)
{
	SIG( "GX_Position3f32" );
	dllPosition3f32( x, y, z );
}

static void APIENTRY logPrioritizeTextures(GLsizei n, const GLuint *textures, const GLclampf *priorities)
{
	SIG( "glPrioritizeTextures" );
	dllPrioritizeTextures( n, textures, priorities );
}

static void APIENTRY logPushAttrib(GLbitfield mask)
{
	SIG( "glPushAttrib" );
	dllPushAttrib( mask );
}

static void APIENTRY logPushClientAttrib(GLbitfield mask)
{
	SIG( "glPushClientAttrib" );
	dllPushClientAttrib( mask );
}

static void APIENTRY logPushName(GLuint name)
{
	SIG( "glPushName" );
	dllPushName( name );
}

static void APIENTRY logRasterPos2d(GLdouble x, GLdouble y)
{
	SIG ("glRasterPot2d" );
	dllRasterPos2d( x, y );
}

static void APIENTRY logRasterPos2dv(const GLdouble *v)
{
	SIG( "glRasterPos2dv" );
	dllRasterPos2dv( v );
}

static void APIENTRY logRasterPos2f(GLfloat x, GLfloat y)
{
	SIG( "glRasterPos2f" );
	dllRasterPos2f( x, y );
}
static void APIENTRY logRasterPos2fv(const GLfloat *v)
{
	SIG( "glRasterPos2dv" );
	dllRasterPos2fv( v );
}
static void APIENTRY logRasterPos2i(GLint x, GLint y)
{
	SIG( "glRasterPos2if" );
	dllRasterPos2i( x, y );
}
static void APIENTRY logRasterPos2iv(const GLint *v)
{
	SIG( "glRasterPos2iv" );
	dllRasterPos2iv( v );
}
static void APIENTRY logRasterPos2s(GLshort x, GLshort y)
{
	SIG( "glRasterPos2s" );
	dllRasterPos2s( x, y );
}
static void APIENTRY logRasterPos2sv(const GLshort *v)
{
	SIG( "glRasterPos2sv" );
	dllRasterPos2sv( v );
}
static void APIENTRY logRasterPos3d(GLdouble x, GLdouble y, GLdouble z)
{
	SIG( "glRasterPos3d" );
	dllRasterPos3d( x, y, z );
}
static void APIENTRY logRasterPos3dv(const GLdouble *v)
{
	SIG( "glRasterPos3dv" );
	dllRasterPos3dv( v );
}
static void APIENTRY logRasterPos3f(GLfloat x, GLfloat y, GLfloat z)
{
	SIG( "glRasterPos3f" );
	dllRasterPos3f( x, y, z );
}
static void APIENTRY logRasterPos3fv(const GLfloat *v)
{
	SIG( "glRasterPos3fv" );
	dllRasterPos3fv( v );
}
static void APIENTRY logRasterPos3i(GLint x, GLint y, GLint z)
{
	SIG( "glRasterPos3i" );
	dllRasterPos3i( x, y, z );
}
static void APIENTRY logRasterPos3iv(const GLint *v)
{
	SIG( "glRasterPos3iv" );
	dllRasterPos3iv( v );
}
static void APIENTRY logRasterPos3s(GLshort x, GLshort y, GLshort z)
{
	SIG( "glRasterPos3s" );
	dllRasterPos3s( x, y, z );
}
static void APIENTRY logRasterPos3sv(const GLshort *v)
{
	SIG( "glRasterPos3sv" );
	dllRasterPos3sv( v );
}
static void APIENTRY logRasterPos4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	SIG( "glRasterPos4d" );
	dllRasterPos4d( x, y, z, w );
}
static void APIENTRY logRasterPos4dv(const GLdouble *v)
{
	SIG( "glRasterPos4dv" );
	dllRasterPos4dv( v );
}
static void APIENTRY logRasterPos4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	SIG( "glRasterPos4f" );
	dllRasterPos4f( x, y, z, w );
}
static void APIENTRY logRasterPos4fv(const GLfloat *v)
{
	SIG( "glRasterPos4fv" );
	dllRasterPos4fv( v );
}
static void APIENTRY logRasterPos4i(GLint x, GLint y, GLint z, GLint w)
{
	SIG( "glRasterPos4i" );
	dllRasterPos4i( x, y, z, w );
}
static void APIENTRY logRasterPos4iv(const GLint *v)
{
	SIG( "glRasterPos4iv" );
	dllRasterPos4iv( v );
}
static void APIENTRY logRasterPos4s(GLshort x, GLshort y, GLshort z, GLshort w)
{
	SIG( "glRasterPos4s" );
	dllRasterPos4s( x, y, z, w );
}
static void APIENTRY logRasterPos4sv(const GLshort *v)
{
	SIG( "glRasterPos4sv" );
	dllRasterPos4sv( v );
}
static void APIENTRY logReadBuffer(GLenum mode)
{
	SIG( "glReadBuffer" );
	dllReadBuffer( mode );
}
static void APIENTRY logReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels)
{
	SIG( "glReadPixels" );
	dllReadPixels( x, y, width, height, format, type, pixels );
}

static void APIENTRY logRectd(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2)
{
	SIG( "glRectd" );
	dllRectd( x1, y1, x2, y2 );
}

static void APIENTRY logRectdv(const GLdouble *v1, const GLdouble *v2)
{
	SIG( "glRectdv" );
	dllRectdv( v1, v2 );
}

static void APIENTRY logRectf(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)
{
	SIG( "glRectf" );
	dllRectf( x1, y1, x2, y2 );
}

static void APIENTRY logRectfv(const GLfloat *v1, const GLfloat *v2)
{
	SIG( "glRectfv" );
	dllRectfv( v1, v2 );
}
static void APIENTRY logRecti(GLint x1, GLint y1, GLint x2, GLint y2)
{
	SIG( "glRecti" );
	dllRecti( x1, y1, x2, y2 );
}
static void APIENTRY logRectiv(const GLint *v1, const GLint *v2)
{
	SIG( "glRectiv" );
	dllRectiv( v1, v2 );
}
static void APIENTRY logRects(GLshort x1, GLshort y1, GLshort x2, GLshort y2)
{
	SIG( "glRects" );
	dllRects( x1, y1, x2, y2 );
}
static void APIENTRY logRectsv(const GLshort *v1, const GLshort *v2)
{
	SIG( "glRectsv" );
	dllRectsv( v1, v2 );
}
static GLint APIENTRY logRenderMode(GLenum mode)
{
	SIG( "glRenderMode" );
	return dllRenderMode( mode );
}

static void APIENTRY logScissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
	SIG( "glScissor" );
	dllScissor( x, y, width, height );
}

static void APIENTRY logSelectBuffer(GLsizei size, GLuint *buffer)
{
	SIG( "glSelectBuffer" );
	dllSelectBuffer( size, buffer );
}
static void APIENTRY logSetAlphaCompare(u8 comp0, u8 ref0, u8 aop, u8 comp1, u8 ref1)
{
	fprintf( log_fp, "GX_SetAlphaCompare( 0x%x, 0x%x, 0x%x, 0x%x, 0x%x )\n", comp0, ref0, aop, comp1, ref1 );
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
static void APIENTRY logSetTevOp(u8 tevstage, u8 mode)
{
	fprintf( log_fp, "GX_SetTevOp( 0x%x, 0x%x )\n", tevstage, mode );
	dllSetTevOp( tevstage, mode );
}
static void APIENTRY logSetViewport(f32 xOrig, f32 yOrig, f32 wd, f32 ht, f32 nearZ, f32 farZ)
{
	SIG( "GX_SetViewport" );
	dllSetViewport( xOrig, yOrig, wd, ht, nearZ, farZ );
}
static void APIENTRY logSetZMode(u8 enable, u8 func, u8 update_enable)
{
	SIG( "GX_SetZMode" );
	dllSetZMode( enable, func, update_enable );
}


static void APIENTRY logShadeModel(GLenum mode)
{
	SIG( "glShadeModel" );
	dllShadeModel( mode );
}

static void APIENTRY logStencilFunc(GLenum func, GLint ref, GLuint mask)
{
	SIG( "glStencilFunc" );
	dllStencilFunc( func, ref, mask );
}

static void APIENTRY logStencilMask(GLuint mask)
{
	SIG( "glStencilMask" );
	dllStencilMask( mask );
}

static void APIENTRY logStencilOp(GLenum fail, GLenum zfail, GLenum zpass)
{
	SIG( "glStencilOp" );
	dllStencilOp( fail, zfail, zpass );
}

static void APIENTRY logTexCoord2f32(f32 s, f32 t)
{
	SIG( "GX_TexCoord2f32" );
	dllTexCoord2f32( s, t );
}
static void APIENTRY logTexCoordPointer(GLint size, GLenum type, GLsizei stride, const void *pointer)
{
	SIG( "glTexCoordPointer" );
	dllTexCoordPointer( size, type, stride, pointer );
}

static void APIENTRY logTexGend(GLenum coord, GLenum pname, GLdouble param)
{
	SIG( "glTexGend" );
	dllTexGend( coord, pname, param );
}

static void APIENTRY logTexGendv(GLenum coord, GLenum pname, const GLdouble *params)
{
	SIG( "glTexGendv" );
	dllTexGendv( coord, pname, params );
}

static void APIENTRY logTexGenf(GLenum coord, GLenum pname, GLfloat param)
{
	SIG( "glTexGenf" );
	dllTexGenf( coord, pname, param );
}
static void APIENTRY logTexGenfv(GLenum coord, GLenum pname, const GLfloat *params)
{
	SIG( "glTexGenfv" );
	dllTexGenfv( coord, pname, params );
}
static void APIENTRY logTexGeni(GLenum coord, GLenum pname, GLint param)
{
	SIG( "glTexGeni" );
	dllTexGeni( coord, pname, param );
}
static void APIENTRY logTexGeniv(GLenum coord, GLenum pname, const GLint *params)
{
	SIG( "glTexGeniv" );
	dllTexGeniv( coord, pname, params );
}

static void APIENTRY logVertexPointer(GLint size, GLenum type, GLsizei stride, const void *pointer)
{
	SIG( "glVertexPointer" );
	dllVertexPointer( size, type, stride, pointer );
}

/*
** QGL_Shutdown
**
** Unloads the specified DLL then nulls out all the proc pointers.
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
	qglAccum                     = NULL;
	qglAreTexturesResident       = NULL;
	qglArrayElement              = NULL;
	qgxBegin                     = NULL;
	qglBitmap                    = NULL;
	qglCallList                  = NULL;
	qglCallLists                 = NULL;
	qglClear                     = NULL;
	qglClearAccum                = NULL;
	qglClearColor                = NULL;
	qglClearDepth                = NULL;
	qglClearIndex                = NULL;
	qglClearStencil              = NULL;
	qglClipPlane                 = NULL;
	qgxColor4u8                  = NULL;
	qglColorMask                 = NULL;
	qglColorMaterial             = NULL;
	qglColorPointer              = NULL;
	qglCopyPixels                = NULL;
	qglCopyTexImage1D            = NULL;
	qglCopyTexImage2D            = NULL;
	qglCopyTexSubImage1D         = NULL;
	qglCopyTexSubImage2D         = NULL;
	qglDeleteLists               = NULL;
	qglDeleteTextures            = NULL;
	qglDisable                   = NULL;
	qglDisableClientState        = NULL;
	qgxDisableTexture            = NULL;
	qglDrawArrays                = NULL;
	qglDrawBuffer                = NULL;
	qglDrawElements              = NULL;
	qglDrawPixels                = NULL;
	qglEdgeFlag                  = NULL;
	qglEdgeFlagPointer           = NULL;
	qglEdgeFlagv                 = NULL;
	qglEnable                    = NULL;
	qglEnableClientState         = NULL;
	qgxEnableTexture             = NULL;
	qgxEnd                       = NULL;
	qglEndList                   = NULL;
	qglEvalCoord1d               = NULL;
	qglEvalCoord1dv              = NULL;
	qglEvalCoord1f               = NULL;
	qglEvalCoord1fv              = NULL;
	qglEvalCoord2d               = NULL;
	qglEvalCoord2dv              = NULL;
	qglEvalCoord2f               = NULL;
	qglEvalCoord2fv              = NULL;
	qglEvalMesh1                 = NULL;
	qglEvalMesh2                 = NULL;
	qglEvalPoint1                = NULL;
	qglEvalPoint2                = NULL;
	qglFeedbackBuffer            = NULL;
	qglFinish                    = NULL;
	qglFlush                     = NULL;
	qglFogf                      = NULL;
	qglFogfv                     = NULL;
	qglFogi                      = NULL;
	qglFogiv                     = NULL;
	qglFrontFace                 = NULL;
	qglGenLists                  = NULL;
	qglGenTextures               = NULL;
	qglGetBooleanv               = NULL;
	qglGetClipPlane              = NULL;
	qglGetDoublev                = NULL;
	qglGetError                  = NULL;
	qglGetIntegerv               = NULL;
	qglGetLightfv                = NULL;
	qglGetLightiv                = NULL;
	qglGetMapdv                  = NULL;
	qglGetMapfv                  = NULL;
	qglGetMapiv                  = NULL;
	qglGetMaterialfv             = NULL;
	qglGetMaterialiv             = NULL;
	qglGetPixelMapfv             = NULL;
	qglGetPixelMapuiv            = NULL;
	qglGetPixelMapusv            = NULL;
	qglGetPointerv               = NULL;
	qglGetPolygonStipple         = NULL;
	qglGetString                 = NULL;
	qglGetTexEnvfv               = NULL;
	qglGetTexEnviv               = NULL;
	qglGetTexGendv               = NULL;
	qglGetTexGenfv               = NULL;
	qglGetTexGeniv               = NULL;
	qglGetTexImage               = NULL;
	qglGetTexLevelParameterfv    = NULL;
	qglGetTexLevelParameteriv    = NULL;
	qglGetTexParameterfv         = NULL;
	qglGetTexParameteriv         = NULL;
	qglHint                      = NULL;
	qglIndexMask                 = NULL;
	qglIndexPointer              = NULL;
	qglIndexd                    = NULL;
	qglIndexdv                   = NULL;
	qglIndexf                    = NULL;
	qglIndexfv                   = NULL;
	qglIndexi                    = NULL;
	qglIndexiv                   = NULL;
	qglIndexs                    = NULL;
	qglIndexsv                   = NULL;
	qglIndexub                   = NULL;
	qglIndexubv                  = NULL;
	qglInitNames                 = NULL;
	qgxInitTexObj                = NULL;
	qgxInitTexObjFilterMode      = NULL;
	qglInterleavedArrays         = NULL;
	qgxInvalidateTexAll          = NULL;
	qglIsEnabled                 = NULL;
	qglIsList                    = NULL;
	qglIsTexture                 = NULL;
	qglLightModelf               = NULL;
	qglLightModelfv              = NULL;
	qglLightModeli               = NULL;
	qglLightModeliv              = NULL;
	qglLightf                    = NULL;
	qglLightfv                   = NULL;
	qglLighti                    = NULL;
	qglLightiv                   = NULL;
	qglLineStipple               = NULL;
	qglLineWidth                 = NULL;
	qglListBase                  = NULL;
	qglLoadName                  = NULL;
	qgxLoadPosMtxImm             = NULL;
	qgxLoadProjectionMtx         = NULL;
	qgxLoadTexObj                = NULL;
	qglLogicOp                   = NULL;
	qglMap1d                     = NULL;
	qglMap1f                     = NULL;
	qglMap2d                     = NULL;
	qglMap2f                     = NULL;
	qglMapGrid1d                 = NULL;
	qglMapGrid1f                 = NULL;
	qglMapGrid2d                 = NULL;
	qglMapGrid2f                 = NULL;
	qglMaterialf                 = NULL;
	qglMaterialfv                = NULL;
	qglMateriali                 = NULL;
	qglMaterialiv                = NULL;
	qglNewList                   = NULL;
	qglNormal3b                  = NULL;
	qglNormal3bv                 = NULL;
	qglNormal3d                  = NULL;
	qglNormal3dv                 = NULL;
	qglNormal3f                  = NULL;
	qglNormal3fv                 = NULL;
	qglNormal3i                  = NULL;
	qglNormal3iv                 = NULL;
	qglNormal3s                  = NULL;
	qglNormal3sv                 = NULL;
	qglNormalPointer             = NULL;
	qglPassThrough               = NULL;
	qglPixelMapfv                = NULL;
	qglPixelMapuiv               = NULL;
	qglPixelMapusv               = NULL;
	qglPixelStoref               = NULL;
	qglPixelStorei               = NULL;
	qglPixelTransferf            = NULL;
	qglPixelTransferi            = NULL;
	qglPixelZoom                 = NULL;
	qglPointSize                 = NULL;
	qglPolygonMode               = NULL;
	qglPolygonOffset             = NULL;
	qglPolygonStipple            = NULL;
	qglPopAttrib                 = NULL;
	qglPopClientAttrib           = NULL;
	qglPopName                   = NULL;
	qgxPosition3f32              = NULL;
	qglPrioritizeTextures        = NULL;
	qglPushAttrib                = NULL;
	qglPushClientAttrib          = NULL;
	qglPushName                  = NULL;
	qglRasterPos2d               = NULL;
	qglRasterPos2dv              = NULL;
	qglRasterPos2f               = NULL;
	qglRasterPos2fv              = NULL;
	qglRasterPos2i               = NULL;
	qglRasterPos2iv              = NULL;
	qglRasterPos2s               = NULL;
	qglRasterPos2sv              = NULL;
	qglRasterPos3d               = NULL;
	qglRasterPos3dv              = NULL;
	qglRasterPos3f               = NULL;
	qglRasterPos3fv              = NULL;
	qglRasterPos3i               = NULL;
	qglRasterPos3iv              = NULL;
	qglRasterPos3s               = NULL;
	qglRasterPos3sv              = NULL;
	qglRasterPos4d               = NULL;
	qglRasterPos4dv              = NULL;
	qglRasterPos4f               = NULL;
	qglRasterPos4fv              = NULL;
	qglRasterPos4i               = NULL;
	qglRasterPos4iv              = NULL;
	qglRasterPos4s               = NULL;
	qglRasterPos4sv              = NULL;
	qglReadBuffer                = NULL;
	qglReadPixels                = NULL;
	qglRectd                     = NULL;
	qglRectdv                    = NULL;
	qglRectf                     = NULL;
	qglRectfv                    = NULL;
	qglRecti                     = NULL;
	qglRectiv                    = NULL;
	qglRects                     = NULL;
	qglRectsv                    = NULL;
	qglRenderMode                = NULL;
	qglScissor                   = NULL;
	qglSelectBuffer              = NULL;
	qgxSetAlphaCompare           = NULL;
	qgxSetBlendMode              = NULL;
	qgxSetCullMode               = NULL;
	qgxSetTevOp                  = NULL;
	qgxSetViewport               = NULL;
	qgxSetZMode                  = NULL;
	qglShadeModel                = NULL;
	qglStencilFunc               = NULL;
	qglStencilMask               = NULL;
	qglStencilOp                 = NULL;
	qgxTexCoord2f32              = NULL;
	qglTexCoordPointer           = NULL;
	qglTexGend                   = NULL;
	qglTexGendv                  = NULL;
	qglTexGenf                   = NULL;
	qglTexGenfv                  = NULL;
	qglTexGeni                   = NULL;
	qglTexGeniv                  = NULL;
	qglVertexPointer             = NULL;

	qglColorTableEXT             = NULL;

}

/*
** QGL_Init
**
** This is responsible for binding our qgl function pointers to 
** the appropriate GL stuff.  In Windows this means doing a 
** LoadLibrary and a bunch of calls to GetProcAddress.  On other
** operating systems we need to do the right thing, whatever that
** might be.
** 
*/
qboolean QGL_Init( const char *dllname )
{
	gx_config.allow_cds = true;

	qguMtxConcat                 = dllMtxConcat = guMtxConcat;
	qguMtxCopy                   = dllMtxCopy = guMtxCopy;
	qguMtxIdentity               = dllMtxIdentity = guMtxIdentity;
	qguMtxRotAxisDeg             = dllMtxRotAxisDeg = GXU_CallguMtxRotAxisDeg;
	qguMtxScale                  = dllMtxScale = guMtxScale;
	qguOrtho                     = dllOrtho = guOrtho;
	qguPerspective               = dllPerspective = guPerspective;
	qguMtxTrans                  = dllMtxTrans = guMtxTrans;
	qglAccum                     = dllAccum = glAccum;
	qglAreTexturesResident       = dllAreTexturesResident = glAreTexturesResident;
	qglArrayElement              = dllArrayElement = glArrayElement;
	qgxBegin                     = dllBegin = GX_Begin;
	qglBitmap                    = dllBitmap = glBitmap;
	qglCallList                  = dllCallList = glCallList;
	qglCallLists                 = dllCallLists = glCallLists;
	qglClear                     = dllClear = glClear;
	qglClearAccum                = dllClearAccum = glClearAccum;
	qglClearColor                = dllClearColor = glClearColor;
	qglClearDepth                = dllClearDepth = glClearDepth;
	qglClearIndex                = dllClearIndex = glClearIndex;
	qglClearStencil              = dllClearStencil = glClearStencil;
	qglClipPlane                 = dllClipPlane = glClipPlane;
	qgxColor4u8                  = dllColor4u8 = GXU_CallGXColor4u8;
	qglColorMask                 = dllColorMask = glColorMask;
	qglColorMaterial             = dllColorMaterial = glColorMaterial;
	qglColorPointer              = dllColorPointer = glColorPointer;
	qglCopyPixels                = dllCopyPixels = glCopyPixels;
	qglCopyTexImage1D            = dllCopyTexImage1D = glCopyTexImage1D;
	qglCopyTexImage2D            = dllCopyTexImage2D = glCopyTexImage2D;
	qglCopyTexSubImage1D         = dllCopyTexSubImage1D = glCopyTexSubImage1D;
	qglCopyTexSubImage2D         = dllCopyTexSubImage2D = glCopyTexSubImage2D;
	qglDeleteLists               = dllDeleteLists = glDeleteLists;
	qglDeleteTextures            = dllDeleteTextures = glDeleteTextures;
	qglDisable                   = dllDisable = glDisable;
	qglDisableClientState        = dllDisableClientState = glDisableClientState;
	qgxDisableTexture            = dllDisableTexture = GXU_DisableTexture;
	qglDrawArrays                = dllDrawArrays = glDrawArrays;
	qglDrawBuffer                = dllDrawBuffer = glDrawBuffer;
	qglDrawElements              = dllDrawElements = glDrawElements;
	qglDrawPixels                = dllDrawPixels = glDrawPixels;
	qglEdgeFlag                  = dllEdgeFlag = glEdgeFlag;
	qglEdgeFlagPointer           = dllEdgeFlagPointer = glEdgeFlagPointer;
	qglEdgeFlagv                 = dllEdgeFlagv = glEdgeFlagv;
	qglEnable                    = 	dllEnable                    = glEnable;
	qglEnableClientState         = 	dllEnableClientState         = glEnableClientState;
	qgxEnableTexture             = 	dllEnableTexture             = GXU_EnableTexture;
	qgxEnd                       = 	dllEnd                       = GXU_CallGXEnd;
	qglEndList                   = 	dllEndList                   = glEndList;
	qglEvalCoord1d				 = 	dllEvalCoord1d				 = glEvalCoord1d;
	qglEvalCoord1dv              = 	dllEvalCoord1dv              = glEvalCoord1dv;
	qglEvalCoord1f               = 	dllEvalCoord1f               = glEvalCoord1f;
	qglEvalCoord1fv              = 	dllEvalCoord1fv              = glEvalCoord1fv;
	qglEvalCoord2d               = 	dllEvalCoord2d               = glEvalCoord2d;
	qglEvalCoord2dv              = 	dllEvalCoord2dv              = glEvalCoord2dv;
	qglEvalCoord2f               = 	dllEvalCoord2f               = glEvalCoord2f;
	qglEvalCoord2fv              = 	dllEvalCoord2fv              = glEvalCoord2fv;
	qglEvalMesh1                 = 	dllEvalMesh1                 = glEvalMesh1;
	qglEvalMesh2                 = 	dllEvalMesh2                 = glEvalMesh2;
	qglEvalPoint1                = 	dllEvalPoint1                = glEvalPoint1;
	qglEvalPoint2                = 	dllEvalPoint2                = glEvalPoint2;
	qglFeedbackBuffer            = 	dllFeedbackBuffer            = glFeedbackBuffer;
	qglFinish                    = 	dllFinish                    = glFinish;
	qglFlush                     = 	dllFlush                     = glFlush;
	qglFogf                      = 	dllFogf                      = glFogf;
	qglFogfv                     = 	dllFogfv                     = glFogfv;
	qglFogi                      = 	dllFogi                      = glFogi;
	qglFogiv                     = 	dllFogiv                     = glFogiv;
	qglFrontFace                 = 	dllFrontFace                 = glFrontFace;
	qglGenLists                  = 	dllGenLists                  = glGenLists;
	qglGenTextures               = 	dllGenTextures               = glGenTextures;
	qglGetBooleanv               = 	dllGetBooleanv               = glGetBooleanv;
	qglGetClipPlane              = 	dllGetClipPlane              = glGetClipPlane;
	qglGetDoublev                = 	dllGetDoublev                = glGetDoublev;
	qglGetError                  = 	dllGetError                  = glGetError;
	qglGetIntegerv               = 	dllGetIntegerv               = glGetIntegerv;
	qglGetLightfv                = 	dllGetLightfv                = glGetLightfv;
	qglGetLightiv                = 	dllGetLightiv                = glGetLightiv;
	qglGetMapdv                  = 	dllGetMapdv                  = glGetMapdv;
	qglGetMapfv                  = 	dllGetMapfv                  = glGetMapfv;
	qglGetMapiv                  = 	dllGetMapiv                  = glGetMapiv;
	qglGetMaterialfv             = 	dllGetMaterialfv             = glGetMaterialfv;
	qglGetMaterialiv             = 	dllGetMaterialiv             = glGetMaterialiv;
	qglGetPixelMapfv             = 	dllGetPixelMapfv             = glGetPixelMapfv;
	qglGetPixelMapuiv            = 	dllGetPixelMapuiv            = glGetPixelMapuiv;
	qglGetPixelMapusv            = 	dllGetPixelMapusv            = glGetPixelMapusv;
	qglGetPointerv               = 	dllGetPointerv               = glGetPointerv;
	qglGetPolygonStipple         = 	dllGetPolygonStipple         = glGetPolygonStipple;
	qglGetString                 = 	dllGetString                 = glGetString;
	qglGetTexEnvfv               = 	dllGetTexEnvfv               = glGetTexEnvfv;
	qglGetTexEnviv               = 	dllGetTexEnviv               = glGetTexEnviv;
	qglGetTexGendv               = 	dllGetTexGendv               = glGetTexGendv;
	qglGetTexGenfv               = 	dllGetTexGenfv               = glGetTexGenfv;
	qglGetTexGeniv               = 	dllGetTexGeniv               = glGetTexGeniv;
	qglGetTexImage               = 	dllGetTexImage               = glGetTexImage;
//	qglGetTexLevelParameterfv    = 	dllGetTexLevelParameterfv    = glGetLevelParameterfv;
//	qglGetTexLevelParameteriv    = 	dllGetTexLevelParameteriv    = glGetLevelParameteriv;
	qglGetTexParameterfv         = 	dllGetTexParameterfv         = glGetTexParameterfv;
	qglGetTexParameteriv         = 	dllGetTexParameteriv         = glGetTexParameteriv;
	qglHint                      = 	dllHint                      = glHint;
	qglIndexMask                 = 	dllIndexMask                 = glIndexMask;
	qglIndexPointer              = 	dllIndexPointer              = glIndexPointer;
	qglIndexd                    = 	dllIndexd                    = glIndexd;
	qglIndexdv                   = 	dllIndexdv                   = glIndexdv;
	qglIndexf                    = 	dllIndexf                    = glIndexf;
	qglIndexfv                   = 	dllIndexfv                   = glIndexfv;
	qglIndexi                    = 	dllIndexi                    = glIndexi;
	qglIndexiv                   = 	dllIndexiv                   = glIndexiv;
	qglIndexs                    = 	dllIndexs                    = glIndexs;
	qglIndexsv                   = 	dllIndexsv                   = glIndexsv;
	qglIndexub                   = 	dllIndexub                   = glIndexub;
	qglIndexubv                  = 	dllIndexubv                  = glIndexubv;
	qglInitNames                 = 	dllInitNames                 = glInitNames;
	qgxInitTexObj                =  dllInitTexObj                = GX_InitTexObj;
	qgxInitTexObjFilterMode      =  dllInitTexObjFilterMode      = GX_InitTexObjFilterMode;
	qglInterleavedArrays         = 	dllInterleavedArrays         = glInterleavedArrays;
	qgxInvalidateTexAll          =  dllInvalidateTexAll          = GX_InvalidateTexAll;
	qglIsEnabled                 = 	dllIsEnabled                 = glIsEnabled;
	qglIsList                    = 	dllIsList                    = glIsList;
	qglIsTexture                 = 	dllIsTexture                 = glIsTexture;
	qglLightModelf               = 	dllLightModelf               = glLightModelf;
	qglLightModelfv              = 	dllLightModelfv              = glLightModelfv;
	qglLightModeli               = 	dllLightModeli               = glLightModeli;
	qglLightModeliv              = 	dllLightModeliv              = glLightModeliv;
	qglLightf                    = 	dllLightf                    = glLightf;
	qglLightfv                   = 	dllLightfv                   = glLightfv;
	qglLighti                    = 	dllLighti                    = glLighti;
	qglLightiv                   = 	dllLightiv                   = glLightiv;
	qglLineStipple               = 	dllLineStipple               = glLineStipple;
	qglLineWidth                 = 	dllLineWidth                 = glLineWidth;
	qglListBase                  = 	dllListBase                  = glListBase;
	qglLoadName                  = 	dllLoadName                  = glLoadName;
	qgxLoadPosMtxImm             =  dllLoadPosMtxImm             = GX_LoadPosMtxImm;
	qgxLoadProjectionMtx         =  dllLoadProjectionMtx         = GX_LoadProjectionMtx;
	qgxLoadTexObj                =  dllLoadTexObj                = GX_LoadTexObj;
	qglLogicOp                   = 	dllLogicOp                   = glLogicOp;
	qglMap1d                     = 	dllMap1d                     = glMap1d;
	qglMap1f                     = 	dllMap1f                     = glMap1f;
	qglMap2d                     = 	dllMap2d                     = glMap2d;
	qglMap2f                     = 	dllMap2f                     = glMap2f;
	qglMapGrid1d                 = 	dllMapGrid1d                 = glMapGrid1d;
	qglMapGrid1f                 = 	dllMapGrid1f                 = glMapGrid1f;
	qglMapGrid2d                 = 	dllMapGrid2d                 = glMapGrid2d;
	qglMapGrid2f                 = 	dllMapGrid2f                 = glMapGrid2f;
	qglMaterialf                 = 	dllMaterialf                 = glMaterialf;
	qglMaterialfv                = 	dllMaterialfv                = glMaterialfv;
	qglMateriali                 = 	dllMateriali                 = glMateriali;
	qglMaterialiv                = 	dllMaterialiv                = glMaterialiv;
	qglNewList                   = 	dllNewList                   = glNewList;
	qglNormal3b                  = 	dllNormal3b                  = glNormal3b;
	qglNormal3bv                 = 	dllNormal3bv                 = glNormal3bv;
	qglNormal3d                  = 	dllNormal3d                  = glNormal3d;
	qglNormal3dv                 = 	dllNormal3dv                 = glNormal3dv;
	qglNormal3f                  = 	dllNormal3f                  = glNormal3f;
	qglNormal3fv                 = 	dllNormal3fv                 = glNormal3fv;
	qglNormal3i                  = 	dllNormal3i                  = glNormal3i;
	qglNormal3iv                 = 	dllNormal3iv                 = glNormal3iv;
	qglNormal3s                  = 	dllNormal3s                  = glNormal3s;
	qglNormal3sv                 = 	dllNormal3sv                 = glNormal3sv;
	qglNormalPointer             = 	dllNormalPointer             = glNormalPointer;
	qglPassThrough               = 	dllPassThrough               = glPassThrough;
	qglPixelMapfv                = 	dllPixelMapfv                = glPixelMapfv;
	qglPixelMapuiv               = 	dllPixelMapuiv               = glPixelMapuiv;
	qglPixelMapusv               = 	dllPixelMapusv               = glPixelMapusv;
	qglPixelStoref               = 	dllPixelStoref               = glPixelStoref;
	qglPixelStorei               = 	dllPixelStorei               = glPixelStorei;
	qglPixelTransferf            = 	dllPixelTransferf            = glPixelTransferf;
	qglPixelTransferi            = 	dllPixelTransferi            = glPixelTransferi;
	qglPixelZoom                 = 	dllPixelZoom                 = glPixelZoom;
	qglPointSize                 = 	dllPointSize                 = glPointSize;
	qglPolygonMode               = 	dllPolygonMode               = glPolygonMode;
	qglPolygonOffset             = 	dllPolygonOffset             = glPolygonOffset;
	qglPolygonStipple            = 	dllPolygonStipple            = glPolygonStipple;
	qglPopAttrib                 = 	dllPopAttrib                 = glPopAttrib;
	qglPopClientAttrib           = 	dllPopClientAttrib           = glPopClientAttrib;
	qglPopName                   = 	dllPopName                   = glPopName;
	qgxPosition3f32              = 	dllPosition3f32              = GXU_CallGXPosition3f32;
	qglPrioritizeTextures        = 	dllPrioritizeTextures        = glPrioritizeTextures;
	qglPushAttrib                = 	dllPushAttrib                = glPushAttrib;
	qglPushClientAttrib          = 	dllPushClientAttrib          = glPushClientAttrib;
	qglPushName                  = 	dllPushName                  = glPushName;
	qglRasterPos2d               = 	dllRasterPos2d               = glRasterPos2d;
	qglRasterPos2dv              = 	dllRasterPos2dv              = glRasterPos2dv;
	qglRasterPos2f               = 	dllRasterPos2f               = glRasterPos2f;
	qglRasterPos2fv              = 	dllRasterPos2fv              = glRasterPos2fv;
	qglRasterPos2i               = 	dllRasterPos2i               = glRasterPos2i;
	qglRasterPos2iv              = 	dllRasterPos2iv              = glRasterPos2iv;
	qglRasterPos2s               = 	dllRasterPos2s               = glRasterPos2s;
	qglRasterPos2sv              = 	dllRasterPos2sv              = glRasterPos2sv;
	qglRasterPos3d               = 	dllRasterPos3d               = glRasterPos3d;
	qglRasterPos3dv              = 	dllRasterPos3dv              = glRasterPos3dv;
	qglRasterPos3f               = 	dllRasterPos3f               = glRasterPos3f;
	qglRasterPos3fv              = 	dllRasterPos3fv              = glRasterPos3fv;
	qglRasterPos3i               = 	dllRasterPos3i               = glRasterPos3i;
	qglRasterPos3iv              = 	dllRasterPos3iv              = glRasterPos3iv;
	qglRasterPos3s               = 	dllRasterPos3s               = glRasterPos3s;
	qglRasterPos3sv              = 	dllRasterPos3sv              = glRasterPos3sv;
	qglRasterPos4d               = 	dllRasterPos4d               = glRasterPos4d;
	qglRasterPos4dv              = 	dllRasterPos4dv              = glRasterPos4dv;
	qglRasterPos4f               = 	dllRasterPos4f               = glRasterPos4f;
	qglRasterPos4fv              = 	dllRasterPos4fv              = glRasterPos4fv;
	qglRasterPos4i               = 	dllRasterPos4i               = glRasterPos4i;
	qglRasterPos4iv              = 	dllRasterPos4iv              = glRasterPos4iv;
	qglRasterPos4s               = 	dllRasterPos4s               = glRasterPos4s;
	qglRasterPos4sv              = 	dllRasterPos4sv              = glRasterPos4sv;
	qglReadBuffer                = 	dllReadBuffer                = glReadBuffer;
	qglReadPixels                = 	dllReadPixels                = glReadPixels;
	qglRectd                     = 	dllRectd                     = glRectd;
	qglRectdv                    = 	dllRectdv                    = glRectdv;
	qglRectf                     = 	dllRectf                     = glRectf;
	qglRectfv                    = 	dllRectfv                    = glRectfv;
	qglRecti                     = 	dllRecti                     = glRecti;
	qglRectiv                    = 	dllRectiv                    = glRectiv;
	qglRects                     = 	dllRects                     = glRects;
	qglRectsv                    = 	dllRectsv                    = glRectsv;
	qglRenderMode                = 	dllRenderMode                = glRenderMode;
	qglScissor                   = 	dllScissor                   = glScissor;
	qglSelectBuffer              = 	dllSelectBuffer              = glSelectBuffer;
	qgxSetAlphaCompare           =  dllSetAlphaCompare           = GX_SetAlphaCompare;
	qgxSetBlendMode              = 	dllSetBlendMode              = GX_SetBlendMode;
	qgxSetCullMode               =  dllSetCullMode               = GX_SetCullMode;
	qgxSetTevOp                  = 	dllSetTevOp                  = GX_SetTevOp;
	qgxSetViewport               = 	dllSetViewport               = GX_SetViewport;
	qgxSetZMode                  = 	dllSetZMode                  = GX_SetZMode;
	qglShadeModel                = 	dllShadeModel                = glShadeModel;
	qglStencilFunc               = 	dllStencilFunc               = glStencilFunc;
	qglStencilMask               = 	dllStencilMask               = glStencilMask;
	qglStencilOp                 = 	dllStencilOp                 = glStencilOp;
	qgxTexCoord2f32              = 	dllTexCoord2f32              = GXU_CallGXTexCoord2f32;
	qglTexCoordPointer           = 	dllTexCoordPointer           = glTexCoordPointer;
	qglTexGend                   = 	dllTexGend                   = glTexGend;
	qglTexGendv                  = 	dllTexGendv                  = glTexGendv;
	qglTexGenf                   = 	dllTexGenf                   = glTexGenf;
	qglTexGenfv                  = 	dllTexGenfv                  = glTexGenfv;
	qglTexGeni                   = 	dllTexGeni                   = glTexGeni;
	qglTexGeniv                  = 	dllTexGeniv                  = glTexGeniv;
	qglVertexPointer             = 	dllVertexPointer             = glVertexPointer;

	qglLockArraysEXT = 0;
	qglUnlockArraysEXT = 0;

	qglPointParameterfEXT = 0;
	qglPointParameterfvEXT = 0;
	qglColorTableEXT = 0;
	qglColorTableEXT = 0;
	qglSelectTextureSGIS = 0;
	qglMTexCoord2fSGIS = 0;
	qglActiveTextureARB = 0;
	qglClientActiveTextureARB = 0;

	return true;
}

void GXimp_EnableLogging( qboolean enable )
{
	if ( enable )
	{
		if ( !log_fp )
		{
			struct tm *newtime;
			time_t aclock;
			char buffer[1024];

			time( &aclock );
			newtime = localtime( &aclock );

			asctime( newtime );

			sprintf( buffer, "%s/gl.log", ri.FS_Gamedir() ); 
			log_fp = fopen( buffer, "wt");

			fprintf( log_fp, "%s\n", asctime( newtime ) );
		}

		qguMtxConcat                 = logMtxConcat;
		qguMtxCopy                   = logMtxCopy;
		qguMtxIdentity               = logMtxIdentity;
		qguMtxRotAxisDeg             = logMtxRotAxisDeg;
		qguMtxScale                  = logMtxScale;
		qguMtxTrans                  = logMtxTrans;
		qguOrtho                     = logOrtho;
		qguPerspective               = logPerspective;
		qglAccum                     = logAccum;
		qglAreTexturesResident       = logAreTexturesResident;
		qglArrayElement              = logArrayElement;
		qgxBegin                     = logBegin;
		qglBitmap                    = logBitmap;
		qglCallList                  = logCallList;
		qglCallLists                 = logCallLists;
		qglClear                     = logClear;
		qglClearAccum                = logClearAccum;
		qglClearColor                = logClearColor;
		qglClearDepth                = logClearDepth;
		qglClearIndex                = logClearIndex;
		qglClearStencil              = logClearStencil;
		qglClipPlane                 = logClipPlane;
		qgxColor4u8                  = logColor4u8;
		qglColorMask                 = logColorMask;
		qglColorMaterial             = logColorMaterial;
		qglColorPointer              = logColorPointer;
		qglCopyPixels                = logCopyPixels;
		qglCopyTexImage1D            = logCopyTexImage1D;
		qglCopyTexImage2D            = logCopyTexImage2D;
		qglCopyTexSubImage1D         = logCopyTexSubImage1D;
		qglCopyTexSubImage2D         = logCopyTexSubImage2D;
		qglDeleteLists               = logDeleteLists ;
		qglDeleteTextures            = logDeleteTextures ;
		qglDisable                   = logDisable ;
		qglDisableClientState        = logDisableClientState ;
		qgxDisableTexture            = logDisableTexture ;
		qglDrawArrays                = logDrawArrays ;
		qglDrawBuffer                = logDrawBuffer ;
		qglDrawElements              = logDrawElements ;
		qglDrawPixels                = logDrawPixels ;
		qglEdgeFlag                  = logEdgeFlag ;
		qglEdgeFlagPointer           = logEdgeFlagPointer ;
		qglEdgeFlagv                 = logEdgeFlagv ;
		qglEnable                    = 	logEnable                    ;
		qglEnableClientState         = 	logEnableClientState         ;
		qgxEnableTexture             = 	logEnableTexture             ;
		qgxEnd                       = 	logEnd                       ;
		qglEndList                   = 	logEndList                   ;
		qglEvalCoord1d				 = 	logEvalCoord1d				 ;
		qglEvalCoord1dv              = 	logEvalCoord1dv              ;
		qglEvalCoord1f               = 	logEvalCoord1f               ;
		qglEvalCoord1fv              = 	logEvalCoord1fv              ;
		qglEvalCoord2d               = 	logEvalCoord2d               ;
		qglEvalCoord2dv              = 	logEvalCoord2dv              ;
		qglEvalCoord2f               = 	logEvalCoord2f               ;
		qglEvalCoord2fv              = 	logEvalCoord2fv              ;
		qglEvalMesh1                 = 	logEvalMesh1                 ;
		qglEvalMesh2                 = 	logEvalMesh2                 ;
		qglEvalPoint1                = 	logEvalPoint1                ;
		qglEvalPoint2                = 	logEvalPoint2                ;
		qglFeedbackBuffer            = 	logFeedbackBuffer            ;
		qglFinish                    = 	logFinish                    ;
		qglFlush                     = 	logFlush                     ;
		qglFogf                      = 	logFogf                      ;
		qglFogfv                     = 	logFogfv                     ;
		qglFogi                      = 	logFogi                      ;
		qglFogiv                     = 	logFogiv                     ;
		qglFrontFace                 = 	logFrontFace                 ;
		qglGenLists                  = 	logGenLists                  ;
		qglGenTextures               = 	logGenTextures               ;
		qglGetBooleanv               = 	logGetBooleanv               ;
		qglGetClipPlane              = 	logGetClipPlane              ;
		qglGetDoublev                = 	logGetDoublev                ;
		qglGetError                  = 	logGetError                  ;
		qglGetIntegerv               = 	logGetIntegerv               ;
		qglGetLightfv                = 	logGetLightfv                ;
		qglGetLightiv                = 	logGetLightiv                ;
		qglGetMapdv                  = 	logGetMapdv                  ;
		qglGetMapfv                  = 	logGetMapfv                  ;
		qglGetMapiv                  = 	logGetMapiv                  ;
		qglGetMaterialfv             = 	logGetMaterialfv             ;
		qglGetMaterialiv             = 	logGetMaterialiv             ;
		qglGetPixelMapfv             = 	logGetPixelMapfv             ;
		qglGetPixelMapuiv            = 	logGetPixelMapuiv            ;
		qglGetPixelMapusv            = 	logGetPixelMapusv            ;
		qglGetPointerv               = 	logGetPointerv               ;
		qglGetPolygonStipple         = 	logGetPolygonStipple         ;
		qglGetString                 = 	logGetString                 ;
		qglGetTexEnvfv               = 	logGetTexEnvfv               ;
		qglGetTexEnviv               = 	logGetTexEnviv               ;
		qglGetTexGendv               = 	logGetTexGendv               ;
		qglGetTexGenfv               = 	logGetTexGenfv               ;
		qglGetTexGeniv               = 	logGetTexGeniv               ;
		qglGetTexImage               = 	logGetTexImage               ;
//		qglGetTexLevelParameterfv    = 	logGetTexLevelParameterfv    ;
//		qglGetTexLevelParameteriv    = 	logGetTexLevelParameteriv    ;
		qglGetTexParameterfv         = 	logGetTexParameterfv         ;
		qglGetTexParameteriv         = 	logGetTexParameteriv         ;
		qglHint                      = 	logHint                      ;
		qglIndexMask                 = 	logIndexMask                 ;
		qglIndexPointer              = 	logIndexPointer              ;
		qglIndexd                    = 	logIndexd                    ;
		qglIndexdv                   = 	logIndexdv                   ;
		qglIndexf                    = 	logIndexf                    ;
		qglIndexfv                   = 	logIndexfv                   ;
		qglIndexi                    = 	logIndexi                    ;
		qglIndexiv                   = 	logIndexiv                   ;
		qglIndexs                    = 	logIndexs                    ;
		qglIndexsv                   = 	logIndexsv                   ;
		qglIndexub                   = 	logIndexub                   ;
		qglIndexubv                  = 	logIndexubv                  ;
		qglInitNames                 = 	logInitNames                 ;
		qgxInitTexObj                =  logInitTexObj                ;
		qgxInitTexObjFilterMode      =  logInitTexObjFilterMode      ;
		qglInterleavedArrays         = 	logInterleavedArrays         ;
		qgxInvalidateTexAll          =  logInvalidateTexAll          ;
		qglIsEnabled                 = 	logIsEnabled                 ;
		qglIsList                    = 	logIsList                    ;
		qglIsTexture                 = 	logIsTexture                 ;
		qglLightModelf               = 	logLightModelf               ;
		qglLightModelfv              = 	logLightModelfv              ;
		qglLightModeli               = 	logLightModeli               ;
		qglLightModeliv              = 	logLightModeliv              ;
		qglLightf                    = 	logLightf                    ;
		qglLightfv                   = 	logLightfv                   ;
		qglLighti                    = 	logLighti                    ;
		qglLightiv                   = 	logLightiv                   ;
		qglLineStipple               = 	logLineStipple               ;
		qglLineWidth                 = 	logLineWidth                 ;
		qglListBase                  = 	logListBase                  ;
		qglLoadName                  = 	logLoadName                  ;
		qgxLoadPosMtxImm             =  logLoadPosMtxImm             ;
		qgxLoadProjectionMtx         =  logLoadProjectionMtx         ;
		qgxLoadTexObj                =  logLoadTexObj                ;
		qglLogicOp                   = 	logLogicOp                   ;
		qglMap1d                     = 	logMap1d                     ;
		qglMap1f                     = 	logMap1f                     ;
		qglMap2d                     = 	logMap2d                     ;
		qglMap2f                     = 	logMap2f                     ;
		qglMapGrid1d                 = 	logMapGrid1d                 ;
		qglMapGrid1f                 = 	logMapGrid1f                 ;
		qglMapGrid2d                 = 	logMapGrid2d                 ;
		qglMapGrid2f                 = 	logMapGrid2f                 ;
		qglMaterialf                 = 	logMaterialf                 ;
		qglMaterialfv                = 	logMaterialfv                ;
		qglMateriali                 = 	logMateriali                 ;
		qglMaterialiv                = 	logMaterialiv                ;
		qglNewList                   = 	logNewList                   ;
		qglNormal3b                  = 	logNormal3b                  ;
		qglNormal3bv                 = 	logNormal3bv                 ;
		qglNormal3d                  = 	logNormal3d                  ;
		qglNormal3dv                 = 	logNormal3dv                 ;
		qglNormal3f                  = 	logNormal3f                  ;
		qglNormal3fv                 = 	logNormal3fv                 ;
		qglNormal3i                  = 	logNormal3i                  ;
		qglNormal3iv                 = 	logNormal3iv                 ;
		qglNormal3s                  = 	logNormal3s                  ;
		qglNormal3sv                 = 	logNormal3sv                 ;
		qglNormalPointer             = 	logNormalPointer             ;
		qglPassThrough               = 	logPassThrough               ;
		qglPixelMapfv                = 	logPixelMapfv                ;
		qglPixelMapuiv               = 	logPixelMapuiv               ;
		qglPixelMapusv               = 	logPixelMapusv               ;
		qglPixelStoref               = 	logPixelStoref               ;
		qglPixelStorei               = 	logPixelStorei               ;
		qglPixelTransferf            = 	logPixelTransferf            ;
		qglPixelTransferi            = 	logPixelTransferi            ;
		qglPixelZoom                 = 	logPixelZoom                 ;
		qglPointSize                 = 	logPointSize                 ;
		qglPolygonMode               = 	logPolygonMode               ;
		qglPolygonOffset             = 	logPolygonOffset             ;
		qglPolygonStipple            = 	logPolygonStipple            ;
		qglPopAttrib                 = 	logPopAttrib                 ;
		qglPopClientAttrib           = 	logPopClientAttrib           ;
		qglPopName                   = 	logPopName                   ;
		qgxPosition3f32              = 	logPosition3f32              ;
		qglPrioritizeTextures        = 	logPrioritizeTextures        ;
		qglPushAttrib                = 	logPushAttrib                ;
		qglPushClientAttrib          = 	logPushClientAttrib          ;
		qglPushName                  = 	logPushName                  ;
		qglRasterPos2d               = 	logRasterPos2d               ;
		qglRasterPos2dv              = 	logRasterPos2dv              ;
		qglRasterPos2f               = 	logRasterPos2f               ;
		qglRasterPos2fv              = 	logRasterPos2fv              ;
		qglRasterPos2i               = 	logRasterPos2i               ;
		qglRasterPos2iv              = 	logRasterPos2iv              ;
		qglRasterPos2s               = 	logRasterPos2s               ;
		qglRasterPos2sv              = 	logRasterPos2sv              ;
		qglRasterPos3d               = 	logRasterPos3d               ;
		qglRasterPos3dv              = 	logRasterPos3dv              ;
		qglRasterPos3f               = 	logRasterPos3f               ;
		qglRasterPos3fv              = 	logRasterPos3fv              ;
		qglRasterPos3i               = 	logRasterPos3i               ;
		qglRasterPos3iv              = 	logRasterPos3iv              ;
		qglRasterPos3s               = 	logRasterPos3s               ;
		qglRasterPos3sv              = 	logRasterPos3sv              ;
		qglRasterPos4d               = 	logRasterPos4d               ;
		qglRasterPos4dv              = 	logRasterPos4dv              ;
		qglRasterPos4f               = 	logRasterPos4f               ;
		qglRasterPos4fv              = 	logRasterPos4fv              ;
		qglRasterPos4i               = 	logRasterPos4i               ;
		qglRasterPos4iv              = 	logRasterPos4iv              ;
		qglRasterPos4s               = 	logRasterPos4s               ;
		qglRasterPos4sv              = 	logRasterPos4sv              ;
		qglReadBuffer                = 	logReadBuffer                ;
		qglReadPixels                = 	logReadPixels                ;
		qglRectd                     = 	logRectd                     ;
		qglRectdv                    = 	logRectdv                    ;
		qglRectf                     = 	logRectf                     ;
		qglRectfv                    = 	logRectfv                    ;
		qglRecti                     = 	logRecti                     ;
		qglRectiv                    = 	logRectiv                    ;
		qglRects                     = 	logRects                     ;
		qglRectsv                    = 	logRectsv                    ;
		qglRenderMode                = 	logRenderMode                ;
		qglScissor                   = 	logScissor                   ;
		qglSelectBuffer              = 	logSelectBuffer              ;
		qgxSetAlphaCompare           =  logSetAlphaCompare           ;
		qgxSetBlendMode              = 	logSetBlendMode              ;
		qgxSetCullMode               = 	logSetCullMode               ;
		qgxSetTevOp                  = 	logSetTevOp                  ;
		qgxSetViewport               = 	logSetViewport               ;
		qgxSetZMode                  = 	logSetZMode                  ;
		qglShadeModel                = 	logShadeModel                ;
		qglStencilFunc               = 	logStencilFunc               ;
		qglStencilMask               = 	logStencilMask               ;
		qglStencilOp                 = 	logStencilOp                 ;
		qgxTexCoord2f32              = 	logTexCoord2f32              ;
		qglTexCoordPointer           = 	logTexCoordPointer           ;
		qglTexGend                   = 	logTexGend                   ;
		qglTexGendv                  = 	logTexGendv                  ;
		qglTexGenf                   = 	logTexGenf                   ;
		qglTexGenfv                  = 	logTexGenfv                  ;
		qglTexGeni                   = 	logTexGeni                   ;
		qglTexGeniv                  = 	logTexGeniv                  ;
		qglVertexPointer             = 	logVertexPointer             ;
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
		qglAccum                     = dllAccum;
		qglAreTexturesResident       = dllAreTexturesResident;
		qglArrayElement              = dllArrayElement;
		qgxBegin                     = dllBegin;
		qglBitmap                    = dllBitmap;
		qglCallList                  = dllCallList;
		qglCallLists                 = dllCallLists;
		qglClear                     = dllClear;
		qglClearAccum                = dllClearAccum;
		qglClearColor                = dllClearColor;
		qglClearDepth                = dllClearDepth;
		qglClearIndex                = dllClearIndex;
		qglClearStencil              = dllClearStencil;
		qglClipPlane                 = dllClipPlane;
		qgxColor4u8                  = dllColor4u8;
		qglColorMask                 = dllColorMask;
		qglColorMaterial             = dllColorMaterial;
		qglColorPointer              = dllColorPointer;
		qglCopyPixels                = dllCopyPixels;
		qglCopyTexImage1D            = dllCopyTexImage1D;
		qglCopyTexImage2D            = dllCopyTexImage2D;
		qglCopyTexSubImage1D         = dllCopyTexSubImage1D;
		qglCopyTexSubImage2D         = dllCopyTexSubImage2D;
		qglDeleteLists               = dllDeleteLists ;
		qglDeleteTextures            = dllDeleteTextures ;
		qglDisable                   = dllDisable ;
		qglDisableClientState        = dllDisableClientState ;
		qgxDisableTexture            = dllDisableTexture ;
		qglDrawArrays                = dllDrawArrays ;
		qglDrawBuffer                = dllDrawBuffer ;
		qglDrawElements              = dllDrawElements ;
		qglDrawPixels                = dllDrawPixels ;
		qglEdgeFlag                  = dllEdgeFlag ;
		qglEdgeFlagPointer           = dllEdgeFlagPointer ;
		qglEdgeFlagv                 = dllEdgeFlagv ;
		qglEnable                    = 	dllEnable                    ;
		qglEnableClientState         = 	dllEnableClientState         ;
		qgxEnableTexture             = 	dllEnableTexture             ;
		qgxEnd                       = 	dllEnd                       ;
		qglEndList                   = 	dllEndList                   ;
		qglEvalCoord1d				 = 	dllEvalCoord1d				 ;
		qglEvalCoord1dv              = 	dllEvalCoord1dv              ;
		qglEvalCoord1f               = 	dllEvalCoord1f               ;
		qglEvalCoord1fv              = 	dllEvalCoord1fv              ;
		qglEvalCoord2d               = 	dllEvalCoord2d               ;
		qglEvalCoord2dv              = 	dllEvalCoord2dv              ;
		qglEvalCoord2f               = 	dllEvalCoord2f               ;
		qglEvalCoord2fv              = 	dllEvalCoord2fv              ;
		qglEvalMesh1                 = 	dllEvalMesh1                 ;
		qglEvalMesh2                 = 	dllEvalMesh2                 ;
		qglEvalPoint1                = 	dllEvalPoint1                ;
		qglEvalPoint2                = 	dllEvalPoint2                ;
		qglFeedbackBuffer            = 	dllFeedbackBuffer            ;
		qglFinish                    = 	dllFinish                    ;
		qglFlush                     = 	dllFlush                     ;
		qglFogf                      = 	dllFogf                      ;
		qglFogfv                     = 	dllFogfv                     ;
		qglFogi                      = 	dllFogi                      ;
		qglFogiv                     = 	dllFogiv                     ;
		qglFrontFace                 = 	dllFrontFace                 ;
		qglGenLists                  = 	dllGenLists                  ;
		qglGenTextures               = 	dllGenTextures               ;
		qglGetBooleanv               = 	dllGetBooleanv               ;
		qglGetClipPlane              = 	dllGetClipPlane              ;
		qglGetDoublev                = 	dllGetDoublev                ;
		qglGetError                  = 	dllGetError                  ;
		qglGetIntegerv               = 	dllGetIntegerv               ;
		qglGetLightfv                = 	dllGetLightfv                ;
		qglGetLightiv                = 	dllGetLightiv                ;
		qglGetMapdv                  = 	dllGetMapdv                  ;
		qglGetMapfv                  = 	dllGetMapfv                  ;
		qglGetMapiv                  = 	dllGetMapiv                  ;
		qglGetMaterialfv             = 	dllGetMaterialfv             ;
		qglGetMaterialiv             = 	dllGetMaterialiv             ;
		qglGetPixelMapfv             = 	dllGetPixelMapfv             ;
		qglGetPixelMapuiv            = 	dllGetPixelMapuiv            ;
		qglGetPixelMapusv            = 	dllGetPixelMapusv            ;
		qglGetPointerv               = 	dllGetPointerv               ;
		qglGetPolygonStipple         = 	dllGetPolygonStipple         ;
		qglGetString                 = 	dllGetString                 ;
		qglGetTexEnvfv               = 	dllGetTexEnvfv               ;
		qglGetTexEnviv               = 	dllGetTexEnviv               ;
		qglGetTexGendv               = 	dllGetTexGendv               ;
		qglGetTexGenfv               = 	dllGetTexGenfv               ;
		qglGetTexGeniv               = 	dllGetTexGeniv               ;
		qglGetTexImage               = 	dllGetTexImage               ;
		qglGetTexLevelParameterfv    = 	dllGetTexLevelParameterfv    ;
		qglGetTexLevelParameteriv    = 	dllGetTexLevelParameteriv    ;
		qglGetTexParameterfv         = 	dllGetTexParameterfv         ;
		qglGetTexParameteriv         = 	dllGetTexParameteriv         ;
		qglHint                      = 	dllHint                      ;
		qglIndexMask                 = 	dllIndexMask                 ;
		qglIndexPointer              = 	dllIndexPointer              ;
		qglIndexd                    = 	dllIndexd                    ;
		qglIndexdv                   = 	dllIndexdv                   ;
		qglIndexf                    = 	dllIndexf                    ;
		qglIndexfv                   = 	dllIndexfv                   ;
		qglIndexi                    = 	dllIndexi                    ;
		qglIndexiv                   = 	dllIndexiv                   ;
		qglIndexs                    = 	dllIndexs                    ;
		qglIndexsv                   = 	dllIndexsv                   ;
		qglIndexub                   = 	dllIndexub                   ;
		qglIndexubv                  = 	dllIndexubv                  ;
		qglInitNames                 = 	dllInitNames                 ;
		qgxInitTexObj                =  dllInitTexObj                ;
		qgxInitTexObjFilterMode      =  dllInitTexObjFilterMode      ;
		qglInterleavedArrays         = 	dllInterleavedArrays         ;
		qgxInvalidateTexAll          =  dllInvalidateTexAll          ;
		qglIsEnabled                 = 	dllIsEnabled                 ;
		qglIsList                    = 	dllIsList                    ;
		qglIsTexture                 = 	dllIsTexture                 ;
		qglLightModelf               = 	dllLightModelf               ;
		qglLightModelfv              = 	dllLightModelfv              ;
		qglLightModeli               = 	dllLightModeli               ;
		qglLightModeliv              = 	dllLightModeliv              ;
		qglLightf                    = 	dllLightf                    ;
		qglLightfv                   = 	dllLightfv                   ;
		qglLighti                    = 	dllLighti                    ;
		qglLightiv                   = 	dllLightiv                   ;
		qglLineStipple               = 	dllLineStipple               ;
		qglLineWidth                 = 	dllLineWidth                 ;
		qglListBase                  = 	dllListBase                  ;
		qglLoadName                  = 	dllLoadName                  ;
		qgxLoadPosMtxImm             =  dllLoadPosMtxImm             ;
		qgxLoadProjectionMtx         =  dllLoadProjectionMtx         ;
		qgxLoadTexObj                =  dllLoadTexObj                ;
		qglLogicOp                   = 	dllLogicOp                   ;
		qglMap1d                     = 	dllMap1d                     ;
		qglMap1f                     = 	dllMap1f                     ;
		qglMap2d                     = 	dllMap2d                     ;
		qglMap2f                     = 	dllMap2f                     ;
		qglMapGrid1d                 = 	dllMapGrid1d                 ;
		qglMapGrid1f                 = 	dllMapGrid1f                 ;
		qglMapGrid2d                 = 	dllMapGrid2d                 ;
		qglMapGrid2f                 = 	dllMapGrid2f                 ;
		qglMaterialf                 = 	dllMaterialf                 ;
		qglMaterialfv                = 	dllMaterialfv                ;
		qglMateriali                 = 	dllMateriali                 ;
		qglMaterialiv                = 	dllMaterialiv                ;
		qglNewList                   = 	dllNewList                   ;
		qglNormal3b                  = 	dllNormal3b                  ;
		qglNormal3bv                 = 	dllNormal3bv                 ;
		qglNormal3d                  = 	dllNormal3d                  ;
		qglNormal3dv                 = 	dllNormal3dv                 ;
		qglNormal3f                  = 	dllNormal3f                  ;
		qglNormal3fv                 = 	dllNormal3fv                 ;
		qglNormal3i                  = 	dllNormal3i                  ;
		qglNormal3iv                 = 	dllNormal3iv                 ;
		qglNormal3s                  = 	dllNormal3s                  ;
		qglNormal3sv                 = 	dllNormal3sv                 ;
		qglNormalPointer             = 	dllNormalPointer             ;
		qglPassThrough               = 	dllPassThrough               ;
		qglPixelMapfv                = 	dllPixelMapfv                ;
		qglPixelMapuiv               = 	dllPixelMapuiv               ;
		qglPixelMapusv               = 	dllPixelMapusv               ;
		qglPixelStoref               = 	dllPixelStoref               ;
		qglPixelStorei               = 	dllPixelStorei               ;
		qglPixelTransferf            = 	dllPixelTransferf            ;
		qglPixelTransferi            = 	dllPixelTransferi            ;
		qglPixelZoom                 = 	dllPixelZoom                 ;
		qglPointSize                 = 	dllPointSize                 ;
		qglPolygonMode               = 	dllPolygonMode               ;
		qglPolygonOffset             = 	dllPolygonOffset             ;
		qglPolygonStipple            = 	dllPolygonStipple            ;
		qglPopAttrib                 = 	dllPopAttrib                 ;
		qglPopClientAttrib           = 	dllPopClientAttrib           ;
		qglPopName                   = 	dllPopName                   ;
		qgxPosition3f32              = 	dllPosition3f32              ;
		qglPrioritizeTextures        = 	dllPrioritizeTextures        ;
		qglPushAttrib                = 	dllPushAttrib                ;
		qglPushClientAttrib          = 	dllPushClientAttrib          ;
		qglPushName                  = 	dllPushName                  ;
		qglRasterPos2d               = 	dllRasterPos2d               ;
		qglRasterPos2dv              = 	dllRasterPos2dv              ;
		qglRasterPos2f               = 	dllRasterPos2f               ;
		qglRasterPos2fv              = 	dllRasterPos2fv              ;
		qglRasterPos2i               = 	dllRasterPos2i               ;
		qglRasterPos2iv              = 	dllRasterPos2iv              ;
		qglRasterPos2s               = 	dllRasterPos2s               ;
		qglRasterPos2sv              = 	dllRasterPos2sv              ;
		qglRasterPos3d               = 	dllRasterPos3d               ;
		qglRasterPos3dv              = 	dllRasterPos3dv              ;
		qglRasterPos3f               = 	dllRasterPos3f               ;
		qglRasterPos3fv              = 	dllRasterPos3fv              ;
		qglRasterPos3i               = 	dllRasterPos3i               ;
		qglRasterPos3iv              = 	dllRasterPos3iv              ;
		qglRasterPos3s               = 	dllRasterPos3s               ;
		qglRasterPos3sv              = 	dllRasterPos3sv              ;
		qglRasterPos4d               = 	dllRasterPos4d               ;
		qglRasterPos4dv              = 	dllRasterPos4dv              ;
		qglRasterPos4f               = 	dllRasterPos4f               ;
		qglRasterPos4fv              = 	dllRasterPos4fv              ;
		qglRasterPos4i               = 	dllRasterPos4i               ;
		qglRasterPos4iv              = 	dllRasterPos4iv              ;
		qglRasterPos4s               = 	dllRasterPos4s               ;
		qglRasterPos4sv              = 	dllRasterPos4sv              ;
		qglReadBuffer                = 	dllReadBuffer                ;
		qglReadPixels                = 	dllReadPixels                ;
		qglRectd                     = 	dllRectd                     ;
		qglRectdv                    = 	dllRectdv                    ;
		qglRectf                     = 	dllRectf                     ;
		qglRectfv                    = 	dllRectfv                    ;
		qglRecti                     = 	dllRecti                     ;
		qglRectiv                    = 	dllRectiv                    ;
		qglRects                     = 	dllRects                     ;
		qglRectsv                    = 	dllRectsv                    ;
		qglRenderMode                = 	dllRenderMode                ;
		qglScissor                   = 	dllScissor                   ;
		qglSelectBuffer              = 	dllSelectBuffer              ;
		qgxSetAlphaCompare           =  dllSetAlphaCompare           ;
		qgxSetBlendMode              = 	dllSetBlendMode              ;
		qgxSetCullMode               =  dllSetCullMode               ;
		qgxSetTevOp                  = 	dllSetTevOp                  ;
		qgxSetViewport               = 	dllSetViewport               ;
		qgxSetZMode                  = 	dllSetZMode                  ;
		qglShadeModel                = 	dllShadeModel                ;
		qglStencilFunc               = 	dllStencilFunc               ;
		qglStencilMask               = 	dllStencilMask               ;
		qglStencilOp                 = 	dllStencilOp                 ;
		qgxTexCoord2f32              = 	dllTexCoord2f32              ;
		qglTexCoordPointer           = 	dllTexCoordPointer           ;
		qglTexGend                   = 	dllTexGend                   ;
		qglTexGendv                  = 	dllTexGendv                  ;
		qglTexGenf                   = 	dllTexGenf                   ;
		qglTexGenfv                  = 	dllTexGenfv                  ;
		qglTexGeni                   = 	dllTexGeni                   ;
		qglTexGeniv                  = 	dllTexGeniv                  ;
		qglVertexPointer             = 	dllVertexPointer             ;
	}
}


void GXimp_LogNewFrame( void )
{
	fprintf( log_fp, "*** R_BeginFrame ***\n");
}

void *qwglGetProcAddress(char *symbol)
{
	return NULL;
}
// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// Include only for the GL builds (part 2):
#endif
// <<< FIX
