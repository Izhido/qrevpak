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
// gx_vidrevol.c -- video driver for the GX hardware of the Nintendo Wii 
// (based on vid_null.c)

#ifdef GXQUAKE

#include <gccore.h>

#include "quakedef.h"

unsigned	d_8to24table[256];
unsigned char d_15to8table[65536];

int		texture_mode = GL_LINEAR;

int		texture_extension_number = 1;

float		gxdepthmin, gxdepthmax;

cvar_t	gx_ztrick = {"gx_ztrick","1"};

const char *gx_vendor;
const char *gx_renderer;
const char *gx_version;
const char *gx_extensions;

static float vid_gamma = 1.0;

qboolean is8bit = false;
qboolean isPermedia = false;
qboolean gx_mtexable = false;

extern GXRModeObj* sys_rmode;

/*
===============
QGX_Init
===============
*/
void QGX_Init (void)
{
	gx_vendor = glGetString (GL_VENDOR);
	Con_Printf ("GL_VENDOR: %s\n", gx_vendor);
	gx_renderer = glGetString (GL_RENDERER);
	Con_Printf ("GL_RENDERER: %s\n", gx_renderer);

	gx_version = glGetString (GL_VERSION);
	Con_Printf ("GL_VERSION: %s\n", gx_version);
	gx_extensions = glGetString (GL_EXTENSIONS);
	Con_Printf ("GL_EXTENSIONS: %s\n", gx_extensions);

//	Con_Printf ("%s %s\n", gx_renderer, gx_version);

	glClearColor (1,0,0,0);
	glCullFace(GL_FRONT);
	glEnable(GL_TEXTURE_2D);

	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.666);

	glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
	glShadeModel (GL_FLAT);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

//	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}

void GX_BeginRendering (int *x, int *y, int *width, int *height)
{
	*x = 0;
	*y = sys_rmode->viHeight / 20;
	*width = sys_rmode->viWidth;
	*height = 9 * sys_rmode->viHeight / 10;
}

void GX_EndRendering (void)
{
	glFlush();
}

void	VID_SetPalette (unsigned char *palette)
{
	byte	*pal;
	unsigned r,g,b;
	unsigned v;
	int     r1,g1,b1;
	int		j,k,l,m;
	unsigned short i;
	unsigned	*table;
	FILE *f;
	char s[255];
	int dist, bestdist;
	static qboolean palflag = false;

//
// 8 8 8 encoding
//
	pal = palette;
	table = d_8to24table;
	for (i=0 ; i<256 ; i++)
	{
		r = pal[0];
		g = pal[1];
		b = pal[2];
		pal += 3;
		
		v = (255<<24) + (r<<0) + (g<<8) + (b<<16);
		*table++ = v;
	}
	d_8to24table[255] &= 0xffffff;	// 255 is transparent

	// JACK: 3D distance calcs - k is last closest, l is the distance.
	for (i=0; i < (1<<15); i++) {
		/* Maps
		000000000000000
		000000000011111 = Red  = 0x1F
		000001111100000 = Blue = 0x03E0
		111110000000000 = Grn  = 0x7C00
		*/
		r = ((i & 0x1F) << 3)+4;
		g = ((i & 0x03E0) >> 2)+4;
		b = ((i & 0x7C00) >> 7)+4;
		pal = (unsigned char *)d_8to24table;
		for (v=0,k=0,bestdist=10000*10000; v<256; v++,pal+=4) {
			r1 = (int)r - (int)pal[0];
			g1 = (int)g - (int)pal[1];
			b1 = (int)b - (int)pal[2];
			dist = (r1*r1)+(g1*g1)+(b1*b1);
			if (dist < bestdist) {
				k=v;
				bestdist = dist;
			}
		}
		d_15to8table[i]=k;
	}
}

void	VID_ShiftPalette (unsigned char *palette)
{
	VID_SetPalette(palette);
}

qboolean VID_Is8bit(void)
{
	return is8bit;
}

static void Check_Gamma (unsigned char *pal)
{
	float	f, inf;
	unsigned char	palette[768];
	int		i;

	if ((i = COM_CheckParm("-gamma")) == 0) {
		vid_gamma = 0.7;
	} else
		vid_gamma = Q_atof(com_argv[i+1]);

	for (i=0 ; i<768 ; i++)
	{
		f = pow ( (pal[i]+1)/256.0 , vid_gamma );
		inf = f*255 + 0.5;
		if (inf < 0)
			inf = 0;
		if (inf > 255)
			inf = 255;
		palette[i] = inf;
	}

	memcpy (pal, palette, sizeof(palette));
}

void	VID_Init (unsigned char *palette)
{
	int i;
	char	gxdir[MAX_OSPATH];
	int width = sys_rmode->viWidth;
	int height = 9 * sys_rmode->viHeight / 10;

	Cvar_RegisterVariable (&gx_ztrick);
	
	vid.maxwarpwidth = width;
	vid.maxwarpheight = height;
	vid.colormap = host_colormap;
	vid.fullbright = 256 - LittleLong (*((int *)vid.colormap + 2048));

// set vid parameters
	if ((i = COM_CheckParm("-width")) != 0)
		width = atoi(com_argv[i+1]);
	if ((i = COM_CheckParm("-height")) != 0)
		height = atoi(com_argv[i+1]);

	if ((i = COM_CheckParm("-conwidth")) != 0)
		vid.conwidth = Q_atoi(com_argv[i+1]);
	else
		vid.conwidth = 640;

	vid.conwidth &= 0xfff8; // make it a multiple of eight

	if (vid.conwidth < 320)
		vid.conwidth = 320;

	// pick a conheight that matches with correct aspect
	vid.conheight = vid.conwidth*3 / 4;

	if ((i = COM_CheckParm("-conheight")) != 0)
		vid.conheight = Q_atoi(com_argv[i+1]);
	if (vid.conheight < 200)
		vid.conheight = 200;

	if (vid.conheight > height)
		vid.conheight = height;
	if (vid.conwidth > width)
		vid.conwidth = width;
	vid.width = vid.conwidth;
	vid.height = vid.conheight;

	vid.aspect = ((float)vid.height / (float)vid.width) *
				(320.0 / 240.0);
	vid.numpages = 2;

	QGX_Init();

	sprintf (gxdir, "%s/gxquake", com_gamedir);
	Sys_mkdir (gxdir);

	Check_Gamma(palette);
	VID_SetPalette(palette);

	Con_SafePrintf ("Video mode %dx%d initialized.\n", width, height);

	vid.recalc_refdef = 1;				// force a surface cache flush
}

void	VID_Shutdown (void)
{
}

void	VID_Update (vrect_t *rects)
{
}

/******************* These are part of the GL wrapper and MUST BE DELETED ASAP: ***************************************/
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
	//GX_SetViewport(x, y, width, height, 0, 1);
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
			//guMtxIdentity(gl_modelview_matrices[gl_modelview_matrix]);
			//GX_LoadPosMtxImm(gl_modelview_matrices[gl_modelview_matrix], GX_PNMTX0);
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
			//GX_LoadProjectionMtx(gl_projection_matrices[gl_projection_matrix], GX_PERSPECTIVE);
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
			//guOrtho(gl_projection_matrices[gl_projection_matrix], top, bottom, left, right, nearVal, farVal);
			//GX_LoadProjectionMtx(gl_projection_matrices[gl_projection_matrix], GX_PERSPECTIVE);
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
	//glColor4ub(red * 255.0, green * 255.0, blue * 255.0, alpha * 255.0);
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
				//GX_Begin(GX_QUADS, GX_VTXFMT0, m);
				//for(i = 0; i < m; i++)
				//{
				//	GX_Position3f32(gl_vertices[i].x, gl_vertices[i].y, gl_vertices[i].z);
				//	GX_Color4u8(gl_vertices[i].r, gl_vertices[i].g, gl_vertices[i].b, gl_vertices[i].a);
				//};
				//GX_End();
			};
			break;
		};
		case GL_POLYGON:
		case GL_TRIANGLE_FAN:
		{
			m = gl_vertex_count;
			if(m >= 3)
			{
				//GX_Begin(GX_TRIANGLEFAN, GX_VTXFMT0, m);
				//for(i = 0; i < m; i++)
				//{
				//	GX_Position3f32(gl_vertices[i].x, gl_vertices[i].y, gl_vertices[i].z);
				//	GX_Color4u8(gl_vertices[i].r, gl_vertices[i].g, gl_vertices[i].b, gl_vertices[i].a);
				//};
				//GX_End();
			};
			break;
		};
		case GL_TRIANGLE_STRIP:
		{
			m = gl_vertex_count;
			if(m >= 3)
			{
				//GX_Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, m);
				//for(i = 0; i < m; i++)
				//{
				//	GX_Position3f32(gl_vertices[i].x, gl_vertices[i].y, gl_vertices[i].z);
				//	GX_Color4u8(gl_vertices[i].r, gl_vertices[i].g, gl_vertices[i].b, gl_vertices[i].a);
				//};
				//GX_End();
			};
			break;
		};
		case GL_TRIANGLES:
		{
			m = gl_vertex_count / 3;
			if(m > 0)
			{
				m = m * 3;
				//GX_Begin(GX_TRIANGLES, GX_VTXFMT0, m);
				//for(i = 0; i < m; i++)
				//{
				//	GX_Position3f32(gl_vertices[i].x, gl_vertices[i].y, gl_vertices[i].z);
				//	GX_Color4u8(gl_vertices[i].r, gl_vertices[i].g, gl_vertices[i].b, gl_vertices[i].a);
				//};
				//GX_End();
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
			//guMtxIdentity(m);
			//guMtxTrans(m, x, y, z);
			//guMtxConcat(gl_modelview_matrices[gl_modelview_matrix], m, gl_modelview_matrices[gl_modelview_matrix]);
			//GX_LoadPosMtxImm(gl_modelview_matrices[gl_modelview_matrix], GX_PNMTX0);
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
			//guMtxRotAxisDeg(m, &a, angle);
			//guMtxConcat(gl_modelview_matrices[gl_modelview_matrix], m, gl_modelview_matrices[gl_modelview_matrix]);
			//GX_LoadPosMtxImm(gl_modelview_matrices[gl_modelview_matrix], GX_PNMTX0);
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
			//guFrustum(gl_projection_matrices[gl_projection_matrix], top, bottom, left, right, nearVal, farVal);
			//GX_LoadProjectionMtx(gl_projection_matrices[gl_projection_matrix], GX_PERSPECTIVE);
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
			//guMtxScale(m, x, y, z);
			//guMtxConcat(gl_modelview_matrices[gl_modelview_matrix], m, gl_modelview_matrices[gl_modelview_matrix]);
			//GX_LoadPosMtxImm(gl_modelview_matrices[gl_modelview_matrix], GX_PNMTX0);
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





	GLfloat t = abs(x) + abs(y) + abs(z); if(t == 0) t = 1;

	gl_vertices[gl_vertex_count].r = abs(x) * 255.0 / t;
	gl_vertices[gl_vertex_count].g = abs(y) * 255.0 / t;
	gl_vertices[gl_vertex_count].b = abs(z) * 255.0 / t;
	gl_vertices[gl_vertex_count].a = 127;



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
			//GX_LoadPosMtxImm(gl_modelview_matrices[gl_modelview_matrix], GX_PNMTX0);
			break;
		};
		case GL_PROJECTION:
		{
			gl_projection_matrix--;
			//GX_LoadProjectionMtx(gl_projection_matrices[gl_projection_matrix], GX_PERSPECTIVE);
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
			//GX_LoadPosMtxImm(gl_modelview_matrices[gl_modelview_matrix], GX_PNMTX0);
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
			//GX_LoadProjectionMtx(gl_projection_matrices[gl_projection_matrix], GX_PERSPECTIVE);
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

/************************************************************************************************************************/
#endif

