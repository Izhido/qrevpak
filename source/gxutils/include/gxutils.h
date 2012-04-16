#ifndef GXUTILS_H
#define GXUTILS_H

#include <gccore.h>

extern GXColor gxu_background_color;

extern u8 gxu_clear_buffers;

extern u8 gxu_clear_color_buffer;

void GXU_Init(GXRModeObj* rmode, void* framebuffer);

void GXU_EndFrame(void* framebuffer);

#endif