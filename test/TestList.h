/* intended to be included by ../src/List.h on LIST_TEST */

#include <stdlib.h>	/* EXIT rand */
#include <stdio.h>  /* printf */

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

/** Tries to graph `list` in `fn`. */
static void PN_(graph)(const struct N_(List) *const list, const char *const fn)
{
	FILE *fp;
	struct N_(ListLink) *link;
	char a[12];
	/* What I'm going to miss is most about multi-list is the debug colours
	 "firebrick" "darkseagreen" "orchid", but just make a parent. */
	const char colour[] = "royalblue";
	assert(list && fn);
	if(!(fp = fopen(fn, "w"))) { perror(fn); return; }
	fprintf(fp, "digraph {\n"
		"\tnode [style=filled, fillcolor=pink];\n"
		"\tsubgraph cluster_list {\n"
		"\t\tstyle=filled;\n"
		"\t\tfillcolor=lightgray;\n"
		"\t\tlabel=\"\\<" QUOTE(LIST_NAME) "\\>List\";\n"
		"\t\tp%p [label=\"head\"];\n"
		"\t\tp%p [label=\"tail\"];\n"
		"\t\tp%p -> p%p [style=invis];\n"
		"\t}\n"
		"\tnode [fillcolor=lightsteelblue];\n"
		"\tnode [shape=box];\n"
		"\tp%p -> p%p [color=%s];\n"
		"\tp%p -> p%p [color=%s4];\n",
		(const void *)&list->head, (const void *)&list->tail,
		(const void *)&list->head, (const void *)&list->tail,
		(const void *)&list->head, (void *)list->head.next, colour,
		(const void *)&list->tail, (void *)list->tail.prev, colour);
	for(link = N_(ListFirst)(list); link; link = N_(ListNext)(link)) {
		PN_(to_string)(link, &a);
		fprintf(fp, "\tp%p [label=\"%s\"];\n"
			"\tp%p -> p%p [fillcolor=%s];\n"
			"\tp%p -> p%p [fillcolor=%s4];\n",
			(const void *)link, a,
			(const void *)link, (void *)link->next, colour,
			(const void *)link, (void *)link->prev, colour);
	}
	/*fprintf(fp, "subgraph %s {\n"
		"style=filled;\n"
		"color=%s;\n"
		"node [style=filled,color=white];\n}\n", colour, colour);*/
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
static void PN_(valid)(struct N_(ListLink) *link) { PN_(floyd)(link, 0, 0); }
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
	/* Test positions. */
	link = N_(ListFirst)(0), assert(!link);
	link = N_(ListLast)(0), assert(!link);
	link = N_(ListFirst)(&list), assert(!link);
	link = N_(ListLast)(&list), assert(!link);
	link = N_(ListPrevious)(0), assert(!link);
	link = N_(ListNext)(0), assert(!link);
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
/** Set up the incredibly contrived example involving `a`, `b`, `c`, and
 `link_a`, `link_b`, `link_alt_b`, `link_c`, `link_d` for
 <fn:<PN>test_binary>, `a = ()`, `b = (A,B,D)`, and `c = (B,C)`. */
static void PN_(reset_bool)(struct N_(List) *const a, struct N_(List) *const b,
	struct N_(List) *const c, struct N_(ListLink) *const link_a,
	struct N_(ListLink) *const link_b, struct N_(ListLink) *const link_alt_b,
	struct N_(ListLink) *const link_c, struct N_(ListLink) *const link_d) {
	assert(a && b && c && link_a && link_b && link_alt_b && link_c && link_d);
	N_(ListClear)(a), N_(ListClear)(b), N_(ListClear)(c);
	N_(ListPush)(c, link_b), N_(ListPush)(c, link_c);
	N_(ListPush)(b, link_a), N_(ListPush)(b, link_alt_b),
		N_(ListPush)(b, link_d);
}
#endif /* comp --> */

/** Passed `parent_new` and `parent`, tests binary operations. */
static void PN_(test_binary)(struct N_(ListLink) *(*const parent_new)(void *),
	void *const parent) {
#ifdef LIST_COMPARE /* <!-- comp */
	struct N_(List) a, b, c;
	struct N_(ListLink) *link_a, *link_b, *link_alt_b, *link_c, *link_d;
	int cmp;
	/* Test nulls, (Not comprehensive.) */
	N_(ListClear)(&a);
	N_(ListSort)(0);
	N_(ListMerge)(0, 0);
	cmp = N_(ListCompare)(0, 0), assert(cmp == 0);
	cmp = N_(ListCompare)(&a, 0), assert(cmp > 0);
	cmp = N_(ListCompare)(0, &a), assert(cmp < 0);
	N_(ListDuplicatesTo)(0, 0);
	/* fixme */
	{
		const size_t no_try = 5000;
		struct N_(List) x, y;
		size_t i;
		/* By the PHP, this should be more than enough to get at least the
		 small-entropy ones, (_ie_ `Letter`.) */
		N_(ListClear)(&x), N_(ListClear)(&y);
		for(i = 0; i < no_try; i++) {
			struct N_(ListLink) *link;
			if(!(link = parent_new(parent))) { assert(0); return; }
			PN_(filler)(link);
			N_(ListPush)(&x, link);
			N_(ListSort)(&x);
			N_(ListDuplicatesTo)(&x, &y);
			/* `x = (A,...,B,C,D,...)` and `y = {[A],B,...}`. */
			if(!(link_a = N_(ListFirst)(&x))) continue;
			if(!(link_b = N_(ListFirst)(&y))) continue;
			if(PN_(compare)(link_a, link_b) == 0)
				if(!(link_b = N_(ListNext)(link_b))) continue;
			assert(PN_(compare)(link_a, link_b) < 0);
			for(link_c = N_(ListNext)(link_a);
				link_c && PN_(compare)(link_c, link_b) < 0;
				link_c = N_(ListNext)(link_c));
			assert(link_c && PN_(compare)(link_c, link_b) == 0);
			link_alt_b = link_c;
			if(!(link_c = N_(ListNext)(link_c))
				|| !(link_d = N_(ListNext)(link_c))) continue;
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
	PN_(reset_bool)(&a, &b, &c, link_a, link_b, link_alt_b, link_c, link_d);
	printf("a = %s, ", N_(ListToString)(&a));
	printf("b = %s, c = %s.\n", N_(ListToString)(&b), N_(ListToString)(&c));
	
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
