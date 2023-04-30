#include <game.h>
#include <GL/gl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <config.h>
#include <camera.h>
#include <renderer.h>
#include <GL/glext.h>
#include <os_specific.h>
#include <math_helper.h>
#include <linear_algebra.h>

static void (*gl_clear_color)(f32, f32, f32, f32);
static void (*gl_clear)(u32);
static u32  (*gl_create_shader)(u32);
static void (*gl_shader_source)(u32, i32, const i8 **, const i32 *);
static void (*gl_compile_shader)(u32);
static void (*gl_get_shaderiv)(u32, u32, i32 *);
static void (*gl_get_shader_info_log)(u32, i32, i32 *, i8 *);
static void (*gl_delete_shader)(u32);
static u32  (*gl_create_program)(void);
static void (*gl_attach_shader)(u32, u32);
static void (*gl_link_program)(u32);
static void (*gl_get_programiv)(u32, u32, i32 *);
static void (*gl_get_program_info_log)(u32, i32, i32 *, i8 *);
static void (*gl_delete_program)(u32);
static void (*gl_gen_vertex_arrays)(i32, u32 *);
static void (*gl_bind_vertex_array)(u32);
static void (*gl_delete_vertex_arrays)(i32, const u32 *);
static void (*gl_gen_buffers)(i32, u32 *);
static void (*gl_bind_buffer)(u32, u32);
static void (*gl_buffer_data)(u32, i64, const void *, u32);
static void (*gl_delete_buffers)(i32, const u32 *);
static void (*gl_vertex_attrib_pointer)(u32, i32, u32, b8, i32, const void *);
static void (*gl_enable_vertex_attrib_array)(u32);
static void (*gl_use_program)(u32);
static void (*gl_draw_elements)(u32, i32, u32, const void *);
static void (*gl_buffer_sub_data)(u32, i64, i64, const void *);
static void (*gl_gen_textures)(i32, u32 *);
static void (*gl_bind_texture)(u32, u32);
static void (*gl_tex_parameteri)(u32, u32, i32);
static void (*gl_tex_image_2d)(u32, i32, i32, i32, i32, i32, u32, u32, const void *);
static void (*gl_delete_textures)(i32, const u32 *);
static i32  (*gl_get_uniform_location)(u32, const i8 *);
static void (*gl_uniform_matrix4fv)(i32, i32, b8, const f32 *);
static void (*gl_gen_framebuffers)(i32, u32 *);
static void (*gl_bind_framebuffer)(u32, u32);
static void (*gl_framebuffer_texture_2d)(u32, u32, u32, u32, i32);
static void (*gl_delete_framebuffers)(i32, const u32 *);
static void (*gl_viewport)(i32, i32, i32, i32);
static void (*gl_enable)(u32);
static void (*gl_blend_func)(u32, u32);

#pragma pack(1)
struct tga_header {
	u8  id_length;
	u8  color_map_type;
	u8  image_type;
	u16 color_map_origin;
	u16 color_map_length;
	u8  color_map_depth;
	u16 x;
	u16 y;
	u16 width;
	u16 height;
	u8  bpp;
	u8  image_descriptor;
};
#pragma pack()

struct sprite {
	struct v2f base;
	struct v2f size;
	f32        fps;
	f32        timer;
	f32        offset;
	u32        amount;
	b8         active;
};

struct vertex {
	struct v2f position;
	struct v2f texcoord;
};

static u32 basic_shader;
static i32 u_projview;

static u32           batch_vertex_array;
static u32           batch_vertex_buffer;
static u32           batch_index_buffer;
static struct vertex batch_vertices[VERTEX_CAPACITY];
static u32           batch_vertices_amount;
static u32           batch_indices_amount;
static struct vertex batch_ui_vertices[VERTEX_CAPACITY];
static u32           batch_ui_vertices_amount;
static u32           batch_ui_indices_amount;

static u32 framebuffer;
static u32 framebuffer_texture;
static u32 framebuffer_vertex_array;
static u32 framebuffer_vertex_buffer;
static u32 framebuffer_index_buffer;

static u32        atlas;
static struct v2f one_frame_size;
static struct v2f half_frame_size;

static struct sprite sprites[SPRITE_CAPACITY];
static u32           sprites_pool[SPRITE_CAPACITY];
static u32           sprites_pool_amount;

static b8 can_render;
static b8 is_ui;

static u32
shader_open(i8 *name, u32 type) {
	i8 *path = resource_path(name);
	FILE *file = fopen(path, "r");
	if (!file) {
		fprintf(stderr, "error: could not open %s: %s", path, strerror(errno));
		exit(1);
	}
	fseek(file, 0, SEEK_END);
	const i32 len = ftell(file);
	rewind(file);
	i8 *source = malloc(len + 1);
	fread(source, 1, len, file);
	fclose(file);
	u32 shader = gl_create_shader(type);
	gl_shader_source(shader, 1, (const i8 **)&source, &len);
	free(source);
	gl_compile_shader(shader);
	i32 status;
	gl_get_shaderiv(shader, GL_COMPILE_STATUS, &status);
	if (!status) {
		i8 info_log[256];
		gl_get_shader_info_log(shader, 256, NULL, info_log);
		fprintf(stderr, "error: could not compile %s: %s\n", name, info_log);
		exit(1);
	}
	return shader;
}

static u32
shader_program_create(i8 *name) {
	i8 fragment_name[256], vertex_name[256];
	sprintf(fragment_name, "shaders/%s_fragment.glsl", name);
	sprintf(vertex_name,   "shaders/%s_vertex.glsl",   name);
	u32 fragment = shader_open(fragment_name, GL_FRAGMENT_SHADER);
	u32 vertex   = shader_open(vertex_name,   GL_VERTEX_SHADER);
	u32 program  = gl_create_program();
	gl_attach_shader(program, vertex);
	gl_attach_shader(program, fragment);
	gl_link_program(program);
	gl_delete_shader(vertex);
	gl_delete_shader(fragment);
	i32 status;
	gl_get_programiv(program, GL_LINK_STATUS, &status);
	if (!status) {
		i8 info_log[256];
		gl_get_program_info_log(program, 256, NULL, info_log);
		fprintf(stderr, "error: could not link %s: %s\n", name, info_log);
		exit(1);
	}
	return program;
}

void
renderer_begin(void *(*load_func)(const i8 *name)) {
	/* load opengl */
	gl_clear_color                = load_func("glClearColor");
	gl_clear                      = load_func("glClear");
	gl_create_shader              = load_func("glCreateShader");
	gl_shader_source              = load_func("glShaderSource");
	gl_compile_shader             = load_func("glCompileShader");
	gl_get_shaderiv               = load_func("glGetShaderiv");
	gl_get_shader_info_log        = load_func("glGetShaderInfoLog");
	gl_delete_shader              = load_func("glDeleteShader");
	gl_create_program             = load_func("glCreateProgram");
	gl_attach_shader              = load_func("glAttachShader");
	gl_link_program               = load_func("glLinkProgram");
	gl_get_programiv              = load_func("glGetProgramiv");
	gl_get_program_info_log       = load_func("glGetProgramInfoLog");
	gl_delete_program             = load_func("glDeleteProgram");
	gl_gen_vertex_arrays          = load_func("glGenVertexArrays");
	gl_bind_vertex_array          = load_func("glBindVertexArray");
	gl_delete_vertex_arrays       = load_func("glDeleteVertexArrays");
	gl_gen_buffers                = load_func("glGenBuffers");
	gl_bind_buffer                = load_func("glBindBuffer");
	gl_buffer_data                = load_func("glBufferData");
	gl_delete_buffers             = load_func("glDeleteBuffers");
	gl_vertex_attrib_pointer      = load_func("glVertexAttribPointer");
	gl_enable_vertex_attrib_array = load_func("glEnableVertexAttribArray");
	gl_use_program                = load_func("glUseProgram");
	gl_draw_elements              = load_func("glDrawElements");
	gl_buffer_sub_data            = load_func("glBufferSubData");
	gl_gen_textures               = load_func("glGenTextures");
	gl_bind_texture               = load_func("glBindTexture");
	gl_tex_parameteri             = load_func("glTexParameteri");
	gl_tex_image_2d               = load_func("glTexImage2D");
	gl_delete_textures            = load_func("glDeleteTextures");
	gl_get_uniform_location       = load_func("glGetUniformLocation");
	gl_uniform_matrix4fv          = load_func("glUniformMatrix4fv");
	gl_gen_framebuffers           = load_func("glGenFramebuffers");
	gl_bind_framebuffer           = load_func("glBindFramebuffer");
	gl_framebuffer_texture_2d     = load_func("glFramebufferTexture2D");
	gl_delete_framebuffers        = load_func("glDeleteFramebuffers");
	gl_viewport                   = load_func("glViewport");
	gl_enable                     = load_func("glEnable");
	gl_blend_func                 = load_func("glBlendFunc");
	/* shader */
	basic_shader = shader_program_create("basic");
	gl_use_program(basic_shader);
	/* batch */
	u32 batch_indices[INDEX_CAPACITY];
	u32 cur_vertex = 0;
	for (u32 cur_index = 0; cur_index < INDEX_CAPACITY; cur_index += 6) {
		batch_indices[cur_index + 0] = cur_vertex + 0;
		batch_indices[cur_index + 1] = cur_vertex + 1;
		batch_indices[cur_index + 2] = cur_vertex + 2;
		batch_indices[cur_index + 3] = cur_vertex + 2;
		batch_indices[cur_index + 4] = cur_vertex + 3;
		batch_indices[cur_index + 5] = cur_vertex + 0;
		cur_vertex += 4;
	}
	gl_gen_vertex_arrays(1, &batch_vertex_array);
	gl_gen_buffers(1, &batch_vertex_buffer);
	gl_gen_buffers(1, &batch_index_buffer);
	gl_bind_vertex_array(batch_vertex_array);
	gl_bind_buffer(GL_ELEMENT_ARRAY_BUFFER, batch_index_buffer);
	gl_buffer_data(GL_ELEMENT_ARRAY_BUFFER, sizeof (batch_indices), batch_indices, GL_STATIC_DRAW);
	gl_bind_buffer(GL_ARRAY_BUFFER, batch_vertex_buffer);
	gl_buffer_data(GL_ARRAY_BUFFER, sizeof (batch_vertices), NULL, GL_DYNAMIC_DRAW);
	gl_vertex_attrib_pointer(0, 2, GL_FLOAT, GL_FALSE, sizeof (struct vertex), (void *)offsetof(struct vertex, position));
	gl_enable_vertex_attrib_array(0);
	gl_vertex_attrib_pointer(1, 2, GL_FLOAT, GL_FALSE, sizeof (struct vertex), (void *)offsetof(struct vertex, texcoord));
	gl_enable_vertex_attrib_array(1);
	/* atlas */
	FILE *atlas_file = fopen(resource_path("data/atlas.tga"), "rb");
	if (!atlas_file) {
		fprintf(stderr, "error: could not open atlas: %s", strerror(errno));
	}
	struct tga_header atlas_header;
	fread(&atlas_header, 1, sizeof (atlas_header), atlas_file);
	fseek(atlas_file, atlas_header.id_length, SEEK_CUR);
	u32 pixels_amount = atlas_header.width * atlas_header.height;
	u8 *pixels = malloc(sizeof (u32) * pixels_amount);
	fread(pixels, sizeof (u32), pixels_amount, atlas_file);
	fclose(atlas_file);
	one_frame_size = V2F(
		1.0f / (atlas_header.width  / FRAME_SIZE_PIXELS),
		1.0f / (atlas_header.height / FRAME_SIZE_PIXELS)
	);
	half_frame_size = V2F(one_frame_size.x * 0.5f, one_frame_size.y * 0.5f);
	gl_gen_textures(1, &atlas);
	gl_bind_texture(GL_TEXTURE_2D, atlas);
	gl_tex_parameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	gl_tex_parameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	gl_tex_image_2d(GL_TEXTURE_2D, 0, GL_RGBA, atlas_header.width, atlas_header.height, 0, GL_BGRA, GL_UNSIGNED_BYTE, pixels);
	free(pixels);
	/* uniforms */
	u_projview = gl_get_uniform_location(basic_shader, "projview");
	/* camera */
	camera_setup();
	/* framebuffer */
	gl_gen_framebuffers(1, &framebuffer);
	gl_gen_textures(1, &framebuffer_texture);
	gl_bind_framebuffer(GL_FRAMEBUFFER, framebuffer);
	gl_bind_texture(GL_TEXTURE_2D, framebuffer_texture);
	gl_tex_parameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	gl_tex_parameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	gl_tex_parameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	gl_tex_parameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	gl_tex_image_2d(GL_TEXTURE_2D, 0, GL_RGB, GAME_WIDTH_PIXELS, GAME_HEIGHT_PIXELS, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	gl_framebuffer_texture_2d(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer_texture, 0);
	gl_gen_vertex_arrays(1, &framebuffer_vertex_array);
	gl_bind_vertex_array(framebuffer_vertex_array);
	gl_gen_buffers(1, &framebuffer_vertex_buffer);
	gl_gen_buffers(1, &framebuffer_index_buffer);
	struct vertex framebuffer_vertices[] = { { { -1, -1 }, { 0, 0 } }, { {  1, -1 }, { 1, 0 } }, { {  1,  1 }, { 1, 1 } }, { { -1,  1 }, { 0, 1 } } };
	u32 framebuffer_indices[] = { 0, 1, 2, 2, 3, 0 };
	gl_bind_buffer(GL_ELEMENT_ARRAY_BUFFER, framebuffer_index_buffer);
	gl_buffer_data(GL_ELEMENT_ARRAY_BUFFER, sizeof (framebuffer_indices), framebuffer_indices, GL_STATIC_DRAW);
	gl_bind_buffer(GL_ARRAY_BUFFER, framebuffer_vertex_buffer);
	gl_buffer_data(GL_ARRAY_BUFFER, sizeof (framebuffer_vertices), framebuffer_vertices, GL_STATIC_DRAW);
	gl_vertex_attrib_pointer(0, 2, GL_FLOAT, GL_FALSE, sizeof (struct vertex), (void *)offsetof(struct vertex, position));
	gl_enable_vertex_attrib_array(0);
	gl_vertex_attrib_pointer(1, 2, GL_FLOAT, GL_FALSE, sizeof (struct vertex), (void *)offsetof(struct vertex, texcoord));
	gl_enable_vertex_attrib_array(1);
	/* blending */
	gl_enable(GL_BLEND);
	gl_blend_func(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	/* sprites */
	sprites_pool_amount = SPRITE_CAPACITY;
	for (u32 i = 0; i < SPRITE_CAPACITY; i++) sprites_pool[i] = i;
}

u32
renderer_sprite_alloc(struct v2f frame_index, struct v2f frame_size, u32 frame_amount, f32 fps) {
	if (sprites_pool_amount == 0) {
		fprintf(stderr, "error: not enough sprites\n");
		exit(1);
	}
	u32 sprite_index             = sprites_pool[--sprites_pool_amount];
	frame_size                   = V2F(frame_size.x  * one_frame_size.x, frame_size.y  * one_frame_size.y);
	frame_index                  = V2F(frame_index.x * one_frame_size.x, frame_index.y * one_frame_size.y);
	sprites[sprite_index]        = (struct sprite) { 0 };
	sprites[sprite_index].fps    = fps;
	sprites[sprite_index].base   = frame_index;
	sprites[sprite_index].size   = frame_size;
	sprites[sprite_index].amount = frame_amount;
	sprites[sprite_index].active = 1;
	return sprite_index;
}

void
renderer_sprite_frame_set(u32 sprite_index, u32 frame) {
	if (sprite_index >= SPRITE_CAPACITY || !sprites[sprite_index].active) {
		fprintf(stderr, "error: trying to change frame of invalid sprite\n");
		exit(1);
	}
	sprites[sprite_index].offset = frame % sprites[sprite_index].amount;
}

void
renderer_sprite_update(u32 sprite_index, f64 delta_time) {
	if (sprite_index >= SPRITE_CAPACITY || !sprites[sprite_index].active) {
		fprintf(stderr, "error: trying to update an invalid sprite\n");
		exit(1);
	}
	sprites[sprite_index].timer += delta_time;
	if (sprites[sprite_index].timer >= sprites[sprite_index].fps) {
		sprites[sprite_index].timer = 0;
		sprites[sprite_index].offset++;
		if (sprites[sprite_index].offset == sprites[sprite_index].amount) sprites[sprite_index].offset = 0;
	}
}

void
renderer_sprite_free(u32 sprite_index) {
	if (sprite_index >= SPRITE_CAPACITY || !sprites[sprite_index].active) {
		fprintf(stderr, "error: trying to free invalid sprite\n");
		exit(1);
	}
	sprites[sprite_index].active = 0;
	sprites_pool[sprites_pool_amount++] = sprite_index;
}

static void
renderer_internal(struct vertex *vertices, u32 vertices_amount, struct v2f position, struct v2f size, struct v2f frame_position, struct v2f frame_size) {
	if (!can_render) {
		fprintf(stderr, "error: rendering can only occur in 'game_draw' or 'game_draw_ui'\n");
		exit(1);
	}
	vertices[vertices_amount + 0].position = V2F(position.x,          position.y         );
	vertices[vertices_amount + 1].position = V2F(position.x + size.x, position.y         );
	vertices[vertices_amount + 2].position = V2F(position.x + size.x, position.y + size.y);
	vertices[vertices_amount + 3].position = V2F(position.x         , position.y + size.y);
	vertices[vertices_amount + 0].texcoord = V2F(frame_position.x,                frame_position.y + frame_size.y);
	vertices[vertices_amount + 1].texcoord = V2F(frame_position.x + frame_size.x, frame_position.y + frame_size.y);
	vertices[vertices_amount + 2].texcoord = V2F(frame_position.x + frame_size.x, frame_position.y               );
	vertices[vertices_amount + 3].texcoord = V2F(frame_position.x,                frame_position.y               );
}

void
renderer_sprite(u32 sprite_index, struct v2f position, struct v2f size, struct v2b flip) {
	struct vertex *vertices;
	u32 vertices_amount;
	if (is_ui) {
		vertices_amount = batch_ui_vertices_amount;
		vertices        = batch_ui_vertices;
	} else {
		vertices_amount = batch_vertices_amount;
		vertices        = batch_vertices;
	}
	if (vertices_amount >= VERTEX_CAPACITY) {
		fprintf(stderr, "error: not enough vertices\n");
		exit(1);
	}
	if (sprite_index >= SPRITE_CAPACITY || !sprites[sprite_index].active) {
		fprintf(stderr, "error: trying to render invalid sprite\n");
		exit(1);
	}
	renderer_internal(
		vertices,
		vertices_amount,
		position,
		size,
		V2F(sprites[sprite_index].base.x + (sprites[sprite_index].offset * one_frame_size.x), sprites[sprite_index].base.y),
		sprites[sprite_index].size
	);
	if (flip.x) {
		SWAP32(vertices[vertices_amount + 0].texcoord.x, vertices[vertices_amount + 1].texcoord.x);
		SWAP32(vertices[vertices_amount + 2].texcoord.x, vertices[vertices_amount + 3].texcoord.x);
	}
	if (flip.y) {
		SWAP32(vertices[vertices_amount + 0].texcoord.y, vertices[vertices_amount + 3].texcoord.y);
		SWAP32(vertices[vertices_amount + 1].texcoord.y, vertices[vertices_amount + 2].texcoord.y);
	}
	if (is_ui) {
		batch_ui_indices_amount  += 6;
		batch_ui_vertices_amount += 4;
	} else {
		batch_indices_amount  += 6;
		batch_vertices_amount += 4;
	}
}

void
renderer_text(struct v2f position, const i8 *format, ...) {
	struct vertex *vertices;
	u32 vertices_amount;
	u32 indices_amount;
	if (is_ui) {
		vertices_amount = batch_ui_vertices_amount;
		indices_amount  = batch_ui_indices_amount;
		vertices        = batch_ui_vertices;
	} else {
		vertices_amount = batch_vertices_amount;
		indices_amount  = batch_indices_amount;
		vertices        = batch_vertices;
	}
	i8 text[256];
	va_list args;
	va_start(args, format);
	vsnprintf(text, 256, format, args);
	va_end(args);
	f32 startx = position.x;
	for (u32 i = 0; i < 256; i++) {
		if (vertices_amount >= VERTEX_CAPACITY) {
			fprintf(stderr, "error: not enough vertices\n");
			exit(1);
		}
		if (text[i] == '\0') break;
		if (text[i] >= 'a' && text[i] <= 'z') {
			renderer_internal(
				vertices,
				vertices_amount,
				position,
				V2F(0.5f, 0.5f),
				V2F((text[i] - 'a') * half_frame_size.x, 10 * half_frame_size.y),
				half_frame_size
			);
			indices_amount  += 6;
			vertices_amount += 4;
			position.x += 0.5f;
		} else if (text[i] >= '+' && text[i] <= '.') {
			renderer_internal(
				vertices,
				vertices_amount,
				position,
				V2F(0.5f, 0.5f),
				V2F((26 + (text[i] - '+')) * half_frame_size.x, 10 * half_frame_size.y),
				half_frame_size
			);
			indices_amount  += 6;
			vertices_amount += 4;
			position.x += 0.5f;
		} else if (text[i] == '!') {
			renderer_internal(
				vertices,
				vertices_amount,
				position,
				V2F(0.5f, 0.5f),
				V2F(30 * half_frame_size.x, 10 * half_frame_size.y),
				half_frame_size
			);
			indices_amount  += 6;
			vertices_amount += 4;
			position.x += 0.5f;
		} else if (text[i] >= '0' && text[i] <= '9') {
			renderer_internal(
				vertices,
				vertices_amount,
				position,
				V2F(0.5f, 0.5f),
				V2F((text[i] - '0') * half_frame_size.x, 11 * half_frame_size.y),
				half_frame_size
			);
			indices_amount  += 6;
			vertices_amount += 4;
			position.x += 0.5f;
		} else if (text[i] == ' ') {
			position.x += 0.5f;
		} else if (text[i] == '\n') {
			position.x = startx;
			position.y -= 0.5f;
		} else{
			fprintf(stderr, "error: trying to render invalid character '%c'\n", text[i]);
			exit(1);
		}
	}
	if (is_ui) {
		batch_ui_vertices_amount  = vertices_amount;
		batch_ui_indices_amount   = indices_amount;
	} else {
		batch_vertices_amount  = vertices_amount;
		batch_indices_amount   = indices_amount;
	}
}

void
renderer_update(void) {
	/* game rendering */
	can_render = 1;
	game_draw();
	is_ui = 1;
	game_draw_ui();
	is_ui = 0;
	can_render = 0;
	/* setup for batch submit */
	gl_bind_framebuffer(GL_FRAMEBUFFER, framebuffer);
	gl_viewport(0, 0, GAME_WIDTH_PIXELS, GAME_HEIGHT_PIXELS);
	gl_clear_color(115.0f / 256.0f, 188.0f / 256.0f, 15.0f / 256.0f, 1.0f);
	gl_clear(GL_COLOR_BUFFER_BIT);
	gl_bind_vertex_array(batch_vertex_array);
	gl_bind_buffer(GL_ARRAY_BUFFER, batch_vertex_buffer);
	gl_bind_texture(GL_TEXTURE_2D, atlas);
	/* game batch */
	gl_uniform_matrix4fv(u_projview, 1, 1, (const f32 *)camera_projview(0).m);
	gl_buffer_sub_data(GL_ARRAY_BUFFER, 0, sizeof (batch_vertices), batch_vertices);
	gl_draw_elements(GL_TRIANGLES, batch_indices_amount, GL_UNSIGNED_INT, NULL);
	batch_indices_amount = batch_vertices_amount = 0;
	/* ui batch */
	gl_uniform_matrix4fv(u_projview, 1, 1, (const f32 *)camera_projview(1).m);
	gl_buffer_sub_data(GL_ARRAY_BUFFER, 0, sizeof (batch_ui_vertices), batch_ui_vertices);
	gl_draw_elements(GL_TRIANGLES, batch_ui_indices_amount, GL_UNSIGNED_INT, NULL);
	batch_ui_indices_amount = batch_ui_vertices_amount = 0;
	/* render framebuffer */
	gl_bind_framebuffer(GL_FRAMEBUFFER, 0);
	gl_viewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	gl_uniform_matrix4fv(u_projview, 1, 1, (const f32 *)M4F_IDENTITY.m);
	gl_bind_vertex_array(framebuffer_vertex_array);
	gl_bind_buffer(GL_ARRAY_BUFFER, framebuffer_vertex_buffer);
	gl_bind_texture(GL_TEXTURE_2D, framebuffer_texture);
	gl_draw_elements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
}

void
renderer_end(void) {
	gl_delete_program(basic_shader);
	gl_delete_vertex_arrays(1, &batch_vertex_array);
	gl_delete_buffers(1, &batch_vertex_buffer);
	gl_delete_buffers(1, &batch_index_buffer);
	gl_delete_textures(1, &atlas);
	gl_delete_textures(1, &framebuffer_texture);
	gl_delete_framebuffers(1, &framebuffer);
	gl_delete_vertex_arrays(1, &framebuffer_vertex_array);
	gl_delete_buffers(1, &framebuffer_vertex_buffer);
	gl_delete_buffers(1, &framebuffer_index_buffer);
}
