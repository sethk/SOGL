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

#include "wrap_glut.c"

#define number_of(a) (sizeof(a) / sizeof(*(a)))

int main_win = -1;
int debug_win;
int debug_save_win;
enum {DEBUG_FRONT, DEBUG_LEFT, DEBUG_TOP, DEBUG_PROJECTION, DEBUG_NMODES} debug_mode = 0;
GLint debug_primitive_index = -1;
static GLdouble debug_zoom = 0;

typedef GLdouble vec3_t[3];
typedef GLdouble vec4_t[4];
struct matrix4x4
{
	union
	{
		GLdouble cols[4][4];
		GLdouble m[16];
	};
};
#define IDENTITY_MATRIX {.cols = {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}}

/* Vector */
static GLdouble
vec3_length(const vec3_t v)
{
	GLdouble sum_squares = 0;
	for (GLuint i = 0; i < 3; ++i)
		sum_squares+= v[i] * v[i];
	return sqrtf(sum_squares);
}

void
vec3_add(const vec3_t a, const vec3_t b, vec3_t rv)
{
	for (GLuint i = 0; i < 3; ++i)
		rv[i] = a[i] + b[i];
}

void
vec3_print(const vec3_t v)
{
	fprintf(stderr, "(%g, %g, %g)\n", v[0], v[1], v[2]);
}

void
vec3_mult_scalar(const vec3_t v, GLdouble mult, vec3_t rv)
{
	for (GLuint i = 0; i < 3; ++i)
		rv[i] = v[i] * mult;
}

void
vec3_mult_vec3(const vec3_t a, const vec3_t b, vec3_t rv)
{
	for (GLuint i = 0; i < 3; ++i)
		rv[i] = a[i] * b[i];
}

void
vec3_divide_scalar(const vec3_t v, GLdouble quot, vec3_t rv)
{
	for (GLuint i = 0; i < 3; ++i)
		rv[i] = v[i] / quot;
}

void
vec4_divide_scalar(const vec4_t v, GLdouble quot, vec4_t rv)
{
	for (GLuint i = 0; i < 4; ++i)
		rv[i] = v[i] / quot;
}

void
vec4_copy(const vec4_t v, vec4_t rv)
{
	for (GLuint i = 0; i < 4; ++i)
		rv[i] = v[i];
}

void
vec4_copy_float(const GLfloat *v, vec4_t rv)
{
	for (GLuint i = 0; i < 4; ++i)
		rv[i] = v[i];
}

void
vec3_norm(const vec3_t v, vec3_t rv)
{
	vec3_divide_scalar(v, vec3_length(v), rv);
}

void
vec3_check_norm(const vec3_t v)
{
	GLdouble length = vec3_length(v);
	if (fabs(1.0 - length) > 1.0e-7)
	{
		fprintf(stderr, "Vector not normalized (1.0 - length = %g): ", 1.0 - length);
		vec3_print(v);
	}
}

/* \   \   \     /   /   /
 * +ex +ey +ez -ex -ey -ez
 *  ax  ay  az  ax  ay  az
 *  bx  by  bz  bx  by  bz
 */
void
vec3_cross(const vec3_t a, const vec3_t b, vec3_t rv)
{
	for (GLuint i = 0; i < 3; ++i)
		rv[i] = a[(i + 1) % 3] * b[(i + 2) % 3] - a[(i + 2) % 3] * b[(i + 1) % 3];
}

GLdouble
vec3_dot(const vec3_t a, const vec3_t b)
{
	GLdouble dot = 0;
	for (GLuint i = 0; i < 3; ++i)
		dot+= a[i] * b[i];
	return dot;
}

GLdouble
vec4_dot(const vec4_t a, const vec4_t b)
{
	GLdouble dot = 0;
	for (GLuint i = 0; i < 4; ++i)
		dot+= a[i] * b[i];
	return dot;
}

void
vec4_add(const vec4_t a, const vec4_t b, vec4_t rv)
{
	for (GLuint i = 0; i < 4; ++i)
		rv[i] = a[i] + b[i];
}

void
vec4_mult_vec4(const vec4_t a, const vec4_t b, vec4_t rv)
{
	for (GLuint i = 0; i < 4; ++i)
		rv[i] = a[i] * b[i];
}

void
vec4_print(const vec4_t v)
{
	fprintf(stderr, "(%g, %g, %g, %g)\n", v[0], v[1], v[2], v[3]);
}

/* Matrix */
void
matrix4x4_copy_linear(const GLdouble *ma, struct matrix4x4 *rm)
{
	bcopy(ma, rm->m, sizeof(rm->m));
}

void
matrix4x4_mult_matrix4x4(const struct matrix4x4 m, const struct matrix4x4 n, struct matrix4x4 *rm)
{
	struct matrix4x4 result; // Allow multiply in place
	for (GLuint col = 0; col < 4; ++col)
		for (GLuint row = 0; row < 4; ++row)
		{
			GLdouble dot = 0;
			for (GLuint i = 0; i < 4; ++i)
				dot+= m.cols[i][row] * n.cols[col][i];
			result.cols[col][row] = dot;
		}
	*rm = result;
}

void
matrix4x4_mult_vec4(const struct matrix4x4 m, const vec4_t v, vec4_t rv)
{
	vec4_t result;
	for (GLuint row = 0; row < 4; ++row)
	{
		GLdouble dot = 0;
		for (GLuint i = 0; i < 4; ++i)
			dot+= m.cols[i][row] * v[i];
		result[row] = dot;
	}
	for (GLuint i = 0; i < 4; ++i)
		rv[i] = result[i];
}

struct matrix4x4
matrix4x4_make_scaling(GLdouble x, GLdouble y, GLdouble z)
{
	struct matrix4x4 m = {.cols = {{x, 0, 0, 0}, {0, y, 0, 0}, {0, 0, z, 0}, {0, 0, 0, 1}}};
	return m;
}

void
matrix4x4_print(struct matrix4x4 m)
{
	for (GLuint row = 0; row < 4; ++row)
		fprintf(stderr, "[ %4.4g, %4.4g, %4.4g, %4.4g ]\n",
				m.cols[0][row], m.cols[1][row], m.cols[2][row], m.cols[3][row]);
}

/* draw */
GLIContext opengl_rend;
GLIFunctionDispatch opengl_disp;
struct matrix4x4 modelview_matrix = IDENTITY_MATRIX;
struct matrix4x4 projection_matrix = IDENTITY_MATRIX;
static const vec4_t origin = {0, 0, 0, 1};
static vec4_t global_ambient = {0.2, 0.2, 0.2, 1.0};
static GLuint primitive_index;

struct vertex
{
	vec4_t pos;
	vec4_t norm;
	vec4_t color;
	vec4_t ambient;
	vec4_t diffuse;
	vec4_t specular;
	GLfloat shininess;
};

struct light
{
	bool enabled;
	vec4_t pos;
	vec3_t dir;
	vec4_t diffuse;
	vec4_t specular;
};

static void
draw_push_debug(void)
{
	debug_save_win = glutGetWindow();
	assert(debug_save_win != debug_win);
	openGLUTSetWindow(debug_win);
}

static void
draw_pop_debug(void)
{
	assert(glutGetWindow() == debug_win);
	openGLUTSetWindow(debug_save_win);
}

static struct matrix4x4
draw_get_debug_proj(void)
{
	struct matrix4x4 trans = IDENTITY_MATRIX;
	GLfloat debug_div = pow(2, debug_zoom);
	switch (debug_mode)
	{
		case DEBUG_FRONT:
			trans.cols[0][0] = 1.0 / debug_div;
			trans.cols[1][1] = 1.0 / debug_div;
			trans.cols[2][2] = 1.0 / debug_div;
			break;
		case DEBUG_LEFT:
			trans.cols[0][2] = 1.0 / debug_div;
			trans.cols[0][0] = 0;
			trans.cols[1][1] = 1.0 / debug_div;
			trans.cols[2][0] = 1.0 / debug_div;
			trans.cols[2][2] = 0;
			break;
		case DEBUG_TOP:
			trans.cols[0][1] = 1.0 / debug_div;
			trans.cols[0][0] = 0;
			trans.cols[1][0] = 1.0 / debug_div;
			trans.cols[1][1] = 0;
			trans.cols[2][2] = 1.0 / debug_div;
			break;
		case DEBUG_PROJECTION:
		{
			struct matrix4x4 scaling = matrix4x4_make_scaling(1.0 / debug_div, 1.0 / debug_div, 1.0 / debug_div);
			matrix4x4_mult_matrix4x4(scaling, projection_matrix, &trans);
			break;
		}
		case DEBUG_NMODES:
			break;
	}
	return trans;
}

static struct matrix4x4
draw_get_debug_trans(void)
{
	struct matrix4x4 trans;
	matrix4x4_mult_matrix4x4(draw_get_debug_proj(), modelview_matrix, &trans);
	return trans;
}

static void
draw_axes_debug(void)
{
	struct matrix4x4 trans = draw_get_debug_proj();

	glBegin(GL_LINES);

	vec4_t view_origin;
	matrix4x4_mult_vec4(trans, origin, view_origin);

	glColor3f(1, 1, 1);
	glVertex4dv(view_origin);
	vec4_t right = {5, 0, 0, 1};
	matrix4x4_mult_vec4(trans, right, right);
	glColor3f(0.5, 0.5, 1);
	glVertex4dv(right);

	glColor3f(1, 1, 1);
	glVertex4dv(view_origin);
	vec4_t up = {0, 5, 0, 1};
	matrix4x4_mult_vec4(trans, up, up);
	glColor3f(1.0, 0.5, 0.5);
	glVertex4dv(up);

	glColor3f(1, 1, 1);
	glVertex4dv(view_origin);
	vec4_t forward = {0, 0, 5, 1};
	matrix4x4_mult_vec4(trans, forward, forward);
	glColor3f(0.5, 1, 0.5);
	glVertex4dv(forward);

	glEnd();
}

/*
static void
draw_frustum_debug(void)
{
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
}
*/

/*
static void
draw_lights_debug(struct light *lights, GLuint num_lights)
{
	struct matrix4x4 trans = draw_get_debug_proj();

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
draw_primitive_debug(GLenum mode, struct vertex *vertices, GLuint num_vertices, struct light *lights, GLuint num_lights)
{
	struct matrix4x4 trans = draw_get_debug_trans();
	struct matrix4x4 debug_proj = draw_get_debug_proj();

	glBegin(GL_LINE_LOOP);
	glColor3f(1, 1, 1);
	for (GLuint i = 0; i < num_vertices; ++i)
	{
		vec4_t view_v;
		matrix4x4_mult_vec4(trans, vertices[i].pos, view_v);
		glVertex4dv(view_v);
	}
	glEnd();

	glBegin(GL_LINES);
	for (GLuint i = 0; i < num_vertices; ++i)
	{
		glColor3f(0, 1, 1);
		vec4_t view_v;
		matrix4x4_mult_vec4(trans, vertices[i].pos, view_v);
		//fprintf(stderr, "Vertex[%u]: ", i);
		//vec4_print(view_v);
		glVertex4dv(view_v);
		vec4_t vert_norm;
		vec3_mult_scalar(vertices[i].norm, 0.5, vert_norm);
		vert_norm[3] = 1.0;
		vec3_add(vertices[i].pos, vert_norm, vert_norm);
		matrix4x4_mult_vec4(trans, vert_norm, vert_norm);
		//fprintf(stderr, "Normal[%u]: ", i);
		//vec4_print(vert_norm);
		glVertex4dv(vert_norm);

		if (lights)
		{
			vec3_t world_normal;
			matrix4x4_mult_vec4(modelview_matrix, vertices[i].norm, world_normal);
			//vec3_print(world_normal);
			vec3_check_norm(world_normal);
			//vec3_print(lights[0].dir);
			vec3_check_norm(lights[0].dir);
			//GLdouble cos_theta = vec3_dot(world_normal, lights[0].dir);
			//fprintf(stderr, "%g\n", cos_theta);

			glColor3f(1, 1, 0);
			glVertex4dv(view_v);
			vec4_t vert_light;
			vec3_mult_scalar(lights[0].dir, 0.5, vert_light);
			vert_light[3] = 1.0;
			vec4_t world_v;
			matrix4x4_mult_vec4(modelview_matrix, vertices[i].pos, world_v);
			vec3_add(world_v, vert_light, vert_light);
			matrix4x4_mult_vec4(debug_proj, vert_light, vert_light);
			glVertex4dv(vert_light);

			glColor3f(1, 0.5, 0.5);
			glVertex4dv(view_v);
			vec4_t view_light;
			matrix4x4_mult_vec4(debug_proj, lights[0].pos, view_light);
			glVertex4dv(view_light);
		}
	}
	glEnd();
}

static void
draw_primitive(GLenum mode, struct vertex *vertices, GLuint num_vertices, struct light *lights, GLuint num_lights)
{
	if (debug_primitive_index == -1 || primitive_index == (GLuint)debug_primitive_index)
	{
		draw_push_debug();
		draw_primitive_debug(mode, vertices, num_vertices, lights, number_of(lights));
		draw_pop_debug();
	}

	opengl_disp.begin(opengl_rend, mode);
	for (GLuint i = 0; i < num_vertices; ++i)
	{
		vec4_t view_v;
		matrix4x4_mult_vec4(modelview_matrix, vertices[i].pos, view_v);
		matrix4x4_mult_vec4(projection_matrix, view_v, view_v);

		vec4_t color;
		if (lights)
		{
			// Global ambient
			bcopy(global_ambient, color, sizeof(global_ambient));
			vec4_mult_vec4(color, vertices[i].ambient, color);

			vec4_t world_normal;
			matrix4x4_mult_vec4(modelview_matrix, vertices[i].norm, world_normal);
			for (GLuint light_index = 0; light_index < num_lights; ++light_index)
			{
				if (!lights[light_index].enabled)
					continue;

				vec3_norm(world_normal, world_normal);
				GLdouble cos_theta = vec3_dot(world_normal, lights[light_index].dir);
				GLdouble mix = fmax(0, cos_theta);

				// Diffuse
				vec4_t diffuse;
				vec3_mult_scalar(lights[light_index].diffuse, mix, diffuse);
				diffuse[3] = 1.0;
				//vec4_print(diffuse);
				vec4_mult_vec4(diffuse, vertices[i].diffuse, diffuse);
				//vec4_print(color);
				vec4_add(color, diffuse, color);

				// Specular
				//vec4_sub(vertices[i].pos, view
			}
		}
		else
			bcopy(vertices[i].color, color, sizeof(color));
		opengl_disp.color4dv(opengl_rend, color);
		opengl_disp.vertex4dv(opengl_rend, view_v);
	}
	opengl_disp.end(opengl_rend);

	++primitive_index;
}

/* GL */
#include "gl_stubs.c"

static GLenum matrix_mode = GL_MODELVIEW;
static GLenum primitive_mode;
static vec3_t color = {1, 1, 1};
static vec3_t normal;
static vec4_t ambient = {0.2, 0.2, 0.2, 1.0};
static vec4_t diffuse = {0.8, 0.8, 0.8, 1.0};
static vec4_t specular = {0, 0, 0, 1};
static GLfloat shininess = 0;
static struct vertex vertices[4];
static GLuint num_vertices;
static bool lighting = false;
#define DEFAULT_LIGHT { \
		.enabled = false,\
		.pos = {0, 0, 1, 0},\
		.dir = {0, 0, 1},\
		.diffuse = {0, 0, 0, 0},\
		.specular = {0, 0, 0, 0}\
	}
static struct light lights[] =
{
	{.enabled = false, .pos = {0, 0, 1, 0}, .dir = {0, 0, 1}, .diffuse = {1, 1, 1, 1}, .specular = {1, 1, 1, 1}},
	DEFAULT_LIGHT,
	DEFAULT_LIGHT
};

static void
gl_begin(GLIContext rend, GLenum mode)
{
	primitive_mode = mode;
	if (mode != GL_TRIANGLES && mode != GL_TRIANGLE_STRIP && mode != GL_QUADS && mode != GL_QUAD_STRIP)
	{
		fprintf(stderr, "%s() TODO %u\n", __FUNCTION__, mode);
		return;
	}
	num_vertices = 0;
}

static void
gl_color3fv(GLIContext rend, const GLfloat *c)
{
	for (GLuint i = 0; i < 3; ++i)
		color[i] = c[i];
}

static void
gl_color3f(GLIContext rend, GLfloat red, GLfloat green, GLfloat blue)
{
	GLfloat c[3] = {red, green, blue};
	glColor3fv(c);
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
gl_normal3fv(GLIContext rend, const GLfloat *v)
{
	for (GLuint i = 0; i < 3; ++i)
		normal[i] = v[i];
	vec3_check_norm(normal);
}

static void
gl_normal3f(GLIContext rend, GLfloat x, GLfloat y, GLfloat z)
{
	GLfloat v[3] = {x, y, z};
	glNormal3fv(v);
}

static void
gl_vertex4fv(GLIContext rend, const GLfloat *v)
{
	assert(num_vertices < number_of(vertices));
	for (GLuint i = 0; i < 4; ++i)
	{
		vertices[num_vertices].pos[i] = v[i];
		vertices[num_vertices].norm[i] = normal[i];
		vertices[num_vertices].color[i] = color[i];
		vertices[num_vertices].ambient[i] = ambient[i];
		vertices[num_vertices].diffuse[i] = diffuse[i];
	}
	vec4_copy(specular, vertices[num_vertices].specular);
	vertices[num_vertices].shininess = shininess;
	++num_vertices;

	if (((primitive_mode == GL_TRIANGLES || primitive_mode == GL_TRIANGLE_STRIP) && num_vertices == 3) ||
			((primitive_mode == GL_QUADS || primitive_mode == GL_QUAD_STRIP) && num_vertices == 4))
	{
		draw_primitive(primitive_mode, vertices, num_vertices, (lighting) ? lights : NULL, number_of(lights));

		if (primitive_mode == GL_TRIANGLE_STRIP)
		{
			bcopy(vertices + 1, vertices, sizeof(vertices[0]) * 2);
			num_vertices = 2;
		}
		else if (primitive_mode == GL_QUAD_STRIP)
		{
			bcopy(vertices + 2, vertices, sizeof(vertices[0]) * 2);
			num_vertices = 2;
		}
		else
			num_vertices = 0;
	}
}

static void
gl_vertex3fv(GLIContext rend, const GLfloat *v)
{
	GLfloat v4[4] = {v[0], v[1], v[2], 1.0};
	gl_vertex4fv(rend, v4);
}

static void
gl_vertex3f(GLIContext rend, GLfloat x, GLfloat y, GLfloat z)
{
	GLfloat v[4] = {x, y, z, 1.0};
	gl_vertex4fv(rend, v);
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
	draw_push_debug();
	//draw_frustum_debug();
	draw_axes_debug();
	draw_pop_debug();
}

static void
gl_clear(GLIContext rend, GLbitfield mask)
{
	if (mask & ~GL_COLOR_BUFFER_BIT)
		fprintf(stderr, "%s() TODO: 0x%x\n", __FUNCTION__, mask & ~GL_COLOR_BUFFER_BIT);
	opengl_disp.clear(opengl_rend, mask);

	draw_push_debug();
	glClear(mask);
	draw_pop_debug();
}

static void
gl_enable(GLIContext rend, GLenum cap)
{
	switch (cap)
	{
		//case GL_DEPTH_TEST:
		case GL_LIGHTING:
			lighting = true;
			break;

		case GL_LIGHT0:
			lights[cap - GL_LIGHT0].enabled = true;
			return;

		default:
			fprintf(stderr, "%s() TODO 0x%x\n", __FUNCTION__, cap);
			opengl_disp.enable(opengl_rend, cap);
	}
}

static void
gl_lightfv(GLIContext rend, GLenum light, GLenum pname, const GLfloat *params)
{
	switch (pname)
	{
		case GL_POSITION:
		{
			vec4_t pos;
			for (GLuint i = 0; i < 4; ++i)
				pos[i] = params[i];
			matrix4x4_mult_vec4(modelview_matrix, pos, lights[light - GL_LIGHT0].pos);
			vec3_norm(lights[light - GL_LIGHT0].pos, lights[light - GL_LIGHT0].dir);
			break;
		}
		case GL_DIFFUSE:
			for (GLuint i = 0; i < 4; ++i)
				lights[light - GL_LIGHT0].diffuse[i] = params[i];
			break;
		default:
			fprintf(stderr, "%s() TODO\n", __FUNCTION__);
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
	struct matrix4x4 identity = IDENTITY_MATRIX;
	switch (matrix_mode)
	{
		case GL_MODELVIEW:
			modelview_matrix = identity;
			break;
		case GL_PROJECTION:
			projection_matrix = identity;
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
			target = &modelview_matrix;
			break;
		case GL_PROJECTION:
			target = &projection_matrix;
			break;
		default:
			fprintf(stderr, "%s() TODO\n", __FUNCTION__);
			return;
	}
	matrix4x4_mult_matrix4x4(*target, m, target);
}

static void
gl_rotatef(GLIContext rend, GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
	vec3_t axis = {x, y, z};
	assert(vec3_length(axis) == 1.0);

	GLfloat angle_rad = angle / (180.0 / M_PI);
	GLfloat c = cosf(angle_rad);
	GLfloat s = sinf(angle_rad);
	struct matrix4x4 m;
	m.cols[0][0] = x * x * (1 - c) + c;
	m.cols[0][1] = y * x * (1 - c) + z * s;
	m.cols[0][2] = x * z * (1 - c) - y * s;
	m.cols[0][3] = 0;
	m.cols[1][0] = x * y * (1 - c) - z * s;
	m.cols[1][1] = y * y * (1 - c) + c;
	m.cols[1][2] = y * z * (1 - c) + x * s;
	m.cols[1][3] = 0;
	m.cols[2][0] = x * z * (1 - c) + y * s;
	m.cols[2][1] = y * z * (1 - c) - x * s;
	m.cols[2][2] = z * z * (1 - c) + c;
	m.cols[2][3] = 0;
	m.cols[3][0] = m.cols[3][1] = m.cols[3][2] = 0;
	m.cols[3][3] = 1;
	gl_mult_matrixd(rend, m.m);
}

static void
gl_translated(GLIContext rend, GLdouble x, GLdouble y, GLdouble z)
{
	struct matrix4x4 m = IDENTITY_MATRIX;
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

	draw_push_debug();
	glFlush();
	draw_pop_debug();

}

static void
gl_finish(GLIContext rend)
{
	opengl_disp.finish(rend);
	primitive_index = 0;

	draw_push_debug();
	glFinish();
	draw_pop_debug();
}

static void
gl_ortho(GLIContext rend, GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
	GLdouble width = right - left, height = top - bottom, depth = zFar - zNear;
	struct matrix4x4 m;
	bzero(&m, sizeof(m));
	m.cols[0][0] = 2.0 / width;
	m.cols[1][1] = 2.0 / height;
	m.cols[2][2] = 2.0 / depth;
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
	fprintf(stderr, "%s() TODO\n", __FUNCTION__);
	opengl_disp.viewport(opengl_rend, x, y, width, height);
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
gluLookAt(GLdouble eyeX, GLdouble eyeY, GLdouble eyeZ, GLdouble centerX, GLdouble centerY, GLdouble centerZ, GLdouble upX, GLdouble upY, GLdouble upZ)
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
	draw_mult_matrix(matrix_mode, m);
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
	draw_mult_matrix(matrix_mode, m);
}
*/

/* GLUT */
void (*display_func)(void);
void (*idle_func)(void);

static void
display_debug(void)
{
	//glutPostRedisplay();
	//glutSwapBuffers();
}

static void
debug_idle(void)
{
	if (openGLUTGetWindow() == debug_win)
		openGLUTSetWindow(main_win);

	idle_func();
}

void
debug_key(unsigned char key, int x, int y)
{
	switch (key)
	{
		case '\t':
			debug_mode = (debug_mode + 1) % DEBUG_NMODES;
			openGLUTPostWindowRedisplay(main_win);
			break;
		case 'p':
			if (debug_primitive_index < 12)
				++debug_primitive_index;
			else
				debug_primitive_index = -1;
			openGLUTPostWindowRedisplay(main_win);
			break;
		case 'z':
			debug_zoom = fmin(3.0, debug_zoom + 0.1);
			openGLUTPostWindowRedisplay(main_win);
			break;
		case 'Z':
			debug_zoom = fmax(-3.0, debug_zoom - 0.1);
			openGLUTPostWindowRedisplay(main_win);
			break;
	}
}

void
glutInit(int *argcp, char **argv)
{
	openglut_init();

	openGLUTInit(argcp, argv);

	openGLUTInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	openGLUTInitWindowPosition(400, 20);
	debug_win = openGLUTCreateWindow("Debug");
	openGLUTDisplayFunc(display_debug);
	openGLUTKeyboardFunc(debug_key);

	glMatrixMode(GL_PROJECTION);
	glOrtho(-1, 1, -1, 1, -10, 10);
	glMatrixMode(GL_MODELVIEW);

	openGLUTInitWindowPosition(20, 20);
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

	draw_push_debug();
	openGLUTSwapBuffers();
	draw_pop_debug();
}

void
glutPostRedisplay(void)
{
	if (openGLUTGetWindow() == debug_win)
		openGLUTPostWindowRedisplay(main_win);
	else
		openGLUTPostRedisplay();
}
