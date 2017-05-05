/** 2017 Neil Edelman, distributed under the terms of the MIT License;
 see readme.txt, or \url{ https://opensource.org/licenses/MIT }.

 {<T>Link} stores doubly-linked-list(s) of {<T>LinkNode}, of which data of
 type, {<T>}, must be set using {LINK_TYPE}. The {<T>LinkNode} storage is the
 resposibility of the caller. Specifically, it does not have to be contiguous,
 and it can be nestled in multiple, possibly different, {struct}s. Supports one
 to four different linked-list orders in the same type, {[A, D]}, set using
 {LINK_[A-D]_NAME}. The preprocessor macros are all undefined at the end of the
 file for convenience when including multiple Link types.

 @param LINK_NAME, LINK_TYPE
 The name that becomes {T} and a valid type associated therewith (should be
 conformant to the maximum available length of identifiers.) Must each be
 present before including.

 @param LINK_[A-D]_NAME, LINK_[A-D]_COMPARATOR
 Each {LINK_[A-D]_NAME} becomes {L}. You can define an optional comparator, an
 equivalence relation function implementing {<T>Comparator}. For speed, it
 should be an inlined {static} function, if possible.

 @param LINK_TO_STRING
 Optional print function implementing {<T>ToString} and making available
 \see{<T>Link<L>ToString}. Currently, this uses the C99 function {vsnprintf}.

 @param LINK_DYNAMIC_STORAGE
 This allocates {O(log n)}, space needed for merge sort on the stack every time
 the List is sorted, instead of statically. This allows using the exact same
 sort on different data concurrently without crashing, but it consumes more
 resources.

 @param LINK_OPENMP
 Tries to parallelise using {OpenMP}, \url{ http://www.openmp.org/ }.

 @param LINK_TEST
 Unit testing framework using {<T>LinkTest}, included in a separate header,
 {LinkTest.h}. Must be defined equal to a random filler, satisfying
 {<T>Action}. Also, turns on {assert} private function testing.

 @title		Link.h
 @author	Neil
 @std		C89/90
 @version	1.0; 2017-05
 @since		1.0; 2017-05 separated from List.h
 @fixme		Have specific public function flags to turn off functions so users
 can turn off 'defined but not used' (eg, GCC {-Wunused-function}) warnings.
 @fixme		{GCC} 4.2 has a bug with {-Wconversion} that causes {assert} to
 emit a warning; cast? */



/* original #include in the user's C file, and not from calling recursively */
#if !defined(_LINK_NAME) /* <-- !_LINK_NAME */



#include <stddef.h>	/* ptrdiff_t */
#include <assert.h>	/* assert */
#ifdef LINK_TO_STRING /* <-- print */
#include <stdio.h>	/* vs[n]printf; fixme: C99 */
#endif /* print --> */

/* unused macro */
#ifdef UNUSED
#undef UNUSED
#endif
#define UNUSED(a) ((void)(a))



/* check defines */

#ifndef LINK_NAME
#error Link generic LINK_NAME undefined.
#endif

#ifndef LINK_TYPE
#error Link generic LINK_TYPE undefined.
#endif

/* A..D is just arbitrary; more could be added, just search: [A, D] */
#if !defined(LINK_A_NAME) && !defined(LINK_B_NAME) \
	&& !defined(LINK_C_NAME) && !defined(LINK_D_NAME)
#error Link: must have at least one of LINK_[A-D]_NAME.
#endif
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
#ifndef LINK_TEST
#define NDEBUG
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
#ifdef LA_
#undef LA_
#undef T_LA_
#undef _T_LA_
#endif
#ifdef LB_
#undef LB_
#undef T_LB_
#undef _T_LB_
#endif
#ifdef LC_
#undef LC_
#undef T_LC_
#undef _T_LC_
#endif
#ifdef LD_
#undef LD_
#undef T_LD_
#undef _T_LD_
#endif
#ifdef LINK_A_NAME
#define LA_(thing) PCAT(LINK_A_NAME, thing) /* data, exclusively */
#define T_LA_(thing1, thing2) CAT(CAT(LINK_NAME, thing1), \
	CAT(LINK_A_NAME, thing2)) /* public fn's */
#define _T_LA_(thing1, thing2) CAT(PCAT(LINK_NAME, thing1), \
	PCAT(LINK_A_NAME, thing2)) /* private fn's */
#endif
#ifdef LINK_B_NAME
#define LB_(thing) PCAT(LINK_B_NAME, thing)
#define T_LB_(thing1, thing2) CAT(CAT(LINK_NAME, thing1), \
	CAT(LINK_B_NAME, thing2))
#define _T_LB_(thing1, thing2) CAT(PCAT(LINK_NAME, thing1), \
	PCAT(LINK_B_NAME, thing2))
#endif
#ifdef LINK_C_NAME
#define LC_(thing) PCAT(LINK_C_NAME, thing)
#define T_LC_(thing1, thing2) CAT(CAT(LINK_NAME, thing1), \
	CAT(LINK_C_NAME, thing2))
#define _T_LC_(thing1, thing2) CAT(PCAT(LINK_NAME, thing1), \
	PCAT(LINK_C_NAME, thing2))
#endif
#ifdef LINK_D_NAME
#define LD_(thing) PCAT(LINK_D_NAME, thing)
#define T_LD_(thing1, thing2) CAT(CAT(LINK_NAME, thing1), \
	CAT(LINK_D_NAME, thing2))
#define _T_LD_(thing1, thing2) CAT(PCAT(LINK_NAME, thing1), \
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

/** Passed {T} and {param}, returns (non-zero) true or (zero) false. */
typedef int  (*T_(Predicate))(T *const, void *const);

/** Compares two values and returns less then, equal to, or greater then
 zero. */
typedef int  (*T_(Comparator))(const T *, const T *);

#ifdef LINK_TO_STRING
/** Responsible for turning {<T>} (the first argument) into a 9 {char}
 null-terminated output string (the second.) */
typedef void (*T_(ToString))(const T *, char (*const)[9]);
#endif



/** A single link in the linked-list derived from {<T>}. Intended to be used
 directly in {struct}s. Use the \see{<T>LinkNodeGetData} function to extract
 {<T>}. */
struct T_(LinkNode);
struct T_(LinkNode) {
#ifdef LINK_A_NAME
	struct T_(LinkNode) *LA_(prev), *LA_(next);
#endif
#ifdef LINK_B_NAME
	struct T_(LinkNode) *LB_(prev), *LB_(next);
#endif
#ifdef LINK_C_NAME
	struct T_(LinkNode) *LC_(prev), *LC_(next);
#endif
#ifdef LIST_D_NAME
	struct T_(LinkNode) *LD_(prev), *LD_(next);
#endif
	T data;
};

/** Serves as an a head for linked-list(s) of {<T>LinkNode}. No initialisation
 is necessary when the variable is of {static} duration, otherwise use
 \see{<T>LinkClear}. */
struct T_(Link);
struct T_(Link) {
#ifdef LINK_A_NAME
	struct T_(LinkNode) *LA_(first), *LA_(last);
#endif
#ifdef LINK_B_NAME
	struct T_(LinkNode) *LB_(first), *LB_(last);
#endif
#ifdef LINK_C_NAME
	struct T_(LinkNode) *LC_(first), *LC_(last);
#endif
#ifdef LIST_D_NAME
	struct T_(LinkNode) *LD_(first), *LD_(last);
#endif
	void *param;
};



#ifdef LINK_TO_STRING /* <-- to string */
/* Check that {LINK_TO_STRING} is a function implementing {<T>ToString}. */
static const T_(ToString) _T_(to_string) = (LINK_TO_STRING);
#endif /* to string --> */

/* Prototypes: needed for the next section, but undefined until later. */
static void _T_(add)(struct T_(Link) *const this,
	struct T_(LinkNode) *const node);
static void _T_(remove)(struct T_(Link) *const this,
	struct T_(LinkNode) *const node);

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



/** Private: add to first of list. */
static void _T_(add)(struct T_(Link) *const this,
	struct T_(LinkNode) *const node) {
	assert(this);
	assert(node);
#ifdef LINK_OPENMP /* <-- omp */
	#pragma omp parallel sections
#endif /* omp --> */
	{
#ifdef LINK_A_NAME /* <-- a */
#ifdef LINK_OPENMP /* <-- omp */
		#pragma omp section
#endif /* omp --> */
		_T_LA_(link, add)(this, node);
#endif /* a --> */
#ifdef LINK_B_NAME /* <-- b */
#ifdef LINK_OPENMP /* <-- omp */
		#pragma omp section
#endif /* omp --> */
		_T_LB_(link, add)(this, node);
#endif /* b --> */
#ifdef LINK_C_NAME /* <-- c */
#ifdef LINK_OPENMP /* <-- omp */
		#pragma omp section
#endif /* omp --> */
		_T_LC_(link, add)(this, node);
#endif /* c --> */
#ifdef LINK_D_NAME /* <-- d */
#ifdef LINK_OPENMP /* <-- omp */
		#pragma omp section
#endif /* omp --> */
		_T_LD_(link, add)(this, node);
#endif /* d --> */
	}
}

/** Private: remove from list. */
static void _T_(remove)(struct T_(Link) *const this,
	struct T_(LinkNode) *const node) {
	assert(this);
	assert(node);
#ifdef LINK_OPENMP /* <-- omp */
#pragma omp parallel sections
#endif /* omp --> */
	{
#ifdef LINK_A_NAME /* <-- a */
#ifdef LINK_OPENMP /* <-- omp */
#pragma omp section
#endif /* omp --> */
		_T_LA_(link, remove)(this, node);
#endif /* a --> */
#ifdef LINK_B_NAME /* <-- b */
#ifdef LINK_OPENMP /* <-- omp */
#pragma omp section
#endif /* omp --> */
		_T_LB_(link, remove)(this, node);
#endif /* b --> */
#ifdef LINK_C_NAME /* <-- c */
#ifdef LINK_OPENMP /* <-- omp */
#pragma omp section
#endif /* omp --> */
		_T_LC_(link, remove)(this, node);
#endif /* c --> */
#ifdef LINK_D_NAME /* <-- d */
#ifdef LINK_OPENMP /* <-- omp */
#pragma omp section
#endif /* omp --> */
		_T_LD_(link, remove)(this, node);
#endif /* d --> */
	}
}

/** Private: clear the list. */
static void _T_(clear)(struct T_(Link) *const this) {
	assert(this);
#ifdef LINK_A_NAME
	this->LA_(first) = this->LA_(last) = 0;
#endif
#ifdef LINK_B_NAME
	this->LB_(first) = this->LB_(last) = 0;
#endif
#ifdef LINK_C_NAME
	this->LC_(first) = this->LC_(last) = 0;
#endif
#ifdef LINK_D_NAME
	this->LD_(first) = this->LD_(last) = 0;
#endif
}

/** Get a pointer to the underlying data stored in {this}.
 @allow */
static T *T_(LinkNodeGetData)(struct T_(LinkNode) *const this) {
	if(!this) return 0;
	return &this->data;
}

/** Clears all values from {this}, thereby initialising the {<T>Link}.
 @allow */
static void T_(LinkClear)(struct T_(Link) *const this) {
	if(!this) return;
	_T_(clear)(this);
	this->param = 0;
}

/** Sets the contents of {node} to push it to {this}, thereby initialising the
 non-{<T>} parts of {<T>LinkNode}. Does not do any checks on {node} and
 overwrites the data that was there. Specifically, it invokes undefined
 behaviour to one add {node} to more than one list without removing it each
 time. If either {this} or {node} is null, it does nothing.
 @allow */
static void T_(LinkAdd)(struct T_(Link) *const this,
	struct T_(LinkNode) *const node) {
	if(!this || !node) return;
	_T_(add)(this, node);
}

/** Removes {node} from the {this}. The {node} is now free to add to another
 list. Removing an element that was not added to {this} results in undefined
 behaviour. If either {this} or {node} is null, it does nothing.
 @allow */
static void T_(LinkRemove)(struct T_(Link) *const this,
	struct T_(LinkNode) *const node) {
	if(!this || !node) return;
	_T_(remove)(this, node);
}

/** Appends the elements of {from} onto {this}. If {this} is null, then it
 removes elements.
 @allow */
static void T_(LinkTake)(struct T_(Link) *const this,
	struct T_(Link) *const from) {
	if(!from || from == this) return;
	if(!this) { _T_(clear)(from); return; }
#ifdef LINK_OPENMP /* <-- omp */
#pragma omp parallel sections
#endif /* omp --> */
	{
#ifdef LINK_A_NAME /* <-- a */
#ifdef LINK_OPENMP /* <-- omp */
#pragma omp section
#endif /* omp --> */
		_T_LA_(link, cat)(this, from);
#endif /* a --> */
#ifdef LINK_B_NAME /* <-- b */
#ifdef LINK_OPENMP /* <-- omp */
#pragma omp section
#endif /* omp --> */
		_T_LB_(link, cat)(this, from);
#endif /* b --> */
#ifdef LINK_C_NAME /* <-- c */
#ifdef LINK_OPENMP /* <-- omp */
#pragma omp section
#endif /* omp --> */
		_T_LC_(link, cat)(this, from);
#endif /* c --> */
#ifdef LINK_D_NAME /* <-- d */
#ifdef LINK_OPENMP /* <-- omp */
#pragma omp section
#endif /* omp --> */
		_T_LD_(link, cat)(this, from);
#endif /* d --> */
	}
}

/** Merges the elements of {from} into {this} in (local) order,
 {O(this.n + from.n)}; concatenates all lists that don't have a
 {LINK_[A-D]_COMPARATOR}, {O(1)}. If {this} is null, then it removes
 elements.
 @allow */
static void T_(LinkMerge)(struct T_(Link) *const this,
	struct T_(Link) *const from) {
	if(!from || from == this) return;
	if(!this) { _T_(clear)(from); return; }
#ifdef LINK_OPENMP /* <-- omp */
#pragma omp parallel sections
#endif /* omp --> */
	{
#ifdef LINK_A_NAME /* <-- a */
#ifdef LINK_OPENMP /* <-- omp */
#pragma omp section
#endif /* omp --> */
#ifdef LINK_A_COMPARATOR /* <-- comp */
		_T_LA_(link, merge)(this, from);
#else /* comp --><-- !comp */
		_T_LA_(link, cat)(this, from);
#endif /* !comp --> */
#endif /* a --> */
#ifdef LINK_B_NAME /* <-- b */
#ifdef LINK_OPENMP /* <-- omp */
#pragma omp section
#endif /* omp --> */
#ifdef LINK_B_COMPARATOR /* <-- comp */
		_T_LB_(link, merge)(this, from);
#else /* comp --><-- !comp */
		_T_LB_(link, cat)(this, from);
#endif /* !comp --> */
#endif /* b --> */
#ifdef LINK_C_NAME /* <-- c */
#ifdef LINK_OPENMP /* <-- omp */
#pragma omp section
#endif /* omp --> */
#ifdef LINK_C_COMPARATOR /* <-- comp */
		_T_LC_(link, merge)(this, from);
#else /* comp --><-- !comp */
		_T_LC_(link, cat)(this, from);
#endif /* !comp --> */
#endif /* c --> */
#ifdef LINK_D_NAME /* <-- d */
#ifdef LINK_OPENMP /* <-- omp */
#pragma omp section
#endif /* omp --> */
#ifdef LINK_D_COMPARATOR /* <-- comp */
		_T_LD_(link, merge)(this, from);
#else /* comp --><-- !comp */
		_T_LD_(link, cat)(this, from);
#endif /* !comp --> */
#endif /* d --> */
	}
}

/** Sorts all by greedy natural insertion-merge sort. Like doing
 \see{<T>Link<L>Sort} for all lists in link with comparators. Designed to be
 an {O(n log n)} sort that is adaptive and stable, it's not as good at sorting
 random data as QuickSort.
 @allow */
static void T_(LinkSort)(struct T_(Link) *const this) {
	if(!this) return;
#ifdef LINK_OPENMP /* <-- omp */
	#pragma omp parallel sections
#endif /* omp --> */
	{
#ifdef LINK_A_NAME /* <-- a */
#ifdef LINK_OPENMP /* <-- omp */
		#pragma omp section
#endif /* omp --> */
		_T_LA_(natural, sort)(this);
#endif /* a --> */
#ifdef LINK_B_NAME /* <-- b */
#ifdef LINK_OPENMP /* <-- omp */
		#pragma omp section
#endif /* omp --> */
		_T_LB_(natural, sort)(this);
#endif /* b --> */
#ifdef LINK_C_NAME /* <-- c */
#ifdef LINK_OPENMP /* <-- omp */
		#pragma omp section
#endif /* omp --> */
		_T_LC_(natural, sort)(this);
#endif /* c --> */
#ifdef LINK_D_NAME /* <-- d */
#ifdef LINK_OPENMP /* <-- omp */
		#pragma omp section
#endif /* omp --> */
		_T_LD_(natural, sort)(this);
#endif /* d --> */
	}
}

/** Sets the user-defined {param} of {this}.
 @allow */
static void T_(LinkSetParam)(struct T_(Link) *const this,
	void *const param) {
	if(!this) return;
	this->param = param;
}

/** Use when one {<T>LinkNode} of {this} has switched places in memory from
 {old} to {new}. If {this}, {old}, or {new} is null, doesn't do anything.
 {O(1)}.
 @allow */
static void T_(LinkMove)(struct T_(Link) *const this,
	const struct T_(LinkNode) *const old, struct T_(LinkNode) *const new) {
	if(!this || !old || !new) return;
#ifdef LINK_OPENMP /* <-- omp */
#pragma omp parallel sections
#endif /* omp --> */
	{
#ifdef LINK_A_NAME /* <-- a */
#ifdef LINK_OPENMP /* <-- omp */
#pragma omp section
#endif /* omp --> */
		_T_LA_(link, move)(this, old, new);
#endif /* a --> */
#ifdef LINK_B_NAME /* <-- b */
#ifdef LINK_OPENMP /* <-- omp */
#pragma omp section
#endif /* omp --> */
		_T_LB_(link, move)(this, old, new);
#endif /* b --> */
#ifdef LINK_C_NAME /* <-- c */
#ifdef LINK_OPENMP /* <-- omp */
#pragma omp section
#endif /* omp --> */
		_T_LC_(link, move)(this, old, new);
#endif /* c --> */
#ifdef LINK_D_NAME /* <-- d */
#ifdef LINK_OPENMP /* <-- omp */
#pragma omp section
#endif /* omp --> */
		_T_LD_(link, move)(this, old, new);
#endif /* d --> */
	}
}

/** Use when {this} contains elements from an array of/containing {<T>LinkNode}
 in memory that switched from {old} to {new} with byte-size {byte_size}. If
 {this}, {old}, or {new} is null, doesn't do anything. For example, one must
 call this on a {<T>Link} that is (partially) backed with an array that has
 changed places due to a {realloc}. {O(n)}.

 Relies on not-strictly-defined behaviour because pointers are not necessarily
 contiguous. It should be fine.
 @allow */
static void T_(LinkBlockMove)(struct T_(Link) *const this,
	const void *const old, const size_t byte_size, void *const new) {
	const char *const cold = old;
	const char *const cold_end = cold + byte_size;
	const ptrdiff_t delta = (char *)new - cold;
	if(!this || !old || !byte_size || !new || old == new) return;
#ifdef LINK_OPENMP /* <-- omp */
#pragma omp parallel sections
#endif /* omp --> */
	{
#ifdef LINK_A_NAME /* <-- a */
#ifdef LINK_OPENMP /* <-- omp */
#pragma omp section
#endif /* omp --> */
		_T_LA_(block, move)(this, cold, cold_end, delta);
#endif /* a --> */
#ifdef LINK_B_NAME /* <-- b */
#ifdef LINK_OPENMP /* <-- omp */
#pragma omp section
#endif /* omp --> */
		_T_LB_(block, move)(this, cold, cold_end, delta);
#endif /* b --> */
#ifdef LINK_C_NAME /* <-- c */
#ifdef LINK_OPENMP /* <-- omp */
#pragma omp section
#endif /* omp --> */
		_T_LC_(block, move)(this, cold, cold_end, delta);
#endif /* c --> */
#ifdef LINK_D_NAME /* <-- d */
#ifdef LINK_OPENMP /* <-- omp */
#pragma omp section
#endif /* omp --> */
		_T_LD_(block, move)(this, cold, cold_end, delta);
#endif /* d --> */
	}
}

#ifdef LINK_TEST /* <-- test */
#include "../test/TestLink.h" /* need this file if one is going to run tests */
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
#ifdef LINK_DYNAMIC_STORAGE
#undef LINK_DYNAMIC_STORAGE
#endif
#ifdef LINK_OPENMP
#undef LINK_OPENMP
#endif
#ifdef LINK_TEST
#undef LINK_TEST
#endif
#ifdef _LINK_SORT_INTERNALS
#undef _LINK_SORT_INTERNALS /* each List type has their own */
#endif





#else /* !_LINK_NAME --><-- _LINK_NAME

 Internally #included.

 @param _LIST_NAME: A unique name; required;
 @param _LIST_COMPARATOR: an optional comparator. */





/* After this block, the preprocessor replaces T_M_(X, Y) with
 LINK_NAMEX_LINK_NAMEY, _T_M_(X, Y) with _LINK_NAME_X__LINK_NAME_Y */
#ifdef T_L_
#undef T_L_
#endif
#ifdef _T_L_
#undef _T_L_
#endif
#ifdef L_
#undef L_
#endif
#define T_L_(thing1, thing2) CAT(CAT(LINK_NAME, thing1),CAT(_LINK_NAME, thing2))
#define _T_L_(thing1, thing2) CAT(PCAT(LINK_NAME, thing1),\
	PCAT(_LINK_NAME, thing2))
#define L_(thing) PCAT(_LINK_NAME, thing)



/** Private: add to {this.last} in {<L>}. */
static void _T_L_(link, add)(struct T_(Link) *const this,
	struct T_(LinkNode) *const node) {
	assert(this);
	assert(node);
	node->L_(prev) = this->L_(last);
	node->L_(next) = 0;
	if(this->L_(last)) {
		this->L_(last)->L_(next) = node;
	} else {
		assert(!this->L_(first));
		this->L_(first) = node;
	}
	this->L_(last) = node;
}

/** Private: link remove in {<L>}. */
static void _T_L_(link, remove)(struct T_(Link) *const this,
	struct T_(LinkNode) *const node) {
	assert(this);
	assert(node);
	if(node->L_(prev)) {
		node->L_(prev)->L_(next) = node->L_(next);
	} else {
		assert(this->L_(first) == node);
		this->L_(first) = node->L_(next);
	}
	if(node->L_(next)) {
		node->L_(next)->L_(prev) = node->L_(prev);
	} else {
		assert(this->L_(last) == node);
		this->L_(last) = node->L_(prev);
	}
	node->L_(prev) = 0;
	node->L_(next) = 0;
}

/** Private: cats all {from} to the tail of {this}; {from} will be empty after.
 {O(1)}. */
static void _T_L_(link, cat)(struct T_(Link) *const this,
	struct T_(Link) *const from) {
	assert(this);
	assert(from);
	assert(!from->L_(first) == !from->L_(last));
	if(!from->L_(first)) return; /* there is nothing in {from} */
	if(!this->L_(first)) { /* there is nothing in {this} */
		this->L_(first) = from->L_(first);
	} else { /* there is something in both */
		this->L_(last)->L_(next) = from->L_(first);
	}
	this->L_(last) = from->L_(last);
	from->L_(first) = from->L_(last) = 0;
}

#ifdef _LINK_COMPARATOR /* <-- comp */

/* make sure that LINK_[A-D]_COMPARATOR is a function implementing
 {<T>Comparator}. */
static const T_(Comparator) _T_L_(data, cmp) = (_LINK_COMPARATOR);

/** Private: merges {this} into {recipient}; on equal elements, places {this}
 first. {O(n + m)}. */
static void _T_L_(link, merge)(struct T_(Link) *const this,
	struct T_(Link) *const from) {
	struct T_(LinkNode) *tnode, *fnode;
	assert(this);
	assert(from);
	tnode = this->L_(first);
	while((fnode = from->L_(first))) {
		while(tnode && _T_L_(data, cmp)(&fnode->data, &tnode->data) > 0)
			tnode = tnode->L_(next);
		if(!tnode) break; /* run past the end of {this} */
		/* shift(from); {fnode} goes before {tnode} */
		_T_L_(link, remove)(from, fnode);
		fnode->L_(next) = tnode;
		if((fnode->L_(prev) = tnode->L_(prev))) {
			tnode->L_(prev)->L_(next) = fnode;
		} else {
			/* this is the first, now */
			assert(this->L_(first) == tnode);
			this->L_(first) = fnode;
		}
	}
	_T_L_(link, cat)(this, from);
}

#endif /* comp --> */

/** Private: {old} is not de-referenced, but {new} is. */
static void _T_L_(link, move)(struct T_(Link) *const this,
	const struct T_(LinkNode) *const old, struct T_(LinkNode) *const new) {
	assert(this);
	assert(old);
	assert(new);
	if(new->L_(prev)) {
		assert(new->L_(prev)->L_(next) == old);
		new->L_(prev)->L_(next) = new;
	} else {
		assert(this->L_(first) == old);
		this->L_(first) = new;
	}
	if(new->L_(next)) {
		assert(new->L_(next)->L_(prev) == old);
		new->L_(next)->L_(prev) = new;
	} else {
		assert(this->L_(last) == old);
		this->L_(last) = new;
	}
}

static char _T_L_(get, list)(const struct T_(LinkNode) *const node);

/* {ptr \in [begin, end) -> ptr += delta}. */
static void _T_L_(block, move_one)(struct T_(LinkNode) **const node_ptr,
	const void *const begin, const void *const end, const ptrdiff_t delta) {
	const void *const ptr = *node_ptr;
	if(ptr < begin) return;
	if(ptr >= end) return;
	*(char **)node_ptr += delta;
}

/** Private: when {realloc} changes your pointers. I've tried to keep undefined
 behaviour to a minimum.
 @param [begin, end): Where the pointers have changed.
 @param delta: the offset they have moved to in memory bytes. */
static void _T_L_(block, move)(struct T_(Link) *const this,
	const void *const begin, const void *const end, const ptrdiff_t delta) {
	char a[9];
	struct T_(LinkNode) *node;
	assert(this);
	assert(begin);
	assert(begin < end);
	assert(delta);
	assert(!this->L_(first) == !this->L_(last));
	/* empty -- done */
	if(!this->L_(first)) return;
	/* first and last pointer of {<T>List} */
	printf(QUOTE(_LINK_NAME) " first/last (%c, %c)", _T_L_(get, list)(this->L_(first)), _T_L_(get, list)(this->L_(last)));
	_T_L_(block, move_one)(&this->L_(first), begin, end, delta);
	_T_L_(block, move_one)(&this->L_(last),  begin, end, delta);
	printf(" -> (%c, %c)\n", _T_L_(get, list)(this->L_(first)), _T_L_(get, list)(this->L_(last)));
	/* all the others' */
	for(node = this->L_(first); node; node = node->L_(next)) {
		_T_(to_string)(&node->data, &a);
		printf("node %s, (%c->%c<-%c)", a, _T_L_(get, list)(node->L_(prev)), _T_L_(get, list)(node), _T_L_(get, list)(node->L_(next)));
		_T_L_(block, move_one)(&node->L_(prev), begin, end, delta);
		_T_L_(block, move_one)(&node->L_(next), begin, end, delta);
		printf(" -> (%c->%c<-%c)\n", _T_L_(get, list)(node->L_(prev)), _T_L_(get, list)(node), _T_L_(get, list)(node->L_(next)));
	}
}


/** @return The next element after {this} in {<L>}. When {this} is the last
 element, returns null.
 @allow */
static struct T_(LinkNode) *T_L_(LinkNode, GetNext)(
	struct T_(LinkNode) *const this) {
	if(!this) return 0;
	return this->L_(next);
}

/** @return The previous element before {this} in {<L>}. When {this} is the
 first item, returns null.
 @allow */
static struct T_(LinkNode) *T_L_(LinkNode, GetPrevious)(
	struct T_(LinkNode) *const this) {
	if(!this) return 0;
	return this->L_(prev);
}

/** @return A pointer to the first element of {this}.
 @allow */
static struct T_(LinkNode) *T_L_(Link, GetFirst)(struct T_(Link) *const this) {
	if(!this) return 0;
	return this->L_(first);
}

/** @return A pointer to the last element of {this}.
 @allow */
static struct T_(LinkNode) *T_L_(Link, GetLast)(struct T_(Link) *const this) {
	if(!this) return 0;
	return this->L_(last);
}

#ifdef _LINK_COMPARATOR /* <-- comp */

/* Merge sort internals only once per translation unit. */

#ifndef _LINK_SORT_INTERNALS /* <!-- sort internals */
#define _LINK_SORT_INTERNALS

/* A run is a temporary sequence of values in the array that is weakly
 increasing. */
struct _T_(Run) {
	struct T_(LinkNode) *head, *tail;
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

/* fixme: re-implement LINK_DYNAMIC_STORAGE */
static struct _T_(Runs) _T_L_(runs, elem);

/** Inserts the first element from the larger of two sorted runs, then merges
 the rest. \cite{Peters2002Timsort}, via \cite{McIlroy1993Optimistic}, does
 long merges by galloping, but we don't have random access to the data. In
 practice, this is {2%} slower on randomly distributed keys when the
 linked-list size is over {500 000}; randomly distributed keys have high
 insertion times that to well in standard merging. However, it's (potentially
 much) faster when the keys have structure: observed, {[-2%, 500%]}. Assumes
 array contains at least 2 elements and there are at least two runs. */
static void _T_L_(natural, merge)(struct _T_(Runs) *const r) {
	struct _T_(Run) *const run_a = r->run + r->run_no - 2;
	struct _T_(Run) *const run_b = run_a + 1;
	struct T_(LinkNode) *a = run_a->tail, *b = run_b->head, *chosen;

	/* fixme: we are doing one-to-many compares in some cases? */

	if(run_a->size <= run_b->size) {
		struct T_(LinkNode) *prev_chosen;

		/* run a is smaller: downwards insert b.head followed by upwards
		 merge */

		/* insert the first element of b downwards into a */
		for( ; ; ) {
			if(_T_L_(data, cmp)(&a->data, &b->data) <= 0) {
				chosen = a;
				a = a->L_(next);
				break;
			}
			if(!a->L_(prev)) {
				run_a->head = run_b->head;
				chosen = b;
				b = b->L_(next);
				break;
			}
			a = a->L_(prev);
		}

		/* merge upwards, while the lists are interleaved */
		while(chosen->L_(next)) {
			prev_chosen = chosen;
			if(_T_L_(data, cmp)(&a->data, &b->data) > 0) {
				chosen = b;
				b = b->L_(next);
			} else {
				chosen = a;
				a = a->L_(next);
			}
			prev_chosen->L_(next) = chosen;
			chosen->L_(prev) = prev_chosen;
		}

		/* splice the one list left */
		if(!a) {
			b->L_(prev) = chosen;
			chosen->L_(next) = b;
			run_a->tail = run_b->tail;
		} else {
			a->L_(prev) = chosen;
			chosen->L_(next) = a;
		}

	} else {
		struct T_(LinkNode) *next_chosen;
		int is_a_tail = 0;

		/* run b is smaller; upwards insert followed by downwards merge */

		/* insert the last element of a upwards into b */
		for( ; ; ) {
			if(_T_L_(data, cmp)(&a->data, &b->data) <= 0) {
				chosen = b;
				b = b->L_(prev);
				break;
			}
			/* here, a > b */
			if(!b->L_(next)) {
				is_a_tail = -1;
				chosen = a;
				a = a->L_(prev);
				break;
			}
			b = b->L_(next);
		}
		if(!is_a_tail) run_a->tail = run_b->tail;

		/* merge downwards, while the lists are interleaved */
		while(chosen->L_(prev)) {
			next_chosen = chosen;
			if(_T_L_(data, cmp)(&a->data, &b->data) > 0) {
				chosen = a;
				a = a->L_(prev);
			} else {
				chosen = b;
				b = b->L_(prev);
			}
			next_chosen->L_(prev) = chosen;
			chosen->L_(next) = next_chosen;
		}

		/* splice the one list left */
		if(!a) {
			b->L_(next) = chosen;
			chosen->L_(prev) = b;
			run_a->head = run_b->head;
		} else {
			a->L_(next) = chosen;
			chosen->L_(prev) = a;
		}

	}

	run_a->size += run_b->size;
	r->run_no--;
}

/** It's kind of experimental. It hasn't been optimised; I think it does
 useless compares and I question whether a strict Pascal's triangle-shape
 would be optimum, or whether a long run should be put off merging until
 short runs have finished; it is quite simple as it is. */
static void _T_L_(natural, sort)(struct T_(Link) *const this) {
	/* new_run is an index into link_runs, a temporary sorting structure;
	 head is first smallest, tail is last largest */
	struct _T_(Run) *new_run;
	/* part of the state machine for classifying points wrt their neighbours */
	enum { UNSURE, INCREASING, DECREASING } mono;
	/* the data that we are sorting */
	struct T_(LinkNode) *a, *b, *c, *first_iso_a;
	/* run_count is different from link_runs.run_no in that it only increases;
	 only used for calculating the path up the tree */
	size_t run_count, rc;
	/* the value of the comparison */
	int comp;

	/* ensure we have an 'a' */
	if(!(a = this->L_(first))) return;

	/* reset the state machine and output to just 'a' in the first run */
	mono = UNSURE;
	_T_L_(runs, elem).run_no = 1;
	new_run = _T_L_(runs,elem).run + 0, run_count = 1;
	new_run->size = 1;
	first_iso_a = new_run->head = new_run->tail = a;

	for(b = a->L_(next); b; a = b, b = c) {

		/* b.next can be modified, and we always want the iteration original */
		c = b->L_(next);

		comp = _T_L_(data, cmp)(&a->data, &b->data);

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
				b->L_(next) = first_iso_a;
				first_iso_a->L_(prev) = b;
				new_run->head = first_iso_a = b;
				new_run->size++;
				continue;
			}
			new_run->tail = a; /* terminating an increasing sequence */
		} else { /* a == b */
			if(mono == DECREASING) { /* extend */
				struct T_(LinkNode) *const a_next = a->L_(next);
				b->L_(next) = a_next;
				a_next->L_(prev) = b;
				a->L_(next) = b;
				b->L_(prev) = a;
			} else { /* weakly increasing */
				new_run->tail = b;
			}
			new_run->size++;
			continue;
		}
		/* head and tail don't necessarily correspond to the first and last */
		new_run->head->L_(prev) = new_run->tail->L_(next) = 0;

		/* greedy merge: keeps space to O(log n) instead of O(n) */
		for(rc = run_count; !(rc & 1) && _T_L_(runs,elem).run_no >= 2; rc >>= 1)
			_T_L_(natural, merge)(&_T_L_(runs,elem));
		/* reset the state machine and output to just 'b' at the next run */
		mono = UNSURE;
		new_run = _T_L_(runs,elem).run + _T_L_(runs,elem).run_no++, run_count++;
		new_run->size = 1;
		new_run->head = new_run->tail = first_iso_a = b;
	}

	/* terminating the last increasing sequence */
	if(mono == INCREASING) new_run->tail = a;
		new_run->tail->L_(next) = new_run->head->L_(prev) = 0;

	/* clean up the rest; when only one run, propagate link_runs[0] to head */
	while(_T_L_(runs, elem).run_no > 1)
		_T_L_(natural, merge)(&_T_L_(runs, elem));
	this->L_(first) = _T_L_(runs, elem).run[0].head;
	this->L_(last)  = _T_L_(runs, elem).run[0].tail;
}

/** Sorts {<L>}, but leaves the other lists in {<T>} alone.
 @allow */
static void T_L_(Link, Sort)(struct T_(Link) *const this) {
	if(!this) return;
	_T_L_(natural, sort)(this);
}

/** Compares two linked-lists as sequences in the order specified by {<L>}.
 @return The first comparator that is not equal to zero, or 0 if they are
 equal; if one list is a sub-list starting at the same point of the other,
 returns -1 or 1. Null pointers count as lists that are before every other
 list; two null pointers are considered equal.
 @implements <<T>Link>Comparator
 @allow */
static int T_L_(Link, Compare)(const struct T_(Link) *const this,
	const struct T_(Link) *const that) {
	struct T_(LinkNode) *a, *b;
	int diff;
	/* null counts as -\infty */
	if(!this) {
		return that ? -1 : 0;
	} else if(!that) {
		return 1;
	}
	/* compare element by element */
	for(a = this->L_(first), b = that->L_(first); ;
		a = a->L_(next), b = b->L_(next)) {
		if(!a) {
			return b ? -1 : 0;
		} else if(!b) {
			return 1;
		} else if((diff = _T_L_(data, cmp)(&a->data, &b->data))) {
			return diff;
		}
	}
}

/** Private: {this <- a \mask b}. Prefers {a} to {b} when equal. */
static void _T_L_(boolean, seq)(struct T_(Link) *const this,
	struct T_(Link) *const a, struct T_(Link) *const b,
	const enum LinkOperation mask) {
	struct T_(LinkNode) *ai = a ? a->L_(first) : 0, *bi = b ? b->L_(first) : 0,
		*t; /* iterator, temp */
	int comp; /* comparator */
	while(ai && bi) {
		comp = _T_L_(data, cmp)(&ai->data, &bi->data);
		if(comp < 0) {
			t = ai, ai = ai->L_(next);
			if(mask & LO_SUBTRACTION_AB) {
				_T_(remove)(a, t);
				if(this) _T_(add)(this, t);
			}
		} else if(comp > 0) {
			t = bi, bi = bi->L_(next);
			if(mask & LO_SUBTRACTION_BA) {
				_T_(remove)(b, t);
				if(this) _T_(add)(this, t);
			}
		} else {
			t = ai, ai = ai->L_(next), bi = bi->L_(next);
			if(mask & LO_INTERSECTION) {
				_T_(remove)(a, t);
				if(this) _T_(add)(this, t);
			}
		}
	}
	if(mask & LO_DEFAULT_A) {
		while(ai) {
			t = ai, ai = ai->L_(next);
			_T_(remove)(a, t);
			if(this) _T_(add)(this, t);
		}
	}
	if((mask & LO_DEFAULT_B)) {
		while(bi) {
			t = bi, bi = bi->L_(next);
			_T_(remove)(b, t);
			if(this) _T_(add)(this, t);
		}
	}
}

/** Appends {that} with {b} subtracted from {a} as a sequence in {<L>}. If
 {this} is null, then it removes elements.
 @allow */
static void T_L_(Link, TakeSubtraction)(struct T_(Link) *const this,
	struct T_(Link) *const a, struct T_(Link) *const b) {
	_T_L_(boolean, seq)(this, a, b, LO_SUBTRACTION_AB |LO_DEFAULT_A);
}

/** Appends {this} with the union of {a} and {b} as a sequence in {<L>}. Equal
 elements are moved from {a}. If {this} is null, then it removes elements.
 @allow */
static void T_L_(Link, TakeUnion)(struct T_(Link) *const this,
	struct T_(Link) *const a, struct T_(Link) *const b) {
	_T_L_(boolean, seq)(this, a, b, LO_SUBTRACTION_AB | LO_SUBTRACTION_BA
		| LO_INTERSECTION | LO_DEFAULT_A | LO_DEFAULT_B);
}

/** Appends {this} with the intersection of {a} and {b} as a sequence in {<L>}.
 Equal elements are moved from {a}. If {this} is null, then it removes elements.
 @allow */
static void T_L_(Link, TakeIntersection)(struct T_(Link) *const this,
	struct T_(Link) *const a, struct T_(Link) *const b) {
	_T_L_(boolean, seq)(this, a, b, LO_INTERSECTION);
}

/** Appends {this} with {a} exclusive-or {b} as a sequence in {<L>}. Equal
 elements are moved from {a}. If {this} is null, then it removes elements.
 @allow */
static void T_L_(Link, TakeXor)(struct T_(Link) *const this,
	struct T_(Link) *const a, struct T_(Link) *const b) {
	_T_L_(boolean, seq)(this, a, b, LO_SUBTRACTION_AB | LO_SUBTRACTION_BA
		| LO_DEFAULT_A | LO_DEFAULT_B);
}

#endif /* comp --> */

/** Appends {this} with {from} if {predicate} is null or true in the order
 specified by {<L>}. If {this} is null, then it removes elements.
 @allow */
static void T_L_(Link, TakeIf)(struct T_(Link) *const this,
	struct T_(Link) *const from, const T_(Predicate) predicate) {
	struct T_(LinkNode) *cursor, *next_cursor;
	if(!from || from == this) return;
	for(cursor = from->L_(first); cursor; cursor = next_cursor) {
		next_cursor = cursor->L_(next);
		if(predicate && !predicate(&cursor->data, from->param)) continue;
		_T_(remove)(from, cursor);
		if(!this) continue;
		_T_(add)(this, cursor);
	}
}

/** Performs {action} for each element in the list in the order specified by
 {<L>}. For more flexibility, use \see{<T>List<L>ShortCircuit}, which takes a
 {<T>Predicate}, and return true.
 @allow */
static void T_L_(Link, ForEach)(struct T_(Link) *const this,
	const T_(Action) action) {
	struct T_(LinkNode) *cursor;
	if(!this || !action) return;
	for(cursor = this->L_(first); cursor; cursor = cursor->L_(next)) {
		action(&cursor->data);
	}
}

/** @return The first {<T>LinkNode} in the linked-list, ordered by {<L>}, that
 causes the {predicate} with {<T>} as argument to return false, or null if the
 {predicate} is true for every case.
 @allow */
static struct T_(LinkNode) *T_L_(Link, ShortCircuit)(
	struct T_(Link) *const this, const T_(Predicate) predicate) {
	struct T_(LinkNode) *cursor;
	if(!this || !predicate) return 0;
	for(cursor = this->L_(first); cursor; cursor = cursor->L_(next)) {
		if(!predicate(&cursor->data, this->param)) return cursor;
	}
	return 0;
}

#ifdef LINK_TO_STRING /* <-- print */

#ifndef _LINK_PRINT_THINGS /* <-- once inside translation unit */
#define _LINK_PRINT_THINGS

static const char *const _link_start     = "[ ";
static const char *const _link_end       = " ]";
static const char *const _link_alter_end = "...]";
static const char *const _link_sep       = ", ";
static const char *const _link_star      = "*";
static const char *const _link_null      = "null";

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
	/* fixme: snprintf is C99 */
	took = snprintf(cat->cursor, cat->left, "%s", append);
	if(took < 0)  { cat->is_truncated = -1; return; } /*implementation defined*/
	if(took == 0) { return; }
	if((lu_took=took)>=cat->left) {cat->is_truncated=-1,lu_took=cat->left-1;}
	cat->cursor += lu_took, cat->left -= lu_took;
}
#endif /* once --> */

/** Prints the linked-list in a static buffer; one can print 4 things at once
 before it overwrites. One must set {LINK_TO_STRING} to a function implementing
 {<T>ToString} to get this functionality.
 @allow */
static char *T_L_(Link, ToString)(const struct T_(Link) *const this) {
	static char buffer[4][256];
	static int buffer_i;
	struct _ListSuperCat cat;
	char scratch[9];
	struct T_(LinkNode) *link;
	assert(strlen(_link_alter_end) >= strlen(_link_end));
	assert(sizeof buffer > strlen(_link_alter_end));
	_list_super_cat_init(&cat, buffer[buffer_i],
		sizeof *buffer / sizeof **buffer - strlen(_link_alter_end));
	buffer_i++, buffer_i &= 3;
	if(!this) {
		_list_super_cat(&cat, _link_null);
		return cat.print;
	}
	_list_super_cat(&cat, _link_start);
	for(link = this->L_(first); link; link = link->L_(next)) {
		if(link != this->L_(first)) _list_super_cat(&cat, _link_sep);
		_T_(to_string)(&link->data, &scratch), scratch[8] = '\0';
		_list_super_cat(&cat, scratch);
		if(cat.is_truncated) break;
	}
	sprintf(cat.cursor, "%s", cat.is_truncated ? _link_alter_end : _link_end);
	return cat.print; /* static buffer */
}

#endif /* print --> */



/* un-define stuff for the next */
#undef _LINK_NAME
#ifdef _LINK_COMPARATOR /* <-- comp */
#undef _LINK_COMPARATOR
#endif /* comp --> */

#endif /* _LINK_NAME --> */
