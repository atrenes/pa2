all:
	clang -std=c99 -Wall -pedantic *.c -L. -lruntime