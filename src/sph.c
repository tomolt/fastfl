#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include "graph.h"
#include "pcg32.h"

struct condition {
	bool  x_axis;
	float threshold;
};

#define EVAL_CONDITION(c,v) (((c)->x_axis ? (v)->x : (v)->y) <= (c)->threshold)

static int
partition(FFL_Graph *graph, int low, int high, const struct condition *cond)
{
	for (;;) {
		while (low < high && EVAL_CONDITION(cond, &graph->verts[low])) low++;
		while (low < high && !EVAL_CONDITION(cond, &graph->verts[high-1])) high--;
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
	static pcg32_random_t heuristic_rand;

	if (high - low <= 8) return false;

	float min_x =  INFINITY, min_y =  INFINITY;
	float max_x = -INFINITY, max_y = -INFINITY;

	for (int i = 0; i < 6 || (min_x >= max_x && min_y >= max_y); i++) {
		int v = low + (pcg32_random_r(&heuristic_rand) % (high - low));
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

		clump->com_x  = clump->nut->com_x * clump->nut->mass;
		clump->com_x += clump->geb->com_x * clump->geb->mass;
		clump->com_x /= clump->mass;
		
		clump->com_y  = clump->nut->com_y * clump->nut->mass;
		clump->com_y += clump->geb->com_y * clump->geb->mass;
		clump->com_y /= clump->mass;
		
		float dx0 = clump->nut->com_x - clump->com_x;
		float dy0 = clump->nut->com_y - clump->com_y;
		float dx1 = clump->geb->com_x - clump->com_x;
		float dy1 = clump->geb->com_y - clump->com_y;
		clump->variance  = sqrtf(dx0 * dx0 + dy0 * dy0) * clump->nut->mass;
		clump->variance += sqrtf(dx1 * dx1 + dy1 * dy1) * clump->geb->mass;
		clump->variance /= clump->mass;
	} else {
		clump->is_leaf = true;
		clump->low     = low;
		clump->high    = high;

		for (int i = low; i < high; i++) {
			clump->com_x += graph->verts[i].x;
			clump->com_y += graph->verts[i].y;
		}
		clump->com_x /= clump->mass;
		clump->com_y /= clump->mass;
		
		for (int i = low; i < high; i++) {
			float dx = graph->verts[i].x - clump->com_x;
			float dy = graph->verts[i].y - clump->com_y;
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

