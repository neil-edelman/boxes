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


static void char_to_string(const char *const data, char (*const a)[12])
	{ (*a)[0] = *data, (*a)[1] = '\0'; }
static void char_filler(char *const c) {
	const char *const choose
		= "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	*c = choose[(unsigned)rand() / (RAND_MAX / sizeof choose)];
}
/* `char` */
#define DEQUE_NAME char
#define DEQUE_TYPE char
#define DEQUE_TEST
#define DEQUE_TO_STRING
#include "../src/deque.h"


/* Struct array. */
struct example { int key; char value[4]; };
static void example_to_string(const struct example *const e, char (*const a)[12])
	{ sprintf(*a, "%d%s", e->key, e->value); }
static void example_filler(struct example *const e) {
	e->key = rand() % 10000000; /* Non-uniform distribution. Meh. */
	orcish(e->value, sizeof e->value);
}
#define DEQUE_NAME example
#define DEQUE_TYPE struct example
/*#define DEQUE_PUSH_FRONT Rightfully gives an error. */
#define DEQUE_FRONT
#define DEQUE_TEST
#define DEQUE_TO_STRING
#include "../src/deque.h"

/** Tests front. */
static void example_deque_manual_test(void) {
	struct example_deque es = example_deque();
	struct example examples[5000];
	struct example_deque_cursor cur;
	size_t i;
	for(i = 0; i < sizeof examples / sizeof *examples; i++)
		example_filler(examples + i), examples[i].key = (int)i;
	for(i = 0; i < sizeof examples / sizeof *examples; i++) {
		struct example *const e = example_deque_new_back(&es);
		if(!e) goto catch;
		memcpy(e, examples + i, sizeof *e);
	}
	example_deque_graph_fn(&es, "graph/deque/example-manual.gv");
	i = sizeof examples / sizeof *examples;
	fprintf(stderr, "There are %lu examples.\n", (unsigned long)i);
	for(cur = example_deque_end(&es); example_deque_exists(&cur);
		example_deque_previous(&cur)) {
		struct example *const e = example_deque_entry(&cur);
		i--;
		assert(e->key == (int)i);
	}
	assert(i == 0);
	for(cur = example_deque_begin(&es); example_deque_exists(&cur);
		example_deque_next(&cur)) {
		struct example *const e = example_deque_entry(&cur);
		assert(e->key == (int)i);
		i++;
	}
	example_deque_clear(&es);
	example_deque_graph_fn(&es, "graph/deque/example-clear-2.gv");
	goto finally;
catch:
	perror("example");
	assert(0);
finally:
	example_deque_(&es);
}


/* Including this file will make other files see it. */
static void header_to_string(const int *i, char (*const a)[12])
	{ sprintf(*a, "%d", *i); }
static void header_filler(int *const i) { *i = rand() % 10000000; }
#define DEFINE /* Invert meaning for this compilation unit. */
#include "header_deque.h"


/** Tests; assert crashes on failed test. @return `EXIT_SUCCESS`. */
int main(void) {
	unsigned seed = (unsigned)clock();

	srand(seed), rand(), printf("Seed %u.\n", seed);
	errno = 0;

	example_deque_manual_test();
	char_deque_test();
	example_deque_test();
	header_deque_test();

	printf("Test success.\n\n");

	return EXIT_SUCCESS;
}
