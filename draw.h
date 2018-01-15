//
// Created by Seth Kingsley on 1/8/18.
//

#ifndef SOGL_DRAW_H
#define SOGL_DRAW_H

#include <sys/types.h>
#include <stdbool.h>
#include "vector.h"

struct window;
struct drawable;
struct draw_options
{
	bool test_depth;
};

#define MAX_PRIMITIVE_VERTICES (8)

struct drawable *draw_create(struct window *window);
void draw_destroy(struct drawable *d);
void draw_reshape(struct drawable *d, u_int width, u_int height);
void draw_set_view(struct drawable *d, u_int x, u_int y, u_int width, u_int height);
void draw_clear(struct drawable *d, bool color, const struct vector4 clear_color, bool depth);
void draw_primitive(struct drawable *d,
                    struct draw_options draw_options,
                    struct vector3 coords[],
                    struct vector4 colors[],
                    u_int num_verts);
void draw_flush(struct drawable *d);
void draw_finish(struct drawable *d);
void draw_flip(struct drawable *d);

#endif //SOGL_DRAW_H
