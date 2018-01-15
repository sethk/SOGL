//
// Created by Seth Kingsley on 1/5/18.
//

#ifndef SOGL_VECTOR_H
#define SOGL_VECTOR_H

#include <OpenGL/gl.h>

// Vector operations: from_array, length, add, sub, mult_scalar, mult_vecN, divide_scalar, norm, check_norm, cross, dot,
// project, print

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

GLdouble vector2_length(const struct vector2 v);
struct vector2 vector2_add(const struct vector2 a, const struct vector2 b);
struct vector2 vector2_divide_scalar(const struct vector2 v, GLdouble divisor);
struct vector2 vector2_norm(const struct vector2 v);

struct vector3
{
	union
	{
		struct
		{
			GLdouble x, y, z;
		};
		struct
		{
			GLdouble r, g, b;
		};
		struct vector2 xy;
		GLdouble v[3];
	};
};

struct vector3 vector3_from_array(const GLdouble *v);
GLdouble vector3_length(const struct vector3 v);
struct vector3 vector3_add(const struct vector3 a, const struct vector3 b);
struct vector3 vector3_sub(const struct vector3 a, const struct vector3 b);
struct vector3 vector3_mult_scalar(const struct vector3 v, GLdouble mult);
struct vector3 vector3_mult_vector3(const struct vector3 a, const struct vector3 b);
struct vector3 vector3_divide_scalar(struct vector3 v, GLdouble divisor);
struct vector3 vector3_norm(const struct vector3 v);
void vector3_check_norm(const struct vector3 v, const char *label);
struct vector3 vector3_cross(const struct vector3 a, const struct vector3 b);
GLdouble vector3_dot(const struct vector3 a, const struct vector3 b);
void vector3_print(const struct vector3 v, const char *label);

struct vector4
{
	union
	{
		struct
		{
			GLdouble x, y, z, w;
		};
		struct
		{
			GLdouble r, g, b, a;
		};
		struct vector3 xyz;
		struct vector3 rgb;
		GLdouble v[4];
	};
};

struct vector4 vector4_from_array(const GLdouble *v);
struct vector4 vector4_from_float_array(const GLfloat *v);
struct vector4 vector4_add(const struct vector4 a, const struct vector4 b);
struct vector4 vector4_sub(const struct vector4 a, const struct vector4 b);
struct vector4 vector4_mult_scalar(const struct vector4 v, GLdouble mult);
struct vector4 vector4_lerp(const struct vector4 a, const struct vector4 b, GLdouble t);
struct vector3 vector4_project(const struct vector4 v);

#endif //SOGL_VECTOR_H
