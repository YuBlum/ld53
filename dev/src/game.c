#include <game.h>
#include <sound.h>
#include <stdio.h>
#include <config.h>
#include <player.h>
#include <backpack.h>
#include <renderer.h>
#include <dungeon_generator.h>

u32 music, music_source;

void
game_begin(void) {
	struct v2f start_position = dungeon_generate();
	player_begin(start_position, backpack_begin(start_position));
	sound_master_begin();
	music        = sound_alloc("dungeon");
	music_source = sound_source_alloc(music, 1);
	sound_source_play(music_source);
	sound_source_gain(music_source, 0.5f);
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
	sound_source_free(music_source);
	sound_free(music);
	sound_master_end();
	dungeon_generator_cleanup();
}
