//
// Created by Seth Kingsley on 1/12/18.
//

#include <stdlib.h>
#include <err.h>
#include <assert.h>
#include <math.h>
#include "draw.h"
#include "window.h"
#include "raster.h"

#define FLIPPED_Y 0
#define CLAMP_COLORS 0

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
		free(d->edge_table);
	}
	d->window_width = width;
	d->window_height = height;
	assert(sizeof(*(d->color_buffer)) == 4);
	d->color_buffer = malloc(sizeof(*(d->color_buffer)) * d->window_width * d->window_height);
	d->depth_buffer = malloc(sizeof(*(d->depth_buffer)) * d->window_width * d->window_height);
	d->edge_table = calloc(d->window_height, sizeof(*(d->edge_table)));
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
}

void
draw_clear(struct drawable *d, bool color, const struct vector4 clear_color, bool depth, GLfloat clear_depth)
{
	struct raster_vertex vertex;
	vertex.depth = clear_depth;
	vertex.color = clear_color;
	for (vertex.y = 0; vertex.y < d->window_height; ++vertex.y)
		for (vertex.x = 0; vertex.x < d->window_width; ++vertex.x)
		{
			struct draw_options options;
			options.draw_op = (color) ? GL_COPY : GL_NOOP;
			options.test_depth = depth;
			options.depth_func = GL_ALWAYS;
			raster_pixel(d, options, vertex);
		}
}

static void
draw_line(struct drawable *d,
          struct draw_options options,
          struct device_vertex p1,
          struct device_vertex p2)
{
	GLdouble delta_x = p2.coord.x - p1.coord.x;
	GLdouble delta_y = p2.coord.y - p1.coord.y;
	if (delta_y == 0)
	{
		if (delta_x > 0)
			raster_horiz_line(d, options, p1, p2);
		else
			raster_horiz_line(d, options, p2, p1);
	}
	else if (delta_x == 0)
		raster_vertical_line(d, options, p1, p2);
	else
	{
		if (fabs(delta_y) >= fabs(delta_x))
		{
			if (delta_y > 0)
				raster_steep_line(d, options, p1, p2);
			else
				raster_steep_line(d, options, p2, p1);
		}
		else
		{
			if (delta_x > 0)
				raster_gradual_line(d, options, p1, p2);
			else
				raster_gradual_line(d, options, p2, p1);
		}
	}
}

struct vertical_line_stepper
{
	struct raster_vertex vertex;
	int delta_x;
	u_int delta_y;
	u_int x_num;
	float depth_incr;
	struct vector4 color_incr;
};

struct vertical_line_stepper
draw_create_stepper(struct raster_vertex bottom, struct raster_vertex top)
{
	assert(bottom.y < top.y);
	struct vertical_line_stepper stepper;
	stepper.vertex = bottom;
	stepper.delta_x = top.x - bottom.x;
	stepper.delta_y = top.y - bottom.y;
	stepper.x_num = 0;
	stepper.depth_incr = (top.depth - bottom.depth) / stepper.delta_y;
	stepper.color_incr = vector4_divide_scalar(vector4_sub(top.color, bottom.color), stepper.delta_y);
	return stepper;
}

/*
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

static int
draw_compare_vertices(const struct raster_vertex *a, const struct raster_vertex *b)
{
	int delta_y = (int)a->y - b->y;
	if (delta_y != 0)
		return delta_y;
	return (int)a->x - b->x;
}
 */

/*
static void
draw_triangle(struct drawable *d, struct draw_options options, struct raster_vertex vertices[])
{
	qsort(vertices, 3, sizeof(vertices[0]), (int (*)(const void *, const void *))draw_compare_vertices);
	assert(vertices[0].y <= vertices[1].y && vertices[1].y <= vertices[2].y);

	if (vertices[0].y == vertices[1].y)
		draw_trapezoid(d, options, vertices[0], vertices[1], vertices[2], vertices[2]);
	else if (vertices[1].y == vertices[2].y)
		draw_trapezoid(d, options, vertices[0], vertices[0], vertices[1], vertices[2]);
	else
	{
		//if (vertices[0].x < vertices[1].x)
		{
			GLdouble mid = (GLdouble)(vertices[1].y - vertices[0].y) / (vertices[2].y - vertices[0].y);
			struct raster_vertex mid_vertex;
			//assert(vertices[2].x <= vertices[0].x);
			mid_vertex.x = vertices[0].x + mid * ((GLdouble)vertices[2].x - vertices[0].x);
			mid_vertex.y = vertices[1].y;
			mid_vertex.depth = vertices[0].depth + mid * (vertices[2].depth - vertices[0].depth);
			mid_vertex.color = vector4_lerp(vertices[0].color, vertices[2].color, mid);
			if (mid_vertex.x < vertices[1].x)
			{
				draw_trapezoid(d, options, vertices[0], vertices[0], mid_vertex, vertices[1]);
				draw_trapezoid(d, options, mid_vertex, vertices[1], vertices[2], vertices[2]);
			}
			else
			{
				draw_trapezoid(d, options, vertices[0], vertices[0], vertices[1], mid_vertex);
				draw_trapezoid(d, options, vertices[1], mid_vertex, vertices[2], vertices[2]);
			}
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
	/*
	struct raster_vertex raster_vertices[MAX_PRIMITIVE_VERTICES];
	for (u_int i = 0; i < num_verts; ++i)
	{
		assert(vertices[i].x >= -1.0 && vertices[i].x <= 1.0);
		raster_vertices[i].x = d->view_x + lround(((vertices[i].x + 1.0) / 2.0) * d->view_width);
		assert(vertices[i].y >= -1.0 && vertices[i].x <= 1.0);
#if FLIPPED_Y
		raster_vertices[i].y = (d->view_y + d->view_height) -  lround(((vertices[i].y + 1.0) / 2.0) * d->view_height);
#else
		raster_vertices[i].y = d->view_y + lround(((vertices[i].y + 1.0) / 2.0) * d->view_height);
#endif // FLIPPED_Y
		assert(vertices[i].z >= -1.0 && vertices[i].z <= 1.0);
		raster_vertices[i].depth = (vertices[i].z + 1) / 2.0;
		assert(colors[i].r >= 0 && colors[i].r <= 1.0);
		assert(colors[i].g >= 0 && colors[i].g <= 1.0);
		assert(colors[i].b >= 0 && colors[i].b <= 1.0);
		assert(colors[i].a >= 0 && colors[i].a <= 1.0);
		raster_vertices[i].color = colors[i];
	}
	 */

	switch (num_verts)
	{
		/*
		case 1:
			draw_check_vertex(d, raster_vertices[0]);
			draw_pixel(d, options, raster_vertices[0]);
			break;
		 */
		case 2:
			draw_line(d, options, vertices[0], vertices[1]);
			break;
		default:
			switch (options.polygon_mode)
			{
				/*
				case GL_POINT:
					for (GLuint i = 0; i < num_verts; ++i)
						draw_pixel(d, options, raster_vertices[i]);
					break;
				 */

				case GL_LINE:
					for (GLuint i = 0; i < num_verts; ++i)
						draw_line(d, options, vertices[i], vertices[(i + 1) % num_verts]);
					break;

				case GL_FILL:
					/*
					if (num_verts == 3)
						raster_triangle(d, options, vertices);
					else
					 */
						raster_polygon(d, options, vertices, num_verts);
					break;
			}
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
	window_update(d->window, d->color_buffer, 0, 0, d->window_width, d->window_height, FLIPPED_Y);
}

void
draw_flip(struct drawable *d)
{
	window_flip(d->window);
}
