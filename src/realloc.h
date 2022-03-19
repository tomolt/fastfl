#include <stdlib.h>
#include <stdint.h>
#include <errno.h>

#define IS_POW2(x) (!((x) & ((x)-1)))
#define GROW_ARRAY(arr, len) do {							\
		if (IS_POW2(len)) {							\
			arr = reallocarray(arr, len ? len * 2 : 1, sizeof(arr[0]));	\
		}									\
	} while (0)

/*
 * This is sqrt(SIZE_MAX+1), as s1*s2 <= SIZE_MAX
 * if both s1 < MUL_NO_OVERFLOW and s2 < MUL_NO_OVERFLOW
 */
#define MUL_NO_OVERFLOW	((size_t)1 << (sizeof(size_t) * 4))

inline void *
reallocarray(void *optr, size_t nmemb, size_t size)
{
	if ((nmemb >= MUL_NO_OVERFLOW || size >= MUL_NO_OVERFLOW) &&
	    nmemb > 0 && SIZE_MAX / nmemb < size) {
		errno = ENOMEM;
		return NULL;
	}
	return realloc(optr, size * nmemb);
}

