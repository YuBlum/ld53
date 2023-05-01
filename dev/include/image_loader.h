#ifndef __IMAGE_LOADER_H__
#define __IMAGE_LOADER_H__

#include <types.h>

struct image {
	u16  width;
	u16  height;
	u32 *pixels;
};

struct image image_load(const i8 *name);

#endif/*__IMAGE_LOADER_H__*/
