#ifndef __OS_SPECIFIC_H__
#define __OS_SPECIFIC_H__

#include <types.h>

void  os_setup(void);
void *lib_open(i8 *name);
void *lib_function(void *lib, i8 *name);
void  lib_close(void *lib);
i8   *resource_path(const i8 *name);

#endif/*__OS_SPECIFIC_H__*/
