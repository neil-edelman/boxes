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
 @version	2018-02 Eliminated the need for unnecessarily {<T>List}.
			Now you must initialise static variables with {<T>ListClear}.
			Eliminated {LIST_STATIC_SORT}.
 @since		2017-12 Type information on backing.
			2017-10 Anonymous orders.
			2017-07 Made migrate simpler.
			2017-06 Split Add into Push and Unshift.
			2017-05 Separated from backing.
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



#include <stddef.h>	/* ptrdiff_t offset_of */
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
#error List: LIST_COMPARATOR can only be anonymous; use LIST_U[A-D]_COMPARATOR.
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
 LIST_NAMEX, PT_(X) with LIST_U_NAME_X, and T_NAME with the string
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
#ifdef PT_
#undef PT_
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
#define PT_(thing) PCAT(list, PCAT(LIST_NAME, thing)) /* {private <T>} */
#define T_NAME QUOTE(LIST_NAME)

/* Troubles with this line? check to ensure that LIST_TYPE is a valid type,
 whose definition is placed above {#include "List.h"}. */
typedef LIST_TYPE PT_(Type);
#define T PT_(Type)

/* [A, D] */
#ifdef UA_
#undef UA_
#undef T_UA_
#undef PT_UA_
#endif
#ifdef UB_
#undef UB_
#undef T_UB_
#undef PT_UB_
#endif
#ifdef UC_
#undef UC_
#undef T_UC_
#undef PT_UC_
#endif
#ifdef UD_
#undef UD_
#undef T_UD_
#undef PT_UD_
#endif

/* Data exclusively, public f'ns, and private f'ns. */
#ifdef LIST_U_ANONYMOUS /* <-- anon: "Empty macro arguments were standardized
 in C99," workaround. */
#define UA_(thing) PCAT(anonymous, thing)
#define T_UA_(thing1, thing2) CAT(CAT(LIST_NAME, thing1), thing2)
#define PT_UA_(thing1, thing2) PCAT(list, PCAT(PCAT(LIST_NAME, thing1), \
	CAT(_, thing2)))
#else /* anon --><-- !anon */
#ifdef LIST_UA_NAME
#define UA_(thing) PCAT(LIST_UA_NAME, thing)
#define T_UA_(thing1, thing2) CAT(CAT(LIST_NAME, thing1), \
	CAT(LIST_UA_NAME, thing2))
#define PT_UA_(thing1, thing2) PCAT(list, PCAT(PCAT(LIST_NAME, thing1), \
	PCAT(LIST_UA_NAME, thing2)))
#endif
#ifdef LIST_UB_NAME
#define UB_(thing) PCAT(LIST_UB_NAME, thing)
#define T_UB_(thing1, thing2) CAT(CAT(LIST_NAME, thing1), \
	CAT(LIST_UB_NAME, thing2))
#define PT_UB_(thing1, thing2) PCAT(list, PCAT(PCAT(LIST_NAME, thing1), \
	PCAT(LIST_UB_NAME, thing2)))
#endif
#ifdef LIST_UC_NAME
#define UC_(thing) PCAT(LIST_UC_NAME, thing)
#define T_UC_(thing1, thing2) CAT(CAT(LIST_NAME, thing1), \
	CAT(LIST_UC_NAME, thing2))
#define PT_UC_(thing1, thing2) PCAT(list, PCAT(PCAT(LIST_NAME, thing1), \
	PCAT(LIST_UC_NAME, thing2)))
#endif
#ifdef LIST_UD_NAME
#define UD_(thing) PCAT(LIST_UD_NAME, thing)
#define T_UD_(thing1, thing2) CAT(CAT(LIST_NAME, thing1), \
	CAT(LIST_UD_NAME, thing2))
#define PT_UD_(thing1, thing2) PCAT(list, PCAT(PCAT(LIST_NAME, thing1), \
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
#endif /* LIST_H */

/* Also left in the same translation unit. */
#ifndef MIGRATE /* <-- migrate */
#define MIGRATE
/** Contains information about a {realloc}. */
struct Migrate;
struct Migrate {
	const void *begin, *end; /* Old pointers. */
	ptrdiff_t delta;
};
#endif /* migrate --> */



/* List position. */
struct PT_(X) {
#ifdef LIST_UA_NAME
	struct PT_(X) *UA_(prev), *UA_(next);
#endif
#ifdef LIST_UB_NAME
	struct PT_(X) *UB_(prev), *UB_(next);
#endif
#ifdef LIST_UC_NAME
	struct PT_(X) *UC_(prev), *UC_(next);
#endif
#ifdef LIST_UD_NAME
	struct PT_(X) *UD_(prev), *UD_(next);
#endif
};
/** A single link in the linked-list derived from {<T>}. Storage of this
 structure is the responsibility of the caller. The {<T>} is stored in the
 element {data}. */
struct T_(ListNode);
struct T_(ListNode) {
	T data; /* Must be first. */
	struct PT_(X) x;
};

/** Serves as an a head for linked-list(s) of {<T>ListNode}. Use
 \see{<T>ListClear} to initialise. */
struct T_(List);
struct T_(List) {
	/* The reason there's two instead of one is because we rely on null values
	 to determine the ends of the list. This allows easy differentiation. */
	struct PT_(X) first, last;
};



/** Calls \see{<T>ListMigratePointer}, given to \see{<T>List<U>MigrateEach}, in
 the handler for the {Migrate}. */
typedef void (*T_(ListMigrateElement))(T *const element,
	const struct Migrate *const migrate);

/** Takes {<T>}. */
typedef void (*T_(Action))(T *const);

/** Takes {<T>} and <void *>. */
typedef void (*T_(BiAction))(T *const, void *const);

/** Takes {<T>List}. */
typedef void (*T_(ListAction))(struct T_(List) *const);

/** Takes {<T>List} and {<T>}. */
typedef void (*T_(ListItemAction))(struct T_(List) *const, T *const);

/** Takes two {<T>List}. */
typedef void (*T_(BiListAction))(struct T_(List) *const,
	struct T_(List) *const);

/** Takes three {<T>List}s. */
typedef void (*T_(TriListAction))(struct T_(List) *const,
	struct T_(List) *const, struct T_(List) *const);

/** Takes {<T>} and returns {<T>}. */
typedef T *(*T_(UnaryOperator))(T *const);

/** Takes {<T>}, returns (non-zero) true or (zero) false. */
typedef int (*T_(Predicate))(const T *const);

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
static const T_(ToString) PT_(to_string) = (LIST_TO_STRING);

#endif /* string --> */



/** {container_of}. */
static struct T_(ListNode) *PT_(node_hold_x)(struct PT_(X) *const x) {
	return (struct T_(ListNode) *)(void *)
		((char *)x - offsetof(struct T_(ListNode), x));
}

/** {container_of}; just cast. */
static struct T_(ListNode) *PT_(node_hold_data)(T *const data) {
	return (struct T_(ListNode) *)(void *)data;
}

/* Prototypes: needed for the next section, but undefined until later. */
static void PT_(push)(struct T_(List) *const this, struct PT_(X) *const x);
static void PT_(remove)(struct PT_(X) *const x);
static void PT_(migrate)(const struct Migrate *const migrate,
	struct PT_(X) **const x_ptr);

/* Note to future self: recursive includes. The {LIST_U_NAME} pre-processor
 flag controls this behaviour; we are currently in the {!LIST_U_NAME} section.
 These will get all the functions with {<U>} in them from below. */

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



/** Private: add after. */
static void PT_(add_after)(struct PT_(X) *const anchor,
	struct T_(ListNode) *const node) {
	assert(anchor && node && anchor != &node->x);
#ifdef LIST_UA_NAME /* <-- a */
	PT_UA_(list, add_after)(anchor, node);
#endif /* a --> */
#ifdef LIST_UB_NAME /* <-- b */
	PT_UB_(list, add_after)(anchor, node);
#endif /* b --> */
#ifdef LIST_UC_NAME /* <-- c */
	PT_UC_(list, add_after)(anchor, node);
#endif /* c --> */
#ifdef LIST_UD_NAME /* <-- d */
	PT_UD_(list, add_after)(anchor, node);
#endif /* d --> */
}

/** Private: add before. */
static void PT_(add_before)(struct PT_(X) *const anchor,
	struct PT_(X) *const x) {
	assert(anchor && x && anchor != x);
#ifdef LIST_UA_NAME /* <-- a */
	PT_UA_(list, add_before)(anchor, x);
#endif /* a --> */
#ifdef LIST_UB_NAME /* <-- b */
	PT_UB_(list, add_before)(anchor, x);
#endif /* b --> */
#ifdef LIST_UC_NAME /* <-- c */
	PT_UC_(list, add_before)(anchor, x);
#endif /* c --> */
#ifdef LIST_UD_NAME /* <-- d */
	PT_UD_(list, add_before)(anchor, x);
#endif /* d --> */
}

/** Private: push to the end of the list.
 @implements <T>ListNodeAction */
static void PT_(push)(struct T_(List) *const this,
	struct PT_(X) *const x) {
	assert(this && x);
	PT_(add_before)(&this->last, x);
}

/** Private: unshift to the beginning of the list.
 @implements <T>ListNodeAction */
static void PT_(unshift)(struct T_(List) *const this,
	struct T_(ListNode) *const node) {
	assert(this && node);
	PT_(add_after)(&this->first, node);
}

/** Private: remove from list.
 @implements <T>ListNodeAction */
static void PT_(remove)(struct PT_(X) *const x) {
#ifdef LIST_UA_NAME /* <-- a */
	PT_UA_(list, remove)(x);
#endif /* a --> */
#ifdef LIST_UB_NAME /* <-- b */
	PT_UB_(list, remove)(x);
#endif /* b --> */
#ifdef LIST_UC_NAME /* <-- c */
	PT_UC_(list, remove)(x);
#endif /* c --> */
#ifdef LIST_UD_NAME /* <-- d */
	PT_UD_(list, remove)(x);
#endif /* d --> */
}

/** Private: clears and initialises.
 @implements <T>ListAction */
static void PT_(clear)(struct T_(List) *const this) {
	assert(this);
#ifdef LIST_UA_NAME
	this->first.UA_(prev) = this->last.UA_(next) = 0;
	this->first.UA_(next) = &this->last;
	this->last.UA_(prev) = &this->first;
#endif
#ifdef LIST_UB_NAME
	this->first.UB_(prev) = this->last.UB_(next) = 0;
	this->first.UB_(next) = &this->last;
	this->last.UB_(prev) = &this->first;
#endif
#ifdef LIST_UC_NAME
	this->first.UC_(prev) = this->last.UC_(next) = 0;
	this->first.UC_(next) = &this->last;
	this->last.UC_(prev) = &this->first;
#endif
#ifdef LIST_UD_NAME
	this->first.UD_(prev) = this->last.UD_(next) = 0;
	this->first.UD_(next) = &this->last;
	this->last.UD_(prev) = &this->first;
#endif
}

/** Clears all values from {this}, thereby initialising the {<T>List}. If it
 contained a list, those values are free.
 @implements <T>ListAction
 @order \Theta(1)
 @allow */
static void T_(ListClear)(struct T_(List) *const this) {
	if(!this) return;
	PT_(clear)(this);
}

/** Initialises the contents of {node} to add it to the end of {this}. If
 either {this} or {node} is null, it does nothing.
 @param node: Must be a {<T>ListNode} with an internal {<T>} not associated to
 any list; this associates the {<T>ListNode} with the list until it is removed,
 see \see{<T>ListRemove} or \see{<T>ListClear}.
 @implements <T>ListNodeAction
 @order \Theta(1)
 @allow */
static void T_(ListPush)(struct T_(List) *const this, T *const data) {
	if(!this || !data) return;
	PT_(push)(this, &PT_(node_hold_data)(data)->x);
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
static void T_(ListUnshift)(struct T_(List) *const this, T *const data) {
	if(!this || !data) return;
	PT_(unshift)(this, PT_(node_hold_data)(data));
}

/** Given an item from the list . . . */

/** Removes {data} from the list. The {data} is now free to add to another
 list. Removing an element that was not added to a list results in undefined
 behaviour. If {data} is null, it does nothing.
 @implements <T>ListItemAction
 @order \Theta(1)
 @allow */
static void T_(ListRemove)(T *const data) {
	if(!data) return;
	PT_(remove)(&PT_(node_hold_data)(data)->x);
}

/** Appends the elements of {from} onto {this}. If {this} is null, then it
 removes elements. Unlike \see{<T>List<U>TakeIf} and all other selective
 choosing functions, this function preserves two or more orders.
 @implements <T>BiListAction
 @order \Theta(1)
 @allow */
static void T_(ListTake)(struct T_(List) *const this,
	struct T_(List) *const from) {
	if(!from || from == this) return;
	if(!this) { PT_(clear)(from); return; }
#ifdef LIST_UA_NAME /* <-- a */
	PT_UA_(list, cat)(&this->last, from);
#endif /* a --> */
#ifdef LIST_UB_NAME /* <-- b */
	PT_UB_(list, cat)(&this->last, from);
#endif /* b --> */
#ifdef LIST_UC_NAME /* <-- c */
	PT_UC_(list, cat)(&this->last, from);
#endif /* c --> */
#ifdef LIST_UD_NAME /* <-- d */
	PT_UD_(list, cat)(&this->last, from);
#endif /* d --> */
}

/** Appends the elements from {from} before {element}. If {element} or {from}
 is null, doesn't do anything. The {from} list will be empty at the end.
 @order \Theta(1)
 @fixme Untested.
 @allow */
static void T_(ListNodeTake)(struct T_(ListNode) *const element,
	struct T_(List) *const from) {
	if(!element || !from) return;
#ifdef LIST_UA_NAME /* <-- a */
	PT_UA_(list, cat)(&element->x, from);
#endif /* a --> */
#ifdef LIST_UB_NAME /* <-- b */
	PT_UB_(list, cat)(&element->x, from);
#endif /* b --> */
#ifdef LIST_UC_NAME /* <-- c */
	PT_UC_(list, cat)(&element->x, from);
#endif /* c --> */
#ifdef LIST_UD_NAME /* <-- d */
	PT_UD_(list, cat)(&element->x, from);
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
	if(!this) { PT_(clear)(from); return; }
#ifdef LIST_OPENMP /* <-- omp */
#pragma omp parallel sections
#endif /* omp --> */
	{
#ifdef LIST_UA_NAME /* <-- a */
#ifdef LIST_UA_COMPARATOR /* <-- comp */
#ifdef LIST_OPENMP /* <-- omp */
#pragma omp section
#endif /* omp --> */
		PT_UA_(list, merge)(this, from);
#else /* comp --><-- !comp */
		PT_UA_(list, cat)(this, from);
#endif /* !comp --> */
#endif /* a --> */
#ifdef LIST_UB_NAME /* <-- b */
#ifdef LIST_UB_COMPARATOR /* <-- comp */
#ifdef LIST_OPENMP /* <-- omp */
#pragma omp section
#endif /* omp --> */
		PT_UB_(list, merge)(this, from);
#else /* comp --><-- !comp */
		PT_UB_(list, cat)(this, from);
#endif /* !comp --> */
#endif /* b --> */
#ifdef LIST_UC_NAME /* <-- c */
#ifdef LIST_UC_COMPARATOR /* <-- comp */
#ifdef LIST_OPENMP /* <-- omp */
#pragma omp section
#endif /* omp --> */
		PT_UC_(list, merge)(this, from);
#else /* comp --><-- !comp */
		PT_UC_(list, cat)(this, from);
#endif /* !comp --> */
#endif /* c --> */
#ifdef LIST_UD_NAME /* <-- d */
#ifdef LIST_UD_COMPARATOR /* <-- comp */
#ifdef LIST_OPENMP /* <-- omp */
#pragma omp section
#endif /* omp --> */
		PT_UD_(list, merge)(this, from);
#else /* comp --><-- !comp */
		PT_UD_(list, cat)(this, from);
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
		PT_UA_(natural, sort)(this);
#endif /* a --> */
#ifdef LIST_UB_COMPARATOR /* <-- b */
#ifdef LIST_OPENMP /* <-- omp */
		#pragma omp section
#endif /* omp --> */
		PT_UB_(natural, sort)(this);
#endif /* b --> */
#ifdef LIST_UC_COMPARATOR /* <-- c */
#ifdef LIST_OPENMP /* <-- omp */
		#pragma omp section
#endif /* omp --> */
		PT_UC_(natural, sort)(this);
#endif /* c --> */
#ifdef LIST_UD_COMPARATOR /* <-- d */
#ifdef LIST_OPENMP /* <-- omp */
		#pragma omp section
#endif /* omp --> */
		PT_UD_(natural, sort)(this);
#endif /* d --> */
	}
}
#endif /* !anon --> */

#endif /* comp --> */


/** Adjusts the pointers internal to the {<T>List} when supplied with a
 {Migrate} parameter, when {this} contains {<T>ListNode} elements from memory
 that switched due to a {realloc}. If {this} or {migrate} is null, doesn't do
 anything.
 @param migrate: Should only be called in a {Migrate} function; pass the
 {migrate} parameter.
 @implements <T>Migrate
 @order \Theta(n)
 @fixme Relies on not-strictly-defined behaviour because pointers are not
 necessarily contiguous in memory; it should be fine in practice.
 @allow */
static void T_(ListMigrate)(struct T_(List) *const this,
	const struct Migrate *const migrate) {
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
		PT_UA_(list, migrate)(this, migrate);
#endif /* a --> */
#ifdef LIST_UB_NAME /* <-- b */
#ifdef LIST_OPENMP /* <-- omp */
		#pragma omp section
#endif /* omp --> */
		PT_UB_(list, migrate)(this, migrate);
#endif /* b --> */
#ifdef LIST_UC_NAME /* <-- c */
#ifdef LIST_OPENMP /* <-- omp */
		#pragma omp section
#endif /* omp --> */
		PT_UC_(list, migrate)(this, migrate);
#endif /* c --> */
#ifdef LIST_UD_NAME /* <-- d */
#ifdef LIST_OPENMP /* <-- omp */
		#pragma omp section
#endif /* omp --> */
		PT_UD_(list, migrate)(this, migrate);
#endif /* d --> */
	}
}

/** Private: used in \see{<T>_list_<U>_migrate} and \see{<T>ListMigratePointer}.
 \${ptr \in [begin, end) -> ptr += delta}. */
static void PT_(migrate)(const struct Migrate *const migrate,
	struct PT_(X) **const x_ptr) {
	const void *const x = *x_ptr;
	if(x < migrate->begin || x >= migrate->end) return;
	*(char **)x_ptr += migrate->delta;
}

/** Use this inside the function that is passed to \see{<T>List<U>MigrateEach}
 to fix reallocated pointers. It doesn't affect pointers not in the {realloc}ed
 region. To update the underlying list, see \see{<T>ListMigrate}.
 @fixme Untested.
 @allow */
static void T_(MigratePointer)(T **const t_ptr,
	const struct Migrate *const migrate) {
	if(!migrate || !t_ptr || !*t_ptr) return;
	/* X is (mostly) how we pass it; just getting turned into a {char *}. */
	PT_(migrate)(migrate, (struct PT_(X) **const)t_ptr);
}

#ifdef LIST_TEST /* <-- test */
#include "../test/TestList.h" /* need this file if one is going to run tests */
#endif /* test --> */

/* prototype */
static void PT_(unused_coda)(void);
/** This silences unused function warnings from the pre-processor, but allows
 optimisation, (hopefully.)
 \url{ http://stackoverflow.com/questions/43841780/silencing-unused-static-function-warnings-for-a-section-of-code } */
static void PT_(unused_list)(void) {
	T_(ListClear)(0);
	T_(ListPush)(0, 0);
	T_(ListUnshift)(0, 0);
	T_(ListRemove)(0);
	T_(ListTake)(0, 0);
	T_(ListNodeTake)(0, 0);
#ifdef LIST_SOME_COMPARATOR /* <-- comp */
	T_(ListMerge)(0, 0);
	T_(ListSort)(0);
#endif /* comp --> */
	T_(ListMigrate)(0, 0);
	T_(MigratePointer)(0, 0);
	PT_(unused_coda)();
}
/** {clang}'s pre-processor is not fooled if one has one function. */
static void PT_(unused_coda)(void) { PT_(unused_list)(); }





/* un-define all macros */
#undef LIST_NAME
#undef LIST_TYPE
#undef CAT
#undef CAT_
#undef PCAT
#undef PCAT_
#undef T
#undef T_
#undef PT_
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
 LIST_NAMEXLIST_U_NAMEY, PT_U_(X, Y) with
 list_LIST_U_NAME_X_LIST_U_NAME_Y */
#ifdef T_U_
#undef T_U_
#endif
#ifdef PT_U_
#undef PT_U_
#endif
#ifdef U_
#undef U_
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



/** "Floyd's" tortoise-hare algorithm for cycle detection when in debug mode.
 You do not want cycles! */
static void PT_U_(cycle, crash)(const struct PT_(X) *const x) {
#ifdef LIST_DEBUG
	struct PT_(X) *turtle, *hare;
	assert(this);
	for(turtle = x; turtle->U_(prev); turtle = turtle->U_(prev));
	for(hare = turtle->U_(next); (turtle = turtle->U_(next),
		hare = hare->U_(next)) && (hare = hare->U_(next)); ) {
		assert(turtle != hare);
	}
#else
	UNUSED(x);
#endif
}

/** Private: add {node} after {anchor}. */
static void PT_U_(list, add_after)(struct PT_(X) *const anchor, struct T_(ListNode) *const node) {
	assert(anchor && node && anchor != &node->x && anchor->U_(next));
	node->x.U_(prev) = anchor;
	node->x.U_(next) = anchor->U_(next);
	anchor->U_(next)->U_(prev) = &node->x;
	anchor->U_(next) = &node->x;
	PT_U_(cycle, crash)(&node->x);
}

/** Private: add {node} before {anchor}. */
static void PT_U_(list, add_before)(struct PT_(X) *const anchor,
	struct PT_(X) *const x) {
	assert(anchor && x && anchor != x && anchor->U_(prev));
	x->U_(prev) = anchor->U_(prev);
	x->U_(next) = anchor;
	anchor->U_(prev)->U_(next) = x;
	anchor->U_(prev) = x;
	PT_U_(cycle, crash)(x);
}

/** Private: list remove in {<U>}.
 @implements <T>ListNodeAction */
static void PT_U_(list, remove)(struct PT_(X) *const x) {
	assert(x->U_(prev) && x->U_(next));
	x->U_(prev)->U_(next) = x->U_(next);
	x->U_(next)->U_(prev) = x->U_(prev);
	x->U_(prev) = x->U_(next) = 0; /* Just to be clean. */
}

/** Private: cats all {from} in front of {x}, (don't cat {first}, instead
 {first->next}); {from} will be empty after. Careful that {x} is not in {from}
 because that will just erase the list.
 @implements <T>BiListAction
 @order \Theta(1) */
static void PT_U_(list, cat)(struct PT_(X) *const x,
	struct T_(List) *const from) {
	assert(x && from && x->U_(prev) &&
		!from->first.U_(prev) && from->first.U_(next)
		&& from->last.U_(prev) && !from->last.U_(next));
	from->first.U_(next)->U_(prev) = x->U_(prev);
	x->U_(prev)->U_(next) = from->first.U_(next);
	from->last.U_(prev)->U_(next) = x;
	x->U_(prev) = from->last.U_(prev);
	from->first.U_(next) = &from->last;
	from->last.U_(prev) = &from->first;
	PT_U_(cycle, crash)(x);
	PT_U_(cycle, crash)(&from->first);
}

/** Private: callback when {realloc} changes pointers. Tried to keep undefined
 behaviour to a minimum.
 @order \Theta(n) */
static void PT_U_(list, migrate)(struct T_(List) *const this,
	const struct Migrate *const migrate) {
	struct PT_(X) *x;
	assert(this && migrate && migrate->begin && migrate->begin < migrate->end
		&& migrate->delta && this);
	for(x = &this->first; x; x = x->U_(next)) {
		PT_(migrate)(migrate, &x->U_(prev));
		PT_(migrate)(migrate, &x->U_(next));
	}
	PT_U_(cycle, crash)(&this->first);
}



/** Calls {handler} on every element that is part of the list. This allows
 {<T>} elements in the list to contain pointers to moving structures due to a
 {realloc}, using the sub-types's {Migrate} function. If {this}, {handler}, or
 {migrate} is null, doesn't do anything.
 @param handler: Has the responsibility of calling \see{<T>ListMigratePointer}
 on all pointers affected by the {realloc} of this handler.
 @order \Theta(n)
 @allow */
static void T_U_(List, MigrateEach)(struct T_(List) *const this,
	const T_(ListMigrateElement) handler, const struct Migrate *const migrate) {
	struct PT_(X) *cursor, *next;
	if(!this || !handler || !migrate) return;
	for(cursor = &this->first; cursor; cursor = next) {
		next = cursor->U_(next); /* Careful for user casters. */
		handler(&PT_(node_hold_x)(cursor)->data, migrate);
	}
	PT_U_(cycle, crash)(&this->first);
}

/** @return The next element after {this} in {<U>}. When {this} is the last
 element or when {this} is null, returns null.
 @param this: Must be in a {List} as a {<T>ListNode}.
 @implements <T>UnaryOperator
 @order \Theta(1)
 @allow */
static T *T_U_(Node, GetNext)(T *const data) {
	const struct PT_(X) *const x = &PT_(node_hold_data)(data)->x;
	struct PT_(X) *next_x;
	if(!data) return 0;
	assert(x->U_(next));
	if(!(next_x = x->U_(next))->U_(next)) return 0;
	return &PT_(node_hold_x)(next_x)->data;
}

/** @return The previous element before {this} in {<U>}. When {this} is the
 first item or when {this} is null, returns null.
 @param this: Must be in a {List} as a {<T>ListNode}.
 @implements <T>UnaryOperator
 @order \Theta(1)
 @allow */
static T *T_U_(Node, GetPrevious)(T *const data) {
	const struct PT_(X) *const x = &PT_(node_hold_data)(data)->x;
	struct PT_(X) *prev_x;
	if(!data) return 0;
	assert(x->U_(prev));
	if(!(prev_x = x->U_(prev))->U_(prev)) return 0;
	return &PT_(node_hold_x)(prev_x)->data;
}

/** @return A pointer to the first element of {this}.
 @order \Theta(1)
 @allow */
static T *T_U_(List, GetFirst)(struct T_(List) *const this) {
	if(!this) return 0;
	assert(this->first.U_(next));
	if(!this->first.U_(next)->U_(next)) return 0; /* Empty. */
	return &PT_(node_hold_x)(this->first.U_(next))->data;
}

/** @return A pointer to the last element of {this}.
 @order \Theta(1)
 @allow */
static T *T_U_(List, GetLast)(struct T_(List) *const this) {
	if(!this) return 0;
	assert(this->last.U_(prev));
	if(!this->last.U_(prev)->U_(prev)) return 0; /* Empty. */
	return &PT_(node_hold_x)(this->last.U_(prev))->data;
}

#ifdef LIST_U_COMPARATOR /* <-- comp */

/* Check that each of LIST_U[A-D]_COMPARATOR are functions implementing
 {<T>Comparator}. */
static const T_(Comparator) PT_U_(data, cmp) = (LIST_U_COMPARATOR);

/** Private: merges {blist} into {alist} when we don't know anything about the
 data; on equal elements, places {alist} first.
 @implements <T>BiListAction
 @order {O(n + m)}. */
static void PT_U_(list, merge)(struct T_(List) *const alist,
	struct T_(List) *const blist) {
	struct PT_(X) *hind, *a, *b;
	assert(alist && blist);
	/* {blist} empty -- that was easy. */
	if(!(b = blist->first.U_(next))->U_(next)) return;
	/* {alist} empty -- {O(1)} cat is more efficient. */
	if(!(a = alist->first.U_(next))->U_(next))
		{ PT_U_(list, cat)(&alist->last, blist); return; }
	printf("merge\n");
	/* Merge */
	for(hind = &alist->first; ; ) {
		if(PT_U_(data, cmp)(&PT_(node_hold_x)(a)->data,
			&PT_(node_hold_x)(b)->data) < 0) {
			a->U_(prev) = hind, hind = hind->U_(next) = a;
			if(!(a = a->U_(next))->U_(next))
				{ b->U_(prev) = hind, hind->U_(next) = b,
				alist->last.U_(prev) = blist->last.U_(prev); break; }
		} else {
			b->U_(prev) = hind, hind = hind->U_(next) = b;
			if(!(b = b->U_(next))->U_(next))
				{ a->U_(prev) = hind, hind->U_(next) = a; break; }
		}
	}
	blist->first.U_(next) = &blist->last, blist->last.U_(prev) = &blist->first;
}

#ifndef LIST_SORT_INTERNALS /* <!-- sort internals only once per translation
 unit */
#define LIST_SORT_INTERNALS
/* A run is a temporary sequence of values in the array that is weakly
 increasing; we store it's size temporarily. */
struct PT_(Run) {
	struct PT_(X) *head, *tail;
	size_t size;
};
/* Store the maximum capacity for the indexing with {size_t}. (Much more then
 we need, in most cases.) \${
 range(runs) = Sum_{k=0}^runs 2^{runs-k} - 1
             = 2^{runs+1} - 2
 2^bits      = 2 (r^runs - 1)
 runs        = log(2^{bits-1} + 1) / log 2
 runs       <= 2^{bits - 1}, 2^{bits + 1} > 0} */
struct PT_(Runs) {
	struct PT_(Run) run[(sizeof(size_t) << 3) - 1];
	size_t run_no;
};
#endif /* sort internals --> */

static void PT_U_(natural, print)(const struct PT_(Run) *const run) {
	struct PT_(X) *x;
	char str[12];
	assert(run);
	x = run->head;
	assert(x);
	do {
		PT_(to_string)(&PT_(node_hold_x)(x)->data, &str);
		printf("%s, ", str);
		if(x == run->tail) break;
		x = x->U_(next);
		assert(x);
	} while(1);
	printf("(size %lu.)\n", run->size);
}

/** Inserts the first element from the larger of two sorted runs, then merges
 the rest. \cite{Peters2002Timsort}, via \cite{McIlroy1993Optimistic}, does
 long merges by galloping, but we don't have random access to the data. In
 practice, this is {2%} slower on randomly distributed keys when the
 linked-list size is over {500 000}; randomly distributed keys have high
 insertion times that to well in standard merging. However, it's (potentially
 much) faster when the keys have structure: observed, {[-2%, 500%]}. Assumes
 array contains at least 2 elements and there are at least two runs. */
static void PT_U_(natural, merge)(struct PT_(Runs) *const r) {
	struct PT_(Run) *const run_a = r->run + r->run_no - 2;
	struct PT_(Run) *const run_b = run_a + 1;
	struct PT_(X) *a = run_a->tail, *b = run_b->head, *chosen;
	assert(r->run_no >= 2);
#if defined(LIST_TEST) && defined(LIST_TO_STRING)
	PT_U_(natural, print)(run_a);
	PT_U_(natural, print)(run_b);
#endif
	/* fixme: we are doing one-to-many compares in some cases? */
	if(run_a->size <= run_b->size) {
		struct PT_(X) *prev_chosen;
		/* run a is smaller: downwards insert b.head followed by upwards
		 merge */
		/* insert the first element of b downwards into a */
		for( ; ; ) {
			if(PT_U_(data, cmp)(&PT_(node_hold_x)(a)->data,
				&PT_(node_hold_x)(b)->data) <= 0) {
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
			if(PT_U_(data, cmp)(&PT_(node_hold_x)(a)->data,
				&PT_(node_hold_x)(b)->data) > 0) {
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
		struct PT_(X) *next_chosen;
		int is_a_tail = 0;
		/* run b is smaller; upwards insert followed by downwards merge */
		/* insert the last element of a upwards into b */
		for( ; ; ) {
			if(PT_U_(data, cmp)(&PT_(node_hold_x)(a)->data,
				&PT_(node_hold_x)(b)->data) <= 0) {
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
			if(PT_U_(data, cmp)(&PT_(node_hold_x)(a)->data,
				&PT_(node_hold_x)(b)->data) > 0) {
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

#ifdef LIST_TEST /* <-- test */
#ifdef LIST_TO_STRING
static char *T_U_(List, ToString)(const struct T_(List) *const this);
#endif
static size_t PT_(count)(struct T_(List) *const this);
#endif /* test --> */

/** It's kind of experimental. It hasn't been optimised; I think it does
 useless compares. It's so beatiful.
 @implements <T>ListAction */
static void PT_U_(natural, sort)(struct T_(List) *const this) {
	/* This is potentially half-a-KB; we had an option to store as a global,
	 but that was probably overkill. */
	struct PT_(Runs) runs;
	struct PT_(Run) *new_run;
	/* Part of the state machine for classifying points wrt their neighbours. */
	enum { UNSURE, INCREASING, DECREASING } mono;
	/* The data that we are sorting. */
	struct PT_(X) *a, *b, *c, *first_iso_a;
	/* {run_count} is different from {runs.run_no} in that it only increases;
	 only used for calculating the path up the tree. */
	size_t run_count, rc;
	/* The value of the comparison. */
	int comp;
#ifdef LIST_TEST /* <-- test */
	size_t count = PT_(count)(this), count_after, num = 0;
#ifdef LIST_TO_STRING
	printf("Org: %s.\n", T_U_(List, ToString)(this));
#endif
#endif /* test --> */
	/* Needs an element. */
	a = this->first.U_(next), assert(a);
	if(!(b = a->U_(next))) return;
	/* Reset the state machine and output to just {a} in the first run. */
	mono = UNSURE;
	runs.run_no = 1;
	new_run = runs.run + 0, run_count = (size_t)1;
	new_run->size = 1;
	first_iso_a = new_run->head = new_run->tail = a;
	/* While {a} and {b} are elements (that are consecutive.) {c} may not be. */
	for(c = b->U_(next); c; a = b, b = c, c = c->U_(next)) {
#ifdef LIST_TEST /* <-- test */
		num++;
#endif /* test --> */
		comp = PT_U_(data, cmp)(&PT_(node_hold_x)(a)->data,
			&PT_(node_hold_x)(b)->data);
		/* State machine that considers runs in both directions -- in practice,
		 slightly slower than only considering increasing runs on most cases;
		 however, I would hate to see my code replaced with one line; reverse
		 order is 15 times faster, but it's not likely. */
		if(comp < 0) { /* a < b, increasing -- good */
			if(mono != DECREASING) { /* If decreasing, inflection. */
				mono = INCREASING;
				new_run->size++;
				continue;
			}
		} else if(comp > 0) { /* Decreasing; reverse preserving stability. */
			if(mono != INCREASING) { /* If increasing, inflection. */
				mono = DECREASING;
				b->U_(next) = first_iso_a;
				first_iso_a->U_(prev) = b;
				new_run->head = first_iso_a = b;
				new_run->size++;
				continue;
			}
			new_run->tail = a; /* Terminating an increasing sequence. */
		} else { /* {a} == {b} */
			if(mono == DECREASING) { /* Extend. */
				struct PT_(X) *const a_next = a->U_(next);
				b->U_(next) = a_next;
				a_next->U_(prev) = b;
				a->U_(next) = b;
				b->U_(prev) = a;
			} else { /* Monotone or weakly increasing. */
				new_run->tail = b;
			}
			new_run->size++;
			continue;
		}
		/* Head and tail don't necessarily correspond to the first and last. */
		new_run->head->U_(prev) = new_run->tail->U_(next) = 0;
		/* Greedy merge: keeps space to {O(log n)} instead of {O(n)}. */
		for(rc = run_count; !(rc & 1) && runs.run_no >= 2; rc >>= 1)
			PT_U_(natural, merge)(&runs);
		/* Reset the state machine and output to just {b} at the next run. */
		mono = UNSURE;
		assert(runs.run_no < sizeof(runs.run) / sizeof(*runs.run));
		new_run = runs.run + runs.run_no++, run_count++;
		new_run->size = 1;
		new_run->head = new_run->tail = first_iso_a = b;
	}
	/* Terminating the last increasing sequence. */
	if(mono == INCREASING) new_run->tail = a;
	new_run->tail->U_(next) = new_run->head->U_(prev) = 0;
	/* Clean up the rest; when only one run, propagate list_runs[0] to head. */
	while(runs.run_no > 1) PT_U_(natural, merge)(&runs);
	runs.run[0].head->U_(prev) = &this->first;
	runs.run[0].tail->U_(next) = &this->last;
	this->first.U_(next) = runs.run[0].head;
	this->last.U_(prev)  = runs.run[0].tail;
#ifdef LIST_TEST /* <-- test */
#ifdef LIST_TO_STRING
	printf("Now: %s.\n", T_U_(List, ToString)(this));
#endif
	count_after = PT_(count)(this);
	assert(count == count_after);
#endif /* test --> */
}

/** Sorts {<U>}, but leaves the other lists in {<T>} alone. Must have
 {LIST_U[A-D]_COMPARATOR} defined.
 @implements <T>ListAction
 @order \Omega({this}.n), O({this}.n log {this}.n)
 @allow */
static void T_U_(List, Sort)(struct T_(List) *const this) {
	if(!this) return;
	PT_U_(natural, sort)(this);
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
	struct PT_(X) *a, *b;
	int diff;
	/* null counts as -\infty */
	if(!this) {
		return that ? -1 : 0;
	} else if(!that) {
		return 1;
	}
	/* compare element by element */
	for(a = this->first.U_(next), b = that->first.U_(next); ;
		a = a->U_(next), b = b->U_(next)) {
		if(!a->U_(next)) {
			return b->U_(next) ? -1 : 0;
		} else if(!b->U_(next)) {
			return 1;
		} else if((diff = PT_U_(data, cmp)
			(&PT_(node_hold_x)(a)->data,
			&PT_(node_hold_x)(b)->data))) {
			return diff;
		}
	}
}

/** Private: {this <- a \mask b}. Prefers {a} to {b} when equal.
 @implements <T>TriListAction
 @order O({a}.n + {b}.n) */
static void PT_U_(boolean, seq)(struct T_(List) *const this,
	struct T_(List) *const alist, struct T_(List) *const blist,
	const enum ListOperation mask) {
	struct PT_(X) *a = alist ? alist->first.U_(next) : 0,
		*b = blist ? blist->first.U_(next) : 0, *temp;
	int comp;
	assert(a && b);
	while(a->U_(next) && b->U_(next)) {
		comp = PT_U_(data, cmp)(&PT_(node_hold_x)(a)->data,
			&PT_(node_hold_x)(b)->data);
		if(comp < 0) {
			temp = a, a = a->U_(next);
			if(mask & LO_SUBTRACTION_AB) {
				PT_(remove)(temp);
				if(this) PT_(push)(this, temp);
			}
		} else if(comp > 0) {
			temp = b, b = b->U_(next);
			if(mask & LO_SUBTRACTION_BA) {
				PT_(remove)(temp);
				if(this) PT_(push)(this, temp);
			}
		} else {
			temp = a, a = a->U_(next), b = b->U_(next);
			if(mask & LO_INTERSECTION) {
				PT_(remove)(temp);
				if(this) PT_(push)(this, temp);
			}
		}
	}
	if(mask & LO_DEFAULT_A) {
		while((temp = a, a = a->U_(next))) {
			PT_(remove)(temp);
			if(this) PT_(push)(this, temp);
		}
	}
	if((mask & LO_DEFAULT_B)) {
		while((temp = b, b = b->U_(next))) {
			PT_(remove)(temp);
			if(this) PT_(push)(this, temp);
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
	PT_U_(boolean, seq)(this, a, b, LO_SUBTRACTION_AB | LO_DEFAULT_A);
}

/** Appends {this} with the union of {a} and {b} as a sequence in {<U>}. Equal
 elements are moved from {a}. If {this} is null, then it removes elements. Must
 have {LIST_U[A-D]_COMPARATOR} defined.
 @implements <T>TriListAction
 @order O({a}.n + {b}.n)
 @allow */
static void T_U_(List, TakeUnion)(struct T_(List) *const this,
	struct T_(List) *const a, struct T_(List) *const b) {
	PT_U_(boolean, seq)(this, a, b, LO_SUBTRACTION_AB | LO_SUBTRACTION_BA
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
	PT_U_(boolean, seq)(this, a, b, LO_INTERSECTION);
}

/** Appends {this} with {a} exclusive-or {b} as a sequence in {<U>}. Equal
 elements are moved from {a}. If {this} is null, then it removes elements. Must
 have {LIST_U[A-D]_COMPARATOR} defined.
 @implements <T>TriListAction
 @order O({a}.n + {b}.n)
 @allow */
static void T_U_(List, TakeXor)(struct T_(List) *const this,
	struct T_(List) *const a, struct T_(List) *const b) {
	PT_U_(boolean, seq)(this, a, b, LO_SUBTRACTION_AB | LO_SUBTRACTION_BA
		| LO_DEFAULT_A | LO_DEFAULT_B);
}

#endif /* comp --> */

/** Appends {this} with {from} if {predicate} is null or true in the order
 specified by {<U>}. If {this} is null, then it removes elements.
 @order ~ \Theta({this}.n) \times O({predicate})
 @allow */
static void T_U_(List, TakeIf)(struct T_(List) *const this,
	struct T_(List) *const from, const T_(Predicate) predicate) {
	struct PT_(X) *x, *next_x;
	if(!from || from == this) return;
	for(x = from->first.U_(next); (next_x = x->U_(next)); x = next_x) {
		if(predicate && !predicate(&PT_(node_hold_x)(x)->data)) continue;
		PT_(remove)(x);
		if(this) PT_(push)(this, x);
	}
}

/** Appends {this} with {from} if {bipredicate} is null or true in the order
 specified by {<U>}. If {this} is null, then it removes elements.
 @order ~ \Theta({this}.n) \times O({predicate})
 @allow */
static void T_U_(List, BiTakeIf)(struct T_(List) *const this,
	struct T_(List) *const from, const T_(BiPredicate) bipredicate,
	void *const param) {
	struct PT_(X) *x, *next_x;
	if(!from || from == this) return;
	for(x = from->first.U_(next); (next_x = x->U_(next)); x = next_x) {
		if(bipredicate && !bipredicate(&PT_(node_hold_x)(x)->data, param))
			continue;
		PT_(remove)(x);
		if(this) PT_(push)(this, x);
	}
}

/** Performs {action} for each element in the list in the order specified by
 {<U>}. You can tranfer or delete the data, or see \see{<T>List<U>TakeIf}.
 @order ~ \Theta({this}.n) \times O({action})
 @allow */
static void T_U_(List, ForEach)(struct T_(List) *const this,
	const T_(Action) action) {
	struct PT_(X) *x, *next_x;
	if(!this || !action) return;
	for(x = this->first.U_(next); (next_x = x->U_(next)); x = next_x)
		action(&PT_(node_hold_x)(x)->data);
}

/** Performs {biaction} for each element in the list in the order specified by
 {<U>}.
 @order ~ \Theta({this}.n) \times O({action})
 @fixme Untested.
 @allow */
static void T_U_(List, BiForEach)(struct T_(List) *const this,
	const T_(BiAction) biaction, void *const param) {
	struct PT_(X) *x, *next_x;
	if(!this || !biaction) return;
	for(x = this->first.U_(next); (next_x = x->U_(next)); x = next_x)
		biaction(&PT_(node_hold_x)(x)->data, param);
}

/** @return The first {<T>} in the linked-list, ordered by {<U>}, that causes
 the {predicate} with {<T>} as argument to return false, or null if the
 {predicate} is true for every case. If {this} or {predicate} is null, returns
 null.
 @order ~ O({this}.n) \times O({predicate})
 @allow */
static T *T_U_(List, ShortCircuit)(struct T_(List) *const this,
	const T_(Predicate) predicate) {
	struct PT_(X) *x, *next_x;
	T *data;
	if(!this || !predicate) return 0;
	for(x = this->first.U_(next); (next_x = x->U_(next)); x = next_x)
		if(data = &PT_(node_hold_x)(x)->data, !predicate(data)) return data;
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
	struct PT_(X) *x, *next_x;
	T *data;
	if(!this || !bipredicate) return 0;
	for(x = this->first.U_(next); (next_x = x->U_(next)); x = next_x)
		if(data = &PT_(node_hold_x)(x)->data, !bipredicate(data, param))
			return data;
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
	struct PT_(X) *x;
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
	for(x = this->first.U_(next); x->U_(next); x = x->U_(next)) {
		if(x != this->first.U_(next)) list_super_cat(&cat, list_cat_sep);
		PT_(to_string)(&PT_(node_hold_x)(x)->data, &scratch),
			scratch[sizeof scratch - 1] = '\0';
		list_super_cat(&cat, scratch);
		if(cat.is_truncated) break;
	}
	sprintf(cat.cursor, "%s",
		cat.is_truncated ? list_cat_alter_end : list_cat_end);
	return cat.print;
}

#endif /* print --> */

/* prototype */
static void PT_U_(sub_unused, coda)(void);
/** This silences unused function warnings from the pre-processor, but allows
 optimisation, (hopefully.)
 \url{ http://stackoverflow.com/questions/43841780/silencing-unused-static-function-warnings-for-a-section-of-code } */
static void PT_U_(sub_unused, list)(void) {
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
	T_U_(List, MigrateEach)(0, 0, 0);
#ifdef LIST_TO_STRING /* <-- string */
	T_U_(List, ToString)(0);
#endif /* string --> */
	PT_U_(cycle, crash)(0);
	PT_U_(sub_unused, coda)();
}
/** {clang}'s pre-processor is not fooled. */
static void PT_U_(sub_unused, coda)(void) {
	PT_U_(sub_unused, list)();
}



/* un-define stuff for the next */
#undef LIST_U_NAME
#ifdef LIST_U_COMPARATOR /* <-- comp */
#undef LIST_U_COMPARATOR
#endif /* comp --> */

#endif /* LIST_U_NAME --> */
