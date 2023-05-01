#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#define ATTACK 0x10

#include <types.h>

void keyboard_update(void *window, i32 (*glfw_get_key)(void *, i32), f64 delta_time);
b8   keyboard_click(u8 key);
b8   keyboard_down(u8 key);

#endif/*__KEYBOARD_H__*/
