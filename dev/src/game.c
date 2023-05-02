#include <game.h>
#include <math.h>
#include <window.h>
#include <sound.h>
#include <stdio.h>
#include <config.h>
#include <player.h>
#include <backpack.h>
#include <renderer.h>
#include <keyboard.h>
#include <dungeon_generator.h>

static u32 music, music_source;

static u32 logo;
static f32 blink_timer;
static b8  show_text = 1;
static f32 logo_time;
static f32 logo_offset;

void
menu_begin(void) {
	logo = renderer_sprite_alloc(V2F(10, 0), V2F(6, 2), 1, 0);
	keyboard_delay_set(0.5f);
}

b8
menu_update(f64 delta_time) {
	logo_time += delta_time;
	blink_timer += delta_time;
	if (blink_timer > 0.5f) {
		blink_timer = 0;
		show_text = !show_text;
	}
	logo_offset = sin(logo_time * 3) * 0.5f;
	if (keyboard_click(ATTACK)) {
		menu_end();
		game_begin();
		return 0;
	}
	return 1;
}

void
menu_draw(void) {
	renderer_sprite(logo, V2F(-2.75f, 1 + logo_offset), V2F(6, 2), V2B(0, 0), 0);
	if (show_text) renderer_text(V2F(-2.75f, -1.5f), "press enter\n  to start");
	renderer_text(V2F(-3.75f, -5), "created by blum");
}

void
menu_end(void) {
	renderer_sprite_free(logo);
}

void
game_begin(void) {
	struct dungeon_generation_result result = dungeon_generate();
	sound_master_begin();
	music        = sound_alloc("dungeon");
	music_source = sound_source_alloc(music, 1);
	sound_source_play(music_source);
	sound_source_gain(music_source, 0.5f);
	player_begin(result.start_position, result.ladder, backpack_begin(result.start_position));
}

void
game_update(f64 delta_time) {
	if (!player_update(delta_time)) {
		window_close();
	}
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
	player_end();
	sound_source_free(music_source);
	sound_free(music);
	sound_master_end();
	dungeon_generator_cleanup();
}
