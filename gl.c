#include <GL/gl.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

#define number_of(a) (sizeof(a) / sizeof(*(a)))

enum
{
	V_X = 0,
	V_Y = 1,
	V_Z = 2
};

typedef GLdouble vec3_t[3];
typedef GLdouble matrix4x4_t[4][4];

/* Vector */
static GLfloat
vec3_length(const vec3_t v)
{
	return sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

static void
vec3_divide_scalar(const vec3_t v, GLfloat quot, vec3_t rv)
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
 * -ex -ey -ez +ex +ey +ez
 *  bx  by  bz  bx  by  bz
 *  ax  ay  az  ax  ay  az
 */
static void
vec3_cross(const vec3_t a, const vec3_t b, vec3_t rv)
{
	rv[V_X] = a[V_Y] * b[V_Z] - a[V_Z] * b[V_Y];
	rv[V_Y] = a[V_Z] * b[V_X] - a[V_X] * b[V_Z];
	rv[V_Z] = a[V_X] * b[V_Y] - a[V_Y] * b[V_X];
}

/* GL */
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
		glVertex3dv(vertices[0]);
		glVertex3dv(vertices[1]);
		glVertex3dv(vertices[2]);
		glVertex3dv(vertices[3]);
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
}

void
soglEnable(GLenum cap)
{
	fprintf(stderr, "%s() TODO\n", __FUNCTION__);
}

void
soglLightfv(GLenum light, GLenum pname, const GLfloat *params)
{
	fprintf(stderr, "%s() TODO\n", __FUNCTION__);
}

void
soglMatrixMode(GLenum mode)
{
	fprintf(stderr, "%s() TODO\n", __FUNCTION__);
}

void
soglRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
	fprintf(stderr, "%s() TODO\n", __FUNCTION__);
}

void
soglTranslatef(GLfloat x, GLfloat y, GLfloat z)
{
	fprintf(stderr, "%s() TODO\n", __FUNCTION__);
}

/* GLU */
void
sogluLookAt(GLdouble eyeX, GLdouble eyeY, GLdouble eyeZ, GLdouble centerX, GLdouble centerY, GLdouble centerZ, GLdouble upX, GLdouble upY, GLdouble upZ)
{
	vec3_t forward = {centerX - eyeX, centerY - eyeY, centerZ - eyeZ}, norm_forward;
	vec3_norm(forward, norm_forward);
	vec3_t up = {upX, upY, upZ}, norm_up;
	vec3_norm(up, norm_up);
	vec3_t s;
	vec3_cross(norm_forward, norm_up, s);
	vec3_t u;
	vec3_cross(s, norm_forward, u);
	matrix4x4_t m;
	for (GLuint col = 0; col < 3; ++col)
	{
		m[col][0] = s[col];
		m[col][1] = u[col];
		m[col][2] = -norm_forward[col];
		m[col][3] = 0;
	}
	for (GLuint i = 0; i < 3; ++i)
		m[3][i] = 0;
	m[3][3] = 1;
	GLdouble m16[16];
	for (GLuint col = 0; col < 4; ++col)
		for (GLuint row = 0; row < 4; ++row)
			m16[col * 4 + row] = m[col][row];
	glMultMatrixd(m16);
	glTranslated(-eyeX, -eyeY, -eyeZ);
}

void
sogluPerspective(GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar)
{
	fprintf(stderr, "%s()\n", __FUNCTION__);
}

