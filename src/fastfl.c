#include <fastfl.h>

#include <stdbool.h>
#include <math.h>

#include "realloc.h"

#define FFL_PI 3.14159265358979323846

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

static void
ffl_initial_layout(FFL_Graph *graph)
{
	float radius = (float) graph->nverts / 2.0f;

	for (int v = 0; v < graph->nverts; v++) {
		FFL_Vertex *vert = &graph->verts[v];
		float angle = (float) v / graph->nverts * 2.0f * FFL_PI;

		vert->x = radius * cosf(angle);
		vert->y = radius * sinf(angle);

		vert->forcex = 0.0f;
		vert->forcey = 0.0f;
	}
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
		float dlen = sqrtf(dx * dx + dy * dy);
		if (dlen == 0.0f) continue;
		dx /= dlen;
		dy /= dlen;

		float force = strength * logf(dlen / edge->dlength) * dlen * dlen;

		source->forcex += dx * force;
		source->forcey += dy * force;

		target->forcex -= dx * force;
		target->forcey -= dy * force;
	}
}

static void
ffl_repulsion_forces(FFL_Graph *graph, float strength)
{
	for (int t = 0; t < graph->nverts; t++) {
		FFL_Vertex *target = &graph->verts[t];
		for (int s = 0; s < graph->nverts; s++) {
			if (t == s) continue;

			FFL_Vertex *source = &graph->verts[s];

			float dx = target->x - source->x;
			float dy = target->y - source->y;
			float dlen = sqrtf(dx * dx + dy * dy);
			if (dlen == 0.0f) continue;
			dx /= dlen;
			dy /= dlen;

			float force = strength / dlen;

			target->forcex += dx * force;
			target->forcey += dy * force;
		}
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
		ffl_spring_forces(graph, 1.0f);
		ffl_repulsion_forces(graph, 1.0f);
		ffl_apply_forces(graph);
	}
}

