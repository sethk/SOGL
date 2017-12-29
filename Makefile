CFLAGS+= -I. -Wall -Wformat -Wextra -Wno-deprecated -Wno-unused-parameter
CFLAGS+= -O0 -g -fno-inline

PROGS = cube n64 flat
all: $(PROGS) $(addprefix opengl_,$(PROGS))

.DELETE_ON_ERROR: wrap_glut.c
wrap_glut.c:
	./gen_wrappers.rb glut /System/Library/Frameworks/GLUT.framework/GLUT \
	    < /System/Library/Frameworks/GLUT.framework/Headers/glut.h > wrap_glut.c

gl.o: wrap_glut.c

cube: cube.o gl.o

glutplane: glutplane.o gl.o

n64: n64.o gl.o

flat: flat.o gl.o

movelight: movelight.o gl.o

scene: scene.o gl.o

sphere: sphere.o gl.o

.PHONY: clean
clean::
	rm -f $(PROGS) *.o

.PHONY: debug
debug:: cube
	lldb $^

opengl_%: %.o
	$(CC) -framework OpenGL -framework GLUT -o $@ $< $(LDLIBS)
