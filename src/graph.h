#include <stdbool.h>

#define CLUMPS_PER_POOL 64

typedef struct FFL_Vertex FFL_Vertex;
typedef struct FFL_Edge   FFL_Edge;
typedef struct FFL_Clump  FFL_Clump;
typedef struct FFL_Graph  FFL_Graph;

struct FFL_Vertex {
	float x;
	float y;
	float force_x;
	float force_y;
	int   serial;
};

struct FFL_Edge {
	int   source;
	int   target;
	float d_length; /* desired length */
};

struct FFL_Clump {
	float force_x;
	float force_y;
	float com_x;
	float com_y;
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
	FFL_Vertex *verts;
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

