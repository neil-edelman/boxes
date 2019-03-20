/** 2019 Neil Edelman, distributed under the terms of the MIT License;
 see readme.txt, or \url{ https://opensource.org/licenses/MIT }.

 @file Timing
 @author Neil
 @std C89/90
 @version 2019-03
 @since 2019-03 */

#include <stdlib.h> /* EXIT malloc free srand */
#include <stdio.h>  /* fprintf */
#include <assert.h> /* assert */
#include <time.h>   /* clock time */
#include "../src/Time.h"

static void time_fn(const char f, const TimeFn fn, const size_t replicas) {
	clock_t t = clock();
	assert(fn && replicas);
	fn(replicas);
	printf("%c\t%lu\n", f, (unsigned long)(clock() - t));
}

int main(void) {
	unsigned seed = (unsigned)clock();
	const size_t replicas = 500000;

	srand(seed), rand(), fprintf(stderr, "Seed %u.\n", seed);

	time_fn('s', &TestStatic, replicas);
	time_fn('m', &TestAlloc, replicas);
	time_fn('p', &TestPool, replicas);
	time_fn('a', &TestArray, replicas);
	time_fn('f', &TestFreeArray, replicas);

	return EXIT_SUCCESS;
}
