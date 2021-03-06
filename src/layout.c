#include <fastfl.h>

#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "graph.h"

extern void ffl_repulsion_2on2(FFL_Graph *graph, int i, int j);
extern void ffl_repulsion_accelerated(FFL_Graph *graph);

static void
ffl_initial_layout(FFL_Graph *graph)
{
	float radius = 10.0f * (float) graph->nverts / 2.0f;

	for (int v = 0; v < graph->nverts; v++) {
		float angle = (float) v / graph->nverts * 2.0f * FFL_PI;
		FFL_Vec2 pos;
		pos.x = radius * cosf(angle);
		pos.y = radius * sinf(angle);
		graph->verts_pos[v] = pos;
	}

	memset(graph->verts_force, 0, graph->nverts * sizeof *graph->verts_force);
}

static void
ffl_spring_forces(FFL_Graph *graph)
{
	for (int e = 0; e < graph->nedges; e++) {
		FFL_Edge *edge = &graph->edges[e];
		FFL_Vec2 source = graph->verts_pos[edge->source];
		FFL_Vec2 target = graph->verts_pos[edge->target];

		float dx = target.x - source.x;
		float dy = target.y - source.y;
		float dist_sq = dx * dx + dy * dy;
		if (dist_sq == 0.0f) continue;
		
		float inv_dist = dist_sq;
		__asm__ inline ("rsqrtss %0, %0" : "+v"(inv_dist));
		float dist = inv_dist;
		__asm__ inline ("rcpss %0, %0" : "+v"(dist));

		float factor = 0.5f * graph->spring_strength;
		factor *= logf(dist / edge->d_length);
		factor *= edge->d_length;
		factor *= inv_dist;

		dx *= factor;
		dy *= factor;

		graph->verts_force[edge->source].x += dx;
		graph->verts_force[edge->source].y += dy;

		graph->verts_force[edge->target].x -= dx;
		graph->verts_force[edge->target].y -= dy;
	}
}

static void
ffl_apply_forces(FFL_Graph *graph, float temperature)
{
	for (int v = 0; v < graph->nverts; v++) {
		FFL_Vec2 force = graph->verts_force[v];

		float magnitude = sqrtf(force.x * force.x + force.y * force.y);
		if (magnitude > temperature) {
			force.x *= temperature / magnitude;
			force.y *= temperature / magnitude;
		}

		graph->verts_pos[v].x += force.x;
		graph->verts_pos[v].y += force.y;
	}

	memset(graph->verts_force, 0, graph->nverts * sizeof *graph->verts_force);
}

static void
layout_graph(FFL_Graph *graph)
{
	printf("Laying out graph of size %d.\n", graph->nverts);

	const int TOTAL_ROUNDS = 100;
	const float eccentricity = 2.0f;

	int rounds = TOTAL_ROUNDS;
	while (rounds) {
		float temperature = (float) rounds / TOTAL_ROUNDS;
		temperature = powf(temperature, eccentricity);
		temperature *= 2000.0f;

		ffl_spring_forces(graph);
		ffl_repulsion_accelerated(graph);
		//ffl_repulsion_naive(graph);
		ffl_apply_forces(graph, temperature);
		rounds--;
	}
}

void
ffl_compute_layout(FFL_Graph *graph)
{
	int *mapping = calloc(graph->nverts, sizeof *mapping);
	FFL_Graph *reduced = ffl_reduce_graph(graph, mapping);
	if (reduced) {
		reduced->spring_strength = graph->spring_strength;
		reduced->repulsion_strength = graph->repulsion_strength;
		reduced->repulsion_accuracy = graph->repulsion_accuracy;
		ffl_compute_layout(reduced);
		ffl_interpolate_layout(reduced, mapping, graph);
		ffl_free_graph(reduced);
	} else {
		ffl_initial_layout(graph);
	}
	layout_graph(graph);
	free(mapping);
}

