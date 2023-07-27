CC=g++
OBJC=clang++
CFLAGS=-I. -I/opt/homebrew/opt/glm/include -I/opt/homebrew/opt/jansson/include -I/opt/homebrew/opt/glfw/include -Wall -Wno-deprecated-declarations -std=c++17
LIBS=-L/opt/homebrew/opt/jansson/lib -ljansson -L/opt/homebrew/opt/glfw/lib -lglfw -framework OpenGL -framework GLUT -framework AppKit
DEPS = parser.h shaders.h
OBJ = main2.o parser.o shaders.o

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

main: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

debug: CFLAGS += -g
debug: $(OBJ)
	$(CC) -g -o $@ $^ $(CFLAGS) $(LIBS)


.PHONY: clean

clean:
	rm -f *.o main
