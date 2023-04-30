#ifndef __MATH_HELPER_H__
#define __MATH_HELPER_H__

#include <types.h>
#include <linear_algebra.h>

#define SWAP32(V1, V2) do {\
	*(u32 *)&(V1) ^= *(u32 *)&(V2);\
	*(u32 *)&(V2) ^= *(u32 *)&(V1);\
	*(u32 *)&(V1) ^= *(u32 *)&(V2);\
} while(0)

f32 lerp(f32 a, f32 b, f32 t);
f32 clamp(f32 x, f32 min, f32 max);
f32 smootherstep(f32 a, f32 b, f32 t);
b8  collided(struct v2f p1, struct v2f s1, struct v2f p2, struct v2f s2);

#endif/*__MATH_HELPER_H__*/
