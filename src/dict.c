#include <stdlib.h>
#include <string.h>

#include "dict.h"

#define SWAP(type,a,b) do { type _t=(a); (a)=(b); (b)=_t; } while (0)

uint32_t
ffl_hash_string(const char *str)
{
	uint32_t hash = 2166136261;
	for (size_t i = 0; str[i]; i++) {
		hash ^= str[i];
		hash *= 16777619;
	}
	return hash;
}

/* cap must be a power of two! */
void
ffl_dict_init(FFL_Dict *dict, int cap)
{
	dict->num    = 0;
	dict->cap    = cap;
	dict->slots  = calloc(dict->cap, sizeof *dict->slots);
	dict->values = calloc(dict->cap, sizeof *dict->values);
}

void
ffl_dict_free(FFL_Dict *dict)
{
	free(dict->slots);
	free(dict->values);
}

static void
dict_insert(FFL_Dict *dict, char *key, uint32_t hash, void *value)
{
	for (int dib = 0;; dib++) {
		int idx = (hash + dib) & (dict->cap - 1);
		FFL_DSlot *slot = &dict->slots[idx];

		if (!slot->key) {
			slot->key  = key;
			slot->hash = hash;
			slot->dib  = dib;
			dict->values[idx] = value;
			break;
		}

		if (dib > slot->dib) {
			SWAP(char *,   key,   slot->key);
			SWAP(uint32_t, hash,  slot->hash);
			SWAP(int,      dib,   slot->dib);
			SWAP(void *,   value, dict->values[idx]);
		}

		if (slot->hash == hash && !strcmp(slot->key, key)) {
			dict->values[idx] = value;
			free(key);
			break;
		}
	}
}

void
ffl_dict_put(FFL_Dict *dict, const char *key, void *value)
{
	if (4 * dict->num > 3 * dict->cap) {
		FFL_Dict ndict;
		ffl_dict_init(&ndict, 2 * dict->cap);
		for (int i = 0; i < dict->cap; i++) {
			FFL_DSlot *slot = &dict->slots[i];
			if (!slot->key) continue;
			dict_insert(&ndict, slot->key, slot->hash, dict->values[i]);
		}
		ffl_dict_free(dict);
		memcpy(dict, &ndict, sizeof ndict);
	}

	char *akey = malloc(strlen(key) + 1);
	strcpy(akey, key);
	uint32_t hash = ffl_hash_string(key);
	dict_insert(dict, akey, hash, value);
}

bool
ffl_dict_get(FFL_Dict *dict, const char *key, void **value)
{
	uint32_t hash = ffl_hash_string(key);
	for (int dib = 0;; dib++) {
		int idx = (hash + dib) & (dict->cap - 1);
		FFL_DSlot *slot = &dict->slots[idx];
		if (!slot->key) return false;
		if (dib > slot->dib) return false;
		if (slot->hash == hash && !strcmp(slot->key, key)) {
			dict->values[idx] = value;
			return true;
		}
	}
}

