/* intended to be included by ../src/List.h on LIST_TEST */

#include <stdlib.h>	/* EXIT rand */
#include <stdio.h>  /* printf */
#include <stddef.h>

#ifdef QUOTE
#undef QUOTE
#endif
#ifdef QUOTE_
#undef QUOTE_
#endif
#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)

#ifdef LIST_TO_STRING /* <!-- to string: Only one, tests all base code. */

/* Copy functions for later includes. */
static void (*PL_(to_string))(const struct L_(list_node) *, char (*)[12])
	= (LIST_TO_STRING);
static const char *(*PL_(list_to_string))(const struct L_(list) *)
	= Z_(to_string);

/* Check that LIST_TEST is a function implementing <typedef:<PL>action_fn>. */
static void (*const PL_(filler))(struct L_(list_node) *) = (LIST_TEST);

/** Given `l` and `offset`, calculate the graph node. */
static const void *PL_(node)(const struct L_(list_node) *const l,
	const size_t offset) {
	assert(l);
	return (const char *)l - (l->prev && l->next ? offset : 0);
}

/** Digraph `list` to `fp`.
 @param[colour] A colour that can also have a 4 appended; suggest "royalblue",
 "firebrick" "darkseagreen" "orchid".
 @param[offset] For printing multiple lists, offset to the parent type.
 @param[is_nodes] Print nodes; if one is printing the same list, different
 order, then this would be off. */
static void PL_(subgraph)(const struct L_(list) *const list, FILE *const fp,
	const char *const colour, const size_t offset, const int is_nodes) {
	struct L_(list_node) *link;
	char a[12];
	assert(list && fp && colour);
	fprintf(fp, "\t# fill %s for list %p\n"
		"\tnode [style=filled, fillcolor=pink];\n"
		"\tsubgraph cluster_%p {\n"
		"\t\tstyle=filled;\n"
		"\t\tfillcolor=lightgray;\n"
		"\t\tlabel=\"\\<" QUOTE(LIST_NAME) "\\>list\";\n",
		colour, (const void *)((const char *)&list - offset),
		(const void *)((const char *)&list - offset));
	fprintf(fp,
		"\t\tp%p [label=\"head\", shape=ellipse];\n"
		"\t\tp%p [label=\"tail\", shape=ellipse];\n"
		"\t\tp%p -> p%p [style=invis]; # vertical\n"
		"\t}\n", PL_(node)(&list->head, offset), PL_(node)(&list->tail, offset),
		PL_(node)(&list->head, offset), PL_(node)(&list->tail, offset));
	fprintf(fp,
		"\tnode [fillcolor=lightsteelblue];\n"
		"\tnode [shape=box];\n"
		"\tp%p -> p%p [color=%s];\n"
		"\tp%p -> p%p [color=%s4];\n",
		PL_(node)(&list->head, offset), PL_(node)(list->head.next, offset),
		colour,
		PL_(node)(&list->tail, offset), PL_(node)(list->tail.prev, offset),
		colour);
	for(link = L_(list_first)(list); link; link = L_(list_next)(link)) {
		if(is_nodes) {
			PL_(to_string)(link, &a);
			fprintf(fp, "\tp%p [label=\"%s\"];\n", PL_(node)(link, offset), a);
		}
		fprintf(fp, "\tp%p -> p%p [color=%s];\n"
			"\tp%p -> p%p [color=%s4];\n",
			PL_(node)(link, offset), PL_(node)(link->next, offset), colour,
			PL_(node)(link, offset), PL_(node)(link->prev, offset), colour);
	}
}

/** Tries to graph `list` in `fn`. */
static void PL_(graph)(const struct L_(list) *const list, const char *const fn)
{
	FILE *fp;
	assert(list && fn);
	if(!(fp = fopen(fn, "w"))) { perror(fn); return; }
	fprintf(fp, "digraph {\n");
	PL_(subgraph)(list, fp, "royalblue", 0, 1);
	fprintf(fp, "\tnode [colour=red, style=filled];\n");
	fprintf(fp, "}\n");
	fclose(fp);
}

/** Perform "Floyd's" tortoise-hare algorithm for cycle detection for the list
 on which `link` is a part and expect `count`. `list` must have at least one
 element, (it can't be the head of tail.)
 @order \O(|`list`|) */
static void PL_(floyd)(const struct L_(list_node) *link, const size_t count) {
	size_t fw = 0, b1 = 0, b2 = 0;
	const struct L_(list_node) *hare = link, *turtle = hare;
	assert(link && link->prev && link->next);
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
static void PL_(count)(const struct L_(list) *const list, const size_t count) {
	const struct L_(list_node) *const head = &list->head,
		*const tail = &list->tail, *first;
	assert(list && head && tail && !list->head.prev && !list->tail.next);
	if((first = head->next) == tail)
		{ assert(tail->prev == head && !count); return; }
	PL_(floyd)(first, count);
}

/** Returns `0,1,0,1,...` whatever `link`. */
static int PL_(parity)(const struct L_(list_node) *const link) {
	static int p;
	(void)(link);
	return !(p = !p);
}

/** Returns true whatever `link`. */
static int PL_(true)(const struct L_(list_node) *const link) {
	(void)(link);
	return 1;
}

/** Passed `parent_new` and `parent`, tests basic functionality. */
static void PL_(test_basic)(struct L_(list_node) *(*const parent_new)(void *),
	void *const parent) {
	struct L_(list) l1, l2;
	struct L_(list_node) *link, *link_first = 0, *link_last = 0;
	const size_t test_size = 10;
	size_t i;
	assert(parent_new && parent);
	printf("Basic tests of <" QUOTE(LIST_NAME) ">list:\n");
	/* Clear */
	L_(list_clear)(&l1);
	L_(list_clear)(&l2);
	PL_(count)(&l1, 0);
	/* Test positions null. */
	link = L_(list_first)(&l1), assert(!link);
	link = L_(list_last)(&l1), assert(!link);
	/* Test returns on null and empty. */
	link = L_(list_shift)(&l1), assert(!link);
	link = L_(list_pop)(&l1), assert(!link);
	/* Add */
	printf("Adding %lu elements to l1.\n", (unsigned long)test_size);
	for(i = 0; i < test_size; i++) {
		if(!(link = parent_new(parent))) { assert(0); return; };
		PL_(filler)(link);
		L_(list_push)(&l1, link);
		if(i == 0) link_first = link;
		link_last = link;
	}
	PL_(graph)(&l1, "graph/" QUOTE(LIST_NAME) "-small.gv");
	PL_(count)(&l1, test_size);
	printf("l1 = %s.\n", PL_(list_to_string)(&l1));
	/* Test positions when contents. */
	link = L_(list_first)(&l1), assert(link == link_first);
	link = L_(list_last)(&l1), assert(link == link_last);
	link = L_(list_previous)(link), assert(link);
	link = L_(list_next)(link), assert(link == link_last);
	/* Test remove contents. */
	link = L_(list_shift)(&l1), assert(link == link_first);
	link = L_(list_pop)(&l1), assert(link = link_last);
	PL_(count)(&l1, test_size - 2);
	L_(list_unshift)(&l1, link_first);
	L_(list_push)(&l1, link_last);
	PL_(count)(&l1, test_size);
	link = L_(list_first)(&l1), assert(link == link_first);
	link = L_(list_last)(&l1), assert(link == link_last);
	/* Test movement. */
	PL_(count)(&l1, test_size);
	PL_(count)(&l2, 0);
	L_(list_to_if)(&l1, &l2, &PL_(parity));
	printf("Transferring . . . l1 = %s; l2 = %s.\n",
		PL_(list_to_string)(&l1), PL_(list_to_string)(&l2));
	PL_(count)(&l1, test_size >> 1);
	PL_(count)(&l2, test_size - (test_size >> 1));
	assert(L_(list_first)(&l1) == link_first);
	L_(list_to_before)(&l2, link_first->next);
	PL_(count)(&l1, test_size);
	PL_(count)(&l2, 0);
	assert(L_(list_first)(&l1) == link_first);
	L_(list_to)(&l1, &l2);
	PL_(count)(&l1, 0);
	PL_(count)(&l2, test_size);
	assert(L_(list_first)(&l2) == link_first);
	/* Test any. */
	link = L_(list_any)(&l1, &PL_(true)), assert(!link);
	link = L_(list_any)(&l2, &PL_(true)), assert(link == link_first);
	/* Test add before/after. */
	if(!(link = parent_new(parent))) { assert(0); return; };
	PL_(filler)(link);
	L_(list_add_before)(L_(list_first)(&l2), link);
	link_first = L_(list_first)(&l2);
	assert(link == link_first);
	PL_(count)(&l2, test_size + 1);
	if(!(link = parent_new(parent))) { assert(0); return; };
	PL_(filler)(link);
	L_(list_add_before)(L_(list_last)(&l2), link);
	PL_(count)(&l2, test_size + 2);
	L_(list_clear)(&l2);
	PL_(count)(&l2, 0);
}

/** Passed `parent_new` and `parent`, tests sort and meta-sort. */
static void PL_(test_sort)(struct L_(list_node) *(*const parent_new)(void *),
	void *const parent) {
#ifdef LIST_COMPARE /* <!-- comp */
	struct L_(list) lists[64], *list;
	const size_t lists_size = sizeof lists / sizeof *lists;
	struct L_(list) *const lists_end = lists + lists_size;
	int cmp;
	/* Random lists. */
	for(list = lists; list < lists_end; list++) {
		size_t no_links = (unsigned)rand() / (RAND_MAX / 5 + 1);
		struct L_(list_node) *link, *link_a, *link_b;
		L_(list_clear)(list);
		while(no_links) {
			if(!(link = parent_new(parent))) { assert(0); return; }
			PL_(filler)(link);
			L_(list_push)(list, link);
			no_links--;
		}
		L_(list_sort)(list);
		for(link_a = 0, link_b = L_(list_first)(list); link_b;
			link_a = link_b, link_b = L_(list_next)(link_b)) {
			if(!link_a) continue;
			cmp = PL_(compare)(link_a, link_b);
			assert(cmp <= 0);
		}
	}
	/* Now sort the lists. */
	qsort(lists, lists_size, sizeof *lists,
		(int (*)(const void *, const void *))&L_(list_compare));
	printf("Sorted array of sorted <" QUOTE(LIST_NAME) ">list by "
		QUOTE(LIST_COMPARE) ":\n");
	for(list = lists; list < lists_end; list++) {
		L_(list_self_correct)(list); /* `qsort` moves the pointers. */
		printf("list: %s.\n", PL_(list_to_string)(list));
		if(list == lists) continue;
		cmp = L_(list_compare)(list - 1, list);
		assert(cmp <= 0);
	}
#else /* comp --><!-- !comp */
	(void)(parent_new), (void)(parent);
#endif /* !comp --> */
}

#ifdef LIST_COMPARE /* <!-- comp */
/** Set up the incredibly contrived example involving `la`, `lb`, `result`, and
 `a`, `b`, `b_alt`, `c`, `d` for <fn:<PL>test_binary>, where `a = ()`,
 `b = (A,B,D)`, and `c = (B,C)`. */
static void PL_(reset_b)(struct L_(list) *const la, struct L_(list) *const lb,
	struct L_(list) *const result, struct L_(list_node) *const a,
	struct L_(list_node) *const b, struct L_(list_node) *const b_alt,
	struct L_(list_node) *const c, struct L_(list_node) *const d) {
	assert(la && lb && result && a && b && b_alt && c && d);
	L_(list_clear)(la), L_(list_clear)(lb), L_(list_clear)(result);
	L_(list_push)(la, a), L_(list_push)(la, b), L_(list_push)(la, d);
	L_(list_push)(lb, b_alt), L_(list_push)(lb, c);
}
/** Verifies that `list` is `a`, `b`, `c`, `d`, null. */
static void PL_(exact)(const struct L_(list) *const list,
	const struct L_(list_node) *const a, const struct L_(list_node) *const b,
	const struct L_(list_node) *const c, const struct L_(list_node) *const d) {
	struct L_(list_node) *i;
	assert(list);
	i = L_(list_first)(list), assert(i == a);
	if(!i) return;
	i = L_(list_next)(i), assert(i == b);
	if(!i) return;
	i = L_(list_next)(i), assert(i == c);
	if(!i) return;
	i = L_(list_next)(i), assert(i == d);
	if(!i) return;
	i = L_(list_next)(i), assert(!i);
}
#endif /* comp --> */

/** Passed `parent_new` and `parent`, tests binary operations. */
static void PL_(test_binary)(struct L_(list_node) *(*const parent_new)(void *),
	void *const parent) {
#ifdef LIST_COMPARE /* <!-- comp */
	struct L_(list) la, lb, result;
	struct L_(list_node) *link, *a = 0, *b = 0, *b_alt = 0, *c = 0, *d = 0;
	int cmp;
	/* Test nulls, (Not comprehensive.) */
	L_(list_clear)(&la);
	cmp = L_(list_compare)(0, 0), assert(cmp == 0);
	cmp = L_(list_compare)(&la, 0), assert(cmp > 0);
	cmp = L_(list_compare)(0, &la), assert(cmp < 0);
	L_(list_subtraction_to)(0, 0, 0);
	L_(list_subtraction_to)(0, 0, &la);
	L_(list_union_to)(0, 0, 0);
	L_(list_union_to)(0, 0, &la);
	L_(list_intersection_to)(0, 0, 0);
	L_(list_intersection_to)(0, 0, &la);
	L_(list_xor_to)(0, 0, 0);
	L_(list_xor_to)(0, 0, &la);
	assert(!L_(list_first)(&la));
	{
		const size_t no_try = 5000;
		struct L_(list) x, y;
		size_t i;
		/* By the PHP, this should be more than enough to get at least the
		 small-entropy ones, (_ie_ `Letter`.) */
		L_(list_clear)(&x), L_(list_clear)(&y);
		for(i = 0; i < no_try; i++) {
			if(!(link = parent_new(parent))) { assert(0); return; }
			PL_(filler)(link);
			L_(list_push)(&x, link);
			L_(list_sort)(&x);
			L_(list_duplicates_to)(&x, &y);
			/* fixme: list_duplicates_to is suspect? it keeps giving wrong. */
			printf("x = %s, y = %s\n", L_(list_to_string)(&x), L_(list_to_string)(&y));
			/* Honestly, what am I doing? */
			/* `x = (A,...,B,C,D,...)` and `y = {[A],B,...}`. */
			if(!(a = L_(list_first)(&x))) continue;
			if(!(b = L_(list_first)(&y))) continue;
			if(PL_(compare)(a, b) == 0 && !(b = L_(list_next)(b))) continue;
			assert(PL_(compare)(a, b) < 0);
			for(c = L_(list_next)(a); c && PL_(compare)(c, b) < 0;
				c = L_(list_next)(c));
			assert(c && PL_(compare)(c, b) == 0);
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
		PL_(list_to_string)(&la), PL_(list_to_string)(&lb),
		PL_(list_to_string)(&result));
	L_(list_subtraction_to)(&la, &lb, &result);
	printf("a - b = %s.\n", PL_(list_to_string)(&result));
	PL_(exact)(&la, b, 0, 0, 0);
	PL_(exact)(&lb, b_alt, c, 0, 0);
	PL_(exact)(&result, a, d, 0, 0);

	PL_(reset_b)(&la, &lb, &result, a, b, b_alt, c, d);
	L_(list_union_to)(&la, &lb, &result);
	printf("a \\cup b = %s.\n", PL_(list_to_string)(&result));
	PL_(exact)(&la, 0, 0, 0, 0);
	PL_(exact)(&lb, b_alt, 0, 0, 0);
	PL_(exact)(&result, a, b, c, d);

	PL_(reset_b)(&la, &lb, &result, a, b, b_alt, c, d);
	L_(list_intersection_to)(&la, &lb, &result);
	printf("a \\cap b = %s.\n", PL_(list_to_string)(&result));
	PL_(exact)(&la, a, d, 0, 0);
	PL_(exact)(&lb, b_alt, c, 0, 0);
	PL_(exact)(&result, b, 0, 0, 0);

	PL_(reset_b)(&la, &lb, &result, a, b, b_alt, c, d);
	L_(list_xor_to)(&la, &lb, &result);
	printf("a \\xor b = %s.\n", PL_(list_to_string)(&result));
	PL_(exact)(&la, b, 0, 0, 0);
	PL_(exact)(&lb, b_alt, 0, 0, 0);
	PL_(exact)(&result, a, c, d, 0);

#else /* comp --><!-- !comp */
	(void)(parent_new), (void)(parent);
#endif /* !comp --> */
}

/** The linked-list will be tested on stdout. `LIST_TEST` has to be set.
 @param[parent_new, parent] Responsible for creating new objects and returning
 the list. @allow */
static void L_(list_test)(struct L_(list_node) *(*const parent_new)(void *),
	void *const parent) {
	printf("<" QUOTE(LIST_NAME) ">list was created using: "
#ifdef LIST_COMPARE
		"LIST_COMPARE: <" QUOTE(LIST_COMPARE) ">; "
#endif
		"LIST_TO_STRING<" QUOTE(LIST_TO_STRING) ">; "
		"testing:\n");
	PL_(test_basic)(parent_new, parent);
	PL_(test_sort)(parent_new, parent);
	PL_(test_binary)(parent_new, parent);
}

#else /* to string --><!-- */
#error Test unsupported option; testing is out-of-sync?
#endif /* --> */

#undef QUOTE
#undef QUOTE_
