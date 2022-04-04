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

		//clump->variance = ;
	} else {
		clump->is_leaf = true;
		clump->low     = low;
		clump->high    = high;
		for (int i = low; i < high; i++) {
			clump->sum_x += graph->verts[i].x;
			clump->sum_y += graph->verts[i].y;
		}
		for (int i = low; i < high; i++) {
			float dx = graph->verts[i].x - clump->sum_x / clump->mass;
			float dy = graph->verts[i].y - clump->sum_y / clump->mass;
			clump->variance += sqrtf(dx * dx + dy * dy);
		}
		clump->variance /= clump->mass;
	}
	return clump;
}

void
ffl_treeify(FFL_Graph *graph)
{
	graph->root_clump = build_clump(graph, 0, graph->nverts);
}

static void
declump_rec(FFL_Graph *graph, FFL_Clump *clump, float force_x, float force_y)
{
	force_x += clump->force_x;
	force_y += clump->force_y;
	if (clump->is_leaf) {
		for (int v = clump->low; v < clump->high; v++) {
			graph->verts[v].force_x += force_x;
			graph->verts[v].force_y += force_y;
		}
	} else {
		declump_rec(graph, clump->nut, force_x, force_y);
		declump_rec(graph, clump->geb, force_x, force_y);
	}
	free(clump);
}

void
ffl_linearize(FFL_Graph *graph)
{
	declump_rec(graph, graph->root_clump, 0.0f, 0.0f);

	FFL_Vertex *new_verts = calloc(graph->cverts, sizeof *new_verts);

	for (int v = 0; v < graph->nverts; v++) {
		const FFL_Vertex *vert = &graph->verts[v];
		new_verts[vert->serial] = *vert;
	}

	free(graph->verts);
	graph->verts = new_verts;
}

