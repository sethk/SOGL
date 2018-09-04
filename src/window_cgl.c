//
// Created by Seth Kingsley on 1/12/18.
//

#include <OpenGL/CGLContext.h>
#include <OpenGL/gl.h>
#include <stdlib.h>
#include <err.h>
#include <stdio.h>
#include <stdbool.h>
#include "window_cgl.h"

struct window *
window_create_cgl(CGLContextObj context)
{
	struct cgl_window *window = calloc(1, sizeof(*window));
	if (!window)
		err(1, "Alloc CGL window");
	window->context = context->rend;
	window->dispatch = context->disp;
	return (struct window *)window;
}

static void
window_check_error(struct cgl_window *cw, const char *label)
{
	GLenum error;
	while ((error = cw->dispatch.get_error(cw->context)) != GL_NO_ERROR)
		fprintf(stderr, "%s: GL error %x\n", label, error);
}

void
window_update(struct window *w,
              const struct raster_color *frame,
              u_int x,
              u_int y,
              u_int width,
              u_int height,
              bool flipped_y)
{
	struct cgl_window *cw = (struct cgl_window *)w;
	if (flipped_y)
	{
		cw->dispatch.pixel_zoom(cw->context, 1, -1);
		cw->dispatch.window_pos2i(cw->context, x, height - y);
	}
	else
		cw->dispatch.window_pos2i(cw->context, x, y);
	window_check_error(cw, "raster_pos");
	cw->dispatch.draw_pixels(cw->context, width, height, GL_RGBA, GL_UNSIGNED_BYTE, frame);
	window_check_error(cw, "draw_pixels");
	cw->dispatch.flush(cw->context);
}

void
window_flip(struct window *w)
{
	struct cgl_window *cw = (struct cgl_window *)w;
	cw->dispatch.swap_APPLE(cw->context);
}
