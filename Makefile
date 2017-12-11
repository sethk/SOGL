CFLAGS+= -I. -Wall -Wformat -Wextra -Wno-deprecated -Wno-unused-parameter
CFLAGS+= -O0 -g -fno-inline
LDFLAGS = -framework GLUT -framework OpenGL

all: cube

cube.o: sogl.h

cube: cube.o gl.o

.PHONY: clean
clean::
	rm -f cube *.o

.PHONY: debug
debug:: cube
	lldb $^
