#include <stdlib.h>
#include <math.h>

#include "graph.h"
#include "realloc.h"

FFL_Rect
ffl_bounding_box(FFL_Vec2 *vecs, int low, int high)
{
	float min_x = INFINITY, min_y = INFINITY, max_x = -INFINITY, max_y = -INFINITY;
	for (int v = low; v < high; v++) {
		FFL_Vec2 vec = vecs[v];
		if (vec.x < min_x) min_x = vec.x;
		if (vec.y < min_y) min_y = vec.y;
		if (vec.x > max_x) max_x = vec.x;
		if (vec.y > max_y) max_y = vec.y;
	}
	return (FFL_Rect) { { min_x, min_y }, { max_x, max_y } };
}

FFL_Graph *
ffl_make_graph(void)
{
	FFL_Graph *graph = calloc(1, sizeof (FFL_Graph));
	graph->nverts = 0;
	graph->cverts = 16;

#	define X(type,field) graph->field = calloc(graph->cverts, sizeof (type));
	EXPAND_FOR_EACH_VERTEX_FIELD(X);
#	undef X

	graph->nedges = 0;
	graph->cedges = 16;
	graph->edges  = calloc(graph->cedges, sizeof *graph->edges);
	return graph;
}

void
ffl_free_graph(FFL_Graph *graph)
{
#	define X(type,field) free(graph->field);
	EXPAND_FOR_EACH_VERTEX_FIELD(X);
#	undef X
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
#		define X(type,field) graph->field = reallocarray(graph->field, graph->cverts, sizeof (type));
		EXPAND_FOR_EACH_VERTEX_FIELD(X);
#		undef X
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

int
ffl_compare_edges(const void *p1, const void *p2)
{
	const FFL_Edge *e1 = p1, *e2 = p2;
	int d = e1->source - e2->source;
	return d ? d : e1->target - e2->target;
}

void
ffl_graph_sort_edges(FFL_Graph *graph)
{
	qsort(graph->edges, graph->nedges,
		sizeof *graph->edges, ffl_compare_edges);
}

FFL_Clump *
ffl_alloc_clump(FFL_Graph *graph)
{
	int p = graph->next_clump / CLUMPS_PER_POOL;
	int c = graph->next_clump % CLUMPS_PER_POOL;
	graph->next_clump++;
	if (p >= graph->num_pools) {
		graph->num_pools++;
		graph->clump_pools = reallocarray(graph->clump_pools, graph->num_pools, sizeof *graph->clump_pools);
		graph->clump_pools[p] = malloc(CLUMPS_PER_POOL * sizeof (FFL_Clump));
	}
	return &graph->clump_pools[p][c];
}

