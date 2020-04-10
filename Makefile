COMPILER = g++
COMPILERFLAGS = -std=c++11 -Wall -Wextra -Wpedantic -Werror -O2
INCLUDEFLAGS =
LINKERFLAGS = -lglfw -lGL -lGLEW -Iglm -ltriangle -lboost_system -lboost_filesystem

all: bin/gdsiiview

run: bin/gdsiiview
	./bin/gdsiiview example/example.gdsiiview

bin/gdsiiview: src/main.cpp src/mesh.h src/part.h src/shader.h src/tesselate.h src/gdsii.h
	$(COMPILER) $(COMPILERFLAGS) $(INCLUDEFLAGS) $(LINKERFLAGS) -o bin/gdsiiview src/main.cpp

test: src/test
	./bin/test

src/test: src/test.cpp src/gdsii.h
	$(COMPILER) $(COMPILERFLAGS) $(INCLUDEFLAGS) $(LINKERFLAGS) -o bin/test src/test.cpp

