#include <stdio.h>
#include <string.h>

#include "graph.h"

#define LINE_MAX 1024

#define FREM 0
#define FDEL 1
#define FENC 2

static int
get_cell(int idx)
{
}

static int
edge_line(FFL_Graph *graph, char *line)
{
	int source = get_cell(0);
	int target = get_cell(1);

	int eid = ffl_add_edge(graph);

	return 0;
}

// TODO incidence_line()
// TODO adjacency_line()
// TODO matrix_line()

FFL_Graph *
ffl_import(FILE *file, const char *flavor)
{
	FFL_Graph *graph = ffl_make_graph();

	int (*linefunc)(FFL_Graph *, char *);
	switch (flavor[FENC]) {
	case 'E': linefunc = edge_line; break;
	default: goto fail;
	}

	char line[LINE_MAX];
	while (fgets(line, LINE_MAX, file)) {
		if (flavor[FREM] != '0' && line[0] == flavor[FREM]) continue;
		if (!linefunc(graph, line)) goto fail;
	}
	if (ferror(file)) goto fail;

	return graph;
fail:
	ffl_free_graph(graph);
	return NULL;
}

