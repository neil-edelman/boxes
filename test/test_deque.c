/** @std C89/90 */

#include "../src/orcish.h"
#include <stdlib.h> /* EXIT_ */
#include <stdio.h>  /* printf */
#include <string.h> /* strcmp */
#include <time.h>   /* clock */
#include <assert.h> /* assert */


/* Not used because it's not set up for testing. The minimal array. */
#define DEQUE_NAME number
#define DEQUE_TYPE int
#include "../src/deque.h"


/* `char` */
#if 0
#define DEQUE_NAME char
#define DEQUE_TYPE char
#define DEQUE_TEST
#define DEQUE_TO_STRING
#include "../src/deque.h"
#endif

#if 0
/* Struct array. */
struct example { int key; char value[4]; };
static void example_to_string(const struct example *const e, char (*const a)[12])
	{ sprintf(*a, "%.8d%.3s", e->key, e->value); }
static void str4_filler(struct str4 *const s)
	{ orcish(s->value, sizeof s->value); }
#define DEQUE_NAME str4
#define DEQUE_TYPE struct str4
#define DEQUE_TEST
#define DEQUE_TO_STRING
#include "../src/array.h"


/* Including this file will make other files see it. */
static void header_to_string(const int *i, char (*const a)[12])
	{ int_to_string(i, a); }
static void header_filler(int *const i) { int_filler(i); }
static int header_compare(const int *const a, const int *const b)
	{ return int_compare(a, b); }
#define DEFINE /* Invert meaning for this compilation unit. */
#include "header_array.h"
#endif


/** Tests; assert crashes on failed test. @return `EXIT_SUCCESS`. */
int main(void) {
	unsigned seed = (unsigned)clock();

	srand(seed), rand(), printf("Seed %u.\n", seed);
	errno = 0;

	printf("Test success.\n\n");

	return EXIT_SUCCESS;
}
