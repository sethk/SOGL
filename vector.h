//
// Created by Seth Kingsley on 1/5/18.
//

#ifndef SOGL_VECTOR_H
#define SOGL_VECTOR_H

#include <OpenGL/gl.h>

struct vector2
{
	union
	{
		struct
		{
			GLdouble x, y;
		};
		GLdouble v[2];
	};
};
struct vector3
{
	union
	{
		struct
		{
			struct vector2 xy;
			GLdouble z;
		};
		GLdouble v[3];
	};
};
typedef GLdouble vec3_t[3];
typedef GLdouble vec4_t[4];

struct vector2 vector2_add(const struct vector2 a, const struct vector2 b);
struct vector2 vector2_divide_scalar(const struct vector2 v, GLdouble divisor);
GLdouble vector2_length(const struct vector2 v);
struct vector2 vector2_norm(const struct vector2 v);
GLdouble vec3_length(const vec3_t v);
void vec3_add(const vec3_t a, const vec3_t b, vec3_t rv);
void vec3_print(const vec3_t v);
void vec3_mult_scalar(const vec3_t v, GLdouble mult, vec3_t rv);
void vec3_mult_vec3(const vec3_t a, const vec3_t b, vec3_t rv);
void vec3_divide_scalar(const vec3_t v, GLdouble quot, vec3_t rv);
void vec4_divide_scalar(const vec4_t v, GLdouble quot, vec4_t rv);
void vec3_copy(const vec3_t v, vec3_t rv);
void vec4_copy(const vec4_t v, vec4_t rv);
void vec4_copy_float(const GLfloat *v, vec4_t rv);
void vec3_copy_vec4(const vec3_t v, GLdouble w, vec4_t rv);
void vec3_norm(const vec3_t v, vec3_t rv);
void vec3_check_norm(const vec3_t v, const char *label);
void vec3_cross(const vec3_t a, const vec3_t b, vec3_t rv);
GLdouble vec3_dot(const vec3_t a, const vec3_t b);
GLdouble vec4_dot(const vec4_t a, const vec4_t b);
void vec4_add(const vec4_t a, const vec4_t b, vec4_t rv);
void vec4_sub(const vec4_t a, const vec4_t b, vec4_t rv);
void vec4_mult_vec4(const vec4_t a, const vec4_t b, vec4_t rv);
void vec4_print(const vec4_t v);

#endif //SOGL_VECTOR_H
