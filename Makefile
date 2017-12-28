CFLAGS+= -I. -Wall -Wformat -Wextra -Wno-deprecated -Wno-unused-parameter
CFLAGS+= -O0 -g -fno-inline
LDFLAGS = -framework GLUT #-framework OpenGL

all: cube n64 #glutplane

cube.o: sogl.h

cube: cube.o gl.o

glutplane: glutplane.o gl.o

n64: n64.o gl.o

.PHONY: clean
clean::
	rm -f cube glutplane n64 *.o

.PHONY: debug
debug:: cube
	lldb $^
