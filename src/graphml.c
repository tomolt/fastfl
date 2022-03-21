#include <stdio.h>
#include <string.h>

#include <expat.h>
#include <fastfl.h>

#include "realloc.h"
#include "graph.h"
#include "dict.h"

#define READ_BUFFER_SIZE 4096
#define STACK_SIZE 10
#define TEXT_SIZE 256

#define BAIL(state) XML_StopParser(state->xp, XML_FALSE)
#define PUSHEL(state,el) state->stack[++state->stack_head] = el

typedef struct GML_State GML_State;

enum {
	EL_NONE,
	EL_GRAPHML,
	EL_GRAPH,
	EL_KEY,
	EL_DEFAULT,
	EL_NODE,
	EL_DATA,
	EL_EDGE,
	EL_UNKNOWN
};

struct GML_State {
	XML_Parser xp;
	
	int stack_head;
	int stack[STACK_SIZE];

	int  text_length;
	char text[TEXT_SIZE];
	
	FFL_Dict key_dict;
	FFL_Dict node_dict;

	int cur_node;
	int cur_edge;
};

static void
process_start_tag(void *data, const XML_Char *elem, const XML_Char **attr)
{
	GML_State *state = data;
	if (state->stack_head >= STACK_SIZE - 1) {
		BAIL(state);
		return;
	}
	switch (state->stack[state->stack_head]) {
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
			for (int i = 0; attr[i]; i += 2) {
				if (!strcmp(attr[i], "id")) {
				}
				if (!strcmp(attr[i], "for")) {
				}
				if (!strcmp(attr[i], "attr.name")) {
				}
				if (!strcmp(attr[i], "attr.type")) {
				}
			}
		} else if (!strcmp(elem, "node")) {
			PUSHEL(state, EL_NODE);
			for (int i = 0; attr[i]; i += 2) {
				if (!strcmp(attr[i], "id")) {
				}
			}
		} else if (!strcmp(elem, "edge")) {
			PUSHEL(state, EL_EDGE);
			for (int i = 0; attr[i]; i += 2) {
				if (!strcmp(attr[i], "source")) {
				}
				if (!strcmp(attr[i], "target")) {
				}
			}
		} else {
			PUSHEL(state, EL_UNKNOWN);
		}
		break;

	case EL_NODE:
		if (!strcmp(elem, "data")) {
			PUSHEL(state, EL_DATA);
			for (int i = 0; attr[i]; i += 2) {
				if (!strcmp(attr[i], "key")) {
				}
			}
			state->text_length = 0;
		} else {
			PUSHEL(state, EL_UNKNOWN);
		}
		break;

	default:
		PUSHEL(state, EL_UNKNOWN);
	}
}

static void
process_text(void *data, const XML_Char *str, int len)
{
	GML_State *state = data;
	switch (state->stack[state->stack_head]) {
	case EL_DATA:
		if (state->text_length + len > TEXT_SIZE) {
			BAIL(state);
		}
		memcpy(state->text + state->text_length, str, len);
		state->text_length += len;
		break;

	default:
	}
}

static void
process_end_tag(void *data, const XML_Char *elem)
{
	(void) elem;
	GML_State *state = data;
	printf("Element '%s' had text:%.*s\n", elem, state->text_length, state->text);
	state->stack_head--;
	state->text_length = 0;
}

int
graphml_read(const char *filename)
{
	int status = 0;
	GML_State state = { 0 };
	state.stack[0] = EL_NONE;
	ffl_dict_init(&state.node_dict, 16);

	state.xp = XML_ParserCreate(NULL);
	XML_SetUserData(state.xp, &state);
	XML_SetElementHandler(state.xp, process_start_tag, process_end_tag);
	XML_SetCharacterDataHandler(state.xp, process_text);

	FILE *file = fopen(filename, "r");
	if (!file) {
		status = -1;
		goto cleanup;
	}

	do {
		void *buf = XML_GetBuffer(state.xp, READ_BUFFER_SIZE);
		if (!buf) {
			status = -1;
			break;
		}

		size_t got = fread(buf, 1, READ_BUFFER_SIZE, file);
		if (ferror(file)) {
			status = -1;
			break;
		}

		if (!XML_ParseBuffer(state.xp, got, feof(file))) {
			status = -1;
			break;
		}
	} while (!feof(file));

cleanup:
	ffl_dict_free(&state.node_dict);
	XML_ParserFree(state.xp);
	if (file) fclose(file);
	return status;
}

