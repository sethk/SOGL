//
// Created by Seth Kingsley on 1/25/18.
//

#include <GLUT/glut.h>
#include <assert.h>

static const GLint size = 5;
static const GLdouble scale_denom = 1.0;

static void
viewport(GLint col, GLint row)
{
	static const GLint padding = 1;

	assert(col >= 0 && row >= 0);
	glViewport((col + 1) * padding + col * size, (row + 1) * padding + row * size, size, size);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, 4, 0, 4);
	glColor3f(0, 0, 0);
	glRectd(-1, -1, 6, 6);
	glColor3f(1, 1, 1);
}

static void
draw_line(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2)
{
	glBegin(GL_LINES);
	glVertex2d(x1 / scale_denom, y1 / scale_denom);
	glVertex2d(x2 / scale_denom, y2 / scale_denom);
	glEnd();
}

/*
static void
draw_triangle(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2, GLdouble x3, GLdouble y3)
{
}
 */

static void
display(void)
{
	glClearColor(0.5, 0.5, 0.5, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	/*
	viewport(0, 0);
	draw_line(-2, 2, 2, 2);

	viewport(1, 0);
	draw_line(-2, 2, 2, 1);

	viewport(2, 0);
	draw_line(-2, 2, 2, 0);

	viewport(3, 0);
	draw_line(-2, 2, 2, -1);

	viewport(4, 0);
	draw_line(-2, 2, 2, -2);

	viewport(5, 0);
	draw_line(-2, 2, 1, -2);

	viewport(6, 0);
	draw_line(-2, 2, 0, -2);

	viewport(7, 0);
	draw_line(-2, 2, -1, -2);

	viewport(8, 0);
	draw_line(-2, 2, -2, -2);

	viewport(9, 0);
	draw_line(-2, -2, 2, -2);

	viewport(10, 0);
	draw_line(-2, -2, 2, -1);

	viewport(11, 0);
	draw_line(-2, -2, 2, 0);

	viewport(12, 0);
	draw_line(-2, -2, 2, 1);

	viewport(13, 0);
	draw_line(-2, -2, 2, 2);
	 */

	GLint vx = 0, vy = 0;
	for (GLint x1 = 0; x1 < size; ++x1)
		for (GLint y1 = 0; y1 < size; ++y1)
			for (GLint x2 = 0; x2 < size; ++x2)
				for (GLint y2 = 0; y2 < size; ++y2)
				{
					viewport(vx, vy);
					draw_line(x1, y1, x2, y2);

					if (++vx == 25)
					{
						vx = 0;
						++vy;
					}
				}


	glutSwapBuffers();
}

int
main(int ac, char *av[])
{
	glutInit(&ac, av);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutCreateWindow(av[0]);
	glutDisplayFunc(display);
	glutMainLoop();
	return 0;
}