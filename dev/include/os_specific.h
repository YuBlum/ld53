#ifndef __OS_SPECIFIC_H__
#define __OS_SPECIFIC_H__

#include <types.h>

void  os_setup(void);
void *lib_open(i8 *name);
void *lib_function(void *lib, i8 *name);
void  lib_close(void *lib);

#endif/*__OS_SPECIFIC_H__*/
