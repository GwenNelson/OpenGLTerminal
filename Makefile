CC = cc  -Wall -g 

all:
	$(CC) gl_term.c glterm_test.c -o glterm_test -framework OpenGL -lGLU -lm `sdl-config --cflags --libs` 

clean:
	@echo Cleaning up...
	@rm lambda_logo
	@echo Done.
