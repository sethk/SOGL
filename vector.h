//
// Created by Seth Kingsley on 1/5/18.
//

#ifndef SOGL_VECTOR_H
#define SOGL_VECTOR_H

typedef double scalar_t;

// Vector operations: from_array, length, add, clamp, sub, mult_scalar, mult_vecN, divide_scalar, norm, check_norm,
// cross, dot, lerp, project, print

struct vector2
{
	union
	{
		struct
		{
			scalar_t x, y;
		};
		scalar_t v[2];
	};
};

scalar_t vector2_length(const struct vector2 v);
struct vector2 vector2_add(const struct vector2 a, const struct vector2 b);
struct vector2 vector2_sub(const struct vector2 a, const struct vector2 b);
struct vector2 vector2_divide_scalar(const struct vector2 v, scalar_t divisor);
struct vector2 vector2_norm(const struct vector2 v);
struct vector2 vector2_dot(const struct vector2 a, const struct vector2 b);
void vector2_print(const struct vector2 v, const char *label);

struct vector3
{
	union
	{
		struct
		{
			scalar_t x, y, z;
		};
		struct
		{
			scalar_t r, g, b;
		};
		struct vector2 xy;
		scalar_t v[3];
	};
};

struct vector3 vector3_from_array(const scalar_t *v);
scalar_t vector3_length(const struct vector3 v);
struct vector3 vector3_add(const struct vector3 a, const struct vector3 b);
struct vector3 vector3_clamp(const struct vector3 v);
struct vector3 vector3_sub(const struct vector3 a, const struct vector3 b);
struct vector3 vector3_mult_scalar(const struct vector3 v, scalar_t mult);
struct vector3 vector3_mult_vector3(const struct vector3 a, const struct vector3 b);
struct vector3 vector3_divide_scalar(struct vector3 v, scalar_t divisor);
struct vector3 vector3_norm(const struct vector3 v);
void vector3_check_norm(const struct vector3 v, const char *label);
struct vector3 vector3_cross(const struct vector3 a, const struct vector3 b);
scalar_t vector3_dot(const struct vector3 a, const struct vector3 b);
struct vector3 vector3_lerp(const struct vector3 a, const struct vector3 b, scalar_t t);
void vector3_print(const struct vector3 v, const char *label);

struct vector4
{
	union
	{
		struct
		{
			scalar_t x, y, z, w;
		};
		struct
		{
			scalar_t r, g, b, a;
		};
		struct vector3 xyz;
		struct vector3 rgb;
		scalar_t v[4];
	};
};

struct vector4 vector4_from_array(const scalar_t *v);
struct vector4 vector4_from_float_array(const float *v);
struct vector4 vector4_add(const struct vector4 a, const struct vector4 b);
struct vector4 vector4_sub(const struct vector4 a, const struct vector4 b);
struct vector4 vector4_mult_scalar(const struct vector4 v, scalar_t mult);
struct vector4 vector4_divide_scalar(const struct vector4 v, scalar_t divisor);
struct vector4 vector4_lerp(const struct vector4 a, const struct vector4 b, scalar_t t);
struct vector3 vector4_project(const struct vector4 v);

#endif //SOGL_VECTOR_H
