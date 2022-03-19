#include <fastfl.h>

#include <stdbool.h>

#include "realloc.h"

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
	float weight;
};

struct FFL_Graph {
	int nverts;
	int nedges;
	FFL_Vertex *verts;
	FFL_Edge   *edges;
};

static void
ffl_initial_layout(FFL_Graph *graph)
{
}

static void
ffl_spring_forces(FFL_Graph *graph, float strength)
{
	for (int e = 0; e < graph->nedges; e++) {
		FFL_Edge   *edge   = &graph->edges[e];
		FFL_Vertex *source = &graph->verts[edge->source];
		FFL_Vertex *target = &graph->verts[edge->target];

		float dx = target->x - source->x;
		float dy = target->y - source->y;

		source->forcex += dx * strength;
		source->forcey += dy * strength;

		target->forcex -= dx * strength;
		target->forcey -= dy * strength;
	}
}

static void
ffl_repulsion_forces(FFL_Graph *graph, float strength)
{
	for (int a = 0; a < graph->nverts; a++) {
		for (int b = 0; b < graph->nverts; b++) {

		}
	}
}

static void
ffl_cohesion_force(FFL_Graph *graph, float centerx, float centery, float strength)
{
	for (int v = 0; v < graph->nverts; v++) {
		FFL_Vertex *vert = &graph->verts[v];
		
		float dx = vert->x - centerx;
		float dy = vert->y - centery;

		vert->forcex -= dx;
		vert->forcey -= dy;
	}
}

static void
ffl_apply_forces(FFL_Graph *graph)
{
	for (int v = 0; v < graph->nverts; v++) {
		FFL_Vertex *vert = &graph->verts[v];
		vert->x += vert->forcex;
		vert->y += vert->forcey;
		vert->forcex = 0.0f;
		vert->forcey = 0.0f;
	}
}

void
ffl_compute_layout(FFL_Graph *graph)
{
	const int TOTAL_ROUNDS = 50;

	ffl_initial_layout(graph);

	int rounds = TOTAL_ROUNDS;
	while (rounds--) {
		ffl_spring_forces(graph, );
		ffl_repulsion_forces(graph, );
		ffl_cohesion_force(graph, );
		ffl_apply_forces(graph);
	}
}

