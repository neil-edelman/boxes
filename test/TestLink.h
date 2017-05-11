/* intended to be included by ../src/Link.h on LINK_TEST */

#ifndef LINK_L_NAME /* <-- !LINK_L_NAME */

#include <stdlib.h>	/* EXIT_SUCCESS rand */



/* prototype */
#ifdef LINK_SOME_COMPARATOR /* <-- comp */
static int PRIVATE_T_(in_order)(struct T_(Link) *const this);
#endif /* comp --> */
static int PRIVATE_T_(in_array)(struct T_(Link) *const this,
	const struct T_(LinkNode) *const array, const size_t array_size);
#ifdef LINK_SOME_COMPARATOR /* <-- comp */
static int PRIVATE_T_(exactly_unordered)(struct T_(Link) *const this,
	const size_t n);
#endif /* comp --> */

/* Check that LINK_TEST is a function implementing {<T>Action}. */
static const T_(Action) PRIVATE_T_(filler) = (LINK_TEST);

/* For \see{PRIVATE_T_L_(exactly, elements)}. */
struct PRIVATE_T_(Verify) {
	size_t i;
	const struct T_(LinkNode) *array;
	size_t array_no;
};
/* For \see{PRIVATE_T_L_(count, unordered)}. */
struct PRIVATE_T_(Order) {
	T *prev;
	size_t count;
};

#ifdef LINK_A_NAME /* <-- a */
#define LINK_L_NAME LINK_A_NAME
#ifdef LINK_A_COMPARATOR /* <-- comp */
#define LINK_L_COMPARATOR LINK_A_COMPARATOR
#endif /* comp --> */
#include "TestLink.h"
#endif /* a --> */

#ifdef LINK_B_NAME /* <-- b */
#define LINK_L_NAME LINK_B_NAME
#ifdef LINK_B_COMPARATOR /* <-- comp */
#define LINK_L_COMPARATOR LINK_B_COMPARATOR
#endif /* comp --> */
#include "TestLink.h"
#endif /* b --> */

#ifdef LINK_C_NAME /* <-- c */
#define LINK_L_NAME LINK_C_NAME
#ifdef LINK_C_COMPARATOR /* <-- comp */
#define LINK_L_COMPARATOR LINK_C_COMPARATOR
#endif /* comp --> */
#include "TestLink.h"
#endif /* c --> */

#ifdef LINK_D_NAME /* <-- d */
#define LINK_L_NAME LINK_D_NAME
#ifdef LINK_D_COMPARATOR /* <-- comp */
#define LINK_L_COMPARATOR LINK_D_COMPARATOR
#endif /* comp --> */
#include "TestLink.h"
#endif /* d --> */

/** The linked-list will be tested on stdout.
 @fixme This tests only a small coverage of code, expand it. */
static void T_(LinkTest)(void) {
	printf("Link<" T_NAME "> of type <" QUOTE(LINK_TYPE)
		"> was created using: "
#ifdef LINK_TO_STRING
		"TO_STRING<" QUOTE(LINK_TO_STRING) ">; "
#endif
#ifdef LINK_TEST
		"TEST<" QUOTE(LINK_TEST) ">; "
#endif
#ifdef LINK_A_NAME
		"list A: <" QUOTE(LINK_A_NAME)
#ifdef LINK_A_COMPARATOR
		"> comparator <" QUOTE(LINK_A_COMPARATOR)
#endif
		">; "
#endif
#ifdef LINK_B_NAME
		"list B: <" QUOTE(LINK_B_NAME)
#ifdef LINK_B_COMPARATOR
		"> comparator <" QUOTE(LINK_B_COMPARATOR)
#endif
		">; "
#endif
#ifdef LINK_C_NAME
		"index C: <" QUOTE(LINK_C_NAME)
#ifdef LINK_C_COMPARATOR
		"> comparator <" QUOTE(LINK_C_COMPARATOR)
#endif
		">; "
#endif
#ifdef LINK_D_NAME
		"index D: <" QUOTE(LINK_D_NAME)
#ifdef LINK_D_COMPARATOR
		"> comparator <" QUOTE(LINK_D_COMPARATOR)
#endif
		">; "
#endif
		"testing:\n");
#ifdef LINK_A_NAME
	PRIVATE_T_LA_(test, list)();
#endif
#ifdef LINK_B_NAME
	PRIVATE_T_LB_(test, list)();
#endif
#ifdef LINK_C_NAME
	PRIVATE_T_LC_(test, list)();
#endif
#ifdef LINK_D_NAME
	PRIVATE_T_LD_(test, list)();
#endif
}

/* test helper functions */

#ifdef LINK_SOME_COMPARATOR /* <-- comp */
static int PRIVATE_T_(in_order)(struct T_(Link) *const this) {
	assert(this);
	return 1
#ifdef LINK_A_COMPARATOR
		&& PRIVATE_T_LA_(in, order)(this)
#endif
#ifdef LINK_B_COMPARATOR
		&& PRIVATE_T_LB_(in, order)(this)
#endif
#ifdef LINK_C_COMPARATOR
		&& PRIVATE_T_LC_(in, order)(this)
#endif
#ifdef LINK_D_COMPARATOR
		&& PRIVATE_T_LD_(in, order)(this)
#endif
		;
}
#endif /* comp --> */

static int PRIVATE_T_(in_array)(struct T_(Link) *const this,
	const struct T_(LinkNode) *const array, const size_t array_size) {
	assert(this);
	assert(array);
	assert(array_size);
	/* overkill; only one would do */
	return 1
#ifdef LINK_A_NAME
		&& PRIVATE_T_LA_(in, array)(this, array, array_size)
#endif
#ifdef LINK_B_NAME
		&& PRIVATE_T_LB_(in, array)(this, array, array_size)
#endif
#ifdef LINK_C_NAME
		&& PRIVATE_T_LC_(in, array)(this, array, array_size)
#endif
#ifdef LINK_D_NAME
		&& PRIVATE_T_LD_(in, array)(this, array, array_size)
#endif
		;
}

#ifdef LINK_SOME_COMPARATOR /* <-- comp */
static int PRIVATE_T_(exactly_unordered)(struct T_(Link) *const this,
	const size_t n) {
	assert(this);
	UNUSED(n);
	return 1
#ifdef LINK_A_COMPARATOR
		&& PRIVATE_T_LA_(count, unordered)(this) == n
#endif
#ifdef LINK_B_COMPARATOR
		&& PRIVATE_T_LB_(count, unordered)(this) == n
#endif
#ifdef LINK_C_COMPARATOR
		&& PRIVATE_T_LC_(count, unordered)(this) == n
#endif
#ifdef LINK_D_COMPARATOR
		&& PRIVATE_T_LD_(count, unordered)(this) == n
#endif
		;
}
#endif /* comp --> */



#else /* !LINK_L_NAME --><-- LINK_L_NAME */



#ifdef PRIVATE_T_L_
#undef PRIVATE_T_L_
#endif
#define PRIVATE_T_L_(thing1, thing2) PCAT(link, PCAT(PCAT(LINK_NAME, thing1), \
	PCAT(LINK_L_NAME, thing2)))



/* test helper functions that depend on <L> */

/** \see{PRIVATE_T_L_(count, elements)}
 @param param: (size_t *)
 @implements <T>Predicate */
static int PRIVATE_T_L_(count, predicate)(T *const this, void *const param) {
	size_t count = *(size_t *)param;
	UNUSED(this);
	*(size_t *)param = ++count;
	return 1;
}
/** Counts the elements. */
static size_t PRIVATE_T_L_(count, elements)(struct T_(Link) *const this) {
	size_t count = 0;
	T_(LinkSetParam)(this, &count);
	T_L_(Link, ShortCircuit)(this, &PRIVATE_T_L_(count, predicate));
	return count;
}

/* Global count \see{<T>_count_<L>_another}. */
static size_t PRIVATE_T_L_(count, var);
/** @implements <T>Action */
static void PRIVATE_T_L_(count, another)(T *const this) {
	UNUSED(this);
	PRIVATE_T_L_(count, var)++;
}

/** \see{PRIVATE_T_L_(exactly, elements)}
 @param param: struct <T>Link<L>Verify
 @implements <T>Predicate */
static int PRIVATE_T_L_(exactly, predicate)(T *const this, void *const param) {
	struct PRIVATE_T_(Verify) *lv = param;
	if(lv->array_no <= lv->i
		|| memcmp(this, &lv->array[lv->i].data, sizeof *this))
		return fprintf(stderr, "Failed at index %lu.\n", (unsigned long)lv->i),
			0;
	lv->i++;
	return 1;
}
/** Verifies that the elements are exactly as in {array}. */
static size_t PRIVATE_T_L_(exactly, elements)(struct T_(Link) *const this,
	const struct T_(LinkNode) *array, const size_t array_no) {
	struct PRIVATE_T_(Verify) lv = { 0, 0, 0 };
	lv.array    = array;
	lv.array_no = array_no;
	T_(LinkSetParam)(this, &lv);
	return !T_L_(Link, ShortCircuit)(this, &PRIVATE_T_L_(exactly, predicate));
}

#ifdef LINK_L_COMPARATOR /* <-- comp */
/** \see{PRIVATE_T_L_(in, order)}.
 @param param: (T *[1]), last element.
 @implements <T>Predicate */
static int PRIVATE_T_L_(order, predicate)(T *const this, void *const param) {
	T **prev_one_array = param;
	T *const prev = prev_one_array[0];
	/*char scratch[12];
	PRIVATE_T_(to_string)(data, &scratch), scratch[8] = '\0';
	printf("%s%s", prev ? " <= " : "", scratch);*/
	if(prev && PRIVATE_T_L_(data, cmp)(prev, this) > 0) return 0;
	prev_one_array[0] = this;
	return 1;
}
/** Verifies sorting on index. */
static int PRIVATE_T_L_(in, order)(struct T_(Link) *const this) {
	T *one_array[] = { 0 };
	T_(LinkSetParam)(this, one_array);
	return !T_L_(Link, ShortCircuit)(this, &PRIVATE_T_L_(order, predicate));
}

/** \see{PRIVATE_T_L_(count, unordered)}.
 @param param: (struct PRIVATE_T_(Order) *).
 @implements <T>Predicate */
static int PRIVATE_T_L_(unorder, predicate)(T *const this, void *const param) {
	struct PRIVATE_T_(Order) *info = param;
	char a[12], b[12];
	if(info->prev && PRIVATE_T_L_(data, cmp)(info->prev, this) > 0)
		printf("Unorder %lu: %s > %s\n", (unsigned long)(++info->count),
		(PRIVATE_T_(to_string)(info->prev, &a), a), (PRIVATE_T_(to_string)(this, &b), b));
	info->prev = this;
	return 1;
}
/** How many of them are not in order?
 @implements <T>Predicate */
static size_t PRIVATE_T_L_(count, unordered)(struct T_(Link) *this) {
	struct PRIVATE_T_(Order) info = { 0, 0 };
	printf("Unordered(" QUOTE(LINK_NAME) "-" QUOTE(LINK_L_NAME) ": %s)\n",
		T_L_(Link, ToString)(this));
	T_(LinkSetParam)(this, &info);
	T_L_(Link, ShortCircuit)(this, &PRIVATE_T_L_(unorder, predicate));
	return info.count;
}
#endif /* comp --> */

/** All elements are in the same array? */
static int PRIVATE_T_L_(in, array)(struct T_(Link) *const this,
	const struct T_(LinkNode) *const array, const size_t array_size) {
	struct T_(LinkNode) *item;
	for(item = this->L_(first); item; item = item->L_(next))
		if(item < array || item >= array + array_size) return 0;
	return 1;
}

/** Returns true. Used in \see{PRIVATE_T_L_(test, basic)}.
 @implements <T>Predicate */
static int PRIVATE_T_L_(true, index)(T *const this, void *const param) {
	UNUSED(this); UNUSED(param);
	return 1;
}
/** Used in \see{PRIVATE_T_L_(test, basic)}.
 @param param: (int *), boolean.
 @implements <T>Predicate */
static int PRIVATE_T_L_(every, second)(T *const this, void *const param) {
	int *const pbinary = param;
	UNUSED(this);
	return !(*pbinary = !*pbinary);
}



/* now tests */

static void PRIVATE_T_L_(test, basic)(void) {
	char str[12];
	T *data;
	struct T_(Link) a;
	struct T_(LinkNode) buf[1000], *const new_buf = buf + 2,
		*item_a, *item_b, *item_y, *item_z;
	const size_t buf_size = sizeof buf / sizeof *buf, new_buf_size = buf_size-4;
	size_t i;
	int is_parity = 1;

	printf("Basic tests of " QUOTE(LINK_NAME) " linked-list " QUOTE(LINK_L_NAME)
		":\n");
	/* Clear */
	T_(LinkClear)(0);
	T_(LinkClear)(&a);
	printf("Adding %lu elements to a.\n", (unsigned long)buf_size);
	/* Add */
	T_(LinkAdd)(0, 0);
	T_(LinkAdd)(&a, 0);
	item_a = buf;
	PRIVATE_T_(filler)(&item_a->data);
	T_(LinkAdd)(0, item_a);
	for(i = 0; i < buf_size; i++) {
		item_a = buf + i;
		PRIVATE_T_(filler)(&item_a->data);
		T_(LinkAdd)(&a, item_a);
	}
	item_a = T_L_(Link, GetFirst)(&a);
	assert(item_a);
	data = (T *)item_a;
	assert(data);
	PRIVATE_T_(to_string)(data, &str);
	printf("LinkNode get first data: %s.\n", str);
	assert(memcmp(&buf[0].data, data, sizeof *data) == 0);
	/* ShortCircuit */
	printf("ShortCircuit: for all true returns null.\n");
	assert(!T_L_(Link, ShortCircuit)(0, 0));
	assert(!T_L_(Link, ShortCircuit)(0, &PRIVATE_T_L_(true, index)));
	assert(!T_L_(Link, ShortCircuit)(&a, 0));
	assert(!T_L_(Link, ShortCircuit)(&a, &PRIVATE_T_L_(true, index)));
	printf("ShortCircuit: parity [ 1, 0, 1, ... ] ends on index 1.\n");
	/* SetParam */
	assert(!a.param);
	T_(LinkSetParam)(0, 0);
	assert(!a.param);
	T_(LinkSetParam)(0, &is_parity);
	assert(!a.param);
	T_(LinkSetParam)(&a, 0);
	assert(!a.param);
	T_(LinkSetParam)(&a, &is_parity);
	assert(a.param == &is_parity);
	/* SetParam with ShortCircuit */
	assert(!T_L_(Link, ShortCircuit)(0, 0));
	assert(!T_L_(Link, ShortCircuit)(&a, 0));
	assert(!T_L_(Link, ShortCircuit)(0, &PRIVATE_T_L_(every, second)));
	assert(T_L_(Link, ShortCircuit)(&a, &PRIVATE_T_L_(every, second)) == buf + 1);
	/* GetNext, GetPrevious, GetFirst, GetLast */
	printf("Removing 3 elements from a.\n");
	assert(PRIVATE_T_L_(exactly, elements)(&a, buf, buf_size));
	assert(!T_L_(Link, GetFirst)(0));
	assert(!T_L_(Link, GetLast)(0));
	assert(!T_L_(LinkNode, GetPrevious)(0));
	assert(!T_L_(LinkNode, GetNext)(0));
	item_a = T_L_(Link, GetFirst)(&a);
	assert(!T_L_(LinkNode, GetPrevious)(item_a));
	item_b = T_L_(LinkNode, GetNext)(item_a);
	item_z = T_L_(Link, GetLast)(&a);
	assert(!T_L_(LinkNode, GetNext)(item_z));
	item_y = T_L_(LinkNode, GetPrevious)(item_z);
	assert(item_a && item_b && item_y && item_z);
	/* Remove */
	T_(LinkRemove)(0, item_y);
	T_(LinkRemove)(&a, item_y);
	T_(LinkRemove)(&a, item_z);
	T_(LinkRemove)(&a, item_b);
	T_(LinkRemove)(&a, item_a);
	assert(PRIVATE_T_L_(exactly, elements)(&a, new_buf, new_buf_size)),
		UNUSED(new_buf), UNUSED(new_buf_size);
	/* ForEach */
	printf("Counting %lu elements.\n", (unsigned long)new_buf_size);
	PRIVATE_T_L_(count, var) = 0;
	T_L_(Link, ForEach)(0, 0);
	T_L_(Link, ForEach)(0, &PRIVATE_T_L_(count, another));
	T_L_(Link, ForEach)(&a, 0);
	T_L_(Link, ForEach)(&a, &PRIVATE_T_L_(count, another));
	assert(PRIVATE_T_L_(count, var) == new_buf_size);
	assert(PRIVATE_T_L_(count, elements)(&a) == new_buf_size);
#ifdef LINK_SOME_COMPARATOR /* <-- comp */
	/* <L>Sort */
	printf("Sorting a only by " QUOTE(LINK_L_NAME) ".\n");
	T_L_(Link, Sort)(0);
	T_L_(Link, Sort)(&a);
	assert(PRIVATE_T_L_(in, order)(&a));
	/* Sort */
	T_(LinkSort)(0);
	T_(LinkSort)(&a);
	printf("Sorting, a = %s.\n", T_L_(Link, ToString)(&a));
	assert(PRIVATE_T_(in_order)(&a));
#endif /* comp --> */
	/* ToString (unchecked) */
	printf("ToString: null = %s; a = %s.\n",
		T_L_(Link, ToString)(0), T_L_(Link, ToString)(&a));
	printf("Clear.\n");
	T_(LinkClear)(&a);
	assert(!PRIVATE_T_L_(count, elements)(&a));
	printf("\n");
}

static void PRIVATE_T_L_(test, memory)(void) {
	struct T_(Link) a, b;
	struct T_(LinkNode) buf[3][100], *node_a, *node_b, *node_c;
	const size_t buf_size = sizeof buf[0] / sizeof *buf[0];
	size_t i;
	int is_parity;

	printf("Memory moving tests of " QUOTE(LINK_NAME) " linked-list "
		QUOTE(LINK_L_NAME) ":\n");
	T_(LinkClear)(&a), T_(LinkClear)(&b);
	/* fill the items in buf0 */
	for(i = 0; i < buf_size; i++) node_a = buf[0]+i, PRIVATE_T_(filler)(&node_a->data);
	/* copy the items to buf1; buf2 zeroed */
	memcpy(buf[1], buf[0], buf_size * sizeof *buf[0]);
	memset(&buf[2], 0, buf_size * sizeof *buf[2]);
	/* put all items in buf0 and buf1 */
	for(i = 0; i < buf_size; i++) T_(LinkAdd)(&a, buf[0] + i);
	for(i = 0; i < buf_size; i++) T_(LinkAdd)(&a, buf[1] + i);
#ifdef LINK_SOME_COMPARATOR /* <-- comp */
	T_(LinkSort)(&a);
	printf("Sorting, (backed by two arrays,) a = %s.\n",
		T_L_(Link, ToString)(&a));
	assert(PRIVATE_T_(in_order)(&a));
#endif /* comp --> */
	/* now add all of the odd to list_b, remove all the even from list_a */
	printf("Spliting odd/even a to b = %s by " QUOTE(LINK_L_NAME) ";\n",
		T_L_(Link, ToString)(&b));
	T_(LinkSetParam)(&a, &is_parity);
	is_parity = 0;
	T_L_(Link, TakeIf)(&b, &a, &PRIVATE_T_L_(every, second));
	printf("a = %s, b = %s.\n",
		T_L_(Link, ToString)(&a), T_L_(Link, ToString)(&b));
#ifdef LINK_SOME_COMPARATOR /* <-- comp */
	/* tests stability of sort; false! only if all items unique */
	/*assert(PRIVATE_T_L_(in, array)(&a, buf[0], buf_size));*/
	/*assert(PRIVATE_T_(in_array)(&b, buf[1], buf_size));*/
	assert(PRIVATE_T_(in_order)(&a));
	assert(PRIVATE_T_L_(in, order)(&b)); /* only <L> is in order */
#endif /* comp --> */
#ifdef LINK_L_COMPARATOR /* <-- comp */
	/* Compare */
	assert(!T_L_(Link, Compare)(0, 0));
	assert(T_L_(Link, Compare)(&a, 0) > 0);
	assert(T_L_(Link, Compare)(0, &b) < 0);
	assert(!T_L_(Link, Compare)(&a, &b));
#endif /* comp --> */
	/* Test done. */
	T_L_(Link, TakeIf)(&b, &a, 0);
	printf("Moving all a to b; a = %s, b = %s.\n",
		T_L_(Link, ToString)(&a), T_L_(Link, ToString)(&b));
	assert(PRIVATE_T_L_(count, elements)(&a) == 0);
	assert(PRIVATE_T_L_(count, elements)(&b) == 2 * buf_size);
	/* Clear (the rest) */
	printf("Clear b.\n");
	T_(LinkClear)(&b);
	assert(PRIVATE_T_L_(count, elements)(&b) == 0);
	/* Test done; move. */
	for(i = 0; i < buf_size; i++)
		T_(LinkAdd)(&a, buf[0] + i), T_(LinkAdd)(&b, buf[1] + i);
	assert(PRIVATE_T_L_(exactly, elements)(&a, buf[0], buf_size));
	assert(PRIVATE_T_L_(exactly, elements)(&b, buf[1], buf_size));
#ifdef LINK_L_COMPARATOR /* <-- comp */
	assert(!T_L_(Link, Compare)(&a, &b));
#endif /* comp --> */
	for(i = 0; i < buf_size; i += 2) {
		node_b = buf[1] + i;
		node_c = buf[2] + i;
		memcpy(node_c, node_b, sizeof *node_b);
		memset(node_b, 0, sizeof *node_b);
		T_(LinkMigrate)(&b, node_c);
	}
	printf("Testing memory relocation with <" QUOTE(LINK_NAME)
		">LinkMigrate: a = %s, b = %s.\n",
		T_L_(Link, ToString)(&a), T_L_(Link, ToString)(&b));
#ifdef LINK_L_COMPARATOR /* <-- comp */
	assert(!T_L_(Link, Compare)(&a, &b));
#endif /* comp --> */
	assert(PRIVATE_T_(in_array)(&a, buf[0], buf_size));
	assert(PRIVATE_T_(in_array)(&b, buf[1], buf_size << 1));
	printf("\n");
}

#ifdef LINK_L_COMPARATOR /* <-- compare */

static void PRIVATE_T_L_(test, boolean)(void) {
	struct T_(Link) a, b, c, ia, ib, ic;
	struct Test { struct T_(LinkNode) a, b, ia, ib; char str[12]; } x[3];
	unsigned i; /* for not-getting into an infty loop */
	const unsigned limit = 1000;

	printf("Boolean sequence operations on " QUOTE(LINK_NAME) " linked-list "
		QUOTE(LINK_L_NAME) ":\n");
	/* distinct elements x, y, z */
	PRIVATE_T_(filler)(&x[0].a.data), PRIVATE_T_(to_string)(&x[0].a.data, &x[0].str);
	memcpy(&x[0].b.data,  &x[0].a.data, sizeof x[0].a.data);
	memcpy(&x[0].ia.data, &x[0].a.data, sizeof x[0].a.data);
	memcpy(&x[0].ib.data, &x[0].a.data, sizeof x[0].a.data);
	i = 0; do { PRIVATE_T_(filler)(&x[1].a.data); i++; }
	while(i < limit && !PRIVATE_T_L_(data, cmp)(&x[0].a.data, &x[1].a.data));
	assert(i < limit); /* <- need to get more variety in {LINK_TEST} filler */
	PRIVATE_T_(to_string)(&x[1].a.data, &x[1].str);
	memcpy(&x[1].b.data,  &x[1].a.data, sizeof x[1].a.data);
	memcpy(&x[1].ia.data, &x[1].a.data, sizeof x[1].a.data);
	memcpy(&x[1].ib.data, &x[1].a.data, sizeof x[1].a.data);
	i = 0; do { PRIVATE_T_(filler)(&x[2].a.data); i++; }
	while(i < limit
		&& (!PRIVATE_T_L_(data, cmp)(&x[0].a.data, &x[2].a.data) || !PRIVATE_T_L_(data, cmp)(&x[1].a.data, &x[2].a.data)));
	assert(i < limit); /* <- need to get more variety in {LINK_TEST} filler */
	PRIVATE_T_(to_string)(&x[2].a.data, &x[2].str);
	memcpy(&x[2].b.data,  &x[2].a.data, sizeof x[2].a.data);
	memcpy(&x[2].ia.data, &x[2].a.data, sizeof x[2].a.data);
	memcpy(&x[2].ib.data, &x[2].a.data, sizeof x[2].a.data);
	printf("Three distinct " QUOTE(LINK_L_NAME) "-elements: "
		"%s, %s, %s.\n", x[0].str, x[1].str, x[2].str);
	/* add to the sequences a, b */
	T_(LinkClear)(&a), T_(LinkClear)(&b), T_(LinkClear)(&c);
	T_(LinkClear)(&ia), T_(LinkClear)(&ib), T_(LinkClear)(&ic);
	T_(LinkAdd)(&a, &x[0].a), T_(LinkAdd)(&a, &x[1].a);
	T_(LinkAdd)(&b, &x[0].b), T_(LinkAdd)(&b, &x[2].b);
	printf("Two " QUOTE(LINK_L_NAME) "-sequences, %s and %s.\n",
		   T_L_(Link, ToString)(&a), T_L_(Link, ToString)(&b));
	/* verify comparing things first */
	printf("Comparing-" QUOTE(LINK_L_NAME) " a = %s and c = %s.\n",
		T_L_(Link, ToString)(&a), T_L_(Link, ToString)(&c));
	assert(T_L_(Link, Compare)(&a, &c) == 1);
	assert(T_L_(Link, Compare)(&c, &a) == -1);
	assert(T_L_(Link, Compare)(&a, &a) == 0);
	/* - */
	printf("(a = %s) - (b = %s) =\n", T_L_(Link, ToString)(&a),
		T_L_(Link, ToString)(&b));
	T_L_(Link, TakeSubtraction)(&c, &a, &b);
	printf("%s; (a = %s, b = %s.)\n", T_L_(Link, ToString)(&c),
		T_L_(Link, ToString)(&a), T_L_(Link, ToString)(&b));
	T_(LinkAdd)(&ia, &x[0].ia);
	T_(LinkAdd)(&ib, &x[0].ib), T_(LinkAdd)(&ib, &x[2].ib);
	T_(LinkAdd)(&ic, &x[1].ia);
	assert(!T_L_(Link, Compare)(&a, &ia));
	assert(!T_L_(Link, Compare)(&b, &ib));
	assert(!T_L_(Link, Compare)(&c, &ic));
	T_(LinkClear)(&a), T_(LinkClear)(&b), T_(LinkClear)(&c);
	T_(LinkClear)(&ia), T_(LinkClear)(&ib), T_(LinkClear)(&ic);
	/* u */
	T_(LinkAdd)(&a, &x[0].a), T_(LinkAdd)(&a, &x[1].a);
	T_(LinkAdd)(&b, &x[0].b), T_(LinkAdd)(&b, &x[2].b);
	printf("(a = %s) u (b = %s) =\n", T_L_(Link, ToString)(&a),
		T_L_(Link, ToString)(&b));
	T_L_(Link, TakeUnion)(&c, &a, &b);
	printf("%s; (a = %s, b = %s.)\n", T_L_(Link, ToString)(&c),
		T_L_(Link, ToString)(&a), T_L_(Link, ToString)(&b));
	T_(LinkAdd)(&ib, &x[0].ib);
	T_(LinkAdd)(&ic, &x[0].ia);
	if(PRIVATE_T_L_(data, cmp)(&x[1].ia.data, &x[2].ib.data) < 0) {
		T_(LinkAdd)(&ic, &x[1].ia), T_(LinkAdd)(&ic, &x[2].ib);
	} else {
		T_(LinkAdd)(&ic, &x[2].ib), T_(LinkAdd)(&ic, &x[1].ia);
	}
	assert(!T_L_(Link, Compare)(&a, &ia));
	assert(!T_L_(Link, Compare)(&b, &ib));
	assert(!T_L_(Link, Compare)(&c, &ic));
	T_(LinkClear)(&a), T_(LinkClear)(&b), T_(LinkClear)(&c);
	T_(LinkClear)(&ia), T_(LinkClear)(&ib), T_(LinkClear)(&ic);
	/* n */
	T_(LinkAdd)(&a, &x[0].a), T_(LinkAdd)(&a, &x[1].a);
	T_(LinkAdd)(&b, &x[0].b), T_(LinkAdd)(&b, &x[2].b);
	printf("(a = %s) n (b = %s) =\n", T_L_(Link, ToString)(&a),
		T_L_(Link, ToString)(&b));
	T_L_(Link, TakeIntersection)(&c, &a, &b);
	printf("%s; (a = %s, b = %s.)\n", T_L_(Link, ToString)(&c),
		T_L_(Link, ToString)(&a), T_L_(Link, ToString)(&b));
	T_(LinkAdd)(&ia, &x[1].ia);
	T_(LinkAdd)(&ib, &x[0].ib), T_(LinkAdd)(&ib, &x[2].ib);
	T_(LinkAdd)(&ic, &x[0].ia);
	assert(!T_L_(Link, Compare)(&a, &ia));
	assert(!T_L_(Link, Compare)(&b, &ib));
	assert(!T_L_(Link, Compare)(&c, &ic));
	T_(LinkClear)(&a), T_(LinkClear)(&b), T_(LinkClear)(&c);
	T_(LinkClear)(&ia), T_(LinkClear)(&ib), T_(LinkClear)(&ic);
	/* xor */
	T_(LinkAdd)(&a, &x[0].a), T_(LinkAdd)(&a, &x[1].a);
	T_(LinkAdd)(&b, &x[0].b), T_(LinkAdd)(&b, &x[2].b);
	printf("(a = %s) xor (b = %s) =\n", T_L_(Link, ToString)(&a),
		T_L_(Link, ToString)(&b));
	T_L_(Link, TakeXor)(&c, &a, &b);
	printf("%s; (a = %s, b = %s.)\n", T_L_(Link, ToString)(&c),
		T_L_(Link, ToString)(&a), T_L_(Link, ToString)(&b));
	T_(LinkAdd)(&ia, &x[0].ia);
	T_(LinkAdd)(&ib, &x[0].ib);
	if(PRIVATE_T_L_(data, cmp)(&x[1].ia.data, &x[2].ib.data) < 0) {
		T_(LinkAdd)(&ic, &x[1].ia), T_(LinkAdd)(&ic, &x[2].ib);
	} else {
		T_(LinkAdd)(&ic, &x[2].ib), T_(LinkAdd)(&ic, &x[1].ia);
	}
	assert(!T_L_(Link, Compare)(&a, &ia));
	assert(!T_L_(Link, Compare)(&b, &ib));
	assert(!T_L_(Link, Compare)(&c, &ic));
	printf("\n");
}

static void PRIVATE_T_L_(test, order)(void) {
	struct T_(LinkNode) buf[3000], *node = buf;
	const size_t buf_size = sizeof buf / sizeof *buf;
	struct T_(Link) a, b;
	size_t i;
	T_(LinkClear)(&a), T_(LinkClear)(&b);
	assert(T_L_(Link, Compare)(0, 0) == 0);
	assert(T_L_(Link, Compare)(&a, 0) > 0);
	assert(T_L_(Link, Compare)(0, &b) < 0);
	assert(T_L_(Link, Compare)(&a, &b) == 0);
	for(i = 0; i < buf_size; i++) node = buf + i, PRIVATE_T_(filler)(&node->data);
	for(i = 0; i < buf_size >> 1; i++) T_(LinkAdd)(&a, node--);
	assert(T_L_(Link, Compare)(&a, &b) > 0);
	for( ; i < buf_size; i++) T_(LinkAdd)(&b, node--);
	assert(PRIVATE_T_L_(count, elements)(&a) == buf_size >> 1);
	assert(PRIVATE_T_L_(count, elements)(&b) == buf_size - (buf_size >> 1));
	T_(LinkSort)(&a), T_(LinkSort)(&b);
	T_(LinkTake)(&a, &b);
	printf("Testing " QUOTE(LINK_NAME) "-" QUOTE(LINK_L_NAME) " a, b for order "
		"on <" QUOTE(LINK_NAME) ">LinkTake(a, b).\n");
	assert(PRIVATE_T_L_(count, elements)(&a) == buf_size);
	assert(PRIVATE_T_L_(count, elements)(&b) == 0);
	/* technically, \${(1/2)^(buf_size/2+1)} change of getting this one wrong */
	assert(PRIVATE_T_(exactly_unordered)(&a, (size_t)1));
	/* done now merge */
	T_(LinkClear)(&a), T_(LinkClear)(&b);
	node = buf;
	for(i = 0; i < buf_size >> 1; i++) T_(LinkAdd)(&a, node++);
	for( ; i < buf_size; i++) T_(LinkAdd)(&b, node++);
	T_(LinkSort)(&a), T_(LinkSort)(&b);
	assert(PRIVATE_T_L_(count, elements)(&a) == buf_size >> 1);
	assert(PRIVATE_T_L_(count, elements)(&b) == buf_size - (buf_size >> 1));
	assert(PRIVATE_T_(in_order)(&a));
	assert(PRIVATE_T_(in_order)(&b));
	printf("Testing " QUOTE(LINK_NAME) "-" QUOTE(LINK_L_NAME) " a, b for order "
		"on <" QUOTE(LINK_NAME) ">LinkMerge(a, b): %s, %s.\n",
		T_L_(Link, ToString)(&a), T_L_(Link, ToString)(&b));
#ifdef LINK_A_COMPARATOR
	printf("By " QUOTE(LINK_A_NAME) ": a = %s, b = %s.\n",
		T_LA_(Link, ToString)(&a), T_LA_(Link, ToString)(&b));
#endif
#ifdef LINK_B_COMPARATOR
	printf("By " QUOTE(LINK_B_NAME) ": a = %s, b = %s.\n",
		T_LB_(Link, ToString)(&a), T_LB_(Link, ToString)(&b));
#endif
#ifdef LINK_C_COMPARATOR
	printf("By " QUOTE(LINK_C_NAME) ": a = %s, b = %s.\n",
		T_LC_(Link, ToString)(&a), T_LC_(Link, ToString)(&b));
#endif
#ifdef LINK_D_COMPARATOR
	printf("By " QUOTE(LINK_D_NAME) ": a = %s, b = %s.\n",
		T_LD_(Link, ToString)(&a), T_LD_(Link, ToString)(&b));
#endif
	T_(LinkMerge)(&a, &b);
	printf("Testing " QUOTE(LINK_NAME) "-" QUOTE(LINK_L_NAME) " a, b for order "
		"on <" QUOTE(LINK_NAME) ">LinkMerge(a, b).\n");
	assert(PRIVATE_T_L_(count, elements)(&a) == buf_size);
	assert(PRIVATE_T_L_(count, elements)(&b) == 0);
	assert(PRIVATE_T_(in_order)(&a));
	printf("\n");
}

static void PRIVATE_T_L_(test, meta)(void) {
	struct T_(LinkNode) nodes[256], *node = nodes;
	const size_t nodes_size = sizeof nodes / sizeof *nodes;
	struct T_(Link) links[32];
	const size_t links_size = sizeof links / sizeof *links;
	size_t i, nodes_left = nodes_size, links_left = links_size;

	printf("An array of lists of " T_NAME ".\n");
	for(i = 0; i < nodes_size; i++) PRIVATE_T_(filler)(&nodes[i].data);
	while(links_left) {
		struct T_(Link) *const link = links + links_size - links_left;
		int take = (int)((double)nodes_left / links_left
			+ 0.4 * (links_left - 1) * (2.0 * rand() / (1.0 + RAND_MAX) - 1.0));
		if(take < 0) take = 0;
		else if((size_t)take > nodes_left) take = (int)nodes_left;
		nodes_left -= (size_t)take;
		T_(LinkClear)(link);
		while(take) T_(LinkAdd)(link, node++), take--;
		links_left--;
		printf("%lu. %s\n", (unsigned long)(links_size - links_left),
			T_L_(Link, ToString)(link));
	}
	printf("Sorting with qsort by " QUOTE(LINK_L_NAME) ".\n");
	qsort(links, links_size, sizeof *links,
		(int (*)(const void *, const void *))&T_L_(Link, Compare));
	for(i = 0; i < links_size; i++) {
		printf("%lu. %s\n", (unsigned long)(i + 1),
			T_L_(Link, ToString)(links + i));
		if(i) { /* like {strcmp} comparing the first letter -- good enough */
			const struct T_(LinkNode) *const less = links[i - 1].L_(first),
				*const more = links[i].L_(first);
			if(!less) continue;
			assert(more), UNUSED(more);
			assert(PRIVATE_T_L_(data, cmp)(&less->data, &more->data) <= 0);
		}
	}
	printf("\n");
}

#endif /* compare --> */

/* all list tests */
static void PRIVATE_T_L_(test, list)(void) {
	printf("Link<" QUOTE(LINK_NAME) "> linked-list "
		   QUOTE(LINK_L_NAME) ":\n");
	PRIVATE_T_L_(test, basic)();
	PRIVATE_T_L_(test, memory)();
#ifdef LINK_L_COMPARATOR /* <-- compare */
	PRIVATE_T_L_(test, boolean)();
	PRIVATE_T_L_(test, order)();
	PRIVATE_T_L_(test, meta)();
#endif /* compare --> */
}



/* undefine stuff for the next */
#undef LINK_L_NAME
#ifdef LINK_L_COMPARATOR /* <-- comp */
#undef LINK_L_COMPARATOR
#endif /* comp --> */

#endif /* LINK_L_NAME --> */
