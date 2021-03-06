#include <stdlib.h>

#include "graph.h"
#include "realloc.h"
#include "pcg32.h"

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

	ffl_graph_sort_edges(reduced);

	for (r = 0, w = 0; r < reduced->nedges; r++) {
		const FFL_Edge *base = &reduced->edges[r];
		float d_length = base->d_length;
		n = 1;
		while (r + n < reduced->nedges &&
			!ffl_compare_edges(&reduced->edges[r + n], base)) {
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
ffl_reduce_graph(const FFL_Graph *graph, int *mapping)
{
	if (graph->nverts <= 3) {
		return NULL;
	}

	int nverts = compute_mapping(graph, mapping);
	if (100 * nverts > 80 * graph->nverts) {
		return NULL;
	}

	FFL_Graph *reduced = ffl_make_graph();
	ffl_grow_vertices(reduced, nverts);

	for (int v = 0; v < reduced->nverts; v++) {
		reduced->verts_charge[v] = 0.0f;
	}
	for (int v = 0; v < graph->nverts; v++) {
		reduced->verts_charge[mapping[v]] += graph->verts_charge[v];
	}

	reduced->cedges = graph->cedges;
	reduced->edges = reallocarray(reduced->edges, reduced->cedges, sizeof *reduced->edges);

	transfer_edges(graph, mapping, reduced);

	return reduced;
}

void
ffl_interpolate_layout(const FFL_Graph *reduced, const int *mapping, FFL_Graph *graph)
{
	static pcg32_random_t rng;
	for (int v = 0; v < graph->nverts; v++) {
		int r = mapping[v];
		float off_x = 2.0f * ((float) pcg32_random_r(&rng) / UINT32_MAX) - 1.0f;
		float off_y = 2.0f * ((float) pcg32_random_r(&rng) / UINT32_MAX) - 1.0f;
		graph->verts_pos[v].x = reduced->verts_pos[r].x + off_x;
		graph->verts_pos[v].y = reduced->verts_pos[r].y + off_y;
	}
}

