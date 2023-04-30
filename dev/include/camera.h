#ifndef __CAMERA_H__
#define __CAMERA_H__

#include <linear_algebra.h>

void       camera_setup(void);
struct m4f camera_projview(b8 ui);

#endif/*__CAMERA_H__*/
