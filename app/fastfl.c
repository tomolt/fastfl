#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <fastfl.h>

#include "graph.h"
#include "import.h"
#include "arg.h"
#include "draw.h"

char *argv0;

static void
usage(void)
{
	fprintf(stderr, "usage: %s [FILE]\n", argv0);
	exit(1);
}

static void
dump_graph(const FFL_Graph *graph)
{
	printf("V=%d, E=%d\n", graph->nverts, graph->nedges);
	for (int e = 0; e < graph->nedges; e++) {
		printf("v%d -> v%d\n", graph->edges[e].source, graph->edges[e].target);
	}
}

void
ffl_rescale(FFL_Graph *graph, float wanted_width, float wanted_height)
{
	FFL_Rect rect;
	ffl_bounding_box(graph->verts_pos, 0, graph->nverts, &rect);
	printf("Graph bounding box: (%f, %f) - (%f, %f)\n", rect.min.x, rect.min.y, rect.max.x, rect.max.y);

	float scale1 = wanted_width  / (rect.max.x - rect.min.x);
	float scale2 = wanted_height / (rect.max.y - rect.min.y);
	float scale = scale1 < scale2 ? scale1 : scale2;

	for (int v = 0; v < graph->nverts; v++) {
		FFL_Vec2 pos = graph->verts_pos[v];
		pos.x = (pos.x - rect.min.x) * scale;
		pos.y = (pos.y - rect.min.y) * scale;
		graph->verts_pos[v] = pos;
	}
}

int
main(int argc, char **argv)
{
	ARGBEGIN {
	default:
		usage();
	} ARGEND

	if (argc != 1) {
		usage();
	}

	FILE *file = fopen(*argv, "r");
	if (!file) {
		fprintf(stderr, "Error reading graph file.\n");
		exit(1);
	}

	/* incidence table in .tsv format */
	FFL_FileFlavor flavor;
	flavor.format_reader = ffl_edge_format;
	flavor.delimiter = '\t';
	flavor.comment_marker = '#';
	flavor.comments_allowed = true;

	FFL_Graph *graph = ffl_import(file, &flavor);
	fclose(file);
	if (!graph) {
		fprintf(stderr, "Error reading graph file.\n");
		exit(1);
	}

	dump_graph(graph);

	graph->spring_strength    = 1.0f;
	graph->repulsion_strength = 1.0f;
	graph->repulsion_accuracy = 0.1f;
	ffl_compute_layout(graph);

	ffl_rescale(graph, 2048 - 100, 2048 - 100);

	FFL_Image image;
	image.width    = 2048;
	image.height   = 2048;
	image.offset_x = 50;
	image.offset_y = 50;
	image.pixels   = calloc(image.width, image.height);

#if 1
	ffl_draw_graph(graph, &image);
#else
	ffl_form_clumps(graph);
	ffl_draw_clumping(graph, &image);
	ffl_homogenize(graph);
#endif

	FILE *outfile = fopen("out.pgm", "wb");
	ffl_write_pgm(&image, outfile);
	fclose(outfile);

	free(image.pixels);

	ffl_free_graph(graph);
	return 0;
}

