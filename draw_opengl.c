//
// Created by Seth Kingsley on 1/8/18.
//

#include <OpenGL/CGLContext.h>
#include <OpenGL/CGLCurrent.h>
#include <stdlib.h>
#include <err.h>
#include <strings.h>

#include "draw.h"
#include "window_cgl.h"

struct drawable
{
	GLIContext rend;
	GLIFunctionDispatch disp;
};

struct drawable *
draw_create(struct window *window)
{
	struct cgl_window *cgl_window = (struct cgl_window *)window;
	struct drawable *d = calloc(1, sizeof(*d));
	if (!d)
		err(1, "Alloc drawable");
	d->rend = cgl_window->context;
	d->disp = cgl_window->dispatch;
	return d;
}

void
draw_destroy(struct drawable *d)
{
	free(d);
}

void
draw_reshape(struct drawable *d, u_int width, u_int height)
{
}

void
draw_set_view(struct drawable *d, u_int x, u_int y, u_int width, u_int height)
{
	d->disp.viewport(d->rend, x, y, width, height);
}

void
draw_clear(struct drawable *d, bool color, const struct vector4 clear_color, bool depth, GLfloat clear_depth)
{
	d->disp.clear_color(d->rend, clear_color.r, clear_color.g, clear_color.b, clear_color.a);
	GLenum mask = ((color) ? GL_COLOR_BUFFER_BIT : 0) | ((depth) ? GL_DEPTH_BUFFER_BIT : 0);
	d->disp.clear(d->rend, mask);
}

void
draw_primitive(struct drawable *d,
               struct draw_options options,
               const struct device_vertex vertices[],
               u_int num_verts)
{
	GLenum mode;
	switch (num_verts)
	{
		case 1: mode = GL_POINTS; break;
		case 2: mode = GL_LINES; break;
		case 3: mode = GL_TRIANGLES; break;
		case 4: mode = GL_QUADS; break;
		default: mode = GL_POLYGON;
	}
	d->disp->logic_op(d->rend, options.draw_op);
	if (options.test_depth)
		d->disp.enable(d->rend, GL_DEPTH_TEST);
	else
		d->disp.disable(d->rend, GL_DEPTH_TEST);
	d->disp.begin(d->rend, mode);
	for (GLuint i = 0; i < num_verts; ++i)
	{
		d->disp.color4dv(d->rend, vertices[i].color.v);
		d->disp.vertex3dv(d->rend, vertices[i].coord.v);
	}
	d->disp.end(d->rend);
	//d->disp.flush(d->rend);
}

void
draw_flush(struct drawable *d)
{
	d->disp.flush(d->rend);
}

void
draw_finish(struct drawable *d)
{
	d->disp.finish(d->rend);
}

void
draw_flip(struct drawable *d)
{
	d->disp.swap_APPLE(d->rend);
}
