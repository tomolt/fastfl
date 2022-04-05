#include <assert.h>

#include "graph.h"

extern void ffl_repulsion_2on2(FFL_Graph *graph, int i, int j);
extern void ffl_repulsion_1onN(FFL_Graph *graph, int t, int low, int high);
extern void ffl_repulsion_naive(FFL_Graph *graph);

static void
repulsion_rec(FFL_Graph *graph, FFL_Clump *c0, FFL_Clump *c1)
{
	/* TODO this case can be a separate function, much cleaner. */
	if (c0 == c1) {
		if (c0->is_leaf) {
			int i, j;
			for (i = c0->low; i + 2 <= c0->high; i += 2) {
				for (j = c0->low; j + 2 <= i; j += 2) {
					ffl_repulsion_2on2(graph, i, j);
				}
				if (j < i) {
					ffl_repulsion_1onN(graph, i-1, i, i+1);
				}
			}
			if (i < c0->high) {
				ffl_repulsion_1onN(graph, c0->high-1, c0->low, c0->high-1);
			}
		} else {
			repulsion_rec(graph, c0->nut, c0->nut);
			repulsion_rec(graph, c0->nut, c0->geb);
			repulsion_rec(graph, c0->geb, c0->geb);
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

		c0->force.x -= dx * c1->mass;
		c0->force.y -= dy * c1->mass;

		c1->force.x += dx * c0->mass;
		c1->force.y += dy * c0->mass;
		return;
	}

	if (c0->is_leaf && c1->is_leaf) {
		int i, j = c1->low;
		for (i = c0->low; i + 2 <= c0->high; i += 2) {
			for (j = c1->low; j + 2 <= c1->high; j += 2) {
				ffl_repulsion_2on2(graph, i, j);
			}
		}
		if (i < c0->high) {
			ffl_repulsion_1onN(graph, c0->high-1, c1->low, c1->high);
		}
		if (j < c1->high) {
			ffl_repulsion_1onN(graph, c1->high-1, c0->low, i < c0->high ? c0->high-1 : c0->high);
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

