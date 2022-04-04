#include <fastfl.h>

#include <stdbool.h>
#include <math.h>
#include <stdio.h>

#include "graph.h"

#define FFL_PI 3.14159265358979323846

static void
ffl_initial_layout(FFL_Graph *graph)
{
	float radius = 10.0f * (float) graph->nverts / 2.0f;

	for (int v = 0; v < graph->nverts; v++) {
		FFL_Vertex *vert = &graph->verts[v];
		float angle = (float) v / graph->nverts * 2.0f * FFL_PI;

		vert->x = radius * cosf(angle);
		vert->y = radius * sinf(angle);

		vert->force_x = 0.0f;
		vert->force_y = 0.0f;
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

		float force = dlen - edge->d_length;
		force *= 0.5f * strength;

		source->force_x += dx * force;
		source->force_y += dy * force;

		target->force_x -= dx * force;
		target->force_y -= dy * force;
	}
}

static void
ffl_repulsion_forces(FFL_Graph *graph, float strength)
{
	for (int t = 0; t < graph->nverts; t++) {
		FFL_Vertex *target = &graph->verts[t];
		for (int s = 0; s < t; s++) {
			FFL_Vertex *source = &graph->verts[s];

			float dx = target->x - source->x;
			float dy = target->y - source->y;
			float dist_sq = dx * dx + dy * dy;
			if (dist_sq == 0.0f) continue;

			float force = strength / dist_sq;
			dx *= force;
			dy *= force;

			source->force_x -= dx;
			source->force_y -= dy;

			target->force_x += dx;
			target->force_y += dy;
		}
	}
}

static void
ffl_apply_forces(FFL_Graph *graph)
{
	for (int v = 0; v < graph->nverts; v++) {
		FFL_Vertex *vert = &graph->verts[v];

		vert->x += vert->force_x;
		vert->y += vert->force_y;
		
		vert->force_x = 0.0f;
		vert->force_y = 0.0f;
	}
}

void
ffl_compute_layout(FFL_Graph *graph, const FFL_Settings *settings)
{
	const int TOTAL_ROUNDS = 200;

	ffl_initial_layout(graph);

	int rounds = TOTAL_ROUNDS;
	while (rounds--) {
		float temperature = 0.1f + 0.9f * (float) rounds / TOTAL_ROUNDS;
		ffl_spring_forces(graph, temperature * settings->spring_strength);
		ffl_repulsion_forces(graph, temperature * settings->repulsion_strength);
		ffl_apply_forces(graph);
	}
}

