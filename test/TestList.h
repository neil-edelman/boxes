/* intended to be included by ../src/List.h on LIST_TEST */

#ifndef LIST_U_NAME /* <-- !LIST_U_NAME */

#include <stdlib.h>	/* EXIT_SUCCESS rand */

/* Define macros. */
#ifdef QUOTE
#undef QUOTE
#endif
#ifdef QUOTE_
#undef QUOTE_
#endif
#ifdef T_NAME
#undef T_NAME
#endif
#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)
#define T_NAME QUOTE(LIST_NAME)



/* prototype */
static void PT_(graph)(const struct T_(List) *const this,
	const struct T_(Link) *const array, const size_t array_size,
	const char *const fn);
static void PT_(legit)(const struct T_(List) *const this);
static size_t PT_(count)(struct T_(List) *const this);
#ifdef LIST_SOME_COMPARATOR /* <-- comp */
static int PT_(in_order)(struct T_(List) *const this);
static int PT_(exactly_unordered)(struct T_(List) *const this,
	const size_t n);
#endif /* comp --> */

/* Check that LIST_TEST is a function implementing {<T>Action}, viz,
 {void (*)(T *const)}. */
static void (*const PT_(filler))(T *const) = (LIST_TEST);

/* For \see{PT_U_(exactly, elements)}. */
struct PT_(Verify) {
	size_t i;
	const struct T_(Link) *array;
	size_t array_no;
};
/* For \see{PT_U_(count, unordered)}. */
struct PT_(Order) {
	T *prev;
	size_t count;
};

#ifdef LIST_UA_NAME /* <-- a */
#define LIST_U_NAME LIST_UA_NAME
#ifdef LIST_UA_COMPARATOR /* <-- comp */
#define LIST_U_COMPARATOR LIST_UA_COMPARATOR
#endif /* comp --> */
#include "TestList.h"
#endif /* a --> */

#ifdef LIST_UB_NAME /* <-- b */
#define LIST_U_NAME LIST_UB_NAME
#ifdef LIST_UB_COMPARATOR /* <-- comp */
#define LIST_U_COMPARATOR LIST_UB_COMPARATOR
#endif /* comp --> */
#include "TestList.h"
#endif /* b --> */

#ifdef LIST_UC_NAME /* <-- c */
#define LIST_U_NAME LIST_UC_NAME
#ifdef LIST_UC_COMPARATOR /* <-- comp */
#define LIST_U_COMPARATOR LIST_UC_COMPARATOR
#endif /* comp --> */
#include "TestList.h"
#endif /* c --> */

#ifdef LIST_UD_NAME /* <-- d */
#define LIST_U_NAME LIST_UD_NAME
#ifdef LIST_UD_COMPARATOR /* <-- comp */
#define LIST_U_COMPARATOR LIST_UD_COMPARATOR
#endif /* comp --> */
#include "TestList.h"
#endif /* d --> */

/** The linked-list will be tested on stdout.
 @fixme This tests only a small coverage of code, expand it. */
static void T_(ListTest)(void) {
	printf("List<" T_NAME "> of type <" QUOTE(LIST_TYPE)
		"> was created using: "
#ifdef LIST_TO_STRING
		"TO_STRING<" QUOTE(LIST_TO_STRING) ">; "
#endif
#ifdef LIST_TEST
		"TEST<" QUOTE(LIST_TEST) ">; "
#endif
#ifdef LIST_UA_NAME
		"order A: <" QUOTE(LIST_UA_NAME)
#ifdef LIST_UA_COMPARATOR
		"> comparator <" QUOTE(LIST_UA_COMPARATOR)
#endif
		">; "
#endif
#ifdef LIST_UB_NAME
		"order B: <" QUOTE(LIST_UB_NAME)
#ifdef LIST_UB_COMPARATOR
		"> comparator <" QUOTE(LIST_UB_COMPARATOR)
#endif
		">; "
#endif
#ifdef LIST_UC_NAME
		"order C: <" QUOTE(LIST_UC_NAME)
#ifdef LIST_UC_COMPARATOR
		"> comparator <" QUOTE(LIST_UC_COMPARATOR)
#endif
		">; "
#endif
#ifdef LIST_UD_NAME
		"order D: <" QUOTE(LIST_UD_NAME)
#ifdef LIST_UD_COMPARATOR
		"> comparator <" QUOTE(LIST_UD_COMPARATOR)
#endif
		">; "
#endif
		"testing:\n");
#ifdef LIST_UA_NAME
	PT_UA_(test, list)();
#endif
#ifdef LIST_UB_NAME
	PT_UB_(test, list)();
#endif
#ifdef LIST_UC_NAME
	PT_UC_(test, list)();
#endif
#ifdef LIST_UD_NAME
	PT_UD_(test, list)();
#endif
}

/* test helper functions */

static void PT_(graph)(const struct T_(List) *const this,
	const struct T_(Link) *const array, const size_t array_size,
	const char *const fn) {
	FILE *fp;
	const struct T_(Link) *a;
	char str[12];
	size_t i;
	assert(this && array && fn);
	if(!(fp = fopen(fn, "w"))) { perror(fn); return; }
	fprintf(fp, "digraph {\n"
		"p%p [label=\"head\"];\n"
		"p%p [label=\"tail\"];\n"
		"node [shape=box];\n",
		(const void *)&this->head, (const void *)&this->tail);
	for(i = 0; i < array_size; i++) {
		a = array + i;
		PT_(to_string)(&a->data, &str);
		fprintf(fp, "p%p [label=\"%s\"];\n", (const void *)&a->x, str);
	}
	fprintf(fp, "node [colour=red, style=filled];\n");
#ifdef LIST_UA_NAME
	PT_UA_(graph, index)(this, array, array_size, fp, "royalblue");
#endif
#ifdef LIST_UB_NAME
	PT_UB_(graph, index)(this, array, array_size, fp, "firebrick");
#endif
#ifdef LIST_UC_NAME
	PT_UC_(graph, index)(this, array, array_size, fp, "darkseagreen");
#endif
#ifdef LIST_UD_NAME
	PT_UD_(graph, index)(this, array, array_size, fp, "orchid");
#endif
	fprintf(fp, "}\n");
	fclose(fp);
}

/** Assertion function for seeing if it is in a valid state.
 @order O(n) */
static void PT_(legit)(const struct T_(List) *const this) {
	size_t count = 0, index_count;
	int is_valid = 0;
	assert(this);
#ifdef LIST_UA_NAME /* <-- a */
	index_count = PT_UA_(legit, count)(this);
	if(is_valid) assert(count == index_count);
	else count = index_count, is_valid = 1;
#endif /* a --> */
#ifdef LIST_UB_NAME /* <-- b */
	index_count = PT_UB_(legit, count)(this);
	if(is_valid) assert(count == index_count);
	else count = index_count, is_valid = 1;
#endif /* b --> */
#ifdef LIST_UC_NAME /* <-- c */
	index_count = PT_UC_(legit, count)(this);
	if(is_valid) assert(count == index_count);
	else count = index_count, is_valid = 1;
#endif /* c --> */
#ifdef LIST_UD_NAME /* <-- d */
	index_count = PT_UD_(legit, count)(this);
	if(is_valid) assert(count == index_count);
	else count = index_count, is_valid = 1;
#endif /* d --> */
}

static size_t PT_(count)(struct T_(List) *const this) {
	size_t count = 0, index_count;
	int is_valid = 0;
	assert(this);
#ifdef LIST_UA_NAME
	index_count = PT_UA_(count, elements)(this);
	if(is_valid) assert(count == index_count);
	else count = index_count, is_valid = 1;
#endif
#ifdef LIST_UB_NAME
	index_count = PT_UB_(count, elements)(this);
	if(is_valid) assert(count == index_count);
	else count = index_count, is_valid = 1;
#endif
#ifdef LIST_UC_NAME
	index_count = PT_UC_(count, elements)(this);
	if(is_valid) assert(count == index_count);
	else count = index_count, is_valid = 1;
#endif
#ifdef LIST_UD_NAME
	index_count = PT_UD_(count, elements)(this);
	if(is_valid) assert(count == index_count);
	else count = index_count, is_valid = 1;
#endif
	assert(is_valid);
	return count;
}

#ifdef LIST_SOME_COMPARATOR /* <-- comp */
static int PT_(in_order)(struct T_(List) *const this) {
	assert(this);
	return 1
#ifdef LIST_UA_COMPARATOR
		&& PT_UA_(in, order)(this)
#endif
#ifdef LIST_UB_COMPARATOR
		&& PT_UB_(in, order)(this)
#endif
#ifdef LIST_UC_COMPARATOR
		&& PT_UC_(in, order)(this)
#endif
#ifdef LIST_UD_COMPARATOR
		&& PT_UD_(in, order)(this)
#endif
		;
}
#endif /* comp --> */

#ifdef LIST_SOME_COMPARATOR /* <-- comp */
static int PT_(exactly_unordered)(struct T_(List) *const this,
	const size_t n) {
	assert(this);
	(void)(n);
	return 1
#ifdef LIST_UA_COMPARATOR
		&& PT_UA_(count, unordered)(this) == n
#endif
#ifdef LIST_UB_COMPARATOR
		&& PT_UB_(count, unordered)(this) == n
#endif
#ifdef LIST_UC_COMPARATOR
		&& PT_UC_(count, unordered)(this) == n
#endif
#ifdef LIST_UD_COMPARATOR
		&& PT_UD_(count, unordered)(this) == n
#endif
		;
}
#endif /* comp --> */



/* Un-define all macros. */
#undef QUOTE
#undef QUOTE_
#undef T_NAME





#else /* !LIST_U_NAME --><-- LIST_U_NAME
 Internally #included. */

#ifdef PT_U_
#undef PT_U_
#endif
#ifdef LIST_U_ANONYMOUS /* <-- anon: "empty macro arguments were standardized
in C99" */
#define U_(thing) PCAT(anonymous, thing)
#define T_U_(thing1, thing2) CAT(CAT(LIST_NAME, thing1), thing2)
#define PT_U_(thing1, thing2) PCAT(list, PCAT(PCAT(LIST_NAME, thing1), \
	CAT(_, thing2)))
#else /* anon --><-- !anon */
#define U_(thing) PCAT(LIST_U_NAME, thing)
#define T_U_(thing1, thing2) CAT(CAT(LIST_NAME, thing1), \
	CAT(LIST_U_NAME, thing2))
#define PT_U_(thing1, thing2) PCAT(list, PCAT(PCAT(LIST_NAME, thing1), \
	PCAT(LIST_U_NAME, thing2)))
#endif /* !anon --> */



/* test helper functions that depend on <U> */

static void PT_U_(graph, index)(const struct T_(List) *const this,
	const struct T_(Link) *const array, const size_t array_size,
	FILE *const fp, const char *const colour) {
	const struct T_(Link) *a;
	size_t i;
	assert(this && array && fp && colour);
	/*fprintf(fp, "subgraph %s {\n"
		"style=filled;\n"
		"color=%s;\n"
		"node [style=filled,color=white];\n}\n", colour, colour);*/
	fprintf(fp, "p%p -> p%p [color=%s];\n"
		"p%p -> p%p [color=%s4];\n",
		(const void *)&this->head, (void *)this->head.U_(next), colour,
		(const void *)&this->tail, (void *)this->tail.U_(prev), colour);
	for(i = 0; i < array_size; i++)
		a = array + i, fprintf(fp, "p%p -> p%p [color=%s];\n"
		"p%p -> p%p [color=%s4];\n",
		(const void *)(&a->x), (void *)(a->x.U_(next)), colour,
		(const void *)(&a->x), (void *)(a->x.U_(prev)), colour);
}

/** This checks if the index is valid by counting forward then back.
 @return Count. */
static size_t PT_U_(legit, count)(const struct T_(List) *const this) {
	size_t forward = 0, back = 0;
	struct PT_(X) *turtle, *hare, *next;
	assert(this && !this->head.U_(prev) && this->head.U_(next)
		&& this->tail.U_(prev) && !this->tail.U_(next));
	/* I just discovered the comma operator. Forgive me. */
	/*for(hare = turtle = this->head.U_(next); (next = hare->U_(next))
		&& (hare = next, forward++, turtle = turtle->U_(next),
		next = hare->U_(next)); hare = next, forward++, assert(turtle != hare));
	assert(&this->tail == hare);
	for(hare = turtle = this->tail.U_(prev); (next = hare->U_(prev))
		&& (hare = next, back++, turtle = turtle->U_(prev),
		next = hare->U_(prev)); hare = next, back++, assert(turtle != hare));*/
	hare = turtle = this->head.U_(next);
	while((next = hare->U_(next))) {
		hare = next;
		forward++;
		turtle = turtle->U_(next);
		if(!(next = hare->U_(next))) break;
		hare = next;
		forward++;
		assert(turtle != hare);
	}
	{ const struct PT_(X) *const tail = &this->tail; assert(tail == hare); }
	hare = turtle = this->tail.U_(prev);
	while((next = hare->U_(prev))) {
		hare = next;
		back++;
		turtle = turtle->U_(prev);
		if(!(next = hare->U_(prev))) break;
		hare = next;
		back++;
		assert(turtle != hare);
	}
	assert(&this->head == hare);
	assert(forward == back);
	return forward;
}

/** \see{PT_U_(count, elements)}
 @param param: (size_t *)
 @implements <T>BiPredicate */
static int PT_U_(count, predicate)(T *const this, void *const param) {
	size_t count = *(size_t *)param;
	(void)(this);
	*(size_t *)param = ++count;
	return 1;
}
/** Counts the elements. */
static size_t PT_U_(count, elements)(struct T_(List) *const this) {
	size_t count = 0;
	T_U_(List, BiAll)(this, &PT_U_(count, predicate), &count);
	return count;
}

/* Global count \see{<T>_count_<U>_another}. */
static size_t PT_U_(count, var);
/** @implements <T>Action */
static void PT_U_(count, another)(T *const this) {
	(void)(this);
	PT_U_(count, var)++;
}

/** \see{PT_U_(exactly, elements)}
 @param param: struct <T>List<U>Verify
 @implements <T>Predicate */
static int PT_U_(exactly, predicate)(T *const this, void *const param) {
	struct PT_(Verify) *lv = param;
	if(lv->array_no <= lv->i
		|| memcmp(this, &lv->array[lv->i].data, sizeof *this))
		return fprintf(stderr, "Failed at index %lu.\n", (unsigned long)lv->i),
			0;
	lv->i++;
	return 1;
}
/** Verifies that the elements are exactly as in {array}. */
static size_t PT_U_(exactly, elements)(struct T_(List) *const this,
	const struct T_(Link) *array, const size_t array_no) {
	struct PT_(Verify) lv = { 0, 0, 0 };
	lv.array    = array;
	lv.array_no = array_no;
	return !T_U_(List, BiAll)(this, &PT_U_(exactly, predicate),
		&lv);
}

#ifdef LIST_U_COMPARATOR /* <-- comp */
/** \see{PT_U_(in, order)}.
 @param param: (T *[1]), last element.
 @implements <T>BiPredicate */
static int PT_U_(order, predicate)(T *const this, void *const param) {
	T **prev_one_array = param;
	T *const prev = prev_one_array[0];
	/*char scratch[12];
	PT_(to_string)(data, &scratch), scratch[8] = '\0';
	printf("%s%s", prev ? " <= " : "", scratch);*/
	if(prev && PT_U_(data, cmp)(prev, this) > 0) return 0;
	prev_one_array[0] = this;
	return 1;
}
/** Verifies sorting on index. */
static int PT_U_(in, order)(struct T_(List) *const this) {
	T *one_array[] = { 0 };
	return !T_U_(List, BiAll)(this, &PT_U_(order, predicate),
		one_array);
}

/** \see{PT_U_(count, unordered)}.
 @param param: (struct PT_(Order) *).
 @implements <T>BiPredicate */
static int PT_U_(unorder, predicate)(T *const this, void *const param) {
	struct PT_(Order) *info = param;
	char a[12], b[12];
	if(info->prev && PT_U_(data, cmp)(info->prev, this) > 0)
		printf("Unorder %lu: %s > %s\n", (unsigned long)(++info->count),
		(PT_(to_string)(info->prev, &a), a), (PT_(to_string)(this, &b), b));
	info->prev = this;
	return 1;
}
/** How many of them are not in order? */
static size_t PT_U_(count, unordered)(struct T_(List) *this) {
	struct PT_(Order) info = { 0, 0 };
	printf("Unordered(" QUOTE(LIST_NAME) "-" QUOTE(LIST_U_NAME) ": %s)\n",
		T_U_(List, ToString)(this));
	T_U_(List, BiAll)(this, &PT_U_(unorder, predicate), &info);
	return info.count;
}
#endif /* comp --> */

#if 0
/** All elements are in the same array? */
static int PT_U_(in, array)(struct T_(List) *const this,
	const struct T_(Link) *const array, const size_t array_size) {
	struct T_(Link) *item;
	for(item = this->U_(head); item; item = item->U_(next))
		if(item < array || item >= array + array_size) return 0;
	return 1;
}
#endif

/** Returns true. Used in \see{PT_U_(test, basic)}.
 @implements <T>Predicate */
static int PT_U_(true, index)(const T *const this) {
	(void)(this);
	return 1;
}
/** Used in \see{PT_U_(test, basic)}.
 @param param: (int *), boolean.
 @implements <T>BiPredicate */
static int PT_U_(every, second)(T *const this, void *const param) {
	int *const pbinary = param;
	(void)(this);
	return !(*pbinary = !*pbinary);
}



/* now tests */

static void PT_U_(test, basic)(void) {
	char str[12];
	T *data, *item_a, *item_b, *item_y, *item_z;
	struct T_(List) a;
	struct T_(Link) buf[100/*0*/], *const new_buf = buf + 2, *node;
	const size_t buf_size = sizeof buf / sizeof *buf, new_buf_size = buf_size-4;
	size_t i;
	int is_parity = 1;

	printf("Basic tests of " QUOTE(LIST_NAME) " linked-list " QUOTE(LIST_U_NAME)
		":\n");
	/* {valgrind}? */
	memset(buf, 0, sizeof buf);
	/* Clear */
	T_(ListClear)(0);
	T_(ListClear)(&a);
	printf("Adding %lu elements to a.\n", (unsigned long)buf_size);
	/* Add */
	T_(ListPush)(0, 0);
	T_(ListPush)(&a, 0);
	node = buf;
	PT_(filler)(&node->data);
	T_(ListPush)(0, &node->data);
	for(i = 0; i < buf_size; i++) {
		node = buf + i;
		PT_(filler)(&node->data);
		T_(ListPush)(&a, &node->data);
	}
	item_a = T_U_(List, First)(&a);
	assert(item_a);
	data = item_a;
	assert(data);
	PT_(legit)(&a);
	PT_(to_string)(data, &str);
	printf("Link get first data: %s.\n", str);
	assert(memcmp(&buf[0].data, data, sizeof *data) == 0);
	/* All */
	printf("All: for all true returns null.\n");
	assert(!T_U_(List, All)(0, 0));
	assert(!T_U_(List, All)(0, &PT_U_(true, index)));
	assert(!T_U_(List, All)(&a, 0));
	assert(!T_U_(List, All)(&a, &PT_U_(true, index)));
	printf("All: parity [ 1, 0, 1, ... ] ends on index 1.\n");
	/* BiAll */
	assert(!T_U_(List, BiAll)(0, 0, 0));
	assert(!T_U_(List, BiAll)(&a, 0, 0));
	assert(!T_U_(List, BiAll)(0, &PT_U_(every, second), 0));
	is_parity = 1;
	assert(T_U_(List, BiAll)(&a, &PT_U_(every, second),
		&is_parity) == (T *)(buf + 1));
	/* Next, Previous, First, Last */
	printf("Removing 3 elements from a.\n");
	assert(PT_U_(exactly, elements)(&a, buf, buf_size));
	assert(!T_U_(List, First)(0));
	assert(!T_U_(List, Last)(0));
	assert(!T_U_(List, Previous)(0));
	assert(!T_U_(List, Next)(0));
	item_a = T_U_(List, First)(&a);
	assert(!T_U_(List, Previous)(item_a));
	item_b = T_U_(List, Next)(item_a);
	item_z = T_U_(List, Last)(&a);
	assert(!T_U_(List, Next)(item_z));
	item_y = T_U_(List, Previous)(item_z);
	assert(item_a && item_b && item_y && item_z);
	/* Remove */
	/*T_(ListRemove)(0, item_y);*/
	T_(ListRemove)(item_y);
	T_(ListRemove)(item_z);
	T_(ListRemove)(item_b);
	T_(ListRemove)(item_a);
	assert(PT_U_(exactly, elements)(&a, new_buf, new_buf_size));
	(void)(new_buf), (void)(new_buf_size);
	/* ForEach */
	printf("Counting %lu elements.\n", (unsigned long)new_buf_size);
	PT_U_(count, var) = 0;
	T_U_(List, ForEach)(0, 0);
	T_U_(List, ForEach)(0, &PT_U_(count, another));
	T_U_(List, ForEach)(&a, 0);
	T_U_(List, ForEach)(&a, &PT_U_(count, another));
	assert(PT_U_(count, var) == new_buf_size);
	assert(PT_U_(count, elements)(&a) == new_buf_size);
	PT_(legit)(&a);
#ifdef LIST_SOME_COMPARATOR /* <-- comp */
	/* <U>Sort */
	printf("Sorting a only by " QUOTE(LIST_U_NAME) ".\n");
	T_U_(List, Sort)(0);
	T_U_(List, Sort)(&a);
	assert(PT_U_(in, order)(&a));
	/* Sort */
	T_(ListSort)(0);
	T_(ListSort)(&a);
	printf("Sorting, a = %s.\n", T_U_(List, ToString)(&a));
	assert(PT_(in_order)(&a));
#endif /* comp --> */
	/* ToString (unchecked) */
	printf("ToString: null = %s; a = %s.\n",
		T_U_(List, ToString)(0), T_U_(List, ToString)(&a));
	/* Output graphviz. */
	PT_(graph)(&a, buf, buf_size,
		"graph/basic-" QUOTE(LIST_NAME) "-" QUOTE(LIST_U_NAME) ".gv");
	printf("Clear.\n");
	T_(ListClear)(&a);
	assert(!PT_U_(count, elements)(&a));
	PT_(legit)(&a);
	printf("\n");
}

static void PT_U_(test, memory)(void) {
	struct T_(List) a, b;
	struct T_(Link) buf[3][10], *node;
	const size_t buf_size = sizeof buf[0] / sizeof *buf[0];
	size_t i;
	int is_parity;

	printf("Memory moving tests of " QUOTE(LIST_NAME) " linked-list "
		QUOTE(LIST_U_NAME) ":\n");
	T_(ListClear)(&a), T_(ListClear)(&b);
	PT_(legit)(&a);
	/* {valgrind}? */
	memset(buf, 0, sizeof buf);
	/* fill the items in buf0 */
	for(i = 0; i < buf_size; i++)
		node = buf[0] + i, PT_(filler)(&node->data);
	/* copy the items to buf1; buf2 zeroed */
	memcpy(buf[1], buf[0], buf_size * sizeof *buf[0]);
	memset(&buf[2], 0, buf_size * sizeof *buf[2]);
	/* put all items in buf0 and buf1 */
	for(i = 0; i < buf_size; i++) T_(ListPush)(&a, &buf[0][i].data);
	PT_(legit)(&a);
	for(i = 0; i < buf_size; i++) {
		T_(ListPush)(&a, &buf[1][i].data);
		PT_(legit)(&a);
	}
	assert(PT_(count)(&a) == buf_size << 1);
#ifdef LIST_SOME_COMPARATOR /* <-- comp */
	T_(ListSort)(&a);
	PT_(legit)(&a);
	printf("Sorting, (backed by two arrays,) a = %s.\n",
		T_U_(List, ToString)(&a));
	assert(PT_(in_order)(&a));
#endif /* comp --> */
	/* now add all of the odd to list_b, remove all the even from list_a */
	printf("Spliting odd/even a to b = %s by " QUOTE(LIST_U_NAME) ";\n",
		T_U_(List, ToString)(&b));
	is_parity = 0;
	PT_(legit)(&a);
	PT_(legit)(&b);
	T_U_(List, BiTakeIf)(&b, &a, &PT_U_(every, second), &is_parity);
	printf("a = %s, b = %s.\n",
		T_U_(List, ToString)(&a), T_U_(List, ToString)(&b));
#ifdef LIST_SOME_COMPARATOR /* <-- comp */
	assert(PT_(in_order)(&a));
	assert(PT_U_(in, order)(&b)); /* only <U> is in order */
#endif /* comp --> */
#ifdef LIST_U_COMPARATOR /* <-- comp */
	/* Compare */
	assert(!T_U_(List, Compare)(0, 0));
	assert(T_U_(List, Compare)(&a, 0) > 0);
	assert(T_U_(List, Compare)(0, &b) < 0);
	assert(!T_U_(List, Compare)(&a, &b));
#endif /* comp --> */
	/* Test done. */
	T_U_(List, TakeIf)(&b, &a, 0);
	printf("Moving all a to b; a = %s, b = %s.\n",
		T_U_(List, ToString)(&a), T_U_(List, ToString)(&b));
	assert(PT_U_(count, elements)(&a) == 0);
	assert(PT_U_(count, elements)(&b) == 2 * buf_size);
	/* Clear (the rest) */
	printf("Clear b.\n");
	T_(ListClear)(&b);
	assert(PT_U_(count, elements)(&b) == 0);
	/* Test done; move. */
	for(i = 0; i < buf_size; i++)
		T_(ListPush)(&a, &buf[0][i].data), T_(ListPush)(&b, &buf[1][i].data);
	assert(PT_U_(exactly, elements)(&a, buf[0], buf_size));
	assert(PT_U_(exactly, elements)(&b, buf[1], buf_size));
#ifdef LIST_U_COMPARATOR /* <-- comp */
	assert(!T_U_(List, Compare)(&a, &b));
#endif /* comp --> */
#if 0
	/* this is not what ListMigrate does anymore */
	for(i = 0; i < buf_size; i += 2) {
		node_b = buf[1] + i;
		node_c = buf[2] + i;
		memcpy(node_c, node_b, sizeof *node_b);
		memset(node_b, 0, sizeof *node_b);
		T_(ListMigrate)(&b, node_c);
	}
	printf("Testing memory relocation with <" QUOTE(LIST_NAME)
		">ListMigrate: a = %s, b = %s.\n",
		T_U_(List, ToString)(&a), T_U_(List, ToString)(&b));
#ifdef LIST_U_COMPARATOR /* <-- comp */
	assert(!T_U_(List, Compare)(&a, &b));
#endif /* comp --> */
	assert(PT_(in_array)(&a, buf[0], buf_size));
	assert(PT_(in_array)(&b, buf[1], buf_size << 1));
	printf("\n");
#endif
}

#ifdef LIST_U_COMPARATOR /* <-- compare */

static void PT_U_(test, sort)(void) {
	struct T_(Link) buf[4], *node;
	const size_t buf_size = sizeof buf / sizeof *buf;
	size_t i, count;
	struct T_(List) a;
	printf("Sort test of " QUOTE(LIST_NAME) " linked-list "
		QUOTE(LIST_U_NAME) ":\n");
	T_(ListClear)(&a);
	for(i = 0; i < buf_size; i++) {
		node = buf + i;
		PT_(filler)(&node->data);
		T_(ListPush)(&a, &node->data);
	}
	count = PT_(count)(&a);
	assert(count == buf_size);
	T_U_(List, Sort)(&a);
	count = PT_(count)(&a);
	assert(count == buf_size);
	PT_(legit)(&a);
}

static void PT_U_(test, boolean)(void) {
	struct T_(List) a, b, c, ia, ib, ic;
	struct Test { struct T_(Link) a, b, ia, ib; char str[12]; } x[3];
	unsigned i; /* for not-getting into an infty loop */
	const unsigned limit = 1000;

	printf("Boolean sequence operations on " QUOTE(LIST_NAME) " linked-list "
		QUOTE(LIST_U_NAME) ":\n");
	/* distinct elements x, y, z */
	PT_(filler)(&x[0].a.data), PT_(to_string)(&x[0].a.data, &x[0].str);
	memcpy(&x[0].b.data,  &x[0].a.data, sizeof x[0].a.data);
	memcpy(&x[0].ia.data, &x[0].a.data, sizeof x[0].a.data);
	memcpy(&x[0].ib.data, &x[0].a.data, sizeof x[0].a.data);
	i = 0; do { PT_(filler)(&x[1].a.data); i++; }
	while(i < limit && !PT_U_(data, cmp)(&x[0].a.data, &x[1].a.data));
	assert(i < limit); /* <- need to get more variety in {LIST_TEST} filler */
	PT_(to_string)(&x[1].a.data, &x[1].str);
	memcpy(&x[1].b.data,  &x[1].a.data, sizeof x[1].a.data);
	memcpy(&x[1].ia.data, &x[1].a.data, sizeof x[1].a.data);
	memcpy(&x[1].ib.data, &x[1].a.data, sizeof x[1].a.data);
	i = 0; do { PT_(filler)(&x[2].a.data); i++; }
	while(i < limit
		&& (!PT_U_(data, cmp)(&x[0].a.data, &x[2].a.data) || !PT_U_(data, cmp)(&x[1].a.data, &x[2].a.data)));
	assert(i < limit); /* <- need to get more variety in {LIST_TEST} filler */
	PT_(to_string)(&x[2].a.data, &x[2].str);
	memcpy(&x[2].b.data,  &x[2].a.data, sizeof x[2].a.data);
	memcpy(&x[2].ia.data, &x[2].a.data, sizeof x[2].a.data);
	memcpy(&x[2].ib.data, &x[2].a.data, sizeof x[2].a.data);
	printf("Three distinct " QUOTE(LIST_U_NAME) "-elements: "
		"%s, %s, %s.\n", x[0].str, x[1].str, x[2].str);
	/* add to the sequences a, b */
	T_(ListClear)(&a), T_(ListClear)(&b), T_(ListClear)(&c);
	T_(ListClear)(&ia), T_(ListClear)(&ib), T_(ListClear)(&ic);
	T_(ListPush)(&a, &x[0].a.data), T_(ListPush)(&a, &x[1].a.data);
	T_(ListPush)(&b, &x[0].b.data), T_(ListPush)(&b, &x[2].b.data);
	printf("Two " QUOTE(LIST_U_NAME) "-sequences, %s and %s.\n",
		   T_U_(List, ToString)(&a), T_U_(List, ToString)(&b));
	/* verify comparing things first */
	printf("Comparing-" QUOTE(LIST_U_NAME) " a = %s and c = %s.\n",
		T_U_(List, ToString)(&a), T_U_(List, ToString)(&c));
	assert(T_U_(List, Compare)(&a, &c) == 1);
	assert(T_U_(List, Compare)(&c, &a) == -1);
	assert(T_U_(List, Compare)(&a, &a) == 0);
	/* - */
	printf("(a = %s) - (b = %s) =\n", T_U_(List, ToString)(&a),
		T_U_(List, ToString)(&b));
	T_U_(List, TakeSubtraction)(&c, &a, &b);
	printf("%s; (a = %s, b = %s.)\n", T_U_(List, ToString)(&c),
		T_U_(List, ToString)(&a), T_U_(List, ToString)(&b));
	T_(ListPush)(&ia, &x[0].ia.data);
	T_(ListPush)(&ib, &x[0].ib.data), T_(ListPush)(&ib, &x[2].ib.data);
	T_(ListPush)(&ic, &x[1].ia.data);
	assert(!T_U_(List, Compare)(&a, &ia));
	assert(!T_U_(List, Compare)(&b, &ib));
	assert(!T_U_(List, Compare)(&c, &ic));
	T_(ListClear)(&a), T_(ListClear)(&b), T_(ListClear)(&c);
	T_(ListClear)(&ia), T_(ListClear)(&ib), T_(ListClear)(&ic);
	/* u */
	T_(ListPush)(&a, &x[0].a.data), T_(ListPush)(&a, &x[1].a.data);
	T_(ListPush)(&b, &x[0].b.data), T_(ListPush)(&b, &x[2].b.data);
	printf("(a = %s) u (b = %s) =\n", T_U_(List, ToString)(&a),
		T_U_(List, ToString)(&b));
	T_U_(List, TakeUnion)(&c, &a, &b);
	printf("%s; (a = %s, b = %s.)\n", T_U_(List, ToString)(&c),
		T_U_(List, ToString)(&a), T_U_(List, ToString)(&b));
	T_(ListPush)(&ib, &x[0].ib.data);
	T_(ListPush)(&ic, &x[0].ia.data);
	if(PT_U_(data, cmp)(&x[1].ia.data, &x[2].ib.data) < 0) {
		T_(ListPush)(&ic, &x[1].ia.data), T_(ListPush)(&ic, &x[2].ib.data);
	} else {
		T_(ListPush)(&ic, &x[2].ib.data), T_(ListPush)(&ic, &x[1].ia.data);
	}
	assert(!T_U_(List, Compare)(&a, &ia));
	assert(!T_U_(List, Compare)(&b, &ib));
	assert(!T_U_(List, Compare)(&c, &ic));
	T_(ListClear)(&a), T_(ListClear)(&b), T_(ListClear)(&c);
	T_(ListClear)(&ia), T_(ListClear)(&ib), T_(ListClear)(&ic);
	/* n */
	T_(ListPush)(&a, &x[0].a.data), T_(ListPush)(&a, &x[1].a.data);
	T_(ListPush)(&b, &x[0].b.data), T_(ListPush)(&b, &x[2].b.data);
	printf("(a = %s) n (b = %s) =\n", T_U_(List, ToString)(&a),
		T_U_(List, ToString)(&b));
	T_U_(List, TakeIntersection)(&c, &a, &b);
	printf("%s; (a = %s, b = %s.)\n", T_U_(List, ToString)(&c),
		T_U_(List, ToString)(&a), T_U_(List, ToString)(&b));
	T_(ListPush)(&ia, &x[1].ia.data);
	T_(ListPush)(&ib, &x[0].ib.data), T_(ListPush)(&ib, &x[2].ib.data);
	T_(ListPush)(&ic, &x[0].ia.data);
	assert(!T_U_(List, Compare)(&a, &ia));
	assert(!T_U_(List, Compare)(&b, &ib));
	assert(!T_U_(List, Compare)(&c, &ic));
	T_(ListClear)(&a), T_(ListClear)(&b), T_(ListClear)(&c);
	T_(ListClear)(&ia), T_(ListClear)(&ib), T_(ListClear)(&ic);
	/* xor */
	T_(ListPush)(&a, &x[0].a.data), T_(ListPush)(&a, &x[1].a.data);
	T_(ListPush)(&b, &x[0].b.data), T_(ListPush)(&b, &x[2].b.data);
	printf("(a = %s) xor (b = %s) =\n", T_U_(List, ToString)(&a),
		T_U_(List, ToString)(&b));
	T_U_(List, TakeXor)(&c, &a, &b);
	printf("%s; (a = %s, b = %s.)\n", T_U_(List, ToString)(&c),
		T_U_(List, ToString)(&a), T_U_(List, ToString)(&b));
	T_(ListPush)(&ia, &x[0].ia.data);
	T_(ListPush)(&ib, &x[0].ib.data);
	if(PT_U_(data, cmp)(&x[1].ia.data, &x[2].ib.data) < 0) {
		T_(ListPush)(&ic, &x[1].ia.data), T_(ListPush)(&ic, &x[2].ib.data);
	} else {
		T_(ListPush)(&ic, &x[2].ib.data), T_(ListPush)(&ic, &x[1].ia.data);
	}
	assert(!T_U_(List, Compare)(&a, &ia));
	assert(!T_U_(List, Compare)(&b, &ib));
	assert(!T_U_(List, Compare)(&c, &ic));
	printf("\n");
}

static void PT_U_(test, order)(void) {
	struct T_(Link) buf[10000], *node = buf;
	const size_t buf_size = sizeof buf / sizeof *buf;
	struct T_(List) a, b;
	size_t i, count;
	T_(ListClear)(&a), T_(ListClear)(&b);
	assert(T_U_(List, Compare)(0, 0) == 0);
	assert(T_U_(List, Compare)(&a, 0) > 0);
	assert(T_U_(List, Compare)(0, &b) < 0);
	assert(T_U_(List, Compare)(&a, &b) == 0);
	for(i = 0; i < buf_size; i++) node = buf + i, PT_(filler)(&node->data);
	for(i = 0; i < buf_size >> 1; i++) T_(ListPush)(&a, &(node--)->data);
	assert(T_U_(List, Compare)(&a, &b) > 0);
	for( ; i < buf_size; i++) T_(ListPush)(&b, &(node--)->data);
	assert(PT_U_(count, elements)(&a) == buf_size >> 1);
	assert(PT_U_(count, elements)(&b) == buf_size - (buf_size >> 1));
	T_(ListSort)(&a), T_(ListSort)(&b);
	T_(ListTake)(&a, &b);
	printf("Testing " QUOTE(LIST_NAME) "-" QUOTE(LIST_U_NAME) " a, b for order "
		"on <" QUOTE(LIST_NAME) ">ListTake(a, b).\n");
	assert(PT_U_(count, elements)(&a) == buf_size);
	assert(PT_U_(count, elements)(&b) == 0);
	/* \${(1/2)^(buf_size/2+1)} chance of getting a false positive! \/ */
	assert(PT_(exactly_unordered)(&a, (size_t)1));
	/* done now merge */
	T_(ListClear)(&a), T_(ListClear)(&b);
	node = buf;
	for(i = 0; i < buf_size >> 1; i++) T_(ListPush)(&a, &(node++)->data);
	for( ; i < buf_size; i++) T_(ListPush)(&b, &(node++)->data);
	T_(ListSort)(&a), T_(ListSort)(&b);
	assert(PT_U_(count, elements)(&a) == buf_size >> 1);
	assert(PT_U_(count, elements)(&b) == buf_size - (buf_size >> 1));
	assert(PT_(in_order)(&a));
	assert(PT_(in_order)(&b));
	PT_(legit)(&a);
	PT_(legit)(&b);
	printf("Testing " QUOTE(LIST_NAME) "-" QUOTE(LIST_U_NAME) " a, b for order "
		"on <" QUOTE(LIST_NAME) ">ListMerge(a, b): %s, %s.\n",
		T_U_(List, ToString)(&a), T_U_(List, ToString)(&b));
#ifdef LIST_UA_COMPARATOR
	printf("By " QUOTE(LIST_UA_NAME) ": a = %s, b = %s.\n",
		T_UA_(List, ToString)(&a), T_UA_(List, ToString)(&b));
#endif
#ifdef LIST_UB_COMPARATOR
	printf("By " QUOTE(LIST_UB_NAME) ": a = %s, b = %s.\n",
		T_UB_(List, ToString)(&a), T_UB_(List, ToString)(&b));
#endif
#ifdef LIST_UC_COMPARATOR
	printf("By " QUOTE(LIST_UC_NAME) ": a = %s, b = %s.\n",
		T_UC_(List, ToString)(&a), T_UC_(List, ToString)(&b));
#endif
#ifdef LIST_UD_COMPARATOR
	printf("By " QUOTE(LIST_UD_NAME) ": a = %s, b = %s.\n",
		T_UD_(List, ToString)(&a), T_UD_(List, ToString)(&b));
#endif
	PT_(graph)(&a, buf, buf_size >> 1,
		"graph/order-" QUOTE(LIST_NAME) "-" QUOTE(LIST_U_NAME) "-a.gv");
	PT_(graph)(&b, buf + (buf_size >> 1), buf_size - (buf_size >> 1),
		"graph/order-" QUOTE(LIST_NAME) "-" QUOTE(LIST_U_NAME) "-b.gv");
	T_(ListMerge)(&a, &b);
	PT_(graph)(&a, buf, buf_size,
		"graph/order-" QUOTE(LIST_NAME) "-" QUOTE(LIST_U_NAME) "-merged-a.gv");
	PT_(legit)(&a); /* <-fails/\ */
	PT_(legit)(&b);
	printf("Testing " QUOTE(LIST_NAME) "-" QUOTE(LIST_U_NAME) " a, b for order "
		"on <" QUOTE(LIST_NAME) ">ListMerge(a, b).\n");
	count = PT_U_(count, elements)(&a);
	assert(count == buf_size);
	count = PT_U_(count, elements)(&b);
	assert(count == 0);
	assert(PT_(in_order)(&a));
	printf("\n");
}

static void PT_U_(test, meta)(void) {
	struct T_(Link) nodes[256], *node = nodes;
	const size_t nodes_size = sizeof nodes / sizeof *nodes;
	struct T_(List) lists[32];
	const size_t lists_size = sizeof lists / sizeof *lists;
	size_t i, nodes_left = nodes_size, lists_left = lists_size;

	printf("An array of lists of " T_NAME ".\n");
	for(i = 0; i < nodes_size; i++) PT_(filler)(&nodes[i].data);
	while(lists_left) {
		struct T_(List) *const list = lists + lists_size - lists_left;
		int take = (int)((double)nodes_left / lists_left
			+ 0.4 * (lists_left - 1) * (2.0 * rand() / (1.0 + RAND_MAX) - 1.0));
		if(take < 0) take = 0;
		else if((size_t)take > nodes_left) take = (int)nodes_left;
		nodes_left -= (size_t)take;
		T_(ListClear)(list);
		while(take) T_(ListPush)(list, &(node++)->data), take--;
		lists_left--;
		printf("%lu. %s\n", (unsigned long)(lists_size - lists_left),
			T_U_(List, ToString)(list));
	}
	printf("Sorting with qsort by " QUOTE(LIST_U_NAME) ".\n");
	qsort(lists, lists_size, sizeof *lists,
		(int (*)(const void *, const void *))&T_U_(List, Compare));
	for(i = 0; i < lists_size; i++) {
		printf("%lu. %s\n", (unsigned long)(i + 1),
			T_U_(List, ToString)(lists + i));
		if(i) { /* like {strcmp} comparing the first letter -- good enough */
			struct PT_(X) *const less = lists[i - 1].head.U_(next),
				*const more = lists[i].head.U_(next);
			assert(less);
			if(!less->U_(next)) continue;
			assert(more && more->U_(next)), (void)(more);
			assert(PT_U_(data, cmp)(&PT_(node_holds_x)(less)->data,
				&PT_(node_holds_x)(more)->data) <= 0);
		}
	}
	printf("\n");
}

#endif /* compare --> */

/* all list tests */
static void PT_U_(test, list)(void) {
	printf("List<" QUOTE(LIST_NAME) "> linked-list "
		   QUOTE(LIST_U_NAME) ":\n");
	{
		struct T_(List) a;
		struct T_(Link) nodes[5];
		const size_t nodes_size = sizeof nodes / sizeof *nodes;
		size_t i, count;
		T_(ListClear)(&a);
		for(i = 0; i < nodes_size; i++) {
			PT_(filler)(&nodes[i].data);
			T_(ListPush)(&a, &nodes[i].data);
		}
		count = PT_(count)(&a);
		assert(count == nodes_size);
		PT_(graph)(&a, nodes, nodes_size,
			"graph/test-" QUOTE(LIST_NAME) "-" QUOTE(LIST_U_NAME) ".gv");
	}
#ifdef LIST_U_COMPARATOR /* <-- compare */
	PT_U_(test, sort)();
#endif /* compare --> */
	PT_U_(test, basic)();
	PT_U_(test, memory)();
#ifdef LIST_U_COMPARATOR /* <-- compare */
	PT_U_(test, boolean)();
	PT_U_(test, order)();
	PT_U_(test, meta)();
#endif /* compare --> */
}



/* undefine stuff for the next */
#undef LIST_U_NAME
#ifdef LIST_U_COMPARATOR /* <-- comp */
#undef LIST_U_COMPARATOR
#endif /* comp --> */

#endif /* LIST_U_NAME --> */
