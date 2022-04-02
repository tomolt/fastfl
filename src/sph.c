#include "sph.h"

int
partition(int low, int high)
{
	for (;;) {
		while (low < high && COND(verts[low])) low++;
		while (low < high && !COND(verts[high-1])) high--;
		if (!(low < high)) break;
		FFL_Vertex temp = verts[low];
		verts[low]      = verts[high-1];
		verts[high-1]   = temp;
		low++, high--;
	}
	return low;
}

static bool
split_heuristic(int low, int high)
{
	if (high - low <= 8) return true;
	return false;
}

static FFL_SpaceNode *
build_node(int low, int high)
{
	SPH_SpaceNode *node = calloc(1, sizeof *node);
	node->mass = high - low;
	if (split_heuristic(low, high)) {
		node->is_leaf = true;
		node->start   = low;
		node->end     = high;
		for (int i = low; i < high; i++) {
			node->sum_x += verts[i].x;
			node->sum_y += verts[i].y;
		}
	} else {
		int border = partition(low, high);
		node->nut = build_node(low, border);
		node->geb = build_node(border, high);
		node->sum_x = node->nut->sum_x + node->geb->sum_x;
		node->sum_y = node->nut->sum_y + node->geb->sum_y;
	}
	return node;
}

void
ffl_treeify(FFL_SPH *sph)
{
	(void) sph;
	sph->root = build_node(0, nverts);
}

void
ffl_linearize(FFL_SPH *sph)
{
	(void) sph;
}

