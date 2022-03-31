#include <stdbool.h>

typedef struct FFL_Graph FFL_Graph;
typedef struct FFL_SpaceNode FFL_SpaceNode;
typedef struct FFL_SPH FFL_SPH;

struct FFL_SpaceNode {
	float min_x;
	float min_y;
	float max_x;
	float max_y;

	float force_x;
	float force_y;
	float com_x;
	float com_y;
	int   mass;
	bool  is_leaf;

	union {
		FFL_SpaceNode *children[2];
		int            head;
	};
};

struct FFL_SPH {
	FFL_Graph     *graph;
	FFL_SpaceNode *root;
};

void ffl_treeify(FFL_SPH *sph);
void ffl_linearize(FFL_SPH *sph);

