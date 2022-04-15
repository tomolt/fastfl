#define _POSIX_C_SOURCE 200112L
#define DH_IMPLEMENT_HERE 1
#include "dh_cuts.h"

int
main()
{
	dh_init(stderr);
	dh_summarize();
	return 0;
}

