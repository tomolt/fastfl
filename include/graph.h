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
	float charge;
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
	float    *verts_charge;
	
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

int  ffl_compare_edges(const void *p1, const void *p2);
void ffl_graph_sort_edges(FFL_Graph *graph);

void ffl_treeify(FFL_Graph *graph);
void ffl_linearize(FFL_Graph *graph);

FFL_Graph *ffl_reduce_graph(const FFL_Graph *graph, int *mapping);
void ffl_interpolate_layout(const FFL_Graph *reduced, const int *mapping, FFL_Graph *graph);

