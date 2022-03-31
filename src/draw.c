#include <stdlib.h>

#include "graph.h"
#include "draw.h"

static void
set_pixel(FFL_Image *image, int x, int y)
{
	image->pixels[image->width * y + x] = 0xFFu;
}

static void
draw_line(FFL_Image *image, int x0, int y0, int x1, int y1)
{
    int dx =  abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy; /* error value e_xy */
    
    for (;;) {
        set_pixel(image, x0, y0);
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
				set_pixel(image, x + ox, y + oy);
			}
		}
	}
}

void
ffl_draw_graph(const FFL_Graph *graph, FFL_Image *image)
{
	for (int e = 0; e < graph->nedges; e++) {
		const FFL_Edge   *edge   = &graph->edges[e];
		const FFL_Vertex *source = &graph->verts[edge->source];
		const FFL_Vertex *target = &graph->verts[edge->target];
		draw_line(image, (int) source->x, (int) source->y, (int) target->x, (int) target->y);
	}

	for (int v = 0; v < graph->nverts; v++) {
		const FFL_Vertex *vert = &graph->verts[v];
		draw_rhombus(image, (int) vert->x, (int) vert->y, 5);
	}
}

