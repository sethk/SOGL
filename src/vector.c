//
// Created by Seth Kingsley on 1/5/18.
//

#include <sys/types.h>
#include <math.h>
#include <stdio.h>
#include <strings.h>
#include "vector.h"

static inline void
vector_add(const scalar_t *av, const scalar_t *bv, u_int size, scalar_t *sumv)
{
	for (u_int i = 0; i < size; ++i)
		sumv[i] = av[i] + bv[i];
}

static inline void
vector_clamp(const scalar_t *v, u_int size, scalar_t *cv)
{
	for (u_int i = 0; i < size; ++i)
		cv[i] = fmax(fmin(v[i], 1.0), 0);
}

static inline void
vector_sub(const scalar_t *av, const scalar_t *bv, u_int size, scalar_t *diffv)
{
	for (u_int i = 0; i < size; ++i)
		diffv[i] = av[i] - bv[i];
}

static inline void
vector_mult_scalar(const scalar_t *v, u_int size, scalar_t mult, scalar_t *prodv)
{
	for (u_int i = 0; i < size; ++i)
		prodv[i] = v[i] * mult;
}

static inline void
vector_mult_vector(const scalar_t *av, const scalar_t *bv, u_int size, scalar_t *prodv)
{
	for (u_int i = 0; i < size; ++i)
		prodv[i] = av[i] * bv[i];
}

static inline void
vector_divide_scalar(const scalar_t *v, u_int size, scalar_t divisor, scalar_t *quotv)
{
	for (u_int i = 0; i < size; ++i)
		quotv[i] = v[i] / divisor;
}

static inline scalar_t
vector_length(const scalar_t *v, u_int size)
{
	scalar_t sum_squares = 0;
	for (u_int i = 0; i < size; ++i)
		sum_squares+= v[i] * v[i];
	return sqrt(sum_squares);
}

static inline void
vector_norm(const scalar_t *v, u_int size, scalar_t *nv)
{
	vector_divide_scalar(v, size, vector_length(v, size), nv);
}

static inline void
vector_print(const scalar_t *v, u_int size, const char *label)
{
	fputs(label, stderr);
	fputs(" (", stderr);
	for (u_int i = 0; i < size; ++i)
		fprintf(stderr, "%s%g", (i > 0) ? ", " : "", v[i]);
	fputs(")\n", stderr);
}

static inline void
vector_check_norm(const scalar_t *v, u_int size, const char *label)
{
	scalar_t length = vector_length(v, size);
	if (fabs(1.0 - length) > 1.0e-7)
	{
		char err[64];
		snprintf(err, sizeof(err), "%s: Vector not normalized (1.0 - length = %g): ", label, 1.0 - length);
		vector_print(v, size, err);
	}
}

static inline scalar_t
vector_dot(const scalar_t *av, const scalar_t *bv, u_int size)
{
	scalar_t dot = 0;
	for (u_int i = 0; i < size; ++i)
		dot+= av[i] * bv[i];
	return dot;
}

struct vector2
vector2_add(const struct vector2 a, const struct vector2 b)
{
	struct vector2 sum;
	vector_add(a.v, b.v, 2, sum.v);
	return sum;
}

struct vector2 vector2_sub(const struct vector2 a, const struct vector2 b)
{
	struct vector2 diff;
	vector_sub(a.v, b.v, 2, diff.v);
	return diff;
}

struct vector2
vector2_divide_scalar(const struct vector2 v, scalar_t divisor)
{
	struct vector2 quot;
	vector_divide_scalar(v.v, 2, divisor, quot.v);
	return quot;
}

scalar_t
vector2_length(const struct vector2 v)
{
	return vector_length(v.v, 2);
}

struct vector2
vector2_norm(const struct vector2 v)
{
	struct vector2 nv;
	vector_norm(v.v, 2, nv.v);
	return nv;
}

scalar_t
vector2_dot(const struct vector2 a, const struct vector2 b)
{
	return vector_dot(a.v, b.v, 2);
}

void
vector2_print(const struct vector2 v, const char *label)
{
	vector_print(v.v, 2, label);
}

struct vector3
vector3_from_array(const scalar_t *vv)
{
	struct vector3 v;
	bcopy(vv, v.v, sizeof(v.v));
	return v;
}

scalar_t
vector3_length(const struct vector3 v)
{
	return vector_length(v.v, 3);
}

struct vector3
vector3_add(const struct vector3 a, const struct vector3 b)
{
	struct vector3 sum;
	vector_add(a.v, b.v, 3, sum.v);
	return sum;
}

struct vector3
vector3_clamp(const struct vector3 v)
{
	struct vector3 clamped;
	vector_clamp(v.v, 3, clamped.v);
	return clamped;
}

struct vector3
vector3_sub(const struct vector3 a, const struct vector3 b)
{
	struct vector3 diff;
	vector_sub(a.v, b.v, 3, diff.v);
	return diff;
}

struct vector3
vector3_mult_scalar(const struct vector3 v, scalar_t mult)
{
	struct vector3 prod;
	vector_mult_scalar(v.v, 3, mult, prod.v);
	return prod;
}

struct vector3
vector3_mult_vector3(const struct vector3 a, const struct vector3 b)
{
	struct vector3 prod;
	vector_mult_vector(a.v, b.v, 3, prod.v);
	return prod;
}

struct vector3
vector3_divide_scalar(struct vector3 v, scalar_t divisor)
{
	struct vector3 quot;
	vector_divide_scalar(v.v, 3, divisor, quot.v);
	return quot;
}

struct vector3
vector3_norm(const struct vector3 v)
{
	struct vector3 norm;
	vector_norm(v.v, 3, norm.v);
	return norm;
}

void
vector3_check_norm(const struct vector3 v, const char *label)
{
	vector_check_norm(v.v, 3, label);
}

/* \   \   \     /   /   /
 * +ex +ey +ez -ex -ey -ez
 *  ax  ay  az  ax  ay  az
 *  bx  by  bz  bx  by  bz
 */
struct vector3
vector3_cross(const struct vector3 a, const struct vector3 b)
{
	struct vector3 cross;
	for (u_int i = 0; i < 3; ++i)
		cross.v[i] = a.v[(i + 1) % 3] * b.v[(i + 2) % 3] - a.v[(i + 2) % 3] * b.v[(i + 1) % 3];
	return cross;
}

scalar_t
vector3_dot(const struct vector3 a, const struct vector3 b)
{
	return vector_dot(a.v, b.v, 3);
}

struct vector3
vector3_lerp(const struct vector3 a, const struct vector3 b, scalar_t t)
{
	if (t < 0 || t > 1)
		fprintf(stderr, "%s: invalid t %g\n", __FUNCTION__, t);
	return vector3_add(vector3_mult_scalar(a, 1 - t), vector3_mult_scalar(b, t));
}

void
vector3_print(const struct vector3 v, const char *label)
{
	vector_print(v.v, 3, label);
}

struct vector4
vector4_from_array(const scalar_t *vv)
{
	struct vector4 v;
	bcopy(vv, v.v, sizeof(v.v));
	return v;
}

struct vector4
vector4_from_float_array(const float *fv)
{
	struct vector4 v;
	for (u_int i = 0; i < 4; ++i)
		v.v[i] = fv[i];
	return v;
}

struct vector4
vector4_add(const struct vector4 a, const struct vector4 b)
{
	struct vector4 sum;
	vector_add(a.v, b.v, 4, sum.v);
	return sum;
}

struct vector4
vector4_sub(const struct vector4 a, const struct vector4 b)
{
	struct vector4 diff;
	vector_sub(a.v, b.v, 4, diff.v);
	return diff;
}

struct vector4
vector4_mult_scalar(const struct vector4 v, scalar_t mult)
{
	struct vector4 prod;
	vector_mult_scalar(v.v, 4, mult, prod.v);
	return prod;
}

struct vector4
vector4_divide_scalar(const struct vector4 v, scalar_t divisor)
{
	struct vector4 quot;
	vector_divide_scalar(v.v, 4, divisor, quot.v);
	return quot;
}

struct vector4
vector4_lerp(const struct vector4 a, const struct vector4 b, scalar_t t)
{
	if (t < 0 || t > 1)
		fprintf(stderr, "%s: invalid t %g\n", __FUNCTION__, t);
	return vector4_add(vector4_mult_scalar(a, 1 - t), vector4_mult_scalar(b, t));
}

struct vector3
vector4_project(const struct vector4 v)
{
	return vector3_divide_scalar(v.xyz, v.w);
}
