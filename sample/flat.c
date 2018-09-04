#include <GLUT/glut.h>
#include <math.h>

GLfloat light_diffuse[] = {1.0, 0.0, 0.0, 1.0};  /* Red diffuse light. */
GLfloat light_position[] = {0.0, 0.0, 1.0, 0.0};
GLfloat heading = 0;

void
display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);

	glLoadIdentity();

	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glRotatef(heading, 0, 1, 0);

	glBegin(GL_QUADS);
	glNormal3f(0, 0, 1);
	glVertex3f(-1, -1, 0);
	glVertex3f(-1, 1, 0);
	glVertex3f(1, 1, 0);
	glVertex3f(1, -1, 0);
	glEnd();

	glutSwapBuffers();
}

void
keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
		case 'x':
			heading = roundf(heading + 5);
			glutPostRedisplay();
			break;
	}
}

int
main(int ac, char **av)
{
	glutInit(&ac, av);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutCreateWindow("Flat");
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);

	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);

	glMatrixMode(GL_PROJECTION);
	gluOrtho2D(-1, 1, -1, 1);
	glMatrixMode(GL_MODELVIEW);

	glutMainLoop();
	return 0;
}
