CC=g++
OBJC=clang++
CFLAGS=-I. -I/opt/homebrew/opt/jansson/include -Wall -Wno-deprecated-declarations -std=c++17
LIBS=-L/opt/homebrew/opt/jansson/lib -ljansson -framework OpenGL -framework GLUT -framework AppKit
DEPS = parser.h
OBJ = main.o parser.o zoom_handler.o

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

zoom_handler.o: zoom_handler.mm
	$(OBJC) -c -o $@ $< $(CFLAGS)

main: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

debug: CFLAGS += -g
debug: $(OBJ)
	$(CC) -g -o $@ $^ $(CFLAGS) $(LIBS)


.PHONY: clean

clean:
	rm -f *.o main
