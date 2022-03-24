#include <stdio.h>
#include <string.h>

#include <expat.h>
#include <fastfl.h>

#include "graph.h"
#include "dict.h"

#define READ_BUFFER_SIZE 4096
#define TEXT_SIZE 256

#define BAIL(state) XML_StopParser(state->xp, XML_FALSE)

typedef struct GML_State GML_State;

struct GML_State {
	XML_Parser xp;
	
	int  text_length;
	char text[TEXT_SIZE];
	
	FFL_Dict node_dict;

	int      cur_node;
	int      cur_edge;
};

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
	state->text_length = 0;
	if (!strcmp(elem, "key")) {
		//get_attr(attr, "attr.name");
		//get_attr(attr, "for");
		//get_attr(attr, "attr.type");
		//get_attr(attr, "id");
	} else if (!strcmp(elem, "default")) {
	} else if (!strcmp(elem, "node")) {
		get_attr(attr, "id");
	} else if (!strcmp(elem, "edge")) {
		const char *s_source, *s_target;
		s_source = get_attr(attr, "source");
		s_target = get_attr(attr, "target");
		if (!s_source || !s_target) goto fail;
	} else if (!strcmp(elem, "data")) {
		get_attr(attr, "key");
	}
	return;
fail:
	BAIL(state);
}

static void
process_text(void *data, const XML_Char *str, int len)
{
	GML_State *state = data;
	if (state->text_length + len > TEXT_SIZE) {
		BAIL(state);
	}
	memcpy(state->text + state->text_length, str, len);
	state->text_length += len;
}

static void
process_end_tag(void *data, const XML_Char *elem)
{
	GML_State *state = data;
	if (!strcmp(elem, "default")) {
	} else if (!strcmp(elem, "data")) {
	}
	state->text_length = 0;
}

int
graphml_read(const char *filename)
{
	int status = 0;
	GML_State state = { 0 };
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

