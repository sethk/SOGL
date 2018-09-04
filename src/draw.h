//
// Created by Seth Kingsley on 1/8/18.
//

#ifndef SOGL_DRAW_H
#define SOGL_DRAW_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <sys/types.h>
#include <stdbool.h>
#include <OpenGL/OpenGL.h> // TODO: Replace with enums
#include "vector.h"

struct window;
struct drawable;
struct draw_options
{
	GLenum draw_op;
	bool test_depth;
	GLenum depth_func;
	GLenum polygon_modes[2];
	bool cull_faces;
	bool faces_culled[2];
};

struct device_vertex
{
	struct vector4 coord;
	struct vector4 color;
};

#define MAX_PRIMITIVE_VERTICES (8)

struct drawable *draw_create(struct window *window);
void draw_destroy(struct drawable *d);
void draw_reshape(struct drawable *d, u_int width, u_int height);
void draw_set_view(struct drawable *d, u_int x, u_int y, u_int width, u_int height);
void draw_clear(struct drawable *d, bool color, const struct vector4 clear_color, bool depth, GLfloat clear_depth);
void draw_primitive(struct drawable *d,
                    struct draw_options draw_options,
                    const struct device_vertex vertices[],
                    u_int num_verts);
void draw_flush(struct drawable *d);
void draw_finish(struct drawable *d);
void draw_flip(struct drawable *d);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif //SOGL_DRAW_H
