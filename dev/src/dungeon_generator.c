#include <game.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <config.h>
#include <string.h>
#include <keyboard.h>
#include <renderer.h>
#include <math_helper.h>
#include <image_loader.h>
#include <linear_algebra.h>
#include <dungeon_generator.h>

#define ROOMS_AMOUNT 5
#define DUNGEON_LIMIT 5

static const f32    pixel = 1.0f / FRAME_SIZE_PIXELS;
static u32          square1;
static u32          square2;
static u32          dungeon[DUNGEON_LIMIT * DUNGEON_LIMIT];
static u32          free_rooms_amount;
static struct v2u   free_rooms[DUNGEON_LIMIT * DUNGEON_LIMIT];
static struct image rooms;
static struct block blocks[DUNGEON_LIMIT * DUNGEON_LIMIT][GAME_WIDTH * GAME_HEIGHT];
static u32          block_sprite;

static void
load_room(struct v2u room, u8 lock) {
	u32 room_index = room.y * DUNGEON_LIMIT + room.x;
	if (!dungeon[room_index]) {
		fprintf(stderr, "error: trying to load invalid room\n");
		exit(1);
	}
	u32 index = lock ? 0 : dungeon[room_index] * GAME_HEIGHT;
	struct v2f offset = { room.x * GAME_WIDTH + (GAME_WIDTH >> 1), room.y * GAME_HEIGHT + (GAME_HEIGHT >> 1) };
	for (u32 i = 0; i < GAME_HEIGHT; i++) {
		for (u32 j = 0; j < GAME_WIDTH; j++) {
			u32 pixel = rooms.pixels[(i + index) * GAME_WIDTH + j];
			if (lock) {
				if (!(lock & LEFT)  && j == 0)               continue;
				if (!(lock & RIGHT) && j == GAME_WIDTH  - 1) continue;
				if (!(lock & UP)    && i == 0)               continue;
				if (!(lock & DOWN)  && i == GAME_HEIGHT - 1) continue;
			}
			if (pixel == 0xffffffff) {
				blocks[room_index][i * GAME_WIDTH + j].exists   = 1;
				blocks[room_index][i * GAME_WIDTH + j].position = V2F(offset.x + j, offset.y + (GAME_HEIGHT - 1) - i);
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

struct v2f
dungeon_generate(void) {
	rooms  = image_load("rooms.tga");
	square1 = renderer_sprite_alloc(V2F(15, 15), V2F(1, 1), 1, 0);
	square2 = renderer_sprite_alloc(V2F(14, 15), V2F(1, 1), 1, 0);
	block_sprite = renderer_sprite_alloc(V2F(0, 4), V2F(1, 1), 16, 0);
	struct v2u start_room = {
		rand() % DUNGEON_LIMIT,
		rand() % DUNGEON_LIMIT
	};
	dungeon[start_room.y * DUNGEON_LIMIT + start_room.x] = 1;
	free_surrounds(start_room);
	load_room(start_room, 0);
	for (u32 i = 0; i < ROOMS_AMOUNT - 1; i++) {
		u32        index = rand() % free_rooms_amount;
		struct v2u room  = free_rooms[index];
		remove_free_room(index);
		dungeon[room.y * DUNGEON_LIMIT + room.x] = 1;
		free_surrounds(room);
		load_room(room, 0);
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
			if (!lock) continue;
			load_room(V2U(j, i), lock);
			for (u32 k = 0; k < GAME_HEIGHT; k++) {
				for (u32 l = 0; l < GAME_WIDTH; l++) {
					if (!blocks[room_index][k * GAME_WIDTH + l].exists) continue;
					if (k > 0 && blocks[room_index][(k - 1) * GAME_WIDTH + l].exists) {
						blocks[room_index][(k - 1) * GAME_WIDTH + l].col_mask |= DOWN;
						blocks[room_index][k * GAME_WIDTH + l].col_mask       |= UP;
					}
					if (l > 0 && blocks[room_index][k * GAME_WIDTH + (l - 1)].exists) {
						blocks[room_index][k * GAME_WIDTH + (l - 1)].col_mask |= RIGHT;
						blocks[room_index][k * GAME_WIDTH + l].col_mask       |= LEFT;
					}
				}
			}
		}
	}
	return V2F(start_room.x * GAME_WIDTH + GAME_WIDTH, start_room.y * GAME_HEIGHT + GAME_HEIGHT - 1);
}

struct block *
dungeon_blocks(struct v2f position) {
	struct v2u indexv = { floor(position.x + (GAME_WIDTH  >> 1)) / GAME_WIDTH - 1, floor(position.y + (GAME_HEIGHT >> 1) + 1) / GAME_HEIGHT - 1 };
	u32 index = ( indexv.y * DUNGEON_LIMIT + indexv.x );
	printf("%u\n", index);
	if (!dungeon[index]) {
		fprintf(stderr, "error: trying to get blocks of unexisting room %u\n", index);
		exit(1);
	}
	return blocks[index];
}

void
dungeon_debug(void) {
	struct v2f offset = { 0, 0 };
	f32 size = pixel * 2;
	for (u32 i = 0; i < DUNGEON_LIMIT; i++) {
		for (u32 j = 0; j < DUNGEON_LIMIT; j++) {
			u32 square = dungeon[i * DUNGEON_LIMIT + j] ? square1 : square2;
			renderer_sprite(square, V2F(offset.x + (j * size), offset.y + (i * size)), V2F(size, size), V2B(0, 0));
		}
	}
}

void
dungeon_draw(void) {
	for (u32 i = 0; i < DUNGEON_LIMIT * DUNGEON_LIMIT; i++) {
		for (u32 j = 0; j < GAME_WIDTH * GAME_HEIGHT; j++) {
			if (!blocks[i][j].exists) continue;
			renderer_sprite_frame_set(block_sprite, blocks[i][j].col_mask);
			renderer_sprite(block_sprite, blocks[i][j].position, V2F(1, 1), V2B(0, 0));
		}
	}
}

void
dungeon_generator_cleanup(void) {
	free(rooms.pixels);
}
