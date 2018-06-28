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
	clipped.depth = MIN(MAX(clip.min.depth, rcoord.depth), clip.max.depth);
	return clipped;
}

static void
raster_cliprect_update(struct raster_rect *rect, struct raster_coord rcoord, struct raster_rect view_rect)
{
	struct raster_coord clipped = raster_coord_clip(rcoord, view_rect);
	rect->min.x = MIN(clipped.x, rect->min.x);
	rect->min.y = MIN(clipped.y, rect->min.y);
	rect->max.depth = MIN(clipped.depth, rect->min.depth);
	rect->max.x = MAX(clipped.x, rect->max.x);
	rect->max.y = MAX(clipped.y, rect->max.y);
	rect->max.depth = MAX(clipped.depth, rect->max.depth);
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
		rvert.coord.x+= x_step;
		scalar_t t = (scalar_t)(rvert.coord.x - p1_x) / delta_x;
		assert(t >= 0.0 && t <= 1.0);
		rvert.coord.y = raster_y_from_device(d, p1.coord.y + t * device_delta_y);
		rvert.color = vector4_lerp(p1.color, p2.color, t);
		raster_pixel(d, options, rvert);
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
		rvert.coord.y+= y_step;
		scalar_t t = (scalar_t)(rvert.coord.y - p1_y) / delta_y;
		assert(t >= 0.0 && t <= 1.0);
		rvert.coord.x = raster_x_from_device(d, p1.coord.x + t * device_delta_x);
		rvert.color = vector4_lerp(p1.color, p2.color, t);
		raster_pixel(d, options, rvert);
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
raster_scan_trapezoid(struct drawable *d,
                      const struct device_vertex *left_bottom, const struct device_vertex *left_top,
                      const struct device_vertex *right_bottom, const struct device_vertex *right_top)
{
	assert(left_bottom->coord.x < right_bottom->coord.x);
	assert(left_bottom->coord.y == right_bottom->coord.y);
	assert(left_top->coord.y == right_top->coord.y);
	scalar_t y_delta = left_top->coord.y - left_bottom->coord.y;
	scalar_t left_x_delta = left_top->coord.x - left_bottom->coord.x;
	scalar_t left_depth_delta = left_top->coord.z - left_bottom->coord.z;
	scalar_t right_x_delta = right_top->coord.x - right_bottom->coord.x;
	scalar_t right_depth_delta = right_top->coord.z - right_bottom->coord.z;
	raster_loc_t min_y = floor(left_bottom->coord.y);
	raster_loc_t max_y = ceil(left_top->coord.y);
	for (raster_loc_t y = min_y; y < max_y; ++y)
	{
		scalar_t t = (y - left_bottom->coord.y) / y_delta;
		scalar_t left_x = left_bottom->coord.x + left_x_delta * t;
		raster_loc_t left_raster_x = floor(left_x);
		scalar_t right_x = right_bottom->coord.x + right_x_delta * t;
		raster_loc_t right_raster_x = floor(right_x);
		raster_dist_t x_delta = right_raster_x - left_raster_x;
		if (x_delta > 0)
		{
			scalar_t left_depth = left_bottom->coord.z + left_depth_delta * t;
			scalar_t right_depth = right_bottom->coord.z + right_depth_delta * t;
			d->spans[d->num_spans].y = y;
			d->spans[d->num_spans].left_x = left_x;
			d->spans[d->num_spans].x_delta = x_delta;
			d->spans[d->num_spans].left_depth = left_depth;
			d->spans[d->num_spans].depth_delta = right_depth - left_depth;
		}
	}
	/*
	struct vector3 win_coords[3];
	scalar_t inv_slopes[4];
	for (u_int i = 0; i < 3; ++i)
		win_coords[i] = vector4_project(matrix4x4_mult_vector4(d->view_trans, vertices[i].coord));
	raster_loc_t min_y = floor(win_coords[0].y);
	raster_loc_t mid_y = floor(win_coords[1].y);
	raster_loc_t max_y = floor(win_coords[2].y);

	for (raster_loc_t y = min_y; y < max_y; ++y)
	{
		if (y < mid_y)
		{
			scalar_t x0 = (y - win_coords[0].y) * inv_slopes[0];
			scalar_t x1 = (y - win_coords[1].y) * inv_slopes[1];
		}
	}
	 */
	/*
	raster_loc_t max_y = (raster_loc_t)floor(win_coords[0].y);
	raster_loc_t mid_y = (raster_loc_t)floor(win_coords[1].y);
	raster_loc_t mid_x = (raster_loc_t)floor(win_coords[1].x);
	raster_loc_t opposite_x = (raster_loc_t)floor(win_coords[2].x);
	if (mid_x < opposite_x)
		raster_scan_trapezoid(d, mid_x, mid_y, opposite_x, max_y
	 */
	/*
	scalar_t top_y = -1;
	u_int top_index;
	for (u_int i = 0; i < 3; ++i)
	{
		if (vertices[i].coord.y > top_y)
		{
			top_index = i;
			top_y = vertices[i].coord.y;
		}
	}

	scalar_t right_x = -1;
	u_int right_index;
	for (u_int i = 0; i < 3; ++i)
	{
		if (i != top_index)
		{
			if (vertices[i].coord.x > right_x)
			{
				right_index = i;
				right_x = vertices[i].coord.x;
			}
		}
	}

	u_int left_index;
	for (u_int i = 0; i < 3; ++i)
	{
		if (i != top_index && i != right_index)
		{
			left_index = i;
			break;
		}
	}

	struct vector3 top_coord, right_coord, left_coord;
	top_coord = vector4_project(matrix4x4_mult_vector4(d->view_trans, vertices[top_index].coord));
	right_coord = vector4_project(matrix4x4_mult_vector4(d->view_trans, vertices[right_index].coord));
	left_coord = vector4_project(matrix4x4_mult_vector4(d->view_trans, vertices[left_index].coord));
	 */
}

void
raster_triangle(struct drawable *d, struct draw_options options, const struct window_vertex vertices[3])
{
	struct vector2 edge1 = vector2_sub(vertices[1].coord.xy, vertices[0].coord.xy);
	struct vector2 edge2 = vector2_sub(vertices[2].coord.xy, vertices[0].coord.xy);
	scalar_t double_area = edge1.x * edge2.y - edge1.y * edge2.x;
	struct implicit_line2 lines[3];
	if (double_area >= 0)
	{
		lines[0] = raster_make_edge(vertices[0].coord.xy, vertices[1].coord.xy);
		lines[1] = raster_make_edge(vertices[1].coord.xy, vertices[2].coord.xy);
		lines[2] = raster_make_edge(vertices[2].coord.xy, vertices[0].coord.xy);
	}
	else
	{
		lines[0] = raster_make_edge(vertices[0].coord.xy, vertices[2].coord.xy);
		lines[1] = raster_make_edge(vertices[2].coord.xy, vertices[1].coord.xy);
		lines[2] = raster_make_edge(vertices[1].coord.xy, vertices[0].coord.xy);
	}

	struct raster_rect view_rect = raster_rect_from_view(d);
	struct raster_rect clip = view_rect; //{.min = view_rect.max, .max = view_rect.max};
	//struct raster_vertex rverts[3];
	//for (u_int i = 0; i < 3; ++i)
	//{
		//rverts[i] = raster_from_device(d, vertices[i]);
		//raster_cliprect_update(&clip, vertices[i].coord, view_rect);
	//}

	struct raster_vertex rvert;
	for (rvert.coord.x = clip.min.x; rvert.coord.x < clip.max.x; ++rvert.coord.x)
		for (rvert.coord.y = clip.min.y; rvert.coord.y < clip.max.y; ++rvert.coord.y)
		{
			bool top_left = false;

			//struct vector2 dcoord = raster_to_device(d, rvert.coord);
			struct vector2 coord = {.x = rvert.coord.x + 0.5, .y = rvert.coord.y + 0.5};
			u_int edge_index;
			for (edge_index = 0; edge_index < 3; ++edge_index)
			{
				scalar_t dist = raster_edge_dist(coord, lines[edge_index]);
				if (dist < 0)
					break;
				else if (dist == 0.0)
				{
					//break;
					top_left = true;
				}
			}

			if (edge_index == 3)
			{
				struct vector2 vert_vec = vector2_sub(coord, vertices[0].coord.xy);
				scalar_t u = (vert_vec.x * edge2.y - vert_vec.y * edge2.x) / double_area;
				scalar_t v = (edge1.x * vert_vec.y - edge1.y * vert_vec.x) / double_area;
				scalar_t t = 1.0 - (u + v);
				rvert.coord.depth =
						vertices[1].coord.z * u + vertices[2].coord.z * v + vertices[0].coord.z * t;
				if (rvert.coord.depth >= 0 && rvert.coord.depth <= 1.0)
				{
					if (top_left)
					{
						rvert.color = (struct vector4){.r = 1, .g = 0, .b = 0, .a = 1};
					}
					else
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

void
raster_fill_spans(struct drawable *d, struct draw_options options)
{
	for (u_int i = 0; i < d->num_spans; ++i)
	{
		struct raster_vertex rvert;
		rvert.coord.y = d->spans[i].y;
		rvert.coord.x = d->spans[i].left_x;
		for (raster_dist_t off = 0; off < d->spans[i].x_delta; ++off)
		{
			scalar_t u = off / d->spans[i].x_delta;
			rvert.coord.depth = d->spans[i].left_depth + u * d->spans[i].depth_delta;
			rvert.color = vector4_add(d->spans[i].left_color, vector4_mult_scalar(d->spans[i].color_delta, u));
			raster_pixel(d, options, rvert);
			rvert.coord.x+= 1;
		}
	}
}
