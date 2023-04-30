#include <math.h>
#include <player.h>
#include <config.h>
#include <keyboard.h>
#include <renderer.h>
#include <math_helper.h>
#include <linear_algebra.h>
#include <stdio.h>

#define DIR_STOP  0
#define DIR_UP    1
#define DIR_LEFT  2
#define DIR_DOWN  3
#define DIR_RIGHT 4

static u32        sprite_current;
static u32        sprite_idle;
static u32        sprite_walk;
static struct v2f position = { -1, -1 };
static struct v2f next_position;
static struct v2f prev_position;
static f32        walk_timer;
static const f32  walk_timer_inc = 3.25f;
static u32        input_buffer;
static b8         flip;
static f32        time;

void
player_begin(void) {
	sprite_idle    = renderer_sprite_alloc(V2F(0, 0), V2F(1, 1), 2, 0.5f);
	sprite_walk    = renderer_sprite_alloc(V2F(0, 1), V2F(1, 1), 3, 0);
	sprite_current = sprite_idle;
	next_position  = position;
}

void
player_update(f64 delta_time) {
	time += delta_time;
	if (position.x == next_position.x && position.y == next_position.y && walk_timer == 0) {
		prev_position = position;
		if (keyboard_click(RIGHT) || input_buffer == DIR_RIGHT) {
			input_buffer = DIR_STOP;
			next_position.x++;
			flip = 0;
		} else if (keyboard_click(LEFT) || input_buffer == DIR_LEFT) {
			input_buffer = DIR_STOP;
			next_position.x--;
			flip = 1;
		} else if (keyboard_click(UP) || input_buffer == DIR_UP) {
			input_buffer = DIR_STOP;
			next_position.y++;
		} else if (keyboard_click(DOWN) || input_buffer == DIR_DOWN) {
			input_buffer = DIR_STOP;
			next_position.y--;
		}
		renderer_sprite_update(sprite_current, delta_time);
	} else if (walk_timer < 1) {
		sprite_current = sprite_walk;
		walk_timer += walk_timer_inc * delta_time;
		position.x = lerp(prev_position.x, next_position.x, walk_timer);
		position.y = lerp(prev_position.y, next_position.y, walk_timer);
		u32 frame = lerp(0, 3, walk_timer);
		if (frame < 3) renderer_sprite_frame_set(sprite_walk, frame);
		if (keyboard_click(RIGHT)) input_buffer = DIR_RIGHT;
		if (keyboard_click(LEFT))  input_buffer = DIR_LEFT;
		if (keyboard_click(UP))    input_buffer = DIR_UP;
		if (keyboard_click(DOWN))  input_buffer = DIR_DOWN;
	} else {
		sprite_current = sprite_idle;
		walk_timer = 0;
		position   = next_position;
	}
}

void
player_draw(void) {
	renderer_sprite(sprite_current, position, V2F(1, 1), V2B(flip, 0));
}
