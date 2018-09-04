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

struct shaded_vertex
{
	struct vector4 world_pos;
	struct vector4 view_pos;
	struct vector3 world_norm;
	struct vector3 light_dirs[MAX_LIGHTS];
	struct vector3 lighting_eye_dir;
};

extern struct drawable *drawable;
extern GLuint primitive_index;
extern GLIContext debug_rend;
extern GLIFunctionDispatch *debug_disp;

void render_init_debug(void);
void render_update_debug_title(void);
void render_debug_key(u_char key, int x, int y);
void render_primitive(const struct modelview modelview,
                      const struct matrix4x4 proj,
                      struct vertex *vertices,
                      u_int *indices,
                      u_int num_vertices,
                      struct lighting *lighting,
                      struct draw_options option);

#endif //SOGL_RENDER_H
