#ifndef __GAME_H__
#define __GAME_H__

#include <types.h>
#include <linear_algebra.h>


void menu_begin(void);
b8   menu_update(f64 delta_time);
void menu_draw(void);
void menu_end(void);
void game_begin(void);
void game_update(f64 delta_time);
void game_draw(void);
void game_draw_ui(void);
void game_end(void);

#endif/*__GAME_H__*/
