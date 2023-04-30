#include <math_helper.h>

f32
lerp(f32 a, f32 b, f32 t) {
	return a * (1 - t) + b * t;
}

f32
clamp(f32 x, f32 min, f32 max) {
	return x < min ? min : x > max ? max : x;
}

f32
smootherstep(f32 a, f32 b, f32 t) {
	t = clamp((t - a) / (b - a), 0, 1);
	return t * t * t * (t * (t * 6 - 15) + 10);
}

b8
collided(struct v2f p1, struct v2f s1, struct v2f p2, struct v2f s2) {
	return p1.x + s1.x > p2.x && p1.x < p2.x + s2.x && p1.y + s1.y > p2.y && p1.y < p2.y + s2.y;
}
