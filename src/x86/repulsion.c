#include <fastfl.h>

#include <immintrin.h>

#include "graph.h"

void
ffl_repulsion_2on2(FFL_Graph *graph, int i, int j)
{
	/* Load & shuffle pos of A and B */
	__m128 ab_xy  = _mm_loadu_ps((void *) &graph->verts_pos[i]);
	__m128 abab_x = _mm_shuffle_ps(ab_xy, ab_xy, _MM_SHUFFLE(2,0,2,0));
	__m128 abab_y = _mm_shuffle_ps(ab_xy, ab_xy, _MM_SHUFFLE(3,1,3,1));

	/* Load & shuffle pos of C and D */
	__m128 cd_xy  = _mm_loadu_ps((void *) &graph->verts_pos[j]);
	__m128 ccdd_x = _mm_shuffle_ps(cd_xy, cd_xy, _MM_SHUFFLE(2,2,0,0));
	__m128 ccdd_y = _mm_shuffle_ps(cd_xy, cd_xy, _MM_SHUFFLE(3,3,1,1));

	/* Compute difference vectors for each pair in {A,B}x{C,D} */
	__m128 ab2cd_x = ccdd_x - abab_x;
	__m128 ab2cd_y = ccdd_y - abab_y;

	/* Divide by distance squared, multiply with strength constant */
	__m128 dist_sq = ab2cd_x * ab2cd_x + ab2cd_y * ab2cd_y;
	__m128 factor = _mm_rcp_ps(dist_sq) * graph->repulsion_strength;
	ab2cd_x *= factor;
	ab2cd_y *= factor;

	/* Swizzle difference vectors into the same format as the force vectors */
	__m128 ab2c_xy = _mm_unpacklo_ps(ab2cd_x, ab2cd_y);
	__m128 ab2d_xy = _mm_unpackhi_ps(ab2cd_x, ab2cd_y);
	__m128 a2cd_xy = _mm_shuffle_ps(ab2c_xy, ab2d_xy, _MM_SHUFFLE(1,0,1,0));
	__m128 b2cd_xy = _mm_shuffle_ps(ab2c_xy, ab2d_xy, _MM_SHUFFLE(3,2,3,2));

	/* Add to force on A and B */
	__m128 ab_force_xy = _mm_loadu_ps((void *) &graph->verts_force[i]);
	ab_force_xy -= ab2c_xy * graph->verts_charge[j];
	ab_force_xy -= ab2d_xy * graph->verts_charge[j+1];
	_mm_storeu_ps((void *) &graph->verts_force[i], ab_force_xy);

	/* Add to force on C and D */
	__m128 cd_force_xy = _mm_loadu_ps((void *) &graph->verts_force[j]);
	cd_force_xy += a2cd_xy * graph->verts_charge[i];
	cd_force_xy += b2cd_xy * graph->verts_charge[i+1];
	_mm_storeu_ps((void *) &graph->verts_force[j], cd_force_xy);
}

