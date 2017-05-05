/* intended to be included by ../src/Link.h on LINK_TEST */

#ifndef _LINK_NAME /* <-- !_LINK_NAME */





/* prototype */
static int _T_(in_order)(struct T_(Link) *const this);
static void _T_(print_all)(struct T_(Link) *const this);

/* LIST_TEST must be a function that implements {<T>Action}. */
static const T_(Action) _T_(filler) = (LINK_TEST);

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
	UNUSED(this);
	assert(this);
	return -1
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

static void _T_(print_all)(struct T_(Link) *const this) {
	UNUSED(this);
	assert(this);
#ifdef LINK_A_NAME
	printf(".%8s: %s.\n", QUOTE(LINK_A_NAME), T_LA_(Link, ToString)(this));
#endif
#ifdef LINK_B_NAME
	printf(".%8s: %s.\n", QUOTE(LINK_B_NAME), T_LB_(Link, ToString)(this));
#endif
#ifdef LINK_C_NAME
	printf(".%8s: %s.\n", QUOTE(LINK_C_NAME), T_LC_(Link, ToString)(this));
#endif
#ifdef LINK_D_NAME
	printf(".%8s: %s.\n", QUOTE(LINK_D_NAME), T_LD_(Link, ToString)(this));
#endif
}

#else /* !index --><-- index */

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

/* for \see{_T_L_(exactly, elements)} */
struct T_L_(Link, Verify) {
	size_t i;
	const struct T_(LinkNode) *array;
	size_t array_no;
};
/** \see{_T_L_(exactly, elements)}
 @param param: struct <T>Link<L>Verify
 @implements <T>Predicate */
static int _T_L_(exactly, predicate)(T *const this, void *const param) {
	struct T_L_(Link, Verify) *lv = param;
	if(lv->array_no <= lv->i
		|| memcmp(this, &lv->array[lv->i].data, sizeof *this))
		return fprintf(stderr, "Failed at index %lu.\n", lv->i), 0;
	lv->i++;
	return 1;
}
/** Verifies that the elements are exactly as in {array}. */
static size_t _T_L_(exactly, elements)(struct T_(Link) *const this,
	const struct T_(LinkNode) *array, const size_t array_no) {
	struct T_L_(Link, Verify) lv = { 0, 0, 0 };
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

/** Basic:
 GetData, Clear, Add, Remove, Sort, SetParam, GetNext, GetPrevious, GetFirst, GetLast, <L>Sort, ForEach, ShortCircut, ToString */
static void _T_L_(test, basic)(void) {
	char str[9];
	T *data;
	struct T_(Link) a;
	struct T_(LinkNode) buf[1000], *const new_buf = buf + 2,
		*item_a, *item_b, *item_y, *item_z;
	const size_t buf_size = sizeof buf / sizeof *buf, new_buf_size = buf_size-4;
	size_t i;
	int is_parity = 1;

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
	/* GetData */
	assert(!T_(LinkNodeGetData)(0));
	data = T_(LinkNodeGetData)(item_a);
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
	printf("Sorting all.\n");
	T_(LinkSort)(0);
	T_(LinkSort)(&a);
	assert(_T_(in_order)(&a));
	/* ToString (unchecked) */
	printf("ToString: null = %s; a = %s.\n",
		T_L_(Link, ToString)(0), T_L_(Link, ToString)(&a));
	printf("Clear.\n");
	T_(LinkClear)(&a);
	assert(!_T_L_(count, elements)(&a));
}

#define LINK_BUFFER_SIZE 4
struct _T_L_(Link, Buffers) {
	struct T_(LinkNode) x[LINK_BUFFER_SIZE], y[LINK_BUFFER_SIZE],
		z[LINK_BUFFER_SIZE];
} _T_L_(buf, buf);
#define LINK_BUFFER_BYTES (sizeof(struct T_(LinkNode)) * LINK_BUFFER_SIZE)
static char _T_L_(get, list)(const struct T_(LinkNode) *const node) {
	if(node >= _T_L_(buf, buf).x
		&& node < _T_L_(buf, buf).x + LINK_BUFFER_SIZE) {
		return 'x';
	} else if(node >= _T_L_(buf, buf).y
		&& node < _T_L_(buf, buf).y + LINK_BUFFER_SIZE) {
		return 'y';
	} else if(node >= _T_L_(buf, buf).z
		&& node < _T_L_(buf, buf).z + LINK_BUFFER_SIZE) {
		return 'z';
	} else if(!node) {
		return '0';
	}
	return '?';
}
static void _T_L_(check, list)(const struct T_(Link) *const this,
	int (*const fn)(const char)) {
	struct T_(LinkNode) *node, *prev = 0;
	char a[9], char_list;
	assert(this);
	assert(!this->L_(first) == !this->L_(last));
	for(node = this->L_(first); node; node = node->L_(next)) {
		printf("");
		assert(node->L_(prev) == prev);
		_T_(to_string)(&node->data, &a);
		prev = node;
		char_list = _T_L_(get, list)(node);
		printf("%c(%s)%s", char_list, a, node->L_(next) ? ", " : ".\n");
		if(fn) assert(fn(char_list));
	}
	assert(this->L_(last) == prev);
}
static int _T_L_(verify, x)(const char a) { return a == 'x'; }
static int _T_L_(verify, yz)(const char a) { return a == 'y' || a == 'z'; }
static int _T_L_(verify, z)(const char a) { return a == 'z'; }
/* interleave: (Take, Merge,) (Move,
 ContiguousMove,)(Compare,
 Subtraction, Union, Intersection, Xor, If,)
 (GetData, Clear, Add,
 Remove, Take, Merge,
 \\ Sort,
 SetParam, (Move,
 ContiguousMove,) GetNext, GetPrevious, GetFirst, GetLast, <L>Sort, Compare,
 Subtraction, Union, Intersection, Xor, If, ForEach, ShortCircut, ToString */
static void _T_L_(test, memory)(void) {
	struct T_(Link) a, b;
	struct T_(LinkNode) *node_a, *node_b, *node_c;
	size_t i;
	int is_parity;

	T_(LinkClear)(&a), T_(LinkClear)(&b);
	/* fill the items in bufbuf.x */
	for(i = 0; i < LINK_BUFFER_SIZE; i++) {
		node_a = _T_L_(buf, buf).x + i;
		_T_(filler)(&node_a->data);
	}
	/* copy the items to bufbuf.y */
	memcpy(_T_L_(buf, buf).y, _T_L_(buf, buf).x, LINK_BUFFER_BYTES);
	memset(&_T_L_(buf, buf).z, 0, LINK_BUFFER_BYTES);
	/* put all items in a */
	for(i = 0; i < LINK_BUFFER_SIZE; i++) {
		node_a = _T_L_(buf, buf).x + i;
		node_b = _T_L_(buf, buf).y + i;
		T_(LinkAdd)(&a, node_a);
		T_(LinkAdd)(&a, node_b);
	}
	T_(LinkSort)(&a); /* tests if stable */
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
	printf("a:\n"), _T_(print_all)(&a);
	printf("b:\n"), _T_(print_all)(&b);
	assert(_T_(in_order)(&a));
	assert(_T_L_(in, order)(&b)); /* only <L> is in order */
	/* Compare (needs to be tested more!) */
	assert(!T_L_(Link, Compare)(0, 0));
	assert(T_L_(Link, Compare)(&a, 0) > 0);
	assert(T_L_(Link, Compare)(0, &b) < 0);
	assert(!T_L_(Link, Compare)(&a, &b));
	/* Test done. */
	printf("Moving all a to b.\n");
	T_L_(Link, TakeIf)(&b, &a, 0);
	printf("Now a = %s, b = %s.\n",
		T_L_(Link, ToString)(&a), T_L_(Link, ToString)(&b));
	printf("a:\n"), _T_(print_all)(&a);
	printf("b:\n"), _T_(print_all)(&b);
	assert(_T_L_(count, elements)(&a) == 0);
	assert(_T_L_(count, elements)(&b) == 2 * LINK_BUFFER_SIZE);
	/* Clear (the rest) */
	printf("Clear b.\n");
	T_(LinkClear)(&b);
	assert(_T_L_(count, elements)(&b) == 0);
	/* Test done; move. */
	for(i = 0; i < LINK_BUFFER_SIZE; i++) {
		T_(LinkAdd)(&a, _T_L_(buf, buf).x + i);
		T_(LinkAdd)(&b, _T_L_(buf, buf).y + i);
	}
	assert(_T_L_(exactly, elements)(&a, _T_L_(buf, buf).x, LINK_BUFFER_SIZE));
	assert(_T_L_(exactly, elements)(&b, _T_L_(buf, buf).y, LINK_BUFFER_SIZE));
	assert(!T_L_(Link, Compare)(&a, &b));
	memset(_T_L_(buf, buf).z, 0, LINK_BUFFER_BYTES);
	for(i = 0; i < LINK_BUFFER_SIZE; i += 2) {
		node_b = _T_L_(buf, buf).y + i;
		node_c = _T_L_(buf, buf).z + i;
		memcpy(node_c, node_b, sizeof *node_b);
		memset(node_b, 0, sizeof *node_b);
		T_(LinkMove)(&b, node_b, node_c);
	}
	printf("Testing memory relocation with <" QUOTE(LINK_NAME)
		">LinkMove: a = %s, b = %s.\n",
		T_L_(Link, ToString)(&a), T_L_(Link, ToString)(&b));
	assert(!T_L_(Link, Compare)(&a, &b));
	printf("a: "), _T_L_(check, list)(&a, &_T_L_(verify, x));
	printf("b: "), _T_L_(check, list)(&b, &_T_L_(verify, yz));
	/* Test done; ContiguousMove */
	printf("a:\n"), _T_(print_all)(&a);
	printf("b:\n"), _T_(print_all)(&b);
	memcpy(_T_L_(buf, buf).z, _T_L_(buf, buf).x, LINK_BUFFER_BYTES);
	memset(_T_L_(buf, buf).x, 0, LINK_BUFFER_BYTES);
	T_(LinkBlockMove)(&a, _T_L_(buf, buf).x, LINK_BUFFER_BYTES, _T_L_(buf, buf).z);
	printf("Testing contiguous memory relocation a = %s, b = %s.\n",
		T_L_(Link, ToString)(&a), T_L_(Link, ToString)(&b));
	assert(!T_L_(Link, Compare)(&a, &b));
	_T_L_(check, list)(&a, &_T_L_(verify, x));
	_T_L_(check, list)(&b, &_T_L_(verify, z));
}

#ifdef _LINK_COMPARATOR /* <-- compare */

/* boolean:
 T_(Link), T_(LinkAdd), T_I_(Link, ToString), T_I_(Link, Compare),
 T_I_(LinkAdd, Subtraction), T_(LinkClear), T_I_(LinkAdd, Union),
 T_I_(LinkAdd, Intersection), T_I_(LinkAdd, Xor), T_(Link_) */
static int _T_L_(test, boolean)(void) {
#if 0
	struct T_(Link) *a = 0, *b = 0, *c = 0, *intend_c = 0;
	struct { T t; char str[9]; } x, y, z;
	unsigned i;
	const unsigned limit = 1000;
	enum Error { E_NO, E_LIST, E_HOMO, E_UNEXPECTED } error = E_NO;

	/* try */ do {

		printf("Set operations on " QUOTE(_LIST_INDEX_NAME) ":\n");

		a = T_(Link)();
		b = T_(Link)();
		c = T_(Link)();
		intend_c = T_(Link)();

		/* distinct elements x, y, z */
		_T_(filler)(&x.t), _T_(to_string)(&x.t, &x.str);
		i = 0; do { _T_(filler)(&y.t); i++; }
		while(i < limit && !_T_L_(elem, cmp)(&x.t, &y.t));
		if(i >= limit) { error = E_HOMO; break; }
		_T_(to_string)(&y.t, &y.str);
		i = 0; do { _T_(filler)(&z.t); i++; }
		while(i < limit
			&&(!_T_L_(elem, cmp)(&x.t, &z.t) || !_T_L_(elem, cmp)(&y.t, &z.t)));
		if(i >= limit) { error = E_HOMO; break; }
		_T_(to_string)(&z.t, &z.str);
		printf("Three distinct " QUOTE(_LIST_INDEX_NAME) "-elements: "
			"%s, %s, %s.\n", x.str, y.str, z.str);
		/* fixme: Three distinct elements, Foo, Baz, Baz. */
		/* Three distinct Int-elements: 73, 99, 99. */

		/* add to the sequences a, b */
		T_(LinkAdd)(a, &x.t);
		T_(LinkAdd)(a, &y.t);
		T_(LinkAdd)(b, &x.t);
		T_(LinkAdd)(b, &z.t);
		printf("Two " QUOTE(_LIST_INDEX_NAME) "-sequences, %s and %s.\n",
			T_I_(Link, ToString)(a), T_I_(Link, ToString)(b));

		/* verify comparing things first */
		printf("Comparing-" QUOTE(_LIST_INDEX_NAME) " %s and %s.\n",
			T_I_(Link, ToString)(a), T_I_(Link, ToString)(c));
		if(T_I_(Link, Compare)(a, c) != 1
			|| T_I_(Link, Compare)(c, a) != -1
			|| T_I_(Link, Compare)(a, a) != 0) { error = E_UNEXPECTED; break; }

		/* - */
		if(!T_(LinkAdd)(intend_c, &y.t)
			|| !T_I_(LinkAdd, Subtraction)(c, a, b))
			{ error = E_LIST; break; }
		if(intend_c->size != 1) exit(1);
		printf("%s \\setminus %s = %s.\n", T_I_(Link, ToString)(a),
			T_I_(Link, ToString)(b), T_I_(Link, ToString)(c));
		if(T_I_(Link, Compare)(c, intend_c)) { error = E_UNEXPECTED; break; }
		T_(LinkClear)(c);
		T_(LinkClear)(intend_c);

		/* u */
		if(!T_(LinkAdd)(intend_c, &x.t)) { error = E_LIST; break; }
		if(_T_L_(elem, cmp)(&y.t, &z.t) > 0) {
			if(!T_(LinkAdd)(intend_c, &z.t)
				|| !T_(LinkAdd)(intend_c, &y.t)) { error = E_LIST; break; }
		} else {
			if(!T_(LinkAdd)(intend_c, &y.t)
				|| !T_(LinkAdd)(intend_c, &z.t)) { error = E_LIST; break; }
		}
		if(!T_I_(LinkAdd, Union)(c, a, b)) { error = E_LIST; break; }
		printf("%s \\cup %s = %s.\n", T_I_(Link, ToString)(a),
			T_I_(Link, ToString)(b), T_I_(Link, ToString)(c));
		if(T_I_(Link, Compare)(intend_c, c)) { error = E_UNEXPECTED; break; }
		T_(LinkClear)(c);
		T_(LinkClear)(intend_c);

		/* n */
		if(!T_(LinkAdd)(intend_c, &x.t)
			|| !T_I_(LinkAdd, Intersection)(c, a, b))
			{ error = E_LIST; break; }
		printf("%s \\cap %s = %s.\n", T_I_(Link, ToString)(a),
			T_I_(Link, ToString)(b), T_I_(Link, ToString)(c));
		if(T_I_(Link, Compare)(intend_c, c)) { error = E_UNEXPECTED; break; }
		T_(LinkClear)(c);
		T_(LinkClear)(intend_c);

		/* xor */
		if(_T_L_(elem, cmp)(&y.t, &z.t) > 0) {
			if(!T_(LinkAdd)(intend_c, &z.t)
				|| !T_(LinkAdd)(intend_c, &y.t)) { error = E_LIST; break; }
		} else {
			if(!T_(LinkAdd)(intend_c, &y.t)
				|| !T_(LinkAdd)(intend_c, &z.t)) { error = E_LIST; break; }
		}
		if(!T_I_(LinkAdd, Xor)(c, a, b))
			{ error = E_LIST; break; }
		printf("%s \\oplus %s = %s.\n", T_I_(Link, ToString)(a),
			T_I_(Link, ToString)(b), T_I_(Link, ToString)(c));
		if(T_I_(Link, Compare)(intend_c, c)) { error = E_UNEXPECTED; break; }
		T_(LinkClear)(c);
		T_(LinkClear)(intend_c);

	} while(0);
	/* catch */ switch(error) {
		case E_NO: break;
		case E_LIST:printf("Failed; message: [ %s, %s, %s, %s ].\n",
			T_(LinkGetError)(a), T_(LinkGetError)(b), T_(LinkGetError)(c),
			T_(LinkGetError)(intend_c)); break;
		case E_HOMO: printf("Failed: couldn't get 3 things different from "
			"filler.\n"); break;
		case E_UNEXPECTED: printf("Failed: unexpected result.\n"); break;
	}
	/* finally */ {
		T_(Link_)(&intend_c);
		T_(Link_)(&c);
		T_(Link_)(&b);
		T_(Link_)(&a);
	}

	return error ? 0 : -1;
#endif
	return 1;
}

/* bumping:
 T_(Link), T_(LinkAdd), T_I_(Link, Sort), T_I_(Link, ToString),
 T_I_(Link, Bump), T_(LinkResetCursor), T_(LinkSetParam),
 T_I_(Link, ShortCircuit), T_(Link_) */
static int _T_L_(test, bump)(void) {
#if 0
	struct T_(Link) *list = 0;
	T t, *pt;
	char str[9];
	const unsigned limit = 5; /* small number: first, middle, last */
	unsigned i;
	enum Error { E_NO, E_LIST, E_UNEXPECTED, E_CURSOR } error = E_NO;

	/* try */ do {

		list = T_(Link)();
		for(i = 0; i < limit; i++) {
			_T_(filler)(&t);
			if(!T_(LinkAdd)(list, &t)) { error = E_LIST; break; }
		}
		if(i < limit) break;

		T_I_(Link, Sort)(list);

		_T_(filler)(&t);
		if(!(pt = T_(LinkAdd)(list, &t))) { error = E_LIST; break; }
		_T_(to_string)(pt, &str);
		printf("%s out-of-" QUOTE(_LIST_INDEX_NAME) "-place? %s;\n", str,
			T_I_(Link, ToString)(list));

		T_I_(Link, Bump)(list);
		printf("bump, %s.\n", T_I_(Link, ToString)(list));

		T_(LinkResetCursor)(list);
		_T_(filler)(&t);
		if(!(pt = T_(LinkAdd)(list, &t))) { error = E_LIST; break; }
		_T_(to_string)(pt, &str);
		printf("Added %s, %s;\n", str, T_I_(Link, ToString)(list));

		T_I_(Link, Bump)(list);
		printf("bump, %s.\n", T_I_(Link, ToString)(list));

		/* one index _T_(in_order)(list) */
		if(!_T_L_(in, order)(list)) { error = E_UNEXPECTED; break; }

	} while(0);
	/* catch */ switch(error) {
		case E_NO: break;
		case E_LIST:printf("Failed; message: %%s.\n"/*, T_(LinkGetError)(list)*/);break;
		case E_UNEXPECTED: printf("Failed: unexpected result.\n"); break;
		case E_CURSOR: printf("Failed: cursor position was wrong.\n"); break;
	}
	/* finally */ {
		T_(Link_)(&list);
	}

	return error ? 0 : -1;
#endif
	return 1;
}

/* array:
 T_(Link), T_(LinkAdd), T_I_(Link, ToString), T_(LinkSetCursor), T_(LinkGet),
 T_(Link_) */
static int _T_L_(test, array)(void) {
#if 0
	struct T_(Link) *array_of_lists[16] = { 0 };
	T t;
	const unsigned array_of_lists_size
		= sizeof array_of_lists / sizeof *array_of_lists;
	unsigned i;
	enum Error { E_NO, E_LIST, E_UNEXPECTED } error = E_NO;

	/* elimiates [-Wmaybe-uninitialized] on Linux 4.4.0-62-generic #83-Ubuntu,
	 (I don't know what it's talking about) */
	_T_(filler)(&t);

	/* try */ do {
		printf("Sorting arrays of lists of " T_NAME " by "
			   QUOTE(_LIST_INDEX_NAME) ":\n");
		for(i = 0; i < array_of_lists_size; i++) {
			const unsigned j_limit = (unsigned)((float)rand() / RAND_MAX * 4);
			unsigned j;
			if(!(array_of_lists[i] = T_(Link)())) { error = E_LIST; break; }
			for(j = 0; j < j_limit; j++) {
				_T_(filler)(&t);
				if(!T_(LinkAdd)(array_of_lists[i], &t)) { error = E_LIST;break;}
			}
			if(j != j_limit) break;
			printf("%u. %s\n", i, T_I_(Link, ToString)(array_of_lists[i]));
		}
		if(i != array_of_lists_size) break;
		printf("sorting with qsort,\n");
		qsort(array_of_lists, (size_t)array_of_lists_size,
			sizeof *array_of_lists, &_T_L_(qsort, cmp));
		for(i = 0; i < array_of_lists_size; i++) {
			printf("%u. %s\n", i, T_I_(Link, ToString)(array_of_lists[i]));
			T_(LinkSetCursor)(array_of_lists[i], 0);
		}
		for(i = 0; i < array_of_lists_size - 1; i++) {
			const T *const a = T_(LinkGet)(array_of_lists[i]);
			const T *const b = T_(LinkGet)(array_of_lists[i+1]);
			if(a && (!b || _T_L_(elem, cmp)(a, b) > 0))
				{ error = E_UNEXPECTED; break; }
			T_(Link_)(&array_of_lists[i]);
			if(array_of_lists[i]) { error = E_UNEXPECTED; break; }
		}
		if(i != array_of_lists_size - 1) break;
	} while(0);
	/* catch */ switch(error) {
		case E_NO: break;
		case E_LIST:printf("Failed; message: %s.\n",
			T_(LinkGetError)(array_of_lists[i])); break;
		case E_UNEXPECTED: printf("Failed: unexpected result.\n"); break;
	}
	/* finally */ {
		for(i = 0; i < array_of_lists_size; i++) T_(Link_)(&array_of_lists[i]);
	}

	return error ? 0 : -1;
#endif
	return 1;
}

#endif /* compare --> */

/* all list tests */
static void _T_L_(test, list)(void) {
	printf("Link<" QUOTE(LINK_NAME) "> linked-list "
		   QUOTE(_LINK_NAME) ":\n");
	_T_L_(test, basic)();
	_T_L_(test, memory)();
	 /*#ifdef _LINK_COMPARATOR
	 && _T_L_(test, boolean)()
	 && _T_L_(test, bump)()
	 && _T_L_(test, array)()
	 #endif*/
}



/* undefine stuff for the next */
#undef _LINK_NAME
#ifdef _LINK_COMPARATOR /* <-- comp */
#undef _LINK_COMPARATOR
#endif /* comp --> */

#endif /* index --> */
