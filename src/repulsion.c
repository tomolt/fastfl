#include <assert.h>

#include "graph.h"

extern void ffl_repulsion_1onN(FFL_Graph *graph, int t, int low, int high);
void ffl_repulsion_naive(FFL_Graph *graph);

static void
repulsion_rec(FFL_Graph *graph, FFL_Clump *c0, FFL_Clump *c1)
{
	/* TODO this case can be a separate function, much cleaner. */
	if (c0 == c1) {
		if (c0->is_leaf) {
			for (int v = c0->low; v < c0->high; v++) {
				ffl_repulsion_1onN(graph, v, c0->low, v);
			}
		} else {
			repulsion_rec(graph, c0->nut, c0->nut);
			repulsion_rec(graph, c0->nut, c0->geb);
			repulsion_rec(graph, c0->geb, c0->geb);
		}
		return;
	}

	float dx = c1->com_x - c0->com_x;
	float dy = c1->com_y - c0->com_y;
	float dist_sq = dx * dx + dy * dy;

	if (c0->variance * c1->variance < graph->repulsion_accuracy * dist_sq) {
		float force = graph->repulsion_strength / dist_sq;
		dx *= force;
		dy *= force;

		c0->force_x -= dx * c1->mass;
		c0->force_y -= dy * c1->mass;

		c1->force_x += dx * c0->mass;
		c1->force_y += dy * c0->mass;
		return;
	}

	if (c0->is_leaf && c1->is_leaf) {
		for (int v = c0->low; v < c0->high; v++) {
			ffl_repulsion_1onN(graph, v, c1->low, c1->high);
		}
		return;
	}

	/* TODO this conditional works for now, but it's not intuitive at all. */
	if (!c1->is_leaf && (c0->variance < c1->variance || c0->is_leaf)) {
		assert(!c1->is_leaf);
		repulsion_rec(graph, c0, c1->nut);
		repulsion_rec(graph, c0, c1->geb);
	} else {
		assert(!c0->is_leaf);
		repulsion_rec(graph, c0->nut, c1);
		repulsion_rec(graph, c0->geb, c1);
	}
}

void
ffl_repulsion_accelerated(FFL_Graph *graph)
{
	ffl_treeify(graph);
	repulsion_rec(graph, graph->root_clump, graph->root_clump);
	ffl_linearize(graph);
	//ffl_repulsion_naive(graph);
}

