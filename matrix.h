//
// Created by Seth Kingsley on 1/5/18.
//

#ifndef SOGL_MATRIX_H
#define SOGL_MATRIX_H

#include <OpenGL/OpenGL.h>
#include "vector.h"

struct matrix3x3
{
	union
	{
		GLdouble cols[3][3];
		GLdouble m[9];
	};
};
#define IDENTITY_MATRIX3X3 {.cols = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}};
struct matrix4x4
{
	union
	{
		GLdouble cols[4][4];
		GLdouble m[16];
	};
};
#define IDENTITY_MATRIX4X4 {.cols = {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}}

GLdouble matrix3x3_minor(const struct matrix3x3 m, GLuint col, GLuint row);
GLdouble matrix3x3_det(const struct matrix3x3 m);
struct matrix3x3 matrix3x3_invert(const struct matrix3x3 m);
void matrix3x3_check_inverse(const struct matrix3x3 m, const struct matrix3x3 im);
void matrix3x3_print(const struct matrix3x3 m, const char *label);
void matrix4x4_copy_linear(const GLdouble *ma, struct matrix4x4 *rm);
struct matrix4x4 matrix4x4_mult_matrix4x4(const struct matrix4x4 m, const struct matrix4x4 n);
void matrix4x4_mult_vec4(const struct matrix4x4 m, const vec4_t v, vec4_t rv);
struct matrix4x4 matrix4x4_make_rotation(GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
struct matrix4x4 matrix4x4_make_scaling(GLdouble x, GLdouble y, GLdouble z);
GLdouble matrix4x4_det(const struct matrix4x4 m);
struct matrix4x4 matrix4x4_invert(const struct matrix4x4 m);
void matrix4x4_check_inverse(const struct matrix4x4 m, const struct matrix4x4 im);
void matrix4x4_print(const struct matrix4x4 m, const char *label);

#endif //SOGL_MATRIX_H
