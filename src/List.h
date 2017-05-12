/** 2017 Neil Edelman, distributed under the terms of the MIT License;
 see readme.txt, or \url{ https://opensource.org/licenses/MIT }.

 {<T>List} stores doubly-linked-list(s) of {<T>ListNode}, of which data of
 type, {<T>}, must be set using {LIST_TYPE}. Supports one to four different
 linked-list orders in the same type, {[A, D]}, set using {LIST_[A-D]_NAME}.
 The preprocessor macros are all undefined at the end of the file for
 convenience when including multiple {List} types in the same file.

 The {<T>ListNode} storage is the responsibility of the caller. Specifically,
 it does not have to be contiguous, and it can be nestled in multiple, possibly
 different, structures. You can move (part) of the memory that is in an active
 {List} under some conditions, and still keep the integrity of the linked-list,
 by \see{<T>ListMigrate} or \see{<T>ListMigrateBlock}.

 @param LIST_NAME, LIST_TYPE
 The name that literally becomes {<T>}, and a valid type associated therewith;
 should be conformant to naming and to the maximum available length of
 identifiers. Must each be present before including.

 @param LIST_[A-D]_NAME, LIST_[A-D]_COMPARATOR
 Each {LIST_[A-D]_NAME} literally becomes {<L>}. You can define an optional
 comparator, an equivalence relation function implementing {<T>Comparator}. For
 speed, it should be an inlined {static} function, if possible.

 @param LIST_TO_STRING
 Optional print function implementing {<T>ToString}; makes available
 \see{<T>List<L>ToString}.

 @param LIST_DYNAMIC_STORAGE
 This allocates {O(log n)} space needed for merge sort on the stack every time
 the List is sorted, instead of statically. This allows using the exact same
 sort on different data concurrently without crashing, but it consumes more
 resources.

 @param LIST_OPENMP
 Tries to parallelise using {OpenMP}, \url{ http://www.openmp.org/ }.

 @param LIST_TEST
 Unit testing framework using {<T>ListTest}, included in a separate header,
 {../test/ListTest.h}. Must be defined equal to a (random) filler, satisfying
 {<T>Action}. If {NDEBUG} is not defined, turns on {assert} private function
 integrity testing. Requires {LIST_TO_STRING}.

 @title		List.h
 @author	Neil
 @std		C89/90
 @version	1.0; 2017-05
 @since		1.0; 2017-05 separated from List.h
 @fixme {GCC}: {#pragma GCC diagnostic ignored "-Wconversion"}; libc 4.2
 {assert} bug on {LIST_TEST}.
 @fixme {MSVC}: {#pragma warning(disable: x)} where {x} is: 4464 contains '..'
 uhm, thanks?; 4706 not {Java}; 4710, 4711 inlined info; 4820 padding info;
 4996 not {C++11}.
 @fixme {clang}: {#pragma clang diagnostic ignored "-Wx"} where {x} is:
 {padded}; {documentation}; {documentation-unknown-command} it's not quite
 {clang-tags}; 3.8 {disabled-macro-expansion} on {LIST_TEST}. */

/* Tested with:
 gcc version 4.2.1 (Apple Inc. build 5666) (dot 3)
 Apple clang version 1.7 (tags/Apple/clang-77) (based on LLVM 2.9svn)
 gcc version 4.9.2 (Debian 4.9.2-10)
 Microsoft Visual Studio Enterprise 2015 Version 14.0.25424.00 Update 3
 Borland 10.1 Embarcadero C++ 7.20 for Win32
 MinGW gcc version 4.9.3 (GCC) Win32
 gcc version 5.4.0 20160609 (Ubuntu 5.4.0-6ubuntu1~16.04.4)
 clang version 3.8.0-2ubuntu4 (tags/RELEASE_380/final) */



/* original #include in the user's C file, and not from calling recursively */
#if !defined(LIST_L_NAME) /* <-- !LIST_L_NAME */



#include <stddef.h>	/* ptrdiff_t */
#include <assert.h>	/* assert */
#ifdef LIST_TO_STRING /* <-- print */
#include <stdio.h>	/* sprintf */
#endif /* print --> */

/* unused macro */
#ifdef UNUSED
#undef UNUSED
#endif
#define UNUSED(a) ((void)(a))



/* check defines; {[A, D]} is just arbitrary; more could be added */
#ifndef LIST_NAME
#error List generic LIST_NAME undefined.
#endif
#ifndef LIST_TYPE
#error List generic LIST_TYPE undefined.
#endif
#if !defined(LIST_A_NAME) && !defined(LIST_B_NAME) \
	&& !defined(LIST_C_NAME) && !defined(LIST_D_NAME)
#error List: must have at least one of LIST_[A-D]_NAME.
#endif
#if defined(LIST_A_COMPARATOR) && !defined(LIST_A_NAME)
#error List: LIST_A_COMPARATOR requires LIST_A_NAME.
#endif
#if defined(LIST_B_COMPARATOR) && !defined(LIST_B_NAME)
#error List: LIST_B_COMPARATOR requires LIST_B_NAME.
#endif
#if defined(LIST_C_COMPARATOR) && !defined(LIST_C_NAME)
#error List: LIST_C_COMPARATOR requires LIST_C_NAME.
#endif
#if defined(LIST_D_COMPARATOR) && !defined(LIST_D_NAME)
#error List: LIST_D_COMPARATOR requires LIST_D_NAME.
#endif
#if defined(LIST_TEST) && !defined(LIST_TO_STRING)
#error LIST_TEST requires LIST_TO_STRING.
#endif
#ifndef LIST_TEST
#define NDEBUG
#endif
#if defined(LIST_A_COMPARATOR) || defined(LIST_B_COMPARATOR) \
	|| defined(LIST_C_COMPARATOR) || defined(LIST_D_COMPARATOR)
#define LIST_SOME_COMPARATOR
#endif





/* After this block, the preprocessor replaces T with LIST_TYPE, T_(X) with
 LIST_NAMEX, PRIVATE_T_(X) with LIST_L_NAME_X, and T_NAME with the string
 version. http://stackoverflow.com/questions/16522341/pseudo-generics-in-c */
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
#ifdef PRIVATE_T_
#undef PRIVATE_T_
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
#define PCAT_(x, y) x ## _ ## y
#define PCAT(x, y) PCAT_(x, y)
#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)
#define T_(thing) CAT(LIST_NAME, thing)
#define PRIVATE_T_(thing) PCAT(list, PCAT(LIST_NAME, thing))
#define T_NAME QUOTE(LIST_NAME)

/* Troubles with this line? check to ensure that LIST_TYPE is a valid type,
 whose definition is placed above {#include "List.h"}. */
typedef LIST_TYPE PRIVATE_T_(Type);
#define T PRIVATE_T_(Type)

/* [A, D] */
#ifdef LA_
#undef LA_
#undef T_LA_
#undef PRIVATE_T_LA_
#endif
#ifdef LB_
#undef LB_
#undef T_LB_
#undef PRIVATE_T_LB_
#endif
#ifdef LC_
#undef LC_
#undef T_LC_
#undef PRIVATE_T_LC_
#endif
#ifdef LD_
#undef LD_
#undef T_LD_
#undef PRIVATE_T_LD_
#endif
#ifdef LIST_A_NAME
#define LA_(thing) PCAT(LIST_A_NAME, thing) /* data, exclusively */
#define T_LA_(thing1, thing2) CAT(CAT(LIST_NAME, thing1), \
	CAT(LIST_A_NAME, thing2)) /* public fn's */
#define PRIVATE_T_LA_(thing1, thing2) PCAT(list, PCAT(PCAT(LIST_NAME, thing1), \
	PCAT(LIST_A_NAME, thing2))) /* private fn's */
#endif
#ifdef LIST_B_NAME
#define LB_(thing) PCAT(LIST_B_NAME, thing)
#define T_LB_(thing1, thing2) CAT(CAT(LIST_NAME, thing1), \
	CAT(LIST_B_NAME, thing2))
#define PRIVATE_T_LB_(thing1, thing2) PCAT(list, PCAT(PCAT(LIST_NAME, thing1), \
	PCAT(LIST_B_NAME, thing2)))
#endif
#ifdef LIST_C_NAME
#define LC_(thing) PCAT(LIST_C_NAME, thing)
#define T_LC_(thing1, thing2) CAT(CAT(LIST_NAME, thing1), \
	CAT(LIST_C_NAME, thing2))
#define PRIVATE_T_LC_(thing1, thing2) PCAT(list, PCAT(PCAT(LIST_NAME, thing1), \
	PCAT(LIST_C_NAME, thing2)))
#endif
#ifdef LIST_D_NAME
#define LD_(thing) PCAT(LIST_D_NAME, thing)
#define T_LD_(thing1, thing2) CAT(CAT(LIST_NAME, thing1), \
	CAT(LIST_D_NAME, thing2))
#define PRIVATE_T_LD_(thing1, thing2) PCAT(list, PCAT(PCAT(LIST_NAME, thing1), \
	PCAT(LIST_D_NAME, thing2)))
#endif



/* constants across multiple includes in the same translation unit */
#ifndef LIST_H /* <-- LIST_H */
#define LIST_H
/* combine_sets() operations bit-vector */
enum ListOperation {
	LO_SUBTRACTION_AB = 1,
	LO_SUBTRACTION_BA = 2,
	LO_A,
	LO_INTERSECTION   = 4,
	LO_B, LO_C, LO_D,
	LO_DEFAULT_A      = 8,
	LO_E, LO_F, LO_G, LO_H, LO_I, LO_J, LO_K,
	LO_DEFAULT_B      = 16,
	LO_L, LO_M, LO_N, LO_O, LO_P, LO_Q, LO_R, LO_S,
	LO_T, LO_U, LO_V, LO_W, LO_X, LO_Y, LO_Z
};
#endif /* LIST_H */



/** Operates by side-effects only. */
typedef void (*T_(Action))(T *const);

/** Passed {T} and {param}, (see \see{<T>ListSetParam},) returns (non-zero)
 true or (zero) false. */
typedef int  (*T_(Predicate))(T *const, void *const);

/** Compares two values and returns less then, equal to, or greater then
 zero. Should do so forming an equivalence relation with respect to {T}. */
typedef int  (*T_(Comparator))(const T *, const T *);

#ifdef LIST_TO_STRING
/** Responsible for turning {<T>} (the first argument) into a 12 {char}
 null-terminated output string (the second.) */
typedef void (*T_(ToString))(const T *const, char (*const)[12]);
/* Check that {LIST_TO_STRING} is a function implementing {<T>ToString}. */
static const T_(ToString) PRIVATE_T_(to_string) = (LIST_TO_STRING);
#endif



/** A single link in the linked-list derived from {<T>}. Storage of this
 structure is the responsibility of the caller. A {<T>ListNode} can be
 reinterpreted (cast) to {<T>} as a single element; that is, {<T>} shall be the
 first element of {<T>ListNode}.
 \${|    <T>ListNode *node;
 |    T *t;
 |    for(node = <T>List<L>GetFirst(list);
 |        node;
 |        node = <T>ListNode<L>GetNext(node)) {
 |        t = (T *)node;
 |    }} or
 \${|    static void for_all_fn(T *const t) {
 |        struct <T>ListNode *const node = (struct <T>ListNode *)t;
 |    }} */
struct T_(ListNode);
struct T_(ListNode) {
	T data; /* 1st so we can cast without the mess of {container_of} */
#ifdef LIST_A_NAME
	struct T_(ListNode) *LA_(prev), *LA_(next);
#endif
#ifdef LIST_B_NAME
	struct T_(ListNode) *LB_(prev), *LB_(next);
#endif
#ifdef LIST_C_NAME
	struct T_(ListNode) *LC_(prev), *LC_(next);
#endif
#ifdef LIST_D_NAME
	struct T_(ListNode) *LD_(prev), *LD_(next);
#endif
};

/** Serves as an a head for linked-list(s) of {<T>ListNode}. No initialisation
 is necessary when the variable is of {static} duration, otherwise use
 \see{<T>ListClear}. */
struct T_(List);
struct T_(List) {
#ifdef LIST_A_NAME
	struct T_(ListNode) *LA_(first), *LA_(last);
#endif
#ifdef LIST_B_NAME
	struct T_(ListNode) *LB_(first), *LB_(last);
#endif
#ifdef LIST_C_NAME
	struct T_(ListNode) *LC_(first), *LC_(last);
#endif
#ifdef LIST_D_NAME
	struct T_(ListNode) *LD_(first), *LD_(last);
#endif
	void *param;
};



/* Prototypes: needed for the next section, but undefined until later. */
static void PRIVATE_T_(add)(struct T_(List) *const this,
	struct T_(ListNode) *const node);
static void PRIVATE_T_(remove)(struct T_(List) *const this,
	struct T_(ListNode) *const node);

/* Note to future self: recursive includes. The {LIST_L_NAME} pre-processor flag
 controls this behaviour; we are currently in the {!LIST_L_NAME} section. These
 will get all the functions with {<L>} in them. */

#ifdef LIST_A_NAME /* <-- a */
#define LIST_L_NAME LIST_A_NAME
#ifdef LIST_A_COMPARATOR /* <-- comp */
#define LIST_L_COMPARATOR LIST_A_COMPARATOR
#endif /* comp --> */
#include "List.h"
#endif /* a --> */

#ifdef LIST_B_NAME /* <-- b */
#define LIST_L_NAME LIST_B_NAME
#ifdef LIST_B_COMPARATOR /* <-- comp */
#define LIST_L_COMPARATOR LIST_B_COMPARATOR
#endif /* comp --> */
#include "List.h"
#endif /* b --> */

#ifdef LIST_C_NAME /* <-- c */
#define LIST_L_NAME LIST_C_NAME
#ifdef LIST_C_COMPARATOR /* <-- comp */
#define LIST_L_COMPARATOR LIST_C_COMPARATOR
#endif /* comp --> */
#include "List.h"
#endif /* c --> */

#ifdef LIST_D_NAME /* <-- d */
#define LIST_L_NAME LIST_D_NAME
#ifdef LIST_D_COMPARATOR /* <-- comp */
#define LIST_L_COMPARATOR LIST_D_COMPARATOR
#endif /* comp --> */
#include "List.h"
#endif /* d --> */



/** Private: add to the end of the list. */
static void PRIVATE_T_(add)(struct T_(List) *const this,
	struct T_(ListNode) *const node) {
	assert(this);
	assert(node);
#ifdef LIST_A_NAME /* <-- a */
	PRIVATE_T_LA_(list, add)(this, node);
#endif /* a --> */
#ifdef LIST_B_NAME /* <-- b */
	PRIVATE_T_LB_(list, add)(this, node);
#endif /* b --> */
#ifdef LIST_C_NAME /* <-- c */
	PRIVATE_T_LC_(list, add)(this, node);
#endif /* c --> */
#ifdef LIST_D_NAME /* <-- d */
	PRIVATE_T_LD_(list, add)(this, node);
#endif /* d --> */
}

/** Private: remove from list. */
static void PRIVATE_T_(remove)(struct T_(List) *const this,
	struct T_(ListNode) *const node) {
	assert(this);
	assert(node);
#ifdef LIST_A_NAME /* <-- a */
	PRIVATE_T_LA_(list, remove)(this, node);
#endif /* a --> */
#ifdef LIST_B_NAME /* <-- b */
	PRIVATE_T_LB_(list, remove)(this, node);
#endif /* b --> */
#ifdef LIST_C_NAME /* <-- c */
	PRIVATE_T_LC_(list, remove)(this, node);
#endif /* c --> */
#ifdef LIST_D_NAME /* <-- d */
	PRIVATE_T_LD_(list, remove)(this, node);
#endif /* d --> */
}

/** Private: clear the list. */
static void PRIVATE_T_(clear)(struct T_(List) *const this) {
	assert(this);
#ifdef LIST_A_NAME
	this->LA_(first) = this->LA_(last) = 0;
#endif
#ifdef LIST_B_NAME
	this->LB_(first) = this->LB_(last) = 0;
#endif
#ifdef LIST_C_NAME
	this->LC_(first) = this->LC_(last) = 0;
#endif
#ifdef LIST_D_NAME
	this->LD_(first) = this->LD_(last) = 0;
#endif
}

/** Clears all values from {this}, thereby initialising the {<T>List}. If it
 contained a list, those values are free.
 @order \Theta(1)
 @allow */
static void T_(ListClear)(struct T_(List) *const this) {
	if(!this) return;
	PRIVATE_T_(clear)(this);
	this->param = 0;
}

/** Sets the contents of {node} to push it to {this}, thereby initialising the
 non-{<T>} parts of {<T>ListNode}. Does not do any checks on {node} and
 overwrites the data that was there. Specifically, it invokes undefined
 behaviour to one add {node} to more than one list without removing it each
 time. If either {this} or {node} is null, it does nothing.
 @order \Theta(1)
 @allow */
static void T_(ListAdd)(struct T_(List) *const this,
	struct T_(ListNode) *const node) {
	if(!this || !node) return;
	PRIVATE_T_(add)(this, node);
}

/** Removes {node} from the {this}. The {node} is now free to add to another
 list. Removing an element that was not added to {this} results in undefined
 behaviour. If either {this} or {node} is null, it does nothing.
 @order \Theta(1)
 @allow */
static void T_(ListRemove)(struct T_(List) *const this,
	struct T_(ListNode) *const node) {
	if(!this || !node) return;
	PRIVATE_T_(remove)(this, node);
}

/** Appends the elements of {from} onto {this}. If {this} is null, then it
 removes elements. Unlike the {<T>List<L>Take*}, where the elements are
 re-ordered based on {<L>}, (they would not be in-place, otherwise,) this
 function concatenates all the elements in each linked-list order.
 @order \Theta(1)
 @allow */
static void T_(ListTake)(struct T_(List) *const this,
	struct T_(List) *const from) {
	if(!from || from == this) return;
	if(!this) { PRIVATE_T_(clear)(from); return; }
#ifdef LIST_A_NAME /* <-- a */
	PRIVATE_T_LA_(list, cat)(this, from);
#endif /* a --> */
#ifdef LIST_B_NAME /* <-- b */
	PRIVATE_T_LB_(list, cat)(this, from);
#endif /* b --> */
#ifdef LIST_C_NAME /* <-- c */
	PRIVATE_T_LC_(list, cat)(this, from);
#endif /* c --> */
#ifdef LIST_D_NAME /* <-- d */
	PRIVATE_T_LD_(list, cat)(this, from);
#endif /* d --> */
}

#ifdef LIST_SOME_COMPARATOR /* <-- comp */
/** Merges the elements into {this} from {from} in (local) order; concatenates
 all lists that don't have a {LIST_[A-D]_COMPARATOR}. If {this} is null, then
 it removes elements.
 @order O({this}.n + {from}.n)
 @allow */
static void T_(ListMerge)(struct T_(List) *const this,
	struct T_(List) *const from) {
	if(!from || from == this) return;
	if(!this) { PRIVATE_T_(clear)(from); return; }
#ifdef LIST_OPENMP /* <-- omp */
#pragma omp parallel sections
#endif /* omp --> */
	{
#ifdef LIST_A_NAME /* <-- a */
#ifdef LIST_A_COMPARATOR /* <-- comp */
#ifdef LIST_OPENMP /* <-- omp */
#pragma omp section
#endif /* omp --> */
		PRIVATE_T_LA_(list, merge)(this, from);
#else /* comp --><-- !comp */
		PRIVATE_T_LA_(list, cat)(this, from);
#endif /* !comp --> */
#endif /* a --> */
#ifdef LIST_B_NAME /* <-- b */
#ifdef LIST_B_COMPARATOR /* <-- comp */
#ifdef LIST_OPENMP /* <-- omp */
#pragma omp section
#endif /* omp --> */
		PRIVATE_T_LB_(list, merge)(this, from);
#else /* comp --><-- !comp */
		PRIVATE_T_LB_(list, cat)(this, from);
#endif /* !comp --> */
#endif /* b --> */
#ifdef LIST_C_NAME /* <-- c */
#ifdef LIST_C_COMPARATOR /* <-- comp */
#ifdef LIST_OPENMP /* <-- omp */
#pragma omp section
#endif /* omp --> */
		PRIVATE_T_LC_(list, merge)(this, from);
#else /* comp --><-- !comp */
		PRIVATE_T_LC_(list, cat)(this, from);
#endif /* !comp --> */
#endif /* c --> */
#ifdef LIST_D_NAME /* <-- d */
#ifdef LIST_D_COMPARATOR /* <-- comp */
#ifdef LIST_OPENMP /* <-- omp */
#pragma omp section
#endif /* omp --> */
		PRIVATE_T_LD_(list, merge)(this, from);
#else /* comp --><-- !comp */
		PRIVATE_T_LD_(list, cat)(this, from);
#endif /* !comp --> */
#endif /* d --> */
	}
}

/** Performs a stable, adaptive sort. If {LIST_OPENMP} is defined, then it will
 try to parallelise; otherwise it is equivalent to calling \see{<T>List<L>Sort}
 for all linked-lists with comparators. Requires one of {LIST_[A-D]_COMPARATOR}
 be set.
 @order \Omega({this}.n), O({this}.n log {this}.n)
 @allow */
static void T_(ListSort)(struct T_(List) *const this) {
	if(!this) return;
#ifdef LIST_OPENMP /* <-- omp */
	#pragma omp parallel sections
#endif /* omp --> */
	{
#ifdef LIST_A_COMPARATOR /* <-- a */
#ifdef LIST_OPENMP /* <-- omp */
		#pragma omp section
#endif /* omp --> */
		PRIVATE_T_LA_(natural, sort)(this);
#endif /* a --> */
#ifdef LIST_B_COMPARATOR /* <-- b */
#ifdef LIST_OPENMP /* <-- omp */
		#pragma omp section
#endif /* omp --> */
		PRIVATE_T_LB_(natural, sort)(this);
#endif /* b --> */
#ifdef LIST_C_COMPARATOR /* <-- c */
#ifdef LIST_OPENMP /* <-- omp */
		#pragma omp section
#endif /* omp --> */
		PRIVATE_T_LC_(natural, sort)(this);
#endif /* c --> */
#ifdef LIST_D_COMPARATOR /* <-- d */
#ifdef LIST_OPENMP /* <-- omp */
		#pragma omp section
#endif /* omp --> */
		PRIVATE_T_LD_(natural, sort)(this);
#endif /* d --> */
	}
}
#endif /* comp --> */

/** Sets the user-defined {param} of {this}.
 @order \Theta(1)
 @allow */
static void T_(ListSetParam)(struct T_(List) *const this,
	void *const param) {
	if(!this) return;
	this->param = param;
}

/** Use when one {node} of {this} has switched places in memory. If {this} or
 {node} is null, doesn't do anything.
 @order \Theta(1)
 @allow */
static void T_(ListMigrate)(struct T_(List) *const this,
	struct T_(ListNode) *const node) {
	if(!this || !node) return;
#ifdef LIST_A_NAME /* <-- a */
	PRIVATE_T_LA_(list, migrate)(this, node);
#endif /* a --> */
#ifdef LIST_B_NAME /* <-- b */
	PRIVATE_T_LB_(list, migrate)(this, node);
#endif /* b --> */
#ifdef LIST_C_NAME /* <-- c */
	PRIVATE_T_LC_(list, migrate)(this, node);
#endif /* c --> */
#ifdef LIST_D_NAME /* <-- d */
	PRIVATE_T_LD_(list, migrate)(this, node);
#endif /* d --> */
}

/** When {this} contains elements from an array of/containing {<T>ListNode} in
 memory that switched from {old_array} to {array} with byte-size {array_size}.
 If {this}, {array}, or {old_array} is null, doesn't do anything.

 Specifically, use when {this} is (partially) backed with an array that has
 changed places due to a {realloc}.

 @order \Theta(n)
 @fixme Relies on not-strictly-defined behaviour because pointers are not
 necessarily contiguous in memory; it should be fine in practice.
 @allow */
static void T_(ListMigrateBlock)(struct T_(List) *const this,
	void *const array, const size_t array_size, const void *const old_array) {
	const char *const cold = old_array;
	const char *const cold_end = cold + array_size;
	const ptrdiff_t delta = (char *)array - cold;
	if(!this || !array || !array_size || !old_array || array == old_array)
		return;
#ifdef LIST_OPENMP /* <-- omp */
	#pragma omp parallel sections
#endif /* omp --> */
	{
#ifdef LIST_A_NAME /* <-- a */
#ifdef LIST_OPENMP /* <-- omp */
		#pragma omp section
#endif /* omp --> */
		PRIVATE_T_LA_(block, migrate)(this, cold, cold_end, delta);
#endif /* a --> */
#ifdef LIST_B_NAME /* <-- b */
#ifdef LIST_OPENMP /* <-- omp */
		#pragma omp section
#endif /* omp --> */
		PRIVATE_T_LB_(block, migrate)(this, cold, cold_end, delta);
#endif /* b --> */
#ifdef LIST_C_NAME /* <-- c */
#ifdef LIST_OPENMP /* <-- omp */
		#pragma omp section
#endif /* omp --> */
		PRIVATE_T_LC_(block, migrate)(this, cold, cold_end, delta);
#endif /* c --> */
#ifdef LIST_D_NAME /* <-- d */
#ifdef LIST_OPENMP /* <-- omp */
		#pragma omp section
#endif /* omp --> */
		PRIVATE_T_LD_(block, migrate)(this, cold, cold_end, delta);
#endif /* d --> */
	}
}

#ifdef LIST_TEST /* <-- test */
#include "../test/TestList.h" /* need this file if one is going to run tests */
#endif /* test --> */

/* prototype */
static void PRIVATE_T_(unused_coda)(void);
/** This silences unused function warnings from the pre-processor, but allows
 optimisation, (hopefully.)
 \url{ http://stackoverflow.com/questions/43841780/silencing-unused-static-function-warnings-for-a-section-of-code } */
static void PRIVATE_T_(unused_list)(void) {
	T_(ListClear)(0);
	T_(ListAdd)(0, 0);
	T_(ListRemove)(0, 0);
	T_(ListTake)(0, 0);
#ifdef LIST_SOME_COMPARATOR /* <-- comp */
	T_(ListMerge)(0, 0);
	T_(ListSort)(0);
#endif /* comp --> */
	T_(ListSetParam)(0, 0);
	T_(ListMigrate)(0, 0);
	T_(ListMigrateBlock)(0, 0, (size_t)0, 0);
	PRIVATE_T_(unused_coda)();
}
/** {clang}'s pre-processor is clever? */
static void PRIVATE_T_(unused_coda)(void) { PRIVATE_T_(unused_list)(); }





/* un-define all macros */
#undef LIST_NAME
#undef LIST_TYPE
#ifdef LIST_TO_STRING
#undef LIST_TO_STRING
#endif
#ifdef LIST_FILLER
#undef LIST_FILLER
#endif
#ifdef LIST_A_NAME
#undef LIST_A_NAME
#endif
#ifdef LIST_A_COMPARATOR
#undef LIST_A_COMPARATOR
#endif
#ifdef LIST_B_NAME
#undef LIST_B_NAME
#endif
#ifdef LIST_B_COMPARATOR
#undef LIST_B_COMPARATOR
#endif
#ifdef LIST_C_NAME
#undef LIST_C_NAME
#endif
#ifdef LIST_C_COMPARATOR
#undef LIST_C_COMPARATOR
#endif
#ifdef LIST_D_NAME
#undef LIST_D_NAME
#endif
#ifdef LIST_D_COMPARATOR
#undef LIST_D_COMPARATOR
#endif
#ifdef LIST_DYNAMIC_STORAGE
#undef LIST_DYNAMIC_STORAGE
#endif
#ifdef LIST_OPENMP
#undef LIST_OPENMP
#endif
#ifdef LIST_TEST
#undef LIST_TEST
#endif
#ifdef LIST_SOME_COMPARATOR
#undef LIST_SOME_COMPARATOR
#endif
#ifdef LIST_SORT_INTERNALS
#undef LIST_SORT_INTERNALS /* each List type has their own */
#endif





#else /* !LIST_L_NAME --><-- LIST_L_NAME

 Internally #included.

 @param LIST_L_NAME: A unique name of the linked list; required;
 @param LIST_L_COMPARATOR: an optional comparator. */





/* After this block, the preprocessor replaces T_M_(X, Y) with
 LIST_NAMEXLIST_L_NAMEY, PRIVATE_T_M_(X, Y) with
 list_LIST_L_NAME_X_LIST_L_NAME_Y */
#ifdef T_L_
#undef T_L_
#endif
#ifdef PRIVATE_T_L_
#undef PRIVATE_T_L_
#endif
#ifdef L_
#undef L_
#endif
#define T_L_(thing1, thing2) CAT(CAT(LIST_NAME, thing1), \
	CAT(LIST_L_NAME, thing2))
#define PRIVATE_T_L_(thing1, thing2) PCAT(list, PCAT(PCAT(LIST_NAME, thing1), \
	PCAT(LIST_L_NAME, thing2)))
#define L_(thing) PCAT(LIST_L_NAME, thing)



/** Private: add to {this.last} in {<L>}. */
static void PRIVATE_T_L_(list, add)(struct T_(List) *const this,
	struct T_(ListNode) *const node) {
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

/** Private: list remove in {<L>}. */
static void PRIVATE_T_L_(list, remove)(struct T_(List) *const this,
	struct T_(ListNode) *const node) {
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
}

/** Private: cats all {from} to the tail of {this}; {from} will be empty after.
 @order \Theta(1) */
static void PRIVATE_T_L_(list, cat)(struct T_(List) *const this,
	struct T_(List) *const from) {
	assert(this);
	assert(from);
	assert(!this->L_(first) == !this->L_(last));
	assert(!from->L_(first) == !from->L_(last));
	if(!from->L_(first)) {        /* there is nothing in {from} */
		return;
	} else if(!this->L_(first)) { /* there is nothing in {this} */
		this->L_(first) = from->L_(first);
	} else {                      /* there is something in both */
		this->L_(last)->L_(next) = from->L_(first);
	}
	this->L_(last) = from->L_(last);
	from->L_(first) = from->L_(last) = 0;
}

/** Private: when you've moved in memory just one thing. */
static void PRIVATE_T_L_(list, migrate)(struct T_(List) *const this,
	struct T_(ListNode) *const node) {
	assert(this);
	assert(node);
	if(node->L_(prev)) {
		node->L_(prev)->L_(next) = node;
	} else {
		this->L_(first) = node;
	}
	if(node->L_(next)) {
		node->L_(next)->L_(prev) = node;
	} else {
		this->L_(last) = node;
	}
}

/** Private: used in \see{_<T>_block_<L>_migrate}.
 \${ptr \in [begin, end) -> ptr += delta}. */
static void PRIVATE_T_L_(block, migrate_node)(
	struct T_(ListNode) **const node_ptr,
	const void *const begin, const void *const end, const ptrdiff_t delta) {
	const void *const ptr = *node_ptr;
	if(ptr < begin || ptr >= end) return;
	*(char **)node_ptr += delta;
}

/** Private: when {realloc} changes your pointers. I've tried to keep undefined
 behaviour to a minimum.
 @param [begin, end): Where the pointers have changed.
 @param delta: the offset they have moved to in memory bytes.
 @order \Theta(n) */
static void PRIVATE_T_L_(block, migrate)(struct T_(List) *const this,
	const void *const begin, const void *const end, const ptrdiff_t delta) {
	struct T_(ListNode) *node;
	assert(this);
	assert(begin);
	assert(begin < end);
	assert(delta);
	assert(!this->L_(first) == !this->L_(last));
	/* empty -- done */
	if(!this->L_(first)) return;
	/* first and last pointer of {<T>List} */
	PRIVATE_T_L_(block, migrate_node)(&this->L_(first), begin, end, delta);
	PRIVATE_T_L_(block, migrate_node)(&this->L_(last),  begin, end, delta);
	/* all the others' {<T>ListNode} */
	for(node = this->L_(first); node; node = node->L_(next)) {
		PRIVATE_T_L_(block, migrate_node)(&node->L_(prev), begin, end, delta);
		PRIVATE_T_L_(block, migrate_node)(&node->L_(next), begin, end, delta);
	}
}


/** @return The next element after {this} in {<L>}. When {this} is the last
 element or when {this} is null, returns null.
 @order \Theta(1)
 @allow */
static struct T_(ListNode) *T_L_(ListNode, GetNext)(
	struct T_(ListNode) *const this) {
	if(!this) return 0;
	return this->L_(next);
}

/** @return The previous element before {this} in {<L>}. When {this} is the
 first item or when {this} is null, returns null.
 @order \Theta(1)
 @allow */
static struct T_(ListNode) *T_L_(ListNode, GetPrevious)(
	struct T_(ListNode) *const this) {
	if(!this) return 0;
	return this->L_(prev);
}

/** @return A pointer to the first element of {this}.
 @order \Theta(1)
 @allow */
static struct T_(ListNode) *T_L_(List, GetFirst)(struct T_(List) *const this) {
	if(!this) return 0;
	return this->L_(first);
}

/** @return A pointer to the last element of {this}.
 @order \Theta(1)
 @allow */
static struct T_(ListNode) *T_L_(List, GetLast)(struct T_(List) *const this) {
	if(!this) return 0;
	return this->L_(last);
}

#ifdef LIST_L_COMPARATOR /* <-- comp */

#ifndef LIST_SORT_INTERNALS /* <!-- sort internals only once per translation
 unit */
#define LIST_SORT_INTERNALS
/* A run is a temporary sequence of values in the array that is weakly
 increasing; we store it's size temporarily. */
struct PRIVATE_T_(Run) {
	struct T_(ListNode) *head, *tail;
	size_t size;
};
/* Store the maximum capacity for the indexing with {size_t}. (Much more then
 we need, in most cases.) \${
 range(runs) = Sum_{k=0}^runs 2^{runs-k} - 1
             = 2^{runs+1} - 2
 2^bits      = 2 (r^runs - 1)
 runs        = log(2^{bits-1} + 1) / log 2
 runs       <= 2^{bits - 1}, 2^{bits + 1} > 0} */
struct PRIVATE_T_(Runs) {
	struct PRIVATE_T_(Run) run[(sizeof(size_t) << 3) - 1];
	size_t run_no;
};
#endif /* sort internals --> */

/* Check that each of LIST_[A-D]_COMPARATOR are functions implementing
 {<T>Comparator}. */
static const T_(Comparator) PRIVATE_T_L_(data, cmp) = (LIST_L_COMPARATOR);

/** Private: merges {from} into {this} when we don't know anything about the
 data; on equal elements, places {this} first.
 @order {O(n + m)}. */
static void PRIVATE_T_L_(list, merge)(struct T_(List) *const this,
	struct T_(List) *const from) {
	struct T_(ListNode) *a, *b; /* {a} ~ {this}, {b} ~ {from} */
	assert(this);
	assert(from);
	if(!(b = from->L_(first))) {
		/* {from} empty; declare success */
		return;
	} else if(!(a = this->L_(first))) {
		/* assignment */
		this->L_(first) = from->L_(first), this->L_(last) = from->L_(last);
	} else {
		/* merge */
		struct T_(ListNode) nemo, *const first = &nemo, *last = first;
		for( ; ; ) {
			if(PRIVATE_T_L_(data, cmp)(&a->data, &b->data) < 0) {
				a->L_(prev) = last, last = last->L_(next) = a;
				if(!(a = a->L_(next)))
					{ b->L_(prev) = last, last->L_(next) = b,
					this->L_(last) = from->L_(last); break; }
			} else {
				b->L_(prev) = last, last = last->L_(next) = b;
				if(!(b = b->L_(next)))
					{ a->L_(prev) = last, last->L_(next) = a; break; }
			}
			this->L_(first) = nemo.L_(next);
		}
	}
	from->L_(first) = from->L_(last) = 0;
}

#ifndef LIST_DYNAMIC_STORAGE /* <-- not dynamic: it will crash if it calls
 exactly this function concurrently */
static struct PRIVATE_T_(Runs) PRIVATE_T_L_(runs, elem);
#endif /* not dynamic --> */

/** Inserts the first element from the larger of two sorted runs, then merges
 the rest. \cite{Peters2002Timsort}, via \cite{McIlroy1993Optimistic}, does
 long merges by galloping, but we don't have random access to the data. In
 practice, this is {2%} slower on randomly distributed keys when the
 linked-list size is over {500 000}; randomly distributed keys have high
 insertion times that to well in standard merging. However, it's (potentially
 much) faster when the keys have structure: observed, {[-2%, 500%]}. Assumes
 array contains at least 2 elements and there are at least two runs. */
static void PRIVATE_T_L_(natural, merge)(struct PRIVATE_T_(Runs) *const r) {
	struct PRIVATE_T_(Run) *const run_a = r->run + r->run_no - 2;
	struct PRIVATE_T_(Run) *const run_b = run_a + 1;
	struct T_(ListNode) *a = run_a->tail, *b = run_b->head, *chosen;

	/* fixme: we are doing one-to-many compares in some cases? */

	if(run_a->size <= run_b->size) {
		struct T_(ListNode) *prev_chosen;

		/* run a is smaller: downwards insert b.head followed by upwards
		 merge */

		/* insert the first element of b downwards into a */
		for( ; ; ) {
			if(PRIVATE_T_L_(data, cmp)(&a->data, &b->data) <= 0) {
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
			if(PRIVATE_T_L_(data, cmp)(&a->data, &b->data) > 0) {
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
		struct T_(ListNode) *next_chosen;
		int is_a_tail = 0;

		/* run b is smaller; upwards insert followed by downwards merge */

		/* insert the last element of a upwards into b */
		for( ; ; ) {
			if(PRIVATE_T_L_(data, cmp)(&a->data, &b->data) <= 0) {
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
			if(PRIVATE_T_L_(data, cmp)(&a->data, &b->data) > 0) {
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
static void PRIVATE_T_L_(natural, sort)(struct T_(List) *const this) {
#ifdef LIST_DYNAMIC_STORAGE /* <-- dynamic: this is potentially half-a-KB */
	static struct PRIVATE_T_(Runs) PRIVATE_T_L_(runs, elem);
#endif /* dynamic --> */
	/* new_run is an index into list_runs, a temporary sorting structure;
	 head is first smallest, tail is last largest */
	struct PRIVATE_T_(Run) *new_run;
	/* part of the state machine for classifying points wrt their neighbours */
	enum { UNSURE, INCREASING, DECREASING } mono;
	/* the data that we are sorting */
	struct T_(ListNode) *a, *b, *c, *first_iso_a;
	/* run_count is different from list_runs.run_no in that it only increases;
	 only used for calculating the path up the tree */
	size_t run_count, rc;
	/* the value of the comparison */
	int comp;

	/* ensure we have an 'a' */
	if(!(a = this->L_(first))) return;

	/* reset the state machine and output to just 'a' in the first run */
	mono = UNSURE;
	PRIVATE_T_L_(runs, elem).run_no = 1;
	new_run = PRIVATE_T_L_(runs,elem).run + 0, run_count = 1;
	new_run->size = 1;
	first_iso_a = new_run->head = new_run->tail = a;

	for(b = a->L_(next); b; a = b, b = c) {

		/* b.next can be modified, and we always want the iteration original */
		c = b->L_(next);

		comp = PRIVATE_T_L_(data, cmp)(&a->data, &b->data);

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
				struct T_(ListNode) *const a_next = a->L_(next);
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
		for(rc=run_count; !(rc & 1)&&PRIVATE_T_L_(runs,elem).run_no>=2; rc >>=1)
			PRIVATE_T_L_(natural, merge)(&PRIVATE_T_L_(runs,elem));
		/* reset the state machine and output to just 'b' at the next run */
		mono = UNSURE;
		new_run = PRIVATE_T_L_(runs,elem).run +
			PRIVATE_T_L_(runs,elem).run_no++, run_count++;
		new_run->size = 1;
		new_run->head = new_run->tail = first_iso_a = b;
	}

	/* terminating the last increasing sequence */
	if(mono == INCREASING) new_run->tail = a;
		new_run->tail->L_(next) = new_run->head->L_(prev) = 0;

	/* clean up the rest; when only one run, propagate list_runs[0] to head */
	while(PRIVATE_T_L_(runs, elem).run_no > 1)
		PRIVATE_T_L_(natural, merge)(&PRIVATE_T_L_(runs, elem));
	this->L_(first) = PRIVATE_T_L_(runs, elem).run[0].head;
	this->L_(last)  = PRIVATE_T_L_(runs, elem).run[0].tail;
}

/** Sorts {<L>}, but leaves the other lists in {<T>} alone. Must have
 {LIST_[A-D]_COMPARATOR} defined.
 @order \Omega({this}.n), O({this}.n log {this}.n)
 @allow */
static void T_L_(List, Sort)(struct T_(List) *const this) {
	if(!this) return;
	PRIVATE_T_L_(natural, sort)(this);
}

/** Compares two linked-lists as sequences in the order specified by {<L>}.
 @return The first comparator that is not equal to zero, or 0 if they are
 equal. Two null pointers are considered equal. Must have
 {LIST_[A-D]_COMPARATOR} defined.
 @order \Theta(min({this}.n, {that}.n))
 @implements <<T>List>Comparator
 @allow */
static int T_L_(List, Compare)(const struct T_(List) *const this,
	const struct T_(List) *const that) {
	struct T_(ListNode) *a, *b;
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
		} else if((diff = PRIVATE_T_L_(data, cmp)(&a->data, &b->data))) {
			return diff;
		}
	}
}

/** Private: {this <- a \mask b}. Prefers {a} to {b} when equal.
 @order O({a}.n + {b}.n) */
static void PRIVATE_T_L_(boolean, seq)(struct T_(List) *const this,
	struct T_(List) *const a, struct T_(List) *const b,
	const enum ListOperation mask) {
	struct T_(ListNode) *ai = a ? a->L_(first) : 0, *bi = b ? b->L_(first) : 0,
		*t; /* iterator, temp */
	int comp; /* comparator */
	while(ai && bi) {
		comp = PRIVATE_T_L_(data, cmp)(&ai->data, &bi->data);
		if(comp < 0) {
			t = ai, ai = ai->L_(next);
			if(mask & LO_SUBTRACTION_AB) {
				PRIVATE_T_(remove)(a, t);
				if(this) PRIVATE_T_(add)(this, t);
			}
		} else if(comp > 0) {
			t = bi, bi = bi->L_(next);
			if(mask & LO_SUBTRACTION_BA) {
				PRIVATE_T_(remove)(b, t);
				if(this) PRIVATE_T_(add)(this, t);
			}
		} else {
			t = ai, ai = ai->L_(next), bi = bi->L_(next);
			if(mask & LO_INTERSECTION) {
				PRIVATE_T_(remove)(a, t);
				if(this) PRIVATE_T_(add)(this, t);
			}
		}
	}
	if(mask & LO_DEFAULT_A) {
		while(ai) {
			t = ai, ai = ai->L_(next);
			PRIVATE_T_(remove)(a, t);
			if(this) PRIVATE_T_(add)(this, t);
		}
	}
	if((mask & LO_DEFAULT_B)) {
		while(bi) {
			t = bi, bi = bi->L_(next);
			PRIVATE_T_(remove)(b, t);
			if(this) PRIVATE_T_(add)(this, t);
		}
	}
}

/** Appends {that} with {b} subtracted from {a} as a sequence in {<L>}. If
 {this} is null, then it removes elements. Must have {LIST_[A-D]_COMPARATOR}
 defined.
 @order O({a}.n + {b}.n)
 @allow */
static void T_L_(List, TakeSubtraction)(struct T_(List) *const this,
	struct T_(List) *const a, struct T_(List) *const b) {
	PRIVATE_T_L_(boolean, seq)(this, a, b, LO_SUBTRACTION_AB |LO_DEFAULT_A);
}

/** Appends {this} with the union of {a} and {b} as a sequence in {<L>}. Equal
 elements are moved from {a}. If {this} is null, then it removes elements. Must
 have {LIST_[A-D]_COMPARATOR} defined.
 @order O({a}.n + {b}.n)
 @allow */
static void T_L_(List, TakeUnion)(struct T_(List) *const this,
	struct T_(List) *const a, struct T_(List) *const b) {
	PRIVATE_T_L_(boolean, seq)(this, a, b, LO_SUBTRACTION_AB | LO_SUBTRACTION_BA
		| LO_INTERSECTION | LO_DEFAULT_A | LO_DEFAULT_B);
}

/** Appends {this} with the intersection of {a} and {b} as a sequence in {<L>}.
 Equal elements are moved from {a}. If {this} is null, then it removes elements.
 Must have {LIST_[A-D]_COMPARATOR} defined.
 @order O({a}.n + {b}.n)
 @allow */
static void T_L_(List, TakeIntersection)(struct T_(List) *const this,
	struct T_(List) *const a, struct T_(List) *const b) {
	PRIVATE_T_L_(boolean, seq)(this, a, b, LO_INTERSECTION);
}

/** Appends {this} with {a} exclusive-or {b} as a sequence in {<L>}. Equal
 elements are moved from {a}. If {this} is null, then it removes elements. Must
 have {LIST_[A-D]_COMPARATOR} defined.
 @order O({a}.n + {b}.n)
 @allow */
static void T_L_(List, TakeXor)(struct T_(List) *const this,
	struct T_(List) *const a, struct T_(List) *const b) {
	PRIVATE_T_L_(boolean, seq)(this, a, b, LO_SUBTRACTION_AB | LO_SUBTRACTION_BA
		| LO_DEFAULT_A | LO_DEFAULT_B);
}

#endif /* comp --> */

/** Appends {this} with {from} if {predicate} is null or true in the order
 specified by {<L>}. If {this} is null, then it removes elements.
 @order ~ \Theta({this}.n) \times O({predicate})
 @allow */
static void T_L_(List, TakeIf)(struct T_(List) *const this,
	struct T_(List) *const from, const T_(Predicate) predicate) {
	struct T_(ListNode) *cursor, *next_cursor;
	if(!from || from == this) return;
	for(cursor = from->L_(first); cursor; cursor = next_cursor) {
		next_cursor = cursor->L_(next);
		if(predicate && !predicate(&cursor->data, from->param)) continue;
		PRIVATE_T_(remove)(from, cursor);
		if(!this) continue;
		PRIVATE_T_(add)(this, cursor);
	}
}

/** Performs {action} for each element in the list in the order specified by
 {<L>}. For more flexibility, use \see{<T>List<L>ShortCircuit}, which takes a
 {<T>Predicate}, and return true.
 @order ~ \Theta({this}.n) \times O({action})
 @allow */
static void T_L_(List, ForEach)(struct T_(List) *const this,
	const T_(Action) action) {
	struct T_(ListNode) *cursor;
	if(!this || !action) return;
	for(cursor = this->L_(first); cursor; cursor = cursor->L_(next)) {
		action(&cursor->data);
	}
}

/** @return The first {<T>ListNode} in the linked-list, ordered by {<L>}, that
 causes the {predicate} with {<T>} as argument to return false, or null if the
 {predicate} is true for every case. If {this} or {predicate} is null, returns
 null.
 @order ~ O({this}.n) \times O({predicate})
 @allow */
static struct T_(ListNode) *T_L_(List, ShortCircuit)(
	struct T_(List) *const this, const T_(Predicate) predicate) {
	struct T_(ListNode) *cursor;
	if(!this || !predicate) return 0;
	for(cursor = this->L_(first); cursor; cursor = cursor->L_(next)) {
		if(!predicate(&cursor->data, this->param)) return cursor;
	}
	return 0;
}

#ifdef LIST_TO_STRING /* <-- print */

#ifndef LIST_PRINT_THINGS /* <-- once inside translation unit */
#define LIST_PRINT_THINGS

static const char *const list_cat_start     = "[ ";
static const char *const list_cat_end       = " ]";
static const char *const list_cat_alter_end = "...]";
static const char *const list_cat_sep       = ", ";
static const char *const list_cat_star      = "*";
static const char *const list_cat_null      = "null";

struct List_SuperCat {
	char *print, *cursor;
	size_t left;
	int is_truncated;
};
static void list_super_cat_init(struct List_SuperCat *const cat,
	char *const print, const size_t print_size) {
	cat->print = cat->cursor = print;
	cat->left = print_size;
	cat->is_truncated = 0;
	print[0] = '\0';
}
static void list_super_cat(struct List_SuperCat *const cat,
	const char *const append) {
	size_t lu_took; int took;
	if(cat->is_truncated) return;
	took = sprintf(cat->cursor, "%.*s", (int)cat->left, append);
	if(took < 0)  { cat->is_truncated = -1; return; } /*implementation defined*/
	if(took == 0) { return; }
	if((lu_took = (size_t)took) >= cat->left)
		cat->is_truncated = -1, lu_took = cat->left - 1;
	cat->cursor += lu_took, cat->left -= lu_took;
}
#endif /* once --> */

/** One can print 4 things at once before it overwrites. One must set
 {LIST_TO_STRING} to a function implementing {<T>ToString} to get this
 functionality.
 @return Prints the {this} in a static buffer.
 @order \Theta(1); it has a 255 character limit; every element takes some of it.
 @allow */
static char *T_L_(List, ToString)(const struct T_(List) *const this) {
	static char buffer[4][256];
	static int buffer_i;
	struct List_SuperCat cat;
	char scratch[12];
	struct T_(ListNode) *list;
	assert(strlen(list_cat_alter_end) >= strlen(list_cat_end));
	assert(sizeof buffer > strlen(list_cat_alter_end));
	list_super_cat_init(&cat, buffer[buffer_i],
		sizeof *buffer / sizeof **buffer - strlen(list_cat_alter_end));
	buffer_i++, buffer_i &= 3;
	if(!this) {
		list_super_cat(&cat, list_cat_null);
		return cat.print;
	}
	list_super_cat(&cat, list_cat_start);
	for(list = this->L_(first); list; list = list->L_(next)) {
		if(list != this->L_(first)) list_super_cat(&cat, list_cat_sep);
		PRIVATE_T_(to_string)(&list->data, &scratch), scratch[8] = '\0';
		list_super_cat(&cat, scratch);
		if(cat.is_truncated) break;
	}
	sprintf(cat.cursor, "%s",
		cat.is_truncated ? list_cat_alter_end : list_cat_end);
	return cat.print; /* static buffer */
}

#endif /* print --> */

/* prototype */
static void PRIVATE_T_L_(unused, coda)(void);
/** This silences unused function warnings from the pre-processor, but allows
 optimisation, (hopefully.)
 \url{ http://stackoverflow.com/questions/43841780/silencing-unused-static-function-warnings-for-a-section-of-code } */
static void PRIVATE_T_L_(unused, list)(void) {
	T_L_(ListNode, GetNext)(0);
	T_L_(ListNode, GetPrevious)(0);
	T_L_(List, GetFirst)(0);
	T_L_(List, GetLast)(0);
#ifdef LIST_L_COMPARATOR /* <-- comp */
	T_L_(List, Sort)(0);
	T_L_(List, Compare)(0, 0);
	T_L_(List, TakeSubtraction)(0, 0, 0);
	T_L_(List, TakeUnion)(0, 0, 0);
	T_L_(List, TakeIntersection)(0, 0, 0);
	T_L_(List, TakeXor)(0, 0, 0);
#endif /* comp --> */
	T_L_(List, TakeIf)(0, 0, 0);
	T_L_(List, ForEach)(0, 0);
	T_L_(List, ShortCircuit)(0, 0);
#ifdef LIST_TO_STRING /* <-- string */
	T_L_(List, ToString)(0);
#endif /* string --> */
	PRIVATE_T_L_(unused, coda)();
}
/** {clang}'s pre-processor is clever? */
static void PRIVATE_T_L_(unused, coda)(void) { PRIVATE_T_L_(unused, list)(); }



/* un-define stuff for the next */
#undef LIST_L_NAME
#ifdef LIST_L_COMPARATOR /* <-- comp */
#undef LIST_L_COMPARATOR
#endif /* comp --> */

#endif /* LIST_L_NAME --> */
