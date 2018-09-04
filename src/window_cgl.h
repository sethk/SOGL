//
// Created by Seth Kingsley on 1/12/18.
//

#ifndef SOGL_WINDOW_CGL_H
#define SOGL_WINDOW_CGL_H

#include <OpenGL/OpenGL.h>
#include <OpenGL/CGLContext.h>
#include "window.h"

struct cgl_window
{
	struct window base;
	GLIContext context;
	GLIFunctionDispatch dispatch;
};

struct window *window_create_cgl(CGLContextObj context);

#endif //SOGL_WINDOW_CGL_H
