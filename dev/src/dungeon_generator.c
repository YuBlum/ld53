#include <game.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <config.h>
#include <zombie.h>
#include <assert.h>
#include <string.h>
#include <keyboard.h>
#include <renderer.h>
#include <math_helper.h>
#include <image_loader.h>
#include <linear_algebra.h>
#include <dungeon_generator.h>

#define ROOMS_AMOUNT 20
#define DUNGEON_LIMIT 10

static u32           dungeon[DUNGEON_LIMIT * DUNGEON_LIMIT];
static u32           free_rooms_amount;
static struct v2u    free_rooms[DUNGEON_LIMIT * DUNGEON_LIMIT];
static struct v2u    rooms_indexes[ROOMS_AMOUNT - 1];
static struct image  rooms;
static u32           block_sprite;
static struct block  blocks[DUNGEON_LIMIT * DUNGEON_LIMIT][CAMERA_WIDTH * CAMERA_HEIGHT];
static u32           heart_sprite;
static struct heart  hearts[DUNGEON_LIMIT * DUNGEON_LIMIT][ENTITIES_MAX_PER_ROOM];
static struct zombie zombies[DUNGEON_LIMIT * DUNGEON_LIMIT][ENTITIES_MAX_PER_ROOM];
static u32           zombies_amount[DUNGEON_LIMIT * DUNGEON_LIMIT];
static u32           dead_zombies[DUNGEON_LIMIT * DUNGEON_LIMIT * ENTITIES_MAX_PER_ROOM][2];
static u32           dead_zombies_amount;
static u32           previous_room;
static struct v2f    ladder;
static u32           ladder_sprite;

static void
load_room(struct v2u room, u8 lock) {
	u32 room_index = room.y * DUNGEON_LIMIT + room.x;
	if (!dungeon[room_index]) {
		fprintf(stderr, "error: trying to load invalid room\n");
		exit(1);
	}
	u32 index = lock ? 0 : dungeon[room_index] * CAMERA_HEIGHT;
	struct v2f offset = { room.x * CAMERA_WIDTH + (CAMERA_WIDTH >> 1), room.y * CAMERA_HEIGHT + (CAMERA_HEIGHT >> 1) - 1 };
	for (u32 i = 0; i < CAMERA_HEIGHT; i++) {
		for (u32 j = 0; j < CAMERA_WIDTH; j++) {
			u32 pixel = rooms.pixels[(i + index) * GAME_WIDTH + j];
			if (lock) {
				if (!(lock & LEFT)  && j == 0)               continue;
				if (!(lock & RIGHT) && j == CAMERA_WIDTH  - 1) continue;
				if (!(lock & UP)    && i == 0)               continue;
				if (!(lock & DOWN)  && i == CAMERA_HEIGHT - 1) continue;
			}
			if (pixel == 0xffffffff) {
				blocks[room_index][i * CAMERA_WIDTH + j].exists   = 1;
				blocks[room_index][i * CAMERA_WIDTH + j].position = V2F(offset.x + j, offset.y + (CAMERA_HEIGHT - 1) - i);
			} else if (pixel == 0xffff0000) {
				assert(zombies_amount[room_index] < ENTITIES_MAX_PER_ROOM);
				zombies[room_index][zombies_amount[room_index]].exists         = 1;
				zombies[room_index][zombies_amount[room_index]].offset         = offset;
				zombies[room_index][zombies_amount[room_index]].start_position = V2F(offset.x + j, offset.y + (CAMERA_HEIGHT - 1) - i);
				zombies_amount[room_index]++;
			} else if (pixel == 0xff000000) {
				ladder = V2F(offset.x + j, offset.y + (CAMERA_HEIGHT - 1) - i);
			}
		}
	}
}

static void
add_free_room(struct v2u room) {
	if (dungeon[room.y * DUNGEON_LIMIT + room.x] != 0) return;
	for (u32 i = 0; i < free_rooms_amount; i++) {
		if (free_rooms[i].x == room.x && free_rooms[i].y == room.y) return;
	}
	free_rooms[free_rooms_amount++] = V2U(room.x, room.y);
}

static void
free_surrounds(struct v2u room) {
	if (room.y > 0)                add_free_room(V2U(room.x + 0, room.y - 1));
	if (room.y < DUNGEON_LIMIT - 1) add_free_room(V2U(room.x + 0, room.y + 1));
	if (room.x < DUNGEON_LIMIT - 1) add_free_room(V2U(room.x + 1, room.y + 0));
	if (room.x > 0)                add_free_room(V2U(room.x - 1, room.y + 0));
}

static void
remove_free_room(u32 index) {
	if (free_rooms_amount == 0) return;
	memmove(free_rooms + index, free_rooms + index + 1, (free_rooms_amount - index) * sizeof (struct v2u));
	free_rooms_amount--;
}

struct dungeon_generation_result
dungeon_generate(void) {
	rooms  = image_load("rooms");
	block_sprite  = renderer_sprite_alloc(V2F(0, 4), V2F(1, 1), 16, 0);
	heart_sprite  = renderer_sprite_alloc(V2F(3, 2), V2F(1, 1),  1, 0);
	ladder_sprite = renderer_sprite_alloc(V2F(4, 2), V2F(2, 2),  1, 0);
	struct v2u start_room = {
		rand() % DUNGEON_LIMIT,
		rand() % DUNGEON_LIMIT
	};
	dungeon[start_room.y * DUNGEON_LIMIT + start_room.x] = 2;
	struct v2u farthest_room = { 0 };
	f32 farthest_distance = 0;
	free_surrounds(start_room);
	load_room(start_room, 0);
	for (u32 i = 0; i < ROOMS_AMOUNT - 1; i++) {
		u32        index = rand() % free_rooms_amount;
		struct v2u room  = free_rooms[index];
		remove_free_room(index);
		rooms_indexes[i] = room;
		dungeon[room.y * DUNGEON_LIMIT + room.x] = rand() % 6 + 2;
		free_surrounds(room);
		struct v2f difference = { (f32)room.x - (f32)start_room.x, (f32)room.y - (f32)start_room.y };
		f32 distance = sqrt((difference.x * difference.x) + (difference.y * difference.y));
		if (distance > farthest_distance) {
			farthest_distance = distance;
			farthest_room = room;
		}
	}
	printf("%u %u\n", farthest_room.x, farthest_room.y);
	dungeon[farthest_room.y * DUNGEON_LIMIT + farthest_room.x] = 1;
	for (u32 i = 0; i < ROOMS_AMOUNT - 1; i++) {
		load_room(rooms_indexes[i], 0);
	}
	for (u32 i = 0; i < DUNGEON_LIMIT; i++) {
		for (u32 j = 0; j < DUNGEON_LIMIT; j++) {
			u32 room_index = i * DUNGEON_LIMIT + j;
			if (!dungeon[room_index]) continue;
			u8 lock = 0;
			if (i == 0                   || !dungeon[(i - 1) * DUNGEON_LIMIT + j]) lock |= DOWN;
			if (i == (DUNGEON_LIMIT - 1) || !dungeon[(i + 1) * DUNGEON_LIMIT + j]) lock |= UP;
			if (j == 0                   || !dungeon[i * DUNGEON_LIMIT + (j - 1)]) lock |= LEFT;
			if (j == (DUNGEON_LIMIT - 1) || !dungeon[i * DUNGEON_LIMIT + (j + 1)]) lock |= RIGHT;
			if (lock) load_room(V2U(j, i), lock);
			for (u32 k = 0; k < CAMERA_HEIGHT; k++) {
				for (u32 l = 0; l < CAMERA_WIDTH; l++) {
					if (!blocks[room_index][k * CAMERA_WIDTH + l].exists) continue;
					if (k > 0 && blocks[room_index][(k - 1) * CAMERA_WIDTH + l].exists) {
						blocks[room_index][(k - 1) * CAMERA_WIDTH + l].col_mask |= DOWN;
						blocks[room_index][k * CAMERA_WIDTH + l].col_mask       |= UP;
					}
					if (l > 0 && blocks[room_index][k * CAMERA_WIDTH + (l - 1)].exists) {
						blocks[room_index][k * CAMERA_WIDTH + (l - 1)].col_mask |= RIGHT;
						blocks[room_index][k * CAMERA_WIDTH + l].col_mask       |= LEFT;
					}
				}
			}
		}
	}
	return (struct dungeon_generation_result) { V2F(start_room.x * CAMERA_WIDTH + CAMERA_WIDTH, start_room.y * CAMERA_HEIGHT + CAMERA_HEIGHT - 1), ladder };
}

struct block *
dungeon_blocks(struct v2f position) {
	struct v2u indexv = { floor(position.x + (CAMERA_WIDTH  >> 1)) / CAMERA_WIDTH - 1, floor(position.y + (CAMERA_HEIGHT >> 1) + 1) / CAMERA_HEIGHT - 1 };
	u32 index = ( indexv.y * DUNGEON_LIMIT + indexv.x );
	if (!dungeon[index]) {
		fprintf(stderr, "error: trying to get blocks of unexisting room %u\n", index);
		exit(1);
	}
	return blocks[index];
}

struct zombie *
dungeon_zombies(struct v2f position) {
	struct v2u indexv = { floor(position.x + (CAMERA_WIDTH  >> 1)) / CAMERA_WIDTH - 1, floor(position.y + (CAMERA_HEIGHT >> 1) + 1) / CAMERA_HEIGHT - 1 };
	u32 index = ( indexv.y * DUNGEON_LIMIT + indexv.x );
	if (!dungeon[index]) {
		fprintf(stderr, "error: trying to get zombies of unexisting room %u\n", index);
		exit(1);
	}
	return zombies[index];
}

struct heart *
dungeon_hearts(struct v2f position) {
	struct v2u indexv = { floor(position.x + (CAMERA_WIDTH  >> 1)) / CAMERA_WIDTH - 1, floor(position.y + (CAMERA_HEIGHT >> 1) + 1) / CAMERA_HEIGHT - 1 };
	u32 index = ( indexv.y * DUNGEON_LIMIT + indexv.x );
	if (!dungeon[index]) {
		fprintf(stderr, "error: trying to get hearts of unexisting room %u\n", index);
		exit(1);
	}
	return hearts[index];
}

void
dungeon_ressurect_zombies(void) {
	for (u32 i = 0; i < dead_zombies_amount; i++) {
		zombies[dead_zombies[i][0]][dead_zombies[i][1]].destroyed = 0;
	}
	dead_zombies_amount = 0;
}

void
dungeon_kill_zombie(struct v2f position, u32 zombie_index) {
	struct v2u indexv = { floor(position.x + (CAMERA_WIDTH  >> 1)) / CAMERA_WIDTH - 1, floor(position.y + (CAMERA_HEIGHT >> 1) + 1) / CAMERA_HEIGHT - 1 };
	u32 index = ( indexv.y * DUNGEON_LIMIT + indexv.x );
	if (!dungeon[index]) {
		fprintf(stderr, "error: trying to kill a zombie of unexisting room %u\n", index);
		exit(1);
	}
	if (!zombies[index][zombie_index].exists) {
		fprintf(stderr, "error: trying to kill an unexisting zombie\n");
		exit(1);
	}
	zombies[index][zombie_index].active    = 0;
	zombies[index][zombie_index].destroyed = 1;
	dead_zombies[dead_zombies_amount][0]   = index;
	dead_zombies[dead_zombies_amount++][1] = zombie_index;
	u32 die = rand() % 100;
	if (die < 100) {
		hearts[index][zombie_index].active   = 1;
		hearts[index][zombie_index].timer    = (rand() % 12 + 4) * 0.5f;
		hearts[index][zombie_index].position = V2F(floor(zombies[index][zombie_index].position_next.x), floor(zombies[index][zombie_index].position_next.y));
	}
	zombie_end(&zombies[index][zombie_index]);
}

void
dungeon_room_update(struct v2f position, f64 delta_time) {
	struct v2u indexv = { floor(position.x + (CAMERA_WIDTH  >> 1)) / CAMERA_WIDTH - 1, floor(position.y + (CAMERA_HEIGHT >> 1) + 1) / CAMERA_HEIGHT - 1 };
	u32 index = ( indexv.y * DUNGEON_LIMIT + indexv.x );
	if (!dungeon[index]) {
		fprintf(stderr, "error: trying to update unexisting room %u\n", index);
		exit(1);
	}
	for (u32 i = 0; i < ENTITIES_MAX_PER_ROOM && dungeon[previous_room] && previous_room != index; i++) {
		if (!zombies[previous_room][i].exists || zombies[previous_room][i].destroyed) continue;
		if (zombies[previous_room][i].active) zombie_end(&zombies[previous_room][i]);
	}
	for (u32 i = 0; i < ENTITIES_MAX_PER_ROOM; i++) {
		if (hearts[index][i].active) {
			hearts[index][i].timer -= delta_time;
			if (hearts[index][i].timer <= 0) hearts[index][i].active = 0;
		}
		if (zombies[index][i].exists && !zombies[index][i].destroyed) {
			if (zombies[index][i].active) {
				zombie_update(&zombies[index][i], delta_time);
			} else {
				zombie_begin(&zombies[index][i]);
			}
		}
	}
	previous_room = index;
}

void
dungeon_draw(void) {
	for (u32 i = 0; i < DUNGEON_LIMIT * DUNGEON_LIMIT; i++) {
		for (u32 j = 0; j < CAMERA_WIDTH * CAMERA_HEIGHT; j++) {
			if (!blocks[i][j].exists) continue;
			renderer_sprite_frame_set(block_sprite, blocks[i][j].col_mask);
			renderer_sprite(block_sprite, blocks[i][j].position, V2F(1, 1), V2B(0, 0), 0);
		}
		for (u32 j = 0; j < ENTITIES_MAX_PER_ROOM; j++) {
			if (zombies[i][j].active) {
				zombie_draw(&zombies[i][j]);
			}
			if (hearts[i][j].active) {
				renderer_sprite(heart_sprite, hearts[i][j].position, V2F(1, 1), V2B(0, 0), 0);
			}
		}
	}
	renderer_sprite(ladder_sprite, ladder, V2F(2, 2), V2B(0, 0), 0);
}

void
dungeon_generator_cleanup(void) {
	free(rooms.pixels);
}
