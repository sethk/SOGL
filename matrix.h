//
// Created by Seth Kingsley on 1/5/18.
//

#ifndef SOGL_MATRIX_H
#define SOGL_MATRIX_H

#include <sys/types.h>
#include "vector.h"

struct matrix3x3
{
	union
	{
		scalar_t cols[3][3];
		scalar_t m[9];
	};
};
#define IDENTITY_MATRIX3X3 {.cols = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}};
struct matrix4x4
{
	union
	{
		scalar_t cols[4][4];
		scalar_t m[16];
	};
};
#define IDENTITY_MATRIX4X4 {.cols = {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}}

scalar_t matrix3x3_minor(const struct matrix3x3 m, u_int col, u_int row);
scalar_t matrix3x3_det(const struct matrix3x3 m);
struct matrix3x3 matrix3x3_invert(const struct matrix3x3 m);
void matrix3x3_check_inverse(const struct matrix3x3 m, const struct matrix3x3 im);
void matrix3x3_print(const struct matrix3x3 m, const char *label);
void matrix4x4_copy_linear(const scalar_t *ma, struct matrix4x4 *rm);
struct matrix4x4 matrix4x4_mult_matrix4x4(const struct matrix4x4 m, const struct matrix4x4 n);
struct vector4 matrix4x4_mult_vector4(const struct matrix4x4 m, const struct vector4 v);
struct matrix4x4 matrix4x4_make_rotation(scalar_t angle, scalar_t x, scalar_t y, scalar_t z);
struct matrix4x4 matrix4x4_make_scaling(scalar_t x, scalar_t y, scalar_t z);
scalar_t matrix4x4_det(const struct matrix4x4 m);
struct matrix4x4 matrix4x4_invert_trans(const struct matrix4x4 m);
struct matrix4x4 matrix4x4_invert(const struct matrix4x4 m);
void matrix4x4_check_inverse(const struct matrix4x4 m, const struct matrix4x4 im);
void matrix4x4_print(const struct matrix4x4 m, const char *label);

#endif //SOGL_MATRIX_H
