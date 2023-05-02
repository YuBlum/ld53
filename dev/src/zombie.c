#include <game.h>
#include <stdio.h>
#include <stdlib.h>
#include <config.h>
#include <zombie.h>
#include <renderer.h>
#include <math_helper.h>
#include <dungeon_generator.h>

#define ZOMBIE_SPEED            4
#define ZOMBIE_CHANGE_DIR_SPEED 2

void
zombie_begin(struct zombie *zombie) {
	zombie->active        = 1;
	zombie->flip          = 0;
	zombie->position      = zombie->start_position;
	zombie->position_next = zombie->position;
	zombie->sprite        = renderer_sprite_alloc(V2F(1, 3), V2F(1, 1), 3, 0.25f);
}

void
zombie_update(struct zombie *zombie, f64 delta_time) {
	renderer_sprite_update(zombie->sprite, delta_time, 1);
	zombie->timer += delta_time * ZOMBIE_CHANGE_DIR_SPEED;
	if (zombie->timer >= 1.0f) {
		zombie->timer = 0;
		zombie->direction = rand() % DIR_COUNT;
	}
	if (zombie->position.x == zombie->position_next.x && zombie->position.y == zombie->position_next.y && zombie->walk_timer == 0) {
		zombie->position_prev = zombie->position;
		switch (zombie->direction) {
			case DIR_RIGHT:
				zombie->position_next = V2F(zombie->position.x + 1, zombie->position.y);
				zombie->flip = 0;
				break;
			case DIR_LEFT:
				zombie->position_next = V2F(zombie->position.x - 1, zombie->position.y);
				zombie->flip = 1;
				break;
			case DIR_UP:
				zombie->position_next = V2F(zombie->position.x, zombie->position.y + 1);
				break;
			case DIR_DOWN:
				zombie->position_next = V2F(zombie->position.x, zombie->position.y - 1);
				break;
		}
		if (zombie->position.x != zombie->position_next.x || zombie->position.y != zombie->position_next.y) {
			b8 collided = 0;
			if (
				zombie->position_next.x - zombie->offset.x < 0                  ||
				zombie->position_next.x - zombie->offset.x > (CAMERA_WIDTH - 1) ||
				zombie->position_next.y - zombie->offset.y < 0                  ||
				zombie->position_next.y - zombie->offset.y > (CAMERA_HEIGHT - 1)
			) {
				zombie->position_next = zombie->position;
				collided = 1;
			}
			struct block *blocks = dungeon_blocks(zombie->position);
			for (u32 i = 0; i < GAME_WIDTH * GAME_HEIGHT && !collided; i++) {
				if (!blocks[i].exists) continue;
				if (zombie->position_next.x == blocks[i].position.x && zombie->position_next.y == blocks[i].position.y) {
					zombie->position_next = zombie->position;
					break;
				}
			}
		}
	} else if (zombie->walk_timer < 1) {
		zombie->walk_timer += delta_time * ZOMBIE_SPEED;
		zombie->position.x = lerp(zombie->position_prev.x, zombie->position_next.x, zombie->walk_timer);
		zombie->position.y = lerp(zombie->position_prev.y, zombie->position_next.y, zombie->walk_timer);
	} else {
		zombie->walk_timer = 0;
		zombie->position = zombie->position_next;
	}
}

void
zombie_draw(struct zombie *zombie) {
	renderer_sprite(zombie->sprite, zombie->position, V2F(1, 1), V2B(zombie->flip, 0), 0);
}

void
zombie_end(struct zombie *zombie) {
	zombie->active = 0;
	renderer_sprite_free(zombie->sprite);
}
