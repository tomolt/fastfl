#include <stdlib.h>

#include "graph.h"
#include "realloc.h"

static int
edge_cmp(const void *p1, const void *p2)
{
	const FFL_Edge *e1 = p1, *e2 = p2;
	int d = e1->source - e2->source;
	return d ? d : e1->target - e2->target;
}

static int
compute_mapping(const FFL_Graph *graph, int *mapping)
{
	int nverts = 0;
	for (int v = 0; v < graph->nverts; v++) {
		mapping[v] = -1;
	}
	for (int e = 0; e < graph->nedges; e++) {
		const FFL_Edge *edge = &graph->edges[e];
		if (mapping[edge->source] < 0 && mapping[edge->target] < 0) {
			mapping[edge->source] = nverts;
			mapping[edge->target] = nverts;
			nverts++;
		}
	}
	for (int v = 0; v < graph->nverts; v++) {
		if (mapping[v] < 0) {
			mapping[v] = nverts++;
		}
	}
	return nverts;
}

static void
transfer_edges(const FFL_Graph *graph, const int *mapping, FFL_Graph *reduced)
{
	for (int e = 0; e < graph->nedges; e++) {
		const FFL_Edge *edge = &graph->edges[e];
		int source = mapping[edge->source];
		int target = mapping[edge->target];
		reduced->edges[e] = (FFL_Edge) { source, target, edge->d_length };
	}

	qsort(reduced->edges, graph->nedges, sizeof *reduced->edges, edge_cmp);

	FFL_Edge prev = { -1, -1, 0.0f };
	int w = 0;
	for (int r = 0; r < graph->nedges; r++) {
		const FFL_Edge *edge = &reduced->edges[r];
		if (edge->source != edge->target && !(edge->source == prev.source && edge->target == prev.target)) {
			reduced->edges[w++] = *edge;
		}
		prev = *edge;
	}
	
	reduced->nedges = w;
}

FFL_Graph *
ffl_reduce_graph(const FFL_Graph *graph)
{
	if (graph->nverts < 16) {
		return NULL;
	}

	int *mapping = calloc(graph->nverts, sizeof *graph);
	int nverts = compute_mapping(graph, mapping);
	if (100 * nverts > 80 * graph->nverts) {
		free(mapping);
		return NULL;
	}

	FFL_Graph *reduced = ffl_make_graph();
	ffl_grow_vertices(reduced, nverts);
	reduced->cedges = graph->cedges;
	reduced->edges = reallocarray(reduced->edges, reduced->cedges, sizeof *reduced->edges);

	transfer_edges(graph, mapping, reduced);

	free(mapping);
	return reduced;
}

