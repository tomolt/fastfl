#include <stdio.h>

#include <expat.h>
#include <fastfl.h>

#include "realloc.h"
#include "graph.h"

#define READ_BUFFER_SIZE 4096
#define STACK_SIZE 10

#define BAIL(state) XML_StopParser(state->xp, XML_FALSE)
#define PUSHEL(state,el) state->stack[state->head] = el

typedef struct GML_State GML_State;

enum {
	EL_NONE,
	EL_GRAPHML,
	EL_GRAPH,
	EL_KEY,
	EL_NODE,
	EL_DATA,
	EL_EDGE,
	EL_UNKNOWN
};

struct GML_State {
	XML_Parser xp;
	int status;
	int head;
	int stack[STACK_SIZE];
};

static void
start(void *data, const XML_Char *elem, const XML_Char **attr)
{
	GML_State *state = data;
	if (state->head >= STACK_SIZE - 1) {
		BAIL(state);
		return;
	}
	switch (state->stack[state->head]) {
	case EL_NONE:
		if (!strcmp(elem, "graphml")) {
			PUSHEL(state, EL_GRAPHML);
		} else {
			BAIL(state);
		}
		break;

	case EL_GRAPHML:
		if (!strcmp(elem, "graph")) {
			PUSHEL(state, EL_GRAPH);
		} else {
			PUSHEL(state, EL_UNKNOWN);
		}
		break;

	case EL_GRAPH:
		if (!strcmp(elem, "key")) {
			PUSHEL(state, EL_KEY);
			for (int i = 0; attrs[i]; i += 2) {
				if (!strcmp(attrs[i], "id")) {
				}
				if (!strcmp(attrs[i], "for")) {
				}
				if (!strcmp(attrs[i], "attr.name")) {
				}
				if (!strcmp(attrs[i], "attr.type")) {
				}
			}
		} else if (!strcmp(elem, "node")) {
			PUSHEL(state, EL_NODE);
			for (int i = 0; attrs[i]; i += 2) {
				if (!strcmp(attrs[i], "id")) {
				}
			}
		} else if (!strcmp(elem, "edge")) {
			PUSHEL(state, EL_EDGE);
			for (int i = 0; attrs[i]; i += 2) {
				if (!strcmp(attrs[i], "id")) {
				}
				if (!strcmp(attrs[i], "source")) {
				}
				if (!strcmp(attrs[i], "target")) {
				}
			}
		} else {
			PUSHEL(state, EL_UNKNOWN);
		}
		break;

	case EL_NODE:
		if (!strcmp(elem, "data")) {
			PUSHEL(EL_DATA);
			for (int i = 0; attrs[i]; i += 2) {
				if (!strcmp(attrs[i], "key")) {
				}
			}
		} else {
			PUSHEL(UL_UNKNOWN);
		}
		break;

	default:
		PUSHEL(state, EL_UNKNOWN);
	}
}

static void
text(void *data, const XML_Char *str, int len)
{
}

static void
end(void *data, const XML_Char *elem)
{
	(void) elem;
	GML_State *state = data;
	state->head--;
}

int
graphml_read(const char *filename)
{
	int status = 0;
	GML_State state = { 0 };

	XML_Parser xp = XML_ParserCreate(NULL);
	XML_SetUserData(xp, &state);
	XML_SetElementHandler(xp, start, end);
	XML_SetCharacterDataHandler(xp, text);

	FILE *file = fopen(filename, "r");
	if (!file) {
		status = -1;
		goto cleanup;
	}

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

cleanup:
	XML_ParserFree(xp);
	if (file) fclose(file);
	return status;
}

