#ifndef __RENDERER_H__
#define __RENDERER_H__

#include <types.h>
#include <linear_algebra.h>

void renderer_begin(void *(*load_func)(const i8 *name));
u32  renderer_sprite_alloc(struct v2f frame_index, struct v2f frame_size, u32 frame_amount, f32 fps);
void renderer_sprite_frame_set(u32 sprite_index, u32 frame);
void renderer_sprite_timer_set(u32 sprite_index, f32 value);
void renderer_sprite_update(u32 sprite_index, f64 delta_time);
void renderer_sprite_free(u32 sprite_index);
void renderer_sprite(u32 sprite_index, struct v2f position, struct v2f size, struct v2b flip);
void renderer_text(struct v2f position, const i8 *format, ...);
void renderer_update(void);
void renderer_end(void);

#endif/*__RENDERER_H__*/
