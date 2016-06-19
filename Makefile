CC = cc  -Wall -g 

all:
	$(CC) gl_term.c glterm_test.c -o glterm_test -lGL -lGLU -lm `sdl2-config --cflags --libs` 

clean:
	@echo Cleaning up...
	@rm lambda_logo
	@echo Done.
