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

#define GL_MODELVIEW_MATRIX 36

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

void glTexImage2D(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* data);

void glTexParameterf(GLenum target, GLenum pname, GLfloat param);

void glViewport(GLint x, GLint y, GLsizei width, GLsizei height);

void glMatrixMode(GLenum mode);

void glLoadIdentity(void);

void glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble nearVal, GLdouble farVal);

void glEnable(GLenum cap);

void glDisable(GLenum cap);

void glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);

void glBegin(GLenum mode);

void glVertex2f(GLfloat x, GLfloat y);

void glEnd(void);

void glColor3f(GLfloat red, GLfloat green, GLfloat blue);

void glBindTexture(GLenum target, GLuint texture);

void glTexCoord2f(GLfloat s, GLfloat t);

void glDrawBuffer(GLenum mode);

void glDrawBuffer(GLenum mode);

void glClear(GLbitfield mask);

void glDepthFunc(GLenum func);

void glDepthRange(GLclampd nearVal, GLclampd farVal);

void glTranslatef(GLfloat x, GLfloat y, GLfloat z);

void glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);

void glFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble nearVal, GLdouble farVal);

void glScalef(GLfloat x, GLfloat y, GLfloat z);

void glCullFace(GLenum mode);

void glGetFloatv(GLenum pname, GLfloat* params);

void glColor4fv(const GLfloat* v);

void glVertex3f(GLfloat x, GLfloat y, GLfloat z);

void glVertex3fv(const GLfloat* v);

void glPushMatrix(void);

void glTexEnvf(GLenum target, GLenum pname, GLfloat param);

void glShadeModel(GLenum mode);

void glPopMatrix(void);

void glHint(GLenum target, GLenum mode);

void glLoadMatrixf(const GLfloat* m);

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

void glColor3ubv(const GLubyte* v);

#endif
