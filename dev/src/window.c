#include <game.h>
#include <stdio.h>
#include <glfw3.h>
#include <config.h>
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

void
window_begin(void) {
	/* load glfw */
#ifdef LINUX
	void *glfw = lib_open("libglfw.so");
#else
	void *glfw = lib_open("glfw3.dll");
#endif
	glfw_init                 = lib_function(glfw, "glfwInit");
	glfw_terminate            = lib_function(glfw, "glfwTerminate");
	glfw_window_hint          = lib_function(glfw, "glfwWindowHint");
	glfw_create_window        = lib_function(glfw, "glfwCreateWindow");
	glfw_make_context_current = lib_function(glfw, "glfwMakeContextCurrent");
	glfw_window_should_close  = lib_function(glfw, "glfwWindowShouldClose");
	glfw_poll_events          = lib_function(glfw, "glfwPollEvents");
	glfw_swap_buffers         = lib_function(glfw, "glfwSwapBuffers");
	glfw_get_error            = lib_function(glfw, "glfwGetError");
	glfw_get_primary_monitor  = lib_function(glfw, "glfwGetPrimaryMonitor");
	glfw_get_video_mode       = lib_function(glfw, "glfwGetVideoMode");
	glfw_set_window_pos       = lib_function(glfw, "glfwSetWindowPos");
	glfw_get_proc_address     = lib_function(glfw, "glfwGetProcAddress");
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
	void *window = glfw_create_window(WINDOW_WIDTH, WINDOW_HEIGHT, "ld53", NULL, NULL);
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
	renderer_begin(glfw_get_proc_address);
	game_begin();
	while (!glfw_window_should_close(window)) {
		game_update();
		renderer_update();
		glfw_poll_events();
		glfw_swap_buffers(window);
	}
	game_end();
	renderer_end();
	/* close glfw */
	glfw_terminate();
	lib_close(glfw);
}
