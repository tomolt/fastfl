#include <stdio.h>
#include <stdlib.h>

#include <fastfl.h>

#include "graph.h"
#include "arg.h"

extern int graphml_read(const char *filename, FFL_Graph *graph);

char *argv0;

static void
usage(void)
{
	fprintf(stderr, "usage: %s [FILE]\n", argv0);
	exit(1);
}

static void
dump_graph(const FFL_Graph *graph)
{
	printf("V=%d, E=%d\n", graph->nverts, graph->nedges);
	for (int e = 0; e < graph->nedges; e++) {
		printf("v%d -> v%d\n", graph->edges[e].source, graph->edges[e].target);
	}
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

	FFL_Graph graph;
	if (graphml_read(*argv, &graph) < 0) {
		fprintf(stderr, "Error reading GraphML file.\n");
		exit(1);
	}

	dump_graph(&graph);

	return 0;
}

