#!/usr/bin/tcc -run

#include <stdio.h>
#include <stdlib.h>

int
main(int argc, const char *argv[])
{
	if (argc != 2) exit(1);
	int side = atoi(argv[1]);
	for (int r = 0; r < side; r++) {
		for (int c = 0; c < side; c++) {
			int v = r * side + c;
			if (c < side - 1) printf("%d\t%d\n", v, v + 1);
			if (r < side - 1) printf("%d\t%d\n", v, v + side);
		}
	}
}

