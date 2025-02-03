#ifdef DEQUE_NON_STATIC
void T_(test)(void);
#endif
#ifndef DEQUE_DECLARE_ONLY

#	if defined QUOTE || defined QUOTE_
#		error Cannot be defined.
#	endif
#	define QUOTE_(name) #name
#	define QUOTE(name) QUOTE_(name)

/** @return Is `a` in a valid state? */
static void pT_(valid_state)(const struct t_(deque) *const deque) {
	struct pT_(block) *block;
	if(!deque) return;
	for(block = deque->back; block; block = block->previous) {
		/* Power of two. */
		assert(block->size <= block->capacity
			&& block->capacity && !(block->capacity & (block->capacity - 1)));
	}
}

static void pT_(test_basic)(void) {
	struct t_(deque) deque = t_(deque)();
	pT_(type) items[1000], *item, *item1;
	const size_t items_size = sizeof items / sizeof *items;
	struct t_(deque_cursor) cur;
	size_t i;

	assert(errno == 0);
	pT_(valid_state)(0);

	printf("Test empty.\n");
	assert(errno == 0);
	pT_(valid_state)(&deque);
	T_(graph_fn)(&deque, "graph/deque/" QUOTE(DEQUE_NAME) "-empty.gv");

	/* This is un-necessary, but `valgrind` reports an error if we don't. */
	memset(items, 0, sizeof items);
	/* Get elements. */
	for(item = items, item1 = item + items_size; item < item1; item++)
		t_(filler)(item);

	printf("Test one element.\n");
	if(!(item = T_(new_back)(&deque))) { assert(0); goto catch; }
	memcpy(item, items + 0, sizeof *item);
	T_(graph_fn)(&deque, "graph/deque/" QUOTE(DEQUE_NAME) "-one.gv");

	printf("Test twenty elements.\n");
	if(!(item = T_(append_back)(&deque, 19))) { assert(0); goto catch; }
	memcpy(item, items + 1, sizeof *item * 19);
	T_(graph_fn)(&deque, "graph/deque/" QUOTE(DEQUE_NAME) "-twenty.gv");

	printf("Test forty elements.\n");
	if(!(item = T_(append_back)(&deque, 20))) { assert(0); goto catch; }
	memcpy(item, items + 20, sizeof *item * 20);
	T_(graph_fn)(&deque, "graph/deque/" QUOTE(DEQUE_NAME) "-forty.gv");

	printf("Test sixty elements.\n");
	if(!(item = T_(append_back)(&deque, 20))) { assert(0); goto catch; }
	memcpy(item, items + 40, sizeof *item * 20);
	T_(graph_fn)(&deque, "graph/deque/" QUOTE(DEQUE_NAME) "-sixty.gv");

	pT_(valid_state)(&deque);
	/*printf("%.60s\n", items);*/
	for(i = 60, cur = T_(end)(&deque); T_(exists)(&cur); T_(previous)(&cur)) {
		char a[12], b[12];
		assert(i);
		i--;
		t_(to_string)(T_(entry)(&cur), &a);
		t_(to_string)(items + i, &b);
		printf("deque: %s, array: %s\n", a, b);
		assert(!memcmp(items + i, T_(entry)(&cur), sizeof *item1));
	}
	assert(i == 0);

	T_(clear)(&deque);
	T_(graph_fn)(&deque, "graph/deque/" QUOTE(DEQUE_NAME) "-clear.gv");
	pT_(valid_state)(&deque);

	printf("Test %lu elements.\n", (unsigned long)items_size);
	for(i = 0; i < items_size; i++) {
		if(!(item = T_(new_back)(&deque))) { assert(0); goto catch; }
		memcpy(item, items + i, sizeof *item);
	}
	T_(graph_fn)(&deque, "graph/deque/" QUOTE(DEQUE_NAME) "-all.gv");
	pT_(valid_state)(&deque);
	goto finally;
catch:
	perror("This should not happen.");
	assert(0);
finally:
	printf("Destructor:\n");
	t_(deque_)(&deque);
	pT_(valid_state)(&deque);
}

#	define BOX_PUBLIC_OVERRIDE
#	include "../src/box.h"

/** Will be tested on stdout. @allow */
static void T_(test)(void) {
	printf("<" QUOTE(DEQUE_NAME) ">deque of type <" QUOTE(DEQUE_TYPE)
		"> was created using: DEQUE_TO_STRING; DEQUE_TEST; testing:\n");
	pT_(test_basic)();
	fprintf(stderr, "Done tests of <" QUOTE(DEQUE_NAME) ">deque.\n\n");
}

#	define BOX_PRIVATE_AGAIN
#	include "../src/box.h"

#	undef QUOTE
#	undef QUOTE_
#endif
