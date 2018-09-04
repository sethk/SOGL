//
// Created by Seth Kingsley on 1/25/18.
//

#include <GLUT/glut.h>
#include <stdio.h>
#include <math.h>
#include <sys/param.h>

static GLuint win_width, win_height;
static GLuint width = 10, height = 10;
static GLuint mode = 0;
static GLuint order = 0;
static GLuint vert_order = 0;

void
reshape(int w, int h)
{
	win_width = w;
	win_height = h;
	width = MIN(win_width - 5, width);
	height = MIN(win_height - 5, height);
	fprintf(stderr, "%ux%u\n", width, height);
	glViewport(5, 5, width, height);
}

void
rectangle1(void)
{
	GLfloat vertices[][2] =
			{
					{-1, 0},
					{-1, 1},
					{1, 1},
					{1, 0}
			};
	glBegin(GL_QUADS);
	glColor3f(1.0f, 0.0f, 0.0f);
	for (GLuint i = 0; i < 4; ++i)
		glVertex2fv(vertices[(vert_order + i) % 4]);
	glEnd();
}

void
rectangle2(void)
{
	GLfloat vertices[][2] =
			{
					{-1, -1},
					{-1, 0},
					{1, 0},
					{1, -1}
			};
	glBegin(GL_QUADS);
	glColor3f(0.0f, 0.0f, 1.0f);
	for (GLuint i = 0; i < 4; ++i)
		glVertex2fv(vertices[(vert_order + i) % 4]);
	glEnd();
}

void
rectangle3(void)
{
	glBegin(GL_QUADS);
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex2f(-1, 1);
	glVertex2f(0, 1);
	glVertex2f(0, -1);
	glVertex2f(-1, -1);
	glEnd();
}

void
rectangle4(void)
{
	glBegin(GL_QUADS);
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex2f(0, 1);
	glVertex2f(1, 1);
	glVertex2f(1, -1);
	glVertex2f(0, -1);
	glEnd();
}

void
triangle1(void)
{
	GLfloat vertices[][2] =
			{
					{-1, -1},
					{-1, 1},
					{1, -1}
			};
	glBegin(GL_TRIANGLES);
	glColor3f(1.0f, 0.0f, 0.0f);
	for (GLuint i = 0; i < 3; ++i)
		glVertex2fv(vertices[(vert_order + i) % 3]);
	glEnd();
}

void
triangle2(void)
{
	glBegin(GL_TRIANGLES);
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex2f(-1, 1);
	glVertex2f(1, 1);
	glVertex2f(1, -1);
	glEnd();
}

void
grid(void)
{
	glBegin(GL_POINTS);
	glColor3f(0.0, 1.0, 0.0);
	for (GLuint x = 0; x < width; x+= 2)
		for (GLuint y = 0; y < height; y+= 2)
			//if (x == 2 && y == 2)
			glVertex2f(-1 + x * (2.0 / width), -1 + y * (2.0 / height));
	glEnd();
}
void
display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);

	switch (mode)
	{
		case 0:
			if ((order % 2) == 0)
			{
				triangle1();
				triangle2();
			}
			else
			{
				triangle2();
				triangle1();
			}
			break;
		case 1:
			if ((order % 2) == 0)
			{
				rectangle1();
				rectangle2();
			}
			else
			{
				rectangle2();
				rectangle1();
			}
			break;
		case 2:
			if ((order % 2) == 0)
			{
				rectangle3();
				rectangle4();
			}
			else
			{
				rectangle4();
				rectangle3();
			}
			break;
	}
	grid();

	glutSwapBuffers();
}

void
idle(void)
{
	++order;
	glutPostRedisplay();
}

void
key(unsigned char key, int x, int y)
{
	static GLfloat zoom = 1.0;

	switch (key)
	{
		case '\t':
			mode = (mode + 1) % 3;
			break;
		case 'v':
			++vert_order;
			break;
		case 'V':
			--vert_order;
			break;
		case 'z':
			zoom = fmax(zoom - 0.1, 0.001);
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glScalef(zoom, zoom, zoom);
			break;
		case 'Z':
			zoom = fmin(zoom + 0.1, 1.0);
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glScalef(zoom, zoom, zoom);
			break;
		case 'w':
			++width;
			reshape(win_width, win_height);
			break;
		case 'h':
			++height;
			reshape(win_width, win_height);
			break;
	}
	glutPostRedisplay();
}

int
main(int ac, char **av)
{
	glutInit(&ac, av);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutCreateWindow("overlap");
	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutKeyboardFunc(key);

	glutMainLoop();
}
