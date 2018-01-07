//
// Created by Seth Kingsley on 1/5/18.
//

#include <math.h>
#include <stdio.h>
#include "vector.h"

GLdouble
vec3_length(const vec3_t v)
{
	GLdouble sum_squares = 0;
	for (GLuint i = 0; i < 3; ++i)
		sum_squares+= v[i] * v[i];
	return sqrt(sum_squares);
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
vec3_copy(const vec3_t v, vec3_t rv)
{
	for (GLuint i = 0; i < 3; ++i)
		rv[i] = v[i];
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
vec3_copy_vec4(const vec3_t v, GLdouble w, vec4_t rv)
{
	for (GLuint i = 0; i < 3; ++i)
		rv[i] = v[i];
	rv[3] = w;
}

void
vec3_norm(const vec3_t v, vec3_t rv)
{
	vec3_divide_scalar(v, vec3_length(v), rv);
}

void
vec3_check_norm(const vec3_t v, const char *label)
{
	GLdouble length = vec3_length(v);
	if (fabs(1.0 - length) > 1.0e-7)
	{
		fprintf(stderr, "%s: Vector not normalized (1.0 - length = %g): ", label, 1.0 - length);
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
vec4_sub(const vec4_t a, const vec4_t b, vec4_t rv)
{
	for (GLuint i = 0; i < 4; ++i)
		rv[i] = a[i] - b[i];
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

