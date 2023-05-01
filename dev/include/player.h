#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <game.h>
#include <types.h>
#include <config.h>

void player_begin(struct v2f start_position);
void player_update(f64 delta_time);
void player_draw(void);

#endif/*__PLAYER_H__*/
