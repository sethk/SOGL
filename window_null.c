//
// Created by Seth Kingsley on 6/25/18.
//

#include <stdlib.h>
#include <err.h>
#include "window_null.h"

struct window *
window_create_null(void)
{
	struct window *window = calloc(1, sizeof(struct window));
	if (!window)
		err(1, "Could not allocate null window");
	return window;
}

void
window_update(struct window *w,
              const struct raster_color *frame,
              u_int x, u_int y, u_int width, u_int height,
              bool flipped_y)
{
}

void
window_flip(struct window *w)
{
}
