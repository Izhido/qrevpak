#include "gxutils.h"
#include <malloc.h>
#include <string.h>

#define GXU_FIFO_SIZE (256 * 1024)

GXColor gxu_background_color = {0, 0, 0, 0};

void* gxu_gpfifo = NULL;

f32 gxu_yscale;

u32 gxu_xfbHeight;

u8 gxu_clear_buffers = GX_TRUE;

u8 gxu_clear_color_buffer = GX_TRUE;

Mtx44 gxu_projection_matrices[2];

int gxu_cur_projection_matrix = 0;

Mtx gxu_modelview_matrices[32];

int gxu_cur_modelview_matrix = 0;

bool gxu_cull_enabled = false;

u8 gxu_cull_mode = GX_CULL_FRONT;

u8 gxu_z_test_enabled = GX_FALSE;

u8 gxu_cur_z_func = GX_LEQUAL;

u8 gxu_z_write_enabled = GX_TRUE;

u8 gxu_blend_src_value = GX_BL_ONE;

u8 gxu_blend_dst_value = GX_BL_ZERO;

bool gxu_alpha_test_enabled = false;

u8 gxu_alpha_test_lower = 0;

u8 gxu_alpha_test_higher = 255;

u8 gxu_cur_vertex_format = GX_VTXFMT0;

u8 gxu_cur_r;

u8 gxu_cur_g;

u8 gxu_cur_b;

u8 gxu_cur_a;

f32 gxu_viewport_x;

f32 gxu_viewport_y;

f32 gxu_viewport_width;

f32 gxu_viewport_height;

void GXU_Init(GXRModeObj* rmode, void* framebuffer)
{
	gxu_gpfifo = memalign(32, GXU_FIFO_SIZE);
	memset(gxu_gpfifo, 0, GXU_FIFO_SIZE);
 
	GX_Init(gxu_gpfifo, GXU_FIFO_SIZE);
 
	GX_SetCopyClear(gxu_background_color, GX_MAX_Z24);
 
	gxu_yscale = GX_GetYScaleFactor(rmode->efbHeight, rmode->xfbHeight);
	gxu_xfbHeight = GX_SetDispCopyYScale(gxu_yscale);
	GX_SetScissor(0, 0, rmode->fbWidth, rmode->efbHeight);
	GX_SetDispCopySrc(0, 0, rmode->fbWidth, rmode->efbHeight);
	GX_SetDispCopyDst(rmode->fbWidth, gxu_xfbHeight);
	GX_SetCopyFilter(rmode->aa, rmode->sample_pattern, GX_TRUE, rmode->vfilter);
	GX_SetFieldMode(rmode->field_rendering, ((rmode->viHeight == 2 * rmode->xfbHeight) ? GX_ENABLE : GX_DISABLE));
 
	if (rmode->aa)
        GX_SetPixelFmt(GX_PF_RGB565_Z16, GX_ZC_LINEAR);
    else
        GX_SetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);

	GX_CopyDisp(framebuffer, GX_TRUE);
	GX_SetDispCopyGamma(GX_GM_1_0);

	GX_ClearVtxDesc();
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
 	GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
 
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
 
	GX_SetVtxAttrFmt(GX_VTXFMT1, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT1, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT1, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT1, GX_VA_TEX1, GX_TEX_ST, GX_F32, 0);
 
	GX_SetNumChans(1);
	GX_SetNumTexGens(1);
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLOR0A0);
	GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);

	GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
	GX_SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX1, GX_IDENTITY);

	GX_InvVtxCache();
	GX_InvalidateTexAll();
}

void GXU_EndFrame(void* framebuffer)
{
	GX_SetColorUpdate(gxu_clear_color_buffer);
	GX_SetAlphaUpdate(gxu_clear_color_buffer);
	GX_CopyDisp(framebuffer, gxu_clear_buffers);
	GX_SetColorUpdate(GX_TRUE);
	GX_SetAlphaUpdate(GX_TRUE);
	VIDEO_SetNextFramebuffer(framebuffer);
}

void GXU_EnableTexture(void)
{
 	GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);
 	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
	GX_SetTevOp(GX_TEVSTAGE0, GX_REPLACE);
}

void GXU_DisableTexture(void)
{
 	GX_SetVtxDesc(GX_VA_TEX0, GX_NONE);
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLOR0A0);
	GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
}

void GXU_CallguMtxRotAxisDeg(Mtx mt, guVector* axis, f32 deg)
{
	guMtxRotAxisDeg(mt, axis, deg);
}

void GXU_CallGXPosition3f32(f32 x, f32 y, f32 z)
{
	GX_Position3f32(x, y, z);
}

void GXU_CallGXColor4u8(u8 r, u8 g, u8 b, u8 a)
{
	GX_Color4u8(r, g, b, a);
}

void GXU_CallGXTexCoord2f32(f32 s, f32 t)
{
	GX_TexCoord2f32(s, t);
}

void GXU_CallGXEnd(void)
{
	GX_End();
}

unsigned char* GXU_CopyTexRGBA8(unsigned char* src, int width, int height, unsigned char* dst)
{
	int x;
	int y;
	int xi;
	int yi;
	int i;
	int j;
	int k;
	int l;

	i = 0;
	j = 4 * width - 16;
	l = 12 * width;
	for(y = 0; y < height; y += 4)
	{
		for(x = 0; x < width; x += 4)
		{
			k = i;
			for(yi = 0; yi < 4; yi++)
			{
				for(xi = 0; xi < 4; xi++)
				{
					*(dst++) = src[k + 3];
					*(dst++) = src[k];
					k += 4;
				};
				k += j;
			};
			k = i;
			for(yi = 0; yi < 4; yi++)
			{
				for(xi = 0; xi < 4; xi++)
				{
					*(dst++) = src[k + 1];
					*(dst++) = src[k + 2];
					k += 4;
				};
				k += j;
			};
			i += 16;
		};
		i += l;
	};
	return dst;
}

unsigned char* GXU_CopyTexRGB5A3(unsigned char* src, int width, int height, unsigned char* dst)
{
	int x;
	int y;
	int xi;
	int yi;
	int i;
	int j;
	int k;
	int l;
	unsigned char s1;
	unsigned char s2;

	i = 0;
	j = 2 * width - 8;
	l = 6 * width;
	for(y = 0; y < height; y += 4)
	{
		for(x = 0; x < width; x += 4)
		{
			k = i;
			for(yi = 0; yi < 4; yi++)
			{
				for(xi = 0; xi < 4; xi++)
				{
					s1 = src[k];
					s2 = src[k + 1];
					*(dst++) = (((s2 & 15) >> 1) << 4) | (s1 >> 4);
					*(dst++) = ((s1 & 15) << 4) | (s2 >> 4);
					k += 2;
				};
				k += j;
			};
			i += 8;
		};
		i += l;
	};
	return dst;
}

unsigned char* GXU_CopyTexV8(unsigned char* src, int width, int height, unsigned char* dst)
{
	int x;
	int y;
	int xi;
	int yi;
	int i;
	int j;
	int k;
	int l;

	i = 0;
	j = width - 8;
	l = 3 * width;
	for(y = 0; y < height; y += 4)
	{
		for(x = 0; x < width; x += 8)
		{
			k = i;
			for(yi = 0; yi < 4; yi++)
			{
				for(xi = 0; xi < 8; xi++)
				{
					*(dst++) = src[k];
					k++;
				};
				k += j;
			};
			i += 8;
		};
		i += l;
	};
	return dst;
}

unsigned char* GXU_CopyTexIA4(unsigned char* src, int width, int height, unsigned char* dst)
{
	int x;
	int y;
	int xi;
	int yi;
	int i;
	int j;
	int k;
	int l;

	i = 0;
	j = width - 8;
	l = 3 * width;
	for(y = 0; y < height; y += 4)
	{
		for(x = 0; x < width; x += 8)
		{
			k = i;
			for(yi = 0; yi < 4; yi++)
			{
				for(xi = 0; xi < 8; xi++)
				{
					*(dst++) = ((src[k] & 15) << 4) | (src[k] >> 4);
					k++;
				};
				k += j;
			};
			i += 8;
		};
		i += l;
	};
	return dst;
}

void GXU_CopyTexSubRGBA8(unsigned char* src, int width, int height, unsigned char* dst, int xoffset, int yoffset, int tex_width, int tex_height)
{
	int ybegin;
	int yend;
	int x;
	int y;
	int xi;
	int yi;
	int xs;
	int ys;
	int k;
	bool in;

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
							*(dst++) = src[k + 3];
							*(dst++) = src[k];
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
							*(dst++) = src[k + 1];
							*(dst++) = src[k + 2];
						};
					};
					if(!in)
						dst += 2;
				};
			};
		};
	};
}

void GXU_CopyTexSubRGB5A3(unsigned char* src, int width, int height, unsigned char* dst, int xoffset, int yoffset, int tex_width, int tex_height)
{
	int ybegin;
	int yend;
	int x;
	int y;
	int xi;
	int yi;
	int xs;
	int ys;
	int k;
	bool in;
	unsigned char s1;
	unsigned char s2;

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
							s1 = src[k];
							s2 = src[k + 1];
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
}

void GXU_CopyTexSubV8(unsigned char* src, int width, int height, unsigned char* dst, int xoffset, int yoffset, int tex_width, int tex_height)
{
	int ybegin;
	int yend;
	int x;
	int y;
	int xi;
	int yi;
	int xs;
	int ys;
	int k;
	bool in;

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
							*(dst++) = src[k];
						};
					};
					if(!in)
						dst++;
				};
			};
		};
	};
}

void GXU_CopyTexSubIA4(unsigned char* src, int width, int height, unsigned char* dst, int xoffset, int yoffset, int tex_width, int tex_height)
{
	int ybegin;
	int yend;
	int x;
	int y;
	int xi;
	int yi;
	int xs;
	int ys;
	int k;
	bool in;
	unsigned char s1;

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
							s1 = src[k];
							*(dst++) = ((s1 & 15) << 4) | (s1 >> 4);
						};
					};
					if(!in)
						dst++;
				};
			};
		};
	};
}

void GXU_Ortho(Mtx44 mtx, f32 top, f32 bottom, f32 left, f32 right, f32 nearP, f32 farP)
{
	f32 t;

	memset(mtx, 0, 4 * 4 * sizeof(f32));
	t = 1.0f / (right - left);
	mtx[0][0] = 2 * t;
	mtx[0][3] = (-right - left) * t;
	t = 1.0f / (top - bottom);
	mtx[1][1] = 2 * t;
	mtx[1][3] = (-top - bottom)*t;
	t = 1.0f / (farP - nearP);
	mtx[2][2] = -2 * t;
	mtx[2][3] = (-farP - nearP) * t;
	mtx[3][3] = 1;
}

