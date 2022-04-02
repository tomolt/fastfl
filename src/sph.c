#include <stdlib.h>

#include "graph.h"

static int
partition(FFL_Graph *graph, int low, int high)
{
	for (;;) {
		while (low < high && COND(graph->verts[low])) low++;
		while (low < high && !COND(graph->verts[high-1])) high--;
		if (!(low < high)) break;
		FFL_Vertex temp      = graph->verts[low];
		graph->verts[low]    = graph->verts[high-1];
		graph->verts[high-1] = temp;
		low++, high--;
	}
	return low;
}

static bool
split_heuristic(FFL_Graph *graph, int low, int high)
{
	(void) graph;
	if (high - low <= 8) return true;
	return false;
}

static FFL_Clump *
build_clump(FFL_Graph *graph, int low, int high)
{
	FFL_Clump *clump = calloc(1, sizeof *clump);
	clump->mass = high - low;
	if (split_heuristic(graph, low, high)) {
		clump->is_leaf = true;
		clump->low     = low;
		clump->high    = high;
		for (int i = low; i < high; i++) {
			clump->sum_x += graph->verts[i].x;
			clump->sum_y += graph->verts[i].y;
		}
	} else {
		int border = partition(graph, low, high);
		clump->nut = build_clump(graph, low, border);
		clump->geb = build_clump(graph, border, high);
		clump->sum_x = clump->nut->sum_x + clump->geb->sum_x;
		clump->sum_y = clump->nut->sum_y + clump->geb->sum_y;
	}
	return clump;
}

void
ffl_treeify(FFL_Graph *graph)
{
	graph->root_clump = build_clump(graph, 0, graph->nverts);
}

