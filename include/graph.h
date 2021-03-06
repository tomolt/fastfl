#include <stdbool.h>

#define EXPAND_FOR_EACH_VERTEX_FIELD(m)		\
		m(FFL_Vec2, verts_pos)		\
		m(FFL_Vec2, verts_force)	\
		m(int,      verts_serial)	\
		m(float,    verts_charge)

#define CLUMPS_PER_POOL 64

typedef struct FFL_Vec2   FFL_Vec2;
typedef struct FFL_Rect   FFL_Rect;
typedef struct FFL_Edge   FFL_Edge;
typedef struct FFL_Clump  FFL_Clump;
typedef struct FFL_Graph  FFL_Graph;

struct FFL_Vec2 {
	float x;
	float y;
};

struct FFL_Rect {
	FFL_Vec2 min;
	FFL_Vec2 max;
};

struct FFL_Edge {
	int   source;
	int   target;
	float d_length; /* desired length */
};

struct FFL_Clump {
	FFL_Rect rect;
	FFL_Vec2 force;
	FFL_Vec2 com;
	float charge;
	bool  is_leaf;
	union {
		struct {
			FFL_Clump *child0;
			FFL_Clump *child1;
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

void ffl_bounding_box(FFL_Vec2 *vecs, int low, int high, FFL_Rect *rect);

FFL_Graph *ffl_make_graph(void);
void ffl_free_graph(FFL_Graph *graph);

void ffl_grow_vertices(FFL_Graph *graph, int nverts);
void ffl_grow_edges(FFL_Graph *graph, int nedges);

int  ffl_compare_edges(const void *p1, const void *p2);
void ffl_graph_sort_edges(FFL_Graph *graph);

FFL_Clump *ffl_alloc_clump(FFL_Graph *graph);

void ffl_form_clumps(FFL_Graph *graph);
void ffl_homogenize(FFL_Graph *graph);

FFL_Graph *ffl_reduce_graph(const FFL_Graph *graph, int *mapping);
void ffl_interpolate_layout(const FFL_Graph *reduced, const int *mapping, FFL_Graph *graph);

