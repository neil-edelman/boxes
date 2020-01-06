/** @license 2020 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 Test Heap.

 @std C89/90 */

#include <stdlib.h> /* EXIT malloc free rand */
#include <stdio.h>  /* *printf */
/*#include <assert.h>*/ /* assert */
#include <errno.h>  /* errno */

struct IntHeapNode;
static void int_to_string(const struct IntHeapNode *, char (*)[12]);
static void test_int(struct IntHeapNode *);

#define HEAP_NAME Int
#define HEAP_TO_STRING &int_to_string
#define HEAP_TEST &test_int
#include "../src/Heap.h"

static void int_to_string(const struct IntHeapNode *const i,
	char (*const a)[12]) {
	sprintf(*a, "%u", i->priority);
}

static void test_int(struct IntHeapNode *i) {
	i->priority = rand() / (RAND_MAX / 99 + 1) + 1;
	printf("int: %u\n", i->priority);
}

int main(void) {
	rand();
	IntHeapTest();
	return EXIT_SUCCESS;
}
