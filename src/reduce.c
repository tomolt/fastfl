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
	int r, w, n;

	for (r = 0, w = 0; r < graph->nedges; r++) {
		const FFL_Edge *edge = &graph->edges[r];
		int source = mapping[edge->source];
		int target = mapping[edge->target];
		if (source != target) {
			reduced->edges[w++] = (FFL_Edge) { source, target, edge->d_length };
		}
	}
	reduced->nedges = w;

	qsort(reduced->edges, reduced->nedges, sizeof *reduced->edges, edge_cmp);

	r = 0, w = 0;
	while (r < reduced->nedges) {
		const FFL_Edge *base = &reduced->edges[r];
		float d_length = base->d_length;
		n = 1;
		while (r + n < reduced->nedges &&
			!edge_cmp(&reduced->edges[r + n], base)) {
			d_length += reduced->edges[r + n].d_length;
			n++;
		}
		reduced->edges[w++] = (FFL_Edge) {
			base->source,
			base->target,
			d_length / n
		};
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

