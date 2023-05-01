#ifndef __DUNGEON_GENERATOR_H__
#define __DUNGEON_GENERATOR_H__

struct v2f    dungeon_generate(void);
struct block *dungeon_blocks(struct v2f position);
void          dungeon_debug(void);
void          dungeon_draw(void);
void          dungeon_generator_cleanup(void);

#endif/*__DUNGEON_GENERATOR_H__*/
