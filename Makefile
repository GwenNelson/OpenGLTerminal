CC = cc  -Wall -g -I/opt/local/include -L/opt/local/lib

all:
	$(CC) -DDEBUG libvterm/*.c -Ioglconsole/ oglconsole/*.c -Ilibvterm/ gl_term.c glterm_test.c -o glterm_test -framework OpenGL -lvncclient -lpthread -lm `sdl2-config --cflags --libs` 

