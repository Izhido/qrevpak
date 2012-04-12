#include "GL/gl.h"
#include <gccore.h>
#include <malloc.h>
#include <string.h>

typedef struct
{
	f32 x;
	f32 y;
	f32 z;
	u8 r;
	u8 g;
	u8 b;
	u8 a;
} gl_vertex_t;

GLenum gl_primitive_mode = -1;
gl_vertex_t* gl_vertices = 0;
int gl_vertices_size = 0;
int gl_vertex_count = 0;
GLenum gl_matrix_mode = GL_MODELVIEW;
Mtx gl_modelview_matrices[32];
int gl_modelview_matrix = 0;
Mtx44 gl_projection_matrices[2];
int gl_projection_matrix = 0;

void glTexImage2D(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* data)
{
} 

void glTexParameterf(GLenum target, GLenum pname, GLfloat param)
{
}

void glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
	GX_SetViewport(x, y, width, height, 0, 1);
}

void glMatrixMode(GLenum mode)
{
	gl_matrix_mode = mode;
}

void glLoadIdentity(void)
{
	switch(gl_matrix_mode)
	{
		case GL_MODELVIEW:
		{
			guMtxIdentity(gl_modelview_matrices[gl_modelview_matrix]);
			GX_LoadPosMtxImm(gl_modelview_matrices[gl_modelview_matrix], GX_PNMTX0);
			break;
		};
		case GL_PROJECTION:
		{
			gl_projection_matrices[gl_projection_matrix][0][0] = 1;
			gl_projection_matrices[gl_projection_matrix][0][1] = 0;
			gl_projection_matrices[gl_projection_matrix][0][2] = 0;
			gl_projection_matrices[gl_projection_matrix][0][3] = 0;
			gl_projection_matrices[gl_projection_matrix][1][0] = 0;
			gl_projection_matrices[gl_projection_matrix][1][1] = 1;
			gl_projection_matrices[gl_projection_matrix][1][2] = 0;
			gl_projection_matrices[gl_projection_matrix][1][3] = 0;
			gl_projection_matrices[gl_projection_matrix][2][0] = 0;
			gl_projection_matrices[gl_projection_matrix][2][1] = 0;
			gl_projection_matrices[gl_projection_matrix][2][2] = 1;
			gl_projection_matrices[gl_projection_matrix][2][3] = 0;
			gl_projection_matrices[gl_projection_matrix][3][0] = 0;
			gl_projection_matrices[gl_projection_matrix][3][1] = 0;
			gl_projection_matrices[gl_projection_matrix][3][2] = 0;
			gl_projection_matrices[gl_projection_matrix][3][3] = 1;
			GX_LoadProjectionMtx(gl_projection_matrices[gl_projection_matrix], GX_PERSPECTIVE);
			break;
		};
	};
}

void glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble nearVal, GLdouble farVal)
{
	switch(gl_matrix_mode)
	{
		case GL_PROJECTION:
		{
			guOrtho(gl_projection_matrices[gl_projection_matrix], top, bottom, left, right, nearVal, farVal);
			GX_LoadProjectionMtx(gl_projection_matrices[gl_projection_matrix], GX_PERSPECTIVE);
			break;
		};
	};
}

void glEnable(GLenum cap)
{
}

void glDisable(GLenum cap)
{
}

void glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
	glColor4ub(red * 255.0, green * 255.0, blue * 255.0, alpha * 255.0);
}

void glBegin(GLenum mode)
{
	gl_primitive_mode = mode;
	gl_vertex_count = 0;
}

void glVertex2f(GLfloat x, GLfloat y)
{
	glVertex3f(x, y, 0);
}

void glEnd(void)
{
	int i;
	int m;

	switch(gl_primitive_mode)
	{
		case GL_QUADS:
		{
			m = gl_vertex_count / 4;
			if(m > 0)
			{
				m = m * 4;
				GX_Begin(GX_QUADS, GX_VTXFMT0, m);
				for(i = 0; i < m; i++)
				{
					GX_Position3f32(gl_vertices[i].x, gl_vertices[i].y, gl_vertices[i].z);
					GX_Color4u8(gl_vertices[i].r, gl_vertices[i].g, gl_vertices[i].b, gl_vertices[i].a);
				};
				GX_End();
			};
			break;
		};
		case GL_POLYGON:
		case GL_TRIANGLE_FAN:
		{
			m = gl_vertex_count;
			if(m >= 3)
			{
				GX_Begin(GX_TRIANGLEFAN, GX_VTXFMT0, m);
				for(i = 0; i < m; i++)
				{
					GX_Position3f32(gl_vertices[i].x, gl_vertices[i].y, gl_vertices[i].z);
					GX_Color4u8(gl_vertices[i].r, gl_vertices[i].g, gl_vertices[i].b, gl_vertices[i].a);
				};
				GX_End();
			};
			break;
		};
		case GL_TRIANGLE_STRIP:
		{
			m = gl_vertex_count;
			if(m >= 3)
			{
				GX_Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, m);
				for(i = 0; i < m; i++)
				{
					GX_Position3f32(gl_vertices[i].x, gl_vertices[i].y, gl_vertices[i].z);
					GX_Color4u8(gl_vertices[i].r, gl_vertices[i].g, gl_vertices[i].b, gl_vertices[i].a);
				};
				GX_End();
			};
			break;
		};
		case GL_TRIANGLES:
		{
			m = gl_vertex_count / 3;
			if(m > 0)
			{
				m = m * 3;
				GX_Begin(GX_TRIANGLES, GX_VTXFMT0, m);
				for(i = 0; i < m; i++)
				{
					GX_Position3f32(gl_vertices[i].x, gl_vertices[i].y, gl_vertices[i].z);
					GX_Color4u8(gl_vertices[i].r, gl_vertices[i].g, gl_vertices[i].b, gl_vertices[i].a);
				};
				GX_End();
			};
			break;
		};
	};
	gl_primitive_mode = -1;
}

void glColor3f(GLfloat red, GLfloat green, GLfloat blue)
{
	glColor4ub(red * 255.0, green * 255.0, blue * 255.0, 255);
}

void glBindTexture(GLenum target, GLuint texture)
{
}

void glTexCoord2f(GLfloat s, GLfloat t)
{
}

void glDrawBuffer(GLenum mode)
{
}

void glClear(GLbitfield mask)
{
}

void glDepthFunc(GLenum func)
{
}

void glDepthRange(GLclampd nearVal, GLclampd farVal)
{
}

void glTranslatef(GLfloat x, GLfloat y, GLfloat z)
{
	Mtx m;

	switch(gl_matrix_mode)
	{
		case GL_MODELVIEW:
		{
			guMtxIdentity(m);
			guMtxTrans(m, x, y, z);
			guMtxConcat(gl_modelview_matrices[gl_modelview_matrix], m, gl_modelview_matrices[gl_modelview_matrix]);
			GX_LoadPosMtxImm(gl_modelview_matrices[gl_modelview_matrix], GX_PNMTX0);
			break;
		};
	};
}

void glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
	guVector a;
	Mtx m;

	switch(gl_matrix_mode)
	{
		case GL_MODELVIEW:
		{
			a.x = x;
			a.y = y;
			a.z = z;
			guMtxRotAxisDeg(m, &a, angle);
			guMtxConcat(gl_modelview_matrices[gl_modelview_matrix], m, gl_modelview_matrices[gl_modelview_matrix]);
			GX_LoadPosMtxImm(gl_modelview_matrices[gl_modelview_matrix], GX_PNMTX0);
			break;
		};
	};
}

void glFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble nearVal, GLdouble farVal)
{
	switch(gl_matrix_mode)
	{
		case GL_PROJECTION:
		{
			guFrustum(gl_projection_matrices[gl_projection_matrix], top, bottom, left, right, nearVal, farVal);
			GX_LoadProjectionMtx(gl_projection_matrices[gl_projection_matrix], GX_PERSPECTIVE);
			break;
		};
	};
}

void glScalef(GLfloat x, GLfloat y, GLfloat z)
{
	Mtx m;

	switch(gl_matrix_mode)
	{
		case GL_MODELVIEW:
		{
			guMtxScale(m, x, y, z);
			guMtxConcat(gl_modelview_matrices[gl_modelview_matrix], m, gl_modelview_matrices[gl_modelview_matrix]);
			GX_LoadPosMtxImm(gl_modelview_matrices[gl_modelview_matrix], GX_PNMTX0);
			break;
		};
	};
}

void glCullFace(GLenum mode)
{
}

void glGetFloatv(GLenum pname, GLfloat* params)
{
	switch(pname)
	{
		case GL_MODELVIEW_MATRIX:
		{
			params[0] = gl_modelview_matrices[gl_modelview_matrix][0][0];
			params[1] = gl_modelview_matrices[gl_modelview_matrix][0][1];
			params[2] = gl_modelview_matrices[gl_modelview_matrix][0][2];
			params[3] = gl_modelview_matrices[gl_modelview_matrix][0][3];
			params[4] = gl_modelview_matrices[gl_modelview_matrix][1][0];
			params[5] = gl_modelview_matrices[gl_modelview_matrix][1][1];
			params[6] = gl_modelview_matrices[gl_modelview_matrix][1][2];
			params[7] = gl_modelview_matrices[gl_modelview_matrix][1][3];
			params[8] = gl_modelview_matrices[gl_modelview_matrix][2][0];
			params[9] = gl_modelview_matrices[gl_modelview_matrix][2][1];
			params[10] = gl_modelview_matrices[gl_modelview_matrix][2][2];
			params[11] = gl_modelview_matrices[gl_modelview_matrix][2][3];
			params[12] = 0;
			params[13] = 0;
			params[14] = 0;
			params[15] = 1;
			break;
		};
	};
}

void glColor4fv(const GLfloat* v)
{
	glColor4ub((*(v)) * 255.0, (*(v + 1)) * 255.0, (*(v + 2)) * 255.0, (*(v + 3)) * 255.0);
}

void glVertex3f(GLfloat x, GLfloat y, GLfloat z)
{
	int newsize;
	gl_vertex_t* newlist;

	if(gl_vertex_count >= gl_vertices_size)
	{
		newsize = gl_vertices_size + 1024;
		newlist = malloc(newsize * sizeof(gl_vertex_t));
		memset(newlist, 0, newsize * sizeof(gl_vertex_t));
		if(gl_vertices != 0)
		{
			memcpy(newlist, gl_vertices, gl_vertices_size);
			free(gl_vertices);
		};
		gl_vertices = newlist;
		gl_vertices_size = newsize;
	};
	gl_vertices[gl_vertex_count].x = x;
	gl_vertices[gl_vertex_count].y = y;
	gl_vertices[gl_vertex_count].z = z;
	gl_vertex_count++;
}

void glVertex3fv(const GLfloat* v)
{
	glVertex3f(*(v), *(v + 1), *(v + 2));
}

void glPushMatrix(void)
{
	switch(gl_matrix_mode)
	{
		case GL_MODELVIEW:
		{
			gl_modelview_matrices[gl_modelview_matrix + 1][0][0] = gl_modelview_matrices[gl_modelview_matrix][0][0];
			gl_modelview_matrices[gl_modelview_matrix + 1][0][1] = gl_modelview_matrices[gl_modelview_matrix][0][1];
			gl_modelview_matrices[gl_modelview_matrix + 1][0][2] = gl_modelview_matrices[gl_modelview_matrix][0][2];
			gl_modelview_matrices[gl_modelview_matrix + 1][0][3] = gl_modelview_matrices[gl_modelview_matrix][0][3];
			gl_modelview_matrices[gl_modelview_matrix + 1][1][0] = gl_modelview_matrices[gl_modelview_matrix][1][0];
			gl_modelview_matrices[gl_modelview_matrix + 1][1][1] = gl_modelview_matrices[gl_modelview_matrix][1][1];
			gl_modelview_matrices[gl_modelview_matrix + 1][1][2] = gl_modelview_matrices[gl_modelview_matrix][1][2];
			gl_modelview_matrices[gl_modelview_matrix + 1][1][3] = gl_modelview_matrices[gl_modelview_matrix][1][3];
			gl_modelview_matrices[gl_modelview_matrix + 1][2][0] = gl_modelview_matrices[gl_modelview_matrix][2][0];
			gl_modelview_matrices[gl_modelview_matrix + 1][2][1] = gl_modelview_matrices[gl_modelview_matrix][2][1];
			gl_modelview_matrices[gl_modelview_matrix + 1][2][2] = gl_modelview_matrices[gl_modelview_matrix][2][2];
			gl_modelview_matrices[gl_modelview_matrix + 1][2][3] = gl_modelview_matrices[gl_modelview_matrix][2][3];
			gl_modelview_matrix++;
			break;
		};
		case GL_PROJECTION:
		{
			gl_projection_matrices[gl_projection_matrix + 1][0][0] = gl_projection_matrices[gl_projection_matrix][0][0];
			gl_projection_matrices[gl_projection_matrix + 1][0][1] = gl_projection_matrices[gl_projection_matrix][0][1];
			gl_projection_matrices[gl_projection_matrix + 1][0][2] = gl_projection_matrices[gl_projection_matrix][0][2];
			gl_projection_matrices[gl_projection_matrix + 1][0][3] = gl_projection_matrices[gl_projection_matrix][0][3];
			gl_projection_matrices[gl_projection_matrix + 1][1][0] = gl_projection_matrices[gl_projection_matrix][1][0];
			gl_projection_matrices[gl_projection_matrix + 1][1][1] = gl_projection_matrices[gl_projection_matrix][1][1];
			gl_projection_matrices[gl_projection_matrix + 1][1][2] = gl_projection_matrices[gl_projection_matrix][1][2];
			gl_projection_matrices[gl_projection_matrix + 1][1][3] = gl_projection_matrices[gl_projection_matrix][1][3];
			gl_projection_matrices[gl_projection_matrix + 1][2][0] = gl_projection_matrices[gl_projection_matrix][2][0];
			gl_projection_matrices[gl_projection_matrix + 1][2][1] = gl_projection_matrices[gl_projection_matrix][2][1];
			gl_projection_matrices[gl_projection_matrix + 1][2][2] = gl_projection_matrices[gl_projection_matrix][2][2];
			gl_projection_matrices[gl_projection_matrix + 1][2][3] = gl_projection_matrices[gl_projection_matrix][2][3];
			gl_projection_matrices[gl_projection_matrix + 1][3][0] = gl_projection_matrices[gl_projection_matrix][3][0];
			gl_projection_matrices[gl_projection_matrix + 1][3][1] = gl_projection_matrices[gl_projection_matrix][3][1];
			gl_projection_matrices[gl_projection_matrix + 1][3][2] = gl_projection_matrices[gl_projection_matrix][3][2];
			gl_projection_matrices[gl_projection_matrix + 1][3][3] = gl_projection_matrices[gl_projection_matrix][3][3];
			gl_projection_matrix++;
			break;
		};
	};
}

void glTexEnvf(GLenum target, GLenum pname, GLfloat param)
{
}

void glShadeModel(GLenum mode)
{
}

void glPopMatrix(void)
{
	switch(gl_matrix_mode)
	{
		case GL_MODELVIEW:
		{
			gl_modelview_matrix--;
			GX_LoadPosMtxImm(gl_modelview_matrices[gl_modelview_matrix], GX_PNMTX0);
			break;
		};
		case GL_PROJECTION:
		{
			gl_projection_matrix--;
			GX_LoadProjectionMtx(gl_projection_matrices[gl_projection_matrix], GX_PERSPECTIVE);
			break;
		};
	};
}

void glHint(GLenum target, GLenum mode)
{
}

void glLoadMatrixf(const GLfloat* m)
{
	switch(gl_matrix_mode)
	{
		case GL_MODELVIEW:
		{
			gl_modelview_matrices[gl_modelview_matrix][0][0] = m[0];
			gl_modelview_matrices[gl_modelview_matrix][0][1] = m[1];
			gl_modelview_matrices[gl_modelview_matrix][0][2] = m[2];
			gl_modelview_matrices[gl_modelview_matrix][0][3] = m[3];
			gl_modelview_matrices[gl_modelview_matrix][1][0] = m[4];
			gl_modelview_matrices[gl_modelview_matrix][1][1] = m[5];
			gl_modelview_matrices[gl_modelview_matrix][1][2] = m[6];
			gl_modelview_matrices[gl_modelview_matrix][1][3] = m[7];
			gl_modelview_matrices[gl_modelview_matrix][2][0] = m[8];
			gl_modelview_matrices[gl_modelview_matrix][2][1] = m[9];
			gl_modelview_matrices[gl_modelview_matrix][2][2] = m[10];
			gl_modelview_matrices[gl_modelview_matrix][2][3] = m[11];
			GX_LoadPosMtxImm(gl_modelview_matrices[gl_modelview_matrix], GX_PNMTX0);
			break;
		};
		case GL_PROJECTION:
		{
			gl_projection_matrices[gl_projection_matrix][0][0] = m[0];
			gl_projection_matrices[gl_projection_matrix][0][1] = m[1];
			gl_projection_matrices[gl_projection_matrix][0][2] = m[2];
			gl_projection_matrices[gl_projection_matrix][0][3] = m[3];
			gl_projection_matrices[gl_projection_matrix][1][0] = m[4];
			gl_projection_matrices[gl_projection_matrix][1][1] = m[5];
			gl_projection_matrices[gl_projection_matrix][1][2] = m[6];
			gl_projection_matrices[gl_projection_matrix][1][3] = m[7];
			gl_projection_matrices[gl_projection_matrix][2][0] = m[8];
			gl_projection_matrices[gl_projection_matrix][2][1] = m[9];
			gl_projection_matrices[gl_projection_matrix][2][2] = m[10];
			gl_projection_matrices[gl_projection_matrix][2][3] = m[11];
			gl_projection_matrices[gl_projection_matrix][3][0] = m[12];
			gl_projection_matrices[gl_projection_matrix][3][1] = m[13];
			gl_projection_matrices[gl_projection_matrix][3][2] = m[14];
			gl_projection_matrices[gl_projection_matrix][3][3] = m[15];
			GX_LoadProjectionMtx(gl_projection_matrices[gl_projection_matrix], GX_PERSPECTIVE);
			break;
		};
	};
}

void glFinish(void)
{
}

void glReadBuffer(GLenum mode)
{
}

void glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* data)
{
}

void glDepthMask(GLboolean flag)
{
}

void glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* data)
{
}

void glBlendFunc(GLenum sfactor, GLenum dfactor)
{
}

void glFlush(void)
{
}

const GLubyte* glGetString(GLenum name)
{
	switch(name)
	{
		case GL_VENDOR:
		{
			return "";
		};
		case GL_RENDERER:
		{
			return "glRevol - OpenGL to GX translator";
		};
		case GL_VERSION:
		{
			return "R1";
		};
		case GL_EXTENSIONS:
		{
			return "";
		};
		default:
		{
			return "";
		};
	};
}

void glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
}

void glAlphaFunc(GLenum func, GLclampf ref)
{
}

void glPolygonMode(GLenum face, GLenum mode)
{
}

void glColor3ubv(const GLubyte* v)
{
	glColor4ub(*(v), *(v + 1), *(v + 2), 255);
}

void glColor4ubv(const GLubyte* v)
{
	glColor4f(*(v), *(v + 1), *(v + 2), *(v + 3));
}

GLboolean glIsEnabled(GLenum cap)
{
	return false;
} 

void glColor4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha)
{
	int newsize;
	gl_vertex_t* newlist;

	if(gl_vertex_count >= gl_vertices_size)
	{
		newsize = gl_vertices_size + 1024;
		newlist = malloc(newsize * sizeof(gl_vertex_t));
		memset(newlist, 0, newsize * sizeof(gl_vertex_t));
		if(gl_vertices != 0)
		{
			memcpy(newlist, gl_vertices, gl_vertices_size);
			free(gl_vertices);
		};
		gl_vertices = newlist;
		gl_vertices_size = newsize;
	};
	gl_vertices[gl_vertex_count].r = red;
	gl_vertices[gl_vertex_count].g = green;
	gl_vertices[gl_vertex_count].b = blue;
	gl_vertices[gl_vertex_count].a = alpha;
}

void glAccum(GLenum op, GLfloat value)
{
}

GLboolean glAreTexturesResident(GLsizei n, const GLuint *textures, GLboolean *residences)
{
}

void glArrayElement(GLint i)
{
}

void glBitmap(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap)
{
}

void glCallList(GLuint list)
{
}

void glCallLists(GLsizei n, GLenum type, const GLvoid *lists)
{
}

void glClearAccum(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
}

void glClearDepth(GLclampd depth)
{
}

void glClearIndex(GLfloat c)
{
}

void glClearStencil(GLint s)
{
}

void glClipPlane(GLenum plane, const GLdouble *equation)
{
}

void glColor3b(GLbyte red, GLbyte green, GLbyte blue)
{
}

void glColor3bv(const GLbyte *v)
{
}

void glColor3d(GLdouble red, GLdouble green, GLdouble blue)
{
}

void glColor3dv(const GLdouble *v)
{
}

void glColor3fv(const GLfloat *v)
{
}

void glColor3i(GLint red, GLint green, GLint blue)
{
}

void glColor3iv(const GLint *v)
{
}

void glColor3s(GLshort red, GLshort green, GLshort blue)
{
}

void glColor3sv(const GLshort *v)
{
}

void glColor3ub(GLubyte red, GLubyte green, GLubyte blue)
{
}

void glColor3ui(GLuint red, GLuint green, GLuint blue)
{
}

void glColor3uiv(const GLuint *v)
{
}

void glColor3us(GLushort red, GLushort green, GLushort blue)
{
}

void glColor3usv(const GLushort *v)
{
}

void glColor4b(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha)
{
}

void glColor4bv(const GLbyte *v)
{
}

void glColor4d(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha)
{
}

void glColor4dv(const GLdouble *v)
{
}

void glColor4i(GLint red, GLint green, GLint blue, GLint alpha)
{
}

void glColor4iv(const GLint *v)
{
}

void glColor4s(GLshort red, GLshort green, GLshort blue, GLshort alpha)
{
}

void glColor4sv(const GLshort *v)
{
}

void glColor4ui(GLuint red, GLuint green, GLuint blue, GLuint alpha)
{
}

void glColor4uiv(const GLuint *v)
{
}

void glColor4us(GLushort red, GLushort green, GLushort blue, GLushort alpha)
{
}

void glColor4usv(const GLushort *v)
{
}

void glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
}

void glColorMaterial(GLenum face, GLenum mode)
{
}

void glColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
}

void glCopyPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type)
{
}

void glCopyTexImage1D(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border)
{
}

void glCopyTexImage2D(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
}

void glCopyTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)
{
}

void glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
}

void glDeleteLists(GLuint list, GLsizei range)
{
}

void glDeleteTextures(GLsizei n, const GLuint *textures)
{
}

void glDisableClientState(GLenum array)
{
}

void glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
}

void glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
{
}

void glDrawPixels(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels)
{
}

void glEdgeFlag(GLboolean flag)
{
}

void glEdgeFlagPointer(GLsizei stride, const GLvoid *pointer)
{
}

void glEdgeFlagv(const GLboolean *flag)
{
}

void glEnableClientState(GLenum array)
{
}

void glEndList(void)
{
}

void glEvalCoord1d(GLdouble u)
{
}

void glEvalCoord1dv(const GLdouble *u)
{
}

void glEvalCoord1f(GLfloat u)
{
}

void glEvalCoord1fv(const GLfloat *u)
{
}

void glEvalCoord2d(GLdouble u, GLdouble v)
{
}

void glEvalCoord2dv(const GLdouble *u)
{
}

void glEvalCoord2f(GLfloat u, GLfloat v)
{
}

void glEvalCoord2fv(const GLfloat *u)
{
}

void glEvalMesh1(GLenum mode, GLint i1, GLint i2)
{
}

void glEvalMesh2(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2)
{
}

void glEvalPoint1(GLint i)
{
}

void glEvalPoint2(GLint i, GLint j)
{
}

void glFeedbackBuffer(GLsizei size, GLenum type, GLfloat *buffer)
{
}

void glFogf(GLenum pname, GLfloat param)
{
}

void glFogfv(GLenum pname, const GLfloat *params)
{
}

void glFogi(GLenum pname, GLint param)
{
}

void glFogiv(GLenum pname, const GLint *params)
{
}

void glFrontFace(GLenum mode)
{
}

GLuint glGenLists(GLsizei range)
{
}

void glGenTextures(GLsizei n, GLuint *textures)
{
}

void glGetBooleanv(GLenum pname, GLboolean *params)
{
}

void glGetClipPlane(GLenum plane, GLdouble *equation)
{
}

void glGetDoublev(GLenum pname, GLdouble *params)
{
}

GLenum glGetError(void)
{
	return GL_NO_ERROR;
}

void glGetIntegerv(GLenum pname, GLint *params)
{
}

void glGetLightfv(GLenum light, GLenum pname, GLfloat *params)
{
}

void glGetLightiv(GLenum light, GLenum pname, GLint *params)
{
}

void glGetMapdv(GLenum target, GLenum query, GLdouble *v)
{
}

void glGetMapfv(GLenum target, GLenum query, GLfloat *v)
{
}

void glGetMapiv(GLenum target, GLenum query, GLint *v)
{
}

void glGetMaterialfv(GLenum face, GLenum pname, GLfloat *params)
{
}

void glGetMaterialiv(GLenum face, GLenum pname, GLint *params)
{
}

void glGetPixelMapfv(GLenum map, GLfloat *values)
{
}

void glGetPixelMapuiv(GLenum map, GLuint *values)
{
}

void glGetPixelMapusv(GLenum map, GLushort *values)
{
}

void glGetPointerv(GLenum pname, GLvoid* *params)
{
}

void glGetPolygonStipple(GLubyte *mask)
{
}

void glGetTexEnvfv(GLenum target, GLenum pname, GLfloat *params)
{
}

void glGetTexEnviv(GLenum target, GLenum pname, GLint *params)
{
}

void glGetTexGendv(GLenum coord, GLenum pname, GLdouble *params)
{
}

void glGetTexGenfv(GLenum coord, GLenum pname, GLfloat *params)
{
}

void glGetTexGeniv(GLenum coord, GLenum pname, GLint *params)
{
}

void glGetTexImage(GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels)
{
}

void glGetTexLevelParameterfv(GLenum target, GLint level, GLenum pname, GLfloat *params)
{
}

void glGetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint *params)
{
}

void glGetTexParameterfv(GLenum target, GLenum pname, GLfloat *params)
{
}

void glGetTexParameteriv(GLenum target, GLenum pname, GLint *params)
{
}

void glIndexMask(GLuint mask)
{
}

void glIndexPointer(GLenum type, GLsizei stride, const GLvoid *pointer)
{
}

void glIndexd(GLdouble c)
{
}

void glIndexdv(const GLdouble *c)
{
}

void glIndexf(GLfloat c)
{
}

void glIndexfv(const GLfloat *c)
{
}

void glIndexi(GLint c)
{
}

void glIndexiv(const GLint *c)
{
}

void glIndexs(GLshort c)
{
}

void glIndexsv(const GLshort *c)
{
}

void glIndexub(GLubyte c)
{
}

void glIndexubv(const GLubyte *c)
{
}

void glInitNames(void)
{
}

void glInterleavedArrays(GLenum format, GLsizei stride, const GLvoid *pointer)
{
}

GLboolean glIsList(GLuint list)
{
}

GLboolean glIsTexture(GLuint texture)
{
}

void glLightModelf(GLenum pname, GLfloat param)
{
}

void glLightModelfv(GLenum pname, const GLfloat *params)
{
}

void glLightModeli(GLenum pname, GLint param)
{
}

void glLightModeliv(GLenum pname, const GLint *params)
{
}

void glLightf(GLenum light, GLenum pname, GLfloat param)
{
}

void glLightfv(GLenum light, GLenum pname, const GLfloat *params)
{
}

void glLighti(GLenum light, GLenum pname, GLint param)
{
}

void glLightiv(GLenum light, GLenum pname, const GLint *params)
{
}

void glLineStipple(GLint factor, GLushort pattern)
{
}

void glLineWidth(GLfloat width)
{
}

void glListBase(GLuint base)
{
}

void glLoadMatrixd(const GLdouble *m)
{
}

void glLoadName(GLuint name)
{
}

void glLogicOp(GLenum opcode)
{
}

void glMap1d(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points)
{
}

void glMap1f(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points)
{
}

void glMap2d(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points)
{
}

void glMap2f(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points)
{
}

void glMapGrid1d(GLint un, GLdouble u1, GLdouble u2)
{
}

void glMapGrid1f(GLint un, GLfloat u1, GLfloat u2)
{
}

void glMapGrid2d(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2)
{
}

void glMapGrid2f(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2)
{
}

void glMaterialf(GLenum face, GLenum pname, GLfloat param)
{
}

void glMaterialfv(GLenum face, GLenum pname, const GLfloat *params)
{
}

void glMateriali(GLenum face, GLenum pname, GLint param)
{
}

void glMaterialiv(GLenum face, GLenum pname, const GLint *params)
{
}

void glMultMatrixd(const GLdouble *m)
{
}

void glMultMatrixf(const GLfloat *m)
{
}

void glNewList(GLuint list, GLenum mode)
{
}

void glNormal3b(GLbyte nx, GLbyte ny, GLbyte nz)
{
}

void glNormal3bv(const GLbyte *v)
{
}

void glNormal3d(GLdouble nx, GLdouble ny, GLdouble nz)
{
}

void glNormal3dv(const GLdouble *v)
{
}

void glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz)
{
}

void glNormal3fv(const GLfloat *v)
{
}

void glNormal3i(GLint nx, GLint ny, GLint nz)
{
}

void glNormal3iv(const GLint *v)
{
}

void glNormal3s(GLshort nx, GLshort ny, GLshort nz)
{
}

void glNormal3sv(const GLshort *v)
{
}

void glNormalPointer(GLenum type, GLsizei stride, const GLvoid *pointer)
{
}

void glPassThrough(GLfloat token)
{
}

void glPixelMapfv(GLenum map, GLsizei mapsize, const GLfloat *values)
{
}

void glPixelMapuiv(GLenum map, GLsizei mapsize, const GLuint *values)
{
}

void glPixelMapusv(GLenum map, GLsizei mapsize, const GLushort *values)
{
}

void glPixelStoref(GLenum pname, GLfloat param)
{
}

void glPixelStorei(GLenum pname, GLint param)
{
}

void glPixelTransferf(GLenum pname, GLfloat param)
{
}

void glPixelTransferi(GLenum pname, GLint param)
{
}

void glPixelZoom(GLfloat xfactor, GLfloat yfactor)
{
}

void glPointSize(GLfloat size)
{
}

void glPolygonOffset(GLfloat factor, GLfloat units)
{
}

void glPolygonStipple(const GLubyte *mask)
{
}

void glPopAttrib(void)
{
}

void glPopClientAttrib(void)
{
}

void glPopName(void)
{
}

void glPrioritizeTextures(GLsizei n, const GLuint *textures, const GLclampf *priorities)
{
}

void glPushAttrib(GLbitfield mask)
{
}

void glPushClientAttrib(GLbitfield mask)
{
}

void glPushName(GLuint name)
{
}

void glRasterPos2d(GLdouble x, GLdouble y)
{
}

void glRasterPos2dv(const GLdouble *v)
{
}

void glRasterPos2f(GLfloat x, GLfloat y)
{
}

void glRasterPos2fv(const GLfloat *v)
{
}

void glRasterPos2i(GLint x, GLint y)
{
}

void glRasterPos2iv(const GLint *v)
{
}

void glRasterPos2s(GLshort x, GLshort y)
{
}

void glRasterPos2sv(const GLshort *v)
{
}

void glRasterPos3d(GLdouble x, GLdouble y, GLdouble z)
{
}

void glRasterPos3dv(const GLdouble *v)
{
}

void glRasterPos3f(GLfloat x, GLfloat y, GLfloat z)
{
}

void glRasterPos3fv(const GLfloat *v)
{
}

void glRasterPos3i(GLint x, GLint y, GLint z)
{
}

void glRasterPos3iv(const GLint *v)
{
}

void glRasterPos3s(GLshort x, GLshort y, GLshort z)
{
}

void glRasterPos3sv(const GLshort *v)
{
}

void glRasterPos4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
}

void glRasterPos4dv(const GLdouble *v)
{
}

void glRasterPos4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
}

void glRasterPos4fv(const GLfloat *v)
{
}

void glRasterPos4i(GLint x, GLint y, GLint z, GLint w)
{
}

void glRasterPos4iv(const GLint *v)
{
}

void glRasterPos4s(GLshort x, GLshort y, GLshort z, GLshort w)
{
}

void glRasterPos4sv(const GLshort *v)
{
}

void glRectd(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2)
{
}

void glRectdv(const GLdouble *v1, const GLdouble *v2)
{
}

void glRectf(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)
{
}

void glRectfv(const GLfloat *v1, const GLfloat *v2)
{
}

void glRecti(GLint x1, GLint y1, GLint x2, GLint y2)
{
}

void glRectiv(const GLint *v1, const GLint *v2)
{
}

void glRects(GLshort x1, GLshort y1, GLshort x2, GLshort y2)
{
}

void glRectsv(const GLshort *v1, const GLshort *v2)
{
}

GLint glRenderMode(GLenum mode)
{
}

void glRotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z)
{
}

void glScaled(GLdouble x, GLdouble y, GLdouble z)
{
}

void glScissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
}

void glSelectBuffer(GLsizei size, GLuint *buffer)
{
}

void glStencilFunc(GLenum func, GLint ref, GLuint mask)
{
}

void glStencilMask(GLuint mask)
{
}

void glStencilOp(GLenum fail, GLenum zfail, GLenum zpass)
{
}

void glTexCoord1d(GLdouble s)
{
}

void glTexCoord1dv(const GLdouble *v)
{
}

void glTexCoord1f(GLfloat s)
{
}

void glTexCoord1fv(const GLfloat *v)
{
}

void glTexCoord1i(GLint s)
{
}

void glTexCoord1iv(const GLint *v)
{
}

void glTexCoord1s(GLshort s)
{
}

void glTexCoord1sv(const GLshort *v)
{
}

void glTexCoord2d(GLdouble s, GLdouble t)
{
}

void glTexCoord2dv(const GLdouble *v)
{
}

void glTexCoord2fv(const GLfloat *v)
{
}

void glTexCoord2i(GLint s, GLint t)
{
}

void glTexCoord2iv(const GLint *v)
{
}

void glTexCoord2s(GLshort s, GLshort t)
{
}

void glTexCoord2sv(const GLshort *v)
{
}

void glTexCoord3d(GLdouble s, GLdouble t, GLdouble r)
{
}

void glTexCoord3dv(const GLdouble *v)
{
}

void glTexCoord3f(GLfloat s, GLfloat t, GLfloat r)
{
}

void glTexCoord3fv(const GLfloat *v)
{
}

void glTexCoord3i(GLint s, GLint t, GLint r)
{
}

void glTexCoord3iv(const GLint *v)
{
}

void glTexCoord3s(GLshort s, GLshort t, GLshort r)
{
}

void glTexCoord3sv(const GLshort *v)
{
}

void glTexCoord4d(GLdouble s, GLdouble t, GLdouble r, GLdouble q)
{
}

void glTexCoord4dv(const GLdouble *v)
{
}

void glTexCoord4f(GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
}

void glTexCoord4fv(const GLfloat *v)
{
}

void glTexCoord4i(GLint s, GLint t, GLint r, GLint q)
{
}

void glTexCoord4iv(const GLint *v)
{
}

void glTexCoord4s(GLshort s, GLshort t, GLshort r, GLshort q)
{
}

void glTexCoord4sv(const GLshort *v)
{
}

void glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
}

void glTexEnvfv(GLenum target, GLenum pname, const GLfloat *params)
{
}

void glTexEnvi(GLenum target, GLenum pname, GLint param)
{
}

void glTexEnviv(GLenum target, GLenum pname, const GLint *params)
{
}

void glTexGend(GLenum coord, GLenum pname, GLdouble param)
{
}

void glTexGendv(GLenum coord, GLenum pname, const GLdouble *params)
{
}

void glTexGenf(GLenum coord, GLenum pname, GLfloat param)
{
}

void glTexGenfv(GLenum coord, GLenum pname, const GLfloat *params)
{
}

void glTexGeni(GLenum coord, GLenum pname, GLint param)
{
}

void glTexGeniv(GLenum coord, GLenum pname, const GLint *params)
{
}

void glTexImage1D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
}

void glTexParameterfv(GLenum target, GLenum pname, const GLfloat *params)
{
}

void glTexParameteri(GLenum target, GLenum pname, GLint param)
{
}

void glTexParameteriv(GLenum target, GLenum pname, const GLint *params)
{
}

void glTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels)
{
}

void glTranslated(GLdouble x, GLdouble y, GLdouble z)
{
}

void glVertex2d(GLdouble x, GLdouble y)
{
}

void glVertex2dv(const GLdouble *v)
{
}

void glVertex2fv(const GLfloat *v)
{
}

void glVertex2i(GLint x, GLint y)
{
}

void glVertex2iv(const GLint *v)
{
}

void glVertex2s(GLshort x, GLshort y)
{
}

void glVertex2sv(const GLshort *v)
{
}

void glVertex3d(GLdouble x, GLdouble y, GLdouble z)
{
}

void glVertex3dv(const GLdouble *v)
{
}

void glVertex3i(GLint x, GLint y, GLint z)
{
}

void glVertex3iv(const GLint *v)
{
}

void glVertex3s(GLshort x, GLshort y, GLshort z)
{
}

void glVertex3sv(const GLshort *v)
{
}

void glVertex4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
}

void glVertex4dv(const GLdouble *v)
{
}

void glVertex4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
}

void glVertex4fv(const GLfloat *v)
{
}

void glVertex4i(GLint x, GLint y, GLint z, GLint w)
{
}

void glVertex4iv(const GLint *v)
{
}

void glVertex4s(GLshort x, GLshort y, GLshort z, GLshort w)
{
}

void glVertex4sv(const GLshort *v)
{
}

void glVertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
}
