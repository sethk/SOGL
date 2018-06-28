//
// Created by Seth Kingsley on 1/25/18.
//

#include <OpenGL/gl.h>
#include <assert.h>
#include <math.h>
#include "raster.h"
#include "matrix.h"
#include "vector.h"

#define CLAMP_COLORS 1

raster_loc_t
raster_x_from_device(struct drawable *d, scalar_t dx)
{
	scalar_t rx = ((dx + 1.0) / 2.0) * d->view_width;
	return d->view_x + (raster_loc_t)floor(rx + 0.5);
}

raster_loc_t
raster_y_from_device(struct drawable *d, scalar_t dy)
{
	scalar_t ry = ((dy + 1.0) / 2.0) * d->view_height;
	return d->view_y + (raster_loc_t)floor(ry + 0.5);
}

raster_depth_t
raster_z_from_device(struct drawable *d, scalar_t dz)
{
	return (raster_depth_t)((dz + 1.0) / 2.0);
}

struct raster_vertex
raster_from_device(struct drawable *d, struct device_vertex dv)
{
	struct vector3 dcoord = vector4_project(matrix4x4_mult_vector4(d->view_trans, dv.coord));
	struct raster_vertex rv;
	rv.coord.x = lround(dcoord.x); //raster_x_from_device(d, dcoord.x);
	rv.coord.y = lround(dcoord.y); //raster_y_from_device(d, dcoord.y);
	rv.coord.depth = dcoord.z; //raster_z_from_device(d, dcoord.z);
	rv.color = dv.color;
	return rv;
}

struct vector2
raster_to_device(struct drawable *d, struct raster_coord rc)
{
	struct vector2 dv;
	dv.x = (((GLdouble)rc.x /*+ 0.5*/) - d->view_x) / (d->view_width / 2.0) - 1.0;
	dv.y = (((GLdouble)rc.y /*+ 0.5*/) - d->view_y) / (d->view_height / 2.0) - 1.0;
	return dv;
}

void
raster_pixel(struct drawable *d, struct draw_options options, struct raster_vertex vertex)
{
	assert(vertex.coord.x >= 0);
	assert(vertex.coord.x < d->window_width);
	assert(vertex.coord.y >= 0);
	assert(vertex.coord.y < d->window_height);
	if (options.test_depth)
	{
		assert(vertex.coord.depth >= 0 && vertex.coord.depth <= 1.0);
		switch (options.depth_func)
		{
			case GL_NEVER:
				return;

			case GL_LESS:
				if (vertex.coord.depth < d->depth_buffer[vertex.coord.y * d->window_width + vertex.coord.x])
					break;
				else
					return;

			case GL_ALWAYS:
				break;

			default:
				assert(!"Depth func not implemented");
		}
		d->depth_buffer[vertex.coord.y * d->window_width + vertex.coord.x] = vertex.coord.depth;
	}
	switch (options.draw_op)
	{
		case GL_COPY:
		{
			struct raster_color *pixel = d->color_buffer + vertex.coord.y * d->window_width + vertex.coord.x;
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

void
raster_scan_point(struct drawable *d, const struct window_vertex *vertex)
{
	if (vertex->coord.x > 0 && vertex->coord.y < d->view_height)
	{
		d->spans[d->num_spans].y = (raster_loc_t)floor(vertex->coord.y);
		d->spans[d->num_spans].left_x = (raster_loc_t)ceil(vertex->coord.x - 1.0);
		d->spans[d->num_spans].left_depth = vertex->coord.z;
		d->spans[d->num_spans].x_delta = 1;
		d->spans[d->num_spans].left_color = vertex->color;
		d->spans[d->num_spans].color_delta = (struct vector4){.v = {0, 0, 0, 0}};
		++d->num_spans;
	}
}
