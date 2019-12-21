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
		"\tp%p [label=\"head\"];\n"
		"\tp%p [label=\"tail\"];\n"
		"\tnode [shape=box];\n"
		"\tp%p -> p%p [color=%s];\n"
		"\tp%p -> p%p [color=%s4];\n",
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
	hare = hare->next;
	for(turtle = hare; hare->next; hare = hare->next) {
		if(fw++ & 1) turtle = turtle->next;
			assert(turtle != hare);
			}
	hare = hare->prev;
	for(turtle = hare; hare != link; hare = hare->prev) {
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



static void L_(test_basic)(struct L_(ListLink) *(*const parent_new)(void *),
	void *const parent) {
	struct L_(List) list;
	struct L_(ListLink) *link;
	const size_t test_size = 10;
	size_t i;
	assert(parent_new && parent);
	printf("Basic tests of <" QUOTE(LIST_NAME) ">List:\n");
	/* Clear */
	L_(ListClear)(0);
	L_(ListClear)(&list);
	PL_(valid_count)(&list.head, 0);
	printf("Adding %lu elements to a.\n", (unsigned long)test_size);
	/* Add */
	L_(ListPush)(0, 0);
	L_(ListPush)(&list, 0);
	for(i = 0; i < test_size; i++) {
		if(!(link = parent_new(parent))) { assert(0); return; }
		PL_(filler)(link);
		L_(ListPush)(&list, link);
	}
	printf("Result: %s.\n", L_(ListToString)(&list));
	link = L_(ListFirst)(&list);
	assert(link);
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
	L_(test_basic)(parent_new, parent);
}

#undef QUOTE
#undef QUOTE_
