//
// Created by Seth Kingsley on 1/25/18.
//

#include <GLUT/glut.h>
#include <assert.h>

static void
viewport(GLint x, GLint y, GLint w, GLint h)
{
	glViewport(x, y, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, w, 0, h);
	glColor3f(0, 0, 0);
	glRectd(0, 0, w, h);
}

static void
grey(GLdouble intensity)
{
	glColor3d(intensity, intensity, intensity);
}

static void
point(GLdouble x, GLdouble y)
{
	glBegin(GL_POINTS);
	glVertex2d(x, y - 0.5);
	glEnd();
}

static void
line(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2)
{
	glBegin(GL_LINES);
	glVertex2d(x1, y1);
	glVertex2d(x2, y2);
	glEnd();
}

static void
triangle(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2, GLdouble x3, GLdouble y3)
{
	glBegin(GL_TRIANGLES);
	glVertex2d(x1, y1);
	glVertex2d(x2, y2);
	glVertex2d(x3, y3);
	glEnd();
}

static void
display(void)
{
	glClearColor(0.5, 0.5, 0.5, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	viewport(5, 5, 14, 6);
	grey(0.5);
	point(0, 6);
	point(1.5, 4.5);
	point(3.5, 4);
	point(6, 4.5);
	point(12, 4);
	grey(0.25);
	point(13, 4);
	grey(0.5);
	point(0.25, 2.75);
	point(3.5, 1.25);
	point(5.25, 1.5);
	point(8.75, 1.25);
	point(0, 0);
	point(11, 0);
	point(14, 0);

	viewport(5, 15, 16, 8);
	grey(0.5);
	triangle(1, 7, 6, 6, 2, 4);
	triangle(4.5, 7.5, 4.5, 7.5, 4.5, 7.5);
	triangle(5.25, 6.75, 6.25, 7.75, 6.25, 6.75);
	triangle(6.5, 6.5, 7.5, 7.5, 7.5, 6.5);
	grey(0.25);
	triangle(7.75, 5.5, 9.75, 7.25, 11.75, 5.5);
	grey(0.5);
	triangle(13.5, 6.5, 15, 8, 14.5, 5.5);
	triangle(7.75, 5.5, 11.75, 5.5, 9.5, 2.75);
	triangle(13.5, 6.5, 14.5, 5.5, 14.5, 3.5);
	grey(0.25);
	triangle(1, 2, 7, 4, 5, 2);
	grey(0.5);
	triangle(5, 2, 7, 4, 8, 1);
	grey(0.25);
	triangle(7, 4, 9.5, 2.5, 8, 1);
	triangle(11.5, 3.5, 12.5, 2.5, 11.5, 1.5);
	grey(0.5);
	triangle(13.5, 2.5, 15.5, 2.5, 13.5, 0.5);
	grey(0.25);
	triangle(13.5, 0.5, 15.5, 2.5, 15.5, 0.5);
	grey(0.5);
	triangle(9.5, 0.5, 10.5, 0.5, 9.5, -2);
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

	/*
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
				*/


	glutSwapBuffers();
}

int
main(int ac, char *av[])
{
	glutInit(&ac, av);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(50, 50);
	glutCreateWindow(av[0]);
	glutDisplayFunc(display);
	glutMainLoop();
	return 0;
}