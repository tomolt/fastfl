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

void
ffl_grow_vertices(FFL_Graph *graph, int nverts)
{
	if (nverts > graph->cverts) {
		while (nverts > graph->cverts) graph->cverts *= 2;
		graph->verts = realloc(graph->verts, graph->cverts * sizeof *graph->verts);
	}
	for (int v = graph->nverts; v < nverts; v++) {
		graph->verts[v] = (FFL_Vertex) { 0.0f, 0.0f, 0.0f, 0.0f };
	}
	graph->nverts = nverts;
}

void
ffl_grow_edges(FFL_Graph *graph, int nedges)
{
	if (nedges > graph->cedges) {
		while (nedges > graph->cedges) graph->cedges *= 2;
		graph->edges = realloc(graph->edges, graph->cedges * sizeof *graph->edges);
	}
	for (int e = graph->nedges; e < nedges; e++) {
		graph->edges[e] = (FFL_Edge) { -1, -1, 100.0f };
	}
	graph->nedges = nedges;
}

