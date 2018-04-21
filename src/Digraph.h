/** 2016 Neil Edelman, distributed under the terms of the MIT License;
 see readme.txt, or \url{ https://opensource.org/licenses/MIT }.

 {<G>Digraph} is an abstract directed graph represented by adjancency lists
 that is backed by {<G>Vertex} and {<G>Edge}. It does very little except expose
 the data types. The preprocessor macros are all undefined at the end of the
 file for convenience.

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
 @version	2018-04 This is cool. */



#include <stdio.h>  /* FILE for {<G>DigraphOut} */
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
#define G_(thing) CAT(DIGRAPH_NAME, thing)
#define PG_(thing) PCAT(digraph, PCAT(DIGRAPH_NAME, thing))
#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)



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



/** Edge. */
struct G_(Edge);
struct G_(Vertex);
struct G_(Edge) {
#ifdef DIGRAPH_EDGE /* <-- edge */
	E info;
#endif /* edge --> */
	struct G_(Vertex) *to;
};

/** @implements <<G>Edge>ToString */
static void PG_(edge_to_string)(const struct G_(Edge) *const e,
	char (*const a)[12]) {
#ifdef DIGRAPH_EDGE /* <-- edge */
	PG_(e_to_string)(&e->info, a);
#else /* edge --><-- !edge */
	strcpy(*a, "edge");
	(void)e;
#endif /* !edge --> */
}

/* This relies on {List.h} which must be in the same directory. */
#define LIST_NAME G_(Edge)
#define LIST_TYPE struct G_(Edge)
#define LIST_SUBTYPE
#define LIST_TO_STRING &PG_(edge_to_string)
#include "List.h" /* Defines {<G>EdgeList} and {<G>EdgeListNode}. */

/** Vertex. */
struct G_(Vertex);
struct G_(Vertex) {
#ifdef DIGRAPH_VERTEX /* <-- vertex */
	V info;
#endif /* vertex --> */
	struct G_(EdgeList) out;
};

static void PG_(vertex_to_string)(const struct G_(Vertex) *const v,
	char (*const a)[12]) {
#ifdef DIGRAPH_VERTEX /* <-- vertex */
	PG_(v_to_string)(&v->info, a);
#else /* vertex --><-- !vertex */
	strcpy(*a, "vertex");
	(void)v;
#endif /* !vertex --> */
}

/* This relies on {List.h} which must be in the same directory. */
#define LIST_NAME G_(Vertex)
#define LIST_TYPE struct G_(Vertex)
#define LIST_TO_STRING &PG_(vertex_to_string)
#define LIST_SUBTYPE
#include "List.h" /* Defines {<G>VertexList} and {<G>VertexListNode}. */

/** The directed graph. To instantiate, see \see{<V>Digraph}. */
struct G_(Digraph);
struct G_(Digraph) {
	struct G_(VertexList) vertices;
	struct G_(Vertex) *start;
};



/** Called in \see{<G>Digraph_}.
 @implements <G>VertexAction */
static void PG_(v_clear)(struct G_(Vertex) *const v) {
	assert(v);
	G_(EdgeListClear)(&v->out);
}

/** Initialises it to empty. */
static void PG_(clear)(struct G_(Digraph) *const g) {
	G_(VertexListClear)(&g->vertices);
	g->start = 0;
}

/** Destructor for {g}.
 @param g: If null or empty, does nothing.
 @order O({vertices})
 @allow */
static void G_(Digraph_)(struct G_(Digraph) *const g) {
	if(!g) return;
	G_(VertexListForEach)(&g->vertices, &PG_(v_clear));
	PG_(clear)(g);
}

/** Initialises {g} to an empty {Digraph}.
 @param g: If null, does nothing.
 @order \Theta(1)
 @allow */
static void G_(Digraph)(struct G_(Digraph) *const g) {
	if(!g) return;
	PG_(clear)(g);
}

#ifdef DIGRAPH_VERTEX /* <-- vertex */
/** Initialises {v} to contain no edges.
 @return The {<V>} part of the vertex. */
static V *G_(DigraphVertexInit)(struct G_(Vertex) *const v) {
	if(!v) return 0;
	PG_(v_clear)(v);
	return &v->info;
}
#else /* vertex --><-- !vertex */
/** Initialises {v} to contain no edges. */
static void G_(DigraphVertexInit)(struct G_(Vertex) *const v) {
	if(!v) return;
	PG_(v_clear)(v);
}
#endif /* !vertex --> */

/** Undefined behaviour results from adding vertices that have already been
 added. */
static void G_(DigraphVertexAdd)(struct G_(Digraph) *const g,
	struct G_(Vertex) *const v) {
	if(!g || !v) return;
	G_(VertexListPush)(&g->vertices, v);
}

#ifdef DIGRAPH_EDGE /* <-- edge */
/** Initialises {e} to point to {v}.
 @return The {<E>} part of the edge. */
static E *G_(DigraphEdgeInit)(struct G_(Edge) *const e,
	struct G_(Vertex) *const v) {
	if(!e || !v) return 0;
	e->to = v;
	return &e->info;
}
#else /* edge --><-- !edge */
/** Initialises {e} to point to {v}. */
static void G_(DigraphEdgeInit)(struct G_(Edge) *const e,
	struct G_(Vertex) *const v) {
	if(!e || !v) return;
	/* @fixme Check that v is in vertices. */
	e->to = v;
}
#endif /* !edge --> */

/** Undefined behaviour results from adding edges that have already been
 added. */
static void G_(DigraphEdgeAdd)(struct G_(Vertex) *const v,
	struct G_(Edge) *e) {
	if(!v || !e) return;
	G_(EdgeListPush)(&v->out, e);
}

/** Sets the starting vertex. */
static void G_(DigraphStart)(struct G_(Digraph) *const g,
	struct G_(Vertex) *const start) {
	if(!g) return;
	g->start = start;
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
 @throws {fprintf} errors: {IEEE Std 1003.1-2001}.
 @order O(|{vertices}| + |{edges}|)
 @allow */
static int G_(DigraphOut)(const struct G_(Digraph) *const g,
	FILE *const fp) {
	struct G_(Vertex) *v;
	struct G_(Edge) *e;
	char a[12];
	unsigned long v_no, v_to;
	if(fprintf(fp, "digraph " QUOTE(DIGRAPH_NAME) " {\n") < 0) return 0;
	for(v = G_(VertexListFirst)(&g->vertices); v; v = G_(VertexListNext)(v)) {
		v_no = (unsigned long)v;
#ifdef DIGRAPH_VERTEX /* <-- vertex */
		PG_(v_to_string)(&v->info, &a);
#else /* vertex --><-- !vertex */
		*a = '\0';/*strcpy(a, "");*/
#endif /* !vertex --> */
		if(fprintf(fp, "\tv%lu [label=\"%s\"%s];\n", v_no, a,
			v == g->start ? " peripheries=2" : "") < 0) return 0;
		for(e = G_(EdgeListFirst)(&v->out); e; e = G_(EdgeListNext)(e)) {
			v_to = (unsigned long)e->to;
#ifdef DIGRAPH_EDGE /* <-- edge */
			PG_(e_to_string)(&e->info, &a);
			if(fprintf(fp, "\tv%lu -> v%lu [label=\"%s\"];\n", v_no, v_to, a)
				< 0) return 0;
#else /* edge --><-- !edge */
			if(fprintf(fp, "\tv%lu -> v%lu;\n", v_no, v_to) < 0) return 0;
#endif /* !edge --> */
		}
	}
	if(fprintf(fp, "\tnode [fillcolor = red];\n"
		"}\n") < 0) return 0;
	return 1;
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
	G_(DigraphVertexInit)(0);
	G_(DigraphVertexAdd)(0, 0);
	G_(DigraphEdgeInit)(0, 0);
	G_(DigraphEdgeAdd)(0, 0);
	G_(DigraphStart)(0, 0);
	G_(DigraphOut)(0, 0);
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
#undef QUOTE
#undef QUOTE_
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
