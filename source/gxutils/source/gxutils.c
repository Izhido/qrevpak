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