#include <stdint.h>
#include <stdbool.h>

typedef struct FFL_DSlot FFL_DSlot;
typedef struct FFL_Dict  FFL_Dict;

struct FFL_DSlot {
	const char *key;
	uint32_t    hash;
	uint32_t    dib;
};

struct FFL_Dict {
	uint32_t   num;
	uint32_t   cap;
	FFL_DSlot *slots;
	void     **values;
};

uint32_t ffl_hash_string(const char *str);
/* cap must be a power of two! */
void ffl_dict_init(FFL_Dict *dict, uint32_t cap);
void ffl_dict_free(FFL_Dict *dict);
bool ffl_dict_put(FFL_Dict *dict, const char *key, void *value);
bool ffl_dict_get(FFL_Dict *dict, const char *key, void **value);

