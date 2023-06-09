#include <game.h>
#include <time.h>
#include <glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <config.h>
#include <keyboard.h>
#include <renderer.h>
#include <os_specific.h>

static void  (*glfw_window_hint)(i32 hint, i32 value);
static void *(*glfw_create_window)(i32 width, i32 height, const i8 *title, void *monitor, void *share);
static i32   (*glfw_init)(void);
static void  (*glfw_terminate)(void);
static void  (*glfw_make_context_current)(void *window);
static i32   (*glfw_window_should_close)(void *window);
static void  (*glfw_poll_events)(void);
static void  (*glfw_swap_buffers)(void *window);
static i32   (*glfw_get_error)(const i8 **desc);
static void *(*glfw_get_primary_monitor)(void);
static void *(*glfw_get_video_mode)(void *monitor);
static void  (*glfw_set_window_pos)(void *window, i32 xpos, i32 ypos);
static void *(*glfw_get_proc_address)(const i8 *name);
static f64   (*glfw_get_time)(void);
static i32   (*glfw_get_key)(void *, i32);
static void  (*glfw_set_window_should_close)(void *, i32);

static void *window;

void
window_begin(void) {
	/* load glfw */
#ifdef LINUX
	void *glfw = lib_open("libglfw.so");
#else
	void *glfw = lib_open("glfw3.dll");
#endif
	glfw_init                    = lib_function(glfw, "glfwInit");
	glfw_terminate               = lib_function(glfw, "glfwTerminate");
	glfw_window_hint             = lib_function(glfw, "glfwWindowHint");
	glfw_create_window           = lib_function(glfw, "glfwCreateWindow");
	glfw_make_context_current    = lib_function(glfw, "glfwMakeContextCurrent");
	glfw_window_should_close     = lib_function(glfw, "glfwWindowShouldClose");
	glfw_poll_events             = lib_function(glfw, "glfwPollEvents");
	glfw_swap_buffers            = lib_function(glfw, "glfwSwapBuffers");
	glfw_get_error               = lib_function(glfw, "glfwGetError");
	glfw_get_primary_monitor     = lib_function(glfw, "glfwGetPrimaryMonitor");
	glfw_get_video_mode          = lib_function(glfw, "glfwGetVideoMode");
	glfw_set_window_pos          = lib_function(glfw, "glfwSetWindowPos");
	glfw_get_proc_address        = lib_function(glfw, "glfwGetProcAddress");
	glfw_get_time                = lib_function(glfw, "glfwGetTime");
	glfw_get_key                 = lib_function(glfw, "glfwGetKey");
	glfw_set_window_should_close = lib_function(glfw, "glfwSetWindowShouldClose");
	/* open window */
	if (!glfw_init()) {
		const i8 *error;
		glfw_get_error(&error);
		fprintf(stderr, "error: could not initialize glfw: %s\n", error);
		return;
	}
	glfw_window_hint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfw_window_hint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfw_window_hint(GLFW_RESIZABLE, GLFW_FALSE);
	glfw_window_hint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	window = glfw_create_window(WINDOW_WIDTH, WINDOW_HEIGHT, "ld53", NULL, NULL);
	if (!window) {
		const i8 *error;
		glfw_get_error(&error);
		fprintf(stderr, "error: could not create window: %s\n", error);
		glfw_terminate();
		lib_close(glfw);
		return;
	}
	struct { i32 width; i32 height; } *vidmode = glfw_get_video_mode(glfw_get_primary_monitor());
	glfw_set_window_pos(window, (vidmode->width >> 1) - (WINDOW_WIDTH >> 1), (vidmode->height >> 1) - (WINDOW_HEIGHT >> 1));
	glfw_make_context_current(window);
	/* game loop */
	srand(time(NULL));
	renderer_begin(glfw_get_proc_address);
	b8 in_menu = 1;
	menu_begin();
	f64 previous_time = 0;
	while (!glfw_window_should_close(window)) {
		f64 current_time = glfw_get_time();
		f64 delta_time   = current_time - previous_time;
		previous_time    = current_time;
		keyboard_update(window, glfw_get_key, delta_time);
		if (in_menu) {
			in_menu = menu_update(delta_time);
		} else {
			game_update(delta_time);
		}
		renderer_update(in_menu);
		glfw_poll_events();
		glfw_swap_buffers(window);
	}
	if (!in_menu) game_end();
	renderer_end();
	/* close glfw */
	glfw_terminate();
	lib_close(glfw);
}

void
window_close(void) {
	glfw_set_window_should_close(window, 1);
}
