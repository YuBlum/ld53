#include <game.h>
#include <player.h>
#include <config.h>
#include <renderer.h>
#include <dungeon_generator.h>

static struct block blocks[GAME_WIDTH * GAME_HEIGHT];
static u32 block_sprite;

void
game_begin(void) {
	dungeon_generate();
	player_begin();
	block_sprite = renderer_sprite_alloc(V2F(0, 4), V2F(1, 1), 16, 0);
	i8 test_map[] =
		"###....###"
		"#........#"
		"#........#"
		".........."
		".........."
		".........."
		"#........#"
		"#........#"
		"###....###";
	for (i32 i = 0; i < GAME_HEIGHT; i++) {
		for (i32 j = 0; j < GAME_WIDTH; j++) {
			if (test_map[i * GAME_WIDTH + j] == '#') {
				u32 index = i * GAME_WIDTH + j;
				blocks[index].exists   = 1;
				blocks[index].position = V2F(j - 5, (GAME_HEIGHT - 1) - i - 5);
				if (i > 0 && blocks[(i - 1) * GAME_WIDTH + j].exists) {
					blocks[(i - 1) * GAME_WIDTH + j].col_mask |= DOWN;
					blocks[index].col_mask                    |= UP;
				}
				if (j > 0 && blocks[i * GAME_WIDTH + (j - 1)].exists) {
					blocks[i * GAME_WIDTH + (j - 1)].col_mask |= RIGHT;
					blocks[index].col_mask                    |= LEFT;
				}
			}
		}
	}
}

void
game_update(f64 delta_time) {
	player_update(delta_time, blocks);
}

void
game_draw(void) {
	player_draw();
	for (u32 i = 0; i < GAME_WIDTH * GAME_HEIGHT; i++) {
		if (!blocks[i].exists) continue;
		renderer_sprite_frame_set(block_sprite, blocks[i].col_mask);
		renderer_sprite(block_sprite, blocks[i].position, V2F(1, 1), V2B(0, 0));
	}
	//dungeon_draw();
}

void
game_draw_ui(void) {
}

void
game_end(void) {
}
