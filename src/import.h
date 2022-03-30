#include <stdbool.h>

typedef struct FFL_Graph FFL_Graph;
typedef struct FFL_FileFlavor FFL_FileFlavor;

struct FFL_FileFlavor {
	int (*format_reader)(FFL_Graph *graph, char *line, const FFL_FileFlavor *flavor);
	char delimiter;
	char comment_marker;
	bool comments_allowed;
};

int ffl_edge_format(FFL_Graph *graph, char *line, const FFL_FileFlavor *flavor);
FFL_Graph *ffl_import(FILE *file, const FFL_FileFlavor *flavor);

