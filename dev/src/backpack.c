#include <backpack.h>
#include <renderer.h>

b8         active = 1;
u32        sprite;
struct v2f position;

b8 *
backpack_begin(struct v2f start_position) {
	sprite  = renderer_sprite_alloc(V2F(2, 0), V2F(1, 1), 1, 0);
	position = V2F(start_position.x + 2, start_position.y + 2);
	return &active;
}

struct v2f
backpack_position_get(void) {
	return position;
}

void
backpack_position_set(struct v2f new_position) {
	position = new_position;
}

void
backpack_draw(void) {
	if (!active) return;
	renderer_sprite(sprite, position, V2F(1, 1), V2B(0, 0), 0);
}
