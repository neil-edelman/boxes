/* intended to be included by ../src/List.h on LIST_TEST */

#include <stdlib.h>	/* EXIT rand */
#include <stdio.h>  /* printf */
#include <stddef.h>

/* Define QUOTE. */
#ifdef QUOTE
#undef QUOTE
#endif
#ifdef QUOTE_
#undef QUOTE_
#endif
#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)

/* Check that LIST_TEST is a function implementing <typedef:<PN>Action>. */
static void (*const PN_(filler))(struct N_(ListLink) *) = (LIST_TEST);

/** Given `l` and `offset`, calculate the graph node. */
static const void *PN_(node)(const struct N_(ListLink) *const l,
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
static void PN_(subgraph)(const struct N_(List) *const list, FILE *const fp,
	const char *const colour, const size_t offset, const int is_nodes) {
	struct N_(ListLink) *link;
	char a[12];
	assert(list && fp && colour);
	fprintf(fp, "\t# fill %s for list %p\n"
		"\tnode [style=filled, fillcolor=pink];\n"
		"\tsubgraph cluster_%p {\n"
		"\t\tstyle=filled;\n"
		"\t\tfillcolor=lightgray;\n"
		"\t\tlabel=\"\\<" QUOTE(LIST_NAME) "\\>List\";\n",
		colour, (const char *)&list - offset, (const char *)&list - offset);
	fprintf(fp,
		"\t\tp%p [label=\"head\", shape=ellipse];\n"
		"\t\tp%p [label=\"tail\", shape=ellipse];\n"
		"\t\tp%p -> p%p [style=invis]; # vertical\n"
		"\t}\n", PN_(node)(&list->head, offset), PN_(node)(&list->tail, offset),
		PN_(node)(&list->head, offset), PN_(node)(&list->tail, offset));
	fprintf(fp,
		"\tnode [fillcolor=lightsteelblue];\n"
		"\tnode [shape=box];\n"
		"\tp%p -> p%p [color=%s];\n"
		"\tp%p -> p%p [color=%s4];\n",
		PN_(node)(&list->head, offset), PN_(node)(list->head.next, offset),
		colour,
		PN_(node)(&list->tail, offset), PN_(node)(list->tail.prev, offset),
		colour);
	for(link = N_(ListFirst)(list); link; link = N_(ListNext)(link)) {
		if(is_nodes) {
			PN_(to_string)(link, &a);
			fprintf(fp, "\tp%p [label=\"%s\"];\n", PN_(node)(link, offset), a);
		}
		fprintf(fp, "\tp%p -> p%p [color=%s];\n"
			"\tp%p -> p%p [color=%s4];\n",
			PN_(node)(link, offset), PN_(node)(link->next, offset), colour,
			PN_(node)(link, offset), PN_(node)(link->prev, offset), colour);
	}
}

/** Tries to graph `list` in `fn`. */
static void PN_(graph)(const struct N_(List) *const list, const char *const fn)
{
	FILE *fp;
	assert(list && fn);
	if(!(fp = fopen(fn, "w"))) { perror(fn); return; }
	fprintf(fp, "digraph {\n");
	PN_(subgraph)(list, fp, "royalblue", 0, 1);
	fprintf(fp, "\tnode [colour=red, style=filled];\n");
	fprintf(fp, "}\n");
	fclose(fp);
}

/** Perform "Floyd's" tortoise-hare algorithm for cycle detection for the list
 on which `link` is a part. If `is_count` is true, `count` must be the number
 of elements.
 @order \O(|`list`|) */
static void PN_(floyd)(struct N_(ListLink) *link, const int is_count,
	const size_t count) {
	size_t fw = 0, b1 = 0, b2 = 0;
	struct N_(ListLink) *hare = link, *turtle = hare;
	assert(link);
	for(turtle = hare; hare->prev; hare = hare->prev) {
		if(b1++ & 1) turtle = turtle->prev;
		assert(turtle != hare);
	}
	for(turtle = hare, hare = hare->next; hare->next; hare = hare->next) {
		if(fw++ & 1) turtle = turtle->next;
		assert(turtle != hare);
	}
	for(turtle = hare, hare = hare->prev; hare != link; hare = hare->prev) {
		if(b2++ & 1) turtle = turtle->prev;
		assert(hare && turtle != hare);
	}
	assert(fw == b1 + b2 && (!is_count || fw == count));
}
/** Debug: ensures that `link` has no cycles. */
/*static void PN_(valid)(struct N_(ListLink) *link) { PN_(floyd)(link, 0, 0); }*/
/** Debug: ensures that `link` has no cycles and that it has `count`
 elements. */
static void PN_(valid_count)(struct N_(ListLink) *link, const size_t count)
	{ PN_(floyd)(link, 1, count); }


/** Passed `parent_new` and `parent`, tests basic functionality. */
static void PN_(test_basic)(struct N_(ListLink) *(*const parent_new)(void *),
	void *const parent) {
	struct N_(List) list;
	struct N_(ListLink) *link, *link_first = 0, *link_last = 0;
	const size_t test_size = 10;
	size_t i;
	assert(parent_new && parent);
	printf("Basic tests of <" QUOTE(LIST_NAME) ">List:\n");
	/* Clear */
	N_(ListClear)(0);
	N_(ListClear)(&list);
	PN_(valid_count)(&list.head, 0);
	printf("Adding %lu elements to a.\n", (unsigned long)test_size);
	/* Test positions null. */
	link = N_(ListFirst)(0), assert(!link);
	link = N_(ListLast)(0), assert(!link);
	link = N_(ListFirst)(&list), assert(!link);
	link = N_(ListLast)(&list), assert(!link);
	link = N_(ListPrevious)(0), assert(!link);
	link = N_(ListNext)(0), assert(!link);
	/* Test other stuff null, empty. */
	N_(ListUnshift)(0, 0);
	N_(ListUnshift)(&list, 0);
	N_(ListPush)(0, 0);
	N_(ListPush)(&list, 0);
	N_(ListAddBefore)(0, 0);
	N_(ListAddAfter)(0, 0);
	N_(ListRemove)(0);
	PN_(valid_count)(&list.head, 0);
	/* Test returns on null and empty. */
	link = N_(ListShift)(0), assert(!link);
	link = N_(ListShift)(&list), assert(!link);
	link = N_(ListPop)(0), assert(!link);
	link = N_(ListPop)(&list), assert(!link);
	/* Test other stuff null. */
	N_(ListTo)(0, 0);
	N_(ListToBefore)(0, 0);
	N_(ListToIf)(0, 0, 0);
	N_(ListForEach)(0, 0);
	link = N_(ListAny)(0, 0), assert(!link);
	/* Add */
	N_(ListPush)(0, 0);
	N_(ListPush)(&list, 0);
	for(i = 0; i < test_size; i++) {
		if(!(link = parent_new(parent))) { assert(0); return; }
		PN_(filler)(link);
		N_(ListPush)(&list, link);
		if(i == 0) link_first = link;
		link_last = link;
	}
	PN_(graph)(&list, "graph/" QUOTE(LIST_NAME) "-small.gv");
	PN_(valid_count)(&list.head, test_size);
	printf("Result: %s.\n", N_(ListToString)(&list));
	/* Test positions when contents. */
	link = N_(ListFirst)(&list), assert(link == link_first);
	link = N_(ListLast)(&list), assert(link == link_last);
	link = N_(ListPrevious)(link), assert(link);
	link = N_(ListNext)(link), assert(link == link_last);
	/* Test movement. */
}

/** Passed `parent_new` and `parent`, tests sort and meta-sort. */
static void PN_(test_sort)(struct N_(ListLink) *(*const parent_new)(void *),
	void *const parent) {
#ifdef LIST_COMPARE /* <!-- comp */
	struct N_(List) lists[64], *list;
	const size_t lists_size = sizeof lists / sizeof *lists;
	struct N_(List) *const lists_end = lists + lists_size;
	int cmp;
	/* Random lists. */
	for(list = lists; list < lists_end; list++) {
		size_t no_links = rand() / (RAND_MAX / 5 + 1);
		struct N_(ListLink) *link, *link_a, *link_b;
		N_(ListClear)(list);
		while(no_links) {
			if(!(link = parent_new(parent))) { assert(0); return; }
			PN_(filler)(link);
			N_(ListPush)(list, link);
			no_links--;
		}
		N_(ListSort)(list);
		for(link_a = 0, link_b = N_(ListFirst)(list); link_b;
			link_a = link_b, link_b = N_(ListNext)(link_b)) {
			if(!link_a) continue;
			cmp = PN_(compare)(link_a, link_b);
			assert(cmp <= 0);
		}
	}
	/* Now sort the lists. */
	qsort(lists, lists_size, sizeof *lists,
		(int (*)(const void *, const void *))&N_(ListCompare));
	printf("Sorted array of sorted <" QUOTE(LIST_NAME) ">List by "
		QUOTE(LIST_COMPARE) ":\n");
	for(list = lists; list < lists_end; list++) {
		N_(ListSelfCorrect)(list); /* `qsort` moves the pointers. */
		printf("List: %s.\n", N_(ListToString)(list));
		if(list == lists) continue;
		cmp = N_(ListCompare)(list - 1, list);
		assert(cmp <= 0);
	}
#else /* comp --><!-- !comp */
	(void)(parent_new), (void)(parent);
#endif /* !comp --> */
}

#ifdef LIST_COMPARE /* <!-- comp */
/** Set up the incredibly contrived example involving `la`, `lb`, `result`, and
 `a`, `b`, `b_alt`, `c`, `d` for <fn:<PN>test_binary>, where `a = ()`,
 `b = (A,B,D)`, and `c = (B,C)`. */
static void PN_(reset_b)(struct N_(List) *const la, struct N_(List) *const lb,
	struct N_(List) *const result, struct N_(ListLink) *const a,
	struct N_(ListLink) *const b, struct N_(ListLink) *const b_alt,
	struct N_(ListLink) *const c, struct N_(ListLink) *const d) {
	assert(la && lb && result && a && b && b_alt && c && d);
	N_(ListClear)(la), N_(ListClear)(lb), N_(ListClear)(result);
	N_(ListPush)(la, a), N_(ListPush)(la, b), N_(ListPush)(la, d);
	N_(ListPush)(lb, b_alt), N_(ListPush)(lb, c);
}
/** Verifies that `list` is `a`, `b`, `c`, `d`, null. */
static void PN_(exact)(const struct N_(List) *const list,
	const struct N_(ListLink) *const a, const struct N_(ListLink) *const b,
	const struct N_(ListLink) *const c, const struct N_(ListLink) *const d) {
	struct N_(ListLink) *i;
	assert(list);
	i = N_(ListFirst)(list), assert(i == a);
	i = N_(ListNext)(i), assert(i == b);
	i = N_(ListNext)(i), assert(i == c);
	i = N_(ListNext)(i), assert(i == d);
	i = N_(ListNext)(i), assert(!i);
}
#endif /* comp --> */

/** Passed `parent_new` and `parent`, tests binary operations. */
static void PN_(test_binary)(struct N_(ListLink) *(*const parent_new)(void *),
	void *const parent) {
#ifdef LIST_COMPARE /* <!-- comp */
	struct N_(List) la, lb, result;
	struct N_(ListLink) *link, *a, *b, *b_alt, *c, *d;
	int cmp;
	/* Test nulls, (Not comprehensive.) */
	N_(ListClear)(&la);
	N_(ListSort)(0);
	N_(ListMerge)(0, 0);
	cmp = N_(ListCompare)(0, 0), assert(cmp == 0);
	cmp = N_(ListCompare)(&la, 0), assert(cmp > 0);
	cmp = N_(ListCompare)(0, &la), assert(cmp < 0);
	N_(ListDuplicatesTo)(0, 0);
	N_(ListDuplicatesTo)(0, &la);
	N_(ListSubtractionTo)(0, 0, 0);
	N_(ListSubtractionTo)(0, 0, &la);
	N_(ListUnionTo)(0, 0, 0);
	N_(ListUnionTo)(0, 0, &la);
	N_(ListIntersectionTo)(0, 0, 0);
	N_(ListIntersectionTo)(0, 0, &la);
	N_(ListXorTo)(0, 0, 0);
	N_(ListXorTo)(0, 0, &la);
	assert(!N_(ListFirst)(&la));
	{
		const size_t no_try = 5000;
		struct N_(List) x, y;
		size_t i;
		/* By the PHP, this should be more than enough to get at least the
		 small-entropy ones, (_ie_ `Letter`.) */
		N_(ListClear)(&x), N_(ListClear)(&y);
		for(i = 0; i < no_try; i++) {
			if(!(link = parent_new(parent))) { assert(0); return; }
			PN_(filler)(link);
			N_(ListPush)(&x, link);
			N_(ListSort)(&x);
			N_(ListDuplicatesTo)(&x, &y);
			/* `x = (A,...,B,C,D,...)` and `y = {[A],B,...}`. */
			if(!(a = N_(ListFirst)(&x))) continue;
			if(!(b = N_(ListFirst)(&y))) continue;
			if(PN_(compare)(a, b) == 0) if(!(b = N_(ListNext)(b))) continue;
			assert(PN_(compare)(a, b) < 0);
			for(c = N_(ListNext)(a); c && PN_(compare)(c, b) < 0;
				c = N_(ListNext)(c));
			assert(c && PN_(compare)(c, b) == 0);
			b_alt = c;
			if(!(c = N_(ListNext)(c)) || !(d = N_(ListNext)(c))) continue;
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
	PN_(reset_b)(&la, &lb, &result, a, b, b_alt, c, d);
	printf("a = %s, b = %s", N_(ListToString)(&la), N_(ListToString)(&lb));
	printf(", result = %s.\n", N_(ListToString)(&result));
	N_(ListSubtractionTo)(&la, &lb, &result);
	printf("a - b = %s.\n", N_(ListToString)(&result));
	PN_(exact)(&la, b, 0, 0, 0);
	PN_(exact)(&lb, b_alt, c, 0, 0);
	PN_(exact)(&result, a, d, 0, 0);

	PN_(reset_b)(&la, &lb, &result, a, b, b_alt, c, d);
	N_(ListUnionTo)(&la, &lb, &result);
	printf("a \\cup b = %s.\n", N_(ListToString)(&result));
	PN_(exact)(&la, 0, 0, 0, 0);
	PN_(exact)(&lb, b_alt, 0, 0, 0);
	PN_(exact)(&result, a, b, c, d);

	PN_(reset_b)(&la, &lb, &result, a, b, b_alt, c, d);
	N_(ListIntersectionTo)(&la, &lb, &result);
	printf("a \\cap b = %s.\n", N_(ListToString)(&result));
	PN_(exact)(&la, a, d, 0, 0);
	PN_(exact)(&lb, b_alt, c, 0, 0);
	PN_(exact)(&result, b, 0, 0, 0);

	PN_(reset_b)(&la, &lb, &result, a, b, b_alt, c, d);
	N_(ListXorTo)(&la, &lb, &result);
	printf("a \\xor b = %s.\n", N_(ListToString)(&result));
	PN_(exact)(&la, b, 0, 0, 0);
	PN_(exact)(&lb, b_alt, 0, 0, 0);
	PN_(exact)(&result, a, c, d, 0);

#else /* comp --><!-- !comp */
	(void)(parent_new), (void)(parent);
#endif /* !comp --> */
}

/** The linked-list will be tested on stdout. `LIST_TEST` has to be set.
 @param[parent_new, parent] Responsible for creating new objects and returning
 the list.
 @allow */
static void N_(ListTest)(struct N_(ListLink) *(*const parent_new)(void *),
	void *const parent) {
	printf("<" QUOTE(LIST_NAME) ">List was created using: "
#ifdef LIST_COMPARE
		"LIST_COMPARE: <" QUOTE(LIST_COMPARE) ">; "
#endif
		"LIST_TO_STRING<" QUOTE(LIST_TO_STRING) ">; "
		"testing:\n");
	PN_(test_basic)(parent_new, parent);
	PN_(test_sort)(parent_new, parent);
	PN_(test_binary)(parent_new, parent);
}

#undef QUOTE
#undef QUOTE_
