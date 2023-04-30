#include <camera.h>
#include <config.h>

struct m4f projection;
struct m4f projview;

void
camera_setup(void) {
	const f32 left   = -GAME_WIDTH  * 0.5f;
	const f32 top    =  GAME_HEIGHT * 0.5f;
	const f32 right  =  GAME_WIDTH  * 0.5f;
	const f32 bottom = -GAME_HEIGHT * 0.5f;
	const f32 far    = 1;
	const f32 near   = 0;
	const f32 right_left = 1 / (right - left);
	const f32 top_bottom = 1 / (top - bottom);
	const f32 far_near   = 1 / (far - near);
	projection.m[0][0] =  2 * right_left;
	projection.m[1][1] =  2 * top_bottom;
	projection.m[2][2] = -2 * far_near;
	projection.m[0][3] = -(right + left  ) * right_left;
	projection.m[1][3] = -(top   + bottom) * top_bottom;
	projection.m[2][3] = -(far   + near  ) * far_near;
	projection.m[3][3] = 1;
	projview = projection; /* TODO: change this */
}

struct m4f
camera_projview(b8 ui) {
	return ui ? projection : projview;
}
