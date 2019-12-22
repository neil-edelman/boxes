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
}

#undef QUOTE
#undef QUOTE_
