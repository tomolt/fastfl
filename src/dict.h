#include <stdint.h>
#include <stdbool.h>

typedef struct FFL_DSlot FFL_DSlot;
typedef struct FFL_Dict  FFL_Dict;

struct FFL_DSlot {
	char    *key;
	uint32_t hash;
	int      dib;
};

struct FFL_Dict {
	int        num;
	int        cap;
	FFL_DSlot *slots;
	void     **values;
};

uint32_t ffl_hash_string(const char *str);
/* cap must be a power of two! */
void ffl_dict_init(FFL_Dict *dict, int cap);
void ffl_dict_free(FFL_Dict *dict);
void ffl_dict_put(FFL_Dict *dict, const char *key, void *value);
bool ffl_dict_get(FFL_Dict *dict, const char *key, void **value);

