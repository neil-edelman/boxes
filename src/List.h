/** 2017 Neil Edelman, distributed under the terms of the MIT License;
 see readme.txt, or \url{ https://opensource.org/licenses/MIT }.

 {<T>List} organises doubly-linked-list(s) of {<T>ListNode}, (not plain {<T>},)
 of which data of type, {<T>}, must be set using {LIST_TYPE}. The {<T>ListNode}
 storage is the responsibility of the caller; that means it can be nestled in
 multiple polymorphic structures. Supports one to four different orders in the
 same type. The preprocessor macros are all undefined at the end of the file
 for convenience when including multiple {List} types in the same file. Random
 {LIST_*} macros should be avoided.

 @param LIST_NAME, LIST_TYPE
 The name that literally becomes {<T>}, and a valid type associated therewith;
 should be conformant to naming and to the maximum available length of
 identifiers. Must each be present before including.

 @param LIST_COMPARATOR or LIST_U[A-D]_NAME, LIST_U[A-D]_COMPARATOR
 Each {LIST_U[A-D]_NAME} literally becomes, {<U>}, an order. If you only need
 one order, you can skip it's name and define it anonymously, {<U>} will be
 empty. One can define an optional comparator, an equivalence relation function
 implementing {<T>Comparator}; {LIST_COMPARATOR} is used when you define the
 name anonymously.

 @param LIST_TO_STRING
 Optional print function implementing {<T>ToString}; makes available
 \see{<T>List<U>ToString}.

 @param LIST_STATIC_STORAGE
 This allocates {O(log max n)} space needed for merge sort statically, instead
 of stack every time the List is sorted. This does not allow it to sort data
 concurrently without crashing, but it consumes less space on the stack; about
 half-a-kilobyte, depending on {size_t}.

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
 @version	1.3; 2017-10 anonymous orders
 @since		1.2; 2017-07 made migrate simpler
			1.1; 2017-06 split Add into Push and Unshift
			1.0; 2017-05 separated from List.h
 @fixme {GCC}: {#pragma GCC diagnostic ignored "-Wconversion"}; libc 4.2
 {assert} bug on {LIST_TEST}.
 @fixme {MSVC}: {#pragma warning(disable: x)} where {x} is: 4464 contains '..'
 uhm, thanks?; 4706 not {Java}; 4710, 4711 inlined info; 4820 padding info;
 4996 not {C++11}.
 @fixme {clang}: {#pragma clang diagnostic ignored "-Wx"} where {x} is:
 {padded}; {documentation}; {documentation-unknown-command} it's not quite
 {clang-tags}; 3.8 {disabled-macro-expansion} on {toupper} in {LIST_TEST}. */

/* 2017-05-12 tested with:
 gcc version 4.2.1 (Apple Inc. build 5666) (dot 3)
 Apple clang version 1.7 (tags/Apple/clang-77) (based on LLVM 2.9svn)
 gcc version 4.9.2 (Debian 4.9.2-10)
 Microsoft Visual Studio Enterprise 2015 Version 14.0.25424.00 Update 3
 Borland 10.1 Embarcadero C++ 7.20 for Win32
 MinGW gcc version 4.9.3 (GCC) Win32
 gcc version 5.4.0 20160609 (Ubuntu 5.4.0-6ubuntu1~16.04.4)
 clang version 3.8.0-2ubuntu4 (tags/RELEASE_380/final) */



/* original #include in the user's C file, and not from calling recursively
 (all "LIST_*" names are assumed to be reserved) */
#if !defined(LIST_U_NAME) /* <-- !LIST_U_NAME */



#include <stddef.h>	/* ptrdiff_t */
#include <assert.h>	/* assert */
#ifdef LIST_TO_STRING /* <-- print */
#include <stdio.h>	/* sprintf */
#include <string.h>	/* strlen */
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
#if defined(LIST_UA_COMPARATOR) && !defined(LIST_UA_NAME)
#error List: LIST_UA_COMPARATOR requires LIST_UA_NAME.
#endif
#if defined(LIST_UB_COMPARATOR) && !defined(LIST_UB_NAME)
#error List: LIST_UB_COMPARATOR requires LIST_UB_NAME.
#endif
#if defined(LIST_UC_COMPARATOR) && !defined(LIST_UC_NAME)
#error List: LIST_UC_COMPARATOR requires LIST_UC_NAME.
#endif
#if defined(LIST_UD_COMPARATOR) && !defined(LIST_UD_NAME)
#error List: LIST_UD_COMPARATOR requires LIST_UD_NAME.
#endif
/* Anonymous one-order implicit macro into {UA} for convenience and brevity. */
#if !defined(LIST_UA_NAME) && !defined(LIST_UB_NAME) \
	&& !defined(LIST_UC_NAME) && !defined(LIST_UD_NAME) /* <-- anon */
#define LIST_U_ANONYMOUS
#define LIST_UA_NAME
#ifdef LIST_COMPARATOR
#define LIST_UA_COMPARATOR LIST_COMPARATOR
#endif
#else /* anon --><-- !anon */
#ifdef LIST_COMPARATOR
#error List: LIST_COMPARATOR can only be defined anonymously; use LIST_U[A-D]_COMPARATOR.
#endif
#endif /* !anon --> */
#if defined(LIST_TEST) && !defined(LIST_TO_STRING)
#error LIST_TEST requires LIST_TO_STRING.
#endif
#if !defined(LIST_TEST) && !defined(NDEBUG)
#define LIST_NDEBUG
#define NDEBUG
#endif
#if defined(LIST_UA_COMPARATOR) || defined(LIST_UB_COMPARATOR) \
	|| defined(LIST_UC_COMPARATOR) || defined(LIST_UD_COMPARATOR)
#define LIST_SOME_COMPARATOR
#endif





/* After this block, the preprocessor replaces T with LIST_TYPE, T_(X) with
 LIST_NAMEX, PRIVATE_T_(X) with LIST_U_NAME_X, and T_NAME with the string
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
#ifdef UA_
#undef UA_
#undef T_UA_
#undef PRIVATE_T_UA_
#endif
#ifdef UB_
#undef UB_
#undef T_UB_
#undef PRIVATE_T_UB_
#endif
#ifdef UC_
#undef UC_
#undef T_UC_
#undef PRIVATE_T_UC_
#endif
#ifdef UD_
#undef UD_
#undef T_UD_
#undef PRIVATE_T_UD_
#endif
/* data exclusively, public f'ns, and private f'ns */

#ifdef LIST_U_ANONYMOUS /* <-- anon: "empty macro arguments were standardized
in C99" */
#define UA_(thing) PCAT(anonymous, thing)
#define T_UA_(thing1, thing2) CAT(CAT(LIST_NAME, thing1), thing2)
#define PRIVATE_T_UA_(thing1, thing2) PCAT(list, PCAT(PCAT(LIST_NAME, thing1), \
	CAT(_, thing2)))
#else /* anon --><-- !anon */
#ifdef LIST_UA_NAME
#define UA_(thing) PCAT(LIST_UA_NAME, thing)
#define T_UA_(thing1, thing2) CAT(CAT(LIST_NAME, thing1), \
	CAT(LIST_UA_NAME, thing2))
#define PRIVATE_T_UA_(thing1, thing2) PCAT(list, PCAT(PCAT(LIST_NAME, thing1), \
	PCAT(LIST_UA_NAME, thing2)))
#endif
#ifdef LIST_UB_NAME
#define UB_(thing) PCAT(LIST_UB_NAME, thing)
#define T_UB_(thing1, thing2) CAT(CAT(LIST_NAME, thing1), \
	CAT(LIST_UB_NAME, thing2))
#define PRIVATE_T_UB_(thing1, thing2) PCAT(list, PCAT(PCAT(LIST_NAME, thing1), \
	PCAT(LIST_UB_NAME, thing2)))
#endif
#ifdef LIST_UC_NAME
#define UC_(thing) PCAT(LIST_UC_NAME, thing)
#define T_UC_(thing1, thing2) CAT(CAT(LIST_NAME, thing1), \
	CAT(LIST_UC_NAME, thing2))
#define PRIVATE_T_UC_(thing1, thing2) PCAT(list, PCAT(PCAT(LIST_NAME, thing1), \
	PCAT(LIST_UC_NAME, thing2)))
#endif
#ifdef LIST_UD_NAME
#define UD_(thing) PCAT(LIST_UD_NAME, thing)
#define T_UD_(thing1, thing2) CAT(CAT(LIST_NAME, thing1), \
	CAT(LIST_UD_NAME, thing2))
#define PRIVATE_T_UD_(thing1, thing2) PCAT(list, PCAT(PCAT(LIST_NAME, thing1), \
	PCAT(LIST_UD_NAME, thing2)))
#endif
#endif /* !anon --> */



/* constants across multiple includes in the same translation unit */
#ifndef LIST_H /* <-- LIST_H */
#define LIST_H
/* combine_sets() operations bit-vector; dummy {LO_?}: {clang -Weverything}
 complains that it is not closed under union, a very valid point. */
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

#ifndef MIGRATE /* <-- migrate */
#define MIGRATE
/** Contains information about a {realloc}. */
struct Migrate;
struct Migrate {
	const void *begin, *end; /* old pointers */
	ptrdiff_t delta;
};
/** Function call on {realloc}. */
typedef void (*Migrate)(void *const parent,
	const struct Migrate *const migrate);
#endif /* migrate --> */

#endif /* LIST_H */



/** A single link in the linked-list derived from {<T>}. Storage of this
 structure is the responsibility of the caller. The {<T>} is stored in the
 element {data}. */
struct T_(ListNode);
struct T_(ListNode) {
	T data; /* 1st so we can cast without the mess of {container_of} */
#ifdef LIST_UA_NAME
	struct T_(ListNode) *UA_(prev), *UA_(next);
#endif
#ifdef LIST_UB_NAME
	struct T_(ListNode) *UB_(prev), *UB_(next);
#endif
#ifdef LIST_UC_NAME
	struct T_(ListNode) *UC_(prev), *UC_(next);
#endif
#ifdef LIST_UD_NAME
	struct T_(ListNode) *UD_(prev), *UD_(next);
#endif
};

/** Serves as an a head for linked-list(s) of {<T>ListNode}. No initialisation
 is necessary when the variable is of {static} duration, otherwise use
 \see{<T>ListClear}. */
struct T_(List);
struct T_(List) {
#ifdef LIST_UA_NAME
	struct T_(ListNode) *UA_(first), *UA_(last);
#endif
#ifdef LIST_UB_NAME
	struct T_(ListNode) *UB_(first), *UB_(last);
#endif
#ifdef LIST_UC_NAME
	struct T_(ListNode) *UC_(first), *UC_(last);
#endif
#ifdef LIST_UD_NAME
	struct T_(ListNode) *UD_(first), *UD_(last);
#endif
};



/** Takes {<T>}. */
typedef void (*T_(Action))(T *const);

/** Takes {<T>} and <void *>. */
typedef void (*T_(BiAction))(T *const, void *const);

/** Takes {<T>List}. */
typedef void (*T_(ListAction))(struct T_(List) *const);

/** Takes {<T>List} and {<T>}. */
typedef void (*T_(ListItemAction))(struct T_(List) *const, T *const);

/** Takes {<T>List} and {<T>ListNode}. */
typedef void (*T_(ListNodeAction))(struct T_(List) *const,
	struct T_(ListNode) *const);

/** Takes two {<T>List}. */
typedef void (*T_(BiListAction))(struct T_(List) *const,
	struct T_(List) *const);

/** Takes three {<T>List}. */
typedef void (*T_(TriListAction))(struct T_(List) *const,
	struct T_(List) *const, struct T_(List) *const);

/** Takes {<T>} and returns {<T>}. */
typedef T *(*T_(UnaryOperator))(T *const);

/** Takes {<T>}, returns (non-zero) true or (zero) false. */
typedef int (*T_(Predicate))(T *const);

/** Takes {<T>} and {void *}, returns (non-zero) true or (zero) false. */
typedef int (*T_(BiPredicate))(T *const, void *const);

/** Compares two {<T>} values and returns less then, equal to, or greater then
 zero. Should do so forming an equivalence relation with respect to {<T>}. */
typedef int (*T_(Comparator))(const T *, const T *);

#ifdef LIST_TO_STRING /* <-- string */

/** Responsible for turning {<T>} (the first argument) into a 12 {char}
 null-terminated output string (the second.) */
typedef void (*T_(ToString))(const T *const, char (*const)[12]);

/* Check that {LIST_TO_STRING} is a function implementing {<T>ToString}. */
static const T_(ToString) PRIVATE_T_(to_string) = (LIST_TO_STRING);

#endif /* string --> */



/* Prototypes: needed for the next section, but undefined until later. */
static void PRIVATE_T_(push)(struct T_(List) *const this,
	struct T_(ListNode) *const node);
static void PRIVATE_T_(unshift)(struct T_(List) *const this,
	struct T_(ListNode) *const node);
static void PRIVATE_T_(remove)(struct T_(List) *const this,
	struct T_(ListNode) *const node);
static void PRIVATE_T_(migrate)(const struct Migrate *const migrate,
	struct T_(ListNode) **const node_ptr);

/* Note to future self: recursive includes. The {LIST_U_NAME} pre-processor flag
 controls this behaviour; we are currently in the {!LIST_U_NAME} section. These
 will get all the functions with {<U>} in them. */

#ifdef LIST_UA_NAME /* <-- a */
#define LIST_U_NAME LIST_UA_NAME
#ifdef LIST_UA_COMPARATOR /* <-- comp */
#define LIST_U_COMPARATOR LIST_UA_COMPARATOR
#endif /* comp --> */
#include "List.h"
#endif /* a --> */

#ifdef LIST_UB_NAME /* <-- b */
#define LIST_U_NAME LIST_UB_NAME
#ifdef LIST_UB_COMPARATOR /* <-- comp */
#define LIST_U_COMPARATOR LIST_UB_COMPARATOR
#endif /* comp --> */
#include "List.h"
#endif /* b --> */

#ifdef LIST_UC_NAME /* <-- c */
#define LIST_U_NAME LIST_UC_NAME
#ifdef LIST_UC_COMPARATOR /* <-- comp */
#define LIST_U_COMPARATOR LIST_UC_COMPARATOR
#endif /* comp --> */
#include "List.h"
#endif /* c --> */

#ifdef LIST_UD_NAME /* <-- d */
#define LIST_U_NAME LIST_UD_NAME
#ifdef LIST_UD_COMPARATOR /* <-- comp */
#define LIST_U_COMPARATOR LIST_UD_COMPARATOR
#endif /* comp --> */
#include "List.h"
#endif /* d --> */



/** Private: push to the end of the list.
 @implements <T>ListNodeAction */
static void PRIVATE_T_(push)(struct T_(List) *const this,
	struct T_(ListNode) *const node) {
	assert(this);
	assert(node);
#ifdef LIST_UA_NAME /* <-- a */
	PRIVATE_T_UA_(list, push)(this, node);
#endif /* a --> */
#ifdef LIST_UB_NAME /* <-- b */
	PRIVATE_T_UB_(list, push)(this, node);
#endif /* b --> */
#ifdef LIST_UC_NAME /* <-- c */
	PRIVATE_T_UC_(list, push)(this, node);
#endif /* c --> */
#ifdef LIST_UD_NAME /* <-- d */
	PRIVATE_T_UD_(list, push)(this, node);
#endif /* d --> */
}

/** Private: unshift to the beginning of the list.
 @implements <T>ListNodeAction */
static void PRIVATE_T_(unshift)(struct T_(List) *const this,
	struct T_(ListNode) *const node) {
	assert(this);
	assert(node);
#ifdef LIST_UA_NAME /* <-- a */
	PRIVATE_T_UA_(list, unshift)(this, node);
#endif /* a --> */
#ifdef LIST_UB_NAME /* <-- b */
	PRIVATE_T_UB_(list, unshift)(this, node);
#endif /* b --> */
#ifdef LIST_UC_NAME /* <-- c */
	PRIVATE_T_UC_(list, unshift)(this, node);
#endif /* c --> */
#ifdef LIST_UD_NAME /* <-- d */
	PRIVATE_T_UD_(list, unshift)(this, node);
#endif /* d --> */
}

/** Private: remove from list.
 @implements <T>ListNodeAction */
static void PRIVATE_T_(remove)(struct T_(List) *const this,
	struct T_(ListNode) *const node) {
	assert(this);
	assert(node);
#ifdef LIST_UA_NAME /* <-- a */
	PRIVATE_T_UA_(list, remove)(this, node);
#endif /* a --> */
#ifdef LIST_UB_NAME /* <-- b */
	PRIVATE_T_UB_(list, remove)(this, node);
#endif /* b --> */
#ifdef LIST_UC_NAME /* <-- c */
	PRIVATE_T_UC_(list, remove)(this, node);
#endif /* c --> */
#ifdef LIST_UD_NAME /* <-- d */
	PRIVATE_T_UD_(list, remove)(this, node);
#endif /* d --> */
}

/** Private: clear the list.
 @implements <T>ListAction */
static void PRIVATE_T_(clear)(struct T_(List) *const this) {
	assert(this);
#ifdef LIST_UA_NAME
	this->UA_(first) = this->UA_(last) = 0;
#endif
#ifdef LIST_UB_NAME
	this->UB_(first) = this->UB_(last) = 0;
#endif
#ifdef LIST_UC_NAME
	this->UC_(first) = this->UC_(last) = 0;
#endif
#ifdef LIST_UD_NAME
	this->UD_(first) = this->UD_(last) = 0;
#endif
}

/** Clears all values from {this}, thereby initialising the {<T>List}. If it
 contained a list, those values are free.
 @implements <T>ListAction
 @order \Theta(1)
 @allow */
static void T_(ListClear)(struct T_(List) *const this) {
	if(!this) return;
	PRIVATE_T_(clear)(this);
}

/** Initialises the contents of {node} to add it to the end of {this}. If
 either {this} or {node} is null, it does nothing.
 @param node: Must be a {<T>ListNode} with an internal {<T>} not associated to
 any list; this associates the {<T>ListNode} with the list until it is removed,
 see \see{<T>ListRemove} or \see{<T>ListClear}.
 @implements <T>ListNodeAction
 @order \Theta(1)
 @allow */
static void T_(ListPush)(struct T_(List) *const this, T *const node) {
	struct T_(ListNode) *const n
		= (struct T_(ListNode) *const)(void *const)node;
	if(!this || !node) return;
	PRIVATE_T_(push)(this, n);
}

/** Initialises the contents of {node} to add it to the beginning of {this}. If
 either {this} or {node} is null, it does nothing.
 @param node: Must be a {<T>ListNode} with an internal {<T>} not associated to
 any list; this associates the {<T>ListNode} with the list until it is removed,
 see \see{<T>ListRemove} or \see{<T>ListClear}.
 @implements <T>ListNodeAction
 @order \Theta(1)
 @fixme Untested.
 @allow */
static void T_(ListUnshift)(struct T_(List) *const this, T *const node) {
	struct T_(ListNode) *const n
		= (struct T_(ListNode) *const)(void *const)node;
	if(!this || !node) return;
	PRIVATE_T_(unshift)(this, n);
}

/** Removes {data} from the {this}. The {data} is now free to add to another
 list. Removing an element that was not added to {this} results in undefined
 behaviour. If either {this} or {data} is null, it does nothing.
 @implements <T>ListItemAction
 @order \Theta(1)
 @allow */
static void T_(ListRemove)(struct T_(List) *const this, T *const data) {
	if(!this || !data) return;
	PRIVATE_T_(remove)(this, (struct T_(ListNode) *)(void *)data);
}

/** Appends the elements of {from} onto {this}. If {this} is null, then it
 removes elements. Unlike the {<T>TriListAction}, where the elements are
 re-ordered based on {<U>}, (they would not be in-place, otherwise,) this
 function concatenates all the elements in each linked-list order.
 @implements <T>BiListAction
 @order \Theta(1)
 @allow */
static void T_(ListTake)(struct T_(List) *const this,
	struct T_(List) *const from) {
	if(!from || from == this) return;
	if(!this) { PRIVATE_T_(clear)(from); return; }
#ifdef LIST_UA_NAME /* <-- a */
	PRIVATE_T_UA_(list, cat)(this, from);
#endif /* a --> */
#ifdef LIST_UB_NAME /* <-- b */
	PRIVATE_T_UB_(list, cat)(this, from);
#endif /* b --> */
#ifdef LIST_UC_NAME /* <-- c */
	PRIVATE_T_UC_(list, cat)(this, from);
#endif /* c --> */
#ifdef LIST_UD_NAME /* <-- d */
	PRIVATE_T_UD_(list, cat)(this, from);
#endif /* d --> */
}

#ifdef LIST_SOME_COMPARATOR /* <-- comp */

/** Merges the elements into {this} from {from} in (local) order; concatenates
 all lists that don't have a {LIST_U[A-D]_COMPARATOR}. If {this} is null, then
 it removes elements.
 @implements <T>BiListAction
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
#ifdef LIST_UA_NAME /* <-- a */
#ifdef LIST_UA_COMPARATOR /* <-- comp */
#ifdef LIST_OPENMP /* <-- omp */
#pragma omp section
#endif /* omp --> */
		PRIVATE_T_UA_(list, merge)(this, from);
#else /* comp --><-- !comp */
		PRIVATE_T_UA_(list, cat)(this, from);
#endif /* !comp --> */
#endif /* a --> */
#ifdef LIST_UB_NAME /* <-- b */
#ifdef LIST_UB_COMPARATOR /* <-- comp */
#ifdef LIST_OPENMP /* <-- omp */
#pragma omp section
#endif /* omp --> */
		PRIVATE_T_UB_(list, merge)(this, from);
#else /* comp --><-- !comp */
		PRIVATE_T_UB_(list, cat)(this, from);
#endif /* !comp --> */
#endif /* b --> */
#ifdef LIST_UC_NAME /* <-- c */
#ifdef LIST_UC_COMPARATOR /* <-- comp */
#ifdef LIST_OPENMP /* <-- omp */
#pragma omp section
#endif /* omp --> */
		PRIVATE_T_UC_(list, merge)(this, from);
#else /* comp --><-- !comp */
		PRIVATE_T_UC_(list, cat)(this, from);
#endif /* !comp --> */
#endif /* c --> */
#ifdef LIST_UD_NAME /* <-- d */
#ifdef LIST_UD_COMPARATOR /* <-- comp */
#ifdef LIST_OPENMP /* <-- omp */
#pragma omp section
#endif /* omp --> */
		PRIVATE_T_UD_(list, merge)(this, from);
#else /* comp --><-- !comp */
		PRIVATE_T_UD_(list, cat)(this, from);
#endif /* !comp --> */
#endif /* d --> */
	}
}

#ifndef LIST_U_ANONYMOUS /* <-- !anon; already has ListSort, T_U_(List, Sort) */
/** Performs a stable, adaptive sort. If {LIST_OPENMP} is defined, then it will
 try to parallelise; otherwise it is equivalent to calling \see{<T>List<U>Sort}
 for all linked-lists with comparators. Requires one of {LIST_U[A-D]_COMPARATOR}
 be set.
 @implements <T>ListAction
 @order \Omega({this}.n), O({this}.n log {this}.n)
 @allow */
static void T_(ListSort)(struct T_(List) *const this) {
	if(!this) return;
#ifdef LIST_OPENMP /* <-- omp */
	#pragma omp parallel sections
#endif /* omp --> */
	{
#ifdef LIST_UA_COMPARATOR /* <-- a */
#ifdef LIST_OPENMP /* <-- omp */
		#pragma omp section
#endif /* omp --> */
		PRIVATE_T_UA_(natural, sort)(this);
#endif /* a --> */
#ifdef LIST_UB_COMPARATOR /* <-- b */
#ifdef LIST_OPENMP /* <-- omp */
		#pragma omp section
#endif /* omp --> */
		PRIVATE_T_UB_(natural, sort)(this);
#endif /* b --> */
#ifdef LIST_UC_COMPARATOR /* <-- c */
#ifdef LIST_OPENMP /* <-- omp */
		#pragma omp section
#endif /* omp --> */
		PRIVATE_T_UC_(natural, sort)(this);
#endif /* c --> */
#ifdef LIST_UD_COMPARATOR /* <-- d */
#ifdef LIST_OPENMP /* <-- omp */
		#pragma omp section
#endif /* omp --> */
		PRIVATE_T_UD_(natural, sort)(this);
#endif /* d --> */
	}
}
#endif /* !anon --> */

#endif /* comp --> */


/** Adjusts the pointers when supplied with a {Migrate} parameter, when {this}
 contains {<T>ListNode} elements from memory that switched due to a {realloc}.
 If {this} or {migrate} is null, doesn't do anything.
 @param void_this: A {struct <T>List *const} cast as {void *const}.
 @param migrate: A {struct} coming from a {Migrate} function.
 @implements Migrate
 @order \Theta(n)
 @fixme Relies on not-strictly-defined behaviour because pointers are not
 necessarily contiguous in memory; it should be fine in practice.
 @allow */
static void T_(ListMigrate)(void *const void_this,
	const struct Migrate *const migrate) {
	struct T_(List) *const this = void_this;
	if(!this || !migrate || !migrate->delta) return;
#ifdef LIST_DEBUG
	fprintf(stderr, "List<" QUOTE(LIST_NAME)
		"#%p: moved entries at #%p-#%p by %lu.\n", (void *)this,
		migrate->begin, migrate->end, (long unsigned)migrate->delta);
#endif
#ifdef LIST_OPENMP /* <-- omp */
	#pragma omp parallel sections
#endif /* omp --> */
	{
#ifdef LIST_UA_NAME /* <-- a */
#ifdef LIST_OPENMP /* <-- omp */
		#pragma omp section
#endif /* omp --> */
		PRIVATE_T_UA_(list, migrate)(this, migrate);
#endif /* a --> */
#ifdef LIST_UB_NAME /* <-- b */
#ifdef LIST_OPENMP /* <-- omp */
		#pragma omp section
#endif /* omp --> */
		PRIVATE_T_UB_(list, migrate)(this, migrate);
#endif /* b --> */
#ifdef LIST_UC_NAME /* <-- c */
#ifdef LIST_OPENMP /* <-- omp */
		#pragma omp section
#endif /* omp --> */
		PRIVATE_T_UC_(list, migrate)(this, migrate);
#endif /* c --> */
#ifdef LIST_UD_NAME /* <-- d */
#ifdef LIST_OPENMP /* <-- omp */
		#pragma omp section
#endif /* omp --> */
		PRIVATE_T_UD_(list, migrate)(this, migrate);
#endif /* d --> */
	}
}

/** Private: used in \see{<T>_list_<U>_migrate} and \see{<T>Migrate}.
 \${ptr \in [begin, end) -> ptr += delta}. */
static void PRIVATE_T_(migrate)(const struct Migrate *const migrate,
	struct T_(ListNode) **const node_ptr) {
	const void *const ptr = *node_ptr;
	if(ptr < migrate->begin || ptr >= migrate->end) return;
	*(char **)node_ptr += migrate->delta;
}

/* * This is important, sometimes, but a real pain to describe.
 Call this function with the address of any self-referential node pointers
 contained in the data itself, to make sure that they are updated on {realloc}.
 To update the list, see \see{<T>ListMigrate}.
 @fixme Untested.
 @allow */
/*static void T_(Migrate)(const struct Migrate *const migrate,
	T **const t_ptr) {
	if(!migrate || !t_ptr || !*t_ptr) return;
	PRIVATE_T_(migrate)(migrate, (struct T_(ListNode) **const)t_ptr);
}*/

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
	T_(ListPush)(0, 0);
	T_(ListUnshift)(0, 0);
	T_(ListRemove)(0, 0);
	T_(ListTake)(0, 0);
#ifdef LIST_SOME_COMPARATOR /* <-- comp */
	T_(ListMerge)(0, 0);
	T_(ListSort)(0);
#endif /* comp --> */
	T_(ListMigrate)(0, 0);
	/*T_(Migrate)(0, 0);*/
	PRIVATE_T_(unused_coda)();
}
/** {clang}'s pre-processor is not fooled if one has one function. */
static void PRIVATE_T_(unused_coda)(void) { PRIVATE_T_(unused_list)(); }





/* un-define all macros */
#undef LIST_NAME
#undef LIST_TYPE
#undef CAT
#undef CAT_
#undef PCAT
#undef PCAT_
#undef T
#undef T_
#undef PRIVATE_T_
#undef T_NAME
#undef QUOTE
#undef QUOTE_
#ifdef LIST_TO_STRING
#undef LIST_TO_STRING
#endif
#ifdef LIST_FILLER
#undef LIST_FILLER
#endif
#ifdef LIST_COMPARATOR
#undef LIST_COMPARATOR
#endif
#ifdef LIST_U_ANONYMOUS
#undef LIST_U_ANONYMOUS
#endif
#ifdef LIST_UA_NAME
#undef LIST_UA_NAME
#endif
#ifdef LIST_UA_COMPARATOR
#undef LIST_UA_COMPARATOR
#endif
#ifdef LIST_UB_NAME
#undef LIST_UB_NAME
#endif
#ifdef LIST_UB_COMPARATOR
#undef LIST_UB_COMPARATOR
#endif
#ifdef LIST_UC_NAME
#undef LIST_UC_NAME
#endif
#ifdef LIST_UC_COMPARATOR
#undef LIST_UC_COMPARATOR
#endif
#ifdef LIST_UD_NAME
#undef LIST_UD_NAME
#endif
#ifdef LIST_UD_COMPARATOR
#undef LIST_UD_COMPARATOR
#endif
#ifdef LIST_STATIC_STORAGE
#undef LIST_STATIC_STORAGE
#endif
#ifdef LIST_OPENMP
#undef LIST_OPENMP
#endif
#ifdef LIST_TEST
#undef LIST_TEST
#endif
#ifdef LIST_DEBUG
#undef LIST_DEBUG
#endif
#ifdef LIST_NDEBUG
#undef LIST_NDEBUG
#undef NDEBUG
#endif
#ifdef LIST_SOME_COMPARATOR
#undef LIST_SOME_COMPARATOR
#endif
#ifdef LIST_SORT_INTERNALS
#undef LIST_SORT_INTERNALS /* each List type has their own */
#endif





#else /* !LIST_U_NAME --><-- LIST_U_NAME

 Internally #included.

 @param LIST_U_NAME: A unique name of the linked list; required;
 @param LIST_U_COMPARATOR: an optional comparator. */





/* After this block, the preprocessor replaces T_U_(X, Y) with
 LIST_NAMEXLIST_U_NAMEY, PRIVATE_T_U_(X, Y) with
 list_LIST_U_NAME_X_LIST_U_NAME_Y */
#ifdef T_U_
#undef T_U_
#endif
#ifdef PRIVATE_T_U_
#undef PRIVATE_T_U_
#endif
#ifdef U_
#undef U_
#endif
#ifdef LIST_U_ANONYMOUS /* <-- anon: "empty macro arguments were standardized
in C99" */
#define U_(thing) PCAT(anonymous, thing)
#define T_U_(thing1, thing2) CAT(CAT(LIST_NAME, thing1), thing2)
#define PRIVATE_T_U_(thing1, thing2) PCAT(list, PCAT(PCAT(LIST_NAME, thing1), \
	CAT(_, thing2)))
#else /* anon --><-- !anon */
#define U_(thing) PCAT(LIST_U_NAME, thing)
#define T_U_(thing1, thing2) CAT(CAT(LIST_NAME, thing1), \
	CAT(LIST_U_NAME, thing2))
#define PRIVATE_T_U_(thing1, thing2) PCAT(list, PCAT(PCAT(LIST_NAME, thing1), \
	PCAT(LIST_U_NAME, thing2)))
#endif /* !anon --> */



/** "Floyd's" tortoise-hare algorithm for cycle detection when in debug mode.
 You do not want cycles!
 @implements <T>ListAction */
static void PRIVATE_T_U_(cycle, crash)(const struct T_(List) *const this) {
#ifdef LIST_DEBUG
	struct T_(ListNode) *turtle, *hare;
	assert(this);
	assert(!this->U_(first) == !this->U_(last));
	if(!(turtle = hare = this->U_(first))) return;
	for( ; (hare = hare->U_(next)) && (hare = hare->U_(next)); ) {
		turtle = turtle->U_(next);
		assert(turtle != hare);
	}
#else
	UNUSED(this);
#endif
}

/** Private: add to {this.last} in {<U>}.
 @implements <T>ListNodeAction */
static void PRIVATE_T_U_(list, push)(struct T_(List) *const this,
	struct T_(ListNode) *const node) {
	assert(this && node);
	node->U_(prev) = this->U_(last);
	node->U_(next) = 0;
	if(this->U_(last)) {
		this->U_(last)->U_(next) = node;
	} else {
		assert(!this->U_(first));
		this->U_(first) = node;
	}
	this->U_(last) = node;
	PRIVATE_T_U_(cycle, crash)(this);
}

/** Private: add before {this.first} in {<U>}.
 @implements <T>ListNodeAction */
static void PRIVATE_T_U_(list, unshift)(struct T_(List) *const this,
	struct T_(ListNode) *const node) {
	assert(this && node);
	node->U_(prev) = 0;
	node->U_(next) = this->U_(first);
	if(this->U_(first)) {
		this->U_(first)->U_(prev) = node;
	} else {
		assert(!this->U_(last));
		this->U_(last) = node;
	}
	this->U_(first) = node;
	PRIVATE_T_U_(cycle, crash)(this);
}

/** Private: list remove in {<U>}.
 @implements <T>ListNodeAction */
static void PRIVATE_T_U_(list, remove)(struct T_(List) *const this,
	struct T_(ListNode) *const node) {
	assert(this && node);
	if(node->U_(prev)) {
		node->U_(prev)->U_(next) = node->U_(next);
	} else {
		assert(this->U_(first) == node);
		this->U_(first) = node->U_(next);
	}
	if(node->U_(next)) {
		node->U_(next)->U_(prev) = node->U_(prev);
	} else {
		assert(this->U_(last) == node);
		this->U_(last) = node->U_(prev);
	}
}

/** Private: cats all {from} to the tail of {this}; {from} will be empty after.
 @implements <T>BiListAction
 @order \Theta(1) */
static void PRIVATE_T_U_(list, cat)(struct T_(List) *const this,
	struct T_(List) *const from) {
	assert(this && from && !this->U_(first) == !this->U_(last)
		&& !from->U_(first) == !from->U_(last));
	if(!from->U_(first)) {        /* there is nothing in {from} */
		return;
	} else if(!this->U_(first)) { /* there is nothing in {this} */
		this->U_(first) = from->U_(first);
	} else {                      /* there is something in both */
		this->U_(last)->U_(next) = from->U_(first);
	}
	this->U_(last) = from->U_(last);
	from->U_(first) = from->U_(last) = 0;
	PRIVATE_T_U_(cycle, crash)(this);
}

/** Private: callback when {realloc} changes pointers. Tried to keep undefined
 behaviour to a minimum.
 @order \Theta(n) */
static void PRIVATE_T_U_(list, migrate)(struct T_(List) *const this,
	const struct Migrate *const migrate) {
	struct T_(ListNode) *node;
	assert(this && migrate && migrate->begin && migrate->begin < migrate->end
		&& migrate->delta && !this->U_(first) == !this->U_(last));
	/* empty -- done */
	if(!this->U_(first)) return;
	/* first and last pointer of {<T>List} */
	PRIVATE_T_(migrate)(migrate, &this->U_(first));
	PRIVATE_T_(migrate)(migrate, &this->U_(last));
	/* all the others' {<T>ListNode} */
	for(node = this->U_(first); node; node = node->U_(next)) {
		PRIVATE_T_(migrate)(migrate, &node->U_(prev));
		PRIVATE_T_(migrate)(migrate, &node->U_(next));
	}
	PRIVATE_T_U_(cycle, crash)(this);
}



/** @return The next element after {this} in {<U>}. When {this} is the last
 element or when {this} is null, returns null.
 @param this: Must be in a {List} as a {<T>ListNode}.
 @implements <T>UnaryOperator
 @order \Theta(1)
 @allow */
static T *T_U_(Node, GetNext)(T *const this) {
	struct T_(ListNode) *const node = (struct T_(ListNode) *const)(void *)this;
	if(!node || !node->U_(next)) return 0;
	return &node->U_(next)->data;
}

/** @return The previous element before {this} in {<U>}. When {this} is the
 first item or when {this} is null, returns null.
 @param this: Must be in a {List} as a {<T>ListNode}.
 @implements <T>UnaryOperator
 @order \Theta(1)
 @allow */
static T *T_U_(Node, GetPrevious)(T *const this) {
	struct T_(ListNode) *const node = (struct T_(ListNode) *const)(void *)this;
	if(!node || !node->U_(prev)) return 0;
	return &node->U_(prev)->data;
}

/** @return A pointer to the first element of {this}.
 @order \Theta(1)
 @allow */
static T *T_U_(List, GetFirst)(struct T_(List) *const this) {
	if(!this || !this->U_(first)) return 0;
	return &this->U_(first)->data;
}

/** @return A pointer to the last element of {this}.
 @order \Theta(1)
 @allow */
static T *T_U_(List, GetLast)(struct T_(List) *const this) {
	if(!this || !this->U_(last)) return 0;
	return &this->U_(last)->data;
}

#ifdef LIST_U_COMPARATOR /* <-- comp */

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

/* Check that each of LIST_U[A-D]_COMPARATOR are functions implementing
 {<T>Comparator}. */
static const T_(Comparator) PRIVATE_T_U_(data, cmp) = (LIST_U_COMPARATOR);

/** Private: merges {from} into {this} when we don't know anything about the
 data; on equal elements, places {this} first.
 @implements <T>BiListAction
 @order {O(n + m)}. */
static void PRIVATE_T_U_(list, merge)(struct T_(List) *const this,
	struct T_(List) *const from) {
	struct T_(ListNode) *a, *b; /* {a} ~ {this}, {b} ~ {from} */
	assert(this);
	assert(from);
	if(!(b = from->U_(first))) {
		/* {from} empty; declare success */
		return;
	} else if(!(a = this->U_(first))) {
		/* assignment */
		this->U_(first) = from->U_(first), this->U_(last) = from->U_(last);
	} else {
		/* merge */
		struct T_(ListNode) nemo, *const first = &nemo, *last = first;
		for( ; ; ) {
			if(PRIVATE_T_U_(data, cmp)(&a->data, &b->data) < 0) {
				a->U_(prev) = last, last = last->U_(next) = a;
				if(!(a = a->U_(next)))
					{ b->U_(prev) = last, last->U_(next) = b,
					this->U_(last) = from->U_(last); break; }
			} else {
				b->U_(prev) = last, last = last->U_(next) = b;
				if(!(b = b->U_(next)))
					{ a->U_(prev) = last, last->U_(next) = a; break; }
			}
			this->U_(first) = nemo.U_(next);
		}
	}
	from->U_(first) = from->U_(last) = 0;
}

/** Inserts the first element from the larger of two sorted runs, then merges
 the rest. \cite{Peters2002Timsort}, via \cite{McIlroy1993Optimistic}, does
 long merges by galloping, but we don't have random access to the data. In
 practice, this is {2%} slower on randomly distributed keys when the
 linked-list size is over {500 000}; randomly distributed keys have high
 insertion times that to well in standard merging. However, it's (potentially
 much) faster when the keys have structure: observed, {[-2%, 500%]}. Assumes
 array contains at least 2 elements and there are at least two runs. */
static void PRIVATE_T_U_(natural, merge)(struct PRIVATE_T_(Runs) *const r) {
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
			if(PRIVATE_T_U_(data, cmp)(&a->data, &b->data) <= 0) {
				chosen = a;
				a = a->U_(next);
				break;
			}
			if(!a->U_(prev)) {
				run_a->head = run_b->head;
				chosen = b;
				b = b->U_(next);
				break;
			}
			a = a->U_(prev);
		}

		/* merge upwards, while the lists are interleaved */
		while(chosen->U_(next)) {
			prev_chosen = chosen;
			if(PRIVATE_T_U_(data, cmp)(&a->data, &b->data) > 0) {
				chosen = b;
				b = b->U_(next);
			} else {
				chosen = a;
				a = a->U_(next);
			}
			prev_chosen->U_(next) = chosen;
			chosen->U_(prev) = prev_chosen;
		}

		/* splice the one list left */
		if(!a) {
			b->U_(prev) = chosen;
			chosen->U_(next) = b;
			run_a->tail = run_b->tail;
		} else {
			a->U_(prev) = chosen;
			chosen->U_(next) = a;
		}

	} else {
		struct T_(ListNode) *next_chosen;
		int is_a_tail = 0;

		/* run b is smaller; upwards insert followed by downwards merge */

		/* insert the last element of a upwards into b */
		for( ; ; ) {
			if(PRIVATE_T_U_(data, cmp)(&a->data, &b->data) <= 0) {
				chosen = b;
				b = b->U_(prev);
				break;
			}
			/* here, a > b */
			if(!b->U_(next)) {
				is_a_tail = -1;
				chosen = a;
				a = a->U_(prev);
				break;
			}
			b = b->U_(next);
		}
		if(!is_a_tail) run_a->tail = run_b->tail;

		/* merge downwards, while the lists are interleaved */
		while(chosen->U_(prev)) {
			next_chosen = chosen;
			if(PRIVATE_T_U_(data, cmp)(&a->data, &b->data) > 0) {
				chosen = a;
				a = a->U_(prev);
			} else {
				chosen = b;
				b = b->U_(prev);
			}
			next_chosen->U_(prev) = chosen;
			chosen->U_(next) = next_chosen;
		}

		/* splice the one list left */
		if(!a) {
			b->U_(next) = chosen;
			chosen->U_(prev) = b;
			run_a->head = run_b->head;
		} else {
			a->U_(next) = chosen;
			chosen->U_(prev) = a;
		}

	}

	run_a->size += run_b->size;
	r->run_no--;
}

#ifdef LIST_STATIC_STORAGE /* <-- static: it will crash if it calls exactly
this function concurrently */
static struct PRIVATE_T_(Runs) PRIVATE_T_U_(runs, elem);
#endif /* not dynamic --> */

/** It's kind of experimental. It hasn't been optimised; I think it does
 useless compares and I question whether a strict Pascal's triangle-shape
 would be optimum, or whether a long run should be put off merging until
 short runs have finished; it is quite simple as it is.
 @implements <T>ListAction */
static void PRIVATE_T_U_(natural, sort)(struct T_(List) *const this) {
#ifndef LIST_STATIC_STORAGE /* <-- dynamic: this is potentially half-a-KB */
	static struct PRIVATE_T_(Runs) PRIVATE_T_U_(runs, elem);
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
	if(!(a = this->U_(first))) return;

	/* reset the state machine and output to just 'a' in the first run */
	mono = UNSURE;
	PRIVATE_T_U_(runs, elem).run_no = 1;
	new_run = PRIVATE_T_U_(runs,elem).run + 0, run_count = 1;
	new_run->size = 1;
	first_iso_a = new_run->head = new_run->tail = a;

	for(b = a->U_(next); b; a = b, b = c) {

		/* b.next can be modified, and we always want the iteration original */
		c = b->U_(next);

		comp = PRIVATE_T_U_(data, cmp)(&a->data, &b->data);

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
				b->U_(next) = first_iso_a;
				first_iso_a->U_(prev) = b;
				new_run->head = first_iso_a = b;
				new_run->size++;
				continue;
			}
			new_run->tail = a; /* terminating an increasing sequence */
		} else { /* a == b */
			if(mono == DECREASING) { /* extend */
				struct T_(ListNode) *const a_next = a->U_(next);
				b->U_(next) = a_next;
				a_next->U_(prev) = b;
				a->U_(next) = b;
				b->U_(prev) = a;
			} else { /* weakly increasing */
				new_run->tail = b;
			}
			new_run->size++;
			continue;
		}
		/* head and tail don't necessarily correspond to the first and last */
		new_run->head->U_(prev) = new_run->tail->U_(next) = 0;

		/* greedy merge: keeps space to O(log n) instead of O(n) */
		for(rc=run_count; !(rc & 1)&&PRIVATE_T_U_(runs,elem).run_no>=2; rc >>=1)
			PRIVATE_T_U_(natural, merge)(&PRIVATE_T_U_(runs,elem));
		/* reset the state machine and output to just 'b' at the next run */
		mono = UNSURE;
		new_run = PRIVATE_T_U_(runs,elem).run +
			PRIVATE_T_U_(runs,elem).run_no++, run_count++;
		new_run->size = 1;
		new_run->head = new_run->tail = first_iso_a = b;
	}

	/* terminating the last increasing sequence */
	if(mono == INCREASING) new_run->tail = a;
		new_run->tail->U_(next) = new_run->head->U_(prev) = 0;

	/* clean up the rest; when only one run, propagate list_runs[0] to head */
	while(PRIVATE_T_U_(runs, elem).run_no > 1)
		PRIVATE_T_U_(natural, merge)(&PRIVATE_T_U_(runs, elem));
	this->U_(first) = PRIVATE_T_U_(runs, elem).run[0].head;
	this->U_(last)  = PRIVATE_T_U_(runs, elem).run[0].tail;
}

/** Sorts {<U>}, but leaves the other lists in {<T>} alone. Must have
 {LIST_U[A-D]_COMPARATOR} defined.
 @implements <T>ListAction
 @order \Omega({this}.n), O({this}.n log {this}.n)
 @allow */
static void T_U_(List, Sort)(struct T_(List) *const this) {
	if(!this) return;
	PRIVATE_T_U_(natural, sort)(this);
}

/** Compares two linked-lists as sequences in the order specified by {<U>}.
 @return The first comparator that is not equal to zero, or 0 if they are
 equal. Two null pointers are considered equal. Must have
 {LIST_U[A-D]_COMPARATOR} defined.
 @implements <<T>List>Comparator
 @order \Theta(min({this}.n, {that}.n))
 @allow */
static int T_U_(List, Compare)(const struct T_(List) *const this,
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
	for(a = this->U_(first), b = that->U_(first); ;
		a = a->U_(next), b = b->U_(next)) {
		if(!a) {
			return b ? -1 : 0;
		} else if(!b) {
			return 1;
		} else if((diff = PRIVATE_T_U_(data, cmp)(&a->data, &b->data))) {
			return diff;
		}
	}
}

/** Private: {this <- a \mask b}. Prefers {a} to {b} when equal.
 @implements <T>TriListAction
 @order O({a}.n + {b}.n) */
static void PRIVATE_T_U_(boolean, seq)(struct T_(List) *const this,
	struct T_(List) *const a, struct T_(List) *const b,
	const enum ListOperation mask) {
	struct T_(ListNode) *ai = a ? a->U_(first) : 0, *bi = b ? b->U_(first) : 0,
		*t; /* iterator, temp */
	int comp; /* comparator */
	while(ai && bi) {
		comp = PRIVATE_T_U_(data, cmp)(&ai->data, &bi->data);
		if(comp < 0) {
			t = ai, ai = ai->U_(next);
			if(mask & LO_SUBTRACTION_AB) {
				PRIVATE_T_(remove)(a, t);
				if(this) PRIVATE_T_(push)(this, t);
			}
		} else if(comp > 0) {
			t = bi, bi = bi->U_(next);
			if(mask & LO_SUBTRACTION_BA) {
				PRIVATE_T_(remove)(b, t);
				if(this) PRIVATE_T_(push)(this, t);
			}
		} else {
			t = ai, ai = ai->U_(next), bi = bi->U_(next);
			if(mask & LO_INTERSECTION) {
				PRIVATE_T_(remove)(a, t);
				if(this) PRIVATE_T_(push)(this, t);
			}
		}
	}
	if(mask & LO_DEFAULT_A) {
		while(ai) {
			t = ai, ai = ai->U_(next);
			PRIVATE_T_(remove)(a, t);
			if(this) PRIVATE_T_(push)(this, t);
		}
	}
	if((mask & LO_DEFAULT_B)) {
		while(bi) {
			t = bi, bi = bi->U_(next);
			PRIVATE_T_(remove)(b, t);
			if(this) PRIVATE_T_(push)(this, t);
		}
	}
}

/** Appends {that} with {b} subtracted from {a} as a sequence in {<U>}. If
 {this} is null, then it removes elements. Must have {LIST_U[A-D]_COMPARATOR}
 defined.
 @implements <T>TriListAction
 @order O({a}.n + {b}.n)
 @allow */
static void T_U_(List, TakeSubtraction)(struct T_(List) *const this,
	struct T_(List) *const a, struct T_(List) *const b) {
	PRIVATE_T_U_(boolean, seq)(this, a, b, LO_SUBTRACTION_AB |LO_DEFAULT_A);
}

/** Appends {this} with the union of {a} and {b} as a sequence in {<U>}. Equal
 elements are moved from {a}. If {this} is null, then it removes elements. Must
 have {LIST_U[A-D]_COMPARATOR} defined.
 @implements <T>TriListAction
 @order O({a}.n + {b}.n)
 @allow */
static void T_U_(List, TakeUnion)(struct T_(List) *const this,
	struct T_(List) *const a, struct T_(List) *const b) {
	PRIVATE_T_U_(boolean, seq)(this, a, b, LO_SUBTRACTION_AB | LO_SUBTRACTION_BA
		| LO_INTERSECTION | LO_DEFAULT_A | LO_DEFAULT_B);
}

/** Appends {this} with the intersection of {a} and {b} as a sequence in {<U>}.
 Equal elements are moved from {a}. If {this} is null, then it removes elements.
 Must have {LIST_U[A-D]_COMPARATOR} defined.
 @implements <T>TriListAction
 @order O({a}.n + {b}.n)
 @allow */
static void T_U_(List, TakeIntersection)(struct T_(List) *const this,
	struct T_(List) *const a, struct T_(List) *const b) {
	PRIVATE_T_U_(boolean, seq)(this, a, b, LO_INTERSECTION);
}

/** Appends {this} with {a} exclusive-or {b} as a sequence in {<U>}. Equal
 elements are moved from {a}. If {this} is null, then it removes elements. Must
 have {LIST_U[A-D]_COMPARATOR} defined.
 @implements <T>TriListAction
 @order O({a}.n + {b}.n)
 @allow */
static void T_U_(List, TakeXor)(struct T_(List) *const this,
	struct T_(List) *const a, struct T_(List) *const b) {
	PRIVATE_T_U_(boolean, seq)(this, a, b, LO_SUBTRACTION_AB | LO_SUBTRACTION_BA
		| LO_DEFAULT_A | LO_DEFAULT_B);
}

#endif /* comp --> */

/** Appends {this} with {from} if {predicate} is null or true in the order
 specified by {<U>}. If {this} is null, then it removes elements.
 @order ~ \Theta({this}.n) \times O({predicate})
 @allow */
static void T_U_(List, TakeIf)(struct T_(List) *const this,
	struct T_(List) *const from, const T_(Predicate) predicate) {
	struct T_(ListNode) *cursor, *next_cursor;
	if(!from || from == this) return;
	for(cursor = from->U_(first); cursor; cursor = next_cursor) {
		next_cursor = cursor->U_(next);
		if(predicate && !predicate(&cursor->data)) continue;
		PRIVATE_T_(remove)(from, cursor);
		if(!this) continue;
		PRIVATE_T_(push)(this, cursor);
	}
}

/** Appends {this} with {from} if {bipredicate} is null or true in the order
 specified by {<U>}. If {this} is null, then it removes elements.
 @order ~ \Theta({this}.n) \times O({predicate})
 @allow */
static void T_U_(List, BiTakeIf)(struct T_(List) *const this,
	struct T_(List) *const from, const T_(BiPredicate) bipredicate,
	void *const param) {
	struct T_(ListNode) *cursor, *next_cursor;
	if(!from || from == this) return;
	for(cursor = from->U_(first); cursor; cursor = next_cursor) {
		next_cursor = cursor->U_(next);
		if(bipredicate && !bipredicate(&cursor->data, param)) continue;
		PRIVATE_T_(remove)(from, cursor);
		if(!this) continue;
		PRIVATE_T_(push)(this, cursor);
	}
}

/** Performs {action} for each element in the list in the order specified by
 {<U>}. You can tranfer or delete the data, or see \see{<T>List<U>TakeIf}.
 @order ~ \Theta({this}.n) \times O({action})
 @allow */
static void T_U_(List, ForEach)(struct T_(List) *const this,
	const T_(Action) action) {
	struct T_(ListNode) *cursor, *next;
	if(!this || !action) return;
	for(cursor = this->U_(first); cursor; cursor = next) {
		next = cursor->U_(next);
		action(&cursor->data);
	}
}

/** Performs {biaction} for each element in the list in the order specified by
 {<U>}.
 @order ~ \Theta({this}.n) \times O({action})
 @fixme Untested.
 @allow */
static void T_U_(List, BiForEach)(struct T_(List) *const this,
	const T_(BiAction) biaction, void *const param) {
	struct T_(ListNode) *cursor;
	if(!this || !biaction) return;
	for(cursor = this->U_(first); cursor; cursor = cursor->U_(next)) {
		biaction(&cursor->data, param);
	}
}

/** @return The first {<T>} in the linked-list, ordered by {<U>}, that causes
 the {predicate} with {<T>} as argument to return false, or null if the
 {predicate} is true for every case. If {this} or {predicate} is null, returns
 null.
 @order ~ O({this}.n) \times O({predicate})
 @allow */
static T *T_U_(List, ShortCircuit)(struct T_(List) *const this,
	const T_(Predicate) predicate) {
	struct T_(ListNode) *cursor;
	if(!this || !predicate) return 0;
	for(cursor = this->U_(first); cursor; cursor = cursor->U_(next)) {
		if(!predicate(&cursor->data)) return &cursor->data;
	}
	return 0;
}

/** @return The first {<T>} in the linked-list, ordered by {<U>}, that
 causes the {bipredicate} with {<T>} and {param} as arguments to return false,
 or null if the {bipredicate} is true for every case. If {this} or
 {bipredicate} is null, returns null.
 @order ~ O({this}.n) \times O({predicate})
 @allow */
static T *T_U_(List, BiShortCircuit)(struct T_(List) *const this,
	const T_(BiPredicate) bipredicate, void *const param) {
	struct T_(ListNode) *cursor;
	if(!this || !bipredicate) return 0;
	for(cursor = this->U_(first); cursor; cursor = cursor->U_(next)) {
		if(!bipredicate(&cursor->data, param)) return &cursor->data;
	}
	return 0;
}

#ifdef LIST_TO_STRING /* <-- print */

#ifndef LIST_PRINT_THINGS /* <-- once inside translation unit */
#define LIST_PRINT_THINGS

static const char *const list_cat_start     = "{ ";
static const char *const list_cat_end       = " }";
static const char *const list_cat_alter_end = "...}";
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

/** Can print 4 things at once before it overwrites. One must set
 {LIST_TO_STRING} to a function implementing {<T>ToString} to get this
 functionality.
 @return Prints the {this} in a static buffer.
 @order \Theta(1); it has a 255 character limit; every element takes some of it.
 @allow */
static char *T_U_(List, ToString)(const struct T_(List) *const this) {
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
	for(list = this->U_(first); list; list = list->U_(next)) {
		if(list != this->U_(first)) list_super_cat(&cat, list_cat_sep);
		PRIVATE_T_(to_string)(&list->data, &scratch),
			scratch[sizeof scratch - 1] = '\0';
		list_super_cat(&cat, scratch);
		if(cat.is_truncated) break;
	}
	sprintf(cat.cursor, "%s",
		cat.is_truncated ? list_cat_alter_end : list_cat_end);
	return cat.print; /* static buffer */
}

#endif /* print --> */

/* prototype */
static void PRIVATE_T_U_(sub_unused, coda)(void);
/** This silences unused function warnings from the pre-processor, but allows
 optimisation, (hopefully.)
 \url{ http://stackoverflow.com/questions/43841780/silencing-unused-static-function-warnings-for-a-section-of-code } */
static void PRIVATE_T_U_(sub_unused, list)(void) {
	T_U_(Node, GetNext)(0);
	T_U_(Node, GetPrevious)(0);
	T_U_(List, GetFirst)(0);
	T_U_(List, GetLast)(0);
#ifdef LIST_U_COMPARATOR /* <-- comp */
	T_U_(List, Sort)(0);
	T_U_(List, Compare)(0, 0);
	T_U_(List, TakeSubtraction)(0, 0, 0);
	T_U_(List, TakeUnion)(0, 0, 0);
	T_U_(List, TakeIntersection)(0, 0, 0);
	T_U_(List, TakeXor)(0, 0, 0);
#endif /* comp --> */
	T_U_(List, TakeIf)(0, 0, 0);
	T_U_(List, BiTakeIf)(0, 0, 0, 0);
	T_U_(List, ForEach)(0, 0);
	T_U_(List, BiForEach)(0, 0, 0);
	T_U_(List, ShortCircuit)(0, 0);
	T_U_(List, BiShortCircuit)(0, 0, 0);
#ifdef LIST_TO_STRING /* <-- string */
	T_U_(List, ToString)(0);
#endif /* string --> */
	PRIVATE_T_U_(cycle, crash)(0);
	PRIVATE_T_U_(sub_unused, coda)();
}
/** {clang}'s pre-processor is not fooled. */
static void PRIVATE_T_U_(sub_unused, coda)(void) {
	PRIVATE_T_U_(sub_unused, list)();
}



/* un-define stuff for the next */
#undef LIST_U_NAME
#ifdef LIST_U_COMPARATOR /* <-- comp */
#undef LIST_U_COMPARATOR
#endif /* comp --> */

#endif /* LIST_U_NAME --> */
