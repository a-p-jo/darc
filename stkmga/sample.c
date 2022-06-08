#include <stdio.h>    /* printf(), fputs(), stderr          */
#include <time.h>     /* clock_t, clock(), CLOCKS_PER_SEC   */
#include <stdlib.h>   /* EXIT_SUCCESS, EXIT_FAILURE         */
#include <inttypes.h> /* strtoumax()                        */
#include <errno.h>    /* errno, ERANGE                      */

#include "stkvec.h"
STKVEC_DECL(myvec, size_t)

enum {LOAD_FACTOR = 311000};

int main(int argc, char **argv)
{
	size_t load;
	
	/* Get load value from command line arguments */
	if (argc < 2 || !(load = strtoumax(argv[1], NULL, 0)) || errno == ERANGE || SIZE_MAX/LOAD_FACTOR < load) {
		fputs("Error : Abset/invalid load value.\n", stderr);
		return EXIT_FAILURE;
	}

	load *= LOAD_FACTOR;
	myvec x = STKVEC_CREATE(myvec, 0);
	clock_t begin = clock();
	for(size_t i = 0; i < load; i++)
		STKVEC_INSERT(myvec, x, x.len, &i, 1, (int){0});

	long double mili_seconds = (long double)(clock() - begin) / CLOCKS_PER_SEC*1000;
	printf("It took %.3Lf ms for %zu iterations.\n", mili_seconds, load);

	return EXIT_SUCCESS;
}	
