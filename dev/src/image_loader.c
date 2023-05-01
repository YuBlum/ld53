#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <os_specific.h>
#include <image_loader.h>

#pragma pack(1)
struct tga_header {
	u8  id_length;
	u8  color_map_type;
	u8  image_type;
	u16 color_map_origin;
	u16 color_map_length;
	u8  color_map_depth;
	u16 x;
	u16 y;
	u16 width;
	u16 height;
	u8  bpp;
	u8  image_descriptor;
};
#pragma pack()

struct image
image_load(const i8 *name) {
	i8 image_name[256];
	sprintf(image_name, "data/%s.tga", name);
	i8 *path = resource_path(image_name);
	FILE *file = fopen(path, "rb");
	if (!file) {
		fprintf(stderr, "error: could not open %s: %s\n", path, strerror(errno));
		exit(1);
	}
	struct tga_header header;
	fread(&header, 1, sizeof (header), file);
	fseek(file, header.id_length, SEEK_CUR);
	u32 pixels_amount = header.width * header.height;
	struct image image = {
		.width  = header.width,
		.height = header.height,
		.pixels = malloc(sizeof (u32) * pixels_amount)
	};
	fread(image.pixels, sizeof (u32), pixels_amount, file);
	fclose(file);
	return image;
}
