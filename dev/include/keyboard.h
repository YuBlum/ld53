#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#define ATTACK 0x10

#include <types.h>

void keyboard_update(void *window, i32 (*glfw_get_key)(void *, i32), f64 delta_time);
void keyboard_delay_set(f32 new_delay);
b8   keyboard_click(u8 key);
b8   keyboard_down(u8 key);

#endif/*__KEYBOARD_H__*/
