

typedef struct FFL_SpaceNode FFL_SpaceNode;
typedef struct FFL_SPH;

struct FFL_SpaceNode {
	float min_x;
	float min_y;
	float max_x;
	float max_y;

	float force_x;
	float force_y;
	float x;
	float y;
	int   mass;

	FFL_SpaceNode *children[2];
};

struct FFL_SPH {
	FFL_SpaceNode *root;
};

void
ffl_treeify()
{
}

void
ffl_linearize()
{
}

