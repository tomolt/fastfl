#include <stdlib.h>

#include "graph.h"
#include "realloc.h"

FFL_Graph *
ffl_make_graph(void)
{
	FFL_Graph *graph = calloc(1, sizeof (FFL_Graph));
	graph->nverts = 0;
	graph->cverts = 16;
	graph->verts_pos    = calloc(graph->cverts, sizeof *graph->verts_pos);
	graph->verts_force  = calloc(graph->cverts, sizeof *graph->verts_force);
	graph->verts_serial = calloc(graph->cverts, sizeof *graph->verts_serial);
	graph->verts_charge = calloc(graph->cverts, sizeof *graph->verts_charge);
	graph->nedges = 0;
	graph->cedges = 16;
	graph->edges  = calloc(graph->cedges, sizeof *graph->edges);
	return graph;
}

void
ffl_free_graph(FFL_Graph *graph)
{
	free(graph->verts_pos);
	free(graph->verts_force);
	free(graph->verts_serial);
	free(graph->verts_charge);
	free(graph->edges);
	for (int i = 0; i < graph->num_pools; i++) {
		free(graph->clump_pools[i]);
	}
	free(graph->clump_pools);
	free(graph);
}

void
ffl_grow_vertices(FFL_Graph *graph, int nverts)
{
	if (nverts > graph->cverts) {
		while (nverts > graph->cverts) graph->cverts *= 2;
		graph->verts_pos    = reallocarray(graph->verts_pos, graph->cverts, sizeof *graph->verts_pos);
		graph->verts_force  = reallocarray(graph->verts_force, graph->cverts, sizeof *graph->verts_force);
		graph->verts_serial = reallocarray(graph->verts_serial, graph->cverts, sizeof *graph->verts_serial);
		graph->verts_charge = reallocarray(graph->verts_charge, graph->cverts, sizeof *graph->verts_charge);
	}
	for (int v = graph->nverts; v < nverts; v++) {
		graph->verts_pos[v]    = (FFL_Vec2) { 0.0f, 0.0f };
		graph->verts_force[v]  = (FFL_Vec2) { 0.0f, 0.0f };
		graph->verts_serial[v] = v;
		graph->verts_charge[v] = 1.0f;
	}
	graph->nverts = nverts;
}

void
ffl_grow_edges(FFL_Graph *graph, int nedges)
{
	if (nedges > graph->cedges) {
		while (nedges > graph->cedges) graph->cedges *= 2;
		graph->edges = reallocarray(graph->edges, graph->cedges, sizeof *graph->edges);
	}
	for (int e = graph->nedges; e < nedges; e++) {
		graph->edges[e] = (FFL_Edge) { -1, -1, 10.0f };
	}
	graph->nedges = nedges;
}

