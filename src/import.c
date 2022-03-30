#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "graph.h"
#include "import.h"

#define LINE_MAX 1024

int
ffl_edge_format(FFL_Graph *graph, char *line, const FFL_FileFlavor *flavor)
{
	errno = 0;
	int source = (int) strtol(line, &line, 10);
	if (errno) return -1;
	if (*line++ != flavor->delimiter) return -1;
	int target = (int) strtol(line, &line, 10);
	if (errno) return -1;

	if (source >= graph->nverts) ffl_grow_vertices(graph, source + 1);
	if (target >= graph->nverts) ffl_grow_vertices(graph, target + 1);

	int eid = ffl_add_edge(graph);

	graph->edges[eid].source  = source;
	graph->edges[eid].target  = target;
	graph->edges[eid].dlength = 1.0f;

	return 0;
}

int
ffl_incidence_format(FFL_Graph *graph, char *line, const FFL_FileFlavor *flavor)
{
	errno = 0;
	int vert = (int) strtol(line, &line, 10);
	if (errno) return -1;
	if (*line++ != flavor->delimiter) return -1;
	int edge = (int) strtol(line, &line, 10);
	if (errno) return -1;

	if (vert >= graph->nverts) ffl_grow_vertices(graph, vert + 1);
	if (edge >= graph->nedges) ffl_grow_edges(graph, edge + 1);

	FFL_Edge *ep = &graph->edges[edge];
	if (ep->source < 0) {
		ep->source = vert;
	} else if (ep->target < 0) {
		ep->target = vert;
	} else {
		return -1;
	}

	return 0;
}

// TODO ffl_adjacency_format()
// TODO ffl_matrix_format()

FFL_Graph *
ffl_import(FILE *file, const FFL_FileFlavor *flavor)
{
	FFL_Graph *graph = ffl_make_graph();

	char line[LINE_MAX];
	while (fgets(line, LINE_MAX, file)) {
		if (flavor->comments_allowed && line[0] == flavor->comment_marker) continue;
		if (flavor->format_reader(graph, line, flavor) < 0) goto fail;
	}
	if (ferror(file)) goto fail;

	return graph;
fail:
	ffl_free_graph(graph);
	return NULL;
}

