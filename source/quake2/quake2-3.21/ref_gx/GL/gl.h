#ifndef GL_H
#define GL_H

typedef unsigned int GLenum;

typedef float GLfloat;

typedef float GLclampf;

typedef unsigned int GLuint;

typedef unsigned char GLboolean;

typedef int GLsizei;

typedef int GLint;

typedef unsigned char GLubyte;

typedef void GLvoid;

typedef unsigned int GLbitfield;

typedef double GLclampd;

typedef double GLdouble;

typedef signed char GLbyte;

typedef short GLshort;

typedef unsigned short GLushort;

#define GL_LINEAR_MIPMAP_NEAREST 1

#define GL_LINEAR 2

#define GL_TEXTURE_2D 3

#define GL_NEAREST 4

#define GL_NEAREST_MIPMAP_NEAREST 5

#define GL_NEAREST_MIPMAP_LINEAR 6

#define GL_LINEAR_MIPMAP_LINEAR 7

#define GL_TEXTURE_MIN_FILTER 8

#define GL_TEXTURE_MAG_FILTER 9

#define GL_QUADS 10

#define GL_ALPHA_TEST 11

#define GL_BLEND 12

#define GL_RGBA 13

#define GL_UNSIGNED_BYTE 14

#define GL_FRONT 15

#define GL_BACK 16

#define GL_PROJECTION 17

#define GL_MODELVIEW 18

#define GL_DEPTH_TEST 19

#define GL_CULL_FACE 20

#define GL_COLOR_INDEX 21

#define GL_TRIANGLE_FAN 22

#define GL_SMOOTH 23

#define GL_ONE 24

#define GL_SRC_ALPHA 25

#define GL_ONE_MINUS_SRC_ALPHA 26

#define GL_TRIANGLE_STRIP 27

#define GL_TEXTURE_ENV 28

#define GL_TEXTURE_ENV_MODE 29

#define GL_MODULATE 30

#define GL_PERSPECTIVE_CORRECTION_HINT 31

#define GL_FASTEST 32

#define GL_REPLACE 33

#define GL_FLAT 34

#define GL_NICEST 35

#define GL_COLOR_BUFFER_BIT 1024

#define GL_DEPTH_BUFFER_BIT 2048

#define GL_LEQUAL 37

#define GL_GEQUAL 38

#define GL_ALPHA 39

#define GL_LUMINANCE 40

#define GL_INTENSITY 41

#define GL_POLYGON 42

#define GL_ZERO 43

#define GL_ONE_MINUS_SRC_COLOR 44

#define GL_RGB 45

#define GL_TRIANGLES 46

#define GL_VENDOR 47

#define GL_RENDERER 48

#define GL_VERSION 49

#define GL_EXTENSIONS 50

#define GL_GREATER 51

#define GL_FRONT_AND_BACK 52

#define GL_FILL 53

#define GL_TEXTURE_WRAP_S 54

#define GL_REPEAT 55

#define GL_TEXTURE_WRAP_T 56

#define GL_COLOR_INDEX8_EXT 57

#define GL_RGBA8 58

#define GL_RGB5_A1 59

#define GL_RGBA4 60

#define GL_RGBA2 61

#define GL_RGB8 62

#define GL_RGB5 63

#define GL_RGB4 64

#define GL_R3_G3_B2 65

#define GL_VERTEX_ARRAY 66

#define GL_FLOAT 67

#define GL_COLOR_ARRAY 68

#define GL_FALSE 0

#define GL_POINTS 69

#define GL_TRUE 1

#define GL_SCISSOR_TEST 70

#define GL_BACK_LEFT 71

#define GL_LINES 72

#define GL_NO_ERROR 0

#define GL_POINT_SMOOTH 73

#define GL_LINE_STRIP 74

#define GL_SRC_COLOR 75

#define GL_INTENSITY8 76

#define GL_LUMINANCE8 77

void glTexParameterf(GLenum target, GLenum pname, GLfloat param);

void glViewport(GLint x, GLint y, GLsizei width, GLsizei height);

void glEnable(GLenum cap);

void glDisable(GLenum cap);

void glTexCoord2f(GLfloat s, GLfloat t);

void glDrawBuffer(GLenum mode);

void glDrawBuffer(GLenum mode);

void glClear(GLbitfield mask);

void glDepthFunc(GLenum func);

void glDepthRange(GLclampd nearVal, GLclampd farVal);

void glCullFace(GLenum mode);

void glTexEnvf(GLenum target, GLenum pname, GLfloat param);

void glShadeModel(GLenum mode);

void glHint(GLenum target, GLenum mode);

void glFinish(void);

void glReadBuffer(GLenum mode);

void glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* data);

void glDepthMask(GLboolean flag);

void glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* data);

void glBlendFunc(GLenum sfactor, GLenum dfactor);

void glFlush(void);

const GLubyte* glGetString(GLenum name);

void glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);

void glAlphaFunc(GLenum func, GLclampf ref);

void glPolygonMode(GLenum face, GLenum mode);

GLboolean glIsEnabled(GLenum cap);

void glAccum(GLenum op, GLfloat value);

void glAlphaFunc(GLenum func, GLclampf ref);

GLboolean glAreTexturesResident(GLsizei n, const GLuint *textures, GLboolean *residences);

void glArrayElement(GLint i);

void glBitmap(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap);

void glBlendFunc(GLenum sfactor, GLenum dfactor);

void glCallList(GLuint list);

void glCallLists(GLsizei n, GLenum type, const GLvoid *lists);

void glClear(GLbitfield mask);

void glClearAccum(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);

void glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);

void glClearDepth(GLclampd depth);

void glClearIndex(GLfloat c);

void glClearStencil(GLint s);

void glClipPlane(GLenum plane, const GLdouble *equation);

void glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);

void glColorMaterial(GLenum face, GLenum mode);

void glColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);

void glCopyPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);

void glCopyTexImage1D(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border);

void glCopyTexImage2D(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);

void glCopyTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);

void glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);

void glCullFace(GLenum mode);

void glDeleteLists(GLuint list, GLsizei range);

void glDeleteTextures(GLsizei n, const GLuint *textures);

void glDepthFunc(GLenum func);

void glDepthMask(GLboolean flag);

void glDepthRange(GLclampd zNear, GLclampd zFar);

void glDisable(GLenum cap);

void glDisableClientState(GLenum array);

void glDrawArrays(GLenum mode, GLint first, GLsizei count);

void glDrawBuffer(GLenum mode);

void glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);

void glDrawPixels(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);

void glEdgeFlag(GLboolean flag);

void glEdgeFlagPointer(GLsizei stride, const GLvoid *pointer);

void glEdgeFlagv(const GLboolean *flag);

void glEnable(GLenum cap);

void glEnableClientState(GLenum array);

void glEndList(void);

void glEvalCoord1d(GLdouble u);

void glEvalCoord1dv(const GLdouble *u);

void glEvalCoord1f(GLfloat u);

void glEvalCoord1fv(const GLfloat *u);

void glEvalCoord2d(GLdouble u, GLdouble v);

void glEvalCoord2dv(const GLdouble *u);

void glEvalCoord2f(GLfloat u, GLfloat v);

void glEvalCoord2fv(const GLfloat *u);

void glEvalMesh1(GLenum mode, GLint i1, GLint i2);

void glEvalMesh2(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2);

void glEvalPoint1(GLint i);

void glEvalPoint2(GLint i, GLint j);

void glFeedbackBuffer(GLsizei size, GLenum type, GLfloat *buffer);

void glFinish(void);

void glFlush(void);

void glFogf(GLenum pname, GLfloat param);

void glFogfv(GLenum pname, const GLfloat *params);

void glFogi(GLenum pname, GLint param);

void glFogiv(GLenum pname, const GLint *params);

void glFrontFace(GLenum mode);

GLuint glGenLists(GLsizei range);

void glGenTextures(GLsizei n, GLuint *textures);

void glGetBooleanv(GLenum pname, GLboolean *params);

void glGetClipPlane(GLenum plane, GLdouble *equation);

void glGetDoublev(GLenum pname, GLdouble *params);

GLenum glGetError(void);

void glGetIntegerv(GLenum pname, GLint *params);

void glGetLightfv(GLenum light, GLenum pname, GLfloat *params);

void glGetLightiv(GLenum light, GLenum pname, GLint *params);

void glGetMapdv(GLenum target, GLenum query, GLdouble *v);

void glGetMapfv(GLenum target, GLenum query, GLfloat *v);

void glGetMapiv(GLenum target, GLenum query, GLint *v);

void glGetMaterialfv(GLenum face, GLenum pname, GLfloat *params);

void glGetMaterialiv(GLenum face, GLenum pname, GLint *params);

void glGetPixelMapfv(GLenum map, GLfloat *values);

void glGetPixelMapuiv(GLenum map, GLuint *values);

void glGetPixelMapusv(GLenum map, GLushort *values);

void glGetPointerv(GLenum pname, GLvoid* *params);

void glGetPolygonStipple(GLubyte *mask);

const GLubyte * glGetString(GLenum name);

void glGetTexEnvfv(GLenum target, GLenum pname, GLfloat *params);

void glGetTexEnviv(GLenum target, GLenum pname, GLint *params);

void glGetTexGendv(GLenum coord, GLenum pname, GLdouble *params);

void glGetTexGenfv(GLenum coord, GLenum pname, GLfloat *params);

void glGetTexGeniv(GLenum coord, GLenum pname, GLint *params);

void glGetTexImage(GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels);

void glGetTexLevelParameterfv(GLenum target, GLint level, GLenum pname, GLfloat *params);

void glGetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint *params);

void glGetTexParameterfv(GLenum target, GLenum pname, GLfloat *params);

void glGetTexParameteriv(GLenum target, GLenum pname, GLint *params);

void glHint(GLenum target, GLenum mode);

void glIndexMask(GLuint mask);

void glIndexPointer(GLenum type, GLsizei stride, const GLvoid *pointer);

void glIndexd(GLdouble c);

void glIndexdv(const GLdouble *c);

void glIndexf(GLfloat c);

void glIndexfv(const GLfloat *c);

void glIndexi(GLint c);

void glIndexiv(const GLint *c);

void glIndexs(GLshort c);

void glIndexsv(const GLshort *c);

void glIndexub(GLubyte c);

void glIndexubv(const GLubyte *c);

void glInitNames(void);

void glInterleavedArrays(GLenum format, GLsizei stride, const GLvoid *pointer);

GLboolean glIsEnabled(GLenum cap);

GLboolean glIsList(GLuint list);

GLboolean glIsTexture(GLuint texture);

void glLightModelf(GLenum pname, GLfloat param);

void glLightModelfv(GLenum pname, const GLfloat *params);

void glLightModeli(GLenum pname, GLint param);

void glLightModeliv(GLenum pname, const GLint *params);

void glLightf(GLenum light, GLenum pname, GLfloat param);

void glLightfv(GLenum light, GLenum pname, const GLfloat *params);

void glLighti(GLenum light, GLenum pname, GLint param);

void glLightiv(GLenum light, GLenum pname, const GLint *params);

void glLineStipple(GLint factor, GLushort pattern);

void glLineWidth(GLfloat width);

void glListBase(GLuint base);

void glLoadName(GLuint name);

void glLogicOp(GLenum opcode);

void glMap1d(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points);

void glMap1f(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points);

void glMap2d(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points);

void glMap2f(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points);

void glMapGrid1d(GLint un, GLdouble u1, GLdouble u2);

void glMapGrid1f(GLint un, GLfloat u1, GLfloat u2);

void glMapGrid2d(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2);

void glMapGrid2f(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2);

void glMaterialf(GLenum face, GLenum pname, GLfloat param);

void glMaterialfv(GLenum face, GLenum pname, const GLfloat *params);

void glMateriali(GLenum face, GLenum pname, GLint param);

void glMaterialiv(GLenum face, GLenum pname, const GLint *params);

void glNewList(GLuint list, GLenum mode);

void glNormal3b(GLbyte nx, GLbyte ny, GLbyte nz);

void glNormal3bv(const GLbyte *v);

void glNormal3d(GLdouble nx, GLdouble ny, GLdouble nz);

void glNormal3dv(const GLdouble *v);

void glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz);

void glNormal3fv(const GLfloat *v);

void glNormal3i(GLint nx, GLint ny, GLint nz);

void glNormal3iv(const GLint *v);

void glNormal3s(GLshort nx, GLshort ny, GLshort nz);

void glNormal3sv(const GLshort *v);

void glNormalPointer(GLenum type, GLsizei stride, const GLvoid *pointer);

void glPassThrough(GLfloat token);

void glPixelMapfv(GLenum map, GLsizei mapsize, const GLfloat *values);

void glPixelMapuiv(GLenum map, GLsizei mapsize, const GLuint *values);

void glPixelMapusv(GLenum map, GLsizei mapsize, const GLushort *values);

void glPixelStoref(GLenum pname, GLfloat param);

void glPixelStorei(GLenum pname, GLint param);

void glPixelTransferf(GLenum pname, GLfloat param);

void glPixelTransferi(GLenum pname, GLint param);

void glPixelZoom(GLfloat xfactor, GLfloat yfactor);

void glPointSize(GLfloat size);

void glPolygonMode(GLenum face, GLenum mode);

void glPolygonOffset(GLfloat factor, GLfloat units);

void glPolygonStipple(const GLubyte *mask);

void glPopAttrib(void);

void glPopClientAttrib(void);

void glPopName(void);

void glPrioritizeTextures(GLsizei n, const GLuint *textures, const GLclampf *priorities);

void glPushAttrib(GLbitfield mask);

void glPushClientAttrib(GLbitfield mask);

void glPushName(GLuint name);

void glRasterPos2d(GLdouble x, GLdouble y);

void glRasterPos2dv(const GLdouble *v);

void glRasterPos2f(GLfloat x, GLfloat y);

void glRasterPos2fv(const GLfloat *v);

void glRasterPos2i(GLint x, GLint y);

void glRasterPos2iv(const GLint *v);

void glRasterPos2s(GLshort x, GLshort y);

void glRasterPos2sv(const GLshort *v);

void glRasterPos3d(GLdouble x, GLdouble y, GLdouble z);

void glRasterPos3dv(const GLdouble *v);

void glRasterPos3f(GLfloat x, GLfloat y, GLfloat z);

void glRasterPos3fv(const GLfloat *v);

void glRasterPos3i(GLint x, GLint y, GLint z);

void glRasterPos3iv(const GLint *v);

void glRasterPos3s(GLshort x, GLshort y, GLshort z);

void glRasterPos3sv(const GLshort *v);

void glRasterPos4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w);

void glRasterPos4dv(const GLdouble *v);

void glRasterPos4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w);

void glRasterPos4fv(const GLfloat *v);

void glRasterPos4i(GLint x, GLint y, GLint z, GLint w);

void glRasterPos4iv(const GLint *v);

void glRasterPos4s(GLshort x, GLshort y, GLshort z, GLshort w);

void glRasterPos4sv(const GLshort *v);

void glReadBuffer(GLenum mode);

void glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);

void glRectd(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2);

void glRectdv(const GLdouble *v1, const GLdouble *v2);

void glRectf(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);

void glRectfv(const GLfloat *v1, const GLfloat *v2);

void glRecti(GLint x1, GLint y1, GLint x2, GLint y2);

void glRectiv(const GLint *v1, const GLint *v2);

void glRects(GLshort x1, GLshort y1, GLshort x2, GLshort y2);

void glRectsv(const GLshort *v1, const GLshort *v2);

GLint glRenderMode(GLenum mode);

void glScissor(GLint x, GLint y, GLsizei width, GLsizei height);

void glSelectBuffer(GLsizei size, GLuint *buffer);

void glShadeModel(GLenum mode);

void glStencilFunc(GLenum func, GLint ref, GLuint mask);

void glStencilMask(GLuint mask);

void glStencilOp(GLenum fail, GLenum zfail, GLenum zpass);

void glTexCoord1d(GLdouble s);

void glTexCoord1dv(const GLdouble *v);

void glTexCoord1f(GLfloat s);

void glTexCoord1fv(const GLfloat *v);

void glTexCoord1i(GLint s);

void glTexCoord1iv(const GLint *v);

void glTexCoord1s(GLshort s);

void glTexCoord1sv(const GLshort *v);

void glTexCoord2d(GLdouble s, GLdouble t);

void glTexCoord2dv(const GLdouble *v);

void glTexCoord2f(GLfloat s, GLfloat t);

void glTexCoord2fv(const GLfloat *v);

void glTexCoord2i(GLint s, GLint t);

void glTexCoord2iv(const GLint *v);

void glTexCoord2s(GLshort s, GLshort t);

void glTexCoord2sv(const GLshort *v);

void glTexCoord3d(GLdouble s, GLdouble t, GLdouble r);

void glTexCoord3dv(const GLdouble *v);

void glTexCoord3f(GLfloat s, GLfloat t, GLfloat r);

void glTexCoord3fv(const GLfloat *v);

void glTexCoord3i(GLint s, GLint t, GLint r);

void glTexCoord3iv(const GLint *v);

void glTexCoord3s(GLshort s, GLshort t, GLshort r);

void glTexCoord3sv(const GLshort *v);

void glTexCoord4d(GLdouble s, GLdouble t, GLdouble r, GLdouble q);

void glTexCoord4dv(const GLdouble *v);

void glTexCoord4f(GLfloat s, GLfloat t, GLfloat r, GLfloat q);

void glTexCoord4fv(const GLfloat *v);

void glTexCoord4i(GLint s, GLint t, GLint r, GLint q);

void glTexCoord4iv(const GLint *v);

void glTexCoord4s(GLshort s, GLshort t, GLshort r, GLshort q);

void glTexCoord4sv(const GLshort *v);

void glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);

void glTexEnvf(GLenum target, GLenum pname, GLfloat param);

void glTexEnvfv(GLenum target, GLenum pname, const GLfloat *params);

void glTexEnvi(GLenum target, GLenum pname, GLint param);

void glTexEnviv(GLenum target, GLenum pname, const GLint *params);

void glTexGend(GLenum coord, GLenum pname, GLdouble param);

void glTexGendv(GLenum coord, GLenum pname, const GLdouble *params);

void glTexGenf(GLenum coord, GLenum pname, GLfloat param);

void glTexGenfv(GLenum coord, GLenum pname, const GLfloat *params);

void glTexGeni(GLenum coord, GLenum pname, GLint param);

void glTexGeniv(GLenum coord, GLenum pname, const GLint *params);

void glTexParameterf(GLenum target, GLenum pname, GLfloat param);

void glTexParameterfv(GLenum target, GLenum pname, const GLfloat *params);

void glTexParameteri(GLenum target, GLenum pname, GLint param);

void glTexParameteriv(GLenum target, GLenum pname, const GLint *params);

void glTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);

void glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);

void glVertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);

void glViewport(GLint x, GLint y, GLsizei width, GLsizei height);

#endif
