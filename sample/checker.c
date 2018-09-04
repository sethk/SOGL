//
// Created by Seth Kingsley on 4/24/18.
//

#include <GLUT/glut.h>
#include <stdio.h>
#include <sys/param.h>
#include <math.h>
#include <assert.h>

static GLsizei win_width, win_height;
static GLsizei width, height;

void
reshape(int w, int h)
{
	win_width = w;
	win_height = h;
	width = w - 2;
	height = h - 2;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(-1, width + 1, -1, height + 1);
	glMatrixMode(GL_MODELVIEW);
	printf("%ix%i\n", w, h);
}

static enum vertex_mode
{
	VERTEX_DOUBLE,
	VERTEX_FLOAT,
	VERTEX_INT,
	VERTEX_NUM_MODE
} vertex_mode = VERTEX_DOUBLE;

static int float_base = -1;
static int float_exp = 0;

static void
vertex2(GLdouble xd, GLdouble yd)
{
	double float_offset = scalbn(float_base, float_exp);
	switch (vertex_mode)
	{
		case VERTEX_INT:
			glVertex2i(xd, yd);
			break;
		case VERTEX_FLOAT:
			glVertex2f(xd + float_offset, yd + float_offset);
			break;
		case VERTEX_DOUBLE:
			glVertex2d(xd + float_offset, yd + float_offset);
			break;
		default:
			assert(!"Vertex mode not handled");
	}
}

void
display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);

	glBegin(GL_POINTS);
	glColor3f(1, 0.4, 0.4);
	for (GLint x = 0; x < width / 2; ++x)
		for (GLint y = 0; y < height / 2; ++y)
			if (((x + y) % 2) == 0)
				vertex2(x, y);
	glEnd();

	GLsizei mid_height = height / 2;
	GLsizei top_height = height - mid_height;
	glBegin(GL_LINES);
	glColor3f(0.4, 1.0, 0.4);
	for (GLint x = 0; x < width; ++x)
	{
		if (x <= top_height)
		{
			GLint y = mid_height + x;
			if ((y % 2) == 0)
			{
				vertex2(0, y);
				vertex2(top_height - x, height);
			}
			if (((width + y) % 2) == 0)
			{
				vertex2(width - x, mid_height);
				vertex2(width, y);
			}
		}
		else if (((x + height) % 2) == 0)
		{
			vertex2(x - top_height, mid_height);
			vertex2(x, height);
		}
	}
	glEnd();

	glBegin(GL_QUADS);
	glColor3f(0.4, 0.4, 1.0);
	for (GLint x = width / 2; x < width; ++x)
		for (GLint y = 0; y < height / 2; ++y)
			if (((x + y) % 2) == 0)
			{
				vertex2(x, y);
				vertex2(x, y + 1);
				vertex2(x + 1, y + 1);
				vertex2(x + 1, y);
			}
	glEnd();
	glFlush();
	glutReportErrors();
}

static void
update_caption(void)
{
	char title[64];
	double float_offset = scalbn(float_base, float_exp);
	snprintf(title, sizeof(title), "%d %g", vertex_mode, float_offset);
	glutSetWindowTitle(title);
}

static void
keyboard(u_char key, int x, int y)
{
	GLsizei new_width = win_width, new_height = win_height;

	switch (key)
	{
		case 'v':
			if (++vertex_mode == VERTEX_NUM_MODE)
				vertex_mode = 0;
			update_caption();
			glutPostRedisplay();
			break;
		case 'e':
			float_base = -float_base;
			update_caption();
			glutPostRedisplay();
			break;
		case 'f':
			if (float_exp-- == -20)
				float_exp = 0;
			update_caption();
			glutPostRedisplay();
			break;
		case 'F':
			if (float_exp++ == 0)
				float_exp = -20;
			update_caption();
			glutPostRedisplay();
			break;
		case 'x':
			++new_width;
			break;
		case 'X':
			--new_width;
			break;
		case 'y':
			++new_height;
			break;
		case 'Y':
			--new_height;
			break;
		case '1':
			new_width = new_height = 113;
			break;
		case '2':
			new_width = new_height = 226;
			break;
		case '3':
			new_width = new_height = 16;
			break;
	}

	if (new_width != win_width || new_height != win_height)
		glutReshapeWindow(new_width, new_height);
}

int
main(int ac, char *av[])
{
	glutInit(&ac, av);
	glutInitWindowSize(300, 300);
	glutCreateWindow(av[0]);
	update_caption();
	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMainLoop();
}
