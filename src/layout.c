#include <fastfl.h>

#include <stdbool.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <immintrin.h>

#include "graph.h"

#define FFL_PI 3.14159265358979323846

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
		factor *= dist - edge->d_length;
		factor *= inv_dist;

		dx *= factor;
		dy *= factor;

		graph->verts_force[edge->source].x += dx;
		graph->verts_force[edge->source].y += dy;

		graph->verts_force[edge->target].x -= dx;
		graph->verts_force[edge->target].y -= dy;
	}
}

void
ffl_repulsion_2on2(FFL_Graph *graph, int i, int j)
{
	__m128 ab = _mm_loadu_ps((void *) &graph->verts_pos[i]);
	__m128 cd = _mm_loadu_ps((void *) &graph->verts_pos[j]);

	__m128 aabb_x = _mm_shuffle_ps(ab, ab, _MM_SHUFFLE(2,2,0,0));
	__m128 aabb_y = _mm_shuffle_ps(ab, ab, _MM_SHUFFLE(3,3,1,1));
	__m128 cdcd_x = _mm_shuffle_ps(cd, cd, _MM_SHUFFLE(2,0,2,0));
	__m128 cdcd_y = _mm_shuffle_ps(cd, cd, _MM_SHUFFLE(3,1,3,1));

	__m128 dir_x = cdcd_x - aabb_x;
	__m128 dir_y = cdcd_y - aabb_y;
	__m128 dist_sq = dir_x * dir_x + dir_y * dir_y;

	__m128 factor = _mm_rcp_ps(dist_sq) * _mm_set_ps1(graph->repulsion_strength);
	dir_x *= factor;
	dir_y *= factor;

	__m128 cd_force_1 = _mm_unpacklo_ps(dir_x, dir_y);
	__m128 cd_force_2 = _mm_unpackhi_ps(dir_x, dir_y);
	__m128 ab_force_1 = _mm_shuffle_ps(cd_force_1, cd_force_2, _MM_SHUFFLE(1,0,1,0));
	__m128 ab_force_2 = _mm_shuffle_ps(cd_force_1, cd_force_2, _MM_SHUFFLE(3,2,3,2));

	__m128 ab_force = _mm_loadu_ps((void *) &graph->verts_force[i]);
	__m128 cd_force = _mm_loadu_ps((void *) &graph->verts_force[j]);

	ab_force -= ab_force_1 + ab_force_2;
	cd_force += cd_force_1 + cd_force_2;

	_mm_storeu_ps((void *) &graph->verts_force[i], ab_force);
	_mm_storeu_ps((void *) &graph->verts_force[j], cd_force);
}

void
ffl_repulsion_1onN(FFL_Graph *graph, int t, int low, int high)
{
	FFL_Vec2 target = graph->verts_pos[t];
	for (int s = low; s < high; s++) {
		FFL_Vec2 source = graph->verts_pos[s];

		float dx = target.x - source.x;
		float dy = target.y - source.y;
		float dist_sq = dx * dx + dy * dy;
		if (dist_sq == 0.0f) continue;

		float inv_dist_sq = dist_sq;
		__asm__ inline ("rcpss %0, %0" : "+v"(inv_dist_sq));

		float force = graph->repulsion_strength * inv_dist_sq;
		dx *= force;
		dy *= force;

		graph->verts_force[s].x -= dx;
		graph->verts_force[s].y -= dy;

		graph->verts_force[t].x += dx;
		graph->verts_force[t].y += dy;
	}
}

void
ffl_repulsion_naive(FFL_Graph *graph)
{
	for (int t = 0; t < graph->nverts; t++) {
		ffl_repulsion_1onN(graph, t, 0, t);
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

void
ffl_compute_layout(FFL_Graph *graph)
{
	const int TOTAL_ROUNDS = 500;
	const float eccentricity = 1.2f;

	ffl_initial_layout(graph);

	int rounds = TOTAL_ROUNDS;
	while (rounds) {
		float temperature = (float) rounds / TOTAL_ROUNDS;
		temperature = powf(temperature, eccentricity);
		temperature *= 3000.0f;

		ffl_spring_forces(graph);
		ffl_repulsion_accelerated(graph);
		ffl_apply_forces(graph, temperature);
		rounds--;
	}
}

