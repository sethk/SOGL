#if 0
# define glClear soglClear
# define glBegin soglBegin
# define glEnd soglEnd
# define glVertex3fv soglVertex3fv
# define glVertex3f soglVertex3f
# define glNormal3fv soglNormal3fv
# define glNormal3f soglNormal3f
# define glColor3f soglColor3f
#endif
#if 0
# define glEnable soglEnable
# define glLightfv soglLightfv
#endif
#if 0
# define glMatrixMode soglMatrixMode
# define glLoadIdentity soglLoadIdentity
# define glRotatef soglRotatef
# define glTranslatef soglTranslatef
#endif
#if 0
# define gluLookAt sogluLookAt
# define gluPerspective sogluPerspective
#endif
#if 1
# define glutInit soglutInit
//# define glutDisplayFunc soglutDisplayFunc
# define glutIdleFunc soglutIdleFunc
# define glutSwapBuffers soglutSwapBuffers
# define glutCreateWindow soglutCreateWindow
# define glutPostRedisplay soglutPostRedisplay
#endif

#include <GL/glut.h>
