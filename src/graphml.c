#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <expat.h>
#include <fastfl.h>

#include "graph.h"
#include "dict.h"

#define READ_BUFFER_SIZE 4096
#define TEXT_SIZE 256

#define BAIL(state) XML_StopParser(state->xp, XML_FALSE)

typedef struct GML_State GML_State;

enum {
	ATTR_EDGE_WEIGHT,
};

struct GML_State {
	XML_Parser xp;
	FFL_Graph *graph;
	
	bool text_enable;
	int  text_length;
	char text[TEXT_SIZE];

	FFL_Dict key_dict;
	FFL_Dict node_dict;

	int cur_attr;
	char *cur_id;
	FFL_Vertex cur_vert;
	FFL_Edge cur_edge;

	float default_edge_weight;
};

static int
ffl_push_vertex(FFL_Graph *graph, FFL_Vertex vertex)
{
	int idx = graph->nverts;
	if (++graph->nverts > graph->cverts) {
		graph->cverts *= 2;
		graph->verts = realloc(graph->verts, graph->cverts * sizeof *graph->verts);
	}
	graph->verts[idx] = vertex;
	return idx;
}

static int
ffl_push_edge(FFL_Graph *graph, FFL_Edge edge)
{
	int idx = graph->nedges;
	if (++graph->nedges > graph->cedges) {
		graph->cedges *= 2;
		graph->edges = realloc(graph->edges, graph->cedges * sizeof *graph->edges);
	}
	graph->edges[idx] = edge;
	return idx;
}

static char *
store_string(const char *str)
{
	// TODO reduce individual allocations
	char *ptr = malloc(strlen(str) + 1);
	strcpy(ptr, str);
	return ptr;
}

static const char *
get_attr(const XML_Char **attr, const char *name)
{
	for (int i = 0; attr[i]; i += 2) {
		if (!strcmp(attr[i], name)) return attr[i+1];
	}
	return NULL;
}

static void
process_start_tag(void *data, const XML_Char *elem, const XML_Char **attr)
{
	GML_State *state = data;
	if (!strcmp(elem, "key")) {
		const char *xid   = get_attr(attr, "id");
		const char *xname = get_attr(attr, "attr.name");
		const char *xfor  = get_attr(attr, "for");
		const char *xtype = get_attr(attr, "attr.type");
		if (!xid || !xname || !xfor || !xtype) goto fail;

		state->cur_id = store_string(xid);

		if (!strcmp(xname, "weight") && !strcmp(xfor, "edge")) {
			state->cur_attr = ATTR_EDGE_WEIGHT;
		} else {
			state->cur_attr = -1;
		}
	} else if (!strcmp(elem, "node")) {
		memset(&state->cur_vert, 0, sizeof (FFL_Vertex));

		const char *xid = get_attr(attr, "id");

		if (!xid) goto fail;
		
		state->cur_id = store_string(xid);
	} else if (!strcmp(elem, "edge")) {
		memset(&state->cur_edge, 0, sizeof (FFL_Edge));

		const char *xsource = get_attr(attr, "source");
		const char *xtarget = get_attr(attr, "target");

		if (!xsource || !ffl_dict_get(&state->node_dict, xsource, &state->cur_edge.source)) goto fail;
		if (!xtarget || !ffl_dict_get(&state->node_dict, xtarget, &state->cur_edge.target)) goto fail;
	}
	return;
fail:
	BAIL(state);
}

static void
process_end_tag(void *data, const XML_Char *elem)
{
	GML_State *state = data;
	if (!strcmp(elem, "key")) {
		if (!ffl_dict_put(&state->key_dict, state->cur_id, state->cur_attr)) goto fail;
	} else if (!strcmp(elem, "node")) {
		int idx = ffl_push_vertex(state->graph, state->cur_vert);
		if (!ffl_dict_put(&state->node_dict, state->cur_id, idx)) goto fail;
	} else if (!strcmp(elem, "edge")) {
		ffl_push_edge(state->graph, state->cur_edge);
	}
	return;
fail:
	BAIL(state);
}

static void
process_text(void *data, const XML_Char *str, int len)
{
	GML_State *state = data;
	if (!state->text_enable) return;
	if (state->text_length + len > TEXT_SIZE) {
		BAIL(state);
		return;
	}
	memcpy(state->text + state->text_length, str, len);
	state->text_length += len;
}

int
graphml_read(const char *filename, FFL_Graph *graph)
{
	graph->nverts = 0;
	graph->cverts = 16;
	graph->verts  = calloc(graph->cverts, sizeof *graph->verts);
	graph->nedges = 0;
	graph->cedges = 16;
	graph->edges  = calloc(graph->cedges, sizeof *graph->edges);

	int status = 0;
	GML_State state = { 0 };
	state.graph = graph;
	ffl_dict_init(&state.key_dict, 16);
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
	ffl_dict_free(&state.key_dict);
	ffl_dict_free(&state.node_dict);
	XML_ParserFree(state.xp);
	if (file) fclose(file);
	return status;
}

