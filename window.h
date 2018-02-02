//
// Created by Seth Kingsley on 1/12/18.
//

#ifndef SOGL_WINDOW_H
#define SOGL_WINDOW_H

#include <sys/types.h>
#include <stdbool.h>

struct window
{
	struct drawable *drawable;
};

struct raster_color
{
	u_char red, green, blue, alpha;
};

void window_update(struct window *w,
                   const struct raster_color *frame,
                   u_int x, u_int y, u_int width, u_int height,
                   bool flipped_y);
void window_flip(struct window *w);

#endif //SOGL_WINDOW_H
