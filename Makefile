CC=gcc
CFLAGS=-I. -Wall -Wno-deprecated-declaration
LIBS=-L/opt/homebrew/opt/jansson/lib -ljansson -framework OpenGL -framework GLUT 
DEPS =-I/opt/homebrew/opt/jansson/include parser.h
OBJ =

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) $(LIBS)

main:
	$(CC) -o main parser.o $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f *.o main
