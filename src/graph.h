#include <stdbool.h>

#define CLUMPS_PER_POOL 64

typedef struct FFL_Vec2   FFL_Vec2;
typedef struct FFL_Edge   FFL_Edge;
typedef struct FFL_Clump  FFL_Clump;
typedef struct FFL_Graph  FFL_Graph;

struct FFL_Vec2 {
	float x;
	float y;
};

struct FFL_Edge {
	int   source;
	int   target;
	float d_length; /* desired length */
};

struct FFL_Clump {
	FFL_Vec2 force;
	FFL_Vec2 com;
	float variance;
	int   mass;
	bool  is_leaf;
	union {
		struct {
			FFL_Clump *nut;
			FFL_Clump *geb;
		};
		struct {
			int low;
			int high;
		};
	};
};

struct FFL_Graph {
	float spring_strength;
	float repulsion_strength;
	float repulsion_accuracy;
	int nverts;
	int cverts;
	int nedges;
	int cedges;

	FFL_Vec2 *verts_pos;
	FFL_Vec2 *verts_force;
	int      *verts_serial;
	
	FFL_Edge   *edges;
	FFL_Clump  *root_clump;

	FFL_Clump **clump_pools;
	int         num_pools;
	int         next_clump;
};

FFL_Graph *ffl_make_graph(void);
void ffl_free_graph(FFL_Graph *graph);

void ffl_grow_vertices(FFL_Graph *graph, int nverts);
void ffl_grow_edges(FFL_Graph *graph, int nedges);

void ffl_treeify(FFL_Graph *graph);
void ffl_linearize(FFL_Graph *graph);

