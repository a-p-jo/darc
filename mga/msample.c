#include <stdio.h>    /* printf(), fputs(), stderr          */
#include <time.h>     /* clock_t, clock(), CLOCKS_PER_SEC   */
#include <stdlib.h>   /* EXIT_SUCCESS, EXIT_FAILURE         */
#include <inttypes.h> /* strtoumax()                        */
#include <errno.h>    /* errno, ERANGE                      */

#include "mga.h"
MGA_IMPL(myvec, size_t)

enum {LOAD_FACTOR = 1000*1000};

static inline bool myvec_push(myvec *dst, size_t val) 
{
	return myvec_insert(dst, dst->len, &val, 1);
}

int main(int argc, char **argv)
{
	size_t load;
	
	/* Get load value from command line arguments */
	if (argc < 2 || !(load = strtoumax(argv[1], NULL, 0)) || errno == ERANGE || SIZE_MAX/LOAD_FACTOR < load) {
		fputs("Error : Abset/invalid load value.\n", stderr);
		return EXIT_FAILURE;
	}

	load *= LOAD_FACTOR;
	myvec x = myvec_create(0, realloc, free);
	clock_t begin = clock();
	for(size_t i = 0; i < load; i++)
		myvec_push(&x, i);
	
	long double mili_seconds = (long double)(clock() - begin) / CLOCKS_PER_SEC*1000;
	printf("It took %.3Lf ms for %zu iterations.\n", mili_seconds, load);

	myvec_destroy(&x);
	return EXIT_SUCCESS;
}	
