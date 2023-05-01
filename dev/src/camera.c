#include <math.h>
#include <camera.h>
#include <config.h>

struct m4f projection;
struct m4f projview;

void
camera_setup(void) {
	const f32 left   = -GAME_WIDTH  >> 1;
	const f32 right  =  GAME_WIDTH  >> 1;
	const f32 top    =  GAME_HEIGHT >> 1;
	const f32 bottom = -GAME_HEIGHT >> 1;
	const f32 right_left = 1 / (right - left);
	const f32 top_bottom = 1 / (top - bottom);
	projection.m[0][0] =  2 * right_left;
	projection.m[1][1] =  2 * top_bottom;
	projection.m[2][2] = -2;
	projection.m[0][3] = -(right + left  ) * right_left;
	projection.m[1][3] = -(top   + bottom) * top_bottom;
	projection.m[2][3] = -1;
	projection.m[3][3] = 1;
	projview = projection;
}

void
camera_move(struct v2f position) {
	struct m4f view = M4F_IDENTITY;
	view.m[0][3] = -position.x;
	view.m[1][3] = -position.y;
	projview = m4f_multiply(projection, view);
}

struct m4f
camera_projview(b8 ui) {
	return ui ? projection : projview;
}
