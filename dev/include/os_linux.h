#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <linux/limits.h>

static i8  bin_path[PATH_MAX];
static i32 bin_path_len;
static i8  temp_path[PATH_MAX];

void
os_setup(void) {
	bin_path_len           = readlink("/proc/self/exe", bin_path, PATH_MAX);
	bin_path[bin_path_len] = '\0';
	for (; bin_path_len > 0 && bin_path[bin_path_len] != '/'; bin_path_len--) bin_path[bin_path_len] = '\0';
}

void *
lib_open(i8 *name) {
	strcpy(temp_path, bin_path);
	strcat(temp_path, "libs/");
	strcat(temp_path, name);
	void *lib = dlopen(temp_path, RTLD_LAZY);
	if (!lib) {
		fprintf(stderr, "error: could not load %s: %s\n", name, dlerror());
		exit(1);
	}
	return lib;
}

void *
lib_function(void *lib, i8 *name) {
	void *function = dlsym(lib, name);
	if (!function) fprintf(stderr, "error: could not load %s: %s\n", name, dlerror());
	return function;
}

void
lib_close(void *lib) {
	dlclose(lib);
}

i8 *
resource_path(const i8 *name) {
	sprintf(temp_path, "%s%s", bin_path, name);
	return temp_path;
}
