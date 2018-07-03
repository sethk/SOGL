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
#define RASTER_SCAN_LINE_INCR 0

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

enum point_part
{
	TOP_LEFT,
	TOP_RIGHT,
	BOTTOM_LEFT,
	BOTTOM_RIGHT,
	DIAMOND
};

static enum point_part
raster_point_part(scalar_t x_frac, scalar_t y_frac)
{
	if (x_frac < 0.5)
	{
		if (x_frac + y_frac < 0.5)
			return BOTTOM_LEFT;
		else if (x_frac + (1.0 - y_frac) <= 0.5)
			return TOP_LEFT;
		else
			return DIAMOND;
	}
	else if (x_frac > 0.5)
	{
		scalar_t right_frac = 1.0 - x_frac;
		if (right_frac + y_frac < 0.5)
			return BOTTOM_RIGHT;
		else if (right_frac + (1.0 - y_frac) <= 0.5)
			return TOP_RIGHT;
		else
			return DIAMOND;
	}
	else
		return DIAMOND;
}

static void
raster_emit_span_diff(struct drawable *d,
                      raster_loc_t y,
                      raster_loc_t left_x,
                      raster_loc_t right_x,
                      raster_depth_t left_depth,
                      raster_depth_t right_depth,
                      struct vector4 left_color,
                      struct vector4 right_color)
{
	struct raster_span *span = &(d->spans[d->num_spans]);
	span->y = y;
	span->left_x = left_x;
	span->x_delta = (right_x - left_x) + 1;
	span->left_depth = left_depth;
	span->depth_delta = right_depth - left_depth;
	span->left_color = left_color;
	span->color_delta = vector4_sub(right_color, left_color);
	++d->num_spans;
}

static void
raster_emit_span_para(struct drawable *d,
                      raster_loc_t y,
                      raster_loc_t left_x,
                      raster_dist_t x_delta,
                      scalar_t left_t,
                      scalar_t t_delta,
                      raster_depth_t depth1,
                      raster_depth_t depth_delta,
                      struct vector4 color1,
                      struct vector4 color_delta)
{
	struct raster_span *span = &(d->spans[d->num_spans]);
	span->y = y;
	span->left_x = left_x;
	span->x_delta = x_delta;
	span->left_depth = depth1 + left_t * depth_delta;
	span->depth_delta = t_delta * depth_delta;
	span->left_color = vector4_add(color1, vector4_mult_scalar(color_delta, left_t));
	span->color_delta = vector4_mult_scalar(color_delta, t_delta);
	++d->num_spans;
}

static void
raster_scan_horiz_line(struct drawable *d, const struct window_vertex *v1, const struct window_vertex *v2)
{
	assert(v1->coord.y == v2->coord.y);
	const struct window_vertex *left, *right;
	raster_loc_t left_x, right_x;
	if (v1->coord.x < v2->coord.x)
	{
		left = v1;
		left_x = (raster_loc_t)floor(v1->coord.x);
		right = v2;
		right_x = (raster_loc_t)ceil(v2->coord.x);
	}
	else if (v1->coord.x > v2->coord.x)
	{
		left = v2;
		left_x = (raster_loc_t)ceil(v2->coord.x);
		right = v1;
		right_x = (raster_loc_t)floor(v1->coord.x);
	}
	else
		return;

	struct raster_span *span = &(d->spans[d->num_spans]);
	span->y = (raster_loc_t)floor(v1->coord.y);
	span->left_x = left_x;
	span->left_depth = left->coord.z;
	span->x_delta = right_x - left_x;
	span->left_color = left->color;
	span->color_delta = vector4_sub(right->color, left->color);
	++d->num_spans;
}

static void
raster_scan_right_line(struct drawable *d,
                       const struct window_vertex *v1,
                       const struct window_vertex *v2,
                       scalar_t x_delta)
{
	assert(v1->coord.y == v2->coord.y);
	assert(v1->coord.x < v2->coord.x);

	raster_loc_t x1 = (raster_loc_t)floor(v1->coord.x);
	raster_loc_t x2 = (raster_loc_t)floor(v2->coord.x);
	raster_loc_t y = (raster_loc_t)floor(v1->coord.y);

	scalar_t x1_frac = v1->coord.x - x1;
	scalar_t y_frac = v1->coord.y - y;
	bool fill_start;
	scalar_t start_t;
	if (x1_frac > 0.5)
	{
		fill_start = (raster_point_part(x1_frac, y_frac) == DIAMOND);
		start_t = 0;
	}
	else
	{
		fill_start = true;
		start_t = (0.5 - x1_frac) / x_delta;
	}

	raster_loc_t start_x = x1;
	if (!fill_start)
		++start_x;

	scalar_t x2_frac = v2->coord.x - x2;
	raster_loc_t end_x = x2;
	if (x2_frac > 0.5 && raster_point_part(x2_frac, y_frac) != DIAMOND)
		++end_x;

	if (start_x < end_x)
	{
		raster_depth_t depth_delta = v2->coord.z - v1->coord.z;
		struct vector4 color_delta = vector4_sub(v2->color, v1->color);

		scalar_t end_t = ((x2 + 0.5) - v1->coord.x) / x_delta;
		raster_emit_span_para(d,
		                      y, x1, end_x - start_x,
		                      start_t, end_t - start_t,
		                      v1->coord.z, depth_delta,
		                      v1->color, color_delta);
	}
}

static void
raster_scan_vertical_line(struct drawable *d,
                          const struct window_vertex *v1, const struct window_vertex *v2,
                          scalar_t y_delta)
{
	assert(v1->coord.x == v2->coord.x);
	raster_loc_t x = floor(v1->coord.x);
	raster_loc_t y1 = floor(v1->coord.y);
	raster_loc_t y2 = floor(v2->coord.y);
	raster_dist_t y_step;
	if (y_delta < 0)
		y_step = -1;
	else
		y_step = 1;
	scalar_t depth_delta = v2->coord.z - v1->coord.z;
	for (raster_loc_t y = y1; y != y2; y+= y_step)
	{
		struct raster_span *span = &(d->spans[d->num_spans]);
		scalar_t t = (y - v1->coord.y) / y_delta;
		//assert(t >= 0 && t <= 1.0);
		span->y = y;
		span->left_x = x;
		span->left_depth = t * v1->coord.z + t * depth_delta;
		span->x_delta = 1;
		span->left_color = vector4_lerp(v1->color, v2->color, t);
		span->color_delta = (struct vector4){.v = {0, 0, 0, 0}};
		++d->num_spans;
	}
};

static void
raster_scan_up_line(struct drawable *d,
                          const struct window_vertex *v1, const struct window_vertex *v2,
                          scalar_t y_delta)
{
	assert(y_delta > 0);
	assert(v1->coord.x == v2->coord.x);

	raster_loc_t x = (raster_loc_t)floor(v1->coord.x);
	raster_loc_t y1 = (raster_loc_t)floor(v1->coord.y);
	raster_loc_t y2 = (raster_loc_t)floor(v2->coord.y);

	scalar_t x_frac = v1->coord.x - x;
	scalar_t y1_frac = v1->coord.y - y1;
	scalar_t y2_frac = v2->coord.y - y2;

	raster_loc_t start_y = y1;
	bool fill_start;
	scalar_t start_t;
	if (y1_frac > 0.5)
	{
		start_t = 0;
		fill_start = (raster_point_part(x_frac, y1_frac) == DIAMOND);
	}
	else
	{
		start_t = (0.5 - y1_frac) / y_delta;
		fill_start = true;
	}

	if (!fill_start)
		++start_y;

	raster_loc_t end_y = y2;
	if (y2_frac > 0.5)
	{
		if (raster_point_part(x_frac, y2_frac) != DIAMOND)
			++end_y;
	}

	if (start_y < end_y)
	{
		scalar_t depth_delta = v2->coord.z - v1->coord.z;
		struct vector4 color_delta = vector4_sub(v2->color, v1->color);

		if (fill_start)
		{
			raster_emit_span_para(d, start_y, x, 1, start_t, 0, v1->coord.z, depth_delta, v1->color, color_delta);
			++start_y;
		}

		for (raster_loc_t y = start_y; y != end_y; ++y)
		{
			struct raster_span *span = &(d->spans[d->num_spans]);
			scalar_t t = (y - v1->coord.y) / y_delta;
			//assert(t >= 0 && t <= 1.0);
			span->y = y;
			span->left_x = x;
			span->left_depth = t * v1->coord.z + t * depth_delta;
			span->x_delta = 1;
			span->left_color = vector4_add(v1->color, vector4_mult_scalar(color_delta, t));
			span->color_delta = (struct vector4) {.v = {0, 0, 0, 0}};
			++d->num_spans;
		}
	}
};
static void
raster_scan_gradual_line(struct drawable *d,
                         const struct window_vertex *v1, const struct window_vertex *v2,
                         scalar_t x_delta)
{
	scalar_t y_delta = v2->coord.y - v1->coord.y;
	assert(fabs(y_delta) <= fabs(x_delta));
	const raster_loc_t x1 = ceil(v1->coord.x - 0.5);
	const raster_loc_t x2 = ceil(v2->coord.x - 0.5);
	raster_dist_t x_step;
	if (x_delta > 0)
		x_step = 1;
	else
		x_step = -1;
	scalar_t depth_delta = v2->coord.z - v1->coord.z;
	struct vector4 color_delta = vector4_sub(v2->color, v1->color);

	raster_loc_t next_x = x1;
	scalar_t next_t = ((next_x + 0.5) - v1->coord.x) / x_delta;
	assert(next_t >= 0 && next_t <= 1.0);
	raster_loc_t next_y = floor(v1->coord.y + next_t * y_delta);

	while (next_x != x2)
	{
		raster_loc_t scan_x1 = next_x;
		scalar_t scan_t1 = next_t;
		raster_loc_t scan_y = next_y;

		raster_loc_t scan_x2;
		scalar_t scan_t2;
		do
		{
			scan_x2 = next_x;
			scan_t2 = next_t;

			next_x+= x_step;
			if (next_x == x2)
				break;
			next_t = ((next_x + 0.5) - v1->coord.x) / x_delta;
			assert(next_t >= 0 && next_t <= 1.0);
			next_y = floor(v1->coord.y + next_t * y_delta);
		} while (next_y == scan_y);

		struct raster_span *span = &(d->spans[d->num_spans]);
		span->y = scan_y;
		if (scan_x1 <= scan_x2)
		{
			span->left_x = scan_x1;
			span->x_delta = (scan_x2 - scan_x1) + 1;
			span->left_depth = v1->coord.z + scan_t1 * depth_delta;
			raster_depth_t right_depth = v1->coord.z + scan_t2 * depth_delta;
			span->depth_delta = right_depth - span->left_depth;
			struct vector4 right_color = vector4_add(v1->color, vector4_mult_scalar(color_delta, scan_t2));
			span->left_color = vector4_add(v1->color, vector4_mult_scalar(color_delta, scan_t1));
			span->color_delta = vector4_sub(right_color, span->left_color);
		}
		else
		{
			span->left_x = scan_x2;
			span->x_delta = (scan_x1 - scan_x2) + 1;
			span->left_depth = v1->coord.z + scan_t2 * depth_delta;
			raster_depth_t right_depth = v1->coord.z + scan_t1 * depth_delta;
			span->depth_delta = right_depth - span->left_depth;
			struct vector4 right_color = vector4_add(v1->color, vector4_mult_scalar(color_delta, scan_t1));
			span->left_color = vector4_add(v1->color, vector4_mult_scalar(color_delta, scan_t2));
			span->color_delta = vector4_sub(right_color, span->left_color);
		}
		++d->num_spans;
	}
}

static void
raster_scan_steep_line(struct drawable *d,
                       const struct window_vertex *v1, const struct window_vertex *v2,
                       scalar_t y_delta)
{
#if RASTER_SCAN_LINE_INCR
	//struct raster_vertex v = v1;
	int delta_x = v2.coord.x - v1.coord.x;
	int delta_y = v2.coord.y - v1.coord.y;
	assert(v1.coord.y + delta_y == v2.coord.y);
	assert(abs(delta_x) <= abs(delta_y));
	int x_incr;
	if (delta_x > 0)
		x_incr = 1;
	else
	{
		x_incr = -1;
		delta_x = -delta_x;
	}
	int actual_delta_y = delta_y;
	int y_incr;
	if (delta_y > 0)
		y_incr = 1;
	else
	{
		y_incr = -1;
		delta_y = -delta_y;
	}
	int decide = 2 * delta_x - delta_y;
	int incr_up = 2 * delta_x;
	int incr_diag = 2 * (delta_x - delta_y);
	raster_pixel(d, options, v);
	while (v.coord.y != v2.coord.y)
	{
		if (decide <= 0)
			decide+= incr_up;
		else
		{
			decide += incr_diag;
			v.coord.x += x_incr;
		}
		v.coord.y+= y_incr;
		GLdouble u = ((GLdouble)v.coord.y - v1.coord.y) / actual_delta_y;
		v.color = vector4_lerp(v1.color, v2.color, u);
		raster_pixel(d, options, v);
	}
#else
	scalar_t x_delta = v2->coord.x - v1->coord.x;
	assert(fabs(x_delta) <= fabs(y_delta));
	const raster_loc_t y1 = floor(v1->coord.y);
	const raster_loc_t y2 = floor(v2->coord.y);

	raster_dist_t y_step;
	if (y_delta > 0)
		y_step = 1;
	else
		y_step = -1;
	scalar_t depth_delta = v2->coord.z - v1->coord.z;
	struct vector4 color_delta = vector4_sub(v2->color, v1->color);

	for (raster_loc_t y = y1; y != y2; y+= y_step)
	{
		scalar_t t = (y - v1->coord.y) / y_delta;
		assert(t >= 0 && t <= 1.0);
		raster_loc_t x = floor(v1->coord.x + t * x_delta);

		struct raster_span *span = &(d->spans[d->num_spans]);
		span->y = y;
		span->left_x = x;
		span->x_delta = 1;
		span->left_depth = v1->coord.z + t * depth_delta;
		span->depth_delta = 0;
		span->left_color = vector4_add(v1->color, vector4_mult_scalar(color_delta, t));
		span->color_delta = (struct vector4){.r = 0, .g = 0, .b = 0};
		++d->num_spans;
	}
#endif // RASTER_SCAN_LINE_INCR
}

static void
raster_scan_octant0_line(struct drawable *d,
                               const struct window_vertex *v1, const struct window_vertex *v2,
                               scalar_t x_delta, scalar_t y_delta)
{
	assert(x_delta > 0);
	assert(y_delta > 0);
	assert(x_delta > y_delta);

	const raster_loc_t x1 = (raster_loc_t)floor(v1->coord.x);
	const raster_loc_t y1 = (raster_loc_t)floor(v1->coord.y);
	const raster_loc_t x2 = (raster_loc_t)floor(v2->coord.x);
	const raster_loc_t y2 = (raster_loc_t)floor(v2->coord.y);

	scalar_t x1_frac = v1->coord.x - x1;
	scalar_t y1_frac = v1->coord.y - y1;

	bool fill_start;
	scalar_t start_t;
	if (x1_frac < 0.5)
	{
		start_t = (0.5 - x1_frac) / x_delta;
		scalar_t start_y = v1->coord.y + start_t * y_delta;
		fill_start = (floor(start_y) == y1);
	}
	else if (x1_frac > 0.5)
	{
		start_t = 0;
		fill_start = (raster_point_part(x1_frac, y1_frac) == DIAMOND);
	}
	else
	{
		start_t = 0;
		fill_start = true;
	}

	scalar_t x2_frac = v2->coord.x - x2;
	raster_loc_t end_x = x2;
	if (x2_frac > 0.5)
	{
		scalar_t y2_frac = v2->coord.y - y2;
		if (raster_point_part(x2_frac, y2_frac) != DIAMOND)
			++end_x;
	}

	raster_loc_t next_x;
	scalar_t next_t;
	raster_loc_t next_y;
	if (fill_start)
	{
		next_x = x1;
		next_t = start_t;
		next_y = y1;
	}
	else
	{
		next_x = x1 + 1;
		next_t = ((next_x + 0.5) - v1->coord.x) / x_delta;
		assert(next_t >= 0 && next_t <= 1.0);
		next_y = (raster_loc_t)floor(v1->coord.y + next_t * y_delta);
	}

	scalar_t depth_delta = v2->coord.z - v1->coord.z;
	struct vector4 color_delta = vector4_sub(v2->color, v1->color);

	while (next_x != end_x)
	{
		raster_loc_t scan_x1 = next_x;
		scalar_t scan_t1 = next_t;
		raster_loc_t scan_y = next_y;

		raster_loc_t scan_x2;
		scalar_t scan_t2;
		do
		{
			scan_x2 = next_x;
			scan_t2 = next_t;

			if (++next_x == end_x)
				break;

			next_t = ((next_x + 0.5) - v1->coord.x) / x_delta;
			assert(next_t >= 0 && next_t <= 1.0);
			next_y = floor(v1->coord.y + next_t * y_delta);
		} while (next_y == scan_y);

		raster_depth_t left_depth = v1->coord.z + scan_t1 * depth_delta;
		raster_depth_t right_depth = v1->coord.z + scan_t2 * depth_delta;
		raster_emit_span_diff(d,
		                      scan_y, scan_x1, scan_x2,
		                      left_depth, right_depth,
		                      vector4_add(v1->color, vector4_mult_scalar(color_delta, scan_t1)),
		                      vector4_add(v1->color, vector4_mult_scalar(color_delta, scan_t2)));
	}
}

static void
raster_scan_octant1_line(struct drawable *d,
                         const struct window_vertex *v1, const struct window_vertex *v2,
                         scalar_t x_delta, scalar_t y_delta)
{
	assert(x_delta > 0);
	assert(y_delta > 0);
	assert(x_delta <= y_delta);

	const raster_loc_t x1 = (raster_loc_t)floor(v1->coord.x);
	const raster_loc_t y1 = (raster_loc_t)floor(v1->coord.y);
	const raster_loc_t x2 = (raster_loc_t)floor(v2->coord.x);
	const raster_loc_t y2 = (raster_loc_t)floor(v2->coord.y);

	scalar_t x1_frac = v1->coord.x - x1;
	scalar_t y1_frac = v1->coord.y - y1;

	bool fill_start;
	scalar_t start_t;
	if (y1_frac < 0.5)
	{
		start_t = (0.5 - y1_frac) / y_delta;
		scalar_t start_x = v1->coord.x + start_t * x_delta;
		fill_start = (floor(start_x) == x1);
	}
	else if (y1_frac > 0.5)
	{
		start_t = 0;
		fill_start = (raster_point_part(x1_frac, y1_frac) == DIAMOND);
	}
	else
	{
		start_t = 0;
		fill_start = true;
	}

	raster_loc_t start_y = y1;
	if (!fill_start)
		++start_y;

	scalar_t y2_frac = v2->coord.y - y2;
	raster_loc_t end_y = y2;
	if (y2_frac > 0.5)
	{
		scalar_t x2_frac = v2->coord.y - x2;
		if (raster_point_part(x2_frac, y2_frac) != DIAMOND)
			++end_y;
	}

	if (start_y < end_y)
	{
		scalar_t depth_delta = v2->coord.z - v1->coord.z;
		struct vector4 color_delta = vector4_sub(v2->color, v1->color);

		if (fill_start)
		{
			raster_emit_span_para(d, start_y, x1, 1, start_t, 0, v1->coord.z, depth_delta, v1->color, color_delta);
			++start_y;
		}

		for (raster_loc_t y = start_y; y != end_y; ++y)
		{
			scalar_t t = (y - v1->coord.y) / y_delta;
			assert(t >= 0 && t <= 1.0);
			raster_loc_t x = floor(v1->coord.x + t * x_delta);

			struct raster_span *span = &(d->spans[d->num_spans]);
			span->y = y;
			span->left_x = x;
			span->x_delta = 1;
			span->left_depth = v1->coord.z + t * depth_delta;
			span->depth_delta = 0;
			span->left_color = vector4_add(v1->color, vector4_mult_scalar(color_delta, t));
			span->color_delta = (struct vector4) {.r = 0, .g = 0, .b = 0};
			++d->num_spans;
		}
	}
}

void
raster_scan_line(struct drawable *d, const struct window_vertex *v1, const struct window_vertex *v2)
{
	scalar_t x_delta = v2->coord.x - v1->coord.x;
	scalar_t y_delta = v2->coord.y - v1->coord.y;
	if (x_delta > 0)
	{
		if (y_delta > 0)
		{
			if (x_delta > y_delta)
				raster_scan_octant0_line(d, v1, v2, x_delta, y_delta);
			else
				raster_scan_octant1_line(d, v1, v2, x_delta, y_delta);
		}
		else if (y_delta < 0)
			;
		else
			raster_scan_right_line(d, v1, v2, x_delta);
	}
	else if (x_delta < 0)
		;
	else
	{
		if (y_delta > 0)
			raster_scan_up_line(d, v1, v2, y_delta);
	}
}

struct raster_edge
{
	struct vector3 start;
	struct vector3 dir;
	struct vector4 color_start;
	struct vector4 color_delta;
};

void
raster_scan_trapezoid(struct drawable *d, const struct raster_edge *left_edge, const struct raster_edge *right_edge)
{
	//assert(left_edge->start.x <= right_edge->start.x);
	//assert((left_edge->start.x + left_edge->dir.x) - (right_edge->start.x + right_edge->dir.x) <= 0.000000001);
	assert(fabs(left_edge->start.y - right_edge->start.y) <= 0.0000000001);
	assert(left_edge->dir.y > 0);
	assert(right_edge->dir.y > 0);
	assert(fabs(left_edge->dir.y - right_edge->dir.y) <= 0.0000000001);
	raster_loc_t min_y = lround(left_edge->start.y);
	raster_loc_t max_y = lround(left_edge->start.y + left_edge->dir.y);
	for (raster_loc_t y = min_y; y < max_y; ++y)
	{
		scalar_t t = ((y + 0.5) - left_edge->start.y) / left_edge->dir.y;
		assert(t >= 0 && t <= 1.0);
		scalar_t left_x = left_edge->start.x + left_edge->dir.x * t;
		raster_loc_t left_raster_x = ceil(left_x - 0.5);
		scalar_t right_x = right_edge->start.x + right_edge->dir.x * t;
		raster_loc_t right_raster_x = ceil(right_x - 0.5);
		raster_dist_t x_delta = right_raster_x - left_raster_x;
		if (x_delta > 0)
		{
			struct vector4 left_color =
					vector4_add(left_edge->color_start, vector4_mult_scalar(left_edge->color_delta, t));
			struct vector4 right_color =
					vector4_add(right_edge->color_start, vector4_mult_scalar(right_edge->color_delta, t));
			scalar_t left_depth = left_edge->start.z + left_edge->dir.z * t;
			scalar_t right_depth = right_edge->start.z + right_edge->dir.z * t;
			d->spans[d->num_spans].y = y;
			d->spans[d->num_spans].left_x = left_raster_x;
			d->spans[d->num_spans].x_delta = x_delta;
			d->spans[d->num_spans].left_depth = left_depth;
			d->spans[d->num_spans].depth_delta = right_depth - left_depth;
			d->spans[d->num_spans].left_color = left_color;
			d->spans[d->num_spans].color_delta = vector4_sub(right_color, left_color);
			++d->num_spans;
		}
	}
}

static struct raster_edge
raster_make_edge(const struct window_vertex *start, const struct window_vertex *end)
{
	assert(start->coord.y <= end->coord.y);
	struct raster_edge edge;
	edge.start = start->coord;
	edge.dir = vector3_sub(end->coord, start->coord);
	edge.color_start = start->color;
	edge.color_delta = vector4_sub(end->color, start->color);
	return edge;
}

static void
raster_split_edge(struct raster_edge edges[2], scalar_t y)
{
	scalar_t t = (y - edges[0].start.y) / edges[0].dir.y;
	assert(t > 0 && t < 1.0);
	edges[0].start = edges[0].start;
	struct vector3 mid_dir = vector3_mult_scalar(edges[0].dir, t);
	edges[1].start = vector3_add(edges[0].start, mid_dir);
	edges[1].dir = vector3_sub(edges[0].dir, mid_dir);
	struct vector4 mid_color_delta = vector4_mult_scalar(edges[0].color_delta, t);
	edges[1].color_start = vector4_add(edges[0].color_start, mid_color_delta);
	edges[1].color_delta = vector4_sub(edges[0].color_delta, mid_color_delta);
	edges[0].dir = mid_dir;
	edges[0].color_delta = mid_color_delta;
}

void
raster_scan_triangle(struct drawable *d, const struct window_vertex vertices[3])
{
	struct raster_edge left_edges[2];
	u_int num_left_edges = 0;
	struct raster_edge right_edges[2];
	u_int num_right_edges = 0;

	bool flat = false;
	for (u_int i = 0; i < 3; ++i)
	{
		const struct window_vertex *start = &(vertices[i]);
		const struct window_vertex *end = &(vertices[(i + 1) % 3]);
		if (start->coord.y > end->coord.y)
			left_edges[num_left_edges++] = raster_make_edge(end, start);
		else if (start->coord.y < end->coord.y)
			right_edges[num_right_edges++] = raster_make_edge(start, end);
		else
			flat = true;
	}

	struct raster_edge *top_left, *bottom_left, *top_right, *bottom_right;
	if (num_left_edges == 2)
	{
		assert(num_right_edges == 1);
		if (left_edges[0].start.y < left_edges[1].start.y)
		{
			top_left = &(left_edges[0]);
			bottom_left = &(left_edges[1]);
		}
		else
		{
			top_left = &(left_edges[1]);
			bottom_left = &(left_edges[0]);
		}
		raster_split_edge(right_edges, bottom_left->start.y);
		top_right = &(right_edges[0]);
		bottom_right = &(right_edges[1]);
	}
	else if (num_right_edges == 2)
	{
		assert(num_left_edges == 1);
		if (right_edges[0].start.y < right_edges[1].start.y)
		{
			top_right = &(right_edges[0]);
			bottom_right = &(right_edges[1]);
		}
		else
		{
			top_right = &(right_edges[1]);
			bottom_right = &(right_edges[0]);
		}
		raster_split_edge(left_edges, bottom_right->start.y);
		top_left = &(left_edges[0]);
		bottom_left = &(left_edges[1]);
	}
	else
		assert(flat);

	if (!flat)
	{
		raster_scan_trapezoid(d, top_left, top_right);
		raster_scan_trapezoid(d, bottom_left, bottom_right);
	}
	else
		raster_scan_trapezoid(d, &(left_edges[0]), &(right_edges[0]));
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
			scalar_t u = (scalar_t)off / d->spans[i].x_delta;
			rvert.coord.depth = d->spans[i].left_depth + u * d->spans[i].depth_delta;
			rvert.color = vector4_add(d->spans[i].left_color, vector4_mult_scalar(d->spans[i].color_delta, u));
			raster_pixel(d, options, rvert);
			rvert.coord.x+= 1;
		}
	}
}
