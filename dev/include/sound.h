#ifndef __SOUND_H__
#define __SOUND_H__

#include <types.h>

void sound_master_begin(void);
u32  sound_alloc(const i8 *name);
void sound_free(u32 sound);
u32  sound_source_alloc(u32 sound, b8 loop);
void sound_source_gain(u32 source, f32 gain);
void sound_source_play(u32 source);
void sound_source_stop(u32 source);
void sound_source_free(u32 source);
void sound_master_end(void);

#endif/*__SOUND_H__*/
