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
	flavor.format_reader = ffl_incidence_format;
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

	FFL_Settings settings;
	settings.spring_strength = 1.0f;
	settings.repulsion_strength = 1.0f;
	ffl_compute_layout(graph, &settings);

	float min_x = INFINITY, min_y = INFINITY, max_x = -INFINITY, max_y = -INFINITY;
	for (int v = 0; v < graph->nverts; v++) {
		const FFL_Vertex *vert = &graph->verts[v];
		if (vert->x < min_x) min_x = vert->x;
		if (vert->y < min_y) min_y = vert->y;
		if (vert->x > max_x) max_x = vert->x;
		if (vert->y > max_y) max_y = vert->y;
	}

	printf("Graph bounding box: (%f, %f) - (%f, %f)\n", min_x, min_y, max_x, max_y);

	FFL_Image image;
	image.width  = (int) (max_x - min_x) + 100;
	image.height = (int) (max_y - min_y) + 100;
	image.pixels = calloc(image.width, image.height);
	ffl_draw_graph(graph, -min_x + 50.0f, -min_y + 50.0f, &image);

	FILE *outfile = fopen("out.pgm", "wb");
	ffl_write_pgm(&image, outfile);
	fclose(outfile);

	free(image.pixels);

	ffl_free_graph(graph);
	return 0;
}

