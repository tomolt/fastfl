#include <stdlib.h>

#include "graph.h"

FFL_Graph *
ffl_make_graph(void)
{
	FFL_Graph *graph = calloc(1, sizeof (FFL_Graph));
	graph->nverts = 0;
	graph->cverts = 16;
	graph->verts  = calloc(graph->cverts, sizeof *graph->verts);
	graph->nedges = 0;
	graph->cedges = 16;
	graph->edges  = calloc(graph->cedges, sizeof *graph->edges);
	return graph;
}

void
ffl_free_graph(FFL_Graph *graph)
{
	free(graph->verts);
	free(graph->edges);
	free(graph);
}

int
ffl_add_vertex(FFL_Graph *graph)
{
	int idx = graph->nverts;
	if (++graph->nverts > graph->cverts) {
		graph->cverts *= 2;
		graph->verts = realloc(graph->verts, graph->cverts * sizeof *graph->verts);
	}
	return idx;
}

int
ffl_add_edge(FFL_Graph *graph)
{
	int idx = graph->nedges;
	if (++graph->nedges > graph->cedges) {
		graph->cedges *= 2;
		graph->edges = realloc(graph->edges, graph->cedges * sizeof *graph->edges);
	}
	return idx;
}

