//
// Created by Seth Kingsley on 1/12/18.
//

#include <stdlib.h>
#include <err.h>
#include <assert.h>
#include <math.h>
#include "draw.h"
#include "window.h"

struct drawable
{
	struct window *window;
	u_int window_width, window_height;
	u_int view_x, view_y, view_width, view_height;
	struct raster_color *color_buffer;
	float *depth_buffer;
};

struct raster_position
{
	u_int x, y;
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
	}
	d->window_width = width;
	d->window_height = height;
	assert(sizeof(*(d->color_buffer)) == 4);
	d->color_buffer = malloc(sizeof(*(d->color_buffer)) * d->window_width * d->window_height);
	d->depth_buffer = malloc(sizeof(*(d->depth_buffer)) * d->window_width * d->window_height);
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
draw_pixel(struct drawable *d, u_int x, u_int y, const struct vector4 color)
{
	assert(x < d->window_width);
	assert(y < d->window_height);
	struct raster_color *pixel = d->color_buffer + y * d->window_width + x;
	pixel->red = round(color.r * 255);
	pixel->green = round(color.g * 255);
	pixel->blue = round(color.b * 255);
	pixel->alpha = round(color.a * 255);
}

void
draw_clear(struct drawable *d, bool color, const struct vector4 clear_color, bool depth)
{
	for (GLuint y = 0; y < d->window_height; ++y)
		for (GLuint x = 0; x < d->window_width; ++x)
		{
			if (color)
				draw_pixel(d, x, y, clear_color);
			if (depth)
				d->depth_buffer[y * d->window_width + x] = 1.0;
		}
}

static void
draw_gradual_line(struct drawable *d,
                  struct draw_options options,
                  u_int x1,
                  u_int x2,
                  u_int y,
                  int delta_x,
                  int delta_y,
                  struct vector4 c1,
                  struct vector4 c2)
{
	assert(x1 + delta_x == x2);
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
	int decide = 2 * delta_y - delta_x;
	int incr_across = 2 * delta_y;
	int incr_diag = 2 * (delta_y - delta_x);
	u_int x = x1;
	draw_pixel(d, x, y, c1);
	while (x != x2)
	{
		if (decide <= 0)
			decide+= incr_across;
		else
		{
			decide+= incr_diag;
			y+= y_incr;
		}
		x+= x_incr;
		GLdouble u = ((GLdouble)x - x1) / actual_delta_x;
		draw_pixel(d, x, y, vector4_lerp(c1, c2, u));
	}
}

static void
draw_steep_line(struct drawable *d,
                struct draw_options options,
                u_int x,
                u_int y1,
                u_int y2,
                int delta_x,
                int delta_y,
                struct vector4 c1,
                struct vector4 c2)
{
	assert(y1 + delta_y == y2);
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
	u_int y = y1;
	draw_pixel(d, x, y, c1);
	while (y != y2)
	{
		if (decide <= 0)
			decide+= incr_up;
		else
		{
			decide += incr_diag;
			x += x_incr;
		}
		y+= y_incr;
		GLdouble u = ((GLdouble)y - y1) / actual_delta_y;
		draw_pixel(d, x, y, vector4_lerp(c1, c2, u));
	}
}

static void
draw_line(struct drawable *d,
          struct draw_options options,
          struct raster_position p1,
          struct raster_position p2,
          struct vector4 c1,
          struct vector4 c2)
{
	int delta_x = p2.x - p1.x;
	int delta_y = p2.y - p1.y;
	if (delta_y == 0)
	{
		int x_step;
		if (delta_x > 0)
			x_step = 1;
		else
			x_step = -1;
		u_int x = p1.x;
		while (x != p2.x)
		{
			GLdouble u = ((GLdouble)x - p1.x) / delta_x;
			draw_pixel(d, x, p1.y, vector4_lerp(c1, c2, u));
			x+= x_step;
		}
	}
	else if (delta_x == 0)
	{
		int y_step;
		if (delta_y > 0)
			y_step = 1;
		else
			y_step = -1;
		u_int y = p1.y;
		while (y != p2.y)
		{
			GLdouble u = ((GLdouble)y - p1.y) / delta_y;
			draw_pixel(d, p1.x, y, vector4_lerp(c1, c2, u));
			y+= y_step;
		}
	}
	else
	{
		if (abs(delta_y) >= abs(delta_x))
			draw_steep_line(d, options, p1.x, p1.y, p2.y, delta_x, delta_y, c1, c2);
		else
			draw_gradual_line(d, options, p1.x, p2.x, p1.y, delta_x, delta_y, c1, c2);
	}
}

void
draw_primitive(struct drawable *d,
               struct draw_options options,
               struct vector3 coords[],
               struct vector4 colors[],
               u_int num_verts)
{
	struct raster_position raster_coords[MAX_PRIMITIVE_VERTICES];
	for (u_int i = 0; i < num_verts; ++i)
	{
		raster_coords[i].x = d->view_x + round((coords[i].x + 1) * ((d->view_width - 1) / 2.0));
		raster_coords[i].y = d->view_y + round((coords[i].y + 1) * ((d->view_height - 1) / 2.0));
	}

	switch (num_verts)
	{
		case 1:
			draw_pixel(d, raster_coords[0].x, raster_coords[0].y, colors[0]);
			break;
		case 2:
			draw_line(d, options, raster_coords[0], raster_coords[1], colors[0], colors[1]);
			break;
		default:
			for (GLuint i = 0; i + 1 < num_verts; ++i)
				draw_line(d, options, raster_coords[i], raster_coords[i + 1], colors[i], colors[i + 1]);
			if (num_verts > 2)
				draw_line(d, options, raster_coords[num_verts - 1], raster_coords[0], colors[num_verts - 1], colors[0]);
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
