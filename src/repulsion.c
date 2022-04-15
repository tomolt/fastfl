#include <assert.h>

#include "graph.h"

extern void ffl_repulsion_2on2(FFL_Graph *graph, int i, int j);
extern void ffl_repulsion_1onN(FFL_Graph *graph, int t, int low, int high);
extern void ffl_repulsion_naive(FFL_Graph *graph);

static void
repulsion_self(FFL_Graph *graph, int low, int high)
{
	int i, j;
	for (i = low; i + 2 <= high; i += 2) {
		for (j = low; j + 2 <= i; j += 2) {
			ffl_repulsion_2on2(graph, i, j);
		}
		if (j < i) {
			ffl_repulsion_1onN(graph, i-1, i, i+1);
		}
	}
	if (i < high) {
		ffl_repulsion_1onN(graph, high-1, low, high-1);
	}
}

static void
repulsion_NonM(FFL_Graph *graph, int low0, int high0, int low1, int high1)
{
	int i, j = low1;
	for (i = low0; i + 2 <= high0; i += 2) {
		for (j = low1; j + 2 <= high1; j += 2) {
			ffl_repulsion_2on2(graph, i, j);
		}
	}
	if (i < high0) {
		ffl_repulsion_1onN(graph, high0-1, low1, high1);
	}
	if (j < high1) {
		ffl_repulsion_1onN(graph, high1-1, low0, i < high0 ? high0-1 : high0);
	}
}

static void
repulsion_rec(FFL_Graph *graph, FFL_Clump *c0, FFL_Clump *c1)
{
	/* TODO this case can be a separate function, much cleaner. */
	if (c0 == c1) {
		if (c0->is_leaf) {
			repulsion_self(graph, c0->low, c0->high);
		} else {
			repulsion_rec(graph, c0->child0, c0->child0);
			repulsion_rec(graph, c0->child0, c0->child1);
			repulsion_rec(graph, c0->child1, c0->child1);
		}
		return;
	}

	float dx = c1->com.x - c0->com.x;
	float dy = c1->com.y - c0->com.y;
	float dist_sq = dx * dx + dy * dy;

	if (c0->variance * c1->variance < graph->repulsion_accuracy * dist_sq) {
		float force = graph->repulsion_strength / dist_sq;
		dx *= force;
		dy *= force;

		c0->force.x -= dx * c1->charge;
		c0->force.y -= dy * c1->charge;

		c1->force.x += dx * c0->charge;
		c1->force.y += dy * c0->charge;
		return;
	}

	if (c0->is_leaf && c1->is_leaf) {
		repulsion_NonM(graph, c0->low, c0->high, c1->low, c1->high);
		return;
	}

	/* TODO this conditional works for now, but it's not intuitive at all. */
	if (!c1->is_leaf && (c0->variance < c1->variance || c0->is_leaf)) {
		assert(!c1->is_leaf);
		repulsion_rec(graph, c0, c1->child0);
		repulsion_rec(graph, c0, c1->child1);
	} else {
		assert(!c0->is_leaf);
		repulsion_rec(graph, c0->child0, c1);
		repulsion_rec(graph, c0->child1, c1);
	}
}

void
ffl_repulsion_accelerated(FFL_Graph *graph)
{
	ffl_form_clumps(graph);
	repulsion_rec(graph, graph->root_clump, graph->root_clump);
	ffl_homogenize(graph);
}

