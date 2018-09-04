//
// Created by Seth Kingsley on 1/6/18.
//

#include <GLUT/glut.h>
#include "../src/vector.h"

void
display(void)
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glScalef(1, 0.5, 1);
	glMatrixMode(GL_PROJECTION);

	struct vector2 a = {.x = 0.2, .y = 0.8};
	struct vector2 b = {.x = 0.8, .y = 0.2};
	struct vector2 c = vector2_norm(vector2_add(a, b));
	struct vector3 normal3 = {.x = c.x, .y = c.y, .z = 0};

	glClear(GL_COLOR_BUFFER_BIT);
	glBegin(GL_LINES);
	glNormal3dv(normal3.v);
	glColor3f(1, 1, 1);
	glVertex2dv(a.v);
	glVertex2dv(b.v);
	glColor3f(1, 0, 0);
	glVertex2d(0, 0);
	glVertex3dv(normal3.v);
	glEnd();
	glFlush();
}

int
main(int ac, char *av[])
{
	glutInit(&ac, av);
	glutCreateWindow("normal");
	glutDisplayFunc(display);
	glutMainLoop();
}