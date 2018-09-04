//
// Created by Seth Kingsley on 1/5/18.
//

#include <assert.h>
#include <stdio.h>
#include "../src/matrix.h"

int
main(void)
{
	struct matrix3x3 m3 = {.cols = {{1, 4, 2}, {3, 1, 5}, {2, 3, 2}}};
	scalar_t det = matrix3x3_minor(m3, 0, 1);
	matrix3x3_print(m3, "minor(0, 1)");
	fprintf(stderr, "= %g\n", det);
	assert(det == -4);

	det = matrix3x3_minor(m3, 1, 2);
	matrix3x3_print(m3, "minor(1, 2)");
	fprintf(stderr, "= %g\n", det);
	assert(det == -5);

	det = matrix3x3_det(m3);
	matrix3x3_print(m3, "det");
	fprintf(stderr, "= %g\n", det);

	struct matrix3x3 im3 = matrix3x3_invert(m3);
	matrix3x3_print(im3, "inverted");
	matrix3x3_check_inverse(m3, im3);

	struct matrix4x4 m4 = {.cols = {{3, 4, 3, 9}, {2, 0, 0, 2}, {0, 1, 2, 3}, {1, 2, 1, 1}}};
	det = matrix4x4_det(m4);
	matrix4x4_print(m4, "det");
	fprintf(stderr, "= %g\n", det);
}
