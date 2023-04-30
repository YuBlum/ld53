#ifndef __LINEAR_ALGEBRA_H__
#define __LINEAR_ALGEBRA_H__

#include <types.h>

struct m4f { f32 m[4][4]; };
struct v2f { f32 x, y;    };
struct v2b { b8  x, y;    };
struct v2u { u32 x, y;    };

#define M4F_IDENTITY ((struct m4f) {{ { 1, 0, 0, 0 }, { 0, 1, 0, 0 }, { 0, 0, 1, 0 }, { 0, 0, 0, 1 } }})
#define V2F(X, Y) ((struct v2f) { X, Y })
#define V2B(X, Y) ((struct v2b) { X, Y })
#define V2U(X, Y) ((struct v2u) { X, Y })

struct m4f m4f_multiply(struct m4f m1, struct m4f m2);

#endif/*__LINEAR_ALGEBRA_H__*/
