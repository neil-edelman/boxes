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

static int pTR_(compar)(const void *const a, const void *const b)
	{ return TR_(compare)(a, b); }

/** Passed `parent_new` and `parent`, tests sort and meta-sort. */
static void pT_(test_sort)(struct t_(listlink) *(*const parent_new)(void *),
	void *const parent) {
	struct t_(list) lists[64], *list;
	const size_t lists_size = sizeof lists / sizeof *lists;
	struct t_(list) *const lists_end = lists + lists_size;
	int cmp;
	{ /* Just one, so we can be sure <fn:<L>list_self_correct> works. */
		struct t_(list) eg1, eg2;
		struct t_(listlink) *link;
		char z[12];
		T_(clear)(&eg1), T_(clear)(&eg2);
		if(!(link = parent_new(parent))) { assert(0); return; }
		t_(filler)(link);
		t_(to_string)(link, &z);
		printf("link: %s.\n", z);
		printf("empty eg1: %s, as_head %s, as_tail %s.\n", T_(to_string)(&eg1), orcify(&eg1.u.as_head.head),
			orcify(&eg1.u.as_tail.tail));
		T_(self_correct)(&eg1);
		printf("empty eg1: %s, as_head %s, as_tail %s.\n", T_(to_string)(&eg1), orcify(&eg1.u.as_head.head),
			orcify(&eg1.u.as_tail.tail));
		printf("empty eg2: %s, as_head %s, as_tail %s.\n", T_(to_string)(&eg2), orcify(&eg2.u.as_head.head),
			orcify(&eg2.u.as_tail.tail));
		T_(push)(&eg1, link);
		printf("link in eg1: %s, next %s, prev %s, next.next %s.\n",
			T_(to_string)(&eg1), orcify(eg1.u.flat.next),
			orcify(eg1.u.flat.prev), orcify(eg1.u.flat.next->next));
		/* Intentionally add it to another list! */
		T_(push)(&eg2, link);
		printf("link in eg2: %s, next %p, prev %p, next.next %p.\n",
			T_(to_string)(&eg2), (void *)eg2.u.flat.next,
			(void *)eg2.u.flat.prev, (void *)eg2.u.flat.next->next);
		/* Correct back to `eg1`, (`eg2` will now be invalid.) */
		T_(self_correct)(&eg1);
		printf("link in eg1: %s, next %p, prev %p, next.next %p.\n",
			T_(to_string)(&eg1), (void *)eg1.u.flat.next,
			(void *)eg1.u.flat.prev, (void *)eg1.u.flat.next->next);
	}
	/* Random lists. */
	for(list = lists; list < lists_end; list++) {
		size_t no_links = (unsigned)rand() / (RAND_MAX / 5 + 1);
		struct t_(listlink) *link, *link_a, *link_b;
		T_(clear)(list);
		while(no_links) {
			if(!(link = parent_new(parent))) { assert(0); return; }
			t_(filler)(link);
			T_(push)(list, link);
			no_links--;
		}
		T_(sort)(list);
		for(link_a = 0, link_b = T_(head)(list); link_b;
			link_a = link_b, link_b = T_(link_next)(link_b)) {
			if(!link_a) continue;
			cmp = tr_(compare)(link_a, link_b);
			assert(cmp <= 0);
		}
	}
	/* Now sort the lists. */
	printf("Sort array of sorted <" QUOTE(LIST_NAME) ">list by "
		QUOTE(LIST_COMPARE) ":\n");
	qsort(lists, lists_size, sizeof *lists, &pTR_(compar));
	for(list = lists; list < lists_end; list++) {
		T_(self_correct)(list); /* `qsort` moves the pointers. */
		printf("list: %s.\n", T_(to_string)(list));
		if(list == lists) continue;
		cmp = T_(compare)(list - 1, list);
		assert(cmp <= 0);
	}
}

/** Set up the incredibly contrived example involving `la`, `lb`, `result`, and
 `a`, `b`, `b_alt`, `c`, `d` for <fn:<PL>test_binary>, where `a = ()`,
 `b = (A,B,D)`, and `c = (B,C)`. */
static void pT_(reset_b)(struct t_(list) *const la, struct t_(list) *const lb,
	struct t_(list) *const result, struct t_(listlink) *const a,
	struct t_(listlink) *const b, struct t_(listlink) *const b_alt,
	struct t_(listlink) *const c, struct t_(listlink) *const d) {
	assert(la && lb && result && a && b && b_alt && c && d);
	T_(clear)(la), T_(clear)(lb), T_(clear)(result);
	T_(push)(la, a), T_(push)(la, b), T_(push)(la, d);
	T_(push)(lb, b_alt), T_(push)(lb, c);
}
/** Verifies that `list` is `a`, `b`, `c`, `d`, null. */
static void pT_(exact)(struct t_(list) *const list,
	const struct t_(listlink) *const a, const struct t_(listlink) *const b,
	const struct t_(listlink) *const c, const struct t_(listlink) *const d) {
	struct t_(listlink) *i;
	assert(list);
	i = T_(head)(list), assert(i == a);
	if(!i) return;
	i = T_(link_next)(i), assert(i == b);
	if(!i) return;
	i = T_(link_next)(i), assert(i == c);
	if(!i) return;
	i = T_(link_next)(i), assert(i == d);
	if(!i) return;
	i = T_(link_next)(i), assert(!i);
}

/** Passed `parent_new` and `parent`, tests binary operations. */
static void pT_(test_binary)(struct t_(listlink) *(*const parent_new)(void *),
	void *const parent) {
	struct t_(list) la, lb, result;
	struct t_(listlink) *link, *a = 0, *b = 0, *b_alt = 0, *c = 0, *d = 0;
	int cmp;
	T_(clear)(&la);
	/* Test nulls, (Not comprehensive.) */
	cmp = T_(compare)(0, 0), assert(cmp == 0);
	cmp = T_(compare)(&la, 0), assert(cmp == -1);
	cmp = T_(compare)(0, &la), assert(cmp == 1);
	T_(subtraction_to)(0, 0, 0);
	T_(subtraction_to)(0, 0, &la);
	T_(union_to)(0, 0, 0);
	T_(union_to)(0, 0, &la);
	T_(intersection_to)(0, 0, 0);
	T_(intersection_to)(0, 0, &la);
	T_(xor_to)(0, 0, 0);
	T_(xor_to)(0, 0, &la);
	assert(!T_(head)(&la));
	{
		const size_t no_try = 5000;
		struct t_(list) x, y;
		size_t i;
		T_(clear)(&x), T_(clear)(&y);
		/* By the PHP, this should be more than enough to get at least the
		 small-entropy ones, (_ie_ `Letter`.) */
		for(i = 0; i < no_try; i++) {
			if(!(link = parent_new(parent))) { assert(0); return; }
			t_(filler)(link);
			T_(push)(&x, link);
			T_(sort)(&x);
			T_(duplicates_to)(&x, &y);
			/* fixme: list_duplicates_to is suspect? it keeps giving wrong. */
			printf("x = %s, y = %s\n", T_(to_string)(&x),
				T_(to_string)(&y));
			/* Honestly, what am I doing? */
			/* `x = (A,...,B,C,D,...)` and `y = {[A],B,...}`. */
			if(!(a = T_(head)(&x))) continue;
			if(!(b = T_(head)(&y))) continue;
			if(tr_(compare)(a, b) == 0 && !(b = T_(link_next)(b)))
				continue;
			assert(tr_(compare)(a, b) < 0);
			for(c = T_(link_next)(a); c && tr_(compare)(c, b) < 0;
				c = T_(link_next)(c));
			assert(c && tr_(compare)(c, b) == 0);
			b_alt = c;
			if(!(c = T_(link_next)(c)) || !(d = T_(link_next)(c))) continue;
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
	pT_(reset_b)(&la, &lb, &result, a, b, b_alt, c, d);
	printf("a = (A,B,D) = %s, b = (B,C) = %s, result = %s.\n",
		T_(to_string)(&la), T_(to_string)(&lb),
		T_(to_string)(&result));
	T_(subtraction_to)(&la, &lb, &result);
	printf("a - b = %s.\n", T_(to_string)(&result));
	pT_(exact)(&la, b, 0, 0, 0);
	pT_(exact)(&lb, b_alt, c, 0, 0);
	pT_(exact)(&result, a, d, 0, 0);

	pT_(reset_b)(&la, &lb, &result, a, b, b_alt, c, d);
	T_(union_to)(&la, &lb, &result);
	printf("a \\cup b = %s.\n", T_(to_string)(&result));
	pT_(exact)(&la, 0, 0, 0, 0);
	pT_(exact)(&lb, b_alt, 0, 0, 0);
	pT_(exact)(&result, a, b, c, d);

	pT_(reset_b)(&la, &lb, &result, a, b, b_alt, c, d);
	T_(intersection_to)(&la, &lb, &result);
	printf("a \\cap b = %s.\n", T_(to_string)(&result));
	pT_(exact)(&la, a, d, 0, 0);
	pT_(exact)(&lb, b_alt, c, 0, 0);
	pT_(exact)(&result, b, 0, 0, 0);

	pT_(reset_b)(&la, &lb, &result, a, b, b_alt, c, d);
	T_(xor_to)(&la, &lb, &result);
	printf("a \\xor b = %s.\n", T_(to_string)(&result));
	pT_(exact)(&la, b, 0, 0, 0);
	pT_(exact)(&lb, b_alt, 0, 0, 0);
	pT_(exact)(&result, a, c, d, 0);
}

/** The linked-list will be tested on stdout. `LIST_TEST` has to be set.
 @param[parent_new, parent] Responsible for creating new objects and returning
 the list. @allow */
static void TR_(compare_test)(struct t_(listlink)
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
	pT_(test_sort)(parent_new, parent);
	pT_(test_binary)(parent_new, parent);
	printf("Done tests of " QUOTE(LIST_NAME) ".\n\n");
}

#undef QUOTE
#undef QUOTE_
