//
// Created by Seth Kingsley on 1/25/18.
//

#include <OpenGL/gl.h>
#include <assert.h>
#include <math.h>
#include "raster.h"

GLdouble
raster_x_from_device(struct drawable *d, GLdouble dx)
{
	return (dx + 1.0) * (d->view_width / 2.0) + d->view_x;
}

struct raster_vertex
raster_from_device(struct drawable *d, struct device_vertex dv)
{
	struct raster_vertex rv;
	rv.x = lround(raster_x_from_device(d, dv.coord.x));
	rv.y = d->view_y + lround(((dv.coord.y + 1.0) / 2.0) * d->view_height);
	rv.color = dv.color;
	rv.depth = (dv.coord.z + 1.0) / 2.0;
	return rv;
}

struct vector2 raster_to_device(struct drawable *d, struct raster_vertex rv)
{
	struct vector2 dv;
	dv.x = (((GLdouble)rv.x + 0.5) - d->view_x) / (d->view_width / 2.0) - 1.0;
	dv.y = (((GLdouble)rv.y + 0.5) - d->view_y) / (d->view_height / 2.0) - 1.0;
	return dv;
}

void
raster_pixel(struct drawable *d, struct draw_options options, struct raster_vertex vertex)
{
	assert(vertex.x < d->window_width);
	assert(vertex.y < d->window_height);
	if (options.test_depth)
	{
		assert(vertex.depth >= 0 && vertex.depth <= 1.0);
		switch (options.depth_func)
		{
			case GL_NEVER:
				return;

			case GL_LESS:
				if (vertex.depth < d->depth_buffer[vertex.y * d->window_width + vertex.x])
					break;
				else
					return;

			case GL_ALWAYS:
				break;

			default:
				assert(!"Depth func not implemented");
		}
		d->depth_buffer[vertex.y * d->window_width + vertex.x] = vertex.depth;
	}
	switch (options.draw_op)
	{
		case GL_COPY:
		{
			struct raster_color *pixel = d->color_buffer + vertex.y * d->window_width + vertex.x;
#if CLAMP_COLORS
			pixel->red = round(fmax(fmin(vertex.color.r, 1.0), 0) * 255);
			pixel->green = round(fmax(fmin(vertex.color.g, 1.0), 0) * 255);
			pixel->blue = round(fmax(fmin(vertex.color.b, 1.0), 0) * 255);
			pixel->alpha = round(fmax(fmin(vertex.color.a, 1.0), 0) * 255);
#else
			assert(vertex.color.r >= 0 && vertex.color.r <= 1.0);
			assert(vertex.color.g >= 0 && vertex.color.g <= 1.0);
			assert(vertex.color.b >= 0 && vertex.color.b <= 1.0);
			pixel->red = lround(vertex.color.r * 255.0);
			pixel->green = lround(vertex.color.g * 255.0);
			pixel->blue = lround(vertex.color.b * 255.0);
			pixel->alpha = lround(vertex.color.a * 255.0);
#endif // CLAMP_COLORS
			break;
		}

		case GL_NOOP:
			break;

		default:
			assert(!"Draw operation not implemented");
	}
}

