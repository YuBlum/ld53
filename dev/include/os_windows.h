#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>

static i8  bin_path[PATH_MAX];
static i32 bin_path_len;
static i8  temp_path[PATH_MAX];

void
os_setup(void) {
	bin_path_len           = GetModuleFileName(NULL, bin_path, sizeof (bin_path));
	bin_path[bin_path_len] = '\0';
	for (; bin_path_len > 0 && bin_path[bin_path_len] != '\\'; bin_path_len--) bin_path[bin_path_len] = '\0';
}

void *
lib_open(i8 *name) {
	strcpy(temp_path, bin_path);
	strcat(temp_path, "libs/");
	strcat(temp_path, name);
	void *lib = LoadLibrary(temp_path);
	if (!lib) {
		fprintf(stderr, "could not load: %s", name);
		exit(1);
	}
	return lib;
}

void *
lib_function(void *lib, i8 *name) {
	void *function = GetProcAddress(lib, name);
	if (!function) {
		fprintf(stderr, "could not load: %s", name);
		exit(1);
	}
	return function;
}

void
lib_close(void *lib) {
	FreeLibrary(lib);
}

i8 *
resource_path(const i8 *name) {
	sprintf(temp_path, "%s%s", bin_path, name);
	return temp_path;
}
