#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <config.h>
#include <string.h>
#include <keyboard.h>
#include <renderer.h>
#include <math_helper.h>
#include <linear_algebra.h>
#include <dungeon_generator.h>

#define ROOMS_AMOUNT 5
#define DUNGEON_SIZE 5

static const f32  pixel = 1.0f / FRAME_SIZE_PIXELS;
static u32        square;
static u32        dungeon[DUNGEON_SIZE * DUNGEON_SIZE];
static u32        free_rooms_amount;
static struct v2u free_rooms[DUNGEON_SIZE * DUNGEON_SIZE];

static void
add_free_room(struct v2u room) {
	if (dungeon[room.y * DUNGEON_SIZE + room.x] != 0) return;
	for (u32 i = 0; i < free_rooms_amount; i++) {
		if (free_rooms[i].x == room.x && free_rooms[i].y == room.y) return;
	}
	free_rooms[free_rooms_amount++] = V2U(room.x, room.y);
}

static void
free_surrounds(struct v2u room) {
	if (room.y > 0)                add_free_room(V2U(room.x + 0, room.y - 1));
	if (room.y < DUNGEON_SIZE - 1) add_free_room(V2U(room.x + 0, room.y + 1));
	if (room.x < DUNGEON_SIZE - 1) add_free_room(V2U(room.x + 1, room.y + 0));
	if (room.x > 0)                add_free_room(V2U(room.x - 1, room.y + 0));
}

static void
remove_free_room(u32 index) {
	if (free_rooms_amount == 0) return;
	memmove(free_rooms + index, free_rooms + index + 1, (free_rooms_amount - index) * sizeof (struct v2u));
	free_rooms_amount--;
}

void
dungeon_generate(void) {
	square = renderer_sprite_alloc(V2F(15, 15), V2F(1, 1), 1, 0);
	struct v2u start_room = {
		rand() % DUNGEON_SIZE,
		rand() % DUNGEON_SIZE
	};
	dungeon[start_room.y * DUNGEON_SIZE + start_room.x] = 1;
	printf("%u %u\n", start_room.x, start_room.y);
	free_surrounds(start_room);
	for (u32 i = 0; i < ROOMS_AMOUNT - 1; i++) {
		u32        index = rand() % free_rooms_amount;
		struct v2u room  = free_rooms[index];
		printf("%u %u   %u  %u\n", room.x, room.y, index, free_rooms_amount);
		remove_free_room(index);
		dungeon[room.y * DUNGEON_SIZE + room.x] = 1;
		free_surrounds(room);
	}
}

void
dungeon_draw(void) {
	struct v2f offset = { 0, 0 };
	f32 size = pixel * 2;
	for (u32 i = 0; i < DUNGEON_SIZE; i++) {
		for (u32 j = 0; j < DUNGEON_SIZE; j++) {
			if (dungeon[i * DUNGEON_SIZE + j] == 0) continue;
			renderer_sprite(square, V2F(offset.x + (j * size), offset.y + (i * size)), V2F(size, size), V2B(0, 0));
		}
	}
}
