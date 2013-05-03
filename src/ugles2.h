#ifndef _UGLES2_H_
#define _UGLES2_H_

#include <GLES2/gl2.h>
#include <EGL/egl.h>

#if defined(USE_PNG)
#include <png.h>
#endif

#if defined(USE_JPEG)
#include "jpeglib.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct ugles2_platform {
	NativeWindowType window;
	void *data;
	int width;
	int height;

	 void (*close)(struct ugles2_platform* platform);
};

struct ugles2_context {
	EGLDisplay  display;
	EGLContext  context;
	EGLSurface  surface;
	EGLint      major_version;
	EGLint      minor_version;

	struct ugles2_platform* platform;
	void* user_data;

	int width;
	int height;

	void* freetype;
};

typedef int (*ugles2_open_platform)(struct ugles2_platform* platform, void* arg);

// initialize / finailize
int  ugles2_initialize(struct ugles2_context* context, ugles2_open_platform open_platform, const GLint config_attr[]);
void ugles2_finalize(struct ugles2_context* context);

// shaders
GLuint ugles2_compile_vertex_shader(const char src[]);
GLuint ugles2_compile_fragment_shader(const char src[]);
int    ugles2_link_shaders(GLuint program, GLuint vshader, GLuint fshader);
GLuint ugles2_compile_program(const char vshader_src[], const char fshader_src[]);

// buffer
GLuint ugles2_gen_buffer(GLenum target, void* p, unsigned size, GLenum usage);
GLuint ugles2_gen_vertex_buffer(GLenum target, void* p, unsigned size, GLenum usage);

// texture
int ugles2_load_size(int* width, int* height, const char file[]);
int ugles2_load_pixels(GLubyte* pixels, int width, int height, const char file[]);
GLuint ugles2_load_texture(const char file[]);
GLuint ugles2_create_texture(const GLubyte* pixels, int width, int height);

// text
int ugles2_set_font(struct ugles2_context* context, const char file[]);
int ugles2_text_size(struct ugles2_context* context, int* width, int* count, const char text[], int font_size);
int ugles2_draw_text(struct ugles2_context* context, GLubyte* pixels, int width, int height
					, const char text[], int font_size, GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha
					, int x, int y);

// matrix
void ugles2_matrix_unit(float m[]);
void ugles2_matrix_add(float result[], float a[], float b[]);
void ugles2_matrix_sub(float result[], float a[], float b[]);
void ugles2_matrix_multi(float result[], float a[], float b[]);
void ugles2_matrix_rotate_x(float m[], float degree);
void ugles2_matrix_rotate_y(float m[], float degree);
void ugles2_matrix_rotate_z(float m[], float degree);
void ugles2_matrix_position(float m[], float x, float y, float z);
void ugles2_matrix_frustrum(float m[], float l, float r, float b, float t, float n, float f);
void ugles2_matrix_perspective(float m[], float fovy, float aspect, float near, float far);

#ifdef __cplusplus
}	// end of extern "C" {
#endif

#endif

