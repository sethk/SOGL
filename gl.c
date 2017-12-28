#include <GL/gl.h>
#include <GL/glut.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <strings.h>
#include <dlfcn.h>
#include <err.h>

#define number_of(a) (sizeof(a) / sizeof(*(a)))

int main_win;
int debug_win;
enum {DEBUG_FRONT, DEBUG_LEFT, DEBUG_TOP, DEBUG_MODELVIEW, DEBUG_PROJECTION, DEBUG_NMODES} debug_mode = 0;
GLint debug_primitive_index = -1;

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
GL_WRAPPER(Begin, void, (GLenum));
GL_WRAPPER(End, void, (void));
//GL_WRAPPER(Vertex3fv, void, (const GLfloat *));
GL_WRAPPER(Vertex3dv, void, (const GLdouble *));
GL_WRAPPER(Vertex4dv, void, (const GLdouble *));
//GL_WRAPPER(Normal3fv, void, (const GLfloat *));
GL_WRAPPER(Normal3dv, void, (const GLdouble *));
GL_WRAPPER(Color3fv, void, (const GLfloat *));
GL_WRAPPER(Color3f, void, (GLfloat, GLfloat, GLfloat));
GL_WRAPPER(GetDoublev, void, (GLenum, GLdouble *));
GL_WRAPPER(Enable, void, (GLenum));
GL_WRAPPER(Lightfv, void, (GLenum, GLenum, const GLfloat *));
GL_WRAPPER(MatrixMode, void, (GLenum));
GL_WRAPPER(LoadIdentity, void, (void));
GL_WRAPPER(MultMatrixd, void, (const GLdouble *));
GL_WRAPPER(Ortho, void, (GLdouble, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble));

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
	LOAD_GL_WRAPPER(Begin);
	LOAD_GL_WRAPPER(End);
	//LOAD_GL_WRAPPER(Vertex3fv);
	LOAD_GL_WRAPPER(Vertex3dv);
	LOAD_GL_WRAPPER(Vertex4dv);
	//LOAD_GL_WRAPPER(Normal3fv);
	LOAD_GL_WRAPPER(Normal3dv);
	LOAD_GL_WRAPPER(Color3fv);
	LOAD_GL_WRAPPER(Color3f);
	LOAD_GL_WRAPPER(GetDoublev);
	LOAD_GL_WRAPPER(Enable);
	LOAD_GL_WRAPPER(Lightfv);
	LOAD_GL_WRAPPER(MatrixMode);
	LOAD_GL_WRAPPER(LoadIdentity);
	LOAD_GL_WRAPPER(MultMatrixd);
	LOAD_GL_WRAPPER(Ortho);
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

static void
vec3_divide_scalar(const vec3_t v, GLdouble quot, vec3_t rv)
{
	for (GLuint i = 0; i < 3; ++i)
		rv[i] = v[i] / quot;
}

/*
 static void
 vec4_divide_scalar(const vec4_t v, GLdouble quot, vec4_t rv)
 {
 for (GLuint i = 0; i < 4; ++i)
 rv[i] = v[i] / quot;
 }
 */

void
vec3_norm(const vec3_t v, vec3_t rv)
{
	vec3_divide_scalar(v, vec3_length(v), rv);
}

void
vec3_check_norm(const vec3_t v)
{
	GLdouble length = vec3_length(v);
	if (length != 1.0)
	{
		fprintf(stderr, "Vector not normalized: ");
		vec3_print(v);
	}
}

/* \   \   \     /   /   /
 * +ex +ey +ez -ex -ey -ez
 *  ax  ay  az  ax  ay  az
 *  bx  by  bz  bx  by  bz
 */
static void
vec3_cross(const vec3_t a, const vec3_t b, vec3_t rv)
{
	for (GLuint i = 0; i < 3; ++i)
		rv[i] = a[(i + 1) % 3] * b[(i + 2) % 3] - a[(i + 2) % 3] * b[(i + 1) % 3];
}

/*
 static GLdouble
 vec4_dot(const vec4_t a, const vec4_t b)
 {
 GLdouble dot = 0;
 for (GLuint i = 0; i < 4; ++i)
 dot+= a[i] * b[i];
 return dot;
 }
 */

/*
 void
 vec4_add(const vec4_t a, const vec4_t b, vec4_t rv)
 {
 for (GLuint i = 0; i < 4; ++i)
 rv[i] = a[i] + b[i];
 }
 */

static void
vec4_print(const vec4_t v)
{
	fprintf(stderr, "(%g, %g, %g, %g)\n", v[0], v[1], v[2], v[3]);
}

/* Matrix */
static void
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

static void
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

void
matrix4x4_print(struct matrix4x4 m)
{
	for (GLuint row = 0; row < 4; ++row)
		fprintf(stderr, "[ %4.4g, %4.4g, %4.4g, %4.4g ]\n", m.cols[0][row], m.cols[1][row], m.cols[2][row], m.cols[3][row]);
}

/* draw */
struct matrix4x4 modelview_matrix = IDENTITY_MATRIX;
struct matrix4x4 projection_matrix = IDENTITY_MATRIX;

struct vertex
{
	vec4_t pos;
	vec4_t norm;
	vec4_t color;
};

static struct
{
	vec4_t pos;
	vec4_t diff;
} lights[GL_MAX_LIGHTS];

static struct matrix4x4
draw_get_debug_trans(void)
{
	struct matrix4x4 trans = IDENTITY_MATRIX;
	switch (debug_mode)
	{
		case DEBUG_FRONT:
			break;
		case DEBUG_LEFT:
			trans.cols[0][2] = 1;
			trans.cols[0][0] = 0;
			trans.cols[2][0] = 1;
			trans.cols[2][2] = 0;
			break;
		case DEBUG_TOP:
			trans.cols[0][1] = 1;
			trans.cols[0][0] = 0;
			trans.cols[1][0] = 1;
			trans.cols[1][1] = 0;
			break;
		case DEBUG_MODELVIEW:
			trans = modelview_matrix;
			break;
		case DEBUG_PROJECTION:
			matrix4x4_mult_matrix4x4(projection_matrix, modelview_matrix, &trans);
			break;
		case DEBUG_NMODES:
			break;
	}
	return trans;
}

static void
draw_axes_debug(void)
{
	struct matrix4x4 trans = draw_get_debug_trans();

	openGLBegin(GL_LINES);

	vec4_t origin = {0, 0, 0, 1};
	matrix4x4_mult_vec4(trans, origin, origin);

	openGLColor3f(1, 1, 1);
	openGLVertex4dv(origin);
	vec4_t right = {5, 0, 0, 1};
	matrix4x4_mult_vec4(trans, right, right);
	openGLColor3f(0.5, 0.5, 1);
	openGLVertex4dv(right);

	openGLColor3f(1, 1, 1);
	openGLVertex4dv(origin);
	vec4_t up = {0, 5, 0, 1};
	matrix4x4_mult_vec4(trans, up, up);
	openGLColor3f(1.0, 0.5, 0.5);
	openGLVertex4dv(up);

	openGLColor3f(1, 1, 1);
	openGLVertex4dv(origin);
	vec4_t forward = {0, 0, 5, 1};
	matrix4x4_mult_vec4(trans, forward, forward);
	openGLColor3f(0.5, 1, 0.5);
	openGLVertex4dv(forward);
	openGLEnd();
}

static void
draw_lights_debug(void)
{
	struct matrix4x4 trans = draw_get_debug_trans();

	openGLBegin(GL_LINES);
	openGLColor3f(1, 1, 0);
	vec4_t dir;
	bcopy(lights[0].pos, dir, sizeof(dir));
	dir[3] = 1.0;
	vec3_mult_scalar(dir, 5, dir);
	matrix4x4_mult_vec4(trans, dir, dir);
	openGLVertex4dv(dir);

	vec4_t origin = {0, 0, 0, 1};
	matrix4x4_mult_vec4(trans, origin, origin);
	openGLVertex3dv(origin);
	openGLEnd();
}

static void
draw_primitive_debug(GLenum mode, struct vertex *vertices, GLuint num_vertices)
{
	int win = glutGetWindow();
	assert(win != debug_win);
	glutSetWindow(debug_win);
	struct matrix4x4 trans = draw_get_debug_trans();

	openGLBegin(GL_LINE_LOOP);
	openGLColor3f(1, 1, 0);
	for (GLuint i = 0; i < num_vertices; ++i)
	{
		vec4_t world_v;
		matrix4x4_mult_vec4(trans, vertices[i].pos, world_v);
		openGLVertex4dv(world_v);
	}
	openGLEnd();

	openGLBegin(GL_LINES);
	openGLColor3f(0, 1, 1);
	for (GLuint i = 0; i < num_vertices; ++i)
	{
		vec4_t world_v;
		matrix4x4_mult_vec4(trans, vertices[i].pos, world_v);
		fprintf(stderr, "Vertex[%u]: ", i);
		vec4_print(world_v);
		openGLVertex4dv(world_v);
		vec4_t vert_norm;
		vec3_mult_scalar(vertices[i].norm, 0.5, vert_norm);
		vert_norm[3] = 1.0;
		vec3_add(vertices[i].pos, vert_norm, vert_norm);
		matrix4x4_mult_vec4(trans, vert_norm, vert_norm);
		fprintf(stderr, "Normal[%u]: ", i);
		vec4_print(vert_norm);
		openGLVertex4dv(vert_norm);
	}
	openGLEnd();

	glutSetWindow(win);
}

static void
draw_primitive(GLenum mode, struct vertex *vertices, GLuint num_vertices)
{
	openGLBegin(mode);
	for (GLuint i = 0; i < num_vertices; ++i)
	{
		vec4_t world_normal;
		matrix4x4_mult_vec4(modelview_matrix, vertices[i].norm, world_normal);
		matrix4x4_mult_vec4(projection_matrix, world_normal, world_normal);
		vec3_norm(world_normal, world_normal);
		openGLNormal3dv(world_normal);
		vec4_t world_v;
		matrix4x4_mult_vec4(modelview_matrix, vertices[i].pos, world_v);
		matrix4x4_mult_vec4(projection_matrix, world_v, world_v);
		openGLVertex4dv(world_v);
	}
	openGLEnd();
}

/* GL */
static GLenum matrix_mode = -1;
static GLenum primitive_mode;
static GLuint primitive_index;
static vec4_t color;
static vec4_t normal;
static GLuint num_normals;
struct vertex vertices[4];
static GLuint num_vertices;

void
glBegin(GLenum mode)
{
	primitive_mode = mode;
	if (mode != GL_TRIANGLES && mode != GL_QUADS)
	{
		fprintf(stderr, "%s() TODO\n", __FUNCTION__);
		return;
	}
	num_normals = 0;
	num_vertices = 0;
}

void
glColor3fv(const GLfloat *c)
{
	if (primitive_mode != GL_TRIANGLES && primitive_mode != GL_QUADS)
		fprintf(stderr, "%s() TODO\n", __FUNCTION__);
	else
	{
		for (GLuint i = 0; i < 3; ++i)
			color[i] = c[i];
		color[3] = 1.0;
	}
}

void
glColor3f(GLfloat red, GLfloat green, GLfloat blue)
{
	GLfloat c[3] = {red, green, blue};
	glColor3fv(c);
}

void
glNormal3fv(const GLfloat *v)
{
	if (primitive_mode != GL_TRIANGLES && primitive_mode != GL_QUADS)
		fprintf(stderr, "%s() TODO\n", __FUNCTION__);
	else
	{
		for (GLuint i = 0; i < 3; ++i)
			normal[i] = v[i];
		vec3_check_norm(normal);
	}
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
	if (primitive_mode != GL_TRIANGLES && primitive_mode != GL_QUADS)
	{
		fprintf(stderr, "%s() TODO\n", __FUNCTION__);
		return;
	}

	assert(num_vertices < number_of(vertices));
	for (GLuint i = 0; i < 3; ++i)
	{
		vertices[num_vertices].pos[i] = v[i];
		vertices[num_vertices].norm[i] = normal[i];
	}
	vertices[num_vertices].pos[3] = 1;
	vertices[num_vertices].norm[3] = 0;
	++num_vertices;

	if ((primitive_mode == GL_TRIANGLES && num_vertices == 3) ||
			(primitive_mode == GL_QUADS && num_vertices == 4))
	{
		if (debug_primitive_index == -1 || primitive_index == (GLuint)debug_primitive_index)
			draw_primitive_debug(primitive_mode, vertices, num_vertices);

		draw_primitive(primitive_mode, vertices, num_vertices);
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
	if (primitive_mode == GL_TRIANGLES || primitive_mode == GL_QUADS)
	{
		//struct matrix4x4 identity = IDENTITY_MATRIX;
		//glCheckMatrix(GL_MODELVIEW_MATRIX, &identity);
		//glCheckMatrix(GL_PROJECTION_MATRIX, &identity);
	}
	else
		fprintf(stderr, "%s() TODO\n", __FUNCTION__);
	primitive_mode = -1;
}

void
glClear(GLbitfield mask)
{
	if (mask & ~GL_COLOR_BUFFER_BIT)
		fprintf(stderr, "%s() TODO: 0x%x\n", __FUNCTION__, mask & ~GL_COLOR_BUFFER_BIT);
	openGLClear(mask);

	int win = glutGetWindow();
	assert(win != debug_win);
	glutSetWindow(debug_win);
	openGLClear(mask);

	draw_axes_debug();
	draw_lights_debug();

	glutSetWindow(win);
}

void
glEnable(GLenum cap)
{
	switch (cap)
	{
		//case GL_DEPTH_TEST:
		//case GL_LIGHTING:
			return;

		default:
			fprintf(stderr, "%s() TODO\n", __FUNCTION__);
			openGLEnable(cap);
	}
}

void
glLightfv(GLenum light, GLenum pname, const GLfloat *params)
{
	fprintf(stderr, "%s() TODO\n", __FUNCTION__);
	openGLLightfv(light, pname, params);

	switch (pname)
	{
		case GL_POSITION:
			for (GLuint i = 0; i < 4; ++i)
				lights[light - GL_LIGHT0].pos[i] = params[i];
			break;
	}
}

void
glMatrixMode(GLenum mode)
{
	matrix_mode = mode;
	openGLMatrixMode(mode);
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
glMultMatrix(const struct matrix4x4 m)
{
	struct matrix4x4 *target;
	GLenum param;
	switch (matrix_mode)
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
	glMultMatrix(m);
}

void
glTranslatef(GLfloat x, GLfloat y, GLfloat z)
{
	struct matrix4x4 m = IDENTITY_MATRIX;
	m.cols[3][0] = x;
	m.cols[3][1] = y;
	m.cols[3][2] = z;
	glMultMatrix(m);
}

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
	glMultMatrix(m);
	glTranslatef(-eyeX, -eyeY, -eyeZ);
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
	glMultMatrix(m);
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
	if (glutGetWindow() == debug_win)
		glutSetWindow(main_win);

	idle_func();
}

void
debug_key(unsigned char key, int x, int y)
{
	switch (key)
	{
		case '\t':
			debug_mode = (debug_mode + 1) % DEBUG_NMODES;
			glutPostWindowRedisplay(main_win);
			break;
		case 'p':
			if (debug_primitive_index < 9)
				++debug_primitive_index;
			else
				debug_primitive_index = -1;
			glutPostWindowRedisplay(main_win);
			break;
	}
}

void
soglutInit(int *argcp, char **argv)
{
	glutInit(argcp, argv);

	opengl_init();

	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowPosition(400, 20);
	debug_win = glutCreateWindow("Debug");
	glutDisplayFunc(display_debug);
	glutKeyboardFunc(debug_key);

	openGLMatrixMode(GL_PROJECTION);
	openGLOrtho(-6, 6, -6, 6, -10, 10);
	openGLMatrixMode(GL_MODELVIEW);

	glutInitWindowPosition(20, 20);
}

int
soglutCreateWindow(const char *title)
{
	main_win = glutCreateWindow(title);
	return main_win;
}

void
soglutIdleFunc(void (*fp)(void))
{
	idle_func = fp;
	glutIdleFunc(debug_idle);
}

void
soglutSwapBuffers(void)
{
	glutSwapBuffers();

	int win = glutGetWindow();
	assert(win != debug_win);
	glutSetWindow(debug_win);
	glutSwapBuffers();
	glutSetWindow(win);

	primitive_index = 0;
}

void
soglutPostRedisplay(void)
{
	if (glutGetWindow() == debug_win)
		glutPostWindowRedisplay(main_win);
	else
		glutPostRedisplay();
}
