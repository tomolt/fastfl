#include <stdio.h>

#include <expat.h>
#include <fastfl.h>

#include "realloc.h"
#include "graph.h"

#define READ_BUFFER_SIZE 4096

typedef struct GML_State GML_State;

struct GML_State {
	int depth;
};

static void
start(void *data, const XML_Char *elem, const XML_Char **attr)
{
	GML_State *state = data;
	for (int i = 0; i < state->depth; i++) {
		printf("  ");
	}
	printf("%s", elem);
	for (int i = 0; attr[i]; i += 2) {
		printf(" %s='%s'", attr[i], attr[i+1]);
	}
	printf("\n");
	state->depth++;
}

static void
end(void *data, const XML_Char *el)
{
	(void) el;
	GML_State *state = data;
	state->depth--;
}

int
graphml_read(const char *filename)
{
	int status = 0;
	GML_State state = { 0 };
	FILE *file = fopen(filename, "r");
	XML_Parser xp = XML_ParserCreate(NULL);
	XML_SetElementHandler(xp, start, end);
	XML_SetUserData(xp, &state);

	do {
		void *buf = XML_GetBuffer(xp, READ_BUFFER_SIZE);
		if (!buf) {
			status = -1;
			break;
		}

		size_t got = fread(buf, 1, READ_BUFFER_SIZE, file);
		if (ferror(file)) {
			status = -1;
			break;
		}

		if (!XML_ParseBuffer(xp, got, feof(file))) {
			status = -1;
			break;
		}
	} while (!feof(file));

	XML_ParserFree(xp);
	fclose(file);
	return status;
}

