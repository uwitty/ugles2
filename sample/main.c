#include "../src/ugles2.h"

#if defined(MESA_X)
#include "platform/mesa_x.h"
#elif defined(RASPBERRYPI)
#include "platform/raspberrypi.h"
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <sys/time.h>

struct app_data {
	struct shader {
		GLuint program;
		struct locations {
			GLint  a_position;
			GLint  a_texture;
			GLint  u_model;
			GLint  u_texture;
			GLint  u_vp;
		} locations;
	} shader;

	struct object {
		GLuint vbuffer;
		GLuint ibuffer;
		GLuint texture;
	} triangle, text;
};

void init_shader(struct ugles2_context* context, struct app_data* app_data)
{
	// shader srcs
	const char vshader_src[] =
		"attribute vec3 a_position;\n"
		"attribute vec2 a_texture;\n"
		"varying   vec2 v_texture;\n"
		"uniform   mat4 u_vp_matrix;\n"
		"uniform   mat4 u_model;\n"
		"\n"
		"void main(void) {\n"
		"  v_texture = a_texture;\n"
		"  gl_Position = u_vp_matrix * u_model * vec4(a_position, 1.0);\n"
		"}\n";

	const char fshader_src[] =
		"precision mediump float;\n"
		"varying   vec2  v_texture;\n"
		"uniform sampler2D u_texture;\n"
		"void main()        \n"
		"{                  \n"
		"  gl_FragColor = texture2D(u_texture,vec2(v_texture.s,v_texture.t));\n"
		"}                  \n";

	GLuint program = ugles2_compile_program(vshader_src, fshader_src);
	glUseProgram(program);

	app_data->shader.program = program;
	app_data->shader.locations.a_position = glGetAttribLocation(program, "a_position");
	app_data->shader.locations.a_texture  = glGetAttribLocation(program, "a_texture");
	app_data->shader.locations.u_model    = glGetUniformLocation(program, "u_model");
	app_data->shader.locations.u_texture  = glGetUniformLocation(program, "u_texture");
	app_data->shader.locations.u_vp       = glGetUniformLocation(program, "u_vp_matrix");

	glEnableVertexAttribArray(app_data->shader.locations.a_position);
	glEnableVertexAttribArray(app_data->shader.locations.a_texture);
}

void init_projection(struct ugles2_context* context, struct app_data* app_data)
{
	// projection matrix
	float projection[16];
	ugles2_matrix_perspective(projection, 52.5f, context->height / (float)context->width, 1.0f, 1000.0f);
	float pos[16];
	//ugles2_matrix_position(pos, 0.0f, 0.0f, -3.0f);
	ugles2_matrix_position(pos, 0.0f, 0.0f, -0.5f / tan((26.25f)/360.0f * 2 * M_PI));
	ugles2_matrix_multi(projection, projection, pos);
	glUniformMatrix4fv(app_data->shader.locations.u_vp, 1, GL_FALSE, projection);
}

void init_triangle(struct ugles2_context* context, struct app_data* app_data, const char texture_filename[])
{
#if 1
	float vertices[] = {
		// x   y                                                  z     s     t
		-0.5f, -0.5f * (context->height / (float)context->width), 0.0f, 0.0f, 0.0f,
		 0.5f, -0.5f * (context->height / (float)context->width), 0.0f, 1.0f, 0.0f,
		 0.0f,  0.5f * (context->height / (float)context->width), 0.0f, 0.5f, 1.0f,
	};
#else
	float vertices[] = {
		// x   y                                                  z     s     t
		-0.5f, -0.5f * (context->height / (float)context->width), 0.0f, 0.0f, 0.0f,
		 0.5f, -0.5f * (context->height / (float)context->width), 0.0f, 1.0f, 0.0f,
		 0.0f,  0.5f * (context->height / (float)context->width), 0.0f, 0.5f, 1.0f,
	};
#endif
	unsigned short indices[] = { 0, 1, 2 };

	// buffer
	GLuint vbuffer = ugles2_gen_buffer(GL_ARRAY_BUFFER, vertices, sizeof(vertices), GL_STATIC_DRAW);
	GLuint ibuffer = ugles2_gen_buffer(GL_ELEMENT_ARRAY_BUFFER, indices, sizeof(indices), GL_STATIC_DRAW);

	// load texture image
	GLuint texture;
	int texture_width, texture_height;
	if (ugles2_load_size(&texture_width, &texture_height, texture_filename) == 0) {
		GLubyte* pixels = (GLubyte*)malloc(texture_width*texture_height*4);
		ugles2_load_pixels(pixels, texture_width, texture_height, texture_filename);
		texture =  ugles2_create_texture(pixels, texture_width, texture_height);
		free(pixels);
	} else {
		texture_width  = 1;
		texture_height = 4;
		GLubyte pixels[] = {
			 50,   0,   0, 255,
			100,   0,   0, 255,
			150,   0,   0, 255,
			200,   0,   0, 255,
		};
		texture = ugles2_create_texture(pixels, texture_width, texture_height);
	}

	// update app_data
	app_data->triangle.vbuffer = vbuffer;
	app_data->triangle.ibuffer = ibuffer;
	app_data->triangle.texture = texture;
}

void init_text(struct ugles2_context* context, struct app_data* app_data, const char text[])
{
	float vertices[] = {
		// x   y                                                  z     s     t
		-0.5f, -0.5f * (context->height / (float)context->width), 0.0f, 0.0f, 0.0f,
		 0.5f, -0.5f * (context->height / (float)context->width), 0.0f, 1.0f, 0.0f,
		 0.5f,  0.5f * (context->height / (float)context->width), 0.0f, 1.0f, 1.0f,
		-0.5f,  0.5f * (context->height / (float)context->width), 0.0f, 0.0f, 1.0f,
	};
	unsigned short indices[] = { 0, 1, 3, 2 };

	// buffer
	GLuint vbuffer = ugles2_gen_buffer(GL_ARRAY_BUFFER, vertices, sizeof(vertices), GL_STATIC_DRAW);
	GLuint ibuffer = ugles2_gen_buffer(GL_ELEMENT_ARRAY_BUFFER, indices, sizeof(indices), GL_STATIC_DRAW);

	// load texture image
	int texture_width  = 512;
	int texture_height =  64;
	GLubyte* pixels = (GLubyte*)malloc(texture_width*texture_height*4);
	int i, j;
	for (j = 0; j < texture_height; j++) {
		for (i = 0; i < texture_width; i++) {
			pixels[j*texture_width*4 + i*4  ] = 0x10;
			pixels[j*texture_width*4 + i*4+1] = 0x10;
			pixels[j*texture_width*4 + i*4+2] = 0x10;
			pixels[j*texture_width*4 + i*4+3] = 0x80;
		}
	}
	int font_height = 18;
	GLubyte red   = 220;
	GLubyte green = 220;
	GLubyte blue  = 250;
	GLubyte alpha = 255;
	int x = 24;
	int y = (texture_height - font_height) / 2;
	ugles2_draw_text(context, pixels, texture_width, texture_height, text, font_height, red, green, blue, alpha, x, y);
	GLuint texture = ugles2_create_texture(pixels, texture_width, texture_height);
	free(pixels);

	// update app_data
	app_data->text.vbuffer = vbuffer;
	app_data->text.ibuffer = ibuffer;
	app_data->text.texture = texture;
}

void init(struct ugles2_context* context, struct app_data* app_data, const char texture_filename[])
{
	const char ttf_filename[] = "test.ttf";
	if (ugles2_set_font(context, ttf_filename) != 0) {
		printf("ugles2_set_font(context, \"%s\") failed. @%s:%d\n", ttf_filename, __FILE__, __LINE__);
	}

	// gl setting
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.3f, 0.3f, 0.5f, 1.0f);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	init_shader(context, app_data);
	init_projection(context, app_data);
	init_triangle(context, app_data, texture_filename);
	init_text(context, app_data, texture_filename);
}

void draw_triangle(struct ugles2_context* context, struct app_data* app_data, int frames)
{
	glBindBuffer(GL_ARRAY_BUFFER, app_data->triangle.vbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app_data->triangle.ibuffer);
	glVertexAttribPointer(app_data->shader.locations.a_position, 3, GL_FLOAT, GL_FALSE, 20, (void*)0);
	glVertexAttribPointer(app_data->shader.locations.a_texture , 2, GL_FLOAT, GL_FALSE, 20, (void*)12);

	// texture
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(app_data->shader.locations.u_texture, 0);
	glBindTexture(GL_TEXTURE_2D, app_data->triangle.texture);

	// model matrix
	float t[16];
	ugles2_matrix_rotate_y(t, 1.0f * frames);
	t[14] = -1.0f;
	glUniformMatrix4fv(app_data->shader.locations.u_model, 1, GL_FALSE, t);

	glDrawElements(GL_TRIANGLE_STRIP, 3, GL_UNSIGNED_SHORT, 0);
}

void draw_text(struct ugles2_context* context, struct app_data* app_data, int frames)
{
	glBindBuffer(GL_ARRAY_BUFFER, app_data->text.vbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app_data->text.ibuffer);
	glVertexAttribPointer(app_data->shader.locations.a_position, 3, GL_FLOAT, GL_FALSE, 20, (void*)0);
	glVertexAttribPointer(app_data->shader.locations.a_texture , 2, GL_FLOAT, GL_FALSE, 20, (void*)12);

	// texture
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(app_data->shader.locations.u_texture, 0);
	glBindTexture(GL_TEXTURE_2D, app_data->text.texture);

	// model matrix
	float t[16];
	ugles2_matrix_unit(t);
	int x = 8;
	int y = 8;
	t[0]  = 512.0f / (float)context->width;
	t[5]  =  64.0f / (float)context->height;
	t[12] = (1.0 / (float)context->width * x - 0.5) + (1.0 / context->width) * 512 / 2;
	t[13] = ((1.0 / (float)context->height * (-y) + 0.5f) - (1.0f / context->height) * 64 / 2) * context->height/context->width;
	t[14] = 0.0001f;
	glUniformMatrix4fv(app_data->shader.locations.u_model, 1, GL_FALSE, t);

	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, 0);
}

void draw(struct ugles2_context* context, struct app_data* app_data, int frames)
{
	glViewport(0, 0, context->width, context->height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	draw_triangle(context, app_data, frames);
	draw_text(context, app_data, frames);

	eglSwapBuffers(context->display, context->surface);
}

void finalize(struct ugles2_context* context, struct app_data* app_data)
{
	glDeleteTextures(1, &app_data->text.texture);
	glDeleteBuffers(1, &app_data->text.ibuffer);
	glDeleteBuffers(1, &app_data->text.vbuffer);

	glDeleteTextures(1, &app_data->triangle.texture);
	glDeleteBuffers(1, &app_data->triangle.ibuffer);
	glDeleteBuffers(1, &app_data->triangle.vbuffer);

	glDeleteProgram(app_data->shader.program);
}

int main(int argc, char *argv[])
{
	fprintf(stderr, "%s started. \n", argv[0]);

	struct ugles2_context context;

#if defined(MESA_X)
	ugles2_open_platform open_platform = ugles2_platform_mesa_x_open;
	GLint* config_attr = NULL;
#elif defined(RASPBERRYPI)
	ugles2_open_platform open_platform = ugles2_platform_raspberrypi_open;
	EGLint config_attr[] =
	{
		EGL_RED_SIZE, 8, EGL_GREEN_SIZE,8, EGL_BLUE_SIZE, 8,
		EGL_ALPHA_SIZE, 8,
		EGL_DEPTH_SIZE, 24,
		EGL_NONE
	};
#else
#error unknown platform
#endif

	if (ugles2_initialize(&context, open_platform, config_attr) != 0) {
		printf("ugles2_initialize() failed. \n");
		return 0;
	}

	struct app_data app_data;
	memset(&app_data, 0, sizeof(app_data));

	if (argc > 1) {
		init(&context, &app_data, argv[1]);
	} else {
		init(&context, &app_data, NULL);
	}

	struct timeval st;
	gettimeofday(&st, NULL);

	int frames = 360;
	int i;
	for (i = 0; i < frames; i++) {
		draw(&context, &app_data, i);
	}

	struct timeval et;
	gettimeofday(&et, NULL);

	finalize(&context, &app_data);

	ugles2_finalize(&context);

	int elapsed = (et.tv_sec - st.tv_sec) * 1000 + (et.tv_usec - st.tv_usec) / 1000;
	printf("elapsed: %d.%03d [sec]\n", elapsed / 1000, elapsed % 1000);
	printf("fps: %f \n", frames  /(float)elapsed * 1000.0f);

	return 0;
}

