#ifndef GXUTILS_H
#define GXUTILS_H

#include <gccore.h>

extern GXColor gxu_background_color;

extern u8 gxu_clear_buffers;

extern u8 gxu_clear_color_buffer;

extern Mtx44 gxu_projection_matrix;

extern Mtx gxu_modelview_matrices[];

extern int gxu_cur_modelview_matrix;

extern bool gxu_cull_enabled;

extern u8 gxu_cull_mode;

extern u8 gxu_z_test_enabled;

extern u8 gxu_z_write_enabled;

extern bool gxu_blend_enabled;

extern u8 gxu_blend_src_value;

extern u8 gxu_blend_dst_value;

extern bool gxu_alpha_test_enabled;

extern u8 gxu_alpha_test_lower;

extern u8 gxu_alpha_test_higher;

extern u8 gxu_cur_vertex_format;

extern u8 gxu_cur_r;

extern u8 gxu_cur_g;

extern u8 gxu_cur_b;

extern u8 gxu_cur_a;

extern f32 gxu_viewport_x;

extern f32 gxu_viewport_y;

extern f32 gxu_viewport_width;

extern f32 gxu_viewport_height;

void GXU_Init(GXRModeObj* rmode, void* framebuffer);

void GXU_EndFrame(void* framebuffer);

void GXU_CallGXEnd(void);

#endif