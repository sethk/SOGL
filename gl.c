#include <GLUT/glut.h>
#include <OpenGL/CGLContext.h>
#include <OpenGL/CGLCurrent.h>
#include <OpenGL/gliDispatch.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <strings.h>
#include <dlfcn.h>
#include <err.h>

#include "wrap_glut.h"
#include "vector.h"
#include "matrix.h"

#define number_of(a) (sizeof(a) / sizeof(*(a)))

#define DEBUG_WIN (-5)

#define MAX_PRIMITIVE_VERTICES (8)

/* Render */
struct modelview
{
	struct matrix4x4 matrix;
	struct matrix4x4 inverse_trans;
};

struct projection
{
	struct matrix4x4 matrix;
	vec4_t world_eye_pos;
};

struct vertex
{
	vec4_t pos;
	vec3_t norm;
	vec4_t color;
	vec4_t ambient;
	vec4_t diffuse;
	vec4_t specular;
	GLfloat shininess;
};

static const GLuint max_lights = 8;
struct lighting
{
	vec4_t global_ambient;
    struct
    {
        GLboolean enabled;
	    vec4_t ambient;
        vec4_t diffuse;
        vec4_t specular;
	    vec4_t pos;
    } lights[max_lights];
};

struct shaded_vertex
{
	vec4_t world_pos;
	vec4_t view_pos;
	vec3_t world_norm;
	vec3_t light_dirs[max_lights];
	vec3_t lighting_eye_dir;
};

GLIContext opengl_rend;
GLIFunctionDispatch opengl_disp;
static const vec4_t origin = {0, 0, 0, 1};
static GLuint primitive_index;

int debug_save_win;
GLIContext debug_rend;
GLIFunctionDispatch *debug_disp;
enum {DEBUG_FRONT, DEBUG_LEFT, DEBUG_TOP, DEBUG_PROJECTION, DEBUG_NMODES} debug_mode = 0;
static GLint debug_primitive_index = -1;
static GLint debug_light_index = -1;
static GLdouble debug_zoom = 0;
static struct projection debug_proj;

static void
render_update_debug_title(void)
{
	char title[64];
	char *mode = NULL;
	switch (debug_mode)
	{
		case DEBUG_FRONT: mode = "Front"; break;
		case DEBUG_LEFT: mode = "Left"; break;
		case DEBUG_TOP: mode = "Top"; break;
		case DEBUG_PROJECTION: mode = "Projection"; break;
		case DEBUG_NMODES: break;
	}
	snprintf(title, sizeof(title), "Debug - %s @ %.0f%%", mode, pow(2, debug_zoom) * 100);
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
render_update_debug_proj(const struct projection proj)
{
	GLfloat scale = pow(2, debug_zoom);
	struct matrix4x4 scaling = matrix4x4_make_scaling(scale, scale, scale);
	switch (debug_mode)
	{
		case DEBUG_FRONT:
			debug_proj.matrix = matrix4x4_make_scaling(scale, scale, scale);
			break;
		case DEBUG_LEFT:
			debug_proj.matrix = matrix4x4_make_rotation(90, 0, 1, 0);
			debug_proj.matrix = matrix4x4_mult_matrix4x4(debug_proj.matrix, scaling);
			break;
		case DEBUG_TOP:
			debug_proj.matrix = matrix4x4_make_rotation(90, 1, 0, 0);
			debug_proj.matrix = matrix4x4_mult_matrix4x4(debug_proj.matrix, scaling);
			break;
		case DEBUG_PROJECTION:
		{
			debug_proj.matrix = matrix4x4_mult_matrix4x4(proj.matrix, scaling);
			break;
		}
		case DEBUG_NMODES:
			break;
	}
	vec4_copy(proj.world_eye_pos, debug_proj.world_eye_pos);
}

static void
render_axes_debug()
{
	debug_disp->begin(debug_rend, GL_LINES);

	vec4_t view_origin;
	matrix4x4_mult_vec4(debug_proj.matrix, origin, view_origin);

	const GLdouble axes_size = 0.2;
	debug_disp->color3f(debug_rend, 1, 1, 1);
	debug_disp->vertex4dv(debug_rend, view_origin);
	vec4_t right = {axes_size, 0, 0, 1};
	matrix4x4_mult_vec4(debug_proj.matrix, right, right);
	debug_disp->color3f(debug_rend, 0.5, 0.5, 1);
	debug_disp->vertex4dv(debug_rend, right);

	debug_disp->color3f(debug_rend, 1, 1, 1);
	debug_disp->vertex4dv(debug_rend, view_origin);
	vec4_t up = {0, axes_size, 0, 1};
	matrix4x4_mult_vec4(debug_proj.matrix, up, up);
	debug_disp->color3f(debug_rend, 1.0, 0.5, 0.5);
	debug_disp->vertex4dv(debug_rend, up);

	debug_disp->color3f(debug_rend, 1, 1, 1);
	debug_disp->vertex4dv(debug_rend, view_origin);
	vec4_t forward = {0, 0, axes_size, 1};
	matrix4x4_mult_vec4(debug_proj.matrix, forward, forward);
	debug_disp->color3f(debug_rend, 0.5, 1, 0.5);
	debug_disp->vertex4dv(debug_rend, forward);

	debug_disp->end(debug_rend);
}

static void
render_frustum_debug(struct projection proj)
{
	debug_disp->point_size(debug_rend, 5);
	debug_disp->begin(debug_rend, GL_POINTS);
	debug_disp->color3f(debug_rend, 0, 1, 0);
	vec4_t eye_pos;
	matrix4x4_mult_vec4(debug_proj.matrix, proj.world_eye_pos, eye_pos);
	debug_disp->vertex4dv(debug_rend, eye_pos);
	debug_disp->end(debug_rend);
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

static void
render_shade_vertex(const struct modelview modelview,
                    const struct projection proj,
                    const struct vertex vertex,
                    const struct lighting *lighting,
                    struct shaded_vertex *shaded)
{
	matrix4x4_mult_vec4(modelview.matrix, vertex.pos, shaded->world_pos);
	matrix4x4_mult_vec4(proj.matrix, shaded->world_pos, shaded->view_pos);

	vec4_t norm4;
	vec3_copy_vec4(vertex.norm, 0, norm4);
	matrix4x4_mult_vec4(modelview.inverse_trans, norm4, shaded->world_norm);
	vec3_check_norm(shaded->world_norm, "world_norm");

	if (lighting)
	{
		for (GLuint i = 0; i < number_of(lighting->lights); ++i)
		{
			if (!lighting->lights[i].enabled)
				continue;

			if (lighting->lights[i].pos[3] == 0)
				vec3_norm(lighting->lights[i].pos, shaded->light_dirs[i]);
			else
			{
				vec4_t light_dir;
				vec4_sub(lighting->lights[i].pos, shaded->world_pos, light_dir);
				vec3_norm(light_dir, shaded->light_dirs[i]);
			}
		}

		vec4_t eye_dir4;
		vec4_sub(proj.world_eye_pos, shaded->world_pos, eye_dir4);
		vec3_norm(eye_dir4, shaded->lighting_eye_dir);
	}
}

static void
render_primitive_debug(const struct modelview modelview,
                       const struct projection proj,
                       GLenum mode,
                       struct vertex *vertices,
                       GLuint num_vertices,
                       struct lighting *lighting)
{
	render_update_debug_proj(proj);
	struct shaded_vertex shaded_verts[MAX_PRIMITIVE_VERTICES];

	debug_disp->begin(debug_rend, (mode == GL_POINTS) ? GL_POINTS : GL_LINE_LOOP);
	debug_disp->color3f(debug_rend, 1, 1, 1);
	for (GLuint i = 0; i < num_vertices; ++i)
	{
		GLuint vert_index;
		if (mode == GL_QUAD_STRIP && i == 2)
			vert_index = 3;
		else if (mode == GL_QUAD_STRIP && i == 3)
			vert_index = 2;
		else
			vert_index = i;

		render_shade_vertex(modelview,
		                    debug_proj,
		                    vertices[vert_index],
		                    (debug_light_index != -1) ? lighting : NULL,
		                    &(shaded_verts[vert_index]));
		debug_disp->vertex4dv(debug_rend, shaded_verts[vert_index].view_pos);
	}
	debug_disp->end(debug_rend);

	debug_disp->begin(debug_rend, GL_LINES);
	for (GLuint i = 0; i < num_vertices; ++i)
	{
		struct shaded_vertex *shaded = &(shaded_verts[i]);
		debug_disp->color3f(debug_rend, 0, 1, 1);
		debug_disp->vertex4dv(debug_rend, shaded->view_pos);
		vec4_t vert_norm;
		vec3_mult_scalar(shaded->world_norm, 0.5, vert_norm);
		vert_norm[3] = 1.0;
		vec3_add(shaded->world_pos, vert_norm, vert_norm);
		matrix4x4_mult_vec4(debug_proj.matrix, vert_norm, vert_norm);
		debug_disp->vertex4dv(debug_rend, vert_norm);

		if (lighting && debug_light_index != -1 && lighting->lights[debug_light_index].enabled)
		{
			debug_disp->color3f(debug_rend, 1, 0.5, 0.5);
			debug_disp->vertex4dv(debug_rend, shaded->view_pos);
			vec4_t view_light;
			matrix4x4_mult_vec4(debug_proj.matrix, lighting->lights[debug_light_index].pos, view_light);
			debug_disp->vertex4dv(debug_rend, view_light);

			debug_disp->color3f(debug_rend, 1, 1, 0);
			debug_disp->vertex4dv(debug_rend, shaded->view_pos);
			vec4_t vert_light;
			vec3_mult_scalar(shaded->light_dirs[debug_light_index], 0.5, vert_light);
			vert_light[3] = 1.0;
			vec3_add(shaded->world_pos, vert_light, vert_light);
			matrix4x4_mult_vec4(debug_proj.matrix, vert_light, vert_light);
			debug_disp->vertex4dv(debug_rend, vert_light);

			debug_disp->color3f(debug_rend, 0, 1, 0);
			debug_disp->vertex4dv(debug_rend, shaded->view_pos);
			vec4_t vert_eye;
			vec3_mult_scalar(shaded->lighting_eye_dir, 0.5, vert_eye);
			vert_eye[3] = 1.0;
			vec3_add(shaded->world_pos, vert_eye, vert_eye);
			matrix4x4_mult_vec4(debug_proj.matrix, vert_eye, vert_eye);
			debug_disp->vertex4dv(debug_rend, vert_eye);

		}
	}
	debug_disp->end(debug_rend);

	render_axes_debug();
	render_frustum_debug(proj);
}

static void
render_primitive(const struct modelview modelview,
				 const struct projection proj,
				 GLenum mode,
				 struct vertex *vertices,
				 GLuint num_vertices,
				 struct lighting *lighting)
{
	if (debug_primitive_index == -1 || primitive_index == (GLuint)debug_primitive_index)
		render_primitive_debug(modelview, proj, mode, vertices, num_vertices, lighting);

	opengl_disp.begin(opengl_rend, mode);
	for (GLuint i = 0; i < num_vertices; ++i)
	{
		struct shaded_vertex shaded;
		render_shade_vertex(modelview, proj, vertices[i], lighting, &shaded);

		//render_shade_pixel(vertices[i], vert_lighting, &color);

		vec4_t color;
		if (lighting)
		{
			// Global ambient
			vec3_mult_vec3(lighting->global_ambient, vertices[i].ambient, color);
			color[3] = vertices[i].diffuse[3];

			for (GLuint light_index = 0; light_index < number_of(lighting->lights); ++light_index)
			{
				if (!lighting->lights[light_index].enabled)
					continue;

				// Ambient
				vec3_t ambient;
				vec3_mult_vec3(lighting->lights[light_index].ambient, vertices[i].ambient, ambient);
				vec3_add(color, ambient, color);

				// Diffuse
				GLdouble cos_theta = vec3_dot(shaded.world_norm, shaded.light_dirs[light_index]);
				GLdouble diff_mix = fmax(0, cos_theta);
				vec3_t diffuse;
				vec3_mult_scalar(lighting->lights[light_index].diffuse, diff_mix, diffuse);
				//vec3_print(diffuse);
				vec3_mult_vec3(diffuse, vertices[i].diffuse, diffuse);
				//vec3_print(color);
				vec3_add(color, diffuse, color);

				// Specular
				vec3_t half_dir;
				vec3_add(shaded.light_dirs[light_index], shaded.lighting_eye_dir, half_dir);
				vec3_norm(half_dir, half_dir);
				GLdouble cos_theta_half = vec3_dot(shaded.world_norm, half_dir);
				//fprintf(stderr, "cos_theta_half = %g\n", cos_theta_half);
				GLdouble spec_mix = pow(fmax(0, cos_theta_half), vertices[i].shininess);
				//fprintf(stderr, "spec_mix = %g\n", spec_mix);
				vec3_t specular;
				vec3_mult_scalar(lighting->lights[light_index].specular, spec_mix, specular);
				vec3_mult_vec3(specular, vertices[i].specular, specular);
				//vec3_print(specular);
				vec3_add(color, specular, color);
			}
		}
		else
			vec4_copy(vertices[i].color, color);
		opengl_disp.color4dv(opengl_rend, color);
		opengl_disp.vertex4dv(opengl_rend, shaded.view_pos);
	}
	opengl_disp.end(opengl_rend);

	++primitive_index;
}

/* GL */
#include "gl_stubs.c"

static GLenum matrix_mode = GL_MODELVIEW;
static struct matrix4x4 modelview_stack[32] = {IDENTITY_MATRIX4X4};
static GLuint modelview_depth = 0;
static struct matrix4x4 projection_stack[2] = {IDENTITY_MATRIX4X4};
static GLuint projection_depth = 0;
static GLenum primitive_mode;
static vec4_t color = {1, 1, 1, 1};
static vec3_t normal;
static vec4_t ambient = {0.2, 0.2, 0.2, 1.0};
static vec4_t diffuse = {0.8, 0.8, 0.8, 1.0};
static vec4_t specular = {0, 0, 0, 1};
static GLfloat shininess = 0;
static struct vertex vertices[MAX_PRIMITIVE_VERTICES];
static GLuint num_vertices;

#define DEFAULT_LIGHT {\
        .enabled = GL_FALSE,\
        .pos = {0, 0, 1, 0},\
        .diffuse = {0, 0, 0, 0},\
        .specular = {0, 0, 0, 0}\
    }

static GLboolean lighting_enabled = GL_FALSE;
static struct lighting lighting =
{
	.global_ambient = {0.2, 0.2, 0.2, 1.0},
	.lights =
	{
		{.enabled = GL_FALSE, .pos = {0, 0, 1, 0}, .diffuse = {1, 1, 1, 1}, .specular = {1, 1, 1, 1}},
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
} saved_attrib_stack[max_attrib_depth];
static GLuint saved_attrib_depth = 0;

static void
gl_begin(GLIContext rend, GLenum mode)
{
	primitive_mode = mode;
	num_vertices = 0;
}

static void
gl_color4dv(GLIContext rend, const GLdouble *c)
{
	vec4_copy(c, color);
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
gl_materialfv(GLIContext rend, GLenum face, GLenum pname, const GLfloat *params)
{
	switch (pname)
	{
		case GL_AMBIENT:
			vec4_copy_float(params, ambient);
			break;
		case GL_DIFFUSE:
			vec4_copy_float(params, diffuse);
			break;
		case GL_SPECULAR:
			vec4_copy_float(params, specular);
			break;
		case GL_SHININESS:
			shininess = params[0];
			break;
		default:
			fprintf(stderr, "%s() TODO %0x\n", __FUNCTION__, pname);
	}
}

static void
gl_normal3dv(GLIContext ctx, const GLdouble *v)
{
	vec3_copy(v, normal);
	vec3_check_norm(normal, "gl_normal");
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
	glNormal3dv(v);
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
gl_flush_primitive(void)
{
	struct modelview modelview;
	modelview.matrix = modelview_stack[modelview_depth];
	modelview.inverse_trans = matrix4x4_invert_trans(modelview_stack[modelview_depth]);

	struct projection proj;
	proj.matrix = projection_stack[projection_depth];
	struct matrix4x4 inv_proj;
	inv_proj = matrix4x4_invert(proj.matrix);
	matrix4x4_check_inverse(proj.matrix, inv_proj);
	vec4_t eye_pos = {0, 0, -1, 1};
	matrix4x4_mult_vec4(inv_proj, eye_pos, proj.world_eye_pos);

	render_primitive(modelview, proj, primitive_mode, vertices, num_vertices, (lighting_enabled) ? &lighting : NULL);

	switch (primitive_mode)
    {
        case GL_LINE_STRIP:
            bcopy(vertices + 1, vertices, sizeof(vertices[0]));
            num_vertices = 1;
            break;
        case GL_TRIANGLE_STRIP:
            bcopy(vertices + 1, vertices, sizeof(vertices[0]) * 2);
            num_vertices = 2;
            break;
        case GL_TRIANGLE_FAN:
            bcopy(vertices + 2, vertices + 1, sizeof(vertices[0]));
            num_vertices = 2;
            break;
        case GL_QUAD_STRIP:
            bcopy(vertices + 2, vertices, sizeof(vertices[0]) * 2);
            num_vertices = 2;
            break;
        default:
            num_vertices = 0;
    }
}

static void
gl_vertex4dv(GLIContext rend, const GLdouble *v)
{
	assert(num_vertices < number_of(vertices));
	vec4_copy(v, vertices[num_vertices].pos);
	vec3_copy(normal, vertices[num_vertices].norm);
	vec4_copy(color, vertices[num_vertices].color);
	vec4_copy(ambient, vertices[num_vertices].ambient);
	vec4_copy(diffuse, vertices[num_vertices].diffuse);
	vec4_copy(specular, vertices[num_vertices].specular);
	vertices[num_vertices].shininess = shininess;
	++num_vertices;

	if (primitive_mode == GL_POINTS ||
			((primitive_mode == GL_LINES || primitive_mode == GL_LINE_STRIP) && num_vertices == 2) ||
			(primitive_mode == GL_TRIANGLE_FAN && num_vertices == 3) ||
			((primitive_mode == GL_TRIANGLES || primitive_mode == GL_TRIANGLE_STRIP) && num_vertices == 3) ||
			((primitive_mode == GL_QUADS || primitive_mode == GL_QUAD_STRIP) && num_vertices == 4))
		gl_flush_primitive();
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
	fprintf(stderr, "TODO: vertex2fv()\n");
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
    if (primitive_mode == GL_LINE_LOOP || primitive_mode == GL_POLYGON)
        gl_flush_primitive();
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
gl_clear(GLIContext rend, GLbitfield mask)
{
	if (mask & ~(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT))
		fprintf(stderr, "%s() TODO: 0x%x\n", __FUNCTION__, mask & ~GL_COLOR_BUFFER_BIT);
	opengl_disp.clear(opengl_rend, mask);

	debug_disp->clear(debug_rend, mask & GL_COLOR_BUFFER_BIT);
}

static void
gl_enable(GLIContext rend, GLenum cap)
{
	switch (cap)
	{
		case GL_LIGHTING:
			lighting_enabled = GL_TRUE;
			break;

		case GL_LIGHT0:
			lighting.lights[cap - GL_LIGHT0].enabled = GL_TRUE;
			break;

		default:
			if (cap != GL_DEPTH_TEST)
				fprintf(stderr, "%s() TODO 0x%x\n", __FUNCTION__, cap);
			opengl_disp.enable(opengl_rend, cap);
	}
}

static void
gl_disable(GLIContext ctx, GLenum cap)
{
    switch (cap)
	{
		case GL_LIGHTING:
			lighting_enabled = GL_FALSE;
			break;

		case GL_LIGHT0:
			lighting.lights[cap - GL_LIGHT0].enabled = GL_FALSE;
			break;

		default:
			if (cap != GL_DEPTH_TEST)
				fprintf(stderr, "%s() TODO 0x%x\n", __FUNCTION__, cap);
			opengl_disp.disable(opengl_rend, cap);
	}
}

static void
gl_push_attrib(GLIContext ctx, GLbitfield mask)
{
	assert(saved_attrib_depth < number_of(saved_attrib_stack));
    if (mask & GL_ENABLE_BIT)
	{
		saved_attrib_stack[saved_attrib_depth].lighting_enabled = lighting_enabled;
	}
    saved_attrib_stack[saved_attrib_depth].bits = mask;
	++saved_attrib_depth;
	fprintf(stderr, "TODO: push_attrib()\n");
    opengl_disp.push_attrib(opengl_rend, mask);
}

static void
gl_pop_attrib(GLIContext ctx)
{
	assert(saved_attrib_depth > 0);
	--saved_attrib_depth;
	if (saved_attrib_stack[saved_attrib_depth].bits & GL_ENABLE_BIT)
	{
		lighting_enabled = saved_attrib_stack[saved_attrib_depth].lighting_enabled;
	}
	fprintf(stderr, "TODO: pop_attrib()\n");
	opengl_disp.pop_attrib(opengl_rend);
}

static void
gl_lightfv(GLIContext rend, GLenum light, GLenum pname, const GLfloat *params)
{
	switch (pname)
	{
		case GL_AMBIENT:
			vec4_copy_float(params, lighting.lights[light - GL_LIGHT0].ambient);
			break;
		case GL_DIFFUSE:
			vec4_copy_float(params, lighting.lights[light - GL_LIGHT0].diffuse);
			break;
		case GL_SPECULAR:
			vec4_copy_float(params, lighting.lights[light - GL_LIGHT0].specular);
			break;
		case GL_POSITION:
		{
			vec4_t pos;
			vec4_copy_float(params, pos);
			matrix4x4_mult_vec4(modelview_stack[modelview_depth], pos, lighting.lights[light - GL_LIGHT0].pos);
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
			opengl_disp.load_identity(opengl_rend);
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
	fprintf(stderr, "%s() TODO\n", __FUNCTION__);
	opengl_disp.clear_color(opengl_rend, red, green, blue, alpha);
}

static void
gl_flush(GLIContext rend)
{
	opengl_disp.flush(opengl_rend);
	primitive_index = 0;

	debug_disp->flush(debug_rend);

}

static void
gl_finish(GLIContext rend)
{
	opengl_disp.finish(rend);
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
	opengl_disp.viewport(opengl_rend, x, y, width, height);

	debug_disp->viewport(debug_rend, x, y, width, height);
}

static void
gl_shade_model(GLIContext rend, GLenum mode)
{
	fprintf(stderr, "%s() TODO\n", __FUNCTION__);
	opengl_disp.shade_model(opengl_rend, mode);
}

static void
gl_swap_APPLE(GLIContext ctx)
{
	opengl_disp.swap_APPLE(opengl_rend);
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

int main_win = -1;
struct {int x, y;} main_win_pos;
struct {int width, height;} main_win_size;
int debug_win = -1;

void (*reshape_func)(int width, int height);
void (*idle_func)(void);

static void
reshape(int width, int height)
{
	if (reshape_func)
		reshape_func(width, height);
	else
		glViewport(0, 0, width, height);

	main_win_size.width = width;
	main_win_size.height = height;
	render_push_debug();
	openGLUTReshapeWindow(main_win_size.width, main_win_size.height);
	openGLUTPositionWindow(main_win_pos.x + main_win_size.width + 20, main_win_pos.y);
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
		case 'p':
			if (debug_primitive_index < 12)
				++debug_primitive_index;
			else
				debug_primitive_index = -1;
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
	openGLUTPostWindowRedisplay(main_win);
}

static void
debug_idle(void)
{
	if (openGLUTGetWindow() == debug_win)
		openGLUTSetWindow(main_win);

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
	main_win_size.width = width;
	main_win_size.height = height;
	openGLUTInitWindowSize(width, height);
}

void
glutMainLoop(void)
{
	if (debug_win == -1)
	{
		openGLUTInitWindowPosition(main_win_pos.x + main_win_size.width + 20, main_win_pos.y);
		openGLUTInitWindowSize(main_win_size.width, main_win_size.height);
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
		openGLUTSetWindow(main_win);
	}

	openGLUTMainLoop();
}

int
glutCreateWindow(const char *title)
{
	assert(main_win == -1);
	main_win = openGLUTCreateWindow(title);
	CGLContextObj context = CGLGetCurrentContext();
	opengl_rend = context->rend;
	bcopy(&(context->disp), &opengl_disp, sizeof(context->disp));
#include "gl_setup_ctx.c"
	return main_win;
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
	gl_finish(opengl_rend);
	openGLUTSwapBuffers();

	render_push_debug();
	openGLUTSwapBuffers();
	render_pop_debug();
}

void
glutPostRedisplay(void)
{
	if (openGLUTGetWindow() == debug_win)
		openGLUTPostWindowRedisplay(main_win);
	else
		openGLUTPostRedisplay();
}
