#include <GL/gl.h>
#include <gccore.h>
#include <malloc.h>
#include <string.h>

typedef struct
{
	f32 x;
	f32 y;
	f32 z;
	f32 r;
	f32 g;
	f32 b;
	f32 a;
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
					GX_Color3f32(gl_vertices[i].r, gl_vertices[i].g, gl_vertices[i].b);
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
					GX_Color3f32(gl_vertices[i].r, gl_vertices[i].g, gl_vertices[i].b);
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
					GX_Color3f32(gl_vertices[i].r, gl_vertices[i].g, gl_vertices[i].b);
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
					GX_Color3f32(gl_vertices[i].r, gl_vertices[i].g, gl_vertices[i].b);
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
	glColor4f(red, green, blue, 1);
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
	glColor4f(*(v), *(v + 1), *(v + 2), *(v + 3));
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
	glColor4f(*(v) / 255.0f, *(v + 1) / 255.0f, *(v + 2) / 255.0f, *(v + 3) / 255.0f);
}

