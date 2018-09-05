#include "gl.h"
#include <GLUT/glut.h>
#include <OpenGL/CGLContext.h>
#include <OpenGL/CGLCurrent.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <strings.h>

#include "vector.h"
#include "matrix.h"
#include "render.h"
#include "draw.h"
#include "macro.h"

#include "gl_stubs.c"

static struct vector4 clear_color = {.r = 0, .g = 0, .b = 0, .a = 0};
static GLenum matrix_mode = GL_MODELVIEW;
static struct matrix4x4 modelview_stack[32] = {IDENTITY_MATRIX4X4};
static GLuint modelview_depth = 0;
static struct matrix4x4 projection_stack[2] = {IDENTITY_MATRIX4X4};
static GLuint projection_depth = 0;
static GLenum primitive_mode;
static GLboolean triangle_strip_winding = GL_FALSE;

#define DEFAULT_LIGHT {\
        .enabled = false,\
        .pos = {.x = 0, .y = 0, .z = 1, .w = 0},\
        .diffuse = {.v = {0, 0, 0, 0}},\
        .specular = {.v = {0, 0, 0, 0}}\
    }

static struct render_options render_options =
{
		.smooth_shading = true,
		.lighting_enabled = false,
		.lighting =
				{
						.global_ambient = {.r = 0.2, .g = 0.2, .b = 0.2, .a = 1.0},
						.local_viewer = false,
						.lights =
								{
										{
												.enabled = false,
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
				},
		.draw_options =
				{
						.draw_op = GL_COPY,
						.test_depth = false,
						.depth_func = GL_LESS,
						.polygon_modes = {GL_FILL, GL_FILL},
						.cull_faces = false,
						.faces_culled = {false, true}
				},
};
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

static const GLuint max_attrib_depth = 3;
static struct saved_attrib
{
	GLbitfield bits;
	GLboolean test_depth;
	GLboolean lighting_enabled;
	GLboolean lights_enabled[MAX_LIGHTS];
	GLboolean cull_faces;
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
	render_options.modelview.matrix = modelview_stack[modelview_depth];
	render_options.modelview.inverse_trans = matrix4x4_invert_trans(modelview_stack[modelview_depth]);
	render_options.proj = projection_stack[projection_depth];
	render_primitive(&render_options, vertices, indices, num_indices);
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

	if (debug_rend)
		debug_disp->clear(debug_rend, mask & GL_COLOR_BUFFER_BIT);
}

static void
gl_enable(GLIContext rend, GLenum cap)
{
	switch (cap)
	{
		case GL_DEPTH_TEST:
			render_options.draw_options.test_depth = true;
			break;

		case GL_LIGHTING:
			render_options.lighting_enabled = GL_TRUE;
			break;

		case GL_LIGHT0:
			render_options.lighting.lights[cap - GL_LIGHT0].enabled = GL_TRUE;
			break;

		case GL_CULL_FACE:
			render_options.draw_options.cull_faces = true;
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
			render_options.draw_options.test_depth = false;
			break;

		case GL_LIGHTING:
			render_options.lighting_enabled = GL_FALSE;
			break;

		case GL_LIGHT0:
			render_options.lighting.lights[cap - GL_LIGHT0].enabled = GL_FALSE;
			break;

		case GL_CULL_FACE:
			render_options.draw_options.cull_faces = false;
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
	render_options.draw_options.depth_func = func;
}

static void
gl_polygon_mode(GLIContext ctx, GLenum face, GLenum mode)
{
	switch (face)
	{
		case GL_FRONT:
			render_options.draw_options.polygon_modes[0] = mode;
			break;
		case GL_BACK:
			render_options.draw_options.polygon_modes[1] = mode;
			break;
		case GL_FRONT_AND_BACK:
			render_options.draw_options.polygon_modes[0] = render_options.draw_options.polygon_modes[1] = mode;
			break;
		default:
			assert(!"Invalid polygon face");
	}
}

static void
gl_push_attrib(GLIContext ctx, GLbitfield mask)
{
	assert(saved_attrib_depth < number_of(saved_attrib_stack));
	struct saved_attrib *attrib = &(saved_attrib_stack[saved_attrib_depth]);
	attrib->bits = 0;
    if (mask & GL_ENABLE_BIT)
	{
		attrib->test_depth = render_options.draw_options.test_depth;
		attrib->lighting_enabled = render_options.lighting_enabled;
		for (u_int light = 0; light < MAX_LIGHTS; ++light)
			attrib->lights_enabled[light] = render_options.lighting.lights[light].enabled;
		attrib->cull_faces = render_options.draw_options.cull_faces;
		attrib->bits|= GL_ENABLE_BIT;
	}
	++saved_attrib_depth;
	if (mask & ~attrib->bits)
		fprintf(stderr, "TODO: push_attrib(0x%x)\n", mask & ~attrib->bits);
}

static void
gl_pop_attrib(GLIContext ctx)
{
	assert(saved_attrib_depth > 0);
	--saved_attrib_depth;
	struct saved_attrib *attrib = &(saved_attrib_stack[saved_attrib_depth]);
	if (attrib->bits & GL_ENABLE_BIT)
	{
		render_options.draw_options.test_depth = attrib->test_depth;
		render_options.lighting_enabled = attrib->lighting_enabled;
		for (u_int light = 0; light < MAX_LIGHTS; ++light)
			render_options.lighting.lights[light].enabled = attrib->lights_enabled[light];
		render_options.draw_options.cull_faces = attrib->cull_faces;
	}
	attrib->bits&= ~GL_ENABLE_BIT;
	if (attrib->bits)
		fprintf(stderr, "TODO: pop_attrib(0x%x)\n", attrib->bits);
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
			render_options.lighting.global_ambient = vector4_from_float_array(params);
			break;
		case GL_LIGHT_MODEL_LOCAL_VIEWER:
			render_options.lighting.local_viewer = (params[0] != 0.0);
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
			render_options.lighting.lights[light - GL_LIGHT0].ambient = vector4_from_float_array(params);
			break;
		case GL_DIFFUSE:
			render_options.lighting.lights[light - GL_LIGHT0].diffuse = vector4_from_float_array(params);
			break;
		case GL_SPECULAR:
			render_options.lighting.lights[light - GL_LIGHT0].specular = vector4_from_float_array(params);
			break;
		case GL_POSITION:
		{
			struct vector4 pos = vector4_from_float_array(params);
			render_options.lighting.lights[light - GL_LIGHT0].pos =
					matrix4x4_mult_vector4(modelview_stack[modelview_depth], pos);
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

	if (debug_rend)
		debug_disp->flush(debug_rend);
}

static void
gl_finish(GLIContext rend)
{
	draw_finish(drawable);
	primitive_index = 0;

	if (debug_rend)
		debug_disp->finish(debug_rend);
}

static void
gl_ortho(GLIContext rend, GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near, GLdouble far)
{
	struct matrix4x4 m = matrix4x4_make_ortho(left, right, bottom, top, -near, -far);
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

	if (debug_rend)
		debug_disp->viewport(debug_rend, x, y, width, height);
}

static void
gl_shade_model(GLIContext rend, GLenum mode)
{
	render_options.smooth_shading = (mode == GL_SMOOTH);
}

static GLenum
gl_get_error(GLIContext ctx)
{
	return GL_NO_ERROR;
}

static void
gl_swap_APPLE(GLIContext ctx)
{
	draw_flip(drawable);
}

void
gl_setup_context(CGLContextObj context)
{
#include "gl_setup_ctx.c"
}