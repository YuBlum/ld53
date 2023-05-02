#ifndef __BACKPACK_H__
#define __BACKPACK_H__

#include <types.h>
#include <linear_algebra.h>

b8         *backpack_begin(struct v2f start_position);
void        backpack_position_set(struct v2f new_position);
struct v2f  backpack_position_get(void);
void        backpack_draw(void);

#endif/*__BACKPACK_H__*/
