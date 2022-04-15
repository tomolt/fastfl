#include <stdlib.h>
#include <math.h>

#include "graph.h"
#include "draw.h"

#define SET_PIXEL(image,x,y,v) (image)->pixels[(image)->width * ((y) + (image)->offset_y) + ((x) + (image)->offset_x)] = (v)

static void
draw_line(FFL_Image *image, int x0, int y0, int x1, int y1)
{
    int dx =  abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy; /* error value e_xy */
    
    for (;;) {
        SET_PIXEL(image, x0, y0, 0x7Fu);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > dy) { err += dy; x0 += sx; } /* e_xy+e_x > 0 */
        if (e2 < dx) { err += dx; y0 += sy; } /* e_xy+e_y < 0 */
    }
}

static void
draw_rhombus(FFL_Image *image, int x, int y, int radius)
{
	for (int oy = -radius; oy <= radius; oy++) {
		for (int ox = -radius; ox <= radius; ox++) {
			int d = abs(ox) + abs(oy);
			if (d <= radius) {
				SET_PIXEL(image, x + ox, y + oy, 0xFFu);
			}
		}
	}
}

void
ffl_draw_graph(const FFL_Graph *graph, FFL_Image *image)
{
	for (int e = 0; e < graph->nedges; e++) {
		const FFL_Edge *edge = &graph->edges[e];
		FFL_Vec2 source = graph->verts_pos[edge->source];
		FFL_Vec2 target = graph->verts_pos[edge->target];
		draw_line(image,
			(int) source.x, (int) source.y,
			(int) target.x, (int) target.y);
	}

	for (int v = 0; v < graph->nverts; v++) {
		FFL_Vec2 pos = graph->verts_pos[v];
		draw_rhombus(image, (int) pos.x, (int) pos.y, 5);
	}
}

static int
draw_clump(const FFL_Graph *graph, const FFL_Clump *clump, FFL_Image *image)
{
	float x = clump->com.x;
	float y = clump->com.y;
	int r;
	if (clump->is_leaf) {
		for (int v = clump->low; v < clump->high; v++) {
			FFL_Vec2 pos = graph->verts_pos[v];
			draw_line(image, x, y, pos.x, pos.y);
			draw_rhombus(image, pos.x, pos.y, 2);
		}
		r = 3;
	} else {
		draw_line(image, x, y, clump->child0->com.x, clump->child0->com.y);
		draw_line(image, x, y, clump->child1->com.x, clump->child1->com.y);
		int r1 = draw_clump(graph, clump->child0, image);
		int r2 = draw_clump(graph, clump->child1, image);
		r = (r1 > r2 ? r1 : r2) + 1;
	}
	draw_rhombus(image, x, y, r);
	return r;
}

void
ffl_draw_clumping(const FFL_Graph *graph, FFL_Image *image)
{
	draw_clump(graph, graph->root_clump, image);
}

void
ffl_write_pgm(const FFL_Image *image, FILE *file)
{
	fprintf(file, "P5\n%d %d\n255\n", image->width, image->height);
	fwrite(image->pixels, image->width, image->height, file);
}

