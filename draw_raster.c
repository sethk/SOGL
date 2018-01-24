//
// Created by Seth Kingsley on 1/12/18.
//

#include <stdlib.h>
#include <err.h>
#include <assert.h>
#include <math.h>
#include "draw.h"
#include "window.h"

struct raster_vertex
{
	u_int x, y;
	GLfloat depth;
	struct vector4 color;
};

struct polygon_edge
{
	struct raster_vertex upper_coord;
	struct raster_vertex lower_coord;
	int delta_x;
	u_int delta_y;
	u_int x_num;
	float depth_incr;
	struct polygon_edge *next;
};

struct drawable
{
	struct window *window;
	u_int window_width, window_height;
	u_int view_x, view_y, view_width, view_height;
	struct raster_color *color_buffer;
	GLfloat *depth_buffer;
	struct polygon_edge **edge_table;
};

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
	d->view_y = y;
	d->view_width = width;
	d->view_height = height;
}

static void
draw_pixel(struct drawable *d, struct draw_options options, struct raster_vertex vertex)
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
				if (d->depth_buffer[vertex.y * d->window_width + vertex.x] < vertex.depth)
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
			//assert(vertex.color.r >= 0 && vertex.color.r <= 1.0);
			//assert(vertex.color.g >= 0 && vertex.color.g <= 1.0);
			//assert(vertex.color.b >= 0 && vertex.color.b <= 1.0);
			pixel->red = round(fmax(fmin(vertex.color.r, 1.0), 0) * 255);
			pixel->green = round(fmax(fmin(vertex.color.g, 1.0), 0) * 255);
			pixel->blue = round(fmax(fmin(vertex.color.b, 1.0), 0) * 255);
			pixel->alpha = round(fmax(fmin(vertex.color.a, 1.0), 0) * 255);
			break;
		}

		case GL_NOOP:
			break;

		default:
			assert(!"Draw operation not implemented");
	}
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
			draw_pixel(d, options, vertex);
		}
}

static void
draw_horiz_line(struct drawable *d,
                struct draw_options options,
                struct raster_vertex v1,
                struct raster_vertex v2,
                int delta_x)
{
	struct raster_vertex v = v1;
	int x_step;
	if (delta_x > 0)
		x_step = 1;
	else
		x_step = -1;
	float depth_incr = (v2.depth - v1.depth) / delta_x;
	while (v.x != v2.x)
	{
		GLdouble u = ((GLdouble)v.x - v1.x) / delta_x;
		v.color = vector4_lerp(v1.color, v2.color, u);
		draw_pixel(d, options, v);
		v.x+= x_step;
		v.depth+= depth_incr;
	}
}

static void
draw_vertical_line(struct drawable *d,
                   struct draw_options options,
                   struct raster_vertex v1,
                   struct raster_vertex v2,
                   int delta_y)
{
	struct raster_vertex v = v1;
	int y_incr;
	if (delta_y > 0)
		y_incr = 1;
	else
		y_incr = -1;
	float depth_incr = (v2.depth - v1.depth) / delta_y;
	while (v.y != v2.y)
	{
		GLdouble u = ((GLdouble)v.y - v1.y) / delta_y;
		v.color = vector4_lerp(v1.color, v2.color, u);
		draw_pixel(d, options, v);
		v.y+= y_incr;
		v.depth+= depth_incr;
	}
}

static void
draw_gradual_line(struct drawable *d,
                  struct draw_options options,
                  struct raster_vertex v1,
                  struct raster_vertex v2,
                  //u_int x1,
                  //u_int x2,
                  //u_int y,
                  int delta_x,
                  int delta_y//,
                  //struct vector4 c1,
                  /*struct vector4 c2*/)
{
	struct raster_vertex v = v1;
	assert(v1.x + delta_x == v2.x);
	assert(abs(delta_y) < abs(delta_x));
	int x_incr;
	int actual_delta_x = delta_x;
	if (delta_x > 0)
		x_incr = 1;
	else
	{
		x_incr = -1;
		delta_x = -delta_x;
		//incr_across = -incr_across;
	}
	int y_incr;
	if (delta_y >= 0)
		y_incr = 1;
	else
	{
		y_incr = -1;
		delta_y = -delta_y;
	}
	float depth_incr = (v2.depth - v1.depth) / delta_x;
	int decide = 2 * delta_y - delta_x;
	int incr_across = 2 * delta_y;
	int incr_diag = 2 * (delta_y - delta_x);
	draw_pixel(d, options, v);
	while (v.x != v2.x)
	{
		if (decide <= 0)
			decide+= incr_across;
		else
		{
			decide+= incr_diag;
			v.y+= y_incr;
		}
		v.x+= x_incr;
		v.depth+= depth_incr;
		GLdouble u = ((GLdouble)v.x - v1.x) / actual_delta_x;
		v.color = vector4_lerp(v1.color, v2.color, u);
		draw_pixel(d, options, v);
	}
}

static void
draw_steep_line(struct drawable *d,
                struct draw_options options,
                struct raster_vertex v1,
                struct raster_vertex v2,
                //u_int x,
                //u_int y1,
                //u_int y2,
                int delta_x,
                int delta_y//,
                //struct vector4 c1,
                /*struct vector4 c2*/)
{
	struct raster_vertex v = v1;
	assert(v1.y + delta_y == v2.y);
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
	draw_pixel(d, options, v);
	while (v.y != v2.y)
	{
		if (decide <= 0)
			decide+= incr_up;
		else
		{
			decide += incr_diag;
			v.x += x_incr;
		}
		v.y+= y_incr;
		GLdouble u = ((GLdouble)v.y - v1.y) / actual_delta_y;
		v.color = vector4_lerp(v1.color, v2.color, u);
		draw_pixel(d, options, v);
	}
}

static void
draw_line(struct drawable *d,
          struct draw_options options,
          struct raster_vertex p1,
          struct raster_vertex p2)
{
	int delta_x = p2.x - p1.x;
	int delta_y = p2.y - p1.y;
	if (delta_y == 0)
		draw_horiz_line(d, options, p1, p2, delta_x);
	else if (delta_x == 0)
		draw_vertical_line(d, options, p1, p2, delta_y);
	else
	{
		if (abs(delta_y) >= abs(delta_x))
			draw_steep_line(d, options, p1, p1, delta_x, delta_y);
		else
			draw_gradual_line(d, options, p1, p2, delta_x, delta_y);
	}
}

static struct polygon_edge *
draw_merge_edges(struct polygon_edge *edges1, struct polygon_edge *edges2)
{
	struct polygon_edge *head = NULL;
	struct polygon_edge **tailp = &head;
	while (edges1 || edges2)
	{
		while (edges1 && (!edges2 || edges1->lower_coord.x < edges2->lower_coord.x))
		{
			*tailp = edges1;
			edges1 = edges1->next;
			tailp = &((*tailp)->next);
		}
		if (edges2)
		{
			*tailp = edges2;
			edges2 = edges2->next;
			tailp = &((*tailp)->next);
		}
	}
	return head;
}

static void
draw_check_edges(struct polygon_edge *edges)
{
	u_int min_x = 0;
	while (edges)
	{
		assert(edges->lower_coord.x >= min_x);
		min_x = edges->lower_coord.x;
		edges = edges->next;
	}
}

void
draw_polygon(struct drawable *d, struct draw_options options, struct raster_vertex vertices[], u_int num_verts)
{
#ifndef NDEBUG
	for (u_int assert_y = 0; assert_y < d->window_height; ++assert_y)
		assert(!d->edge_table[assert_y]);
#endif
	struct polygon_edge edges[MAX_PRIMITIVE_VERTICES];
	u_int min_y = d->window_height;
	u_int max_y = 0;
	for (u_int edge_index = 0; edge_index < num_verts; ++edge_index)
	{
		struct polygon_edge *edge = &(edges[edge_index]);
		struct raster_vertex vertex1 = vertices[edge_index], vertex2 = vertices[(edge_index + 1) % num_verts];
		if (vertex1.y < vertex2.y)
		{
			edge->lower_coord = vertex1;
			edge->upper_coord = vertex2;
		}
		else if (vertex1.y > vertex2.y)
		{
			edge->lower_coord = vertex2;
			edge->upper_coord = vertex1;
		}
		else
			continue;

		edge->delta_x = edge->upper_coord.x - edge->lower_coord.x;
		edge->delta_y = edge->upper_coord.y - edge->lower_coord.y;
		edge->x_num = 0;
		edge->depth_incr = (edge->upper_coord.depth - edge->lower_coord.depth) /
				edge->delta_y;
		edge->next = NULL;
		//assert(edge->lower_coord.y < d->window_height);
		d->edge_table[edge->lower_coord.y] = draw_merge_edges(d->edge_table[edge->lower_coord.y], edge);
		if (edge->lower_coord.y < min_y)
			min_y = edge->lower_coord.y;
		if (edge->upper_coord.y > max_y)
			max_y = edge->upper_coord.y;
	}
	struct polygon_edge *active_edges = NULL;
	for (u_int y = min_y; y <= max_y; ++y)
	{
		active_edges = draw_merge_edges(active_edges, d->edge_table[y]);
		d->edge_table[y] = NULL;
#ifndef NDEBUG
		draw_check_edges(active_edges);
#endif

		struct polygon_edge *right_edge;
		for (struct polygon_edge *left_edge = active_edges; left_edge; left_edge = right_edge->next)
		{
			right_edge = left_edge->next;
			if (!right_edge)
				break;

			struct raster_vertex left_vertex;
			left_vertex.x = left_edge->lower_coord.x;
			left_vertex.y = y;
			left_vertex.depth = left_edge->lower_coord.depth;
			GLdouble left_u = ((GLdouble)y - left_edge->lower_coord.y) / left_edge->delta_y;
			left_vertex.color = vector4_lerp(left_edge->lower_coord.color, left_edge->upper_coord.color, left_u);
			struct raster_vertex right_vertex;
			right_vertex.x = right_edge->lower_coord.x;
			right_vertex.y = y;
			right_vertex.depth = right_edge->lower_coord.depth;
			GLdouble right_u = ((GLdouble)y - right_edge->lower_coord.y) / right_edge->delta_y;
			right_vertex.color = vector4_lerp(right_edge->lower_coord.color, right_edge->upper_coord.color, right_u);

			draw_horiz_line(d, options, left_vertex, right_vertex, right_vertex.x - left_vertex.x);
		}
		struct polygon_edge *next_active_edges = NULL;
		while (active_edges)
		{
			struct polygon_edge *edge = active_edges;

			edge->lower_coord.depth += edge->depth_incr;
			if (edge->delta_x > 0)
			{
				edge->x_num += edge->delta_x;
				while (edge->x_num >= edge->delta_y)
				{
					++edge->lower_coord.x;
					edge->x_num -= edge->delta_y;
				}
			}
			else if (edge->delta_x < 0)
			{
				edge->x_num-= edge->delta_x;
				while (edge->x_num >= edge->delta_y)
				{
					--edge->lower_coord.x;
					edge->x_num-= edge->delta_y;
				}
			}

			active_edges = edge->next;
			edge->next = NULL;
			if (edge->upper_coord.y != y + 1)
				next_active_edges = draw_merge_edges(next_active_edges, edge);
		}
		active_edges = next_active_edges;
	}
}

void
draw_primitive(struct drawable *d,
               struct draw_options options,
               struct vector3 vertices[],
               struct vector4 colors[],
               u_int num_verts)
{
	struct raster_vertex raster_vertices[MAX_PRIMITIVE_VERTICES];
	for (u_int i = 0; i < num_verts; ++i)
	{
		assert(vertices[i].x >= -1.0 && vertices[i].x <= 1.0);
		raster_vertices[i].x = d->view_x + round((vertices[i].x + 1) * ((d->view_width - 1) / 2.0));
		assert(vertices[i].y >= -1.0 && vertices[i].x <= 1.0);
		raster_vertices[i].y = d->view_y + round((vertices[i].y + 1) * ((d->view_height - 1) / 2.0));
		assert(vertices[i].z >= -1.0 && vertices[i].z <= 1.0);
		raster_vertices[i].depth = (vertices[i].z + 1) / 2.0;
		assert(colors[i].r >= 0 && colors[i].r <= 1.0);
		assert(colors[i].g >= 0 && colors[i].g <= 1.0);
		assert(colors[i].b >= 0 && colors[i].b <= 1.0);
		assert(colors[i].a >= 0 && colors[i].a <= 1.0);
		raster_vertices[i].color = colors[i];
	}

	switch (num_verts)
	{
		case 1:
			draw_pixel(d, options, raster_vertices[0]);
			break;
		case 2:
			draw_line(d, options, raster_vertices[0], raster_vertices[1]);
			break;
		default:
			draw_polygon(d, options, raster_vertices, num_verts);
	}
}

void
draw_flush(struct drawable *d)
{
	window_update(d->window, d->color_buffer, 0, 0, d->window_width, d->window_height);
}

void
draw_finish(struct drawable *d)
{
	window_update(d->window, d->color_buffer, 0, 0, d->window_width, d->window_height);
}

void
draw_flip(struct drawable *d)
{
	window_flip(d->window);
}
