#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <game.h>
#include <types.h>
#include <config.h>

void player_begin(void);
void player_update(f64 delta_time, struct block blocks[GAME_WIDTH * GAME_HEIGHT]);
void player_draw(void);

#endif/*__PLAYER_H__*/
