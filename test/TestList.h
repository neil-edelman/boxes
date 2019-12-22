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

/* Check that LIST_TEST is a function implementing <typedef:<PL>Action>. */
static void (*const PL_(filler))(struct L_(ListLink) *) = (LIST_TEST);

static void PL_(graph)(const struct L_(List) *const list, const char *const fn)
{
	FILE *fp;
	struct L_(ListLink) *link;
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
	for(link = L_(ListFirst)(list); link; link = L_(ListNext)(link)) {
		PL_(to_string)(link, &a);
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
static void PL_(floyd)(struct L_(ListLink) *link, const int is_count,
	const size_t count) {
	size_t fw = 0, b1 = 0, b2 = 0;
	struct L_(ListLink) *hare = link, *turtle = hare;
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
static void PL_(valid)(struct L_(ListLink) *link) { PL_(floyd)(link, 0, 0); }
/** Debug: ensures that `link` has no cycles and that it has `count`
 elements. */
static void PL_(valid_count)(struct L_(ListLink) *link, const size_t count)
	{ PL_(floyd)(link, 1, count); }



static void PL_(test_basic)(struct L_(ListLink) *(*const parent_new)(void *),
	void *const parent) {
	struct L_(List) list;
	struct L_(ListLink) *link, *link_first = 0, *link_last = 0;
	const size_t test_size = 10;
	size_t i;
	assert(parent_new && parent);
	printf("Basic tests of <" QUOTE(LIST_NAME) ">List:\n");
	/* Clear */
	L_(ListClear)(0);
	L_(ListClear)(&list);
	PL_(valid_count)(&list.head, 0);
	printf("Adding %lu elements to a.\n", (unsigned long)test_size);
	/* Test positions. */
	link = L_(ListFirst)(0), assert(!link);
	link = L_(ListLast)(0), assert(!link);
	link = L_(ListFirst)(&list), assert(!link);
	link = L_(ListLast)(&list), assert(!link);
	link = L_(ListPrevious)(0), assert(!link);
	link = L_(ListNext)(0), assert(!link);
	/* Add */
	L_(ListPush)(0, 0);
	L_(ListPush)(&list, 0);
	for(i = 0; i < test_size; i++) {
		if(!(link = parent_new(parent))) { assert(0); return; }
		PL_(filler)(link);
		L_(ListPush)(&list, link);
		if(i == 0) link_first = link;
		link_last = link;
	}
	PL_(graph)(&list, "graph/" QUOTE(LIST_NAME) "-small.gv");
	PL_(valid_count)(&list.head, test_size);
	printf("Result: %s.\n", L_(ListToString)(&list));
	/* Test positions when contents. */
	link = L_(ListFirst)(&list), assert(link == link_first);
	link = L_(ListLast)(&list), assert(link == link_last);
	link = L_(ListPrevious)(link), assert(link);
	link = L_(ListNext)(link), assert(link == link_last);
}

static void PL_(test_sort)(struct L_(ListLink) *(*const parent_new)(void *),
	void *const parent) {
#ifdef LIST_COMPARE /* <!-- comp */
	struct L_(List) lists[64], *list;
	const size_t lists_size = sizeof lists / sizeof *lists;
	struct L_(List) *const lists_end = lists + lists_size;
	int cmp;
	/* Random lists. */
	for(list = lists; list < lists_end; list++) {
		size_t no_links = rand() / (RAND_MAX / 5 + 1);
		struct L_(ListLink) *link, *link_a, *link_b;
		L_(ListClear)(list);
		while(no_links) {
			if(!(link = parent_new(parent))) { assert(0); return; }
			PL_(filler)(link);
			L_(ListPush)(list, link);
			no_links--;
		}
		L_(ListSort)(list);
		for(link_a = 0, link_b = L_(ListFirst)(list); link_b;
			link_a = link_b, link_b = L_(ListNext)(link_b)) {
			if(!link_a) continue;
			cmp = PL_(compare)(link_a, link_b);
			assert(cmp <= 0);
		}
	}
	/* Now sort the lists. */
	qsort(lists, lists_size, sizeof *lists,
		(int (*)(const void *, const void *))&L_(ListCompare));
	printf("Sorted array of sorted <" QUOTE(LIST_NAME) ">List by "
		QUOTE(LIST_COMPARE) ":\n");
	for(list = lists; list < lists_end; list++) {
		L_(ListSelfCorrect)(list); /* `qsort` moves the pointers. */
		printf("List: %s.\n", L_(ListToString)(list));
		if(list == lists) continue;
		cmp = L_(ListCompare)(list - 1, list);
		assert(cmp <= 0);
	}
#else /* comp --><!-- !comp */
	(void)(parent_new), (void)(parent);
#endif /* !comp --> */
}

/** The linked-list will be tested on stdout. `LIST_TEST` has to be set.
 @allow */
static void L_(ListTest)(struct L_(ListLink) *(*const parent_new)(void *),
	void *const parent) {
	printf("<" QUOTE(LIST_NAME) ">List was created using: "
#ifdef LIST_COMPARE
		"LIST_COMPARE: <" QUOTE(LIST_COMPARE) ">; "
#endif
		"LIST_TO_STRING<" QUOTE(LIST_TO_STRING) ">; "
		"testing:\n");
	PL_(test_basic)(parent_new, parent);
	PL_(test_sort)(parent_new, parent);
}

#undef QUOTE
#undef QUOTE_
