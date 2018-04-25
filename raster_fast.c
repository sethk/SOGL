//
// Created by Seth Kingsley on 1/26/18.
//

#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include "raster.h"

void
raster_horiz_line(struct drawable *d,
                  struct draw_options options,
                  struct device_vertex dv1,
                  struct device_vertex dv2)
{
	raster_gradual_line(d, options, dv1, dv2);
	/*
	struct raster_vertex v1 = raster_from_device(d, dv1);
	struct raster_vertex v2 = raster_from_device(d, dv2);
	struct raster_vertex v = v1;
	raster_pixel(d, options, v);
	int x_step;
	int delta_x = v2.x - v1.x;
	if (delta_x > 0)
		x_step = 1;
	else if (delta_x < 0)
		x_step = -1;
	else
		return;
	GLdouble depth_incr = (v2.depth - v1.depth) / delta_x;
	while (v.x != v2.x)
	{
		v.x+= x_step;
		v.depth+= depth_incr;
		GLdouble u = ((GLdouble)v.x - v1.x) / delta_x;
		v.color = vector4_lerp(v1.color, v2.color, u);
		raster_pixel(d, options, v);
	}
	 */
}

void
raster_vertical_line(struct drawable *d,
                     struct draw_options options,
                     struct device_vertex dv1,
                     struct device_vertex dv2)
{
	raster_steep_line(d, options, dv1, dv2);
	/*
	struct raster_vertex
	struct raster_vertex v = v1;
	int y_incr;
	if (delta_y > 0)
		y_incr = 1;
	else
		y_incr = -1;
	float depth_incr = (v2.depth - v1.depth) / delta_y;
	raster_pixel(d, options, v);
	while (v.y != v2.y)
	{
		v.y+= y_incr;
		v.depth+= depth_incr;
		GLdouble u = ((GLdouble)v.y - v1.y) / delta_y;
		v.color = vector4_lerp(v1.color, v2.color, u);
		raster_pixel(d, options, v);
	}
	 */
}

void
raster_gradual_line(struct drawable *d,
                    struct draw_options options,
                    struct device_vertex dv1,
                    struct device_vertex dv2)
{
	struct raster_vertex v1 = raster_from_device(d, dv1);
	struct raster_vertex v2 = raster_from_device(d, dv2);
	struct raster_vertex v = v1;
	int delta_x = v2.coord.x - v1.coord.x;
	int delta_y = v2.coord.y - v1.coord.y;
	assert(abs(delta_y) < abs(delta_x));
	int x_incr;
	int actual_delta_x = delta_x;
	if (delta_x > 0)
		x_incr = 1;
	else
	{
		x_incr = -1;
		delta_x = -delta_x;
	}
	int y_incr;
	if (delta_y >= 0)
		y_incr = 1;
	else
	{
		y_incr = -1;
		delta_y = -delta_y;
	}
	float depth_incr = (v2.coord.depth - v1.coord.depth) / delta_x;
	int decide = 2 * delta_y - delta_x;
	int incr_across = 2 * delta_y;
	int incr_diag = 2 * (delta_y - delta_x);
	raster_pixel(d, options, v);
	while (v.coord.x != v2.coord.x)
	{
		if (decide <= 0)
			decide+= incr_across;
		else
		{
			decide+= incr_diag;
			v.coord.y+= y_incr;
		}
		v.coord.x+= x_incr;
		v.coord.depth+= depth_incr;
		GLdouble u = ((GLdouble)v.coord.x - v1.coord.x) / actual_delta_x;
		v.color = vector4_lerp(v1.color, v2.color, u);
		raster_pixel(d, options, v);
	}
}

void
raster_steep_line(struct drawable *d,
                  struct draw_options options,
                  struct device_vertex dv1,
                  struct device_vertex dv2)
{
	struct raster_vertex v1 = raster_from_device(d, dv1);
	struct raster_vertex v2 = raster_from_device(d, dv2);
	struct raster_vertex v = v1;
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
}

static void
raster_horiz_span(struct drawable *d,
                  struct draw_options options,
                  struct raster_vertex left_vertex,
                  struct raster_vertex right_vertex)
{
	scalar_t span = right_vertex.coord.x - left_vertex.coord.x;
	if (span > 0)
	{
		raster_depth_t depth_step = (right_vertex.coord.depth - left_vertex.coord.depth) / span;
		struct vector4 color_step = vector4_divide_scalar(vector4_sub(right_vertex.color, left_vertex.color), span);
		do
		{
			raster_pixel(d, options, left_vertex);
			left_vertex.coord.x += 1;
			left_vertex.coord.depth += depth_step;
			left_vertex.color = vector4_add(left_vertex.color, color_step);
		}
		while (left_vertex.coord.x < right_vertex.coord.x);
	}
}

static struct polygon_edge *
draw_merge_edges(struct polygon_edge *edges1, struct polygon_edge *edges2)
{
	struct polygon_edge *head = NULL;
	struct polygon_edge **tailp = &head;
	while (edges1 || edges2)
	{
		while (edges1 && (!edges2 || edges1->lower_vertex.coord.x < edges2->lower_vertex.coord.x))
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
raster_check_vertex(struct drawable *d, struct raster_vertex v)
{
	assert(v.coord.x >= d->view_x && v.coord.x < d->view_x + d->view_width);
	assert(v.coord.y >= d->view_y && v.coord.y < d->view_y + d->view_height);
}

static void
raster_check_edges(struct polygon_edge *edges)
{
	raster_loc_t min_x = 0;
	while (edges)
	{
		assert(edges->lower_vertex.coord.x >= min_x);
		min_x = edges->lower_vertex.coord.x;
		edges = edges->next;
	}
}

void
raster_triangle(struct drawable *d, struct draw_options options, const struct device_vertex vertices[3])
{
	raster_polygon(d, options, vertices, 3);
}

void
raster_polygon(struct drawable *d, struct draw_options options, const struct device_vertex dverts[], u_int num_verts)
{
#ifndef NDEBUG
	for (raster_loc_t assert_y = 0; assert_y < d->window_height; ++assert_y)
		assert(!d->edge_table[assert_y]);
#endif
	struct raster_vertex vertices[MAX_PRIMITIVE_VERTICES];
	for (u_int vertex_index = 0; vertex_index < num_verts; ++vertex_index)
		vertices[vertex_index] = raster_from_device(d, dverts[vertex_index]);

	struct polygon_edge edges[MAX_PRIMITIVE_VERTICES];
	raster_loc_t min_y = d->window_height;
	raster_loc_t max_y = 0;
	for (u_int edge_index = 0; edge_index < num_verts; ++edge_index)
	{
		struct polygon_edge *edge = &(edges[edge_index]);
		struct raster_vertex vertex1 = vertices[edge_index], vertex2 = vertices[(edge_index + 1) % num_verts];
		if (vertex1.coord.y < vertex2.coord.y)
		{
			edge->lower_vertex = vertex1;
			edge->upper_vertex = vertex2;
		}
		else if (vertex1.coord.y > vertex2.coord.y)
		{
			edge->lower_vertex = vertex2;
			edge->upper_vertex = vertex1;
		}
		else
			continue;

		edge->delta_x = edge->upper_vertex.coord.x - edge->lower_vertex.coord.x;
		edge->delta_y = edge->upper_vertex.coord.y - edge->lower_vertex.coord.y;
		edge->x_num = 0;
		edge->depth_incr = (edge->upper_vertex.coord.depth - edge->lower_vertex.coord.depth) / edge->delta_y;
		edge->next = NULL;
		assert(edge->lower_vertex.coord.y >= 0 && edge->lower_vertex.coord.y < d->window_height);
		d->edge_table[edge->lower_vertex.coord.y] = draw_merge_edges(d->edge_table[edge->lower_vertex.coord.y], edge);
		if (edge->lower_vertex.coord.y < min_y)
			min_y = edge->lower_vertex.coord.y;
		if (edge->upper_vertex.coord.y > max_y)
			max_y = edge->upper_vertex.coord.y;
	}
	struct polygon_edge *active_edges = NULL;
	for (raster_loc_t y = min_y; y <= max_y; ++y)
	{
		active_edges = draw_merge_edges(active_edges, d->edge_table[y]);
		d->edge_table[y] = NULL;
#ifndef NDEBUG
		raster_check_edges(active_edges);
#endif

		struct polygon_edge *right_edge;
		for (struct polygon_edge *left_edge = active_edges; left_edge; left_edge = right_edge->next)
		{
			right_edge = left_edge->next;
			if (!right_edge)
				break;

			if (left_edge->lower_vertex.coord.x < right_edge->lower_vertex.coord.x)
			{
				struct raster_vertex left_vertex;
				left_vertex.coord.x = left_edge->lower_vertex.coord.x;
				left_vertex.coord.y = y;
				left_vertex.coord.depth = left_edge->lower_vertex.coord.depth;
				GLdouble left_u = ((GLdouble) y - left_edge->lower_vertex.coord.y) / left_edge->delta_y;
				left_vertex.color = vector4_lerp(left_edge->lower_vertex.color,
				                                 left_edge->upper_vertex.color,
				                                 left_u);

				struct raster_vertex right_vertex;
				right_vertex.coord.x = right_edge->lower_vertex.coord.x;
				right_vertex.coord.y = y;
				right_vertex.coord.depth = right_edge->lower_vertex.coord.depth;
				GLdouble right_u = ((GLdouble) y - right_edge->lower_vertex.coord.y) / right_edge->delta_y;
				right_vertex.color = vector4_lerp(right_edge->lower_vertex.color,
				                                  right_edge->upper_vertex.color,
				                                  right_u);

				raster_check_vertex(d, left_vertex);
				raster_check_vertex(d, right_vertex);
				raster_horiz_span(d, options, left_vertex, right_vertex);
			}
		}
		struct polygon_edge *next_active_edges = NULL;
		while (active_edges)
		{
			struct polygon_edge *edge = active_edges;

			edge->lower_vertex.coord.depth += edge->depth_incr;
			if (edge->delta_x > 0)
			{
				edge->x_num += edge->delta_x;
				while (edge->x_num >= edge->delta_y)
				{
					++edge->lower_vertex.coord.x;
					edge->x_num -= edge->delta_y;
				}
			}
			else if (edge->delta_x < 0)
			{
				edge->x_num-= edge->delta_x;
				while (edge->x_num >= edge->delta_y)
				{
					--edge->lower_vertex.coord.x;
					edge->x_num-= edge->delta_y;
				}
			}

			active_edges = edge->next;
			edge->next = NULL;
			if (edge->upper_vertex.coord.y != y + 1)
				next_active_edges = draw_merge_edges(next_active_edges, edge);
		}
		active_edges = next_active_edges;
	}
}

