//
// Created by Seth Kingsley on 1/12/18.
//

#ifndef SOGL_WINDOW_H
#define SOGL_WINDOW_H

#include <sys/types.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

struct window
{
	struct drawable *drawable;
};

struct window_color
{
	u_int8_t red, green, blue, alpha;
};

void window_update(struct window *w,
                   const struct window_color *frame,
                   u_int x, u_int y, u_int width, u_int height);
void window_flip(struct window *w);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //SOGL_WINDOW_H
