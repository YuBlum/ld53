#ifndef __DUNGEON_GENERATOR_H__
#define __DUNGEON_GENERATOR_H__

struct dungeon_generation_result {
	struct v2f start_position;
	struct v2f ladder;
};

struct block {
	struct v2f position;
	u8 col_mask;
	b8 exists;
};

struct heart {
	b8         active;
	f32        timer;
	struct v2f position;
};


struct dungeon_generation_result  dungeon_generate(void);
struct block                     *dungeon_blocks(struct v2f position);
struct heart                     *dungeon_hearts(struct v2f position);
struct zombie                    *dungeon_zombies(struct v2f position);
void                              dungeon_ressurect_zombies(void);
void                              dungeon_kill_zombie(struct v2f position, u32 zombie_index);
void                              dungeon_room_update(struct v2f position, f64 delta_time);
void                              dungeon_draw(void);
void                              dungeon_generator_cleanup(void);

#endif/*__DUNGEON_GENERATOR_H__*/
