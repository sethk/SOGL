#ifndef SOGL_RENDER_H
#define SOGL_RENDER_H

#include <sys/types.h>
#include <stdbool.h>
#include <OpenGL/CGLContext.h>

#include "matrix.h"
#include "vector.h"
#include "draw.h"

struct modelview
{
	struct matrix4x4 matrix;
	struct matrix4x4 inverse_trans;
};

struct material
{
	struct vector4 color;
	struct vector4 ambient;
	struct vector4 diffuse;
	struct vector4 specular;
	struct vector4 emission;
	scalar_t shininess;
};

struct vertex
{
	struct vector4 pos;
	struct vector3 norm;
	struct material mat;
};

enum {MAX_LIGHTS = 8};

struct lighting
{
	struct vector4 global_ambient;
	bool local_viewer;
	struct
	{
		bool enabled;
		struct vector4 ambient;
		struct vector4 diffuse;
		struct vector4 specular;
		struct vector4 pos;
	} lights[MAX_LIGHTS];
};

struct render_options
{
	struct modelview modelview;
	struct matrix4x4 proj;
	bool smooth_shading;
	bool lighting_enabled;
	struct lighting lighting;
	struct draw_options draw_options;
};

extern struct drawable *drawable;
extern GLuint primitive_index;
extern GLIContext debug_rend;
extern GLIFunctionDispatch *debug_disp;

void render_init_debug(void);
void render_update_debug_title(void);
void render_debug_key(u_char key, int x, int y);
void render_primitive(const struct render_options *options,
                      struct vertex *vertices,
                      u_int *indices,
                      u_int num_vertices);

#endif //SOGL_RENDER_H
