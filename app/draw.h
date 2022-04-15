#include <stdio.h>

typedef struct FFL_Graph FFL_Graph;
typedef struct FFL_Image FFL_Image;

struct FFL_Image {
	int width;
	int height;
	int offset_x;
	int offset_y;
	unsigned char *pixels;
};

void ffl_draw_graph(const FFL_Graph *graph, FFL_Image *image);
void ffl_draw_clumping(const FFL_Graph *graph, FFL_Image *image);
void ffl_write_pgm(const FFL_Image *image, FILE *file);

