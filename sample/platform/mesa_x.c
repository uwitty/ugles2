#include "../../src/ugles2.h"

#include  <X11/Xlib.h>
#include  <X11/Xatom.h>
#include  <X11/Xutil.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

struct platform_data {
	Display* x_display;
};

static EGLNativeWindowType create_native_window(Display* x_display, int width, int height);

static void ugles2_platform_mesa_x_close(struct ugles2_platform* platform)
{
	struct platform_data* data = (struct platform_data*)platform->data;
	XDestroyWindow(data->x_display, platform->window);

	if (data->x_display != NULL) {
		XCloseDisplay(data->x_display);
		data->x_display = NULL;
	}

	free(data);
	platform->data = NULL;
}

int ugles2_platform_mesa_x_open(struct ugles2_platform* platform, void* arg)
{
	Display* x_display = NULL;

	//printf("platform->width:%d platform->height:%d\n", platform->width, platform->height);
	if (platform->width == 0) {
		platform->width = 960;
	}
	if (platform->height == 0) {
		platform->height = 540;
	}
	//printf("platform->width:%d platform->height:%d\n", platform->width, platform->height);

	struct platform_data* data = (struct platform_data*)malloc(sizeof(struct platform_data));
	if (data == NULL) {
		return -1;
	}

	x_display = XOpenDisplay(NULL);
	if (x_display == NULL) {
		goto error_return;
	}

	platform->window = create_native_window(x_display, platform->width, platform->height);
	data->x_display  = x_display;
	platform->data   = data;
	platform->close  = ugles2_platform_mesa_x_close;

	return 0;

error_return:
	if (x_display != NULL) {
		XCloseDisplay(x_display);
	}
	if (data != NULL) {
		free(data);
	}
	return -1;
}

EGLNativeWindowType create_native_window(Display* x_display, int width, int height)
{
	Window root;
	XSetWindowAttributes swa;
	XSetWindowAttributes  xattr;
	Atom wm_state;
	XWMHints hints;
	XEvent xev;
	Window win;

	x_display = XOpenDisplay(NULL);
	if (x_display == NULL)
	{
		return EGL_FALSE;
	}

	root = DefaultRootWindow(x_display);
	swa.event_mask  =  ExposureMask | PointerMotionMask | KeyPressMask;
	win = XCreateWindow(
			x_display, root,
			0, 0, width, height, 0,
			CopyFromParent, InputOutput,
			CopyFromParent, CWEventMask,
			&swa);

	xattr.override_redirect = FALSE;
	XChangeWindowAttributes(x_display, win, CWOverrideRedirect, &xattr);

	hints.input = TRUE;
	hints.flags = InputHint;
	XSetWMHints(x_display, win, &hints);

	XMapWindow (x_display, win);
	XStoreName (x_display, win, "sample");

	wm_state = XInternAtom (x_display, "_NET_WM_STATE", FALSE);

	memset ( &xev, 0, sizeof(xev) );
	xev.type                 = ClientMessage;
	xev.xclient.window       = win;
	xev.xclient.message_type = wm_state;
	xev.xclient.format       = 32;
	xev.xclient.data.l[0]    = 1;
	xev.xclient.data.l[1]    = FALSE;
	XSendEvent(x_display, DefaultRootWindow(x_display), FALSE, SubstructureNotifyMask, &xev);

	return (EGLNativeWindowType)win;
}

