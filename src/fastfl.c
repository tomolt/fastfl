#include <stdio.h>
#include <stdlib.h>

#include <fastfl.h>

#include "arg.h"

extern int graphml_read(const char *filename);

char *argv0;

static void
usage(void)
{
	fprintf(stderr, "usage: %s\n", argv0);
	exit(1);
}

int
main(int argc, char **argv)
{
	ARGBEGIN {
	default:
		usage();
	} ARGEND

	if (argc != 1) {
		usage();
	}
	if (graphml_read(*argv) < 0) {
		fprintf(stderr, "Error reading GraphML file.\n");
		exit(1);
	}

	return 0;
}

