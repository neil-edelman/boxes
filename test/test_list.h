#ifdef LIST_NON_STATIC
void T_(test)(void);
#endif
#ifndef LIST_DECLARE_ONLY

#	if defined QUOTE || defined QUOTE_
#		error Cannot be defined.
#	endif
#	define QUOTE_(name) #name
#	define QUOTE(name) QUOTE_(name)

#	include <stdlib.h>	/* EXIT rand */
#	include <stdio.h>  /* printf */

/** Perform "Floyd's" tortoise-hare algorithm for cycle detection for the list
 on which `link` is a part and expect `count`. `list` must have at least one
 element, (it can't be the head of tail.)
 @order \O(|`list`|) */
static void pT_(floyd)(const struct t_(listlink) *link, const size_t count) {
	size_t fw = 0, b1 = 0, b2 = 0;
	const struct t_(listlink) *hare = link, *turtle = hare;
	assert(link && link->next && link->prev);
	while(hare->prev->prev) {
		hare = hare->prev;
		if(b1++ & 1) turtle = turtle->prev;
		assert(turtle != hare);
	}
	turtle = hare;
	while(hare->next) {
		hare = hare->next;
		if(fw++ & 1) turtle = turtle->next;
		assert(turtle != hare);
	}
	turtle = hare;
	while(hare != link) {
		hare = hare->prev;
		if(b2++ & 1) turtle = turtle->prev;
		assert(hare && turtle != hare);
	}
	assert(fw == b1 + b2 && fw == count);
}
/** Debug: ensures that `list` has no cycles and that it has `count`
 elements. */
static void pT_(assert_count)(const struct t_(list) *const list,
	const size_t count) {
	const struct t_(listlink) *const head = &list->u.as_head.head,
		*const tail = &list->u.as_tail.tail, *first;
	assert(list && head && tail && !list->u.flat.zero);
	first = head->next, assert(first);
	if(first == tail) {
		assert(tail->prev == head && !count);
	} else {
		pT_(floyd)(first, count);
	}
}

#	ifdef HAS_ITERATE_H /* <!-- */
/** Returns `0,1,0,1,...` whatever `link`. */
static int pT_(parity)(const struct t_(listlink) *const link) {
	static int p;
	(void)(link);
	return !(p = !p);
}
/** Returns true whatever `link`. */
static int pT_(true)(const struct t_(listlink) *const link) {
	(void)(link);
	return 1;
}
#	endif /* --> */

/** Passed `parent_new` and `parent`, tests basic functionality. */
static void pT_(test_basic)(struct t_(listlink) *(*const parent_new)(void *),
	void *const parent) {
	struct t_(list) l1, l2;
	struct t_(listlink) *link, *link_first = 0, *link_last = 0;
	const size_t test_size = 10;
	size_t i;
	assert(parent_new && parent);
	assert(!errno);
	T_(clear)(&l1), T_(clear)(&l2);
	printf("Basic tests of <" QUOTE(LIST_NAME) ">list:\n");
	pT_(assert_count)(&l1, 0);
	/* Test positions null. */
	link = T_(head)(&l1), assert(!link);
	link = T_(tail)(&l1), assert(!link);
	/* Test returns on null and empty. */
	link = T_(shift)(&l1), assert(!link);
	link = T_(pop)(&l1), assert(!link);
	assert(!errno);
	/* Add */
	printf("Adding %lu elements to l1.\n", (unsigned long)test_size);
	for(i = 0; i < test_size; i++) {
		char z[12];
		if(!(link = parent_new(parent))) { assert(0); return; }
		/* Must have these functions defined. */
		t_(filler)(link);
		t_(to_string)(link, &z);
		printf("Adding %s.\n", z);
		T_(push)(&l1, link);
		if(i == 0) link_first = link;
		link_last = link;
		assert(!errno);
	}
	T_(graph_fn)(&l1, "graph/list/" QUOTE(LIST_NAME) "-small.gv");
	assert(!errno);
	pT_(assert_count)(&l1, test_size);
	printf("l1 = %s.\n", T_(to_string)(&l1));
	/* Test positions when contents. */
	link = T_(head)(&l1), assert(link == link_first);
	link = T_(tail)(&l1), assert(link == link_last);
	link = T_(link_previous)(link), assert(link);
	link = T_(link_next)(link), assert(link == link_last);
	assert(!errno);
	/* Test remove contents. */
	link = T_(shift)(&l1), assert(link == link_first);
	link = T_(pop)(&l1), assert(link = link_last);
	pT_(assert_count)(&l1, test_size - 2);
	T_(unshift)(&l1, link_first);
	T_(push)(&l1, link_last);
	pT_(assert_count)(&l1, test_size);
	link = T_(head)(&l1), assert(link == link_first);
	link = T_(tail)(&l1), assert(link == link_last);
	printf("After removing and adding: l1 = %s.\n", T_(to_string)(&l1));
	assert(!errno);
#	ifdef HAS_ITERATE_H /* <!-- iterator */
	assert(l2.u.as_head.head.next);
	/* Test movement. */
	pT_(assert_count)(&l1, test_size);
	pT_(assert_count)(&l2, 0);
	T_(to_if)(&l1, &l2, &pT_(parity));
	printf("Transferring all odds: l1 = %s; l2 = %s.\n",
		T_(to_string)(&l1), T_(to_string)(&l2));
	pT_(assert_count)(&l1, test_size / 2);
	pT_(assert_count)(&l2, test_size - test_size / 2);
	assert(T_(head)(&l1) == link_first);
	assert(l2.u.as_head.head.next);
	printf("l1 = %s; l2 = %s.\n",
		T_(to_string)(&l1), T_(to_string)(&l2));
	T_(to_before)(&l2, link_first->next);
	printf("l1 = %s; l2 = %s.\n",
		T_(to_string)(&l1), T_(to_string)(&l2));
	assert(l2.u.as_head.head.next);
	pT_(assert_count)(&l1, test_size);
	assert(l2.u.as_head.head.next);
	pT_(assert_count)(&l2, 0);
	assert(T_(head)(&l1) == link_first);
	printf("Back: l1 = %s; l2 = %s.\n",
		T_(to_string)(&l1), T_(to_string)(&l2));
	T_(to)(&l1, &l2);
	pT_(assert_count)(&l1, 0);
	pT_(assert_count)(&l2, test_size);
	assert(T_(head)(&l2) == link_first);
	printf("***HERE***\n");
	/* Test any. */
	pT_(assert_count)(&l1, 0);
	link = T_(any)(&l1, &pT_(true)), assert(!link);
	link = T_(any)(&l2, &pT_(true)), assert(link == link_first);
	/* Test add before/after. */
	if(!(link = parent_new(parent))) { assert(0); return; }
	t_(filler)(link);
	T_(add_before)(T_(head)(&l2), link);
	link_first = T_(head)(&l2);
	assert(link == link_first);
	pT_(assert_count)(&l2, test_size + 1);
	if(!(link = parent_new(parent))) { assert(0); return; }
	t_(filler)(link);
	T_(add_before)(T_(tail)(&l2), link);
	pT_(assert_count)(&l2, test_size + 2);
#	endif /* iterator --> */
	T_(clear)(&l2);
	pT_(assert_count)(&l2, 0);
	assert(!errno);
}

#	define BOX_PUBLIC_OVERRIDE
#	include "../src/box.h"
/** The linked-list will be tested on stdout. `LIST_TEST` has to be set.
 @param[parent_new, parent] Responsible for creating new objects and returning
 the list. @allow */
static void T_(test)(struct t_(listlink) *(*const parent_new)(void *),
	void *const parent) {
	printf("<" QUOTE(LIST_NAME) ">list was created using: "
#	ifdef LIST_COMPARE
		"LIST_COMPARE; "
#	endif
		"testing:\n");
	pT_(test_basic)(parent_new, parent);
	printf("Done tests of " QUOTE(LIST_NAME) ".\n\n");
}
#	define BOX_PRIVATE_AGAIN
#	include "../src/box.h"

#	undef QUOTE
#	undef QUOTE_
#endif
