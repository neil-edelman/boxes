#if defined(QUOTE) || defined(QUOTE_)
#error QUOTE_? cannot be defined.
#endif
#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)

#if LIST_TRAITS == 0 /* <!-- !traits */

#include <stdlib.h>	/* EXIT rand */
#include <stdio.h>  /* printf */

/* `LIST_TEST` must be a function implementing <typedef:<PL>action_fn>. */
static void (*const PL_(filler))(struct L_(listlink) *) = (LIST_TEST);

static const char *PL_(colour);
static size_t PL_(offset); /* The list's offset to the parent. */

/** Names `l`. `dir` is either 0, it names the node, or positive/negative to
 name edges. */
static char *PL_(name)(const struct L_(listlink) *const l) {
	static char z[8][64];
	static unsigned n;
	char *y = z[n];
	n = (n + 1) % (sizeof z / sizeof *z);
	assert(l);
	/* Normal or sentinel. */
	if(l->prev && l->next) {
		const void *node = (const void *)((const char *)l - PL_(offset));
		sprintf(y, "n%p", node);
	} else {
		sprintf(y, "list_%s:%s", PL_(colour), l->next ? "head" : "tail");
	}
	return y;
}

/** Print `list` to `fp`.
 @param[colour] A colour that can also have a 4 appended; _eg_ "royalblue",
 "firebrick", "darkseagreen", "orchid".
 @param[offset] For printing multiple lists, offset to the parent type.
 @param[is_nodes] Print nodes; if one is printing the same list, different
 order, then this would be off. */
static void PL_(subgraph)(const struct L_(list) *const list, FILE *const fp,
	const char *const colour, const size_t offset, const int is_nodes) {
	struct L_(listlink) *link;
	char a[12];
	assert(list && fp && colour);
	PL_(colour) = colour;
	PL_(offset) = offset;
	fprintf(fp, "\tlist_%s [label=<\n"
		"<TABLE BORDER=\"0\">\n"
		"\t<TR><TD ALIGN=\"LEFT\"><FONT COLOR=\"Gray85\">&lt;" QUOTE(LIST_NAME)
		"&gt;list</FONT></TD></TR>\n"
		"\t<TR><TD PORT=\"tail\" BORDER=\"0\" ALIGN=\"LEFT\""
		" BGCOLOR=\"Gray90\">tail</TD></TR>\n"
		"\t<TR><TD PORT=\"head\" BORDER=\"0\" ALIGN=\"LEFT\">"
		"head</TD></TR>\n"
		"</TABLE>>];\n", PL_(colour));
	assert(list->u.flat.next && !list->u.flat.zero && list->u.flat.prev);
	if(!list->u.flat.next->prev) { /* Empty: drawing has to make an exception. */
		assert(!list->u.flat.prev->next);
		fprintf(fp, "\tlist_%s:tail -> list_%s:head"
			" [color=\"%s4\", style=\"dotted\", arrowhead=\"empty\"];\n"
			"\tlist_%s:head -> list_%s:tail [color=\"%s\"];\n",
			PL_(colour), PL_(colour), PL_(colour),
			PL_(colour), PL_(colour), PL_(colour));
	} else {
		fprintf(fp, "\tlist_%s:tail -> %s"
			" [color=\"%s4\", style=\"dotted\", arrowhead=\"empty\"];\n"
			"\tlist_%s:head -> %s [color=\"%s\"];\n",
			PL_(colour), PL_(name)(list->u.flat.prev), colour,
			PL_(colour), PL_(name)(list->u.flat.next), colour);
	}
	for(link = L_(list_head)(list); link; link = L_(list_next)(link)) {
		if(is_nodes) {
			PL_(to_string)(link, &a);
			fprintf(fp, "\t%s [label=\"%s\"];\n", PL_(name)(link), a);
		}
		fprintf(fp, "\t%s -> %s [color=\"%s\"];\n"
			"\t%s -> %s [color=\"%s4\", style=\"dotted\","
			" arrowhead=\"empty\"];\n",
			PL_(name)(link), PL_(name)(link->next), colour,
			PL_(name)(link), PL_(name)(link->prev), colour);
	}
}

/** Graph `list` in `fn`. */
static void PL_(graph)(const struct L_(list) *const list, const char *const fn)
{
	FILE *fp;
	assert(list && fn);
	if(!(fp = fopen(fn, "w"))) { perror(fn); return; }
	printf("*** Opening graph \"%s\".\n", fn);
	fprintf(fp, "digraph {\n"
		"\tgraph [truecolor=true, bgcolor=transparent, fontface=modern];\n"
		"\tnode [shape=box, style=filled, fillcolor=\"Gray95\"];\n");
	PL_(subgraph)(list, fp, "royalblue", 0, 1);
	fprintf(fp, "\tnode [colour=\"Red\"];\n"
		"}\n");
	fclose(fp);
}

/** Perform "Floyd's" tortoise-hare algorithm for cycle detection for the list
 on which `link` is a part and expect `count`. `list` must have at least one
 element, (it can't be the head of tail.)
 @order \O(|`list`|) */
static void PL_(floyd)(const struct L_(listlink) *link, const size_t count) {
	size_t fw = 0, b1 = 0, b2 = 0;
	const struct L_(listlink) *hare = link, *turtle = hare;
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
	const struct L_(listlink) *const head = &list->u.as_head.head,
		*const tail = &list->u.as_tail.tail, *first;
	assert(list && head && tail && !list->u.flat.zero);
	if((first = head->next) == tail)
		{ assert(tail->prev == head && !count); return; }
	PL_(floyd)(first, count);
}

/** Returns `0,1,0,1,...` whatever `link`. */
static int PL_(parity)(const struct L_(listlink) *const link) {
	static int p;
	(void)(link);
	return !(p = !p);
}

/** Returns true whatever `link`. */
static int PL_(true)(const struct L_(listlink) *const link) {
	(void)(link);
	return 1;
}

/** Passed `parent_new` and `parent`, tests basic functionality. */
static void PL_(test_basic)(struct L_(listlink) *(*const parent_new)(void *),
	void *const parent) {
	struct L_(list) l1, l2;
	struct L_(listlink) *link, *link_first = 0, *link_last = 0;
	const size_t test_size = 10;
	size_t i;
	assert(parent_new && parent);
	L_(list_clear)(&l1), L_(list_clear)(&l2);
	printf("Basic tests of <" QUOTE(LIST_NAME) ">list:\n");
	PL_(count)(&l1, 0);
	/* Test positions null. */
	link = L_(list_head)(&l1), assert(!link);
	link = L_(list_tail)(&l1), assert(!link);
	/* Test returns on null and empty. */
	link = L_(list_shift)(&l1), assert(!link);
	link = L_(list_pop)(&l1), assert(!link);
	/* Add */
	printf("Adding %lu elements to l1.\n", (unsigned long)test_size);
	for(i = 0; i < test_size; i++) {
		char z[12];
		if(!(link = parent_new(parent))) { assert(0); return; }
		PL_(filler)(link);
		PL_(to_string)(link, &z);
		printf("Adding %s.\n", z);
		L_(list_push)(&l1, link);
		if(i == 0) link_first = link;
		link_last = link;
	}
	PL_(graph)(&l1, "graph/" QUOTE(LIST_NAME) "-small.gv");
	PL_(count)(&l1, test_size);
	printf("l1 = %s.\n", PL_(list_to_string)(&l1));
	/* Test positions when contents. */
	link = L_(list_head)(&l1), assert(link == link_first);
	link = L_(list_tail)(&l1), assert(link == link_last);
	link = L_(list_previous)(link), assert(link);
	link = L_(list_next)(link), assert(link == link_last);
	/* Test remove contents. */
	link = L_(list_shift)(&l1), assert(link == link_first);
	link = L_(list_pop)(&l1), assert(link = link_last);
	PL_(count)(&l1, test_size - 2);
	L_(list_unshift)(&l1, link_first);
	L_(list_push)(&l1, link_last);
	PL_(count)(&l1, test_size);
	link = L_(list_head)(&l1), assert(link == link_first);
	link = L_(list_tail)(&l1), assert(link == link_last);
	printf("After removing and adding: l1 = %s.\n", PL_(list_to_string)(&l1));
	/* Test movement. */
	PL_(count)(&l1, test_size);
	PL_(count)(&l2, 0);
	L_(list_to_if)(&l1, &l2, &PL_(parity));
	printf("Transferring all odds: l1 = %s; l2 = %s.\n",
		PL_(list_to_string)(&l1), PL_(list_to_string)(&l2));
	PL_(count)(&l1, test_size / 2);
	PL_(count)(&l2, test_size - test_size / 2);
	assert(L_(list_head)(&l1) == link_first);
	L_(list_to_before)(&l2, link_first->next);
	PL_(count)(&l1, test_size);
	PL_(count)(&l2, 0);
	assert(L_(list_head)(&l1) == link_first);
	printf("Back: l1 = %s; l2 = %s.\n",
		PL_(list_to_string)(&l1), PL_(list_to_string)(&l2));
	L_(list_to)(&l1, &l2);
	PL_(count)(&l1, 0);
	PL_(count)(&l2, test_size);
	assert(L_(list_head)(&l2) == link_first);
	/* Test any. */
	link = L_(list_any)(&l1, &PL_(true)), assert(!link);
	link = L_(list_any)(&l2, &PL_(true)), assert(link == link_first);
	/* Test add before/after. */
	if(!(link = parent_new(parent))) { assert(0); return; }
	PL_(filler)(link);
	L_(list_add_before)(L_(list_head)(&l2), link);
	link_first = L_(list_head)(&l2);
	assert(link == link_first);
	PL_(count)(&l2, test_size + 1);
	if(!(link = parent_new(parent))) { assert(0); return; }
	PL_(filler)(link);
	L_(list_add_before)(L_(list_tail)(&l2), link);
	PL_(count)(&l2, test_size + 2);
	L_(list_clear)(&l2);
	PL_(count)(&l2, 0);
}

/** The linked-list will be tested on stdout. `LIST_TEST` has to be set.
 @param[parent_new, parent] Responsible for creating new objects and returning
 the list. @allow */
static void L_(list_test)(struct L_(listlink) *(*const parent_new)(void *),
	void *const parent) {
	printf("<" QUOTE(LIST_NAME) ">list was created using: "
#ifdef LIST_COMPARE
		"LIST_COMPARE: <" QUOTE(LIST_COMPARE) ">; "
#endif
		"testing:\n");
	PL_(test_basic)(parent_new, parent);
	printf("Done tests of " QUOTE(LIST_NAME) ".\n\n");
}


/* !traits --><!-- compare */
#elif defined(LIST_COMPARE) || defined(LIST_IS_EQUAL)

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
		PL_(filler)(link);
		PL_(to_string)(link, &z);
		printf("link: %s.\n", z);
		printf("empty eg1: %s, as_head %p, as_tail %p.\n", PL_(list_to_string)(&eg1), (void *)&eg1.u.as_head.head,
			(void *)&eg1.u.as_tail.tail);
		L_(list_self_correct)(&eg1);
		printf("empty eg1: %s, as_head %p, as_tail %p.\n", PL_(list_to_string)(&eg1), (void *)&eg1.u.as_head.head,
			(void *)&eg1.u.as_tail.tail);
		printf("empty eg2: %s, as_head %p, as_tail %p.\n", PL_(list_to_string)(&eg2), (void *)&eg2.u.as_head.head,
			(void *)&eg2.u.as_tail.tail);
		L_(list_push)(&eg1, link);
		printf("link in eg1: %s, next %p, prev %p, next.next %p.\n",
			PL_(list_to_string)(&eg1), (void *)eg1.u.flat.next,
			(void *)eg1.u.flat.prev, (void *)eg1.u.flat.next->next);
		/* Intentionally add it to another list! */
		L_(list_push)(&eg2, link);
		printf("link in eg2: %s, next %p, prev %p, next.next %p.\n",
			PL_(list_to_string)(&eg2), (void *)eg2.u.flat.next,
			(void *)eg2.u.flat.prev, (void *)eg2.u.flat.next->next);
		/* Correct back to `eg1`, (`eg2` will now be invalid.) */
		L_(list_self_correct)(&eg1);
		printf("link in eg1: %s, next %p, prev %p, next.next %p.\n",
			PL_(list_to_string)(&eg1), (void *)eg1.u.flat.next,
			(void *)eg1.u.flat.prev, (void *)eg1.u.flat.next->next);
	}
	/* Random lists. */
	for(list = lists; list < lists_end; list++) {
		size_t no_links = (unsigned)rand() / (RAND_MAX / 5 + 1);
		struct L_(listlink) *link, *link_a, *link_b;
		L_(list_clear)(list);
		while(no_links) {
			if(!(link = parent_new(parent))) { assert(0); return; }
			PL_(filler)(link);
			L_(list_push)(list, link);
			no_links--;
		}
		L_(list_sort)(list);
		for(link_a = 0, link_b = L_(list_head)(list); link_b;
			link_a = link_b, link_b = L_(list_next)(link_b)) {
			if(!link_a) continue;
			cmp = PCMP_(compare)(link_a, link_b);
			assert(cmp <= 0);
		}
	}
	/* Now sort the lists. */
	printf("Sorted array of sorted <" QUOTE(LIST_NAME) ">list by "
		QUOTE(LIST_COMPARE) ":\n");
	qsort(lists, lists_size, sizeof *lists, &PCMP_(compar));
	for(list = lists; list < lists_end; list++) {
		L_(list_self_correct)(list); /* `qsort` moves the pointers. */
		printf("list: %s.\n", PL_(list_to_string)(list));
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
static void PL_(exact)(const struct L_(list) *const list,
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
	cmp = L_(list_compare)(&la, 0), assert(cmp > 0);
	cmp = L_(list_compare)(0, &la), assert(cmp < 0);
	/*L_(list_subtraction_to)(0, 0, 0);
	L_(list_subtraction_to)(0, 0, &la);
	L_(list_union_to)(0, 0, 0);
	L_(list_union_to)(0, 0, &la);
	L_(list_intersection_to)(0, 0, 0);
	L_(list_intersection_to)(0, 0, &la);
	L_(list_xor_to)(0, 0, 0);
	L_(list_xor_to)(0, 0, &la);*/
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
			PL_(filler)(link);
			L_(list_push)(&x, link);
			/*L_(list_sort)(&x);
			L_(list_duplicates_to)(&x, &y);*/
			/* fixme: list_duplicates_to is suspect? it keeps giving wrong. */
			printf("x = %s, y = %s\n", PL_(list_to_string)(&x),
				PL_(list_to_string)(&y));
			/* Honestly, what am I doing? */
			/* `x = (A,...,B,C,D,...)` and `y = {[A],B,...}`. */
			if(!(a = L_(list_head)(&x))) continue;
			if(!(b = L_(list_head)(&y))) continue;
			if(PCMP_(compare)(a, b) == 0 && !(b = L_(list_next)(b))) continue;
			assert(PCMP_(compare)(a, b) < 0);
			for(c = L_(list_next)(a); c && PCMP_(compare)(c, b) < 0;
				c = L_(list_next)(c));
			assert(c && PCMP_(compare)(c, b) == 0);
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
#if 0
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
#endif
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
	printf("max: %d\n", RAND_MAX);
	PL_(test_sort)(parent_new, parent);
	PL_(test_binary)(parent_new, parent);
	printf("Done tests of " QUOTE(LIST_NAME) ".\n\n");
}


#else /* compare --><!-- no */
#error Test should not be here.
#endif /* no --> */


#undef QUOTE
#undef QUOTE_

/* We should *not* undef `LIST_TEST`, since it's used to pick the first
 to_string. */
