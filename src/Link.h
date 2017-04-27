/** 2017 Neil Edelman, distributed under the terms of the MIT License;
 see readme.txt, or \url{ https://opensource.org/licenses/MIT }.

 A linked-list of an existing type. This supports four different linked-lists
 ({[A, D]}) per type. However, each type can be associated to at most one
 {Link} type, (without nesting.)

 @param LINK_NAME, LINK_TYPE
 The name that becomes {T} and a valid type associated therewith. Must each be
 present.
 @param LINK_[A-D]_NAME, LINK_[A-D]_COMPARATOR
 The name that becomes {I}, a linked-list. One can have up to four, {[A, D]},
 linked-lists inside the same {struct}. You can define an optional comparator,
 an equivalence relation function implementing {<T>Comparator}. An inlined
 function takes full advantage of the fixed-function sort, {id est}, it should
 be a {static} function, if possible.
 @param LINK_TO_STRING
 Optional print function implementing {<T>ToString}.

 @title		Link.h
 @author	Neil
 @std		C89/90
 @version	1.0; 2017-05
 @since		1.0; 2017-05 separated from List.h */



/* original #include in the user's C file, and not from calling recursively */
#if !defined(_LINK_NAME) /* <-- !_LINK_NAME */



#include <assert.h>	/* assert */



/* <-- ugly
 This messily defines the "unused" macro.
 @author  Neil
 @std     C89/90
 @version 1.1; 2017-05
 @since   1.0; 2017-01 */
#ifdef UNUSED
#undef UNUSED
#endif
#ifndef _MSC_VER /* <-- not msvc */
#define UNUSED(a) while(0 && (a));
#else /* not msvc --><-- msvc: not a C89/90 compiler; needs a little help */
#pragma warning(push)
/* "Assignment within conditional expression." No. */
#pragma warning(disable: 4706)
/* "<ANSI89/ISO90 name>: The POSIX name for this item is deprecated. Instead
 use the ISO C and C++ conformant name <ISO C++11 name>." */
#pragma warning(disable: 4996)
/* the VC pre-compiler is a little too smart for it's own good,
 http://stackoverflow.com/questions/4851075/universally-compiler-independent
 -way-of-implementing-an-unused-macro-in-c-c */
#define UNUSED(a) (void)(sizeof((a), 0))
#endif /* msvc --> */
/* ugly --> */



/* check defines */

#ifndef LINK_NAME
#error Link generic LINK_NAME undefined.
#endif

#ifndef LINK_TYPE
#error Link generic LINK_TYPE undefined.
#endif

/* A..D is just arbitrary; more could be added, just search: [A, D] */
#if defined(LINK_A_COMPARATOR) && !defined(LINK_A_NAME)
#error Link: LINK_A_COMPARATOR requires LINK_A_NAME.
#endif
#if defined(LINK_B_COMPARATOR) && !defined(LINK_B_NAME)
#error Link: LINK_B_COMPARATOR requires LIST_B_NAME.
#endif
#if defined(LINK_C_COMPARATOR) && !defined(LINK_C_NAME)
#error List: LINK_C_COMPARATOR requires LINK_C_NAME.
#endif
#if defined(LINK_D_COMPARATOR) && !defined(LINK_D_NAME)
#error List: LINK_D_COMPARATOR requires LINK_D_NAME.
#endif



/* After this block, the preprocessor replaces T with LIST_TYPE, T_(X) with
 LIST_NAMEX, _T_(X) with _LIST_NAME_X, and T_NAME with the string
 version.
 http://c-faq.com/decl/namespace.html "All identifiers beginning with an
 underscore are reserved for ordinary identifiers (functions, variables,
 typedefs, enumeration constants) with file scope." "You may use identifiers
 consisting of an underscore followed by a digit or lower case letter at
 function, block, or prototype scope." I use a leading underscore to implement
 private functions or data in the hopes that it will not conflict with existing
 code.
 http://stackoverflow.com/questions/16522341/pseudo-generics-in-c */
#ifdef CAT
#undef CAT
#endif
#ifdef CAT_
#undef CAT_
#endif
#ifdef PCAT
#undef PCAT
#endif
#ifdef PCAT_
#undef PCAT_
#endif
#ifdef T
#undef T
#endif
#ifdef T_
#undef T_
#endif
#ifdef _T_
#undef _T_
#endif
#ifdef T_NAME
#undef T_NAME
#endif
#ifdef QUOTE
#undef QUOTE
#endif
#ifdef QUOTE_
#undef QUOTE_
#endif
#define CAT_(x, y) x ## y
#define CAT(x, y) CAT_(x, y)
#define PCAT_(x, y) _ ## x ## _ ## y
#define PCAT(x, y) PCAT_(x, y)
#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)
#define T_(thing) CAT(LINK_NAME, thing)
#define _T_(thing) PCAT(LINK_NAME, thing)
#define T_NAME QUOTE(LINK_NAME)

/* Troubles with this line? check to ensure that LINK_TYPE is a valid type,
 whose definition is placed above <#include "Link.h">. */
typedef LINK_TYPE _T_(Type);
#define T _T_(Type)

/* [A, D] */
#ifdef _IA_
#undef _IA_
#undef T_IA_
#undef _T_IA_
#endif
#ifdef _IB_
#undef _IB_
#undef T_IB_
#undef _T_IB_
#endif
#ifdef _IC_
#undef _IC_
#undef T_IC_
#undef _T_IC_
#endif
#ifdef _ID_
#undef _ID_
#undef T_ID_
#undef _T_ID_
#endif
#ifdef LINK_A_NAME
#define _IA_(thing) PCAT(LINK_A_NAME, thing) /* data, exclusively */
#define T_IA_(thing1, thing2) CAT(CAT(LINK_NAME, thing1), \
	CAT(LINK_A_NAME, thing2)) /* public fn's */
#define _T_IA_(thing1, thing2) CAT(PCAT(LINK_NAME, thing1), \
	PCAT(LINK_A_NAME, thing2)) /* private fn's */
#endif
#ifdef LINK_B_NAME
#define _IB_(thing) PCAT(LINK_B_NAME, thing)
#define T_IB_(thing1, thing2) CAT(CAT(LINK_NAME, thing1), \
	CAT(LINK_B_NAME, thing2))
#define _T_IB_(thing1, thing2) CAT(PCAT(LINK_NAME, thing1), \
	PCAT(LINK_B_NAME, thing2))
#endif
#ifdef LINK_C_NAME
#define _IC_(thing) PCAT(LINK_C_NAME, thing)
#define T_IC_(thing1, thing2) CAT(CAT(LINK_NAME, thing1), \
	CAT(LINK_C_NAME, thing2))
#define _T_IC_(thing1, thing2) CAT(PCAT(LINK_NAME, thing1), \
	PCAT(LINK_C_NAME, thing2))
#endif
#ifdef LINK_D_NAME
#define _ID_(thing) PCAT(LINK_D_NAME, thing)
#define T_ID_(thing1, thing2) CAT(CAT(LINK_NAME, thing1), \
	CAT(LINK_D_NAME, thing2))
#define _T_ID_(thing1, thing2) CAT(PCAT(LINK_NAME, thing1), \
	PCAT(LINK_D_NAME, thing2))
#endif



/* constants across multiple includes in the same translation unit */
#ifndef _LINK_H /* <-- _LINK_H */
#define _LINK_H
/* combine_sets() operations bit-vector */
enum LinkOperation {
	LO_SUBTRACTION_AB	= 1,
	LO_SUBTRACTION_BA	= 2,
	LO_INTERSECTION		= 4,
	LO_DEFAULT_A		= 8,
	LO_DEFAULT_B		= 16
};
#endif /* _LINK_H */



/** Operates by side-effects only. */
typedef void (*T_(Action))(T *const);
/** Returns (non-zero) true or (zero) false. */
typedef int  (*T_(Predicate))(T *const);
/** Compares two values and returns less then, equal to, or greater then
 zero. */
typedef int  (*T_(Comparator))(const T *, const T *);
#ifdef LINK_TO_STRING
/** Responsible for turning {<T>} (the first argument) into a 9 {char}
 null-terminated output string (the second.) */
typedef void (*T_(ToString))(const T *, char (*const)[9]);
#endif



/** Linked-list {<T>Link} derived from {<T>}. Intended to be used directly in
 {struct}s. Use the following functions to interact with it transparently. */
struct T_(Link);
struct T_(Link) {
#ifdef LINK_A_NAME
	struct T_(Link) *_IA_(prev), *_IA_(next);
#endif
#ifdef LINK_B_NAME
	struct T_(Link) *_IB_(prev), *_IB_(next);
#endif
#ifdef LINK_C_NAME
	struct T_(Link) *_IC_(prev), *_IC_(next);
#endif
#ifdef LIST_D_NAME
	struct T_(Link) *_ID_(prev), *_ID_(next);
#endif
	T data;
};

/** Serves as an a head for a linked-list of {<T>Link}s. One instantiates
 this by setting the pointers to null, for example,
 {struct FooLinked foolist = { 0 };} (Although \url{
 http://stackoverflow.com/questions/1538943/why-is-the-compiler-throwing-this-warning-missing-initializer-isnt-the-stru
 }.) */
struct T_(Linked);
struct T_(Linked) {
#ifdef LINK_A_NAME
	struct T_(Link) *_IA_(first);
#endif
#ifdef LINK_B_NAME
	struct T_(Link) *_IB_(first);
#endif
#ifdef LINK_C_NAME
	struct T_(Link) *_IC_(first);
#endif
#ifdef LIST_D_NAME
	struct T_(Link) *_ID_(first);
#endif
};



#ifdef LINK_TO_STRING /* <-- to string */
/* Stuck here? Check that LINK_TO_STRING is a function implementing
 <T>ToString. */
static const T_(ToString) _T_(to_string) = (LINK_TO_STRING);
/* fixme: prototype? */
#endif /* to string --> */



/* Note to future self: recursive includes. The {_LINK_NAME} pre-processor flag
 controls this behaviour; we are currently in the {!_LIST_NAME} section. These
 will get all the functions with {<I>} in them. */

#ifdef LINK_A_NAME /* <-- a */
#define _LINK_NAME LINK_A_NAME
#ifdef LINK_A_COMPARATOR /* <-- comp */
#define _LINK_COMPARATOR LINK_A_COMPARATOR
#endif /* comp --> */
#include "Link.h"
#endif /* a --> */

#ifdef LINK_B_NAME /* <-- b */
#define _LINK_NAME LINK_B_NAME
#ifdef LINK_B_COMPARATOR /* <-- comp */
#define _LINK_COMPARATOR LINK_B_COMPARATOR
#endif /* comp --> */
#include "Link.h"
#endif /* b --> */

#ifdef LINK_C_NAME /* <-- c */
#define _LINK_NAME LINK_C_NAME
#ifdef LINK_C_COMPARATOR /* <-- comp */
#define _LINK_COMPARATOR LINK_C_COMPARATOR
#endif /* comp --> */
#include "Link.h"
#endif /* c --> */

#ifdef LINK_D_NAME /* <-- d */
#define _LINK_NAME LINK_D_NAME
#ifdef LINK_D_COMPARATOR /* <-- comp */
#define _LINK_COMPARATOR LINK_D_COMPARATOR
#endif /* comp --> */
#include "Link.h"
#endif /* d --> */



/** Initialises the contents of all links of {elem} at the first spot in {this}.
 Does not do any checks on {elem} and overwrites the data that was there.
 Specifically, it invokes undefined behaviour to one add {elem} to more than
 one list without removing it each time.
 @allow */
static struct T_(Linked) *T_(LinkedAdd)(struct T_(Linked) *const this,
	struct T_(Link) *const elem) {
	if(!this) return 0;
	if(!elem) return this;
#ifdef LINK_A_NAME
	_T_IA_(link, init)(this, elem);
#endif
#ifdef LINK_B_NAME
	_T_IB_(link, init)(this, elem);
#endif
#ifdef LINK_C_NAME
	_T_IC_(link, init)(this, elem);
#endif
#ifdef LINK_D_NAME
	_T_ID_(link, init)(this, elem);
#endif
	return this;
}

/** Removes {elem} from the {this}. The {elem} is now free to add to another
 list. Removing an element that was not added to {this} results in undefined
 behaviour.
 @allow */
static struct T_(Linked) *T_(LinkedRemove)(struct T_(Linked) *const this,
	struct T_(Link) *const elem) {
	if(!this) return 0;
	if(!elem) return this;
#ifdef LINK_A_NAME
	_T_IA_(link, remove)(this, elem);
#endif
#ifdef LINK_B_NAME
	_T_IB_(link, remove)(this, elem);
#endif
#ifdef LINK_C_NAME
	_T_IC_(link, remove)(this, elem);
#endif
#ifdef LINK_D_NAME
	_T_ID_(link, remove)(this, elem);
#endif
	return this;
}


/** Get a pointer to the underlying data stored in {<T>}.
 @allow */
static T *T_(LinkGet)(struct T_(Link) *const this) {
	assert(this);
	return &this->data;
}

#ifdef LINK_FILLER /* <-- test */
#include "LinkTest.h" /* need this file if one is going to run tests */
#endif /* test --> */



/* un-define all macros */
#undef LINK_NAME
#undef LINK_TYPE
#ifdef LINK_TO_STRING
#undef LINK_TO_STRING
#endif
#ifdef LINK_FILLER
#undef LINK_FILLER
#endif
#ifdef LINK_A_NAME
#undef LINK_A_NAME
#endif
#ifdef LINK_A_COMPARATOR
#undef LINK_A_COMPARATOR
#endif
#ifdef LINK_B_NAME
#undef LINK_B_NAME
#endif
#ifdef LINK_B_COMPARATOR
#undef LINK_B_COMPARATOR
#endif
#ifdef LINK_C_NAME
#undef LINK_C_NAME
#endif
#ifdef LINK_C_COMPARATOR
#undef LINK_C_COMPARATOR
#endif
#ifdef LINK_D_NAME
#undef LINK_D_NAME
#endif
#ifdef LINK_D_COMPARATOR
#undef LINK_D_COMPARATOR
#endif
#ifdef LINK_DYNAMIC_EXTRA_STORAGE
#undef LINK_DYNAMIC_EXTRA_STORAGE
#endif
#ifdef _LINK_SORT_INTERNALS
#undef _LINK_SORT_INTERNALS /* each List type has their own */
#endif



/* <- ugly */
#ifdef _MSC_VER /* <- MSVC */
#pragma warning(pop)
#endif /* MSVC -> */
/* ugly -> */



#else /* !_LINK_NAME --><-- _LINK_NAME

 Internally #included.

 @param _LIST_NAME: A unique name; required;
 @param _LIST_COMPARATOR: an optional comparator. */

/* After this block, the preprocessor replaces T_M_(X, Y) with
 LINK_NAMEX_LINK_NAMEY, _T_M_(X, Y) with _LINK_NAME_X__LINK_NAME_Y */
#ifdef T_I_
#undef T_I_
#endif
#ifdef _T_I_
#undef _T_I_
#endif
#ifdef I_
#undef I_
#endif
#define T_I_(thing1, thing2) CAT(CAT(LINK_NAME, thing1),CAT(_LINK_NAME, thing2))
#define _T_I_(thing1, thing2) CAT(PCAT(LINK_NAME, thing1),\
	PCAT(_LINK_NAME, thing2))
#define I_(thing) PCAT(_LINK_NAME, thing)



/* private prototypes */

/*static void grow(int *const a, int *const b);
static void swap(int *const a, int *const b);
static int difference(int *const a, int *const b);
static int fn(const struct Order *const order);
static void usage(void);*/

#ifdef _LINK_COMPARATOR /* <-- comp */

static const T_(Comparator) _T_I_(elem, cmp) = (_LINK_COMPARATOR);

/* Merge sort internals only once per translation unit. */

#ifndef _LINK_SORT_INTERNALS /* <!-- sort internals */
#define _LINK_SORT_INTERNALS

/* A run is a temporary sequence of values in the array that is weakly
 increasing. */
struct _T_(Run) {
	struct T_(Link) *head, *tail;
	size_t size;
};

/* Store the maximum capacity for the indexing with size_t. (Overkill, really.)
 range(runs) = Sum_{k=0}^runs 2^{runs-k} - 1
             = 2^{runs+1} - 2
 2^bits      = 2 (r^runs - 1)
 runs        = log(2^{bits-1} + 1) / log 2
 runs       <= 2^{bits - 1}, 2^{bits + 1} > 0 */
struct _T_(Runs) {
	struct _T_(Run) run[(sizeof(size_t) << 3) - 1];
	size_t run_no;
};

#endif /* sort internals --> */


static struct _T_(Runs) _T_I_(runs, elem);

/** Inserts the first element from the larger of two sorted runs, then merges
 the rest. \cite{Peters2002Timsort}, via \cite{McIlroy1993Optimistic}, does
 long merges by galloping, but we don't have random access to the data. In
 practice, this is {2%} slower on randomly distributed keys when the
 linked-list size is over {500 000}; randomly distributed keys have high
 insertion times that to well in standard merging. However, it's (potentially
 much) faster when the keys have structure: observed, {[-2%, 500%]}. Assumes
 array contains at least 2 elements and there are at least two runs. */
static void _T_I_(natural, merge)(struct _T_(Runs) *const r) {
	struct _T_(Run) *const run_a = r->run + r->run_no - 2;
	struct _T_(Run) *const run_b = run_a + 1;
	struct T_(Link) *a = run_a->tail, *b = run_b->head, *chosen;

	/* fixme: we are doing one-to-many compares in some cases? */

	if(run_a->size <= run_b->size) {
		struct T_(Link) *prev_chosen;

		/* run a is smaller: downwards insert b.head followed by upwards
		 merge */

		/* insert the first element of b downwards into a */
		for( ; ; ) {
			if(_T_I_(elem, cmp)(&a->data, &b->data) <= 0) {
				chosen = a;
				a = a->I_(next);
				break;
			}
			if(!a->I_(prev)) {
				run_a->head = run_b->head;
				chosen = b;
				b = b->I_(next);
				break;
			}
			a = a->I_(prev);
		}

		/* merge upwards, while the lists are interleaved */
		while(chosen->I_(next)) {
			prev_chosen = chosen;
			if(_T_I_(elem, cmp)(&a->data, &b->data) > 0) {
				chosen = b;
				b = b->I_(next);
			} else {
				chosen = a;
				a = a->I_(next);
			}
			prev_chosen->I_(next) = chosen;
			chosen->I_(prev) = prev_chosen;
		}

		/* splice the one list left */
		if(!a) {
			b->I_(prev) = chosen;
			chosen->I_(next) = b;
			run_a->tail = run_b->tail;
		} else {
			a->I_(prev) = chosen;
			chosen->I_(next) = a;
		}

	} else {
		struct T_(Link) *next_chosen;
		int is_a_tail = 0;

		/* run b is smaller; upwards insert followed by downwards merge */

		/* insert the last element of a upwards into b */
		for( ; ; ) {
			if(_T_I_(elem, cmp)(&a->data, &b->data) <= 0) {
				chosen = b;
				b = b->I_(prev);
				break;
			}
			/* here, a > b */
			if(!b->I_(next)) {
				is_a_tail = -1;
				chosen = a;
				a = a->I_(prev);
				break;
			}
			b = b->I_(next);
		}
		if(!is_a_tail) run_a->tail = run_b->tail;

		/* merge downwards, while the lists are interleaved */
		while(chosen->I_(prev)) {
			next_chosen = chosen;
			if(_T_I_(elem, cmp)(&a->data, &b->data) > 0) {
				chosen = a;
				a = a->I_(prev);
			} else {
				chosen = b;
				b = b->I_(prev);
			}
			next_chosen->I_(prev) = chosen;
			chosen->I_(next) = next_chosen;
		}

		/* splice the one list left */
		if(!a) {
			b->I_(next) = chosen;
			chosen->I_(prev) = b;
			run_a->head = run_b->head;
		} else {
			a->I_(next) = chosen;
			chosen->I_(prev) = a;
		}

	}

	run_a->size += run_b->size;
	r->run_no--;
}

/** It's kind of experimental. It hasn't been optimised; I think it does
 useless compares and I question whether a strict Pascal's triangle-shape
 would be optimum, or whether a long run should be put off merging until
 short runs have finished; it is quite simple as it is. */
static void _T_I_(natural, sort)(struct T_(Linked) *const this) {
	/* new_run is an index into link_runs, a temporary sorting structure;
	 head is first smallest, tail is last largest */
	struct _T_(Run) *new_run;
	/* part of the state machine for classifying points wrt their neighbours */
	enum { UNSURE, INCREASING, DECREASING } mono;
	/* the data that we are sorting */
	struct T_(Link) *a, *b, *c, *first_iso_a;
	/* run_count is different from link_runs.run_no in that it only increases;
	 only used for calculating the path up the tree */
	size_t run_count, rc;
	/* the value of the comparison */
	int comp;

	/* ensure we have an 'a' */
	if(!(a = this->I_(first))) return;

	/* reset the state machine and output to just 'a' in the first run */
	mono = UNSURE;
	_T_I_(runs, elem).run_no = 1;
	new_run = _T_I_(runs,elem).run + 0, run_count = 1;
	new_run->size = 1;
	first_iso_a = new_run->head = new_run->tail = a;

	for(b = a->I_(next); b; a = b, b = c) {

		/* b.next can be modified, and we always want the iteration original */
		c = b->I_(next);

		comp = _T_I_(elem, cmp)(&a->data, &b->data);

		/* state machine that considers runs in both directions -- in practice,
		 slightly slower than only considering increasing runs on most cases;
		 however, I would hate to see my code replaced with one line; reverse
		 order is 15 times faster, but it's not likely */
		if(comp < 0) { /* a < b, increasing -- good */
			if(mono != DECREASING) { /* if decreasing, inflection */
				mono = INCREASING;
				new_run->size++;
				continue;
			}
		} else if(comp > 0) { /* decreasing; reverse preserving stability */
			if(mono != INCREASING) { /* if increasing, inflection */
				mono = DECREASING;
				b->I_(next) = first_iso_a;
				first_iso_a->I_(prev) = b;
				new_run->head = first_iso_a = b;
				new_run->size++;
				continue;
			}
			new_run->tail = a; /* terminating an increasing sequence */
		} else { /* a == b */
			if(mono == DECREASING) { /* extend */
				struct T_(Link) *const a_next = a->I_(next);
				b->I_(next) = a_next;
				a_next->I_(prev) = b;
				a->I_(next) = b;
				b->I_(prev) = a;
			} else { /* weakly increasing */
				new_run->tail = b;
			}
			new_run->size++;
			continue;
		}
		/* head and tail don't necessarily correspond to the first and last */
		new_run->head->I_(prev) = new_run->tail->I_(next) = 0;

		/* greedy merge: keeps space to O(log n) instead of O(n) */
		for(rc = run_count; !(rc & 1) && _T_I_(runs,elem).run_no >= 2; rc >>= 1)
			_T_I_(natural, merge)(&_T_I_(runs,elem));
		/* reset the state machine and output to just 'b' at the next run */
		mono = UNSURE;
		new_run = _T_I_(runs,elem).run + _T_I_(runs,elem).run_no++, run_count++;
		new_run->size = 1;
		new_run->head = new_run->tail = first_iso_a = b;
	}

	/* terminating the last increasing sequence */
	if(mono == INCREASING) new_run->tail = a;
		new_run->tail->I_(next) = new_run->head->I_(prev) = 0;

	/* clean up the rest; when only one run, propagate link_runs[0] to head */
	while(_T_I_(runs, elem).run_no > 1)
		_T_I_(natural, merge)(&_T_I_(runs, elem));
	/*this->I_(first) = _T_I_(runs,elem).run[0].head;
	this->I_(last)  = _T_I_(runs,elem).run[0].tail;*/
	this->I_(first) = _T_I_(runs,elem).run[0].head;

#ifdef LINK_DEBUG_MERGE
	printf("sorted by " QUOTE(_LIST_INDEX_NAME) ": %s. -->\n",
		   T_I_(List, ToString)(this));
#endif
}

/** Greedy natural insertion-merge sort on doubly-linked lists is very adaptive.
 @allow */
static void T_I_(Linked, Sort)(struct T_(Linked) *const this) {
	if(!this) return;
	_T_I_(natural, sort)(this);
}

#endif /* comp --> */

static void _T_I_(link, init)(struct T_(Linked) *const this,
	struct T_(Link) *const elem) {
	assert(this);
	assert(elem);
	elem->I_(prev) = 0;
	elem->I_(next) = this->I_(first);
	if(this->I_(first)) this->I_(first)->I_(prev) = elem;
	this->I_(first) = elem;
}

static void _T_I_(link, remove)(struct T_(Linked) *const this,
	struct T_(Link) *const elem) {
	assert(this);
	assert(elem);
	if(elem->I_(prev)) {
		elem->I_(prev)->I_(next) = elem->I_(next);
	} else {
		assert(this->I_(first) == elem);
		this->I_(first) = elem->I_(next);
	}
	if(elem->I_(next)) {
		elem->I_(next)->I_(prev) = elem->I_(prev);
	}
	elem->I_(prev) = 0;
	elem->I_(next) = 0;
}





/************
 * printing */

#ifdef LINK_TO_STRING /* <-- print */

#ifndef _LINK_PRINT_THINGS /* <-- once inside translation unit */
#define _LINK_PRINT_THINGS

static const char *const _link_start     = "[ ";
static const char *const _link_end       = " ]";
static const char *const _link_alter_end = "...]";
static const char *const _link_sep       = ", ";
static const char *const _link_star      = "*";
static const char *const _link_null      = "Null";

struct _ListSuperCat {
	char *print, *cursor;
	size_t left;
	int is_truncated;
};
static void _list_super_cat_init(struct _ListSuperCat *const cat,
	char *const print, const size_t print_size) {
	cat->print = cat->cursor = print;
	cat->left = print_size;
	cat->is_truncated = 0;
	print[0] = '\0';
}
static void _list_super_cat(struct _ListSuperCat *const cat,
	const char *const append) {
	size_t lu_took; int took;
	if(cat->is_truncated) return;
	took = snprintf(cat->cursor, cat->left, "%s", append);
	if(took < 0)  { cat->is_truncated = -1; return; } /*implementation defined*/
	if(took == 0) { return; }
	if((lu_took=took)>=cat->left) {cat->is_truncated=-1,lu_took=cat->left-1;}
	cat->cursor += lu_took, cat->left -= lu_took;
}
#endif /* once --> */

/** Prints the linked-list in a static buffer; one can print 4 things at once
 before it overwrites.
 @allow */
static char *T_I_(Linked, ToString)(const struct T_(Linked) *const this) {
	static char buffer[4][256];
	static int buffer_i;
	struct _ListSuperCat cat;
	char scratch[9];
	struct T_(Link) *link;
	int is_first = 1, i = 0;
	assert(strlen(_link_alter_end) >= strlen(_link_end));
	assert(sizeof buffer > strlen(_link_alter_end));
	_list_super_cat_init(&cat, buffer[buffer_i],
		sizeof *buffer / sizeof **buffer - strlen(_link_alter_end));
	buffer_i++, buffer_i &= 3;
	if(!this || !(link = this->I_(first))) {
		_list_super_cat(&cat, _link_null);
		return cat.print;
	}
	_list_super_cat(&cat, _link_start);
	do {
		i++;
		if(!is_first) _list_super_cat(&cat, _link_sep); else is_first = 0;
		_T_(to_string)(&link->data, &scratch), scratch[8] = '\0';
		_list_super_cat(&cat, scratch);
		if(cat.is_truncated) break;
	} while((link = link->I_(next)));
	sprintf(cat.cursor, "%s", cat.is_truncated ? _link_alter_end : _link_end);
	return cat.print; /* static buffer */
}

#endif /* print --> */




/* undefine stuff for the next */
#undef _LINK_NAME
#ifdef _LINK_COMPARATOR /* <-- comp */
#undef _LINK_COMPARATOR
#endif /* comp --> */

#endif /* _LINK_NAME */
