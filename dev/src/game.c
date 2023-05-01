#include <game.h>
#include <stdio.h>
#include <player.h>
#include <config.h>
#include <renderer.h>
#include <dungeon_generator.h>

void
game_begin(void) {
	player_begin(dungeon_generate());
}

void
game_update(f64 delta_time) {
	player_update(delta_time);
}

void
game_draw(void) {
	player_draw();
	dungeon_draw();
}

void
game_draw_ui(void) {
	/*dungeon_debug();*/
}

void
game_end(void) {
	dungeon_generator_cleanup();
}
