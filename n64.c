#include <GLUT/glut.h>
#include <sys/types.h>
#include <err.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <math.h>
#include <stdio.h>

const u_char *data = NULL;
float time = 0;

void
idle(void)
{
	time+= 0.05;
	glutPostRedisplay();
}

static void
draw_elems(GLenum mode, off_t offset, u_int16_t count)
{
	GLfloat *vertices = (GLfloat *)(data + 6948);
	GLfloat *normals = (GLfloat *)(data + 612);
	u_int16_t *indices = (u_int16_t *)(data + offset);
	glBegin(mode);
	for (u_int i = 0; i < count; ++i)
	{
		u_int16_t vertex_offset = indices[i] * 3;
		GLfloat *normal = normals + vertex_offset;
		if (normal[0] == 0 && normal[1] == 0 && fabs(normal[2]) == 1)
			glColor3f(1, 1, 0);
		else if (normal[0] == 0 && fabs(normal[1]) == 1 && normal[2] == 0)
			glColor3f(0, 1, 0);
		else if (fabs(normal[0]) == 1 && normal[1] == 0 && normal[2] == 0)
			glColor3f(0, 0, 1);
		else
		{
			//fprintf(stderr, "%g, %g, %g\n", normal[0], normal[1], normal[2]);
			if (normal[2] > 0 || normal[1] > 0)
				glColor3f(1, 0, 0);
			else
				glColor3f(0, 0, 1);
		}

		glNormal3fv(normal);
		glVertex3fv(vertices + vertex_offset);
	}
	glEnd();

}

void
display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0, 0, -5);
	glRotatef(-55, 1, 0, 0);
	GLfloat z_rot = -remainderf(time * 30, 360);
	glRotatef(z_rot, 0, 0, 1);

	draw_elems(GL_TRIANGLES, 0, 27);
	draw_elems(GL_TRIANGLE_STRIP, 56, 278);

	glutSwapBuffers();
}

int
main(int ac, char **av)
{
	int fd;
	if ((fd = open("data/model_file.bin", O_RDONLY)) == -1)
		err(1, "Open data/model_file.bin");
	const size_t len = 9060;
	if ((data = mmap(NULL, len, PROT_READ, MAP_FILE | MAP_SHARED, fd, 0)) == MAP_FAILED)
		err(1, "Mmap() model file");

	glutInit(&ac, av);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(512, 512);
	glutCreateWindow("N64 Logo");
	glutDisplayFunc(display);
	glutIdleFunc(idle);

	glMatrixMode(GL_PROJECTION);
	gluPerspective(70, 1, 0.1, 10.0);

	glEnable(GL_DEPTH_TEST);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glutMainLoop();

	munmap((void *)data, len);
	close(fd);

	return 0;
}
