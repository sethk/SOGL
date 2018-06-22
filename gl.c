#include <GLUT/glut.h>
#include <OpenGL/CGLContext.h>
#include <OpenGL/CGLCurrent.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <strings.h>

#include "wrap_glut.h"
#include "vector.h"
#include "matrix.h"
#include "window_cgl.h"
#include "draw.h"

#define number_of(a) (sizeof(a) / sizeof(*(a)))

#define DEBUG_WIN (-5)

/* Render */
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
	GLdouble shininess;
};

struct vertex
{
	struct vector4 pos;
	struct vector3 norm;
	struct material mat;
};

static const GLuint max_lights = 8;
struct lighting
{
	struct vector4 global_ambient;
	GLuint local_viewer;
    struct
    {
        GLboolean enabled;
	    struct vector4 ambient;
        struct vector4 diffuse;
        struct vector4 specular;
	    struct vector4 pos;
    } lights[max_lights];
};

struct shaded_vertex
{
	struct vector4 world_pos;
	struct vector4 view_pos;
	struct vector3 world_norm;
	struct vector3 light_dirs[max_lights];
	struct vector3 lighting_eye_dir;
};

struct drawable *drawable;
struct draw_options draw_options =
{
	.draw_op = GL_COPY,
	.test_depth = false,
	.depth_func = GL_LESS,
	.polygon_modes = {GL_FILL, GL_FILL}
};
static const struct vector4 origin = {.x = 0, .y = 0, .z = 0, .w = 1};
static GLuint primitive_index;

int debug_save_win;
GLIContext debug_rend;
GLIFunctionDispatch *debug_disp;
enum {DEBUG_PROJECTION, DEBUG_FRONT, DEBUG_LEFT, DEBUG_TOP, DEBUG_NMODES} debug_mode = 0;
static GLint debug_primitive_index = -1;
static GLint debug_light_index = -1;
static GLboolean debug_color = GL_FALSE;
static GLdouble debug_zoom = 0;
static struct matrix4x4 debug_proj;

static void
render_update_debug_title(void)
{
	char *mode = NULL;
	switch (debug_mode)
	{
		case DEBUG_FRONT: mode = "Front"; break;
		case DEBUG_LEFT: mode = "Left"; break;
		case DEBUG_TOP: mode = "Top"; break;
		case DEBUG_PROJECTION: mode = "Projection"; break;
		case DEBUG_NMODES: break;
	}
	char title[64];
	size_t len;
	len = snprintf(title, sizeof(title), "Debug - %s @ %.0f%%", mode, pow(2, debug_zoom) * 100);
	if (debug_light_index != -1)
		len+= snprintf(title + len, sizeof(title) - len, ", Light %u", debug_light_index);
	if (debug_primitive_index != -1)
		len+= snprintf(title + len, sizeof(title) - len, ", Primitive %u", debug_primitive_index);
	if (debug_color)
		len+= snprintf(title + len, sizeof(title) - len, ", Color");
	glutSetWindowTitle(title);
}

static void
render_push_debug(void)
{
	debug_save_win = glutGetWindow();
	assert(debug_save_win != DEBUG_WIN);
	glutSetWindow(DEBUG_WIN);
}

static void
render_pop_debug(void)
{
	assert(glutGetWindow() == DEBUG_WIN);
	glutSetWindow(debug_save_win);
}

static void
render_update_debug_proj(const struct matrix4x4 proj)
{
	GLfloat scale = pow(2, debug_zoom);
	struct matrix4x4 scaling = matrix4x4_make_scaling(scale, scale, scale);
	switch (debug_mode)
	{
		case DEBUG_FRONT:
			debug_proj = matrix4x4_make_scaling(scale, scale, scale);
			break;
		case DEBUG_LEFT:
			debug_proj = matrix4x4_make_rotation(90, 0, 1, 0);
			debug_proj = matrix4x4_mult_matrix4x4(debug_proj, scaling);
			break;
		case DEBUG_TOP:
			debug_proj = matrix4x4_make_rotation(90, 1, 0, 0);
			debug_proj = matrix4x4_mult_matrix4x4(debug_proj, scaling);
			break;
		case DEBUG_PROJECTION:
			debug_proj = matrix4x4_mult_matrix4x4(scaling, proj);
			break;
		case DEBUG_NMODES:
			break;
	}
}

static void
render_axes_debug()
{
	debug_disp->begin(debug_rend, GL_LINES);

	struct vector4 view_origin = matrix4x4_mult_vector4(debug_proj, origin);

	const GLdouble axes_size = 0.2;
	debug_disp->color3f(debug_rend, 1, 1, 1);
	debug_disp->vertex4dv(debug_rend, view_origin.v);
	struct vector4 right = {.x = axes_size, .y = 0, .z = 0, .w = 1};
	right = matrix4x4_mult_vector4(debug_proj, right);
	debug_disp->color3f(debug_rend, 0.5, 0.5, 1);
	debug_disp->vertex4dv(debug_rend, right.v);

	debug_disp->color3f(debug_rend, 1, 1, 1);
	debug_disp->vertex4dv(debug_rend, view_origin.v);
	struct vector4 up = {.x = 0, .y = axes_size, .z = 0, .w = 1};
	up = matrix4x4_mult_vector4(debug_proj, up);
	debug_disp->color3f(debug_rend, 1.0, 0.5, 0.5);
	debug_disp->vertex4dv(debug_rend, up.v);

	debug_disp->color3f(debug_rend, 1, 1, 1);
	debug_disp->vertex4dv(debug_rend, view_origin.v);
	struct vector4 forward = {.x = 0, .y = 0, .z = axes_size, .w = 1};
	forward = matrix4x4_mult_vector4(debug_proj, forward);
	debug_disp->color3f(debug_rend, 0.5, 1, 0.5);
	debug_disp->vertex4dv(debug_rend, forward.v);

	debug_disp->end(debug_rend);
}

static void
render_frustum_debug(struct matrix4x4 proj)
{
	/*
	debug_disp->point_size(debug_rend, 5);
	debug_disp->begin(debug_rend, GL_POINTS);
	debug_disp->color3f(debug_rend, 0, 1, 0);
	struct vector4 eye_pos = matrix4x4_mult_vector4(debug_proj, proj.world_eye_pos);
	debug_disp->vertex4dv(debug_rend, eye_pos.v);
	debug_disp->end(debug_rend);
	debug_disp->point_size(debug_rend, 1);
	 */
	/*
	glBegin(GL_LINES);
	glColor3f(0, 1, 1);
	vec4_t edge = {-1, -1, -1, 1};
	vec4_t trans_edge;
	struct matrix4x4 trans;
	matrix4x4_mult_matrix4x4(modelview_matrix, projection_matrix, &trans);
	matrix4x4_mult_vec4(trans, edge, trans_edge);
	glVertex3dv(trans_edge);
	edge[3] = 1;
	matrix4x4_mult_vec4(trans, edge, trans_edge);
	glVertex3dv(trans_edge);
	glEnd();
	 */
}

/*
static void
render_lights_debug(struct light *lights, GLuint num_lights)
{
	struct matrix4x4 trans = render_get_debug_proj();

	glBegin(GL_LINES);
	glColor3f(1, 1, 0);
	vec4_t dir;
	bcopy(lights[0].dir, dir, sizeof(lights[0].dir));
	dir[3] = 1.0;
	vec3_mult_scalar(dir, 5, dir);
	matrix4x4_mult_vec4(trans, dir, dir);
	glVertex4dv(dir);
	//vec4_print(dir);

	vec4_t trans_origin;
	matrix4x4_mult_vec4(trans, origin, trans_origin);
	glVertex4dv(trans_origin);
	glEnd();
}
*/

static struct shaded_vertex
render_shade_vertex(const struct modelview modelview,
                    const struct matrix4x4 proj,
                    const struct vertex vertex,
                    const struct lighting *lighting)
{
	struct shaded_vertex shaded;
	shaded.world_pos = matrix4x4_mult_vector4(modelview.matrix, vertex.pos);
	shaded.view_pos = matrix4x4_mult_vector4(proj, shaded.world_pos);

	if (lighting)
	{
		struct vector4 norm4;
		norm4.xyz = vertex.norm;
		norm4.w = 0;
		shaded.world_norm = matrix4x4_mult_vector4(modelview.inverse_trans, norm4).xyz;
		vector3_check_norm(shaded.world_norm, "world_norm");

		for (GLuint i = 0; i < number_of(lighting->lights); ++i)
		{
			if (!lighting->lights[i].enabled)
				continue;

			if (lighting->lights[i].pos.w == 0)
				shaded.light_dirs[i] = vector3_norm(lighting->lights[i].pos.xyz);
			else
			{
				struct vector3 light_dir = vector3_sub(lighting->lights[i].pos.xyz, shaded.world_pos.xyz);
				shaded.light_dirs[i] = vector3_norm(light_dir);
			}
		}

		if (lighting->local_viewer)
		{
			struct vector3 eye_dir = vector3_sub(origin.xyz, shaded.world_pos.xyz);
			shaded.lighting_eye_dir = vector3_norm(eye_dir);
		}
		else
			shaded.lighting_eye_dir = (struct vector3){.x = 0, .y = 0, .z = 1};
	}
	return shaded;
}

static struct vector4
render_shade_pixel(const struct material material,
                   const struct shaded_vertex vertex,
                   const struct lighting *lighting)
{
	struct vector4 color;
	if (lighting)
	{
		// Emission
		color.rgb = material.emission.rgb;
		color.a = material.diffuse.a;

		// Global ambient
		struct vector3 ambient = vector3_mult_vector3(lighting->global_ambient.rgb, material.ambient.rgb);
		color.rgb = vector3_add(color.rgb, ambient);

		for (GLuint light_index = 0; light_index < number_of(lighting->lights); ++light_index)
		{
			if (!lighting->lights[light_index].enabled)
				continue;

			// Ambient
			ambient = vector3_mult_vector3(lighting->lights[light_index].ambient.rgb, material.ambient.rgb);
			color.rgb = vector3_add(color.rgb, ambient);

			// Diffuse
			GLdouble cos_theta = vector3_dot(vertex.world_norm, vertex.light_dirs[light_index]);
			GLdouble diff_mix = fmax(0, cos_theta);
			struct vector3 diffuse;
			diffuse = vector3_mult_scalar(lighting->lights[light_index].diffuse.rgb, diff_mix);
			//vec3_print(diffuse);
			diffuse = vector3_mult_vector3(diffuse, material.diffuse.rgb);
			//vec3_print(color);
			color.rgb = vector3_add(color.rgb, diffuse);

			// Specular
			struct vector3 half_dir = vector3_add(vertex.light_dirs[light_index], vertex.lighting_eye_dir);
			half_dir = vector3_norm(half_dir);
			GLdouble cos_theta_half = vector3_dot(vertex.world_norm, half_dir);
			//fprintf(stderr, "cos_theta_half = %g\n", cos_theta_half);
			GLdouble spec_mix = pow(fmax(0, cos_theta_half), material.shininess);
			//fprintf(stderr, "spec_mix = %g\n", spec_mix);
			struct vector3 specular = vector3_mult_scalar(lighting->lights[light_index].specular.rgb, spec_mix);
			specular = vector3_mult_vector3(specular, material.specular.rgb);
			//vec3_print(specular);
			color.rgb = vector3_add(color.rgb, specular);
		}
		color.rgb = vector3_clamp(color.rgb);
		return color;
	}
	else
		return material.color;
}

static void
render_shade_vertices(const struct modelview modelview,
                      const struct matrix4x4 proj,
                      struct vertex *vertices,
                      GLuint *indices,
                      GLuint num_vertices,
                      struct lighting *lighting,
                      struct shaded_vertex *shaded)
{
	for (GLuint i = 0; i < num_vertices; ++i)
	{
		struct vertex vertex = vertices[indices[i]];
		shaded[i] = render_shade_vertex(modelview, proj, vertex, lighting);
	}
}

static void
render_primitive_debug(const struct modelview modelview,
                       const struct matrix4x4 proj,
                       struct vertex *vertices,
                       GLuint *indices,
                       GLuint num_vertices,
                       struct lighting *lighting)
{
	render_update_debug_proj(proj);
	struct lighting *debug_lighting = (debug_light_index != -1) ? lighting : NULL;
	struct shaded_vertex shaded_verts[MAX_PRIMITIVE_VERTICES];
	render_shade_vertices(modelview, debug_proj, vertices, indices, num_vertices, debug_lighting, shaded_verts);

	debug_disp->polygon_mode(debug_rend, GL_FRONT_AND_BACK, GL_LINE);
	debug_disp->begin(debug_rend, GL_LINE_LOOP);
	for (GLuint i = 0; i < num_vertices; ++i)
	{
		if (debug_color)
		{
			struct vector4 color = render_shade_pixel(vertices[indices[i]].mat, shaded_verts[i], debug_lighting);
			debug_disp->color4dv(debug_rend, color.v);
		}
		else
			debug_disp->color3f(debug_rend, 1, 1, 1);
		debug_disp->vertex4dv(debug_rend, shaded_verts[i].view_pos.v);
	}
	debug_disp->end(debug_rend);
	debug_disp->polygon_mode(debug_rend, GL_FRONT_AND_BACK, GL_FILL);

	debug_disp->begin(debug_rend, GL_LINES);
	for (GLuint i = 0; i < num_vertices; ++i)
	{
		struct shaded_vertex *shaded = &(shaded_verts[i]);
		debug_disp->color3f(debug_rend, 0, 1, 1);
		debug_disp->vertex4dv(debug_rend, shaded->view_pos.v);
		struct vector4 vert_norm;
		vert_norm.xyz = vector3_mult_scalar(shaded->world_norm, 0.5);
		vert_norm.w = 1.0;
		vert_norm.xyz = vector3_add(shaded->world_pos.xyz, vert_norm.xyz);
		vert_norm = matrix4x4_mult_vector4(debug_proj, vert_norm);
		debug_disp->vertex4dv(debug_rend, vert_norm.v);

		if (lighting && debug_light_index != -1 && lighting->lights[debug_light_index].enabled)
		{
			debug_disp->color3f(debug_rend, 1, 0.5, 0.5);
			debug_disp->vertex4dv(debug_rend, shaded->view_pos.v);
			struct vector4 view_light;
			view_light = matrix4x4_mult_vector4(debug_proj, lighting->lights[debug_light_index].pos);
			debug_disp->vertex4dv(debug_rend, view_light.v);

			debug_disp->color3f(debug_rend, 1, 1, 0);
			debug_disp->vertex4dv(debug_rend, shaded->view_pos.v);
			struct vector4 vert_light;
			vert_light.xyz = vector3_mult_scalar(shaded->light_dirs[debug_light_index], 0.5);
			vert_light.w = 1.0;
			vert_light.xyz = vector3_add(shaded->world_pos.xyz, vert_light.xyz);
			vert_light = matrix4x4_mult_vector4(debug_proj, vert_light);
			debug_disp->vertex4dv(debug_rend, vert_light.v);

			debug_disp->color3f(debug_rend, 0, 1, 0);
			debug_disp->vertex4dv(debug_rend, shaded->view_pos.v);
			struct vector4 vert_eye;
			vert_eye.xyz = vector3_mult_scalar(shaded->lighting_eye_dir, 0.5);
			vert_eye.w = 1.0;
			vert_eye.xyz = vector3_add(shaded->world_pos.xyz, vert_eye.xyz);
			vert_eye = matrix4x4_mult_vector4(debug_proj, vert_eye);
			debug_disp->vertex4dv(debug_rend, vert_eye.v);

		}
	}
	debug_disp->end(debug_rend);

	render_axes_debug();
	render_frustum_debug(proj);
}

void
render_primitive(const struct modelview modelview,
				 const struct matrix4x4 proj,
				 struct vertex *vertices,
				 GLuint *indices,
				 GLuint num_vertices,
				 struct lighting *lighting)
{
	if (debug_primitive_index == -1 || primitive_index == (GLuint)debug_primitive_index)
	{
		struct shaded_vertex shaded[MAX_PRIMITIVE_VERTICES];
		render_shade_vertices(modelview, proj, vertices, indices, num_vertices, lighting, shaded);
		struct device_vertex device_vertices[MAX_PRIMITIVE_VERTICES];
		//struct vector3 coords[MAX_PRIMITIVE_VERTICES];
		//struct vector4 colors[MAX_PRIMITIVE_VERTICES];
		for (GLuint i = 0; i < num_vertices; ++i)
		{
			device_vertices[i].coord = vector4_project(shaded[i].view_pos);
			device_vertices[i].color = render_shade_pixel(vertices[indices[i]].mat, shaded[i], lighting);
		}

		render_primitive_debug(modelview, proj, vertices, indices, num_vertices, lighting);
		draw_primitive(drawable, draw_options, device_vertices, num_vertices);
	}

	++primitive_index;
}

/* GL */
#include "gl_stubs.c"

static struct vector4 clear_color = {.r = 0, .g = 0, .b = 0, .a = 0};
static GLenum matrix_mode = GL_MODELVIEW;
static struct matrix4x4 modelview_stack[32] = {IDENTITY_MATRIX4X4};
static GLuint modelview_depth = 0;
static struct matrix4x4 projection_stack[2] = {IDENTITY_MATRIX4X4};
static GLuint projection_depth = 0;
static GLenum primitive_mode;
static GLboolean triangle_strip_winding = GL_FALSE;
static struct material material =
{
	.color = {.r = 1, .g = 1, .b = 1, .a = 1},
	.ambient = {.r = 0.2, .g = 0.2, .b = 0.2, .a = 1.0},
	.diffuse = {.r = 0.8, .g = 0.8, .b = 0.8, .a = 1.0},
	.specular = {.r = 0, .g = 0, .b = 0, .a = 1},
	.emission = {.r = 0, .g = 0, .b = 0, .a = 1},
	.shininess = 0
};
static struct vector3 normal;
static struct vertex vertices[MAX_PRIMITIVE_VERTICES];
static GLuint num_vertices;

#define DEFAULT_LIGHT {\
        .enabled = GL_FALSE,\
        .pos = {.x = 0, .y = 0, .z = 1, .w = 0},\
        .diffuse = {.v = {0, 0, 0, 0}},\
        .specular = {.v = {0, 0, 0, 0}}\
    }

static GLboolean lighting_enabled = GL_FALSE;
static struct lighting lighting =
{
	.global_ambient = {.r = 0.2, .g = 0.2, .b = 0.2, .a = 1.0},
	.local_viewer = 0,
	.lights =
	{
		{
			.enabled = GL_FALSE,
			.pos = {.x = 0, .y = 0, .z = 1, .w = 0},
			.diffuse = {.r = 1, .g = 1, .b = 1, .a = 1},
			.specular = {.r = 1, .g = 1, .b = 1, .a = 1}
		},
		DEFAULT_LIGHT,
		DEFAULT_LIGHT,
		DEFAULT_LIGHT,
		DEFAULT_LIGHT,
		DEFAULT_LIGHT,
		DEFAULT_LIGHT,
		DEFAULT_LIGHT
	}
};

static const GLuint max_attrib_depth = 3;
static struct
{
	GLbitfield bits;
	GLboolean lighting_enabled;
	GLboolean test_depth;
} saved_attrib_stack[max_attrib_depth];
static GLuint saved_attrib_depth = 0;

static void
gl_begin(GLIContext rend, GLenum mode)
{
	primitive_mode = mode;
	num_vertices = 0;
	triangle_strip_winding = GL_TRUE;
}

static void
gl_color4dv(GLIContext rend, const GLdouble *c)
{
	material.color = vector4_from_array(c);
}

static void
gl_color3fv(GLIContext rend, const GLfloat *c)
{
	GLdouble c4[4] = {c[0], c[1], c[2], 1.0};
	gl_color4dv(rend, c4);
}

static void
gl_color3f(GLIContext rend, GLfloat red, GLfloat green, GLfloat blue)
{
	GLdouble c[4] = {red, green, blue, 1.0};
	gl_color4dv(rend, c);
}

static void
gl_color3d(GLIContext rend, GLdouble red, GLdouble green, GLdouble blue)
{
	GLdouble c[4] = {red, green, blue, 1.0};
	gl_color4dv(rend, c);
}

static void
gl_materialfv(GLIContext rend, GLenum face, GLenum pname, const GLfloat *params)
{
	switch (pname)
	{
		case GL_AMBIENT:
			material.ambient = vector4_from_float_array(params);
			break;
		case GL_DIFFUSE:
			material.diffuse = vector4_from_float_array(params);
			break;
		case GL_SPECULAR:
			material.specular = vector4_from_float_array(params);
			break;
		case GL_EMISSION:
			material.emission = vector4_from_float_array(params);
			break;
		case GL_SHININESS:
			material.shininess = params[0];
			break;
		default:
			fprintf(stderr, "%s() TODO %0x\n", __FUNCTION__, pname);
	}
}

static void
gl_normal3dv(GLIContext ctx, const GLdouble *v)
{
	normal = vector3_from_array(v);
	vector3_check_norm(normal, "gl_normal");
}

static void
gl_normal3d(GLIContext ctx, GLdouble nx, GLdouble ny, GLdouble nz)
{
	fprintf(stderr, "TODO: normal3d()\n");
}

static void
gl_normal3fv(GLIContext rend, const GLfloat *v)
{
	GLdouble dv[3] = {v[0], v[1], v[2]};
	gl_normal3dv(rend, dv);
}

static void
gl_normal3f(GLIContext rend, GLfloat x, GLfloat y, GLfloat z)
{
	GLdouble v[3] = {x, y, z};
	gl_normal3dv(rend, v);
}

static void
gl_normal3b(GLIContext ctx, GLbyte nx, GLbyte ny, GLbyte nz)
{
	fprintf(stderr, "TODO: normal3b()\n");
}

static void
gl_normal3bv(GLIContext ctx, const GLbyte *v)
{
	fprintf(stderr, "TODO: normal3bv()\n");
}

static void
gl_normal3i(GLIContext ctx, GLint nx, GLint ny, GLint nz)
{
	fprintf(stderr, "TODO: normal3i()\n");
}

static void
gl_normal3iv(GLIContext ctx, const GLint *v)
{
	fprintf(stderr, "TODO: normal3iv()\n");
}

static void
gl_normal3s(GLIContext ctx, GLshort nx, GLshort ny, GLshort nz)
{
	fprintf(stderr, "TODO: normal3s()\n");
}

static void
gl_normal3sv(GLIContext ctx, const GLshort *v)
{
	fprintf(stderr, "TODO: normal3sv()\n");
}

static void
gl_render_primitive(GLuint *indices, GLuint num_indices)
{
	struct modelview modelview;
	modelview.matrix = modelview_stack[modelview_depth];
	modelview.inverse_trans = matrix4x4_invert_trans(modelview_stack[modelview_depth]);
	struct matrix4x4 proj = projection_stack[projection_depth];
	render_primitive(modelview,
	                 proj,
	                 vertices,
	                 indices,
	                 num_indices,
	                 (lighting_enabled) ? &lighting : NULL);
}

static void
gl_vertex4dv(GLIContext rend, const GLdouble *v)
{
	assert(num_vertices < number_of(vertices));
	vertices[num_vertices].pos = vector4_from_array(v);
	vertices[num_vertices].norm = normal;
	vertices[num_vertices].mat = material;
	++num_vertices;

	GLuint indices[MAX_PRIMITIVE_VERTICES];
	switch (primitive_mode)
	{
		case GL_POINTS:
			indices[0] = 0;
			gl_render_primitive(indices, 1);
			num_vertices = 0;
			break;

		case GL_LINES:
			if (num_vertices == 2)
			{
				indices[0] = 0;
				indices[1] = 1;
				gl_render_primitive(indices, 2);
				num_vertices = 0;
			}
			break;

		case GL_LINE_STRIP:
			if (num_vertices == 2)
			{
				indices[0] = 0;
				indices[1] = 1;
				gl_render_primitive(indices, 2);
				vertices[0] = vertices[1];
				num_vertices = 1;
			}
			break;

		case GL_LINE_LOOP:
			if (num_vertices == 2)
			{
				indices[0] = 0;
				indices[1] = 1;
				gl_render_primitive(indices, 2);
			}
			else if (num_vertices == 3)
			{
				indices[0] = 1;
				indices[1] = 2;
				gl_render_primitive(indices, 2);
				vertices[1] = vertices[2];
				num_vertices = 2;
			}
			break;

		case GL_TRIANGLES:
			if (num_vertices == 3)
			{
				indices[0] = 0;
				indices[1] = 1;
				indices[2] = 2;
				gl_render_primitive(indices, 3);
				num_vertices = 0;
			}
			break;

		case GL_TRIANGLE_STRIP:
			if (num_vertices == 3)
			{
				if (triangle_strip_winding)
				{
					indices[0] = 0;
					indices[1] = 1;
				}
				else
				{
					indices[0] = 1;
					indices[1] = 0;
				}
				triangle_strip_winding = !triangle_strip_winding;
				indices[2] = 2;
				// TODO: Skip degenerate triangles
				gl_render_primitive(indices, 3);
				vertices[0] = vertices[1];
				vertices[1] = vertices[2];
				num_vertices = 2;
			}
			break;

		case GL_TRIANGLE_FAN:
			if (num_vertices == 3)
			{
				indices[0] = 0;
				indices[1] = 1;
				indices[2] = 2;
				gl_render_primitive(indices, 3);
				vertices[1] = vertices[2];
				num_vertices = 2;
			}
			break;

		case GL_QUADS:
			if (num_vertices == 4)
			{
				indices[0] = 0;
				indices[1] = 1;
				indices[2] = 2;
				indices[3] = 3;
				gl_render_primitive(indices, 4);
				num_vertices = 0;
			}
			break;

		case GL_QUAD_STRIP:
			if (num_vertices == 4)
			{
				indices[0] = 0;
				indices[1] = 1;
				indices[2] = 3;
				indices[3] = 2;
				gl_render_primitive(indices, 4);
				vertices[0] = vertices[2];
				vertices[1] = vertices[3];
				num_vertices = 2;
			}
			break;
	}
}

static void
gl_vertex2d(GLIContext ctx, GLdouble x, GLdouble y)
{
    GLdouble v[4] = {x, y, 0, 1.0};
    gl_vertex4dv(ctx, v);
}

static void
gl_vertex2dv(GLIContext ctx, const GLdouble *v)
{
	GLdouble v4[4] = {v[0], v[1], 0, 1.0};
	gl_vertex4dv(ctx, v4);
}

static void
gl_vertex2f(GLIContext ctx, GLfloat x, GLfloat y)
{
	GLdouble v[4] = {x, y, 0, 1.0};
	gl_vertex4dv(ctx, v);
}

static void
gl_vertex2fv(GLIContext ctx, const GLfloat *v)
{
	GLdouble dv[4] = {v[0], v[1], 0, 1.0};
	gl_vertex4dv(ctx, dv);
}

static void
gl_vertex2i(GLIContext ctx, GLint x, GLint y)
{
    GLdouble v[4] = {x, y, 0, 1.0};
	gl_vertex4dv(ctx, v);
}

static void
gl_vertex2iv(GLIContext ctx, const GLint *v)
{
	GLdouble dv[4] = {v[0], v[1], 0, 1.0};
    gl_vertex4dv(ctx, dv);
}

static void
gl_vertex2s(GLIContext ctx, GLshort x, GLshort y)
{
	fprintf(stderr, "TODO: vertex2s()\n");
}

static void
gl_vertex2sv(GLIContext ctx, const GLshort *v)
{
	fprintf(stderr, "TODO: vertex2sv()\n");
}

static void
gl_vertex3d(GLIContext ctx, GLdouble x, GLdouble y, GLdouble z)
{
	fprintf(stderr, "TODO: vertex3d()\n");
}

static void
gl_vertex3dv(GLIContext ctx, const GLdouble *v)
{
	GLdouble v4[4] = {v[0], v[1], v[2], 1.0};
	gl_vertex4dv(ctx, v4);
}

static void
gl_vertex3fv(GLIContext rend, const GLfloat *v)
{
	GLdouble v4[4] = {v[0], v[1], v[2], 1.0};
	gl_vertex4dv(rend, v4);
}

static void
gl_vertex3f(GLIContext rend, GLfloat x, GLfloat y, GLfloat z)
{
	GLdouble v[4] = {x, y, z, 1.0};
	gl_vertex4dv(rend, v);
}

static void
gl_vertex3i(GLIContext ctx, GLint x, GLint y, GLint z)
{
	fprintf(stderr, "TODO: vertex3i()\n");
}

static void
gl_vertex3iv(GLIContext ctx, const GLint *v)
{
	fprintf(stderr, "TODO: vertex3iv()\n");
}

static void
gl_vertex3s(GLIContext ctx, GLshort x, GLshort y, GLshort z)
{
	fprintf(stderr, "TODO: vertex3s()\n");
}

static void
gl_vertex3sv(GLIContext ctx, const GLshort *v)
{
	fprintf(stderr, "TODO: vertex3sv()\n");
}

static void
gl_vertex4d(GLIContext ctx, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	fprintf(stderr, "TODO: vertex4d()\n");
}

static void
gl_vertex4f(GLIContext ctx, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	fprintf(stderr, "TODO: vertex4f()\n");
}

static void
gl_vertex4fv(GLIContext rend, const GLfloat *v)
{
	GLdouble dv[4] = {v[0], v[1], v[2], v[3]};
	gl_vertex4dv(rend, dv);
}

static void
gl_vertex4i(GLIContext ctx, GLint x, GLint y, GLint z, GLint w)
{
	fprintf(stderr, "TODO: vertex4i()\n");
}

static void
gl_vertex4iv(GLIContext ctx, const GLint *v)
{
	fprintf(stderr, "TODO: vertex4iv()\n");
}

static void
gl_vertex4s(GLIContext ctx, GLshort x, GLshort y, GLshort z, GLshort w)
{
	fprintf(stderr, "TODO: vertex4s()\n");
}

static void
gl_vertex4sv(GLIContext ctx, const GLshort *v)
{
	fprintf(stderr, "TODO: vertex4sv()\n");
}

/*
static void
gl_check_matrix(GLenum param, struct matrix4x4 *target)
{
	struct matrix4x4 old_m;
	openGLGetDoublev(param, old_m.m);
	for (GLuint col = 0; col < 4; ++col)
		for (GLuint row = 0; row < 4; ++row)
			if (fabs(old_m.cols[col][row] - target->cols[col][row]) > 0.00001)
			{
				fprintf(stderr, "Matrices not equal at %u,%u: %g vs. %g\n",
						col, row,
						old_m.cols[col][row], target->cols[col][row]);
				*target = old_m;
				break;
			}
}
*/

static void
gl_end(GLIContext rend)
{
	GLuint indices[MAX_PRIMITIVE_VERTICES];
	if (primitive_mode == GL_LINE_LOOP && num_vertices == 2)
	{
		indices[0] = 1;
		indices[1] = 0;
		gl_render_primitive(indices, 2);
	}
	else if (primitive_mode == GL_POLYGON && num_vertices >= 3)
    {
	    for (GLuint i = 0; i < num_vertices; ++i)
		    indices[i] = i;
	    gl_render_primitive(indices, num_vertices);
    }
}

static void
gl_recti(GLIContext ctx, GLint x1, GLint y1, GLint x2, GLint y2)
{
	gl_begin(ctx, GL_QUADS);
	gl_vertex2i(ctx, x1, y1);
	gl_vertex2i(ctx, x2, y1);
	gl_vertex2i(ctx, x2, y2);
	gl_vertex2i(ctx, x1, y2);
	gl_end(ctx);
}

static void
gl_rectd(GLIContext ctx, GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2)
{
	gl_begin(ctx, GL_QUADS);
	gl_vertex2d(ctx, x1, y1);
	gl_vertex2d(ctx, x2, y1);
	gl_vertex2d(ctx, x2, y2);
	gl_vertex2d(ctx, x1, y2);
	gl_end(ctx);
}

static void
gl_clear(GLIContext rend, GLbitfield mask)
{
	if (mask & ~(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT))
		fprintf(stderr, "%s() TODO: 0x%x\n", __FUNCTION__, mask & ~GL_COLOR_BUFFER_BIT);
	bool color = ((mask & GL_COLOR_BUFFER_BIT) != 0);
	bool depth = ((mask & GL_DEPTH_BUFFER_BIT) != 0);
	draw_clear(drawable, color, clear_color, depth, 1.0);

	debug_disp->clear(debug_rend, mask & GL_COLOR_BUFFER_BIT);
}

static void
gl_enable(GLIContext rend, GLenum cap)
{
	switch (cap)
	{
		case GL_DEPTH_TEST:
			draw_options.test_depth = true;
			break;

		case GL_LIGHTING:
			lighting_enabled = GL_TRUE;
			break;

		case GL_LIGHT0:
			lighting.lights[cap - GL_LIGHT0].enabled = GL_TRUE;
			break;

		default:
			fprintf(stderr, "%s() TODO 0x%x\n", __FUNCTION__, cap);
			//opengl_disp.enable(opengl_rend, cap);
	}
}

static void
gl_disable(GLIContext ctx, GLenum cap)
{
    switch (cap)
	{
		case GL_DEPTH_TEST:
			draw_options.test_depth = false;
			break;

		case GL_LIGHTING:
			lighting_enabled = GL_FALSE;
			break;

		case GL_LIGHT0:
			lighting.lights[cap - GL_LIGHT0].enabled = GL_FALSE;
			break;

		default:
			if (cap != GL_DEPTH_TEST)
				fprintf(stderr, "%s() TODO 0x%x\n", __FUNCTION__, cap);
			//opengl_disp.disable(opengl_rend, cap);
	}
}

static void
gl_depth_func(GLIContext ctx, GLenum func)
{
	draw_options.depth_func = func;
}

static void
gl_polygon_mode(GLIContext ctx, GLenum face, GLenum mode)
{
	switch (face)
	{
		case GL_FRONT:
			draw_options.polygon_modes[0] = mode;
			break;
		case GL_BACK:
			draw_options.polygon_modes[1] = mode;
			break;
		case GL_FRONT_AND_BACK:
			draw_options.polygon_modes[0] = draw_options.polygon_modes[1] = mode;
			break;
		default:
			assert(!"Invalid polygon face");
	}
}

static void
gl_push_attrib(GLIContext ctx, GLbitfield mask)
{
	assert(saved_attrib_depth < number_of(saved_attrib_stack));
    if (mask & GL_ENABLE_BIT)
	{
		saved_attrib_stack[saved_attrib_depth].lighting_enabled = lighting_enabled;
		saved_attrib_stack[saved_attrib_depth].test_depth = draw_options.test_depth;
	}
    saved_attrib_stack[saved_attrib_depth].bits = mask;
	++saved_attrib_depth;
	fprintf(stderr, "TODO: push_attrib() 0x%x\n", mask);
    //opengl_disp.push_attrib(opengl_rend, mask);
}

static void
gl_pop_attrib(GLIContext ctx)
{
	assert(saved_attrib_depth > 0);
	--saved_attrib_depth;
	if (saved_attrib_stack[saved_attrib_depth].bits & GL_ENABLE_BIT)
	{
		lighting_enabled = saved_attrib_stack[saved_attrib_depth].lighting_enabled;
		draw_options.test_depth = saved_attrib_stack[saved_attrib_depth].test_depth;
	}
	fprintf(stderr, "TODO: pop_attrib()\n");
	//opengl_disp.pop_attrib(opengl_rend);
}

static void
gl_light_modelf(GLIContext ctx, GLenum pname, GLfloat param)
{
	fprintf(stderr, "TODO: light_modelf()\n");
}

static void
gl_light_modelfv(GLIContext ctx, GLenum pname, const GLfloat *params)
{
	switch (pname)
	{
		case GL_LIGHT_MODEL_AMBIENT:
			lighting.global_ambient = vector4_from_float_array(params);
			break;
		case GL_LIGHT_MODEL_LOCAL_VIEWER:
			lighting.local_viewer = (GLuint)params[0];
			break;
		default:
			fprintf(stderr, "TODO: light_modelfv() 0x%x\n", pname);
	}
}

static void
gl_light_modeli(GLIContext ctx, GLenum pname, GLint param)
{
	GLfloat f = param;
	gl_light_modelfv(ctx, pname, &f);
}

static void
gl_light_modeliv(GLIContext ctx, GLenum pname, const GLint *params)
{
	GLfloat fv[4];
	switch (pname)
	{
		case GL_LIGHT_MODEL_AMBIENT:
			for (GLuint i = 0; i < 4; ++i)
				fv[i] = params[i];
			break;
		default:
			fprintf(stderr, "TODO: light_modeliv()\n");
			return;
	}
	gl_light_modelfv(ctx, pname, fv);
}

static void
gl_lightfv(GLIContext rend, GLenum light, GLenum pname, const GLfloat *params)
{
	switch (pname)
	{
		case GL_AMBIENT:
			lighting.lights[light - GL_LIGHT0].ambient = vector4_from_float_array(params);
			break;
		case GL_DIFFUSE:
			lighting.lights[light - GL_LIGHT0].diffuse = vector4_from_float_array(params);
			break;
		case GL_SPECULAR:
			lighting.lights[light - GL_LIGHT0].specular = vector4_from_float_array(params);
			break;
		case GL_POSITION:
		{
			struct vector4 pos = vector4_from_float_array(params);
			lighting.lights[light - GL_LIGHT0].pos = matrix4x4_mult_vector4(modelview_stack[modelview_depth], pos);
			break;
		}
		default:
			fprintf(stderr, "%s() TODO 0x%x\n", __FUNCTION__, pname);
	}
}

static void
gl_matrix_mode(GLIContext rend, GLenum mode)
{
	matrix_mode = mode;
	//openGLMatrixMode(mode);
}

static void
gl_load_identity(GLIContext rend)
{
	struct matrix4x4 identity = IDENTITY_MATRIX4X4;
	switch (matrix_mode)
	{
		case GL_MODELVIEW:
			modelview_stack[modelview_depth] = identity;
			break;
		case GL_PROJECTION:
			projection_stack[projection_depth] = identity;
			break;
		default:
			fprintf(stderr, "%s() TODO\n", __FUNCTION__);
			//opengl_disp.load_identity(opengl_rend);
	}
}

static void
gl_mult_matrixd(GLIContext ctx, const GLdouble *ma)
{
	struct matrix4x4 m;
	matrix4x4_copy_linear(ma, &m);
	struct matrix4x4 *target;
	switch (matrix_mode)
	{
		case GL_MODELVIEW:
			target = &(modelview_stack[modelview_depth]);
			break;
		case GL_PROJECTION:
			target = &(projection_stack[projection_depth]);
			break;
		default:
			fprintf(stderr, "%s() TODO\n", __FUNCTION__);
			return;
	}
	*target = matrix4x4_mult_matrix4x4(*target, m);
}

static void
gl_push_matrix(GLIContext ctx)
{
    switch (matrix_mode)
	{
		case GL_MODELVIEW:
            assert(modelview_depth < number_of(modelview_stack));
			modelview_stack[modelview_depth + 1] = modelview_stack[modelview_depth];
			++modelview_depth;
			break;
		case GL_PROJECTION:
			assert(projection_depth < number_of(projection_stack));
			projection_stack[projection_depth + 1] = projection_stack[projection_depth];
			++projection_depth;
			break;
		default:
            fprintf(stderr, "%s(): TODO\n", __FUNCTION__);
			return;
	}
}

static void
gl_pop_matrix(GLIContext ctx)
{
	switch (matrix_mode)
	{
		case GL_MODELVIEW:
			assert(modelview_depth > 0);
			--modelview_depth;
			break;
		case GL_PROJECTION:
			assert(projection_depth > 0);
			--projection_depth;
			break;
		default:
			fprintf(stderr, "TODO: pop_matrix()\n");
            return;
	}
}

static void
gl_scalef(GLIContext ctx, GLfloat x, GLfloat y, GLfloat z)
{
    struct matrix4x4 m = matrix4x4_make_scaling(x, y, z);
	gl_mult_matrixd(ctx, m.m);
}

static void
gl_rotated(GLIContext rend, GLdouble angle, GLdouble x, GLdouble y, GLdouble z)
{
	struct matrix4x4 m = matrix4x4_make_rotation(angle, x, y, z);
	gl_mult_matrixd(rend, m.m);
}

static void
gl_rotatef(GLIContext rend, GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
    gl_rotated(rend, angle, x, y, z);
}

static void
gl_translated(GLIContext rend, GLdouble x, GLdouble y, GLdouble z)
{
	struct matrix4x4 m = IDENTITY_MATRIX4X4;
	m.cols[3][0] = x;
	m.cols[3][1] = y;
	m.cols[3][2] = z;
	gl_mult_matrixd(rend, m.m);
}

static void
gl_translatef(GLIContext rend, GLfloat x, GLfloat y, GLfloat z)
{
	gl_translated(rend, x, y, z);
}

static void
gl_clear_color(GLIContext rend, GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
	clear_color.r = red;
	clear_color.b = blue;
	clear_color.g = green;
	clear_color.a = alpha;
}

static void
gl_flush(GLIContext rend)
{
	draw_flush(drawable);
	primitive_index = 0;

	debug_disp->flush(debug_rend);

}

static void
gl_finish(GLIContext rend)
{
	draw_finish(drawable);
	primitive_index = 0;

	debug_disp->finish(debug_rend);
}

static void
gl_ortho(GLIContext rend, GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
	GLdouble width = right - left, height = top - bottom, depth = zFar - zNear;
	struct matrix4x4 m;
	bzero(&m, sizeof(m));
	m.cols[0][0] = 2.0 / width;
	m.cols[1][1] = 2.0 / height;
	m.cols[2][2] = -2.0 / depth;
	m.cols[3][0] = -(right + left) / width;
	m.cols[3][1] = -(top + bottom) / height;
	m.cols[3][2] = -(zFar + zNear) / depth;
	m.cols[3][3] = 1.0;
	gl_mult_matrixd(rend, m.m);
}

static void
gl_frustum(GLIContext rend, GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
	/*
	GLdouble fovy_rad = fovy / (180.0 / M_PI);
	GLdouble f = 1.0 / tan(fovy_rad / 2.0);
	*/
	struct matrix4x4 m;
	bzero(&m, sizeof(m));
	//m.cols[0][0] = f / aspect;
	m.cols[0][0] = (2 * zNear) / (right - left);
	//m.cols[1][1] = f;
	m.cols[1][1] = (2 * zNear) / (top - bottom);
	//m.cols[2][2] = (zFar + zNear) / (zNear - zFar);
	m.cols[2][0] = (right + left) / (right - left);
	m.cols[2][1] = (top + bottom) / (top - bottom);
	m.cols[2][2] = -(zFar + zNear) / (zFar - zNear);
	m.cols[2][3] = -1;
	m.cols[3][2] = -(2 * zFar * zNear) / (zFar - zNear);
	gl_mult_matrixd(rend, m.m);
}

static void
gl_viewport(GLIContext rend, GLint x, GLint y, GLsizei width, GLsizei height)
{
	draw_set_view(drawable, x, y, width, height);

	debug_disp->viewport(debug_rend, x, y, width, height);
}

static void
gl_shade_model(GLIContext rend, GLenum mode)
{
	if (mode != GL_SMOOTH)
		fprintf(stderr, "%s() TODO\n", __FUNCTION__);
	//opengl_disp.shade_model(opengl_rend, mode);
}

static GLenum
gl_get_error(GLIContext ctx)
{
	return GL_NO_ERROR;
}

static void
gl_swap_APPLE(GLIContext ctx)
{
	gl_finish(ctx);
	draw_flip(drawable);
}

/* GLU */
/*
void
gluLookAt(GLdouble eyeX,
		  GLdouble eyeY,
		  GLdouble eyeZ,
		  GLdouble centerX,
		  GLdouble centerY,
		  GLdouble centerZ,
		  GLdouble upX,
		  GLdouble upY,
		  GLdouble upZ)
{
	vec3_t forward = {centerX - eyeX, centerY - eyeY, centerZ - eyeZ};
	vec3_norm(forward, forward);
	vec3_t up = {upX, upY, upZ};
	vec3_norm(up, up);
	vec3_t s;
	vec3_cross(forward, up, s);
	vec3_t u;
	vec3_cross(s, forward, u);
	struct matrix4x4 m;
	for (GLuint col = 0; col < 3; ++col)
	{
		m.cols[col][0] = s[col];
		m.cols[col][1] = u[col];
		m.cols[col][2] = -forward[col];
		m.cols[col][3] = 0;
	}
	for (GLuint i = 0; i < 3; ++i)
		m.cols[3][i] = 0;
	m.cols[3][3] = 1;
	render_mult_matrix(matrix_mode, m);
	glTranslatef(-eyeX, -eyeY, -eyeZ);
}

void
gluOrtho2D(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top)
{
	glOrtho(left, right, bottom, top, -1, 1);
}

void
gluPerspective(GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar)
{
	GLdouble fovy_rad = fovy / (180.0 / M_PI);
	GLdouble f = 1.0 / tan(fovy_rad / 2.0);
	struct matrix4x4 m;
	bzero(&m, sizeof(m));
	m.cols[0][0] = f / aspect;
	m.cols[1][1] = f;
	m.cols[2][2] = (zFar + zNear) / (zNear - zFar);
	m.cols[2][3] = -1;
	m.cols[3][2] = (2 * zFar * zNear) / (zNear - zFar);
	render_mult_matrix(matrix_mode, m);
}
*/

/* GLUT */
void *glutStrokeRoman;

struct window *main_window = NULL;
int glut_main_win = -1;
struct {int x, y;} main_win_pos;
struct {int width, height;} init_win_size;
int debug_win = -1;

void (*reshape_func)(int width, int height);
void (*idle_func)(void);

static void
reshape(int width, int height)
{
	draw_reshape(drawable, width, height);

	if (reshape_func)
		reshape_func(width, height);
	else
		glViewport(0, 0, width, height);

	render_push_debug();
	openGLUTReshapeWindow(width, height);
	openGLUTPositionWindow(main_win_pos.x + width + 20, main_win_pos.y);
	render_pop_debug();
}

static void
display_debug(void)
{
	//glutPostRedisplay();
	//glutSwapBuffers();
}

void
debug_key(unsigned char key, int x, int y)
{
	switch (key)
	{
		case '\t':
			debug_mode = (debug_mode + 1) % DEBUG_NMODES;
			break;
		case 0x19:
			debug_mode = (debug_mode + DEBUG_NMODES - 1) % DEBUG_NMODES;
			break;
		case 'c':
			debug_color = !debug_color;
			break;
		case 'p':
			++debug_primitive_index;
			break;
		case 'P':
			--debug_primitive_index;
			break;
		case 'l':
			if (++debug_light_index == max_lights)
				debug_light_index = -1;
			break;
		case 'L':
			if (debug_light_index == -1)
				debug_light_index = max_lights;
			--debug_light_index;
			break;
		case 'z':
			debug_zoom = fmin(3.0, debug_zoom + 0.1);
			break;
		case 'Z':
			debug_zoom = fmax(-6.0, debug_zoom - 0.1);
			break;
		default:
			return;
	}

	render_update_debug_title();
	openGLUTPostWindowRedisplay(glut_main_win);
}

static void
debug_idle(void)
{
	if (openGLUTGetWindow() == debug_win)
		openGLUTSetWindow(glut_main_win);

	idle_func();
}

void
glutInit(int *argcp, char **argv)
{
	openglut_init();

	openGLUTInit(argcp, argv);

	//openGLUTReshapeFunc(reshape);
	//openGLUTInitWindowPosition(20, 20);
	glutInitWindowPosition(20, 20);
	glutInitWindowSize(300, 300);
}

void
glutInitWindowPosition(int x, int y)
{
	main_win_pos.x = x;
	main_win_pos.y = y;
	openGLUTInitWindowPosition(x, y);
}

void
glutInitWindowSize(int width, int height)
{
	init_win_size.width = width;
	init_win_size.height = height;
	openGLUTInitWindowSize(width, height);
}

void
glutMainLoop(void)
{
	if (debug_win == -1)
	{
		openGLUTInitWindowPosition(main_win_pos.x + init_win_size.width + 20, main_win_pos.y);
		openGLUTInitWindowSize(init_win_size.width, init_win_size.height);
		openGLUTInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
		debug_win = openGLUTCreateWindow("Debug");
		render_update_debug_title();
		CGLContextObj context = CGLGetCurrentContext();
		debug_rend = context->rend;
		debug_disp = &(context->disp);

		openGLUTDisplayFunc(display_debug);
		openGLUTKeyboardFunc(debug_key);

		glMatrixMode(GL_PROJECTION);
		glOrtho(-1, 1, -1, 1, -10, 10);
		glMatrixMode(GL_MODELVIEW);
		openGLUTSetWindow(glut_main_win);
	}

	openGLUTMainLoop();
}

int
glutCreateWindow(const char *title)
{
	assert(glut_main_win == -1);
	glut_main_win = openGLUTCreateWindow(title);
	CGLContextObj context = CGLGetCurrentContext();
	main_window = window_create_cgl(context);
	//main_win->width = init_win_size.width;
	//main_win->height = init_win_size.height;
	drawable = draw_create(main_window);
	draw_reshape(drawable, init_win_size.width, init_win_size.height);
#include "gl_setup_ctx.c"
	return glut_main_win;
}

int
glutGetWindow(void)
{
	int win = openGLUTGetWindow();
	return (win == debug_win) ? DEBUG_WIN : win;
}

void
glutSetWindow(int win)
{
	assert(win != DEBUG_WIN || debug_win != -1);
	openGLUTSetWindow((win == DEBUG_WIN) ? debug_win : win);
}

void
glutReshapeFunc(void (*func)(int width, int height))
{
	reshape_func = func;
	openGLUTReshapeFunc(reshape);
}

void
glutIdleFunc(void (*fp)(void))
{
	idle_func = fp;
	openGLUTIdleFunc(debug_idle);
}

void
glutSwapBuffers(void)
{
	draw_finish(drawable);
	openGLUTSwapBuffers();

	render_push_debug();
	openGLUTSwapBuffers();
	render_pop_debug();
}

void
glutPostRedisplay(void)
{
	if (openGLUTGetWindow() == debug_win)
		openGLUTPostWindowRedisplay(glut_main_win);
	else
		openGLUTPostRedisplay();
}
