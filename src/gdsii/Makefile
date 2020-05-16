COMPILER = gcc
COMPILERFLAGS = -std=c99 -Wall -Wextra -Wpedantic -Werror -O2

run: gdsii_test
	./gdsii_test

gdsii_test: $(wildcard *.c) $(wildcard *.h) 
	$(COMPILER) $(COMPILERFLAGS) -o gdsii_test gdsii_test.c