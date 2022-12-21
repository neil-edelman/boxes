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

static void time_fn(const char *const f, const TimeFn fn,
	const size_t replicas) {
	clock_t t = clock();
	assert(f && fn && replicas);
	fn(replicas);
	printf("%s\t%lu\n", f, (unsigned long)(clock() - t));
}

int main(void) {
	unsigned seed = (unsigned)clock();
	const size_t replicas = 1000;

	srand(seed), rand();

	fprintf(stderr, "Seed %u. Replicas %lu. %s\n",
		seed, (unsigned long)replicas,
#if 0
			"Random create/destroy."
#elif 1
			"Random create/destroy + visit @."
#elif 1
			"Random create/destroy + recreate."
#endif
			);

	time_fn("Static Contiguo", &TestStatic, replicas);
	time_fn("Malloc Random", &TestAlloc, replicas);
	time_fn("Stable Pool", &TestPool, replicas);
	time_fn("Dynamic Array", &TestArray, replicas);
	time_fn("Dynamic Contigu", &TestConArray, replicas);
	time_fn("Dynamic Free-Li", &TestFreeArray, replicas);

	return EXIT_SUCCESS;
}
