#include <stdio.h>
#include <string.h>

#define LINE_MAX 1024

#define FREM 0
#define FDEL 1
#define FENC 2

static int
getcell(int idx)
{
}

static int
eline(char *line)
{
	int source = getcell(0);
	int target = getcell(1);
	return 0;
}

static int
iline(char *line)
{
	int vert = getcell(0);
	int edge = getcell(1);
	return 0;
}

//TODO aline()
//TODO mline()

FFL_Graph *
ffl_import(FILE *file, const char *flavor)
{
	FFL_Graph *graph = ffl_new_graph();

	linefunc;
	switch (flavor[FENC]) {
	case 'E': linefunc = eline; break;
	case 'I': linefunc = iline; break;
	}

	char line[LINE_MAX];
	while (fgets(line, LINE_MAX, file)) {
		if (flavor[FREM] != '0' && line[0] == flavor[FREM]) continue;
		linefunc();		
	}
	if (ferror(file)) goto fail;

	return graph;
fail:
	ffl_free_graph(graph);
	return NULL;
}

