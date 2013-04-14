#include "../../src/ugles2.h"
#include <stdio.h>

struct platform_data {
	DISPMANX_DISPLAY_HANDLE_T display;
	DISPMANX_UPDATE_HANDLE_T  update;
	DISPMANX_ELEMENT_HANDLE_T element;
	EGL_DISPMANX_WINDOW_T     window;
	int width;
	int height;
};

static void ugles2_platform_raspberrypi_close(struct ugles2_platform* platform)
{
	struct platform_data* data = (struct platform_data*)platform->data;

	if (data != NULL) {
		vc_dispmanx_element_remove(data->update, data->element);
		vc_dispmanx_update_submit_sync(data->update);
		vc_dispmanx_display_close(data->display);
	}
	free(data);
	memset(platform, 0, sizeof(struct ugles2_platform));
}

int ugles2_platform_raspberrypi_open(struct ugles2_platform* platform, void* arg)
{
	struct platform_data* data = (struct platform_data*)malloc(sizeof(struct platform_data));
	if (data == NULL) {
		return -1;
	}
	memset(data, 0, sizeof(struct platform_data));

	bcm_host_init();

	uint32_t width;
	uint32_t height;
	VC_RECT_T dst_rect;
	VC_RECT_T src_rect;
	DISPMANX_ELEMENT_HANDLE_T dispman_element;
	DISPMANX_DISPLAY_HANDLE_T dispman_display;
	DISPMANX_UPDATE_HANDLE_T  dispman_update;
	VC_DISPMANX_ALPHA_T alpha = {DISPMANX_FLAGS_ALPHA_FIXED_ALL_PIXELS, 255, 0};

	if (graphics_get_display_size(0, &width, &height) < 0) {
		return -1;
	}

	vc_dispmanx_rect_set(&dst_rect, 0, 0, width, height);
	vc_dispmanx_rect_set(&src_rect, 0, 0, width << 16, height << 16);

	dispman_display = vc_dispmanx_display_open(0);
	dispman_update  = vc_dispmanx_update_start(0);
	dispman_element = vc_dispmanx_element_add(dispman_update, dispman_display,
			0, &dst_rect, 0, &src_rect, DISPMANX_PROTECTION_NONE, &alpha, 0, 0);
	vc_dispmanx_update_submit_sync(dispman_update);

	data->element = dispman_element;
	data->update  = dispman_update;
	data->display = dispman_display;
	data->width   = width;
	data->height  = height;

	data->window.element = dispman_element;
	data->window.width   = width;
	data->window.height  = height;

	platform->window = &data->window;
	platform->data   = data;
	platform->close  = ugles2_platform_raspberrypi_close;
	platform->width  = width;
	platform->height = height;

	return 0;
}

#if 0
struct native_config
{
	int width;
	int height;
	DISPMANX_ELEMENT_HANDLE_T dispman_element;
};

int setup_native_window(struct native_config* config)
{
	printf("%s()\n", __func__);

	uint32_t width;
	uint32_t height;
	VC_RECT_T dst_rect;
	VC_RECT_T src_rect;
	DISPMANX_ELEMENT_HANDLE_T dispman_element;
	DISPMANX_DISPLAY_HANDLE_T dispman_display;
	DISPMANX_UPDATE_HANDLE_T  dispman_update;
	VC_DISPMANX_ALPHA_T alpha = {DISPMANX_FLAGS_ALPHA_FIXED_ALL_PIXELS, 255, 0};

	if ( graphics_get_display_size(0, &width, &height) < 0) {
		return -1;
	}

	vc_dispmanx_rect_set(&dst_rect, 0, 0, width, height);
	vc_dispmanx_rect_set(&src_rect, 0, 0, width << 16, height << 16);

	dispman_display = vc_dispmanx_display_open(0);
	dispman_update  = vc_dispmanx_update_start(0);
	dispman_element = vc_dispmanx_element_add(dispman_update, dispman_display,
			0, &dst_rect, 0, &src_rect, DISPMANX_PROTECTION_NONE, &alpha, 0, 0);
	vc_dispmanx_update_submit_sync(dispman_update);

	config->dispman_element = dispman_element;
	config->width = width;
	config->height = height;

	return 0;
}

void getNativeWindow(EGL_DISPMANX_WINDOW_T* window, const struct native_config* config)
{
	window->element = config->dispman_element;
	window->width   = config->width;
	window->height  = config->height;

}

int main(int argc, char *argv[])
{
	struct native_config config;
	struct egl_context   context;
	struct user_data user_data;
	EGL_DISPMANX_WINDOW_T window;

	bcm_host_init();

	fflush(stdout);

	if (setup_native_window(&config) < 0) {
		printf("setup_native_window() failed. \n");
	}

	getNativeWindow(&window, &config);
	if (setup_window_surface(EGL_DEFAULT_DISPLAY, &window, &context, config.width, config.height) < 0) {
		printf("setup_window_surface() failed. \n");
	}

	//ugles_info_configs(context.display);

	//render_simple_triangle(&context, &user_data);
	//render_color(&context, 0.2f, 0.3f, 0.1f, 1.0f);
	render_textured_triangle(&context, &user_data);

	fflush(stdout);
	//char c;
	//fgets(&c, 1, stdin);
	//sleep(3);

	return 0;
}
#endif

