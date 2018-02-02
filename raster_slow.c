//
// Created by Seth Kingsley on 1/25/18.
//

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include "draw.h"
#include "raster.h"

/*
static raster_loc_t
draw_convert_x_bias(struct drawable *d, GLdouble dx, GLdouble bias)
{
	return d->view_x + lround(((dx + 1.0) / 2.0) * d->view_width + bias);
}
 */

static struct device_vertex
raster_device_lerp(struct device_vertex p1, struct device_vertex p2, GLdouble t)
{
	struct device_vertex dv;
	dv.coord = vector3_lerp(p1.coord, p2.coord, t);
	dv.color = vector4_lerp(p1.color, p2.color, t);
	return dv;
}

void
raster_horiz_line(struct drawable *d, struct draw_options options, struct device_vertex p1, struct device_vertex p2)
{
	raster_gradual_line(d, options, p1, p2);
}

static bool
raster_check_clipped(struct drawable *d, struct raster_vertex rv)
{
	return (rv.x >= d->view_x &&
			rv.x < d->view_x + d->view_width &&
			rv.y >= d->view_y &&
			rv.y < d->view_y + d->view_width);
}

void
raster_gradual_line(struct drawable *d, struct draw_options options, struct device_vertex p1, struct device_vertex p2)
{
#if 0
	GLdouble rx1 = raster_convert_device_x(d, p1.coord.x);
	GLdouble rx2 = raster_convert_device_x(d, p2.coord.x);
	GLdouble delta_x = rx2 - rx1;
	assert(delta_x >= 0);
	if (delta_x == 0)
		return;
	/*
	double dy = p2.coord.y - p1.coord.y;
	double dx = p2.coord.x - p1.coord.x;
	assert(fabs(dy) <= fabs(dx));
	double m = dy / dx;
	double y = p1.coord.y;
	raster_loc_t max_x = draw_convert_x(d, p2.coord.x);
	 */
	for (raster_loc_t x = d->view_x; x < d->view_x + d->view_width; ++x)
	{
		GLdouble x_offset = (GLdouble)x - rx1;
		if (x_offset >= 0 && x_offset <= delta_x)
		{
			GLdouble t = x_offset / delta_x;
		}
			/*
		struct device_vertex dv = draw_device_lerp(p1, p2, t);
		struct raster_vertex rv = draw_convert_vertex(d, dv);
		if (draw_check_clipped(d, rv))
			draw_pixel(d, options, rv);
			 */
	}
#endif // 0
}

void
raster_steep_line(struct drawable *d, struct draw_options options, struct device_vertex p1, struct device_vertex p2)
{
	/*
	assert(p1.y < p2.y);
	double dy = p2.y - p1.y;
	double dx = p2.x - p1.x;
	assert(fabs(dx) <= fabs(dy));
	double inv_m = dx / dy;
	double x = p1.x;
	for (struct raster_vertex v = p1; v.y <= p2.y; ++v.y)
	{
		v.x = lround(x);
		draw_pixel(d, options, v);
		x+= inv_m;
		v.color = vector4_lerp(p1.color, p2.color, (v.y - p1.y) / dy);
	}
	 */
}

void
raster_vertical_line(struct drawable *d, struct draw_options options, struct device_vertex p1, struct device_vertex p2)
{
}

struct implicit_line2
{
	struct vector2 norm;
	scalar_t w;
};
struct implicit_line2
raster_make_edge(const struct vector2 p1, const struct vector2 p2)
{
	struct vector2 diff = vector2_sub(p2, p1);
	struct implicit_line2 edge;
	edge.norm = vector2_norm((struct vector2){.x = -diff.y, .y = diff.x});
	edge.w = vector2_dot(edge.norm, p1);
	return edge;
}

void
raster_polygon(struct drawable *d, struct draw_options options, const struct device_vertex dverts[], u_int num_verts)
{
	struct implicit_line edges[MAX_PRIMITIVE_VERTICES];
	for (u_int i = 0; i < num_verts; ++i)
	{
		edges[i] = raster_make_edge()
	}

	struct raster_vertex v;
	v.color = (struct vector4){.v = {1, 1, 1, 1}};
	for (v.x = d->view_x; v.x < d->view_x + d->view_width; ++v.x)
		for (v.y = d->view_y; v.y < d->view_y + d->view_height; ++v.y)
		{
			struct vector2 dvert = raster_to_device(d, v);
			if (sign(dvert, dverts[0].coord.xy, dverts[1].coord.xy) * 100 > 0.0)
				continue;
			fprintf(stderr, "%g\n", sign(dvert, dverts[0].coord.xy, dverts[1].coord.xy));
			/*
			if (sign(dvert, dverts[1].coord.xy, dverts[2].coord.xy) < 0.0)
				continue;
			if (sign(dvert, dverts[2].coord.xy, dverts[0].coord.xy) < 0.0)
				continue;
			 */
			raster_pixel(d, options, v);
		}
}
