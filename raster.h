//
// Created by Seth Kingsley on 1/25/18.
//

#ifndef SOGL_RASTER_H
#define SOGL_RASTER_H

#include <sys/types.h>
#include "vector.h"
#include "window.h"
#include "draw.h"

// TODO: Rename raster_size
#define RASTER_UINT 0
#if RASTER_UINT
	typedef u_int raster_loc_t;
#else
	typedef int raster_loc_t;
#endif // RASTER_UNIT
typedef int raster_dist_t;

typedef float raster_depth_t;

struct raster_coord
{
	raster_loc_t x, y;
	raster_depth_t depth;
};

struct raster_vertex
{
	struct raster_coord coord;
	struct vector4 color;
};

struct polygon_edge
{
	struct raster_vertex upper_vertex;
	struct raster_vertex lower_vertex;
	raster_dist_t delta_x;
	raster_loc_t delta_y;
	raster_loc_t x_num;
	float depth_incr;
	struct polygon_edge *next;
};

struct drawable
{
	struct window *window;
	raster_loc_t window_width, window_height;
	raster_loc_t view_x, view_y, view_width, view_height;
	struct raster_color *color_buffer;
	float *depth_buffer;
	struct polygon_edge **edge_table;
};

raster_loc_t raster_x_from_device(struct drawable *d, GLdouble dx);
raster_loc_t raster_y_from_device(struct drawable *d, GLdouble dy);
raster_depth_t raster_z_from_device(struct drawable *d, scalar_t dz);
struct raster_vertex raster_from_device(struct drawable *d, struct device_vertex dv);
struct vector2 raster_to_device(struct drawable *d, struct raster_coord rc);
void raster_pixel(struct drawable *d, struct draw_options options, struct raster_vertex vertex);
void raster_horiz_line(struct drawable *d,
                       struct draw_options options,
                       struct device_vertex p1,
                       struct device_vertex p2);
void raster_gradual_line(struct drawable *d,
                         struct draw_options options,
                         struct device_vertex p1,
                         struct device_vertex p2);
void raster_steep_line(struct drawable *d,
                       struct draw_options options,
                       struct device_vertex p1,
                       struct device_vertex p2);
void raster_vertical_line(struct drawable *d,
                          struct draw_options options,
                          struct device_vertex p1,
                          struct device_vertex p2);
void raster_triangle(struct drawable *d, struct draw_options options, const struct device_vertex vertices[3]);

#endif //SOGL_RASTER_H
