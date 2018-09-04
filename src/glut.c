#include <GLUT/glut.h>

#include <stddef.h>
#include <assert.h>
#include "glut.h"
#include "gl.h"
#include "../window_cgl.h"
#include "../draw.h"
#include "render.h"
#include "wrap_glut.h"

#define SHOW_DEBUG_WIN 1
#define DEBUG_WIN (-5)

void *glutStrokeRoman;

struct window *main_window = NULL;
int glut_main_win = -1;
struct {int x, y;} main_win_pos;
struct {int width, height;} init_win_size;
int debug_win = -1;
int debug_save_win;

void (*reshape_func)(int width, int height);
void (*idle_func)(void);

static void
glut_reshape(int width, int height)
{
	draw_reshape(drawable, width, height);

	if (reshape_func)
		reshape_func(width, height);
	else
		glViewport(0, 0, width, height);

	if (glut_push_debug())
	{
		openGLUTReshapeWindow(width, height);
		openGLUTPositionWindow(main_win_pos.x + width + 20, main_win_pos.y);
		glut_pop_debug();
	}
}

static void
glut_debug_display(void)
{
	//glutPostRedisplay();
	//glutSwapBuffers();
}

bool
glut_push_debug(void)
{
	if (debug_rend)
	{
		debug_save_win = glutGetWindow();
		assert(debug_save_win != DEBUG_WIN);
		glutSetWindow(DEBUG_WIN);
		return true;
	}
	else
		return false;
}

void
glut_pop_debug(void)
{
	assert(debug_rend);
	assert(glutGetWindow() == DEBUG_WIN);
	glutSetWindow(debug_save_win);
}

static void
glut_idle(void)
{
	if (openGLUTGetWindow() == debug_win)
		openGLUTSetWindow(glut_main_win);

	if (idle_func)
		idle_func();
}

void
glutInit(int *argcp, char **argv)
{
	openglut_init();

	openGLUTInit(argcp, argv);

	//openGLUTInitWindowPosition(20, 20);
	glutInitWindowPosition(20, 20);
	glutInitWindowSize(300, 300);
}

void
glutInitWindowPosition(int x, int y)
{
	main_win_pos.x = x;
	main_win_pos.y = y;
	openGLUTInitWindowPosition(x, y);
}

void
glutInitWindowSize(int width, int height)
{
	init_win_size.width = width;
	init_win_size.height = height;
	openGLUTInitWindowSize(width, height);
}

void
glutMainLoop(void)
{
#if SHOW_DEBUG_WIN
	if (debug_win == -1)
	{
		openGLUTInitWindowPosition(main_win_pos.x + init_win_size.width + 20, main_win_pos.y);
		openGLUTInitWindowSize(init_win_size.width, init_win_size.height);
		openGLUTInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
		debug_win = openGLUTCreateWindow("Debug");
		openGLUTDisplayFunc(glut_debug_display);
		openGLUTKeyboardFunc(render_debug_key);
		render_init_debug();
		openGLUTSetWindow(glut_main_win);
	}
#endif // SHOW_DEBUG_WIN

	openGLUTMainLoop();
}

int
glutCreateWindow(const char *title)
{
	assert(glut_main_win == -1);
	glut_main_win = openGLUTCreateWindow(title);
	openGLUTReshapeFunc(glut_reshape);
	openGLUTIdleFunc(glut_idle);
	CGLContextObj context = CGLGetCurrentContext();
	main_window = window_create_cgl(context);
	//main_win->width = init_win_size.width;
	//main_win->height = init_win_size.height;
	drawable = draw_create(main_window);
	draw_reshape(drawable, init_win_size.width, init_win_size.height);
	gl_setup_context(context);
	return glut_main_win;
}

int
glutGetWindow(void)
{
	int win = openGLUTGetWindow();
	return (win == debug_win) ? DEBUG_WIN : win;
}

void
glutSetWindow(int win)
{
	assert(win != DEBUG_WIN || debug_win != -1);
	openGLUTSetWindow((win == DEBUG_WIN) ? debug_win : win);
}

void
glutReshapeFunc(void (*func)(int width, int height))
{
	reshape_func = func;
}

void
glutIdleFunc(void (*fp)(void))
{
	idle_func = fp;
}

void
glutSwapBuffers(void)
{
	draw_finish(drawable);
	openGLUTSwapBuffers();

	if (glut_push_debug())
	{
		openGLUTSwapBuffers();
		glut_pop_debug();
	}
}

void
glutPostRedisplay(void)
{
	if (openGLUTGetWindow() == debug_win)
		openGLUTPostWindowRedisplay(glut_main_win);
	else
		openGLUTPostRedisplay();
}
