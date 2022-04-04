#include "graph.h"

static void
repulse_leaf_leaf(FFL_Graph *graph, FFL_Clump *c0, FFL_Clump *c1)
{
	for (int t = 0; t < graph->nverts; t++) {
		FFL_Vertex *target = &graph->verts[t];
		for (int s = 0; s < graph->nverts; s++) {
			if (t == s) continue;

			FFL_Vertex *source = &graph->verts[s];

			float dx = target->x - source->x;
			float dy = target->y - source->y;
			float dist_sq = dx * dx + dy * dy;
			if (dist_sq == 0.0f) continue;

			float force = strength / dist_sq;
			target->forcex += dx * force;
			target->forcey += dy * force;
		}
	}
}

static void
repulse_inner_inner()
{
	float dx = c1->sum_x / c1->mass - c0->sum_x / c0->mass;
	float dy = c1->sum_y / c1->mass - c0->sum_y / c0->mass;

	float dist_sq = dx * dx + dy * dy;
	if (dist_sq == 0.0f) continue;

	float force = strength / dist_sq;
	dx *= force;
	dy *= force;

	c0->force_x -= dx;
	c0->force_y -= dy;
	c1->force_x += dx;
	c1->force_y += dy;
}

static void
repulse_clumps(FFL_Graph *graph, FFL_Clump *c0, FFL_Clump *c1)
{
	/* TODO this case can be a separate function, much cleaner. */
	if (c0 == c1) {
		if (c0->is_leaf) {
			repulse_leaf_leaf(graph, c0, c0);
		} else {
			repulse_clumps(graph, c0->nut, c0->nut);
			repulse_clumps(graph, c0->nut, c0->geb);
			repulse_clumps(graph, c0->geb, c0->geb);
		}
		return;
	}

	float dx = c1->sum_x / c1->mass - c0->sum_x / c0->mass;
	float dy = c1->sum_y / c1->mass - c0->sum_y / c0->mass;
	float dist_sq = dx * dx + dy * dy;

	if (dist_sq == 0.0f || c0->variance * c1->variance / dist_sq <= graph->threshold) {
		repulse_inner_inner(graph, c0, c1);
		return;
	}

	if (c0->is_leaf && c1->is_leaf) {
		repulse_leaf_leaf(graph, c0, c1);
		return;
	}

	if (c0->variance < c1->variance || c0->is_leaf) {
		repulse_clumps(graph, c0, c1->nut);
		repulse_clumps(graph, c0, c1->geb);
	} else {
		repulse_clumps(graph, c0->nut, c1);
		repulse_clumps(graph, c0->geb, c1);
	}
}

void
ffl_repulsion(FFL_Graph *graph)
{
	repulse_clumps(graph, graph->root_clump, graph->root_clump);
}

