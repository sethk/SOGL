#include <GLUT/glut.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <strings.h>
#include <dlfcn.h>
#include <err.h>

#include "wrap_glut.c"

#define number_of(a) (sizeof(a) / sizeof(*(a)))

int main_win;
int debug_win;
enum {DEBUG_FRONT, DEBUG_LEFT, DEBUG_TOP, DEBUG_PROJECTION, DEBUG_NMODES} debug_mode = 0;
GLint debug_primitive_index = -1;
static GLdouble debug_zoom = 1;

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

/* OpenGL */
static void *opengl_handle = NULL;

#define GL_WRAPPER(name, ret_type, arg_types) static ret_type (*openGL##name)arg_types = NULL

GL_WRAPPER(Clear, void, (GLbitfield));
GL_WRAPPER(ClearColor, void, (GLclampf, GLclampf, GLclampf, GLclampf));
GL_WRAPPER(Begin, void, (GLenum));
GL_WRAPPER(End, void, (void));
GL_WRAPPER(Flush, void, (void));
GL_WRAPPER(Finish, void, (void));
GL_WRAPPER(Vertex3dv, void, (const GLdouble *));
GL_WRAPPER(Vertex4dv, void, (const GLdouble *));
GL_WRAPPER(Normal3dv, void, (const GLdouble *));
GL_WRAPPER(Color4dv, void, (const GLdouble *));
GL_WRAPPER(Color3f, void, (GLfloat, GLfloat, GLfloat));
GL_WRAPPER(GetDoublev, void, (GLenum, GLdouble *));
GL_WRAPPER(Enable, void, (GLenum));
GL_WRAPPER(Lightfv, void, (GLenum, GLenum, const GLfloat *));
GL_WRAPPER(MatrixMode, void, (GLenum));
GL_WRAPPER(LoadIdentity, void, (void));
GL_WRAPPER(MultMatrixd, void, (const GLdouble *));
GL_WRAPPER(Ortho, void, (GLdouble, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble));
GL_WRAPPER(Viewport, void, (GLint, GLint, GLsizei, GLsizei));

static void *
opengl_load_wrapper(void *handle, const char *name)
{
	void (*fp)(void);
	if (!(fp = dlsym(handle, name)))
		err(1, "Could not resolve %s", name);
	return fp;
}

static void
opengl_init(void)
{
	const char *glpath = "/System/Library/Frameworks/OpenGL.framework/OpenGL";
	if (!(opengl_handle = dlopen(glpath, RTLD_LAZY | RTLD_LOCAL)))
		err(1, "Could not dlopen %s", glpath);

# define LOAD_GL_WRAPPER(name) do { openGL##name = opengl_load_wrapper(opengl_handle, "gl" #name); } while (0)

	LOAD_GL_WRAPPER(Clear);
	LOAD_GL_WRAPPER(ClearColor);
	LOAD_GL_WRAPPER(Begin);
	LOAD_GL_WRAPPER(End);
	LOAD_GL_WRAPPER(Flush);
	LOAD_GL_WRAPPER(Finish);
	LOAD_GL_WRAPPER(Vertex3dv);
	LOAD_GL_WRAPPER(Vertex4dv);
	LOAD_GL_WRAPPER(Normal3dv);
	LOAD_GL_WRAPPER(Color4dv);
	LOAD_GL_WRAPPER(Color3f);
	LOAD_GL_WRAPPER(GetDoublev);
	LOAD_GL_WRAPPER(Enable);
	LOAD_GL_WRAPPER(Lightfv);
	LOAD_GL_WRAPPER(MatrixMode);
	LOAD_GL_WRAPPER(LoadIdentity);
	LOAD_GL_WRAPPER(MultMatrixd);
	LOAD_GL_WRAPPER(Ortho);
	LOAD_GL_WRAPPER(Viewport);
}

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
struct matrix4x4 modelview_matrix = IDENTITY_MATRIX;
struct matrix4x4 projection_matrix = IDENTITY_MATRIX;
static const vec4_t origin = {0, 0, 0, 1};
static vec4_t global_ambient = {0.2, 0.2, 0.2, 1.0};

struct vertex
{
	vec4_t pos;
	vec4_t norm;
	vec4_t color;
};

struct light
{
	bool enabled;
	vec4_t pos;
	vec3_t dir;
	vec4_t diffuse;
};

static void
draw_mult_matrix(GLenum mode, const struct matrix4x4 m)
{
	struct matrix4x4 *target;
	GLenum param;
	switch (mode)
	{
		case GL_MODELVIEW:
			target = &modelview_matrix;
			param = GL_MODELVIEW_MATRIX;
			break;
		case GL_PROJECTION:
			target = &projection_matrix;
			param = GL_PROJECTION_MATRIX;
			break;
		default:
			fprintf(stderr, "%s() TODO\n", __FUNCTION__);
			openGLMultMatrixd(m.m);
			return;
	}
	//glCheckMatrix(param, target);

	matrix4x4_mult_matrix4x4(*target, m, target);
	//openGLLoadMatrixd(target->m);
}

static struct matrix4x4
draw_get_debug_proj(void)
{
	struct matrix4x4 trans = IDENTITY_MATRIX;
	switch (debug_mode)
	{
		case DEBUG_FRONT:
			trans.cols[0][0] = 1.0 / debug_zoom;
			trans.cols[1][1] = 1.0 / debug_zoom;
			trans.cols[2][2] = 1.0 / debug_zoom;
			break;
		case DEBUG_LEFT:
			trans.cols[0][2] = 1.0 / debug_zoom;
			trans.cols[0][0] = 0;
			trans.cols[1][1] = 1.0 / debug_zoom;
			trans.cols[2][0] = 1.0 / debug_zoom;
			trans.cols[2][2] = 0;
			break;
		case DEBUG_TOP:
			trans.cols[0][1] = 1.0 / debug_zoom;
			trans.cols[0][0] = 0;
			trans.cols[1][0] = 1.0 / debug_zoom;
			trans.cols[1][1] = 0;
			trans.cols[2][2] = 1.0 / debug_zoom;
			break;
		case DEBUG_PROJECTION:
		{
			struct matrix4x4 scaling = matrix4x4_make_scaling(1.0 / debug_zoom, 1.0 / debug_zoom, 1.0 / debug_zoom);
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

	openGLBegin(GL_LINES);

	vec4_t view_origin;
	matrix4x4_mult_vec4(trans, origin, view_origin);

	openGLColor3f(1, 1, 1);
	openGLVertex4dv(view_origin);
	vec4_t right = {5, 0, 0, 1};
	matrix4x4_mult_vec4(trans, right, right);
	openGLColor3f(0.5, 0.5, 1);
	openGLVertex4dv(right);

	openGLColor3f(1, 1, 1);
	openGLVertex4dv(view_origin);
	vec4_t up = {0, 5, 0, 1};
	matrix4x4_mult_vec4(trans, up, up);
	openGLColor3f(1.0, 0.5, 0.5);
	openGLVertex4dv(up);

	openGLColor3f(1, 1, 1);
	openGLVertex4dv(view_origin);
	vec4_t forward = {0, 0, 5, 1};
	matrix4x4_mult_vec4(trans, forward, forward);
	openGLColor3f(0.5, 1, 0.5);
	openGLVertex4dv(forward);

	openGLEnd();
}

/*
static void
draw_frustum_debug(void)
{
	openGLBegin(GL_LINES);
	openGLColor3f(0, 1, 1);
	vec4_t edge = {-1, -1, -1, 1};
	vec4_t trans_edge;
	struct matrix4x4 trans;
	matrix4x4_mult_matrix4x4(modelview_matrix, projection_matrix, &trans);
	matrix4x4_mult_vec4(trans, edge, trans_edge);
	openGLVertex3dv(trans_edge);
	edge[3] = 1;
	matrix4x4_mult_vec4(trans, edge, trans_edge);
	openGLVertex3dv(trans_edge);
	openGLEnd();
}
*/

/*
static void
draw_lights_debug(struct light *lights, GLuint num_lights)
{
	struct matrix4x4 trans = draw_get_debug_proj();

	openGLBegin(GL_LINES);
	openGLColor3f(1, 1, 0);
	vec4_t dir;
	bcopy(lights[0].dir, dir, sizeof(lights[0].dir));
	dir[3] = 1.0;
	vec3_mult_scalar(dir, 5, dir);
	matrix4x4_mult_vec4(trans, dir, dir);
	openGLVertex4dv(dir);
	//vec4_print(dir);

	vec4_t trans_origin;
	matrix4x4_mult_vec4(trans, origin, trans_origin);
	openGLVertex4dv(trans_origin);
	openGLEnd();
}
*/

static void
draw_primitive_debug(GLenum mode, struct vertex *vertices, GLuint num_vertices, struct light *lights, GLuint num_lights)
{
	int win = glutGetWindow();
	assert(win != debug_win);
	openGLUTSetWindow(debug_win);
	struct matrix4x4 trans = draw_get_debug_trans();
	struct matrix4x4 debug_proj = draw_get_debug_proj();

	openGLBegin(GL_LINE_LOOP);
	openGLColor3f(1, 1, 1);
	for (GLuint i = 0; i < num_vertices; ++i)
	{
		vec4_t view_v;
		matrix4x4_mult_vec4(trans, vertices[i].pos, view_v);
		openGLVertex4dv(view_v);
	}
	openGLEnd();

	openGLBegin(GL_LINES);
	for (GLuint i = 0; i < num_vertices; ++i)
	{
		openGLColor3f(0, 1, 1);
		vec4_t view_v;
		matrix4x4_mult_vec4(trans, vertices[i].pos, view_v);
		//fprintf(stderr, "Vertex[%u]: ", i);
		//vec4_print(view_v);
		openGLVertex4dv(view_v);
		vec4_t vert_norm;
		vec3_mult_scalar(vertices[i].norm, 0.5, vert_norm);
		vert_norm[3] = 1.0;
		vec3_add(vertices[i].pos, vert_norm, vert_norm);
		matrix4x4_mult_vec4(trans, vert_norm, vert_norm);
		//fprintf(stderr, "Normal[%u]: ", i);
		//vec4_print(vert_norm);
		openGLVertex4dv(vert_norm);

		if (lights)
		{
			vec3_t world_normal;
			matrix4x4_mult_vec4(modelview_matrix, vertices[i].norm, world_normal);
			vec3_print(world_normal);
			vec3_check_norm(world_normal);
			vec3_print(lights[0].dir);
			vec3_check_norm(lights[0].dir);
			GLdouble cos_theta = vec3_dot(world_normal, lights[0].dir);
			fprintf(stderr, "%g\n", cos_theta);

			openGLColor3f(1, 1, 0);
			openGLVertex4dv(view_v);
			vec4_t vert_light;
			vec3_mult_scalar(lights[0].dir, 0.5, vert_light);
			vert_light[3] = 1.0;
			vec4_t world_v;
			matrix4x4_mult_vec4(modelview_matrix, vertices[i].pos, world_v);
			vec3_add(world_v, vert_light, vert_light);
			matrix4x4_mult_vec4(debug_proj, vert_light, vert_light);
			openGLVertex4dv(vert_light);

			openGLColor3f(1, 0.5, 0.5);
			openGLVertex4dv(view_v);
			vec4_t view_light;
			matrix4x4_mult_vec4(debug_proj, lights[0].pos, view_light);
			openGLVertex4dv(view_light);
		}
	}
	openGLEnd();

	openGLUTSetWindow(win);
}

static void
draw_primitive(GLenum mode, struct vertex *vertices, GLuint num_vertices, struct light *lights, GLuint num_lights)
{
	openGLBegin(mode);
	for (GLuint i = 0; i < num_vertices; ++i)
	{
		vec4_t color;
		if (lights)
		{
			// Global ambient
			bcopy(global_ambient, color, sizeof(global_ambient));
			vec4_t material_ambient = {0.2, 0.2, 0.2, 1.0};
			vec4_mult_vec4(color, material_ambient, color);

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
				vec4_t material_diffuse = {0.8, 0.8, 0.8, 1.0};
				vec4_mult_vec4(diffuse, material_diffuse, diffuse);
				//vec4_print(color);
				vec4_add(color, diffuse, color);
			}
		}
		else
			bcopy(vertices[i].color, color, sizeof(color));
		openGLColor4dv(color);
		vec4_t trans_v;
		matrix4x4_mult_vec4(modelview_matrix, vertices[i].pos, trans_v);
		matrix4x4_mult_vec4(projection_matrix, trans_v, trans_v);
		openGLVertex4dv(trans_v);
	}
	openGLEnd();
}

/* GL */
static GLenum matrix_mode = GL_MODELVIEW;
static GLenum primitive_mode;
static GLuint primitive_index;
static vec3_t color = {1, 1, 1};
static vec3_t normal;
static struct vertex vertices[4];
static GLuint num_vertices;
static bool lighting = false;
static struct light lights[GL_MAX_LIGHTS];

void
glBegin(GLenum mode)
{
	primitive_mode = mode;
	if (mode != GL_TRIANGLES && mode != GL_TRIANGLE_STRIP && mode != GL_QUADS)
	{
		fprintf(stderr, "%s() TODO\n", __FUNCTION__);
		return;
	}
	num_vertices = 0;
}

void
glColor3fv(const GLfloat *c)
{
	for (GLuint i = 0; i < 3; ++i)
		color[i] = c[i];
}

void
glColor3f(GLfloat red, GLfloat green, GLfloat blue)
{
	GLfloat c[3] = {red, green, blue};
	glColor3fv(c);
}

void
glMaterialfv(GLenum face, GLenum pname, const GLfloat *params)
{
	fprintf(stderr, "%s() TODO\n", __FUNCTION__);
}

void
glNormal3fv(const GLfloat *v)
{
	for (GLuint i = 0; i < 3; ++i)
		normal[i] = v[i];
	vec3_check_norm(normal);
}

void
glNormal3f(GLfloat x, GLfloat y, GLfloat z)
{
	GLfloat v[3] = {x, y, z};
	glNormal3fv(v);
}

void
glVertex3fv(const GLfloat *v)
{
	assert(num_vertices < number_of(vertices));
	for (GLuint i = 0; i < 3; ++i)
	{
		vertices[num_vertices].pos[i] = v[i];
		vertices[num_vertices].norm[i] = normal[i];
		vertices[num_vertices].color[i] = color[i];
	}
	vertices[num_vertices].pos[3] = 1;
	vertices[num_vertices].norm[3] = 0;
	++num_vertices;

	if (((primitive_mode == GL_TRIANGLES || primitive_mode == GL_TRIANGLE_STRIP) && num_vertices == 3) ||
			(primitive_mode == GL_QUADS && num_vertices == 4))
	{
		if (debug_primitive_index == -1 || primitive_index == (GLuint)debug_primitive_index)
			draw_primitive_debug(primitive_mode, vertices, num_vertices, (lighting) ? lights : NULL, number_of(lights));

		draw_primitive(primitive_mode, vertices, num_vertices, (lighting) ? lights : NULL, number_of(lights));

		if (primitive_mode == GL_TRIANGLE_STRIP)
		{
			bcopy(vertices + 1, vertices, sizeof(vertices[0]) * 2);
			num_vertices = 2;
		}
		else
			num_vertices = 0;
		++primitive_index;
	}
}

void
glVertex3f(GLfloat x, GLfloat y, GLfloat z)
{
	GLfloat v[3] = {x, y, z};
	glVertex3fv(v);
}

void
glCheckMatrix(GLenum param, struct matrix4x4 *target)
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

void
glEnd(void)
{
}

void
glClear(GLbitfield mask)
{
	if (mask & ~GL_COLOR_BUFFER_BIT)
		fprintf(stderr, "%s() TODO: 0x%x\n", __FUNCTION__, mask & ~GL_COLOR_BUFFER_BIT);
	openGLClear(mask);

	int win = glutGetWindow();
	assert(win != debug_win);
	openGLUTSetWindow(debug_win);
	openGLClear(mask);

	//draw_frustum_debug();
	draw_axes_debug();

	openGLUTSetWindow(win);
}

void
glEnable(GLenum cap)
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
			fprintf(stderr, "%s() TODO\n", __FUNCTION__);
			openGLEnable(cap);
	}
}

void
glLightfv(GLenum light, GLenum pname, const GLfloat *params)
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

void
glMatrixMode(GLenum mode)
{
	matrix_mode = mode;
	//openGLMatrixMode(mode);
}

void
glLoadIdentity(void)
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
			openGLLoadIdentity();
	}
}

void
glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
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
	draw_mult_matrix(matrix_mode, m);
}

void
glTranslatef(GLfloat x, GLfloat y, GLfloat z)
{
	struct matrix4x4 m = IDENTITY_MATRIX;
	m.cols[3][0] = x;
	m.cols[3][1] = y;
	m.cols[3][2] = z;
	draw_mult_matrix(matrix_mode, m);
}

void
glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
	fprintf(stderr, "%s() TODO\n", __FUNCTION__);
	openGLClearColor(red, green, blue, alpha);
}

void
glFlush(void)
{
	fprintf(stderr, "%s() TODO\n", __FUNCTION__);
	openGLFlush();
}

void
glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
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
	draw_mult_matrix(matrix_mode, m);
}

void
glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
	fprintf(stderr, "%s() TODO\n", __FUNCTION__);
	openGLViewport(x, y, width, height);
}

void
glShadeModel(GLenum mode)
{
	fprintf(stderr, "%s() TODO\n", __FUNCTION__);
}

/*
void
glBlendFunc(GLenum sfactor, GLenum dfactor)
{
}

void
glDepthFunc(GLenum func)
{
}

void
glDisable(GLenum cap)
{
}

void
glPolygonMode(GLenum face, GLenum mode)
{
}

void
glPushMatrix(void)
{
}

void
glPopMatrix(void)
{
}

void
glScalef(GLfloat x, GLfloat y, GLfloat z)
{
}

*/

/* GLU */
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
			if (debug_primitive_index < 9)
				++debug_primitive_index;
			else
				debug_primitive_index = -1;
			openGLUTPostWindowRedisplay(main_win);
			break;
		case 'z':
			debug_zoom+= 0.1;
			openGLUTPostWindowRedisplay(main_win);
			break;
		case 'Z':
			debug_zoom = fmax(0.1, debug_zoom - 0.1);
			openGLUTPostWindowRedisplay(main_win);
			break;
	}
}

void
glutInit(int *argcp, char **argv)
{
	openglut_init();

	openGLUTInit(argcp, argv);

	opengl_init();

	openGLUTInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	openGLUTInitWindowPosition(400, 20);
	debug_win = openGLUTCreateWindow("Debug");
	openGLUTDisplayFunc(display_debug);
	openGLUTKeyboardFunc(debug_key);

	/*
	openGLMatrixMode(GL_PROJECTION);
	//openGLOrtho(-6, 6, -6, 6, -10, 10);
	openGLMatrixMode(GL_MODELVIEW);
	*/

	openGLUTInitWindowPosition(20, 20);
}

int
glutCreateWindow(const char *title)
{
	main_win = openGLUTCreateWindow(title);
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
	openGLUTSwapBuffers();

	int win = openGLUTGetWindow();
	assert(win != debug_win);
	openGLUTSetWindow(debug_win);
	openGLUTSwapBuffers();
	openGLUTSetWindow(win);

	primitive_index = 0;
}

void
glutPostRedisplay(void)
{
	if (openGLUTGetWindow() == debug_win)
		openGLUTPostWindowRedisplay(main_win);
	else
		openGLUTPostRedisplay();
}
