typedef struct FFL_Vertex FFL_Vertex;
typedef struct FFL_Edge   FFL_Edge;
typedef struct FFL_Graph  FFL_Graph;

struct FFL_Vertex {
	float x;
	float y;
	float forcex;
	float forcey;
	int   next;
};

struct FFL_Edge {
	int   source;
	int   target;
	float dlength; /* desired length */
};

struct FFL_Graph {
	int nverts;
	int cverts;
	int nedges;
	int cedges;
	FFL_Vertex *verts;
	FFL_Edge   *edges;
};

FFL_Graph *ffl_make_graph(void);
void ffl_free_graph(FFL_Graph *graph);
void ffl_grow_vertices(FFL_Graph *graph, int nverts);
void ffl_grow_edges(FFL_Graph *graph, int nedges);

