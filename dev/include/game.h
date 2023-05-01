#ifndef __GAME_H__
#define __GAME_H__

#include <types.h>
#include <linear_algebra.h>

struct block {
	struct v2f position;
	u8 col_mask;
	b8 exists;
};

void game_begin(void);
void game_update(f64 delta_time);
void game_draw(void);
void game_draw_ui(void);
void game_end(void);

#endif/*__GAME_H__*/
