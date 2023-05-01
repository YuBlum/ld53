#include <game.h>
#include <stdio.h>
#include <config.h>
#include <player.h>
#include <backpack.h>
#include <renderer.h>
#include <dungeon_generator.h>

void
game_begin(void) {
	struct v2f start_position = dungeon_generate();
	player_begin(start_position, backpack_begin(start_position));
}

void
game_update(f64 delta_time) {
	player_update(delta_time);
}

void
game_draw(void) {
	dungeon_draw();
	backpack_draw();
	player_draw();
}

void
game_draw_ui(void) {
	player_hud();
}

void
game_end(void) {
	dungeon_generator_cleanup();
}
