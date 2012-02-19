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

static float vid_gamma = 1.0;

qboolean is8bit = false;
qboolean isPermedia = false;
qboolean gx_mtexable = false;

extern GXRModeObj* sys_rmode;

Mtx44 gx_projection_matrix;

Mtx gx_modelview_matrices[32];

int gx_cur_modelview_matrix = 0;

qboolean gx_cull_enabled = false;

u8 gx_cull_mode;

u8 gx_z_test_enabled = GX_FALSE;

u8 gx_z_write_enabled = GX_TRUE;

qboolean gx_blend_enabled = false;

u8 gx_blend_src_value = GX_BL_ONE;

u8 gx_blend_dst_value = GX_BL_ZERO;

u8 gx_cur_vertex_format = GX_VTXFMT0;

u8 gx_cur_r;

u8 gx_cur_g;

u8 gx_cur_b;

u8 gx_cur_a;

/*
===============
QGX_Init
===============
*/
void QGX_Init (void)
{
	gx_cull_mode = GL_BACK;
	if(gx_cull_enabled)
	{
		GX_SetCullMode(gx_cull_mode);
	};
	gx_cur_vertex_format = GX_VTXFMT1;
 	GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);
 	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
	GX_SetTevOp(GX_TEVSTAGE0, GX_REPLACE);

	GX_SetAlphaCompare(GX_GREATER, 0.666, GX_AOP_AND, GX_ALWAYS, 1);

	glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
	glShadeModel (GL_FLAT);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	gx_blend_src_value = GX_BL_SRCALPHA;
	gx_blend_dst_value = GX_BL_INVSRCALPHA;
	GX_SetBlendMode(GX_BM_NONE, gx_blend_src_value, gx_blend_dst_value, GX_LO_NOOP);

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
	GX_DrawDone();
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

void glTexParameterf(GLenum target, GLenum pname, GLfloat param)
{
}

void glEnable(GLenum cap)
{
}

void glDisable(GLenum cap)
{
}

void glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
	gx_cur_r = red * 255.0;
	gx_cur_g = green * 255.0;
	gx_cur_b = blue * 255.0;
	gx_cur_a = alpha * 255.0;
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

	if(gx_cur_vertex_format == GX_VTXFMT1)
	{
 		GX_SetVtxDesc(GX_VA_TEX0, GX_NONE);
		GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLOR0A0);
		GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
	};
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
	if(gx_cur_vertex_format == GX_VTXFMT1)
	{
 		GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);
 		GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
		GX_SetTevOp(GX_TEVSTAGE0, GX_REPLACE);
	};
}

void glColor3f(GLfloat red, GLfloat green, GLfloat blue)
{
	gx_cur_r = red * 255.0;
	gx_cur_g = green * 255.0;
	gx_cur_b = blue * 255.0;
	gx_cur_a = 255;
}

void glTexCoord2f(GLfloat s, GLfloat t)
{
}

void glColor4fv(const GLfloat* v)
{
	gx_cur_r = (*(v)) * 255.0;
	gx_cur_g = (*(v + 1)) * 255.0;
	gx_cur_b = (*(v + 2)) * 255.0;
	gx_cur_a = (*(v + 3)) * 255.0;
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
	gl_vertices[gl_vertex_count].r = gx_cur_r;
	gl_vertices[gl_vertex_count].g = gx_cur_g;
	gl_vertices[gl_vertex_count].b = gx_cur_b;
	gl_vertices[gl_vertex_count].a = gx_cur_a;
	gl_vertex_count++;
}

void glVertex3fv(const GLfloat* v)
{
	glVertex3f(*(v), *(v + 1), *(v + 2));
}

void glTexEnvf(GLenum target, GLenum pname, GLfloat param)
{
}

void glShadeModel(GLenum mode)
{
}

void glHint(GLenum target, GLenum mode)
{
}

void glFlush(void)
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
	gx_cur_r = (*(v)) * 255.0;
	gx_cur_g = (*(v + 1)) * 255.0;
	gx_cur_b = (*(v + 2)) * 255.0;
	gx_cur_a = 255;
}

/************************************************************************************************************************/
#endif

