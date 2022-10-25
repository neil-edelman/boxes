#if defined(QUOTE) || defined(QUOTE_)
#error QUOTE_? cannot be defined.
#endif
#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)

#include <stdlib.h>	/* EXIT rand */
#include <stdio.h>  /* printf */

#ifdef LIST_IS_EQUAL
#error Not implemented in testing.
#endif

static int PCMP_(compar)(const void *const a, const void *const b)
	{ return CMP_(compare)(a, b); }

/** Passed `parent_new` and `parent`, tests sort and meta-sort. */
static void PL_(test_sort)(struct L_(listlink) *(*const parent_new)(void *),
	void *const parent) {
	struct L_(list) lists[64], *list;
	const size_t lists_size = sizeof lists / sizeof *lists;
	struct L_(list) *const lists_end = lists + lists_size;
	int cmp;
	{ /* Just one, so we can be sure <fn:<L>list_self_correct> works. */
		struct L_(list) eg1, eg2;
		struct L_(listlink) *link;
		char z[12];
		L_(list_clear)(&eg1), L_(list_clear)(&eg2);
		if(!(link = parent_new(parent))) { assert(0); return; }
		L_(filler)(link);
		L_(to_string)(link, &z);
		printf("link: %s.\n", z);
		printf("empty eg1: %s, as_head %s, as_tail %s.\n", L_(list_to_string)(&eg1), orcify(&eg1.u.as_head.head),
			orcify(&eg1.u.as_tail.tail));
		L_(list_self_correct)(&eg1);
		printf("empty eg1: %s, as_head %s, as_tail %s.\n", L_(list_to_string)(&eg1), orcify(&eg1.u.as_head.head),
			orcify(&eg1.u.as_tail.tail));
		printf("empty eg2: %s, as_head %s, as_tail %s.\n", L_(list_to_string)(&eg2), orcify(&eg2.u.as_head.head),
			orcify(&eg2.u.as_tail.tail));
		L_(list_push)(&eg1, link);
		printf("link in eg1: %s, next %s, prev %s, next.next %s.\n",
			L_(list_to_string)(&eg1), orcify(eg1.u.flat.next),
			orcify(eg1.u.flat.prev), orcify(eg1.u.flat.next->next));
		/* Intentionally add it to another list! */
		L_(list_push)(&eg2, link);
		printf("link in eg2: %s, next %p, prev %p, next.next %p.\n",
			L_(list_to_string)(&eg2), (void *)eg2.u.flat.next,
			(void *)eg2.u.flat.prev, (void *)eg2.u.flat.next->next);
		/* Correct back to `eg1`, (`eg2` will now be invalid.) */
		L_(list_self_correct)(&eg1);
		printf("link in eg1: %s, next %p, prev %p, next.next %p.\n",
			L_(list_to_string)(&eg1), (void *)eg1.u.flat.next,
			(void *)eg1.u.flat.prev, (void *)eg1.u.flat.next->next);
	}
	/* Random lists. */
	for(list = lists; list < lists_end; list++) {
		size_t no_links = (unsigned)rand() / (RAND_MAX / 5 + 1);
		struct L_(listlink) *link, *link_a, *link_b;
		L_(list_clear)(list);
		while(no_links) {
			if(!(link = parent_new(parent))) { assert(0); return; }
			L_(filler)(link);
			L_(list_push)(list, link);
			no_links--;
		}
		L_(list_sort)(list);
		for(link_a = 0, link_b = L_(list_head)(list); link_b;
			link_a = link_b, link_b = L_(list_next)(link_b)) {
			if(!link_a) continue;
			cmp = CMPEXTERN_(compare)(link_a, link_b);
			assert(cmp <= 0);
		}
	}
	/* Now sort the lists. */
	printf("Sort array of sorted <" QUOTE(LIST_NAME) ">list by "
		QUOTE(LIST_COMPARE) ":\n");
	qsort(lists, lists_size, sizeof *lists, &PCMP_(compar));
	for(list = lists; list < lists_end; list++) {
		L_(list_self_correct)(list); /* `qsort` moves the pointers. */
		printf("list: %s.\n", L_(list_to_string)(list));
		if(list == lists) continue;
		cmp = L_(list_compare)(list - 1, list);
		assert(cmp <= 0);
	}
}

/** Set up the incredibly contrived example involving `la`, `lb`, `result`, and
 `a`, `b`, `b_alt`, `c`, `d` for <fn:<PL>test_binary>, where `a = ()`,
 `b = (A,B,D)`, and `c = (B,C)`. */
static void PL_(reset_b)(struct L_(list) *const la, struct L_(list) *const lb,
	struct L_(list) *const result, struct L_(listlink) *const a,
	struct L_(listlink) *const b, struct L_(listlink) *const b_alt,
	struct L_(listlink) *const c, struct L_(listlink) *const d) {
	assert(la && lb && result && a && b && b_alt && c && d);
	L_(list_clear)(la), L_(list_clear)(lb), L_(list_clear)(result);
	L_(list_push)(la, a), L_(list_push)(la, b), L_(list_push)(la, d);
	L_(list_push)(lb, b_alt), L_(list_push)(lb, c);
}
/** Verifies that `list` is `a`, `b`, `c`, `d`, null. */
static void PL_(exact)(struct L_(list) *const list,
	const struct L_(listlink) *const a, const struct L_(listlink) *const b,
	const struct L_(listlink) *const c, const struct L_(listlink) *const d) {
	struct L_(listlink) *i;
	assert(list);
	i = L_(list_head)(list), assert(i == a);
	if(!i) return;
	i = L_(list_next)(i), assert(i == b);
	if(!i) return;
	i = L_(list_next)(i), assert(i == c);
	if(!i) return;
	i = L_(list_next)(i), assert(i == d);
	if(!i) return;
	i = L_(list_next)(i), assert(!i);
}

/** Passed `parent_new` and `parent`, tests binary operations. */
static void PL_(test_binary)(struct L_(listlink) *(*const parent_new)(void *),
	void *const parent) {
	struct L_(list) la, lb, result;
	struct L_(listlink) *link, *a = 0, *b = 0, *b_alt = 0, *c = 0, *d = 0;
	int cmp;
	L_(list_clear)(&la);
	/* Test nulls, (Not comprehensive.) */
	cmp = L_(list_compare)(0, 0), assert(cmp == 0);
	cmp = L_(list_compare)(&la, 0), assert(cmp == 0);
	cmp = L_(list_compare)(0, &la), assert(cmp == 0);
	L_(list_subtraction_to)(0, 0, 0);
	L_(list_subtraction_to)(0, 0, &la);
	L_(list_union_to)(0, 0, 0);
	L_(list_union_to)(0, 0, &la);
	L_(list_intersection_to)(0, 0, 0);
	L_(list_intersection_to)(0, 0, &la);
	L_(list_xor_to)(0, 0, 0);
	L_(list_xor_to)(0, 0, &la);
	assert(!L_(list_head)(&la));
	{
		const size_t no_try = 5000;
		struct L_(list) x, y;
		size_t i;
		L_(list_clear)(&x), L_(list_clear)(&y);
		/* By the PHP, this should be more than enough to get at least the
		 small-entropy ones, (_ie_ `Letter`.) */
		for(i = 0; i < no_try; i++) {
			if(!(link = parent_new(parent))) { assert(0); return; }
			L_(filler)(link);
			L_(list_push)(&x, link);
			L_(list_sort)(&x);
			L_(list_duplicates_to)(&x, &y);
			/* fixme: list_duplicates_to is suspect? it keeps giving wrong. */
			printf("x = %s, y = %s\n", L_(list_to_string)(&x),
				L_(list_to_string)(&y));
			/* Honestly, what am I doing? */
			/* `x = (A,...,B,C,D,...)` and `y = {[A],B,...}`. */
			if(!(a = L_(list_head)(&x))) continue;
			if(!(b = L_(list_head)(&y))) continue;
			if(CMPEXTERN_(compare)(a, b) == 0 && !(b = L_(list_next)(b)))
				continue;
			assert(CMPEXTERN_(compare)(a, b) < 0);
			for(c = L_(list_next)(a); c && CMPEXTERN_(compare)(c, b) < 0;
				c = L_(list_next)(c));
			assert(c && CMPEXTERN_(compare)(c, b) == 0);
			b_alt = c;
			if(!(c = L_(list_next)(c)) || !(d = L_(list_next)(c))) continue;
			break;
		}
		if(i == no_try) {
			printf("Couldn't get duplicates from " QUOTE(LIST_NAME)
				" in %lu tries; giving up.\n", (unsigned long)no_try);
			return;
		} else {
			printf("Got duplicates in %lu tries.\n", (unsigned long)i);
		}
	}
	PL_(reset_b)(&la, &lb, &result, a, b, b_alt, c, d);
	printf("a = (A,B,D) = %s, b = (B,C) = %s, result = %s.\n",
		L_(list_to_string)(&la), L_(list_to_string)(&lb),
		L_(list_to_string)(&result));
	L_(list_subtraction_to)(&la, &lb, &result);
	printf("a - b = %s.\n", L_(list_to_string)(&result));
	PL_(exact)(&la, b, 0, 0, 0);
	PL_(exact)(&lb, b_alt, c, 0, 0);
	PL_(exact)(&result, a, d, 0, 0);

	PL_(reset_b)(&la, &lb, &result, a, b, b_alt, c, d);
	L_(list_union_to)(&la, &lb, &result);
	printf("a \\cup b = %s.\n", L_(list_to_string)(&result));
	PL_(exact)(&la, 0, 0, 0, 0);
	PL_(exact)(&lb, b_alt, 0, 0, 0);
	PL_(exact)(&result, a, b, c, d);

	PL_(reset_b)(&la, &lb, &result, a, b, b_alt, c, d);
	L_(list_intersection_to)(&la, &lb, &result);
	printf("a \\cap b = %s.\n", L_(list_to_string)(&result));
	PL_(exact)(&la, a, d, 0, 0);
	PL_(exact)(&lb, b_alt, c, 0, 0);
	PL_(exact)(&result, b, 0, 0, 0);

	PL_(reset_b)(&la, &lb, &result, a, b, b_alt, c, d);
	L_(list_xor_to)(&la, &lb, &result);
	printf("a \\xor b = %s.\n", L_(list_to_string)(&result));
	PL_(exact)(&la, b, 0, 0, 0);
	PL_(exact)(&lb, b_alt, 0, 0, 0);
	PL_(exact)(&result, a, c, d, 0);
}

/** The linked-list will be tested on stdout. `LIST_TEST` has to be set.
 @param[parent_new, parent] Responsible for creating new objects and returning
 the list. @allow */
static void CMP_(compare_test)(struct L_(listlink)
	*(*const parent_new)(void *), void *const parent) {
	printf("<" QUOTE(LIST_NAME) ">list testing <"
#ifdef LIST_COMPARE_NAME
		QUOTE(LIST_COMPARE_NAME)
#else
		"anonymous"
#endif
		"> "
#ifdef LIST_COMPARE
		"compare <" QUOTE(LIST_COMPARE)
#elif defined(LIST_IS_EQUAL)
		"is equal <" QUOTE(LIST_IS_EQUAL)
#endif
		">:\n");
	PL_(test_sort)(parent_new, parent);
	PL_(test_binary)(parent_new, parent);
	printf("Done tests of " QUOTE(LIST_NAME) ".\n\n");
}

#undef QUOTE
#undef QUOTE_
