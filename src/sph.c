#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include "graph.h"
#include "pcg32.h"

struct condition {
	bool  x_axis;
	float threshold;
};

#define SWAP(t,a,b) do { t _ = (a); (a) = (b); (b) = _; } while (0)
#define EVAL_CONDITION(c,p) (((c)->x_axis ? (p).x : (p).y) <= (c)->threshold)
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))

static int
partition(FFL_Graph *graph, int low, int high, const struct condition *cond)
{
	for (;;) {
		while (low < high && EVAL_CONDITION(cond, graph->verts_pos[low])) low++;
		while (low < high && !EVAL_CONDITION(cond, graph->verts_pos[high-1])) high--;
		if (!(low < high)) break;

#		define X(type,field) SWAP(type, graph->field[low], graph->field[high-1]);
		EXPAND_FOR_EACH_VERTEX_FIELD(X);
#		undef X

		low++, high--;
	}
	return low;
}

static bool
split_heuristic(const FFL_Graph *graph, int low, int high, struct condition *cond)
{
	static pcg32_random_t rng;

	if (high - low <= 20) return false;

	float min_x =  INFINITY, min_y =  INFINITY;
	float max_x = -INFINITY, max_y = -INFINITY;

	for (int i = 0; i < 6 || (min_x >= max_x && min_y >= max_y); i++) {
		int v = low + (pcg32_random_r(&rng) % (high - low));
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
form_clumps_rec(FFL_Graph *graph, int low, int high)
{
	assert(low < high);
	FFL_Clump *clump = ffl_alloc_clump(graph);
	clump->force = (FFL_Vec2) { 0.0f, 0.0f };
	struct condition cond;
	if (split_heuristic(graph, low, high, &cond)) {
		clump->is_leaf = false;

		int border = partition(graph, low, high, &cond);
		clump->child0 = form_clumps_rec(graph, low, border);
		clump->child1 = form_clumps_rec(graph, border, high);

		clump->charge = clump->child0->charge + clump->child1->charge;

		clump->com.x  = clump->child0->com.x * clump->child0->charge;
		clump->com.x += clump->child1->com.x * clump->child1->charge;
		clump->com.x /= clump->charge;
		
		clump->com.y  = clump->child0->com.y * clump->child0->charge;
		clump->com.y += clump->child1->com.y * clump->child1->charge;
		clump->com.y /= clump->charge;

		clump->rect.min.x = MIN(clump->child0->rect.min.x, clump->child1->rect.min.x);
		clump->rect.min.y = MIN(clump->child0->rect.min.y, clump->child1->rect.min.y);
		clump->rect.max.x = MAX(clump->child0->rect.max.x, clump->child1->rect.max.x);
		clump->rect.max.y = MAX(clump->child0->rect.max.y, clump->child1->rect.max.y);
	} else {
		clump->is_leaf = true;
		clump->low     = low;
		clump->high    = high;

		clump->charge = 0.0f;
		clump->com = (FFL_Vec2) { 0.0f, 0.0f };
		for (int i = low; i < high; i++) {
			float charge = graph->verts_charge[i];
			clump->charge += charge;
			clump->com.x += graph->verts_pos[i].x * charge;
			clump->com.y += graph->verts_pos[i].y * charge;
		}
		clump->com.x /= clump->charge;
		clump->com.y /= clump->charge;

		ffl_bounding_box(graph->verts_pos, low, high, &clump->rect);
	}
	return clump;
}

void
ffl_form_clumps(FFL_Graph *graph)
{
	graph->root_clump = form_clumps_rec(graph, 0, graph->nverts);
}

static void
gather_forces_rec(FFL_Graph *graph, FFL_Clump *clump, float force_x, float force_y)
{
	force_x += clump->force.x;
	force_y += clump->force.y;
	if (clump->is_leaf) {
		for (int v = clump->low; v < clump->high; v++) {
			graph->verts_force[v].x += force_x;
			graph->verts_force[v].y += force_y;
		}
	} else {
		gather_forces_rec(graph, clump->child0, force_x, force_y);
		gather_forces_rec(graph, clump->child1, force_x, force_y);
	}
}

void
ffl_homogenize(FFL_Graph *graph)
{
	gather_forces_rec(graph, graph->root_clump, 0.0f, 0.0f);
	graph->next_clump = 0;

#	define X(type,field) type *new_##field = calloc(graph->cverts, sizeof (type));
	EXPAND_FOR_EACH_VERTEX_FIELD(X);
#	undef X

	for (int v = 0; v < graph->nverts; v++) {
		int s = graph->verts_serial[v];
#		define X(type,field) new_##field[s] = graph->field[v];
		EXPAND_FOR_EACH_VERTEX_FIELD(X);
#		undef X
	}

#	define X(type,field) free(graph->field); graph->field = new_##field;
	EXPAND_FOR_EACH_VERTEX_FIELD(X);
#	undef X
}

