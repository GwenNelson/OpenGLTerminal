CC = cc  -Wall -g 

all:
	$(CC) -DDEBUG libvterm/*.c -Ilibvterm/ gl_term.c glterm_test.c -o glterm_test -framework OpenGL  -lm `sdl2-config --cflags --libs` 

