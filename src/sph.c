#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "graph.h"
#include "pcg32.h"
#include "realloc.h"

struct condition {
	bool  x_axis;
	float threshold;
};

#define SWAP(t,a,b) do { t _ = (a); (a) = (b); (b) = _; } while (0)
#define EVAL_CONDITION(c,p) (((c)->x_axis ? (p).x : (p).y) <= (c)->threshold)

static int
partition(FFL_Graph *graph, int low, int high, const struct condition *cond)
{
	for (;;) {
		while (low < high && EVAL_CONDITION(cond, graph->verts_pos[low])) low++;
		while (low < high && !EVAL_CONDITION(cond, graph->verts_pos[high-1])) high--;
		if (!(low < high)) break;

		SWAP(FFL_Vec2, graph->verts_pos[low], graph->verts_pos[high-1]);
		SWAP(FFL_Vec2, graph->verts_force[low], graph->verts_force[high-1]);
		SWAP(int, graph->verts_serial[low], graph->verts_serial[high-1]);

		low++, high--;
	}
	return low;
}

static bool
split_heuristic(const FFL_Graph *graph, int low, int high, struct condition *cond)
{
	static pcg32_random_t heuristic_rand;

	if (high - low <= 20) return false;

	float min_x =  INFINITY, min_y =  INFINITY;
	float max_x = -INFINITY, max_y = -INFINITY;

	for (int i = 0; i < 6 || (min_x >= max_x && min_y >= max_y); i++) {
		int v = low + (pcg32_random_r(&heuristic_rand) % (high - low));
		FFL_Vec2 pos = graph->verts_pos[v];
		if (pos.x < min_x) min_x = pos.x;
		if (pos.y < min_y) min_y = pos.y;
		if (pos.x > max_x) max_x = pos.x;
		if (pos.y > max_y) max_y = pos.y;
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
alloc_clump(FFL_Graph *graph)
{
	int p = graph->next_clump / CLUMPS_PER_POOL;
	int c = graph->next_clump % CLUMPS_PER_POOL;
	graph->next_clump++;
	if (p >= graph->num_pools) {
		graph->num_pools++;
		graph->clump_pools = reallocarray(graph->clump_pools, graph->num_pools, sizeof *graph->clump_pools);
		graph->clump_pools[p] = malloc(CLUMPS_PER_POOL * sizeof (FFL_Clump));
	}
	return &graph->clump_pools[p][c];
}

static FFL_Clump *
build_clump(FFL_Graph *graph, int low, int high)
{
	assert(low < high);
	FFL_Clump *clump = alloc_clump(graph);
	memset(clump, 0, sizeof *clump);
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
			clump->com_x += graph->verts_pos[i].x;
			clump->com_y += graph->verts_pos[i].y;
		}
		clump->com_x /= clump->mass;
		clump->com_y /= clump->mass;
		
		for (int i = low; i < high; i++) {
			float dx = graph->verts_pos[i].x - clump->com_x;
			float dy = graph->verts_pos[i].y - clump->com_y;
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
			graph->verts_force[v].x += force_x;
			graph->verts_force[v].y += force_y;
		}
	} else {
		declump_rec(graph, clump->nut, force_x, force_y);
		declump_rec(graph, clump->geb, force_x, force_y);
	}
}

void
ffl_linearize(FFL_Graph *graph)
{
	declump_rec(graph, graph->root_clump, 0.0f, 0.0f);
	graph->next_clump = 0;

	FFL_Vec2 *new_pos    = calloc(graph->cverts, sizeof *new_pos);
	FFL_Vec2 *new_force  = calloc(graph->cverts, sizeof *new_force);
	int      *new_serial = calloc(graph->cverts, sizeof *new_serial);

	for (int v = 0; v < graph->nverts; v++) {
		int s = graph->verts_serial[v];
		new_pos[s]    = graph->verts_pos[v];
		new_force[s]  = graph->verts_force[v];
		new_serial[s] = s;
	}

	free(graph->verts_pos);
	free(graph->verts_force);
	free(graph->verts_serial);

	graph->verts_pos    = new_pos;
	graph->verts_force  = new_force;
	graph->verts_serial = new_serial;
}

