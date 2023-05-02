#ifndef __ZOMBIE_H__
#define __ZOMBIE_H__

#include <types.h>
#include <linear_algebra.h>

struct zombie {
	b8         destroyed;
	b8         active;
	b8         exists;
	b8         flip;
	struct v2f start_position;
	struct v2f position_prev;
	struct v2f position_next;
	struct v2f position;
	struct v2f offset;
	f32        walk_timer;
	f32        timer;
	u32        direction;
	u32        sprite;
};

void zombie_begin(struct zombie *zombie);
void zombie_update(struct zombie *zombie, f64 delta_time);
void zombie_draw(struct zombie *zombie);
void zombie_end(struct zombie *zombie);

#endif/*__ZOMBIE_H__*/
