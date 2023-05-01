#include <math.h>
#include <stdio.h>
#include <player.h>
#include <config.h>
#include <camera.h>
#include <keyboard.h>
#include <renderer.h>
#include <math_helper.h>
#include <linear_algebra.h>
#include <dungeon_generator.h>

#define DIR_STOP  0
#define DIR_UP    1
#define DIR_LEFT  2
#define DIR_DOWN  3
#define DIR_RIGHT 4

static u32        sprite_current;
static u32        sprite_idle;
static u32        sprite_walk;
static struct v2f position = { -1, -1 };
static struct v2f position_next;
static struct v2f position_prev;
static struct v2f camera;
static struct v2f camera_next;
static struct v2f camera_prev;
static f32        camera_timer;
static const f32  camera_timer_inc = 2;
static f32        walk_timer;
static const f32  walk_timer_inc = 3.25f;
static u32        input_buffer;
static b8         flip;
static f32        time;

void
player_begin(struct v2f start_position) {
	sprite_idle    = renderer_sprite_alloc(V2F(0, 0), V2F(1, 1), 2, 0.5f);
	sprite_walk    = renderer_sprite_alloc(V2F(0, 1), V2F(1, 1), 3, 0);
	sprite_current = sprite_idle;
	position       = start_position;
	camera         = V2F(
		floor((position.x + (GAME_WIDTH  >> 1)) / GAME_WIDTH)  * GAME_WIDTH,
		floor((position.y + (GAME_HEIGHT >> 1) + 1) / GAME_HEIGHT) * GAME_HEIGHT
	);;
	position_next  = position;
}

void
player_update(f64 delta_time) {
	/* movement */
	time += delta_time;
	if (position.x == position_next.x && position.y == position_next.y && walk_timer == 0) {
		if (camera_timer == 0) {
			position_prev = position;
			if (keyboard_down(RIGHT) || input_buffer == DIR_RIGHT) {
				input_buffer = DIR_STOP;
				position_next.x++;
				flip = 0;
			} else if (keyboard_down(LEFT) || input_buffer == DIR_LEFT) {
				input_buffer = DIR_STOP;
				position_next.x--;
				flip = 1;
			} else if (keyboard_down(UP) || input_buffer == DIR_UP) {
				input_buffer = DIR_STOP;
				position_next.y++;
			} else if (keyboard_down(DOWN) || input_buffer == DIR_DOWN) {
				input_buffer = DIR_STOP;
				position_next.y--;
			}
			struct block *blocks = dungeon_blocks(position);
			for (u32 i = 0; i < GAME_WIDTH * GAME_HEIGHT; i++) {
				if (!blocks[i].exists) continue;
				if (position_next.x == blocks[i].position.x && position_next.y == blocks[i].position.y) {
					position_next = position;
				}
			}
		}
		renderer_sprite_update(sprite_current, delta_time);
	} else if (walk_timer < 1) {
		sprite_current = sprite_walk;
		walk_timer += walk_timer_inc * delta_time;
		position.x = lerp(position_prev.x, position_next.x, walk_timer);
		position.y = lerp(position_prev.y, position_next.y, walk_timer);
		u32 frame = lerp(0, 3, walk_timer);
		if (frame < 3) renderer_sprite_frame_set(sprite_walk, frame);
		if (keyboard_down(RIGHT)) input_buffer = DIR_RIGHT;
		if (keyboard_down(LEFT))  input_buffer = DIR_LEFT;
		if (keyboard_down(UP))    input_buffer = DIR_UP;
		if (keyboard_down(DOWN))  input_buffer = DIR_DOWN;
	} else {
		sprite_current = sprite_idle;
		walk_timer = 0;
		position   = position_next;
	}
	/* camera movement */
	camera_next = V2F(
		floor((position_next.x + (GAME_WIDTH  >> 1)) / GAME_WIDTH)  * GAME_WIDTH,
		floor((position_next.y + (GAME_HEIGHT >> 1) + 1) / GAME_HEIGHT) * GAME_HEIGHT
	);
	if (camera_next.x == camera.x && camera_next.y == camera.y && camera_timer == 0) {
		camera_prev = camera;
	} else if (camera_timer < 1) {
		camera_timer += camera_timer_inc * delta_time;
		f32 t    = smootherstep(0, 1, camera_timer);
		camera.x = lerp(camera_prev.x, camera_next.x, t);
		camera.y = lerp(camera_prev.y, camera_next.y, t);
	} else {
		camera_timer = 0;
		camera       = camera_next;
		camera_prev  = camera;
	}
	camera_move(camera);
}

void
player_draw(void) {
	renderer_sprite(sprite_current, position, V2F(1, 1), V2B(flip, 0));
}
