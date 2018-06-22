//
// Created by Seth Kingsley on 4/24/18.
//

#include <GLUT/glut.h>
#include <stdio.h>
#include <sys/param.h>

static GLsizei width, height;

void
reshape(int w, int h)
{
	width = w;
	height = h;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, width, 0, height);
	glMatrixMode(GL_MODELVIEW);
}

void
display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);

	glBegin(GL_POINTS);
	for (GLint x = 0; x < width / 2; ++x)
		for (GLint y = 0; y < height / 2; ++y)
			if (((x + y) % 2) == 0)
				glVertex2i(x, y);
	glEnd();

	GLint extent = MAX(height, width);
	glBegin(GL_LINES);
	for (GLint x = -extent; x < extent; x+= 2)
	{
		glVertex2i(x, height / 2);
		glVertex2i(x + height / 2, height);
	}
	glEnd();

	glBegin(GL_QUADS);
	for (GLint x = width / 2; x < width; ++x)
		for (GLint y = 0; y < height / 2; ++y)
			if (((x + y) % 2) == 0)
			{
				glVertex2i(x, y);
				glVertex2i(x, y + 1);
				glVertex2i(x + 1, y + 1);
				glVertex2i(x + 1, y);
			}
	glEnd();
	glFlush();
	glutReportErrors();
}

int
main(int ac, char *av[])
{
	glutInit(&ac, av);
	glutInitWindowSize(52, 52);
	glutCreateWindow(av[0]);
	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutMainLoop();
}
