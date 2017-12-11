#if 1
# define glClear soglClear
# define glBegin soglBegin
# define glEnd soglEnd
# define glVertex3fv soglVertex3fv
# define glNormal3fv soglNormal3fv
#endif
#if 1
# define glEnable soglEnable
# define glLightfv soglLightfv
#endif
#if 1
# define glMatrixMode soglMatrixMode
# define glRotatef soglRotatef
# define glTranslatef soglTranslatef
#endif
#if 1
# define gluLookAt sogluLookAt
# define gluPerspective sogluPerspective
#endif

#include <GL/glut.h>
