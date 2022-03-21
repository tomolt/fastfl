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
ffl_dict_init(FFL_Dict *dict, uint32_t cap)
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

static bool
dict_insert(FFL_Dict *dict, const char *key, uint32_t hash, void *value)
{
	for (uint32_t dib = 0;; dib++) {
		uint32_t idx = (hash + dib) & (dict->cap - 1);
		FFL_DSlot *slot = &dict->slots[idx];

		if (!slot->key) {
			slot->key  = key;
			slot->hash = hash;
			slot->dib  = dib;
			dict->values[idx] = value;
			return true;
		}

		if (dib > slot->dib) {
			SWAP(const char *, key,   slot->key);
			SWAP(uint32_t,     hash,  slot->hash);
			SWAP(int,          dib,   slot->dib);
			SWAP(void *,       value, dict->values[idx]);
		}

		if (slot->hash == hash && !strcmp(slot->key, key)) {
			return false;
		}
	}
}

bool
ffl_dict_put(FFL_Dict *dict, const char *key, void *value)
{
	if (3 * dict->num > 2 * dict->cap) {
		FFL_Dict ndict;
		ffl_dict_init(&ndict, 2 * dict->cap);
		for (uint32_t i = 0; i < dict->cap; i++) {
			FFL_DSlot *slot = &dict->slots[i];
			if (!slot->key) continue;
			dict_insert(&ndict, slot->key, slot->hash, dict->values[i]);
		}
		ffl_dict_free(dict);
		memcpy(dict, &ndict, sizeof ndict);
	}

	uint32_t hash = ffl_hash_string(key);
	return dict_insert(dict, key, hash, value);
}

bool
ffl_dict_get(FFL_Dict *dict, const char *key, void **value)
{
	uint32_t hash = ffl_hash_string(key);
	for (uint32_t dib = 0;; dib++) {
		uint32_t idx = (hash + dib) & (dict->cap - 1);
		FFL_DSlot *slot = &dict->slots[idx];
		if (!slot->key) return false;
		if (dib > slot->dib) return false;
		if (slot->hash == hash && !strcmp(slot->key, key)) {
			dict->values[idx] = value;
			return true;
		}
	}
}

