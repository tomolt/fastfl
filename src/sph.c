#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include "graph.h"

struct condition {
	bool  x_axis;
	float threshold;
};

static inline bool
eval_condition(const struct condition *cond, const FFL_Vertex *vert)
{
	return (cond->x_axis ? vert->x : vert->y) <= cond->threshold;
}

static int
partition(FFL_Graph *graph, int low, int high, const struct condition *cond)
{
	for (;;) {
		while (low < high && eval_condition(cond, &graph->verts[low])) low++;
		while (low < high && !eval_condition(cond, &graph->verts[high-1])) high--;
		if (!(low < high)) break;
		FFL_Vertex temp      = graph->verts[low];
		graph->verts[low]    = graph->verts[high-1];
		graph->verts[high-1] = temp;
		low++, high--;
	}
	return low;
}

static bool
split_heuristic(const FFL_Graph *graph, int low, int high, struct condition *cond)
{
	if (high - low <= 8) return false;

	float min_x =  INFINITY, min_y =  INFINITY;
	float max_x = -INFINITY, max_y = -INFINITY;

	for (int i = 0; i < 6 || (min_x >= max_x && min_y >= max_y); i++) {
		int v = low + (rand() % (high - low));
		const FFL_Vertex *vert = &graph->verts[v];
		if (vert->x < min_x) min_x = vert->x;
		if (vert->y < min_y) min_y = vert->y;
		if (vert->x > max_x) max_x = vert->x;
		if (vert->y > max_y) max_y = vert->y;
	}

	if (max_x - min_x >= max_y - min_y) {
		cond->x_axis = true;
		cond->threshold = 0.5f * (max_x + min_x);
	} else {
		cond->x_axis = false;
		cond->threshold = 0.5f * (max_y + min_y);
	}

	return true;
}

static FFL_Clump *
build_clump(FFL_Graph *graph, int low, int high)
{
	assert(low < high);
	FFL_Clump *clump = calloc(1, sizeof *clump);
	clump->mass = high - low;
	struct condition cond;
	if (split_heuristic(graph, low, high, &cond)) {
		int border = partition(graph, low, high, &cond);
		clump->nut = build_clump(graph, low, border);
		clump->geb = build_clump(graph, border, high);
		clump->sum_x = clump->nut->sum_x + clump->geb->sum_x;
		clump->sum_y = clump->nut->sum_y + clump->geb->sum_y;
	} else {
		clump->is_leaf = true;
		clump->low     = low;
		clump->high    = high;
		for (int i = low; i < high; i++) {
			clump->sum_x += graph->verts[i].x;
			clump->sum_y += graph->verts[i].y;
		}
	}
	return clump;
}

void
ffl_treeify(FFL_Graph *graph)
{
	graph->root_clump = build_clump(graph, 0, graph->nverts);
}

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

	if (c0->variance * c1->variance / dist_sq <= graph->threshold) {
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

