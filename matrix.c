//
// Created by Seth Kingsley on 1/5/18.
//

#include <strings.h>
#include <stdio.h>
#include <assert.h>
#include <alloca.h>
#include "matrix.h"
#include "math.h"

static GLdouble matrix_det(const GLdouble *mv, GLuint dim);

static void
matrix_mult_matrix(const GLdouble *mv, const GLdouble *nv, GLuint num_cols, GLuint num_rows, GLdouble *rmv)
{
	for (GLuint col = 0; col < num_cols; ++col)
		for (GLuint row = 0; row < num_rows; ++row)
		{
			GLdouble dot = 0;
			for (GLuint i = 0; i < num_cols; ++i)
				dot+= mv[i * num_cols + row] * nv[col * num_cols + i];
			rmv[col * num_rows + row] = dot;
		}
}

static void
matrix_print(const GLdouble *mv, GLuint num_cols, GLuint num_rows, const char *label)
{
	GLuint label_line = num_rows / 2;
	int label_width = strlen(label);
	for (GLuint row = 0; row < num_rows; ++row)
	{
		fprintf(stderr, "%*s ", label_width, (row == label_line) ? label : "");
		fputc('[', stderr);
		for (GLuint col = 0; col < num_cols; ++col)
			fprintf(stderr, "%s% 5.3g", (col > 0) ? ", " : "", mv[col * num_rows + row]);
		fputs("]\n", stderr);
	}
}

static void
matrix_submatrix(const GLdouble *mv,
                 GLuint num_cols,
                 GLuint num_rows,
                 GLuint without_col,
                 GLuint without_row,
                 GLdouble *smv)
{
	GLuint num_sub_rows = num_rows - 1;
	for (GLuint super_col = 0, sub_col = 0; super_col < num_cols; ++super_col)
		if (super_col != without_col)
		{
			for (GLuint super_row = 0, sub_row = 0; super_row < num_rows; ++super_row)
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

static GLdouble
matrix_minor(const GLdouble *mv, GLuint dim, GLuint without_col, GLuint without_row)
{
	GLuint sub_dim = dim - 1;
	GLdouble *smv = alloca(sizeof(GLdouble) * sub_dim * sub_dim);
	matrix_submatrix(mv, dim, dim, without_col, without_row, smv);
	return matrix_det(smv, sub_dim);
}

static void
matrix_minors(const GLdouble *mv, GLuint dim, GLdouble *minorv)
{
	for (GLuint col = 0; col < dim; ++col)
		for (GLuint row = 0; row < dim; ++row)
			minorv[col * dim + row] = matrix_minor(mv, dim, col, row);
}

static GLdouble
matrix_det(const GLdouble *mv, GLuint dim)
{
	if (dim == 1)
		return mv[0];
	else if (dim == 2)
		return mv[0] * mv[3] - mv[2] * mv[1];
	else
	{
		GLdouble det = 0;
		for (GLuint col = 0; col < dim; ++col)
		{
			GLdouble minor = matrix_minor(mv, dim, col, 0);
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
matrix_trans_square(const GLdouble *mv, GLuint dim, GLdouble *tmv)
{
	for (GLuint col = 0; col + 1 < dim; ++col)
		for (GLuint row = col + 1; row < dim; ++row)
		{
			GLdouble tmp = mv[col * dim + row];
			tmv[col * dim + row] = mv[row * dim + col];
			tmv[row * dim + col] = tmp;
		}
}

static void
matrix_divide_scalar(const GLdouble *mv, GLuint num_cols, GLuint num_rows, GLdouble divisor, GLdouble *rm)
{
	for (GLuint col = 0; col < num_cols; ++col)
		for (GLuint row = 0; row < num_rows; ++row)
			rm[col * num_rows + row]/= divisor;
}

static void
matrix_invert_trans(const GLdouble *mv, GLuint dim, GLdouble *amv)
{
	assert(mv != amv);
	GLdouble det = matrix_det(mv, dim);
	assert(det != 0);
	matrix_minors(mv, dim, amv);
	//matrix_print(imv, dim, dim, "minors");
	for (GLuint col = 0; col < dim; ++col)
		for (GLuint row = 0; row < dim; ++row)
			if ((col + row) % 2 == 1)
				amv[col * dim + row] = -amv[col * dim + row];
	//matrix_print(imv, dim, dim, "transpose cofactors AKA classic adjoint");
	matrix_divide_scalar(amv, dim, dim, det, amv);
}

static void
matrix_invert(const GLdouble *mv, GLuint dim, GLdouble *imv)
{
	matrix_invert_trans(mv, dim, imv);
	matrix_trans_square(imv, dim, imv);
}

static void
matrix_check_inverse(const GLdouble *mv, GLuint dim, const GLdouble *imv)
{
	GLdouble *iv = alloca(sizeof(GLdouble) * dim * dim);
	matrix_mult_matrix(mv, imv, dim, dim, iv);
	for (GLuint col = 0; col < 4; ++col)
		for (GLuint row = 0; row < 4; ++row)
		{
			GLdouble expect = (col == row) ? 1.0 : 0;
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
matrix_mult_vector(const GLdouble *mv, GLuint num_cols, GLuint num_rows, const GLdouble *vv, GLdouble *resultv)
{
	assert(vv != resultv);
	for (GLuint row = 0; row < num_rows; ++row)
	{
		GLdouble dot = 0;
		for (GLuint col = 0; col < num_cols; ++col)
			dot+= mv[col * num_rows + row] * vv[col];
		resultv[row] = dot;
	}
}

GLdouble
matrix3x3_minor(const struct matrix3x3 m, GLuint without_col, GLuint without_row)
{
	return matrix_minor(m.m, 3, without_col, without_row);
}

GLdouble
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
matrix4x4_copy_linear(const GLdouble *ma, struct matrix4x4 *rm)
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
matrix4x4_make_rotation(GLdouble angle, GLdouble x, GLdouble y, GLdouble z)
{
	struct vector3 axis = {.x = x, .y = y, .z = z};
	vector3_check_norm(axis, "rotation axis");
	GLdouble angle_rad = angle / (180.0 / M_PI);
	GLdouble c = cos(angle_rad);
	GLdouble s = sin(angle_rad);
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
matrix4x4_make_scaling(GLdouble x, GLdouble y, GLdouble z)
{
	struct matrix4x4 m = {.cols = {{x, 0, 0, 0}, {0, y, 0, 0}, {0, 0, z, 0}, {0, 0, 0, 1}}};
	return m;
}

GLdouble
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
