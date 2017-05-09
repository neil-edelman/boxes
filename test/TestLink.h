/* intended to be included by ../src/Link.h on LINK_TEST */

#ifndef _LINK_NAME /* <-- !_LINK_NAME */

#include <stdlib.h>	/* EXIT_SUCCESS rand */



/* prototype */
static int _T_(in_order)(struct T_(Link) *const this);
static int _T_(in_array)(struct T_(Link) *const this,
	const struct T_(LinkNode) *const array, const size_t array_size);
static int _T_(exactly_unordered)(struct T_(Link) *const this, const size_t n);

/* Check that LINK_TEST is a function implementing {<T>Action}. */
static const T_(Action) _T_(filler) = (LINK_TEST);

/* For \see{_T_L_(exactly, elements)}. */
struct _T_(Verify) {
	size_t i;
	const struct T_(LinkNode) *array;
	size_t array_no;
};
/* For \see{_T_L_(count, unordered)}. */
struct _T_(Order) {
	T *prev;
	size_t count;
};

#ifdef LINK_A_NAME /* <-- a */
#define _LINK_NAME LINK_A_NAME
#ifdef LINK_A_COMPARATOR /* <-- comp */
#define _LINK_COMPARATOR LINK_A_COMPARATOR
#endif /* comp --> */
#include "TestLink.h"
#endif /* a --> */

#ifdef LINK_B_NAME /* <-- b */
#define _LINK_NAME LINK_B_NAME
#ifdef LINK_B_COMPARATOR /* <-- comp */
#define _LINK_COMPARATOR LINK_B_COMPARATOR
#endif /* comp --> */
#include "TestLink.h"
#endif /* b --> */

#ifdef LINK_C_NAME /* <-- c */
#define _LINK_NAME LINK_C_NAME
#ifdef LINK_C_COMPARATOR /* <-- comp */
#define _LINK_COMPARATOR LINK_C_COMPARATOR
#endif /* comp --> */
#include "TestLink.h"
#endif /* c --> */

#ifdef LINK_D_NAME /* <-- d */
#define _LINK_NAME LINK_D_NAME
#ifdef LINK_D_COMPARATOR /* <-- comp */
#define _LINK_COMPARATOR LINK_D_COMPARATOR
#endif /* comp --> */
#include "TestLink.h"
#endif /* d --> */

/** The linked-list will be tested on stdout.
 @return Success.
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
	_T_LA_(test, list)();
#endif
#ifdef LINK_B_NAME
	_T_LB_(test, list)();
#endif
#ifdef LINK_C_NAME
	_T_LC_(test, list)();
#endif
#ifdef LINK_D_NAME
	_T_LD_(test, list)();
#endif
	fprintf(stderr, "Done tests of Link<" T_NAME ">.\n\n");
}

/* test helper functions */

static int _T_(in_order)(struct T_(Link) *const this) {
	assert(this);
	return 1
#ifdef LINK_A_COMPARATOR
		&& _T_LA_(in, order)(this)
#endif
#ifdef LINK_B_COMPARATOR
		&& _T_LB_(in, order)(this)
#endif
#ifdef LINK_C_COMPARATOR
		&& _T_LC_(in, order)(this)
#endif
#ifdef LINK_D_COMPARATOR
		&& _T_LD_(in, order)(this)
#endif
		;
}

static int _T_(in_array)(struct T_(Link) *const this,
	const struct T_(LinkNode) *const array, const size_t array_size) {
	/* overkill; only one would do */
	return 1
#ifdef LINK_A_COMPARATOR
		&& _T_LA_(in, array)(this, array, array_size)
#endif
#ifdef LINK_B_COMPARATOR
		&& _T_LB_(in, array)(this, array, array_size)
#endif
#ifdef LINK_C_COMPARATOR
		&& _T_LC_(in, array)(this, array, array_size)
#endif
#ifdef LINK_D_COMPARATOR
		&& _T_LD_(in, array)(this, array, array_size)
#endif
		;
}

static int _T_(exactly_unordered)(struct T_(Link) *const this, const size_t n) {
	return 1
#ifdef LINK_A_COMPARATOR
		&& _T_LA_(count, unordered)(this) == n
#endif
#ifdef LINK_B_COMPARATOR
		&& _T_LB_(count, unordered)(this) == n
#endif
#ifdef LINK_C_COMPARATOR
		&& _T_LC_(count, unordered)(this) == n
#endif
#ifdef LINK_D_COMPARATOR
		&& _T_LD_(count, unordered)(this) == n
#endif
		;
}



#else /* !_LINK_NAME --><-- _LINK_NAME */



#ifdef _T_L_
#undef _T_L_
#endif
#define _T_L_(thing1, thing2) CAT(PCAT(LINK_NAME, thing1), \
	PCAT(_LINK_NAME, thing2))



/* test helper functions that depend on <L> */

/** \see{_T_L_(count, elements)}
 @param param: (size_t *)
 @implements <T>Predicate */
static int _T_L_(count, predicate)(T *const this, void *const param) {
	size_t count = *(size_t *)param;
	UNUSED(this);
	*(size_t *)param = ++count;
	return 1;
}
/** Counts the elements. */
static size_t _T_L_(count, elements)(struct T_(Link) *const this) {
	size_t count = 0;
	T_(LinkSetParam)(this, &count);
	T_L_(Link, ShortCircuit)(this, &_T_L_(count, predicate));
	return count;
}

/* Global count \see{<T>_count_<L>_another}. */
size_t _T_L_(count, var);
/** @implements <T>Action */
static void _T_L_(count, another)(T *const this) {
	UNUSED(this);
	_T_L_(count, var)++;
}

/** \see{_T_L_(exactly, elements)}
 @param param: struct <T>Link<L>Verify
 @implements <T>Predicate */
static int _T_L_(exactly, predicate)(T *const this, void *const param) {
	struct _T_(Verify) *lv = param;
	if(lv->array_no <= lv->i
		|| memcmp(this, &lv->array[lv->i].data, sizeof *this))
		return fprintf(stderr, "Failed at index %lu.\n", lv->i), 0;
	lv->i++;
	return 1;
}
/** Verifies that the elements are exactly as in {array}. */
static size_t _T_L_(exactly, elements)(struct T_(Link) *const this,
	const struct T_(LinkNode) *array, const size_t array_no) {
	struct _T_(Verify) lv = { 0, 0, 0 };
	lv.array    = array;
	lv.array_no = array_no;
	T_(LinkSetParam)(this, &lv);
	return !T_L_(Link, ShortCircuit)(this, &_T_L_(exactly, predicate));
}

/** \see{_T_L_(in, order)}.
 @param param: (T *[1]), last element.
 @implements <T>Predicate */
static int _T_L_(order, predicate)(T *const this, void *const param) {
	T **prev_one_array = param;
	T *const prev = prev_one_array[0];
	/*char scratch[9];
	_T_(to_string)(data, &scratch), scratch[8] = '\0';
	printf("%s%s", prev ? " <= " : "", scratch);*/
	if(prev && _T_L_(data, cmp)(prev, this) > 0) return 0;
	prev_one_array[0] = this;
	return 1;
}
/** Verifies sorting on index. */
static int _T_L_(in, order)(struct T_(Link) *const this) {
	T *one_array[] = { 0 };
	T_(LinkSetParam)(this, one_array);
	return !T_L_(Link, ShortCircuit)(this, &_T_L_(order, predicate));
}

/** \see{_T_L_(count, unordered)}.
 @param param: (struct _T_(Order) *).
 @implements <T>Predicate */
static int _T_L_(unorder, predicate)(T *const this, void *const param) {
	struct _T_(Order) *info = param;
	char a[9], b[9];
	if(info->prev && _T_L_(data, cmp)(info->prev, this) > 0)
		printf("Unorder %lu: %s > %s\n", ++info->count,
		(_T_(to_string)(info->prev, &a), a), (_T_(to_string)(this, &b), b));
	info->prev = this;
	return 1;
}
/** How many of them are not in order?
 @implements <T>Predicate */
static size_t _T_L_(count, unordered)(struct T_(Link) *this) {
	struct _T_(Order) info = { 0, 0 };
	printf("Unordered-" QUOTE(LINK_NAME) "-" QUOTE(_LINK_NAME) ": %s.\n",
		T_L_(Link, ToString)(this));
	T_(LinkSetParam)(this, &info);
	T_L_(Link, ShortCircuit)(this, &_T_L_(unorder, predicate));
	return info.count;
}

/** All elements are in the same array? */
static int _T_L_(in, array)(struct T_(Link) *const this,
	const struct T_(LinkNode) *const array, const size_t array_size) {
	struct T_(LinkNode) *item;
	for(item = this->L_(first); item; item = item->L_(next))
		if(item < array || item >= array + array_size) return 0;
	return 1;
}

/** Returns true. Used in \see{_T_L_(test, basic)}.
 @implements <T>Predicate */
static int _T_L_(true, index)(T *const this, void *const param) {
	UNUSED(this); UNUSED(param);
	return 1;
}
/** Used in \see{_T_L_(test, basic)}.
 @param param: (int *), boolean.
 @implements <T>Predicate */
static int _T_L_(every, second)(T *const this, void *const param) {
	int *const pbinary = param;
	UNUSED(this);
	return !(*pbinary = !*pbinary);
}



/* now tests */

static void _T_L_(test, basic)(void) {
	char str[9];
	T *data;
	struct T_(Link) a;
	struct T_(LinkNode) buf[1000], *const new_buf = buf + 2,
		*item_a, *item_b, *item_y, *item_z;
	const size_t buf_size = sizeof buf / sizeof *buf, new_buf_size = buf_size-4;
	size_t i;
	int is_parity = 1;

	printf("Basic tests of " QUOTE(LINK_NAME) " linked-list " QUOTE(_LINK_NAME)
		":\n");
	/* Clear */
	T_(LinkClear)(0);
	T_(LinkClear)(&a);
	printf("Adding %lu elements to a.\n", buf_size);
	/* Add */
	T_(LinkAdd)(0, 0);
	T_(LinkAdd)(&a, 0);
	item_a = buf;
	_T_(filler)(&item_a->data);
	T_(LinkAdd)(0, item_a);
	for(i = 0; i < buf_size; i++) {
		item_a = buf + i;
		_T_(filler)(&item_a->data);
		T_(LinkAdd)(&a, item_a);
	}
	item_a = T_L_(Link, GetFirst)(&a);
	assert(item_a);
	data = (T *)item_a;
	assert(data);
	_T_(to_string)(data, &str);
	printf("LinkNode get first data: %s.\n", str);
	assert(memcmp(&buf[0].data, data, sizeof *data) == 0);
	/* ShortCircuit */
	printf("ShortCircuit: for all true returns null.\n");
	assert(!T_L_(Link, ShortCircuit)(0, 0));
	assert(!T_L_(Link, ShortCircuit)(0, &_T_L_(true, index)));
	assert(!T_L_(Link, ShortCircuit)(&a, 0));
	assert(!T_L_(Link, ShortCircuit)(&a, &_T_L_(true, index)));
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
	assert(!T_L_(Link, ShortCircuit)(0, &_T_L_(every, second)));
	assert(T_L_(Link, ShortCircuit)(&a, &_T_L_(every, second)) == buf + 1);
	/* GetNext, GetPrevious, GetFirst, GetLast */
	printf("Removing 3 elements from a.\n");
	assert(_T_L_(exactly, elements)(&a, buf, buf_size));
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
	assert(_T_L_(exactly, elements)(&a, new_buf, new_buf_size));
	/* ForEach */
	printf("Counting %lu elements.\n", new_buf_size);
	_T_L_(count, var) = 0;
	T_L_(Link, ForEach)(0, 0);
	T_L_(Link, ForEach)(0, &_T_L_(count, another));
	T_L_(Link, ForEach)(&a, 0);
	T_L_(Link, ForEach)(&a, &_T_L_(count, another));
	assert(_T_L_(count, var) == new_buf_size);
	assert(_T_L_(count, elements)(&a) == new_buf_size);
	/* <L>Sort */
	printf("Sorting a only by " QUOTE(_LINK_NAME) ".\n");
	T_L_(Link, Sort)(0);
	T_L_(Link, Sort)(&a);
	assert(_T_L_(in, order)(&a));
	/* Sort */
	T_(LinkSort)(0);
	T_(LinkSort)(&a);
	printf("Sorting, a = %s.\n", T_L_(Link, ToString)(&a));
	assert(_T_(in_order)(&a));
	/* ToString (unchecked) */
	printf("ToString: null = %s; a = %s.\n",
		T_L_(Link, ToString)(0), T_L_(Link, ToString)(&a));
	printf("Clear.\n");
	T_(LinkClear)(&a);
	assert(!_T_L_(count, elements)(&a));
}

static void _T_L_(test, memory)(void) {
	struct T_(Link) a, b;
	struct T_(LinkNode) buf[3][100], *node_a, *node_b, *node_c;
	const size_t buf_size = sizeof buf[0] / sizeof *buf[0];
	size_t i;
	int is_parity;

	printf("Memory moving tests of " QUOTE(LINK_NAME) " linked-list "
		QUOTE(_LINK_NAME) ":\n");
	T_(LinkClear)(&a), T_(LinkClear)(&b);
	/* fill the items in buf0 */
	for(i = 0; i < buf_size; i++) node_a = buf[0]+i, _T_(filler)(&node_a->data);
	/* copy the items to buf1; buf2 zeroed */
	memcpy(buf[1], buf[0], buf_size * sizeof *buf[0]);
	memset(&buf[2], 0, buf_size * sizeof *buf[2]);
	/* put all items in buf0 and buf1 */
	for(i = 0; i < buf_size; i++) T_(LinkAdd)(&a, buf[0] + i);
	for(i = 0; i < buf_size; i++) T_(LinkAdd)(&a, buf[1] + i);
	T_(LinkSort)(&a);
	printf("Sorting, (backed by two arrays,) a = %s.\n",
		T_L_(Link, ToString)(&a));
	assert(_T_(in_order)(&a));
	/* now add all of the odd to list_b, remove all the even from list_a */
	printf("Spliting odd/even a to b = %s by " QUOTE(_LINK_NAME) ";\n",
		T_L_(Link, ToString)(&b));
	T_(LinkSetParam)(&a, &is_parity);
	is_parity = 0;
	T_L_(Link, TakeIf)(&b, &a, &_T_L_(every, second));
	printf("a = %s, b = %s.\n",
		T_L_(Link, ToString)(&a), T_L_(Link, ToString)(&b));
	/* tests stability of sort; false! only if all items unique */
	/*assert(_T_L_(in, array)(&a, buf[0], buf_size));*/
	/*assert(_T_(in_array)(&b, buf[1], buf_size));*/
	assert(_T_(in_order)(&a));
	assert(_T_L_(in, order)(&b)); /* only <L> is in order */
	/* Compare */
	assert(!T_L_(Link, Compare)(0, 0));
	assert(T_L_(Link, Compare)(&a, 0) > 0);
	assert(T_L_(Link, Compare)(0, &b) < 0);
	assert(!T_L_(Link, Compare)(&a, &b));
	/* Test done. */
	T_L_(Link, TakeIf)(&b, &a, 0);
	printf("Moving all a to b; a = %s, b = %s.\n",
		T_L_(Link, ToString)(&a), T_L_(Link, ToString)(&b));
	assert(_T_L_(count, elements)(&a) == 0);
	assert(_T_L_(count, elements)(&b) == 2 * buf_size);
	/* Clear (the rest) */
	printf("Clear b.\n");
	T_(LinkClear)(&b);
	assert(_T_L_(count, elements)(&b) == 0);
	/* Test done; move. */
	for(i = 0; i < buf_size; i++)
		T_(LinkAdd)(&a, buf[0] + i), T_(LinkAdd)(&b, buf[1] + i);
	assert(_T_L_(exactly, elements)(&a, buf[0], buf_size));
	assert(_T_L_(exactly, elements)(&b, buf[1], buf_size));
	assert(!T_L_(Link, Compare)(&a, &b));
	for(i = 0; i < buf_size; i += 2) {
		node_b = buf[1] + i;
		node_c = buf[2] + i;
		memcpy(node_c, node_b, sizeof *node_b);
		memset(node_b, 0, sizeof *node_b);
		T_(LinkMigrate)(&b, node_c, node_b);
	}
	printf("Testing memory relocation with <" QUOTE(LINK_NAME)
		">LinkMigrate: a = %s, b = %s.\n",
		T_L_(Link, ToString)(&a), T_L_(Link, ToString)(&b));
	assert(!T_L_(Link, Compare)(&a, &b));
	assert(_T_(in_array)(&a, buf[0], buf_size));
	assert(_T_(in_array)(&b, buf[1], buf_size << 1));
}

#ifdef _LINK_COMPARATOR /* <-- compare */

static void _T_L_(test, boolean)(void) {
	struct T_(Link) a, b, c, ia, ib, ic;
	struct Test { struct T_(LinkNode) a, b, ia, ib; char str[9]; } x[3];
	unsigned i; /* for not-getting into an infty loop */
	const unsigned limit = 1000;

	printf("Boolean set operations on " QUOTE(LINK_NAME) " linked-list "
		QUOTE(_LINK_NAME) ":\n");
	/* distinct elements x, y, z */
	_T_(filler)(&x[0].a.data), _T_(to_string)(&x[0].a.data, &x[0].str);
	memcpy(&x[0].b.data,  &x[0].a.data, sizeof x[0].a.data);
	memcpy(&x[0].ia.data, &x[0].a.data, sizeof x[0].a.data);
	memcpy(&x[0].ib.data, &x[0].a.data, sizeof x[0].a.data);
	i = 0; do { _T_(filler)(&x[1].a.data); i++; }
	while(i < limit && !_T_L_(data, cmp)(&x[0].a.data, &x[1].a.data));
	assert(i < limit); /* <- need to get more variety in {LINK_TEST} filler */
	_T_(to_string)(&x[1].a.data, &x[1].str);
	memcpy(&x[1].b.data,  &x[1].a.data, sizeof x[1].a.data);
	memcpy(&x[1].ia.data, &x[1].a.data, sizeof x[1].a.data);
	memcpy(&x[1].ib.data, &x[1].a.data, sizeof x[1].a.data);
	i = 0; do { _T_(filler)(&x[2].a.data); i++; }
	while(i < limit
		&& (!_T_L_(data, cmp)(&x[0].a.data, &x[2].a.data) || !_T_L_(data, cmp)(&x[1].a.data, &x[2].a.data)));
	assert(i < limit); /* <- need to get more variety in {LINK_TEST} filler */
	_T_(to_string)(&x[2].a.data, &x[2].str);
	memcpy(&x[2].b.data,  &x[2].a.data, sizeof x[2].a.data);
	memcpy(&x[2].ia.data, &x[2].a.data, sizeof x[2].a.data);
	memcpy(&x[2].ib.data, &x[2].a.data, sizeof x[2].a.data);
	printf("Three distinct " QUOTE(_LINK_NAME) "-elements: "
		"%s, %s, %s.\n", x[0].str, x[1].str, x[2].str);
	/* add to the sequences a, b */
	T_(LinkClear)(&a), T_(LinkClear)(&b), T_(LinkClear)(&c);
	T_(LinkClear)(&ia), T_(LinkClear)(&ib), T_(LinkClear)(&ic);
	T_(LinkAdd)(&a, &x[0].a), T_(LinkAdd)(&a, &x[1].a);
	T_(LinkAdd)(&b, &x[0].b), T_(LinkAdd)(&b, &x[2].b);
	printf("Two " QUOTE(_LINK_NAME) "-sequences, %s and %s.\n",
		   T_L_(Link, ToString)(&a), T_L_(Link, ToString)(&b));
	/* verify comparing things first */
	printf("Comparing-" QUOTE(_LINK_NAME) " a = %s and c = %s.\n",
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
	if(_T_L_(data, cmp)(&x[1].ia.data, &x[2].ib.data) < 0) {
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
	if(_T_L_(data, cmp)(&x[1].ia.data, &x[2].ib.data) < 0) {
		T_(LinkAdd)(&ic, &x[1].ia), T_(LinkAdd)(&ic, &x[2].ib);
	} else {
		T_(LinkAdd)(&ic, &x[2].ib), T_(LinkAdd)(&ic, &x[1].ia);
	}
	assert(!T_L_(Link, Compare)(&a, &ia));
	assert(!T_L_(Link, Compare)(&b, &ib));
	assert(!T_L_(Link, Compare)(&c, &ic));
}

static void _T_L_(test, order)(void) {
	struct T_(LinkNode) buf[10], *node;
	const size_t buf_size = sizeof buf / sizeof *buf;
	struct T_(Link) a, b;
	size_t i;
	T_(LinkClear)(&a), T_(LinkClear)(&b);
	for(i = 0; i < buf_size; i++) node = buf + i, _T_(filler)(&node->data);
	for(i = 0; i < buf_size >> 1; i++) T_(LinkAdd)(&a, node--);
	for( ; i < buf_size; i++) T_(LinkAdd)(&b, node--);
	assert(_T_L_(count, elements)(&a) == buf_size >> 1);
	assert(_T_L_(count, elements)(&b) == buf_size - (buf_size >> 1));
	T_(LinkSort)(&a), T_(LinkSort)(&b);
	T_(LinkTake)(&a, &b);
	printf("Testing " QUOTE(LINK_NAME) "-" QUOTE(_LINK_NAME) " a, b for order "
		"on <" QUOTE(LINK_NAME) ">LinkTake(a, b).\n");
	assert(_T_L_(count, elements)(&a) == buf_size);
	assert(_T_L_(count, elements)(&b) == 0);
	/* technically, \${(1/2)^(buf_size/2+1)} change of getting this one wrong */
	assert(_T_(exactly_unordered)(&a, (size_t)1));
	/* done now merge */
	T_(LinkClear)(&a), T_(LinkClear)(&b);
	T_(LinkSort)(&a), T_(LinkSort)(&b);
	node = buf;
	for(i = 0; i < buf_size >> 1; i++) T_(LinkAdd)(&a, node++);
	for( ; i < buf_size; i++) T_(LinkAdd)(&b, node++);
	assert(_T_L_(count, elements)(&a) == buf_size >> 1);
	assert(_T_L_(count, elements)(&b) == buf_size - (buf_size >> 1));
	T_(LinkMerge)(&a, &b);
	printf("Testing " QUOTE(LINK_NAME) "-" QUOTE(_LINK_NAME) " a, b for order "
		"on <" QUOTE(LINK_NAME) ">LinkMerge(a, b).\n");
	assert(_T_L_(count, elements)(&a) == buf_size);
	assert(_T_L_(count, elements)(&b) == 0);
	assert(_T_(in_order)(&a));
}

static void _T_L_(test, meta)(void) {
	struct T_(LinkNode) nodes[256], *node = nodes;
	const size_t nodes_size = sizeof nodes / sizeof *nodes;
	struct T_(Link) links[32];
	const size_t links_size = sizeof links / sizeof *links;
	size_t i, nodes_left = nodes_size, links_left = links_size;

	printf("An array of lists of " T_NAME ".\n");
	for(i = 0; i < nodes_size; i++) _T_(filler)(&nodes[i].data);
	while(links_left) {
		struct T_(Link) *const link = links + links_size - links_left;
		int take = (int)((double)nodes_left / links_left
			+ 0.4 * (links_left - 1) * (2.0 * rand() / (1.0 + RAND_MAX) - 1.0));
		if(take < 0) take = 0;
		else if((size_t)take > nodes_left) take = (int)nodes_left;
		nodes_left -= take;
		T_(LinkClear)(link);
		while(take) T_(LinkAdd)(link, node++), take--;
		links_left--;
		printf("%lu. %s\n", links_size - links_left,T_L_(Link, ToString)(link));
	}
	printf("Sorting with qsort by " QUOTE(_LINK_NAME) ".\n");
	qsort(links, links_size, sizeof *links,
		(int (*)(const void *, const void *))&T_L_(Link, Compare));
	for(i = 0; i < links_size; i++) {
		printf("%lu. %s\n", i + 1, T_L_(Link, ToString)(links + i));
		if(i) { /* like {strcmp} comparing the first letter -- good enough */
			const struct T_(LinkNode) *const less = links[i - 1].L_(first),
				*const more = links[i].L_(first);
			if(!less) continue;
			assert(more);
			assert(_T_L_(data, cmp)(&less->data, &more->data) <= 0);
		}
	}
}

#endif /* compare --> */

/* all list tests */
static void _T_L_(test, list)(void) {
	printf("Link<" QUOTE(LINK_NAME) "> linked-list "
		   QUOTE(_LINK_NAME) ":\n");
	_T_L_(test, basic)();
	_T_L_(test, memory)();
	_T_L_(test, boolean)();
	_T_L_(test, order)();
	_T_L_(test, meta)();
	/* fixme: assumes {_LINK_COMPARATOR} is defined on all */
}



/* undefine stuff for the next */
#undef _LINK_NAME
#ifdef _LINK_COMPARATOR /* <-- comp */
#undef _LINK_COMPARATOR
#endif /* comp --> */

#endif /* _LINK_NAME --> */
