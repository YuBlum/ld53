#include <game.h>
#include <renderer.h>
#include <linear_algebra.h>

static u32        sprite_test;
static struct v2f sprite_test_pos;

void
game_begin(void) {
	sprite_test = renderer_sprite_alloc(V2F(0, 0), V2F(1, 1), 4, 0.1f);
}

void
game_update(f64 /*delta_time*/) {
}

void
game_draw(void) {
	renderer_sprite(sprite_test, sprite_test_pos, V2F(1, 1), V2B(0, 0));
}

void
game_draw_ui(void) {
	renderer_text(V2F(-6, 0), "aaa\nsim");
}

void
game_end(void) {
}
