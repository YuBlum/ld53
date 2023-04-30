#include <glfw3.h>
#include <keyboard.h>

#define KEYS_AMOUNT 4
static i32 keymap[KEYS_AMOUNT] = {
	GLFW_KEY_W,
	GLFW_KEY_A,
	GLFW_KEY_S,
	GLFW_KEY_D
};

u8 keys;
u8 keys_prev;

void
keyboard_update(void *window, i32 (*glfw_get_key)(void *, i32)) {
	keys_prev = keys;
	for (u32 i = 0; i < KEYS_AMOUNT; i++) {
		if (glfw_get_key(window, keymap[i]) == GLFW_PRESS) keys |= 1 << i;
		else                                               keys &= ~(1 << i);
	}
}

b8
keyboard_click(u8 key) {
	return (keys & key) == key && (keys_prev & key) != key;
}

b8
keyboard_down(u8 key) {
	return (keys & key) == key;
}
