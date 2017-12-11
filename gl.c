#include <GL/gl.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <strings.h>

#define number_of(a) (sizeof(a) / sizeof(*(a)))

enum
{
	V_X = 0,
	V_Y = 1,
	V_Z = 2
};

typedef GLdouble vec3_t[3];
struct matrix4x4
{
	union
	{
		GLdouble cols[4][4];
		GLdouble m[16];
	};
};

/* Vector */
#define IDENTITY_MATRIX {.cols = {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}}

static GLdouble
vec3_length(const vec3_t v)
{
	GLdouble sum_squares = 0;
	for (GLuint i = 0; i < 3; ++i)
		sum_squares+= v[i] * v[i];
	return sqrtf(sum_squares);
}

static void
vec3_divide_scalar(const vec3_t v, GLdouble quot, vec3_t rv)
{
	for (GLuint i = 0; i < 3; ++i)
		rv[i] = v[i] / quot;
}

static void
vec3_norm(const vec3_t v, vec3_t rv)
{
	vec3_divide_scalar(v, vec3_length(v), rv);
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

/* Matrix */
static void
matrix4x4_mult(const struct matrix4x4 m, const struct matrix4x4 n, struct matrix4x4 *rm)
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

/* GL */
static GLenum matrix_mode = -1;
struct matrix4x4 modelview_matrix = IDENTITY_MATRIX;
struct matrix4x4 projection_matrix = IDENTITY_MATRIX;
static GLenum primitive_mode;
static vec3_t normals[10];
static GLuint num_normals;
static vec3_t vertices[10];
static GLuint num_vertices;

void
soglBegin(GLenum mode)
{
	primitive_mode = mode;
	if (mode != GL_QUADS)
	{
		fprintf(stderr, "%s() TODO\n", __FUNCTION__);
		glBegin(mode);
		return;
	}
	num_normals = 0;
	num_vertices = 0;
}

void
soglNormal3fv(const GLfloat *v)
{
	if (primitive_mode != GL_QUADS)
	{
		fprintf(stderr, "%s() TODO\n", __FUNCTION__);
		glNormal3fv(v);
	}
	else
	{
		assert(num_normals < number_of(normals));
		for (GLuint i = 0; i < 3; ++i)
			normals[num_normals][i] = v[i];
		++num_normals;
	}
}

void
soglVertex3fv(const GLfloat *v)
{
	if (primitive_mode != GL_QUADS)
	{
		fprintf(stderr, "%s() TODO\n", __FUNCTION__);
		glVertex3fv(v);
		return;
	}

	assert(num_vertices < number_of(vertices));
	for (GLuint i = 0; i < 3; ++i)
		vertices[num_vertices][i] = v[i];
	++num_vertices;
}

void
soglEnd(void)
{
	if (primitive_mode == GL_QUADS)
	{
		assert(num_vertices == 4);
		assert(num_normals == 1);
		glBegin(GL_QUADS);
		glNormal3dv(normals[0]);
		for (GLuint i = 0; i < num_vertices; ++i)
			glVertex3dv(vertices[i]);
		glEnd();
	}
	else
	{
		fprintf(stderr, "%s() TODO\n", __FUNCTION__);
		glEnd();
	}
	primitive_mode = -1;
}

void
soglClear(GLbitfield mask)
{
	fprintf(stderr, "%s() TODO\n", __FUNCTION__);
	glClear(mask);
}

void
soglEnable(GLenum cap)
{
	fprintf(stderr, "%s() TODO\n", __FUNCTION__);
	glEnable(cap);
}

void
soglLightfv(GLenum light, GLenum pname, const GLfloat *params)
{
	fprintf(stderr, "%s() TODO\n", __FUNCTION__);
	glLightfv(light, pname, params);
}

void
soglMatrixMode(GLenum mode)
{
	matrix_mode = mode;
	glMatrixMode(mode);
}

static void
soglCheckMatrix(GLenum param, struct matrix4x4 *target)
{
	struct matrix4x4 old_m;
	glGetDoublev(param, old_m.m);
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
soglMultMatrix(const struct matrix4x4 m)
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
			glMultMatrixd(m.m);
			return;
	}
	soglCheckMatrix(param, target);

	matrix4x4_mult(*target, m, target);
	glLoadMatrixd(target->m);
}

void
soglRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
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
	soglMultMatrix(m);
}

void
soglTranslatef(GLfloat x, GLfloat y, GLfloat z)
{
	struct matrix4x4 m = IDENTITY_MATRIX;
	m.cols[3][0] = x;
	m.cols[3][1] = y;
	m.cols[3][2] = z;
	soglMultMatrix(m);
}

/* GLU */
void
sogluLookAt(GLdouble eyeX, GLdouble eyeY, GLdouble eyeZ, GLdouble centerX, GLdouble centerY, GLdouble centerZ, GLdouble upX, GLdouble upY, GLdouble upZ)
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
	soglMultMatrix(m);
	soglTranslatef(-eyeX, -eyeY, -eyeZ);
}

void
sogluPerspective(GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar)
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
	soglMultMatrix(m);
}

