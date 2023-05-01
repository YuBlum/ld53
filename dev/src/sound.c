#include <al.h>
#include <alc.h>
#include <sound.h>
#include <types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <os_specific.h>

#define SOUND_CAPACITY 6

#pragma pack(1)
struct wav_header {
	u32 chunk_id;
	u32 chunk_size;
	u32 format;
	u32 fmt_id;
	u32 fmt_size;
	u16 fmt_audio_format;
	u16 fmt_num_channels;
	u32 fmt_sample_rate;
	u32 fmt_byte_rate;
	u16 fmt_block_align;
	u16 fmt_bits_per_sample;
	u32 data_id;
	u32 data_size;
};
#pragma pack()

static void *(*alc_open_device)(const i8 *);
static void *(*alc_create_context)(void *, i32 *);
static b8    (*alc_close_device)(void *);
static void  (*alc_destroy_context)(void *);
static b8    (*alc_make_context_current)(void *);
static u32   (*alc_get_error)(void *);
static i8   *(*alc_get_string)(void *, u32);
static void  (*al_gen_buffers)(i32, u32 *);
static void  (*al_delete_buffers)(i32, u32 *);
static void  (*al_gen_sources)(i32, u32 *);
static void  (*al_delete_sources)(i32, u32 *);
static void  (*al_buffer_data)(u32, u32, const void *, i32, i32);
static void  (*al_sourcei)(u32, u32, i32);
static void  (*al_sourcef)(u32, u32, f32);
static void  (*al_source_play)(u32);
static void  (*al_source_stop)(u32);

static void *device;
static void *context;

void
sound_master_begin(void) {
	printf("here 1 - 0\n");
#ifdef LINUX
	void *openal = lib_open("libopenal.so");
#else
	void *openal = lib_open("OpenAL32.dll");
#endif
	printf("here 1 - 1\n");
	alc_open_device          = lib_function(openal, "alcOpenDevice");
	alc_close_device         = lib_function(openal, "alcCloseDevice");
	alc_destroy_context      = lib_function(openal, "alcDestroyContext");
	alc_create_context       = lib_function(openal, "alcCreateContext");
	alc_make_context_current = lib_function(openal, "alcMakeContextCurrent");
	alc_get_error            = lib_function(openal, "alcGetError");
	alc_get_string           = lib_function(openal, "alcGetString");
	al_gen_buffers           = lib_function(openal, "alGenBuffers");
	al_delete_buffers        = lib_function(openal, "alDeleteBuffers");
	al_gen_sources           = lib_function(openal, "alGenSources");
	al_delete_sources        = lib_function(openal, "alDeleteSources");
	al_buffer_data           = lib_function(openal, "alBufferData");
	al_sourcei               = lib_function(openal, "alSourcei");
	al_sourcef               = lib_function(openal, "alSourcef");
	al_source_play           = lib_function(openal, "alSourcePlay");
	al_source_stop           = lib_function(openal, "alSourceStop");
	printf("here 1 - 2\n");
	const i8 *device_name = alc_get_string(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
	device  = alc_open_device(device_name);
	printf("here 1 - 3   %s\n", device_name);
	if (!device) {
		fprintf(stderr, "error: could not load sound device\n");
		exit(1);
	}
	printf("here 1 - 4\n");
	context = alc_create_context(device, NULL);
	printf("here 1 - 5\n");
	alc_make_context_current(context);
	printf("here 1 - 6\n");
}

u32
sound_alloc(const i8 *name) {
	printf("here 1 - 0\n");
	i8 sound_name[256];
	sprintf(sound_name, "data/%s.wav", name);
	i8 *path = resource_path(sound_name);
	FILE *file = fopen(path, "rb");
	if (!file) {
		fprintf(stderr, "error: could not open %s: %s\n", path, strerror(errno));
		exit(1);
	}
	struct wav_header header;
	fread(&header, 1, sizeof (header), file);
	u8 *data = malloc(header.data_size);
	printf("here 1 - 1\n");
	fread(data, 1, header.data_size, file);
	fclose(file);
	u32 sound;
	printf("here 1 - 2\n");
	al_gen_buffers(1, &sound);
	printf("here 1 - 3\n");
	al_buffer_data(sound, AL_FORMAT_STEREO16, data, header.data_size, header.fmt_sample_rate);
	printf("here 1 - 4\n");
	free(data);
	printf("here 1 - 5\n");
	return sound;
}

void
sound_free(u32 sound) {
	al_delete_buffers(1, &sound);
}

u32
sound_source_alloc(u32 sound, b8 loop) {
	u32 source;
	al_gen_sources(1, &source);
	al_sourcei(source, AL_BUFFER,  sound);
	al_sourcei(source, AL_LOOPING, loop);
	return source;
}

void
sound_source_gain(u32 source, f32 gain) {
	al_sourcef(source, AL_GAIN, gain);
}

void
sound_source_play(u32 source) {
	al_source_play(source);
}

void
sound_source_stop(u32 source) {
	al_source_stop(source);
}

void
sound_source_free(u32 source) {
	al_delete_sources(1, &source);
}

void
sound_master_end(void) {
	alc_close_device(device);
	alc_destroy_context(context);
}
