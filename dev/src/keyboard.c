#include <glfw3.h>
#include <keyboard.h>

#define KEYS_AMOUNT 5
static i32 keymap[KEYS_AMOUNT] = {
	GLFW_KEY_W,
	GLFW_KEY_A,
	GLFW_KEY_S,
	GLFW_KEY_D,
	GLFW_KEY_ENTER
};

static u8  keys;
static u8  keys_prev;
static f32 timer;
static f32 delay;

void
keyboard_update(void *window, i32 (*glfw_get_key)(void *, i32), f64 delta_time) {
	keys_prev = keys;
	for (u32 i = 0; i < KEYS_AMOUNT; i++) {
		if (glfw_get_key(window, keymap[i]) == GLFW_PRESS) keys |=   1 << i;
		else                                               keys &= ~(1 << i);
	}
	if (timer > 0) timer -= delta_time;
	else           timer  = 0.25f;
	if (delay > 0) delay -= delta_time;
}

void
keyboard_delay_set(f32 new_delay) {
	delay = new_delay;
}

b8
keyboard_click(u8 key) {
	if (delay > 0) return 0;
	return (keys & key) == key && (keys_prev & key) != key;
}

b8
keyboard_down(u8 key) {
	if (keyboard_click(key)) return 1;
	if (timer > 0) return 0;
	return (keys & key) == key;
}
