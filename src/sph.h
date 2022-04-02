#include <stdbool.h>

typedef struct FFL_Graph FFL_Graph;
typedef struct FFL_SpaceNode FFL_SpaceNode;
typedef struct FFL_SPH FFL_SPH;

struct FFL_SpaceNode {
	float force_x;
	float force_y;
	float sum_x;
	float sum_y;
	int   mass;
	bool  is_leaf;
	union {
		struct {
			FFL_SpaceNode *nut;
			FFL_SpaceNode *geb;
		};
		struct {
			int low;
			int high;
		};
	};
};

struct FFL_SPH {
	FFL_Graph     *graph;
	FFL_SpaceNode *root;
};

void ffl_treeify(FFL_SPH *sph);
void ffl_linearize(FFL_SPH *sph);

