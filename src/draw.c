#include <stdlib.h>

#include "graph.h"
#include "draw.h"

#define SET_PIXEL(image,x,y,v) (image)->pixels[(image)->width * (y) + (x)] = (v)

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
ffl_draw_graph(const FFL_Graph *graph, float offsetx, float offsety, FFL_Image *image)
{
	for (int e = 0; e < graph->nedges; e++) {
		const FFL_Edge   *edge   = &graph->edges[e];
		const FFL_Vertex *source = &graph->verts[edge->source];
		const FFL_Vertex *target = &graph->verts[edge->target];
		draw_line(image,
			(int) (source->x + offsetx), (int) (source->y + offsety),
			(int) (target->x + offsetx), (int) (target->y + offsety));
	}

	for (int v = 0; v < graph->nverts; v++) {
		const FFL_Vertex *vert = &graph->verts[v];
		draw_rhombus(image, (int) (vert->x + offsetx), (int) (vert->y + offsety), 5);
	}
}

void
ffl_write_pgm(const FFL_Image *image, FILE *file)
{
	fprintf(file, "P5\n%d %d\n255\n", image->width, image->height);
	fwrite(image->pixels, image->width, image->height, file);
}

