/** 2016 Neil Edelman, distributed under the terms of the MIT License;
 see readme.txt, or \url{ https://opensource.org/licenses/MIT }.

 {<G>Digraph} is an abstract directed graph represented by adjancency lists
 that is backed by {<G>Vertex} and {<G>Edge}. The preprocessor macros are all
 undefined at the end of the file for convenience.

 @param DIGRAPH_NAME
 This literally becomes {<G>}. As it's used in function names, this should
 comply with naming rules and be unique; required.

 @param DIGRAPH_VERTEX, DIGRAPH_EDGE
 The optional type(s) associated with {<V>} and {<E>}. Has to be a valid type,
 accessible to the compiler at the time of inclusion.

 @param DIGRAPH_VERTEX_TO_STRING, DIGRAPH_EDGE_TO_STRING
 Optional print function(s) implementing {<G>VertexToString} and
 {<G>EdgeToString}.

 @param DIGRAPH_TEST, DIGRAPH_VERTEX_TEST, DIGRAPH_EDGE_TEST
 Unit testing framework using {<G>StackTest}, included in a separate header,
 {../test/StackTest.h}. If {DIGRAPH_VERTEX <-> DIGRAPH_VERTEX_TEST}; if
 {DIGRAPH_EDGE <-> DIGRAPH_EDGE_TEST}, which must be defined equal to a
 (random) filler function, satisfying {<G>VertexAction} and {<E>EdgeAction}.
 If {NDEBUG} is not defined, turns on {assert} private function integrity
 testing.

 @title		Digraph.h
 @std		C89
 @author	Neil
 @version	2018-04 Try. */



#include <assert.h>	/* assert */
#include <stdio.h>	/* snprintf */
#include <errno.h>	/* errno */



/* check defines */
#ifndef DIGRAPH_NAME /* <-- error */
#error Generic DIGRAPH_NAME undefined.
#endif /* error --> */
#if defined(DIGRAPH_VERTEX_TO_STRING) && !defined(DIGRAPH_VERTEX) \
	|| defined(DIGRAPH_EDGE_TO_STRING) && !defined(DIGRAPH_EDGE) /* <-- error */
#error TO_STRING without VERTEX or EDGE.
#endif /* error --> */
#ifdef DIGRAPH_TEST /* <-- test */
#if defined(DIGRAPH_VERTEX) && !defined(DIGRAPH_VERTEX_TEST) || \
	defined(DIGRAPH_EDGE)   && !defined(DIGRAPH_EDGE_TEST) /* <-- error */
#error DIGRAPH_TEST requires DIGRAPH_VERTEX_TEST and/or DIGRAPH_EDGE_TEST.
#endif /* error --> */
#else /* test--><-- !test */
#if defined(DIGRAPH_VERTEX_TEST) || defined(DIGRAPH_EDGE_TEST) /* <-- error */
#error DIGRAPH_VERTEX_TEST and DIGRAPH_EDGE_TEST require DIGRAPH_TEST.
#endif /* error --> */
#ifndef NDEBUG /* <-- ndebug */
#define DIGRAPH_NDEBUG
#define NDEBUG
#endif /* ndebug --> */
#endif /* !test --> */



/* Generics using the preprocessor;
 \url{ http://stackoverflow.com/questions/16522341/pseudo-generics-in-c }. */
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
#ifdef V
#undef V
#endif
#ifdef E
#undef E
#endif
#ifdef G_
#undef G_
#endif
#ifdef PG_
#undef PG_
#endif
#define CAT_(x, y) x ## y
#define CAT(x, y) CAT_(x, y)
#define PCAT_(x, y) x ## _ ## y
#define PCAT(x, y) PCAT_(x, y)
#define G_(thing) CAT(DIGRAPH_NAME, thing)
#define PG_(thing) PCAT(digraph, PCAT(DIGRAPH_NAME, thing))



/* Troubles with this line? check to ensure that {DIGRAPH_VERTEX} is a valid
 type, whose definition is placed above {#include "Digraph.h"}. */
#ifdef DIGRAPH_VERTEX /* <-- vertex */
typedef DIGRAPH_VERTEX PG_(Vertex);
#define V PG_(Vertex)
#endif /* vertex --> */

/* Troubles with this line? check to ensure that {DIGRAPH_EDGE} is a valid
 type, whose definition is placed above {#include "Digraph.h"}. */
#ifdef DIGRAPH_EDGE /* <-- edge */
typedef DIGRAPH_EDGE PG_(Edge);
#define E PG_(Edge)
#endif /* egde --> */



/** Edge. */
struct G_(Edge);
struct G_(Vertex);
struct G_(Edge) {
#ifdef DIGRAPH_EDGE /* <-- edge */
	E info;
#endif /* edge --> */
	struct G_(Vertex) *to;
};

/* This relies on {List.h} which must be in the same directory. */
#define LIST_NAME G_(Edge)
#define LIST_TYPE struct G_(Edge)
/*#define LIST_TO_STRING &*/
#define LIST_SUBTYPE
#include "List.h" /* Defines {<G>EdgeList} and {<G>EdgeListNode}. */

/** Vertex. */
struct G_(Vertex);
struct G_(Vertex) {
#ifdef DIGRAPH_VERTEX /* <-- vertex */
	V info;
#endif /* vertex --> */
	struct G_(EdgeList) out;
};

/* This relies on {List.h} which must be in the same directory. */
#define LIST_NAME G_(Vertex)
#define LIST_TYPE struct G_(Vertex)
/*#define LIST_TO_STRING &*/
#define LIST_SUBTYPE
#include "List.h" /* Defines {<G>VertexList} and {<G>VertexListNode}. */

/** The directed graph. To instantiate, see \see{<V>Digraph}. */
struct G_(Digraph);
struct G_(Digraph) {
	struct G_(VertexList) vertices;
};



#ifdef DIGRAPH_VERTEX_TO_STRING /* <-- v2string */
/** Responsible for turning {<V>} (the first argument) into a 12 {char}
 null-terminated output string (the second.) */
typedef void (*PG_(VertexToString))(const V *, char (*const)[12]);
/* Check that {DIGRAPH_VERTEX_TO_STRING} is a function implementing
 {<PVE>VertexToString}. */
static const PG_(VertexToString) PG_(v_to_string) =(DIGRAPH_VERTEX_TO_STRING);
#endif /* v2string --> */

#ifdef DIGRAPH_EDGE_TO_STRING /* <-- e2string */
/** Responsible for turning {<E>} (the first argument) into a 12 {char}
 null-terminated output string (the second.) */
typedef void (*PG_(EdgeToString))(const E *, char (*const)[12]);
/* Check that {DIGRAPH_EDGE_TO_STRING} is a function implementing
 {<PVE>EdgeToString}. */
static const PG_(EdgeToString) PG_(e_to_string) = (DIGRAPH_EDGE_TO_STRING);
#endif /* e2string --> */

#ifdef DIGRAPH_TEST /* <-- test */
#ifdef DIGRAPH_VERTEX /* <-- vertex */
/** Performs an action on a vertex-associated {<V>}. */
typedef void (*PG_(VertexAction))(V *const);
#endif /* vertex --> */
#ifdef DIGRAPH_EDGE /* <-- edge */
/** Performs an action on an edge-associated {<E>}. */
typedef void (*PG_(EdgeAction))(E *const);
#endif /* edge --> */
#endif /* test -->*/



/** Called in \see{<G>Digraph_}.
 @implements <G>VertexAction */
static void PG_(clear)(struct G_(Vertex) *const v) {
	assert(v);
	G_(EdgeList_)(&v->out);
}

/** Destructor for {g}.
 @param g: If null or empty, does nothing.
 @order O({vertices})
 @allow */
static void G_(Digraph_)(struct G_(Digraph) *const g) {
	if(!g) return;
	G_(VertexForEach)(&g->vertices, &PG_(clear));
	G_(Vertex_)(&g->vertices);
}

/** Initialises {g} to an empty {Digraph}.
 @param g: If null, does nothing.
 @order \Theta(1)
 @allow */
static void G_(Digraph)(struct G_(Digraph) *const g) {
	if(!g) return;
	G_(Vertex)(&g->vertices);
}

#if 0 /* <-- 0 */

/** Provides a way to iterate through the g.
 @param g: If null, returns null.
 @param prev: Set it to null to start the iteration.
 @return A pointer to the next element or null if there are no more. If you add
 to the g, the pointer becomes invalid.
 @order \Theta(1)
 @allow */
static G_(Vertex) *G_(StackNext)(struct G_(Digraph) *const g, V *const prev) {
	if(!g || !g->size) return 0;
	if(!prev) return g->array;
	if((size_t)(prev - g->array) + 1 >= g->size) return 0;
	return prev + 1;
}

/** Gets an uninitialised new element at the end of the {Stack}. May move the
 {Stack} to a new memory location to fit the new size.
 @param g: If {g} is null, returns null.
 @return A new, un-initialised, element, or null and {errno} may be set.
 @throws ERANGE: Tried allocating more then can fit in {size_t} objects.
 @throws {realloc} errors: {IEEE Std 1003.1-2001}.
 @order amortised O(1)
 @allow */
static V *G_(StackNew)(struct G_(Digraph) *const g) {
	V *elem;
	if(!g) return 0;
	if(sizeof(V) == 1 && g->size == (size_t)-1) { errno = ERANGE; return 0;}
	if(!PG_(reserve)(g, g->size + 1, 0)) return 0; /* ERANGE, ENOMEM? */
	elem = g->array + g->size++;
	PG_(debug)(g, "New", "added.\n");
	return elem;
}

/** Gets an uninitialised new element and updates the {update_ptr} if it is
 within the memory region that was changed.
 @param g: If {g} is null, returns null.
 @param update_ptr: Pointer to update on memory move.
 @return A new, un-initialised, element, or null and {errno} may be set.
 @throws ERANGE: Tried allocating more then can fit in {size_t}.
 @throws {realloc} errors: {IEEE Std 1003.1-2001}.
 @order amortised O(1)
 @fixme Untested.
 @allow */
static V *G_(StackUpdateNew)(struct G_(Digraph) *const g,
	V **const update_ptr) {
	if(!g) return 0;
	if(!PG_(reserve)(g, g->size + 1, update_ptr))
		return 0; /* ERANGE, ENOMEM? */
	PG_(debug)(g, "New", "added.\n");
	return g->array + g->size++;
}

/** Removes all data from {g}. Leaves the g memory alone; if one wants
 to remove memory, see \see{Stack_}.
 @param g: If null, does nothing.
 @order \Theta(1)
 @allow */
static void G_(StackClear)(struct G_(Digraph) *const g) {
	if(!g) return;
	g->size = 0;
	PG_(debug)(g, "Clear", "cleared.\n");
}

/** Iterates though the {Stack} from the bottom and calls {action} on all the
 elements.
 @param g, action: If null, does nothing.
 @order O({size} \times {action})
 @fixme Untested.
 @fixme Sequence interface.
 @allow */
static void G_(StackForEach)(struct G_(Digraph) *const g,
	const PG_(Action) action) {
	V *a, *end;
	if(!g || !action) return;
	for(a = g->array, end = a + g->size; a < end; a++) action(a);
}

#endif /* 0 --> */

/** Appends {g} to {fp} in GraphViz format.
 @param g: If null, does nothing.
 @param fp: File pointer.
 @return Success.
 @order O(|{vertices}| + |{edges}|)
 @allow */
static int G_(DigraphOutput)(const struct G_(Digraph) *const g,
	FILE *const fp) {
	char scratch[12];
	return 0;
}

#ifdef DIGRAPH_TEST /* <-- test */
#include "../test/TestDigraph.h" /* Need this file if running tests. */
#endif /* test --> */

/* Prototype. */
static void PG_(unused_coda)(void);
/** This silences unused function warnings from the pre-processor, but allows
 optimisation, (hopefully.)
 \url{ http://stackoverflow.com/questions/43841780/silencing-unused-static-function-warnings-for-a-section-of-code } */
static void PG_(unused)(void) {
	G_(Digraph_)(0);
	G_(Digraph)(0);
	G_(DigraphOutput)(0, 0);
	PG_(unused_coda)();
}
/** {clang}'s pre-processor is not fooled if you have one function. */
static void PG_(unused_coda)(void) { PG_(unused)(); }



/* Un-define all macros. */
/* Undocumented; allows nestled inclusion so long as: {CAT_}, {CAT}, {PCAT},
 {PCAT_} conform, and {G}, {V}, {E}, {T}, {U}, {S}, and {A}, are not used. */
#ifdef DIGRAPH_SUBTYPE /* <-- sub */
#undef DIGRAPH_SUBTYPE
#else /* sub --><-- !sub */
#undef CAT
#undef CAT_
#undef PCAT
#undef PCAT_
#endif /* !sub --> */
#undef G_
#undef PG_
#ifdef V
#undef V
#endif
#ifdef E
#undef E
#endif
#undef DIGRAPH_NAME
#ifdef DIGRAPH_VERTEX
#undef DIGRAPH_VERTEX
#endif
#ifdef DIGRAPH_EDGE
#undef DIGRAPH_EDGE
#endif
#ifdef DIGRAPH_VERTEX_TO_STRING
#undef DIGRAPH_VERTEX_TO_STRING
#endif
#ifdef DIGRAPH_EDGE_TO_STRING
#undef DIGRAPH_EDGE_TO_STRING
#endif
#ifdef DIGRAPH_TEST
#undef DIGRAPH_TEST
#endif
#ifdef DIGRAPH_VERTEX_TEST
#undef DIGRAPH_VERTEX_TEST
#endif
#ifdef DIGRAPH_EDGE_TEST
#undef DIGRAPH_EDGE_TEST
#endif
#ifdef DIGRAPH_NDEBUG
#undef DIGRAPH_NDEBUG
#undef NDEBUG
#endif
