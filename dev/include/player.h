#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <game.h>
#include <types.h>
#include <config.h>

void player_begin(struct v2f start_position, struct v2f ladder_position, b8 *backpack_active);
b8   player_update(f64 delta_time);
void player_draw(void);
void player_hud(void);
void player_end(void);

#endif/*__PLAYER_H__*/
