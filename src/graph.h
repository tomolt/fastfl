typedef struct FFL_Vertex FFL_Vertex;
typedef struct FFL_Edge   FFL_Edge;

struct FFL_Vertex {
	float x;
	float y;
	float forcex;
	float forcey;
};

struct FFL_Edge {
	int   source;
	int   target;
	float dlength; /* desired length */
};

struct FFL_Graph {
	int nverts;
	int nedges;
	FFL_Vertex *verts;
	FFL_Edge   *edges;
};

