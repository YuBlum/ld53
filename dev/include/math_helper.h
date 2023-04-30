#ifndef __MATH_HELPER_H__
#define __MATH_HELPER_H__

#include <types.h>

#define SWAP32(V1, V2) do {\
	*(u32 *)&(V1) ^= *(u32 *)&(V2);\
	*(u32 *)&(V2) ^= *(u32 *)&(V1);\
	*(u32 *)&(V1) ^= *(u32 *)&(V2);\
} while(0)

#endif/*__MATH_HELPER_H__*/
