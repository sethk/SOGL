//
// Created by Seth Kingsley on 1/25/18.
//

#include <sys/param.h>
#include <math.h>
#include <assert.h>
#include "draw.h"
#include "raster.h"

struct raster_rect
{
	struct raster_coord min, max;
};

static struct raster_coord
raster_coord_clip(struct raster_coord rcoord, struct raster_rect clip)
{
	struct raster_coord clipped;
	clipped.x = MIN(MAX(clip.min.x, rcoord.x), clip.max.x);
	clipped.y = MIN(MAX(clip.min.y, rcoord.y), clip.max.y);
	return clipped;
}

static void
raster_cliprect_update(struct raster_rect *rect, struct raster_coord rcoord, struct raster_rect view_rect)
{
	struct raster_coord clipped = raster_coord_clip(rcoord, view_rect);
	rect->min.x = MIN(clipped.x, rect->min.x);
	rect->min.y = MIN(clipped.y, rect->min.y);
	rect->max.x = MAX(clipped.x, rect->max.x);
	rect->max.y = MAX(clipped.y, rect->max.y);
}

void
raster_horiz_line(struct drawable *d, struct draw_options options, struct device_vertex p1, struct device_vertex p2)
{
	raster_gradual_line(d, options, p1, p2);
}

void
raster_gradual_line(struct drawable *d, struct draw_options options, struct device_vertex p1, struct device_vertex p2)
{
	scalar_t device_delta_x = p2.coord.x - p1.coord.x;
	scalar_t device_delta_y = p2.coord.y - p1.coord.y;
	assert(fabs(device_delta_y) <= fabs(device_delta_x));
	struct raster_vertex rvert = raster_from_device(d, p1);
	raster_loc_t p1_x = rvert.coord.x;
	raster_loc_t p2_x = raster_x_from_device(d, p2.coord.x);
	raster_dist_t delta_x = p2_x - rvert.coord.x;
	raster_dist_t x_step;
	if (delta_x > 0)
		x_step = 1;
	else
		x_step = -1;
	while (rvert.coord.x != p2_x)
	{
		raster_pixel(d, options, rvert);
		scalar_t t = (scalar_t)(rvert.coord.x - p1_x) / delta_x;
		assert(t >= 0.0 && t <= 1.0);
		rvert.coord.y = raster_y_from_device(d, p1.coord.y + t * device_delta_y);
		rvert.color = vector4_lerp(p1.color, p2.color, t);
		rvert.coord.x+= x_step;
	}
}

void
raster_steep_line(struct drawable *d, struct draw_options options, struct device_vertex p1, struct device_vertex p2)
{
	double device_delta_x = p2.coord.x - p1.coord.x;
	double device_delta_y = p2.coord.y - p1.coord.y;
	assert(fabs(device_delta_x) <= fabs(device_delta_y));
	struct raster_vertex rvert = raster_from_device(d, p1);
	raster_loc_t p1_y = rvert.coord.y;
	raster_loc_t p2_y = raster_y_from_device(d, p2.coord.y);
	raster_dist_t delta_y = p2_y - rvert.coord.y;
	raster_dist_t y_step;
	if (delta_y > 0)
		y_step = 1;
	else
		y_step = -1;
	while (rvert.coord.y != p2_y)
	{
		raster_pixel(d, options, rvert);
		scalar_t t = (scalar_t)(rvert.coord.y - p1_y) / delta_y;
		assert(t >= 0.0 && t <= 1.0);
		rvert.coord.x = raster_x_from_device(d, p1.coord.x + t * device_delta_x);
		rvert.color = vector4_lerp(p1.color, p2.color, t);
		rvert.coord.y+= y_step;
	}
}

void
raster_vertical_line(struct drawable *d, struct draw_options options, struct device_vertex p1, struct device_vertex p2)
{
	raster_steep_line(d, options, p1, p2);
}

struct implicit_line2
{
	struct vector2 norm;
	scalar_t w;
};

static struct implicit_line2
raster_make_edge(const struct vector2 p1, const struct vector2 p2)
{
	struct vector2 diff = vector2_sub(p2, p1);
	struct implicit_line2 edge;
	edge.norm = vector2_norm((struct vector2){.x = -diff.y, .y = diff.x});
	edge.w = vector2_dot(edge.norm, p1);
	return edge;
}

static scalar_t
raster_edge_dist(struct vector2 p, struct implicit_line2 edge)
{
	return vector2_dot(edge.norm, p) - edge.w;
}

static bool
raster_point_in_poly(struct vector2 p, struct implicit_line2 edges[], u_int num_edges)
{
	for (u_int i = 0; i < num_edges; ++i)
		if (raster_edge_dist(p, edges[i]) > SCALAR_EPSILON)
			return false;

	return true;
}

static struct raster_rect
raster_rect_from_view(struct drawable *d)
{
	return (struct raster_rect)
	{
		.min = {.x = d->view_x, .y = d->view_y},
		.max = {.x = d->view_x + d->view_width, .y = d->view_y + d->view_height}
	};
}

void
raster_triangle(struct drawable *d, struct draw_options options, const struct device_vertex vertices[3])
{
	struct vector2 edge1 = vector2_sub(vertices[1].coord.xy, vertices[0].coord.xy);
	struct vector2 edge2 = vector2_sub(vertices[2].coord.xy, vertices[0].coord.xy);
	scalar_t double_area = edge1.x * edge2.y - edge1.y * edge2.x;
	scalar_t depths[3];

	struct raster_rect view_rect = raster_rect_from_view(d);
	struct raster_rect clip = {.min = view_rect.max, .max = view_rect.max};
	for (u_int i = 0; i < 3; ++i)
	{
		struct raster_vertex rvert = raster_from_device(d, vertices[i]);
		raster_cliprect_update(&clip, rvert.coord, view_rect);
		depths[i] = rvert.depth;
	}

	struct raster_vertex rvert;
	for (rvert.coord.x = clip.min.x; rvert.coord.x < clip.max.x; ++rvert.coord.x)
		for (rvert.coord.y = clip.min.y; rvert.coord.y < clip.max.y; ++rvert.coord.y)
		{
			struct vector2 dcoord = raster_to_device(d, rvert.coord);
			struct vector2 vert_vec = vector2_sub(dcoord, vertices[0].coord.xy);
			scalar_t u = (vert_vec.x * edge2.y - vert_vec.y * edge2.x) / double_area;

			if (u >= 0.0)
			{
				scalar_t v = (edge1.x * vert_vec.y - edge1.y * vert_vec.x) / double_area;
				if (v >= 0.0)
				{
					scalar_t t = 1.0 - (u + v);
					if (t >= 0.0)
					{
						rvert.depth = depths[1] * u + depths[2] * v + depths[0] * t;
						if (rvert.depth >= 0 && rvert.depth <= 1.0)
						{
							{
								rvert.color = vector4_mult_scalar(vertices[1].color, u);
								rvert.color = vector4_add(rvert.color, vector4_mult_scalar(vertices[2].color, v));
								rvert.color = vector4_add(rvert.color, vector4_mult_scalar(vertices[0].color, t));
							}
							raster_pixel(d, options, rvert);
						}
					}
				}
			}
		}
}

void
raster_polygon(struct drawable *d, struct draw_options options, const struct device_vertex dverts[], u_int num_verts)
{
	struct implicit_line2 edges[MAX_PRIMITIVE_VERTICES];
	struct raster_rect view_rect = raster_rect_from_view(d);
	struct raster_rect clip = {.min = view_rect.max, .max = view_rect.min};
	for (u_int i = 0; i < num_verts; ++i)
	{
		struct raster_vertex rvert = raster_from_device(d, dverts[i]);
		raster_cliprect_update(&clip, rvert.coord, view_rect);
		edges[i] = raster_make_edge(dverts[i].coord.xy, dverts[(i + 1) % num_verts].coord.xy);
	}

	struct raster_vertex v;
	v.depth = (dverts[0].coord.z + 1.0) / 2.0;
	v.color = dverts[0].color;
	for (v.coord.x = clip.min.x; v.coord.x < clip.max.x; ++v.coord.x)
		for (v.coord.y = clip.min.y; v.coord.y < clip.max.y; ++v.coord.y)
		{
			struct vector2 dvert = raster_to_device(d, v.coord);
			if (raster_point_in_poly(dvert, edges, num_verts))
				raster_pixel(d, options, v);
		}
}
