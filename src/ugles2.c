#include "ugles2.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#if defined(USE_PNG)
#include <png.h>
#endif

#if defined(USE_JPEG)
#include "jpeglib.h"
#endif

#if defined(USE_FREETYPE)
#include "ft2build.h"
#include FT_FREETYPE_H
#include FT_GLYPH_H
#endif

static int init_context(struct ugles2_context* context, struct ugles2_platform* platform, const GLint config_attr[]);
static void close_platform(struct ugles2_platform* platform);

int ugles2_initialize(struct ugles2_context* context, ugles2_open_platform open_platform, const GLint config_attr[])
{
	memset(context, 0, sizeof(struct ugles2_context));

#if defined(USE_FREETYPE)
	if (FT_Init_FreeType(&context->library) != 0) {
		return -1;
	}
#endif

	// initialize platform (native window)
	struct ugles2_platform* platform = (struct ugles2_platform*)malloc(sizeof(struct ugles2_platform));
	if (platform == NULL) {
		return -1;
	}
	memset(platform, 0, sizeof(struct ugles2_platform));

	if (open_platform(platform, NULL) != 0) {
		free(platform);
		return -1;
	}

	if (init_context(context, platform, config_attr) != 0) {
		close_platform(platform);
		return -1;
	}

	return 0;
}

void ugles2_finalize(struct ugles2_context* context)
{
	if (context->context != EGL_NO_CONTEXT) {
		eglDestroyContext(context->display, context->context);
		context->context = EGL_NO_CONTEXT;
	}

	if (context->surface != EGL_NO_SURFACE) {
		eglDestroySurface(context->display, context->surface);
		context->surface = EGL_NO_SURFACE;
	}

	if (context->display != EGL_NO_DISPLAY) {
		eglTerminate(context->display);
		context->display = EGL_NO_DISPLAY;
	}

	close_platform(context->platform);
	free(context->platform);
	context->platform = NULL;

#if defined(USE_FREETYPE)
	FT_Done_FreeType(context->library);
#endif
}

int init_context(struct ugles2_context* context, struct ugles2_platform* platform, const EGLint config_attr[])
{
	EGLDisplay display = NULL;
	EGLConfig  config  = NULL;
	EGLSurface surface = NULL;
	EGLContext econtext = NULL;

	display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if (display == EGL_NO_DISPLAY) {
		printf("failed to get display\n");
		return -1;
	}

	if (!eglInitialize(display, &context->major_version, &context->minor_version)) {
		printf("eglInitialize() failed. \n");
		return -1;
	}

	EGLint default_config_attr[] =
	{
		EGL_RED_SIZE, 8, EGL_GREEN_SIZE,8, EGL_BLUE_SIZE, 8,
		EGL_ALPHA_SIZE, EGL_DONT_CARE,
		EGL_DEPTH_SIZE, 24,
		EGL_NONE
	};

	EGLint num_configs;
	if (!eglChooseConfig(display, (config_attr != NULL)? config_attr : default_config_attr, &config, 1, &num_configs)) {
		printf("eglChooseConfig() failed. \n");
		return -1;
	}

	surface = eglCreateWindowSurface(display, config, platform->window, NULL);
	if (surface == EGL_NO_SURFACE) {
		printf("eglCreateSurface() failed. \n");
		return -1;
	}

	EGLint context_attr[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
	econtext = eglCreateContext(display, config, EGL_NO_CONTEXT, context_attr);
	if (econtext == EGL_NO_CONTEXT) {
		printf("eglCreateSurface() failed. \n");
		return -1;
	}

	if (!eglMakeCurrent(display, surface, surface, econtext)) {
		printf("eglMakeCurrent() failed. \n");
		return -1;
	}

	context->display  = display;
	context->platform = platform;
	context->surface  = surface;
	context->context  = econtext;

	context->width    = platform->width;
	context->height   = platform->height;

	return 0;
}

static void close_platform(struct ugles2_platform* platform)
{
	if (platform->close != NULL) {
		platform->close(platform);
	}
}

static GLuint compile_shader(GLenum type, const char src[])
{
	GLuint shader;
	GLint  compiled;

	shader = glCreateShader(type);
	if (shader == 0) {
		return 0;
	}

	glShaderSource(shader, 1, &src, NULL);
	glCompileShader(shader);

	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled) {
		glDeleteShader(shader);
		return 0;
	}

	return shader;
}

GLuint ugles2_compile_vertex_shader(const char src[])
{
	return compile_shader(GL_VERTEX_SHADER, src);
}

GLuint ugles2_compile_fragment_shader(const char src[])
{
	return compile_shader(GL_FRAGMENT_SHADER, src);
}

int ugles2_link_shaders(GLuint program, GLuint vshader, GLuint fshader)
{
	GLint linked;

	glAttachShader(program, vshader);
	glAttachShader(program, fshader);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if (!linked) {
		return -1;
	}

	return 0;
}

GLuint ugles2_compile_program(const char vshader_src[], const char fshader_src[])
{
	GLuint program = glCreateProgram();

	GLuint vshader = ugles2_compile_vertex_shader(vshader_src);
	GLuint fshader = ugles2_compile_fragment_shader(fshader_src);

	if (ugles2_link_shaders(program, vshader, fshader) != 0) {
		glDeleteProgram(program);
		return 0;
	}

	glDeleteShader(fshader);
	glDeleteShader(vshader);

	return program;
}

GLuint ugles2_gen_buffer(GLenum target, void* p, unsigned size, GLenum usage)
{
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(target, buffer);
	glBufferData(target, size, p, usage);

	return buffer;
}

// =============================================================================
// texture

typedef int (*load_func)(const char file[], int* width, int* height, GLubyte* pixels);

static void copy_line_bgr(GLubyte* dst, uint8_t* src, uint8_t alpha, int w)
{
	int i;
	for (i = 0; i < w; i++) {
		dst[i*4  ] = src[i*3+2];	// R
		dst[i*4+1] = src[i*3+1];	// G
		dst[i*4+2] = src[i*3  ];	// B
		dst[i*4+3] = alpha;			// A
	}
}

static void copy_line_rgb(GLubyte* dst, uint8_t* src, uint8_t alpha, int w)
{
	int i;
	for (i = 0; i < w; i++) {
		dst[i*4  ] = src[i*3  ];	// R
		dst[i*4+1] = src[i*3+1];	// G
		dst[i*4+2] = src[i*3+2];	// B
		dst[i*4+3] = alpha;			// A
	}
}

#if defined(USE_PNG)
static int load_png(const char file[], int* width, int* height, GLubyte* pixels)
{
	FILE* fp = fopen(file, "rb");
	if (fp == NULL) {
		return -1;
	}

	char sig[8];
	if ((fread(sig, 1, 8, fp) != 8) || png_sig_cmp(sig, 0, 8) != 0) {
		fclose(fp);
		return -1;
	}
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if ((png_ptr == NULL) || (info_ptr == NULL)) {
		fclose(fp);
		return -1;
	}

	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, 8);

	png_read_info(png_ptr, info_ptr);

	png_uint_32 w, h;
	int bpp, color_type, interlace_method, compression_method, filter_method;
	png_get_IHDR(png_ptr, info_ptr, &w, &h, &bpp, &color_type, &interlace_method, &compression_method, &filter_method);
	//printf("w:%d h:%d bpp:%d color:%d interlace:%d compression:%d filter:%d\n"
	//		, w, h, bpp, color_type, interlace_method, compression_method, filter_method);

	if (width != NULL) {
		*width = w;
	}
	if (height != NULL) {
		*height = h;
	}

	if (pixels != NULL) {
		png_set_strip_16(png_ptr);
		png_set_expand(png_ptr);
		if (color_type != PNG_COLOR_TYPE_RGB_ALPHA) {
			png_set_filler(png_ptr, 255, PNG_FILLER_AFTER);
		}

		png_bytepp image = (png_bytepp)png_malloc(png_ptr, sizeof(png_bytep)*h);
		int j;
		for (j = 0; j < h; j++) {
			image[j] = (png_bytep)&pixels[(h - j - 1)*w*4];
		}
		png_set_rows(png_ptr, info_ptr, image);
		png_read_image(png_ptr, image);
		png_free(png_ptr, image);
	}

	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

	fclose(fp);

	return 0;
}
#endif

#if defined(USE_JPEG)
static int load_jpeg(const char file[], int* width, int* height, GLubyte* pixels)
{
	FILE* fp = fopen(file, "rb");
	if (fp == NULL) {
		return -1;
	}
	struct jpeg_decompress_struct dec;
	memset(&dec, 0, sizeof(dec));
	jpeg_create_decompress(&dec);

	struct jpeg_error_mgr error_mgr;
	memset(&error_mgr, 0, sizeof(error_mgr));
	dec.err = jpeg_std_error(&error_mgr);

	jpeg_stdio_src(&dec, fp);

	jpeg_read_header(&dec, TRUE);

	jpeg_start_decompress(&dec);

	if (width != NULL) {
		*width = dec.output_width;
	}
	if (height != NULL) {
		*height = dec.output_height;
	}
	int pitch = dec.output_width * dec.output_components;

	if (pixels != NULL) {
		JSAMPARRAY buffer = (*dec.mem->alloc_sarray)((j_common_ptr)&dec, JPOOL_IMAGE, pitch, 1);
		int j;
		int w = dec.output_width;
		int h = dec.output_height;
		for (j = 0; j < h; j++) {
			jpeg_read_scanlines(&dec, buffer, 1);
			copy_line_rgb(&pixels[((h - j - 1)*w)*4], buffer[0], 255, w);
		}
		jpeg_finish_decompress(&dec);
	}

	jpeg_destroy_decompress(&dec);

	fclose(fp);

	return 0;
}
#endif

static int load_bmp(const char file[], int* width, int* height, GLubyte* pixels)
{
	int w, h, pitch, bpp;
	FILE* fp = fopen(file, "rb");
	if (fp == NULL) {
		return -1;
	}
	
	unsigned char header[54];
	size_t size = fread(header, 54, 1, fp);

	bpp = header[0x1c+1] << 8 | header[0x1c];
	if ((size != 1U) || bpp != 24) {
		fclose(fp);
		return -1;
	}
	w  = header[0x12+3] << 24 | header[0x12+2] << 16 | header[0x12+1] << 8 | header[0x12];
	h  = header[0x16+3] << 24 | header[0x16+2] << 16 | header[0x16+1] << 8 | header[0x16];
	pitch  = (int)(((w * 3) + 3) & ~3U);

	int offset = header[0x0a+3] << 24 | header[0x0a+2] << 16 | header[0x0a+1] << 8 | header[0x0a];

	if (width != NULL) {
		*width = w;
	}
	if (height != NULL) {
		*height = h;
	}

	if (pixels != NULL) {
		if (offset != 0) {
			fseek(fp, offset, SEEK_SET);
		}
		uint8_t* bitmap = (uint8_t*)malloc(pitch);
		int x, y;
		for (y = 0; y < h; y++) {
			if (fread(bitmap, pitch, 1, fp) < 1) {
				memset(bitmap, 0x80, pitch);
			}
			copy_line_bgr(&pixels[(y*w)*4], bitmap, 255, w);
		}
		free(bitmap);
	}

	fclose(fp);

	return 0;
}

load_func get_load_func(const char ext[])
{
	if (ext == NULL) {
		return NULL;
	}

	if (strcmp(ext, "bmp") == 0) {
		return load_bmp;
	} else if (strcmp(ext, "png") == 0) {
#if defined(USE_PNG)
		return load_png;
#else
		return NULL;
#endif
	} else if ((strcmp(ext, "jpg") == 0) || (strcmp(ext, "jpeg") == 0)) {
#if defined(USE_JPEG)
		return load_jpeg;
#else
		return NULL;
#endif
	} else {
		return NULL;
	}
}

const char* filename_ext(const char filename[])
{
	int len = strlen(filename);
	int i;
	for (i = 0; i < len; i++) {
		if (filename[len - i - 1] == '.') {
			return &filename[len - i];
		}
	}
	return NULL;
}

GLuint ugles2_load_texture(const char file[])
{
	int width;
	int height;

	if (strlen(file) < 4) {
		return 0;
	}

	load_func load_image = get_load_func(filename_ext(file));
	if (load_image == NULL) {
		return 0;
	}

	if (load_image(file, &width, &height, NULL) != 0) {
		return 0;
	}

	GLubyte* pixels = (GLubyte*)malloc(width*height*4);
	if (pixels == NULL) {
		return 0;
	}

	if (load_image(file, &width, &height, pixels) != 0) {
		free(pixels);
		return 0;
	}

	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	GLenum format = GL_RGBA;
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, pixels);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	free(pixels);

	return texture;
}

int ugles2_load_size(int* width, int* height, const char file[])
{
	if ((file == NULL) || (strlen(file) < 4)) {
		return -1;
	}

	load_func load_image = get_load_func(filename_ext(file));
	if (load_image == NULL) {
		return -1;
	}

	if (load_image(file, width, height, NULL) != 0) {
		return -1;
	}

	return 0;
}

int ugles2_load_pixels(GLubyte* pixels, int width, int height, const char file[])
{
	if (strlen(file) < 4) {
		return 0;
	}

	load_func load_image = get_load_func(filename_ext(file));
	if (load_image == NULL) {
		return 0;
	}

	int org_width  = width;
	int org_height = height;
	if (load_image(file, &width, &height, pixels) != 0) {
		return -1;
	}

	return 0;
}

GLuint ugles2_create_texture(const GLubyte pixels[], int width, int height)
{
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	GLenum format = GL_RGBA;
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, pixels);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	return texture;
}


// =============================================================================
// text

#if defined(USE_FREETYPE)
static void blend_glyph_bitmap(
	  GLubyte pixels[], int x, int y, int width, int height
	, GLubyte red, GLubyte green, GLubyte blue
	, unsigned char bitmap_buffer[], int bitmap_width, int bitmap_pitch, int bitmap_height)
{
	int i, j;
	for (j = 0; j < bitmap_height; j++) {
		for (i = 0; i < bitmap_width; i++) {
			if ((x+i) < 0 || width <= (x+i) || (y+j) < 0 || height <= (y+j)) {
				continue;
			}
			GLubyte dst_r = pixels[(height-(y+j)-1)*width*4 + (x+i)*4  ];
			GLubyte dst_g = pixels[(height-(y+j)-1)*width*4 + (x+i)*4+1];
			GLubyte dst_b = pixels[(height-(y+j)-1)*width*4 + (x+i)*4+2];
			GLubyte dst_a = pixels[(height-(y+j)-1)*width*4 + (x+i)*4+3];

			GLubyte src_a = bitmap_buffer[j*bitmap_pitch + i];
			GLubyte out_a = (src_a * 255 + dst_a * (255 - src_a))/255;

			if (out_a == 0) {
				pixels[(height-(y+j)-1)*width*4 + (x+i)*4  ] = 0;
				pixels[(height-(y+j)-1)*width*4 + (x+i)*4+1] = 0;
				pixels[(height-(y+j)-1)*width*4 + (x+i)*4+2] = 0;
				pixels[(height-(y+j)-1)*width*4 + (x+i)*4+3] = out_a;
			} else {
				pixels[(height-(y+j)-1)*width*4 + (x+i)*4  ] = (src_a * red   * 255 + dst_a * dst_r * (255 - src_a)) / (out_a * 255);
				pixels[(height-(y+j)-1)*width*4 + (x+i)*4+1] = (src_a * green * 255 + dst_a * dst_g * (255 - src_a)) / (out_a * 255);
				pixels[(height-(y+j)-1)*width*4 + (x+i)*4+2] = (src_a * blue  * 255 + dst_a * dst_b * (255 - src_a)) / (out_a * 255);
				pixels[(height-(y+j)-1)*width*4 + (x+i)*4+3] = out_a;
			}
		}
	}
}

static void print_slot(FT_GlyphSlot slot)
{
	FT_Bitmap *bitmap = &slot->bitmap;

	printf("left:%d top:%d width:%d height:%d pitch:%d\n"
			, slot->bitmap_left, slot->bitmap_top
			, bitmap->width, bitmap->rows, bitmap->pitch);

	int i, j;
	unsigned char* p = (unsigned char*)bitmap->buffer;
	printf("+");
	for (i = 0; i < bitmap->width; i++) {
		printf("---");
	}
	printf("+\n");
	for (j = 0; j < bitmap->rows; j++) {
		printf("|");
		for (i = 0; i < bitmap->width; i++) {
			unsigned char byte = p[j*bitmap->pitch+i];
			if (byte != 0) {
				printf("%02x ", byte);
			} else {
				printf("   ");
			}
		}
		printf("|\n");
	}
	printf("+");
	for (i = 0; i < bitmap->width; i++) {
		printf("---");
	}
	printf("+\n");
}

static FT_ULong get_charcode(const unsigned char s[], const char** next)
{
	int len = strlen(s);
	if (len == 0) {
		*next = NULL;
		return 0;
	}

	if (s[0] < 0x80UL) {
		*next = &s[1];
		return s[0];
	}

	if (len < 2) {
		*next = NULL;
		return 0;
	}

	if ((0xc2 <= s[0]) && (s[0] <= 0xdf)) {
		*next = &s[2];
		return (s[0] & 0x3f) << 6 | s[1] & 0x3f;
	}

	if (len < 3) {
		*next = NULL;
		return 0;
	}

	if ((0xe0 <= s[0]) && (s[0] <= 0xef)) {
		*next = &s[3];
		return (s[0] & 0x0f) << 12 | (s[1] & 0x3f) << 6 | s[2] & 0x3f;
	}

	if (len < 4) {
		*next = NULL;
		return 0;
	}

	if ((0xf0 <= s[0]) && (s[0] <= 0xf7)) {
		*next = &s[4];
		return (s[0] & 0x0f) << 18 | (s[1] & 0x3f) << 12 | (s[2] & 0x3f) << 6 | s[3] & 0x3f;
	}

#if 0
	if (len < 5) {
		*next = NULL;
		return 0;
	}

	if ((0xf8 <= s[0]) && (s[0] <= 0xfb)) {
		*next = &s[5];
		return (s[0] & 0x07) << 24 | (s[1] & 0x3f) << 18 | (s[2] & 0x3f) << 12 | (s[3] & 0x3f << 6) | s[4] & 0x3f;
	}
#endif

	*next = NULL;
	return 0;
}

#endif

int ugles2_set_font(struct ugles2_context* context, const char file[])
{
#if defined(USE_FREETYPE)
	FT_Face face;
	int res = FT_New_Face(context->library, file, 0, &face);
	if (res == 0) {
		context->face = face;
		return 0;
	} else if (res == FT_Err_Unknown_File_Format) {
		return -2;
	} else {
		return -3;
	}
#else
	return -1;
#endif
}

#if defined(USE_FREETYPE)
static int draw_text(struct ugles2_context* context
		, GLubyte pixels[], int width, int height
		, int* draw_width, int* char_count
		, const char text[], int font_size
		, GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha
		, int x, int y)
{
	if (context->face == NULL) {
		return -1;
	}

	//FT_Set_Char_Size(context->face, 0, 16*64, width, height);
	FT_Set_Pixel_Sizes(context->face, 0, font_size);

	FT_GlyphSlot slot = context->face->glyph;
	const char *s = text;
	int count = 0;
	int w = 0;
	while ((s != NULL) && (strlen(s) > 0)) {
		FT_ULong charcode = get_charcode(s, &s);
		if (charcode == 0) {
			break;
		}

		int glyph_index = FT_Get_Char_Index(context->face, charcode);
		FT_Load_Glyph(context->face, glyph_index, FT_LOAD_DEFAULT);
		if (FT_Render_Glyph(context->face->glyph, FT_RENDER_MODE_NORMAL) != 0) {
			printf("FT_Render_Glyph() failed. \n");
			break;
		}

		//print_slot(slot);

		FT_Bitmap* bitmap = &slot->bitmap;

		int pos_x = x + w + slot->bitmap_left;
		int pos_y = y + (font_size - slot->bitmap_top - 1);
		if (pixels != NULL) {
			blend_glyph_bitmap(pixels, pos_x, pos_y, width, height
							, red, green, blue, bitmap->buffer, bitmap->width, bitmap->pitch, bitmap->rows);
		}

		//x += slot->advance.x >> 6;
		w += slot->advance.x >> 6;
		++count;
	}

	if (char_count != NULL) {
		*char_count = count;
	}
	if (draw_width != NULL) {
		*draw_width = w;
	}

	return 0;
}
#endif

int ugles2_text_size(struct ugles2_context* context, int* width, int* count, const char text[], int font_size)
{
#if defined(USE_FREETYPE)
	return draw_text(context, NULL, 0x7fffffff, 0x7fffffff, width, count, text, font_size, 0, 0, 0, 0, 0, 0);
#else
	return -1;
#endif
}

int ugles2_draw_text(struct ugles2_context* context
					, GLubyte* pixels, int width, int height
					, const char text[], int font_size, GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha
					, int x, int y)
{
#if defined(USE_FREETYPE)
	return draw_text(context, pixels, width, height
			, NULL, NULL
			, text, font_size, red, green, blue, alpha
			, x, y);
#else
	return -1;
#endif
}

// =============================================================================
// matrix
void ugles2_matrix_unit(float m[])
{
	m[ 0] = m[ 5] = m[10] = m[15] = 1.0f;
	m[ 1] = m[ 2] = m[ 3] = 0.0f;
	m[ 4] = m[ 6] = m[ 7] = 0.0f;
	m[ 8] = m[ 9] = m[11] = 0.0f;
	m[12] = m[13] = m[14] = 0.0f;
}

void ugles2_matrix_add(float result[], float a[], float b[])
{
	int i;
	for (i = 0; i < 16; i++) {
		result[i] = a[i] + b[i];
	}
}

void ugles2_matrix_sub(float result[], float a[], float b[])
{
	int i;
	for (i = 0; i < 16; i++) {
		result[i] = a[i] - b[i];
	}
}

void ugles2_matrix_multi(float result[], float a[], float b[])
{
	float tmp[16];

	tmp[ 0] = a[0]*b[0] + a[4]*b[1] + a[8]*b[2]  + a[12]*b[3];
	tmp[ 1] = a[1]*b[0] + a[5]*b[1] + a[9]*b[2]  + a[13]*b[3];
	tmp[ 2] = a[2]*b[0] + a[6]*b[1] + a[10]*b[2] + a[14]*b[3];
	tmp[ 3] = a[3]*b[0] + a[7]*b[1] + a[11]*b[2] + a[15]*b[3];

	tmp[ 4] = a[0]*b[4] + a[4]*b[5] + a[ 8]*b[6] + a[12]*b[7];
	tmp[ 5] = a[1]*b[4] + a[5]*b[5] + a[ 9]*b[6] + a[13]*b[7];
	tmp[ 6] = a[2]*b[4] + a[6]*b[5] + a[10]*b[6] + a[14]*b[7];
	tmp[ 7] = a[3]*b[4] + a[7]*b[5] + a[11]*b[6] + a[15]*b[7];

	tmp[ 8] = a[0]*b[8] + a[4]*b[9] + a[ 8]*b[10] + a[12]*b[11];
	tmp[ 9] = a[1]*b[8] + a[5]*b[9] + a[ 9]*b[10] + a[13]*b[11];
	tmp[10] = a[2]*b[8] + a[6]*b[9] + a[10]*b[10] + a[14]*b[11];
	tmp[11] = a[3]*b[8] + a[7]*b[9] + a[11]*b[10] + a[15]*b[11];

	tmp[12] = a[0]*b[12] + a[4]*b[13] + a[ 8]*b[14] + a[12]*b[15];
	tmp[13] = a[1]*b[12] + a[5]*b[13] + a[ 9]*b[14] + a[13]*b[15];
	tmp[14] = a[2]*b[12] + a[6]*b[13] + a[10]*b[14] + a[14]*b[15];
	tmp[15] = a[3]*b[12] + a[7]*b[13] + a[11]*b[14] + a[15]*b[15];

	int i;
	for (i = 0; i < 16; i++) {
		result[i] = tmp[i];
	}
}

void ugles2_matrix_rotate_x(float m[], float degree)
{
	ugles2_matrix_unit(m);
	float rad = ((float)degree * M_PI / 180.0);
	m[ 5] = cosf(rad);
	m[ 6] = - sinf(rad);
	m[ 9] = sinf(rad);
	m[10] = cosf(rad);
}

void ugles2_matrix_rotate_y(float m[], float degree)
{
	ugles2_matrix_unit(m);
	float rad = ((float)degree * M_PI / 180.0);
	m[ 0] = cosf(rad);
	m[ 2] = - sinf(rad);
	m[ 8] = sinf(rad);
	m[10] = cosf(rad);
}

void ugles2_matrix_rotate_z(float m[], float degree)
{
	ugles2_matrix_unit(m);
	float rad = ((float)degree * M_PI / 180.0);
	m[ 0] = cosf(rad);
	m[ 1] = - sinf(rad);
	m[ 4] = sinf(rad);
	m[ 5] = cosf(rad);
}

void ugles2_matrix_frustrum(float m[], float l, float r, float b, float t, float n, float f)
{
	m[ 0] = 2 * n / (r - l);
	m[ 1] = 0.0f;
	m[ 2] = 0.0f;
	m[ 3] = 0.0f;

	m[ 4] = 0.0f;
	m[ 5] = 2 * n / (t - b);
	m[ 6] = 0.0f;
	m[ 7] = 0.0f;

	m[ 8] = (r + l) / (r - l);
	m[ 9] = (t + b) / (t - b);
	m[10] = - (f + n) / (f - n);
	m[11] = -1.0f;

	m[12] = 0.0f;
	m[13] = 0.0f;
	m[14] = - (2 * f * n) / (f - n);
	m[15] = 0.0f;
}

void ugles2_matrix_perspective(float m[], float fovy, float aspect, float near, float far)
{
	float alpha = fovy * M_PI / 180.0f;
	float w = tanf(alpha / 2.0f);
	float h = w * aspect;

	ugles2_matrix_frustrum(m, -w, w, -h, h, near, far);
}

void ugles2_matrix_position(float m[], float x, float y, float z)
{
	m[0] = m[5] = m[10] = m[15] = 1.0f;

	m[12] = x;
	m[13] = y;
	m[14] = z;

	m[1] = m[2] = m[3] = m[4] = m[6] = m[7] = 0.0f;
	m[8] = m[9] = m[11] = 0.0f;
}

