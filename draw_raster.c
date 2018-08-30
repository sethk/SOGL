//
// Created by Seth Kingsley on 1/12/18.
//

#include <OpenGL/gl.h>
#include <stdlib.h>
#include <err.h>
#include <assert.h>
#include <math.h>
#include <memory.h>
#include "draw.h"
#include "window.h"
#include "raster.h"
#include "clip.h"
#include "matrix.h"

#define FLIPPED_Y 0

struct drawable *
draw_create(struct window *w)
{
	struct drawable *d = calloc(1, sizeof(*d));
	if (!d)
		err(1, "Alloc raster drawable");
	d->window = w;
	return d;
}

void
draw_reshape(struct drawable *d, u_int width, u_int height)
{
	if (d->color_buffer)
	{
		free(d->color_buffer);
		free(d->depth_buffer);
		//free(d->edge_table);
		free(d->spans);
	}
	d->window_width = width;
	d->window_height = height;
	assert(sizeof(*(d->color_buffer)) == 4);
	d->color_buffer = malloc(sizeof(*(d->color_buffer)) * d->window_width * d->window_height);
	d->depth_buffer = malloc(sizeof(*(d->depth_buffer)) * d->window_width * d->window_height);
	//d->edge_table = calloc(d->window_height, sizeof(*(d->edge_table)));
	d->spans = calloc(d->window_height, sizeof(*(d->spans)));
}

void
draw_set_view(struct drawable *d, u_int x, u_int y, u_int width, u_int height)
{
	d->view_x = x;
#if FLIPPED_Y
	d->view_y = d->window_height - (height + y);
#else
	d->view_y = y;
#endif // FLIPPED_Y
	d->view_width = width;
	d->view_height = height;
	scalar_t half_width = width / 2.0, half_height = height / 2.0;
	d->view_trans = (struct matrix4x4)
	{
		.cols = {{half_width, 0, 0}, {0, half_height, 0}, {0, 0, 0.5, 0}, {x + half_width, y + half_height, 0.5, 1}}
	};
}

void
draw_clear(struct drawable *d, bool color, const struct vector4 clear_color, bool depth, GLfloat clear_depth)
{
	if (depth)
	{
		raster_depth_t depth = clear_depth;
		static_assert(sizeof(depth) == 4, "sizeof(depth) must be 4");
		memset_pattern4(d->depth_buffer, &depth, sizeof(*(d->depth_buffer)) * d->window_width * d->window_height);
	}

	if (color)
	{
		struct raster_color pixel;
		raster_write_pixel(&clear_color, &pixel);
		static_assert(sizeof(pixel) == 4, "sizeof(pixel) must be 4");
		memset_pattern4(d->color_buffer, &pixel, sizeof(*(d->color_buffer)) * d->window_width * d->window_height);
	}
}

static struct window_vertex
draw_map_vertex(struct drawable *d, const struct device_vertex *vertex)
{
	struct window_vertex win_vert;
	win_vert.coord = vector4_project(matrix4x4_mult_vector4(d->view_trans, vertex->coord));
	win_vert.color = vertex->color;
	return win_vert;
}

static void
draw_point(struct drawable *d, struct draw_options options, const struct device_vertex *vertex)
{
	if (clip_point(vertex))
	{
		d->num_spans = 0;
		struct window_vertex win_vert = draw_map_vertex(d, vertex);
		raster_scan_point(d, &win_vert);
		raster_fill_spans(d, options);
	}
}

static void
draw_line(struct drawable *d,
          struct draw_options options,
          struct device_vertex p1,
          struct device_vertex p2)
{
	if (!clip_line(&p1, &p2))
		return;

	struct window_vertex win_vert1 = draw_map_vertex(d, &p1);
	struct window_vertex win_vert2 = draw_map_vertex(d, &p2);

	d->num_spans = 0;
	raster_scan_line(d, &win_vert1, &win_vert2);
	raster_fill_spans(d, options);
}

static void
draw_triangle(struct drawable *d,
              struct draw_options options,
              const struct device_vertex vertices[3],
              bool front)
{
	if (!clip_point(&(vertices[0])) ||
			!clip_point(&(vertices[1])) ||
			!clip_point(&(vertices[2])))
		return;

	struct window_vertex win_verts[3];
	if (front)
		for (u_int i = 0; i < 3; ++i)
			win_verts[i] = draw_map_vertex(d, &vertices[i]);
	else
		for (u_int i = 0; i < 3; ++i)
			win_verts[2 - i] = draw_map_vertex(d, &vertices[i]);

	d->num_spans = 0;
	raster_scan_triangle(d, win_verts);
	raster_fill_spans(d, options);
}

static void
draw_polygon(struct drawable *d, struct draw_options options, const struct device_vertex vertices[], u_int num_verts)
{
	GLenum polygon_mode;
	bool front;
	struct vector2 norm_device_coords[3];
	for (u_int i = 0; i < 3; ++i)
		norm_device_coords[i] = vector2_divide_scalar(vertices[i].coord.xy, vertices[i].coord.w);
	struct vector2 u = vector2_sub(norm_device_coords[1], norm_device_coords[0]);
	struct vector2 v = vector2_sub(norm_device_coords[2], norm_device_coords[0]);
	struct matrix2x2 tri_space = matrix2x2_build(u, v);
	scalar_t det = matrix2x2_det(tri_space);
	if (det > 0)
	{
		if (options.cull_faces && options.faces_culled[0])
			return;

		front = true;
		polygon_mode = options.polygon_modes[0];
	}
	else if (det < 0)
	{
		if (options.cull_faces && options.faces_culled[1])
			return;

		front = false;
		polygon_mode = options.polygon_modes[1];
	}
	else
		return;

	switch (polygon_mode)
	{
		case GL_POINT:
			for (GLuint i = 0; i < num_verts; ++i)
				draw_point(d, options, &(vertices[i]));
			break;

		case GL_LINE:
			for (GLuint i = 0; i < num_verts; ++i)
				draw_line(d, options, vertices[i], vertices[(i + 1) % num_verts]);
			break;

		case GL_FILL:
		{
			struct device_vertex tri_verts[3];
			tri_verts[0] = vertices[0];
			for (GLuint i = 1; i < num_verts - 1; ++i)
			{
				tri_verts[1] = vertices[i];
				tri_verts[2] = vertices[i + 1];
				draw_triangle(d, options, tri_verts, front);
			}
			break;
		}
	}
}

/*
struct vertical_line_stepper
{
	struct raster_vertex vertex;
	raster_dist_t delta_x;
	raster_dist_t delta_y;
	u_int x_num;
	scalar_t depth_incr;
	struct vector4 color_incr;
};

struct vertical_line_stepper
draw_create_stepper(struct raster_vertex bottom, struct raster_vertex top)
{
	assert(bottom.coord.y < top.coord.y);
	struct vertical_line_stepper stepper;
	stepper.vertex = bottom;
	stepper.delta_x = top.coord.x - bottom.coord.x;
	stepper.delta_y = top.coord.y - bottom.coord.y;
	stepper.x_num = 0;
	stepper.depth_incr = (top.coord.depth - bottom.coord.depth) / stepper.delta_y;
	stepper.color_incr = vector4_divide_scalar(vector4_sub(top.color, bottom.color), stepper.delta_y);
	return stepper;
}

static void
draw_stepper_incr(struct vertical_line_stepper *stepper)
{
	if (stepper->delta_x > 0)
	{
		stepper->x_num+= stepper->delta_x;
		while (stepper->x_num >= stepper->delta_y)
		{
			++stepper->vertex.x;
			stepper->x_num-= stepper->delta_y;
		}
	}
	else if (stepper->delta_x < 0)
	{
		stepper->x_num-= stepper->delta_x;
		while (stepper->x_num >= stepper->delta_y)
		{
			--stepper->vertex.x;
			stepper->x_num-= stepper->delta_y;
		}
	}
	++stepper->vertex.y;
	stepper->vertex.depth+= stepper->depth_incr;
	stepper->vertex.color = vector4_add(stepper->vertex.color, stepper->color_incr);
}

static void
draw_trapezoid(struct drawable *d,
               struct draw_options options,
               struct raster_vertex bottom_left,
               struct raster_vertex bottom_right,
               struct raster_vertex top_left,
               struct raster_vertex top_right)
{
	assert(top_left.y == top_right.y);
	assert(bottom_left.y == bottom_right.y);
	assert(top_left.x <= top_right.x);
	assert(bottom_left.x <= bottom_right.x);
	if (bottom_left.y < top_left.y)
	{
		struct vertical_line_stepper left_stepper = draw_create_stepper(bottom_left, top_left);
		struct vertical_line_stepper right_stepper = draw_create_stepper(bottom_right, top_right);

		while (left_stepper.vertex.y != top_left.y)
		{
			//draw_horiz_line(d, options, left_stepper.vertex, right_stepper.vertex);
			draw_stepper_incr(&left_stepper);
			draw_stepper_incr(&right_stepper);
		}
	}
}
 */

void
draw_primitive(struct drawable *d,
               struct draw_options options,
               const struct device_vertex vertices[],
               u_int num_verts)
{
	switch (num_verts)
	{
		case 1:
			draw_point(d, options, &(vertices[0]));
			break;
		case 2:
			draw_line(d, options, vertices[0], vertices[1]);
			break;
		default:
			draw_polygon(d, options, vertices, num_verts);
			break;
	}
}

void
draw_flush(struct drawable *d)
{
	window_update(d->window, d->color_buffer, 0, 0, d->window_width, d->window_height, FLIPPED_Y);
}

void
draw_finish(struct drawable *d)
{
}

void
draw_flip(struct drawable *d)
{
	draw_flush(d);
	window_flip(d->window);
}
