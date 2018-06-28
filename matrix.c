//
// Created by Seth Kingsley on 1/5/18.
//

#include <strings.h>
#include <stdio.h>
#include <assert.h>
#include <alloca.h>
#include "matrix.h"
#include "math.h"

static scalar_t matrix_det(const scalar_t *mv, u_int dim);

static void
matrix_mult_matrix(const scalar_t *mv, const scalar_t *nv, u_int num_cols, u_int num_rows, scalar_t *rmv)
{
	for (u_int col = 0; col < num_cols; ++col)
		for (u_int row = 0; row < num_rows; ++row)
		{
			scalar_t dot = 0;
			for (u_int i = 0; i < num_cols; ++i)
				dot+= mv[i * num_cols + row] * nv[col * num_cols + i];
			rmv[col * num_rows + row] = dot;
		}
}

static void
matrix_print(const scalar_t *mv, u_int num_cols, u_int num_rows, const char *label)
{
	u_int label_line = num_rows / 2;
	int label_width = strlen(label);
	for (u_int row = 0; row < num_rows; ++row)
	{
		fprintf(stderr, "%*s ", label_width, (row == label_line) ? label : "");
		fputc('[', stderr);
		for (u_int col = 0; col < num_cols; ++col)
			fprintf(stderr, "%s% 5.3g", (col > 0) ? ", " : "", mv[col * num_rows + row]);
		fputs("]\n", stderr);
	}
}

static void
matrix_submatrix(const scalar_t *mv,
                 u_int num_cols,
                 u_int num_rows,
                 u_int without_col,
                 u_int without_row,
                 scalar_t *smv)
{
	u_int num_sub_rows = num_rows - 1;
	for (u_int super_col = 0, sub_col = 0; super_col < num_cols; ++super_col)
		if (super_col != without_col)
		{
			for (u_int super_row = 0, sub_row = 0; super_row < num_rows; ++super_row)
			{
				if (super_row != without_row)
				{
					smv[sub_col * num_sub_rows + sub_row] = mv[super_col * num_rows + super_row];
					++sub_row;
				}
			}
			++sub_col;
		}
}

static scalar_t
matrix_minor(const scalar_t *mv, u_int dim, u_int without_col, u_int without_row)
{
	u_int sub_dim = dim - 1;
	scalar_t *smv = alloca(sizeof(scalar_t) * sub_dim * sub_dim);
	matrix_submatrix(mv, dim, dim, without_col, without_row, smv);
	return matrix_det(smv, sub_dim);
}

static void
matrix_minors(const scalar_t *mv, u_int dim, scalar_t *minorv)
{
	for (u_int col = 0; col < dim; ++col)
		for (u_int row = 0; row < dim; ++row)
			minorv[col * dim + row] = matrix_minor(mv, dim, col, row);
}

static scalar_t
matrix_det(const scalar_t *mv, u_int dim)
{
	if (dim == 1)
		return mv[0];
	else if (dim == 2)
		return mv[0] * mv[3] - mv[2] * mv[1];
	else
	{
		scalar_t det = 0;
		for (u_int col = 0; col < dim; ++col)
		{
			scalar_t minor = matrix_minor(mv, dim, col, 0);
			if (minor != 0)
			{
				if (col % 2 == 0)
					det += mv[col * dim] * minor;
				else
					det -= mv[col * dim] * minor;
			}
		}
		return det;
	}
}

static void
matrix_trans_square(const scalar_t *mv, u_int dim, scalar_t *tmv)
{
	for (u_int col = 0; col + 1 < dim; ++col)
		for (u_int row = col + 1; row < dim; ++row)
		{
			scalar_t tmp = mv[col * dim + row];
			tmv[col * dim + row] = mv[row * dim + col];
			tmv[row * dim + col] = tmp;
		}
}

static void
matrix_divide_scalar(const scalar_t *mv, u_int num_cols, u_int num_rows, scalar_t divisor, scalar_t *rm)
{
	for (u_int col = 0; col < num_cols; ++col)
		for (u_int row = 0; row < num_rows; ++row)
			rm[col * num_rows + row]/= divisor;
}

static void
matrix_invert_trans(const scalar_t *mv, u_int dim, scalar_t *amv)
{
	assert(mv != amv);
	scalar_t det = matrix_det(mv, dim);
	assert(det != 0);
	matrix_minors(mv, dim, amv);
	//matrix_print(imv, dim, dim, "minors");
	for (u_int col = 0; col < dim; ++col)
		for (u_int row = 0; row < dim; ++row)
			if ((col + row) % 2 == 1)
				amv[col * dim + row] = -amv[col * dim + row];
	//matrix_print(imv, dim, dim, "transpose cofactors AKA classic adjoint");
	matrix_divide_scalar(amv, dim, dim, det, amv);
}

static void
matrix_invert(const scalar_t *mv, u_int dim, scalar_t *imv)
{
	matrix_invert_trans(mv, dim, imv);
	matrix_trans_square(imv, dim, imv);
}

static void
matrix_check_inverse(const scalar_t *mv, u_int dim, const scalar_t *imv)
{
	scalar_t *iv = alloca(sizeof(scalar_t) * dim * dim);
	matrix_mult_matrix(mv, imv, dim, dim, iv);
	for (u_int col = 0; col < 4; ++col)
		for (u_int row = 0; row < 4; ++row)
		{
			scalar_t expect = (col == row) ? 1.0 : 0;
			if (fabs(iv[col * dim + row] - expect) > 1.0e-7)
			{
				fprintf(stderr, "matrix × inverse ≠ identity\n");
				matrix_print(mv, dim, dim, "matrix");
				matrix_print(imv, dim, dim, "inverse");
				matrix_print(iv, dim, dim, "matrix × inverse");
				return;
			}
		}
}

static void
matrix_mult_vector(const scalar_t *mv, u_int num_cols, u_int num_rows, const scalar_t *vv, scalar_t *resultv)
{
	assert(vv != resultv);
	for (u_int row = 0; row < num_rows; ++row)
	{
		scalar_t dot = 0;
		for (u_int col = 0; col < num_cols; ++col)
			dot+= mv[col * num_rows + row] * vv[col];
		resultv[row] = dot;
	}
}

struct matrix2x2
matrix2x2_build(struct vector2 u, struct vector2 v)
{
	return (struct matrix2x2){.cols[0][0] = u.x, .cols[0][1] = u.y, .cols[1][0] = v.x, .cols[1][1] = v.y};
}

scalar_t
matrix2x2_det(const struct matrix2x2 m)
{
	return matrix_det(m.m, 2);
}

struct vector2
matrix2x2_mult_vector2(const struct matrix2x2 m, const struct vector2 v)
{
	struct vector2 result;
	matrix_mult_vector(m.m, 2, 2, v.v, result.v);
	return result;
}

scalar_t
matrix3x3_minor(const struct matrix3x3 m, u_int without_col, u_int without_row)
{
	return matrix_minor(m.m, 3, without_col, without_row);
}

scalar_t
matrix3x3_det(const struct matrix3x3 m)
{
	return matrix_det(m.m, 3);
}

struct matrix3x3
matrix3x3_invert(const struct matrix3x3 m)
{
	struct matrix3x3 im;
	matrix_invert(m.m, 3, im.m);
	return im;
}

void
matrix3x3_check_inverse(const struct matrix3x3 m, const struct matrix3x3 im)
{
	matrix_check_inverse(m.m, 3, im.m);
}

void
matrix3x3_print(const struct matrix3x3 m, const char *label)
{
	matrix_print(m.m, 3, 3, label);
}

void
matrix4x4_copy_linear(const scalar_t *ma, struct matrix4x4 *rm)
{
	bcopy(ma, rm->m, sizeof(rm->m));
}

struct matrix4x4
matrix4x4_mult_matrix4x4(const struct matrix4x4 m, const struct matrix4x4 n)
{
	struct matrix4x4 mn;
	matrix_mult_matrix(m.m, n.m, 4, 4, mn.m);
	return mn;
}

struct vector4
matrix4x4_mult_vector4(const struct matrix4x4 m, const struct vector4 v)
{
	struct vector4 result;
	matrix_mult_vector(m.m, 4, 4, v.v, result.v);
	return result;
}

struct matrix4x4
matrix4x4_make_rotation(scalar_t angle, scalar_t x, scalar_t y, scalar_t z)
{
	struct vector3 axis = {.x = x, .y = y, .z = z};
	vector3_check_norm(axis, "rotation axis");
	scalar_t angle_rad = angle / (180.0 / M_PI);
	scalar_t c = cos(angle_rad);
	scalar_t s = sin(angle_rad);
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
	return m;
}

struct matrix4x4
matrix4x4_make_scaling(scalar_t x, scalar_t y, scalar_t z)
{
	struct matrix4x4 m = {.cols = {{x, 0, 0, 0}, {0, y, 0, 0}, {0, 0, z, 0}, {0, 0, 0, 1}}};
	return m;
}

struct matrix4x4
matrix4x4_make_ortho(scalar_t left, scalar_t right, scalar_t bottom, scalar_t top, scalar_t near_z, scalar_t far_z)
{
	scalar_t width = right - left, height = top - bottom, depth = far_z - near_z;
	struct matrix4x4 m;
	bzero(&m, sizeof(m));
	m.cols[0][0] = 2.0 / width;
	m.cols[1][1] = 2.0 / height;
	m.cols[2][2] = 2.0 / depth;
	m.cols[3][0] = -(right + left) / width;
	m.cols[3][1] = -(top + bottom) / height;
	m.cols[3][2] = -(far_z + near_z) / depth;
	m.cols[3][3] = 1.0;
	return m;
}

scalar_t
matrix4x4_det(const struct matrix4x4 m)
{
	return matrix_det(m.m, 4);
}

struct matrix4x4
matrix4x4_invert_trans(const struct matrix4x4 m)
{
	struct matrix4x4 am;
	matrix_invert_trans(m.m, 4, am.m);
	return am;
}

struct matrix4x4
matrix4x4_invert(const struct matrix4x4 m)
{
	struct matrix4x4 im;
	matrix_invert(m.m, 4, im.m);
	return im;
}

void
matrix4x4_check_inverse(const struct matrix4x4 m, const struct matrix4x4 im)
{
	matrix_check_inverse(m.m, 4, im.m);
}

void
matrix4x4_print(const struct matrix4x4 m, const char *label)
{
	matrix_print(m.m, 4, 4, label);
}
