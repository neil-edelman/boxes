/** Unit test.

 @title		TestDigraph
 @author	Neil
 @std		C89/90
 @version	1.0; 2018-04
 @since		1.0; 2018-04 */

#include <stddef.h>	/* ptrdiff_t offset_of */
#include <stdlib.h> /* EXIT_ rand */
#include <stdio.h>  /* fprintf */
#include <string.h>	/* memcmp strcmp */
#include <time.h>	/* clock */
#include <limits.h>	/* INT_MAX */
#include "Orcish.h"

/* Test Blank. */

#define DIGRAPH_NAME Blank
#define DIGRAPH_TEST
#include "../src/Digraph.h"

/* Test Colour. */

/* {Colour} is an {enum} that's used on {Edge}. */
enum Colour { White , Silver, Gray, Black, Red, Maroon, Bisque, Wheat, Tan,
	Sienna, Brown, Yellow, Khaki, Gold, Olive, Lime, Green, Aqua, Cyan, Teal,
	Salmon, Orange, Powder, Sky, Steel, Royal, Blue, Navy, Fuchsia, Pink,
	Purple };
static const char *const colour_names[] = { "White", "Silver", "Gray", "Black",
	"Red", "Maroon", "Bisque", "Wheat", "Tan", "Sienna", "Brown", "Yellow",
	"Khaki", "Gold", "Olive", "Lime", "Green", "Aqua", "Cyan", "Teal",
	"Salmon", "Orange", "Powder", "Sky", "Steel", "Royal", "Blue", "Navy",
	"Fuchsia", "Pink", "Purple" }; /* max 11 letters */
static const size_t colour_size = sizeof colour_names / sizeof *colour_names;
/** @implements <Colour>ToString */
static void Colour_to_string(const enum Colour colour, char (*const a)[12]) {
	assert(colour < colour_size);
	sprintf(*a, "%s", colour_names[colour]);
}
/** @implements <Colour>Action */
static void Colour_filler(enum Colour *const colour) {
	assert(colour);
	*colour = (enum Colour)(float)(rand() / (RAND_MAX + 1.0) * colour_size);
}

/* Vertex. */
struct V {
	char value[12];
};
/** @implements <V>ToString */
static void V_to_string(const struct V *v, char (*const a)[12]) {
	sprintf(*a, "%.11s", v->value);
}
/** @implements <V>Action */
static void V_filler(struct V *const v) {
	Orcish(v->value, sizeof v->value);
}

/* Edge. */
struct E {
	enum Colour colour;
};
/** @implements <E>ToString */
static void E_to_string(const struct E *e, char (*const a)[12]) {
	Colour_to_string(e->colour, a);
}
/** @implements <E>Action */
static void E_filler(struct E *const e) {
	Colour_filler(&e->colour);
}

#define DIGRAPH_NAME Colour
#define DIGRAPH_VDATA struct V
#define DIGRAPH_EDATA struct E
#define DIGRAPH_VDATA_TO_STRING &V_to_string
#define DIGRAPH_EDATA_TO_STRING &E_to_string
#define DIGRAPH_VDATA_TEST &V_filler
#define DIGRAPH_EDATA_TEST &E_filler
#define DIGRAPH_TEST
#include "../src/Digraph.h"



/* A more complex example using dynimic memory. */

struct Machine;

/**
 * Transition virtual table, referenced from {Transition}. This forms the extra
 * data on the pointed to by the edges of the digraph.
 */
struct Transition;
typedef void (*TransitionToString)(const struct Transition *, char(*const)[12]);
struct TransitionVt {
	const char *debug;
	const TransitionToString to_string;
};

/**
 * Abstract {Transition}. This forms extra data on the edge of the digraph.
 */
struct Transition {
	const struct TransitionVt *vt;
};
/** Constructor; only called from it's children. */
static void Transition(struct Transition *const t,
	const struct TransitionVt *const vt) {
	assert(t && vt);
	t->vt = vt;
}
/** @implements TransitionToString */
static void transition_to_string(const struct Transition *t,char(*const a)[12]){
	t->vt->to_string(t, a);
}
#define DIGRAPH_NAME Machine
#define DIGRAPH_EDATA struct Transition
#define DIGRAPH_EDATA_TO_STRING &transition_to_string
#include "../src/Digraph.h" /* StateDigraph, StateVertex, StateEdge. */

/*
 * {StateVertex} container.
 */
#define POOL_NAME Vertex
#define POOL_TYPE struct MachineVertex
#define POOL_MIGRATE_ALL struct Machine
#include "Pool.h"

/**
 * {Empty} extends {Transition} as {StateEdgeLink}.
 */
/** @implements TransitionToString */
static void empty_to_string(const struct Transition *e, char (*const a)[12]) {
	sprintf(*a, "ε");
	(void)e;
}
static struct TransitionVt empty_vt = { "Empty", empty_to_string };
#define POOL_NAME Empty
#define POOL_TYPE struct MachineEdgeLink
#define POOL_MIGRATE_ALL struct Machine
#include "Pool.h"
/** Constructor.
 @param e: This is instantiated; any data will be erased. */
static struct MachineEdgeLink *Empty(struct EmptyPool *const ep) {
	struct MachineEdgeLink *e;
	assert(ep);
	if(!(e = EmptyPoolNew(ep))) return 0;
	Transition(&e->data.info, &empty_vt);
	printf("Empty.\n");
	return e;
}

/**
 * {Literals} extends {Transition} as {StateEdgeLink}.
 */
struct Literals {
	struct MachineEdgeLink edge;
	char *text;
	size_t text_size;
};
/** {container_of} (probably does nothing.) */
static const struct Literals *literals_holds_transition(const struct
	Transition *const t) {
	return (const struct Literals *)(const void *)
	((const char *)t
		- offsetof(struct MachineEdge, info)
		- offsetof(struct MachineEdgeLink, data)
		- offsetof(struct Literals, edge));
}
/** @implements TransitionToString */
static void literals_to_string(const struct Transition *t, char (*const a)[12]){
	sprintf(*a, "%.11s", literals_holds_transition(t)->text);
}
static struct TransitionVt literals_vt = { "Literals", literals_to_string };
#define POOL_NAME Literals
#define POOL_TYPE struct Literals
#define POOL_MIGRATE_ALL struct Machine
#include "Pool.h"
/** Destructor because this takes up resources, but doesn't do anything about
 the graph. */
static void Literals_(struct Literals *const l) {
	assert(l);
	printf("~Literals: freeing %s.\n", l->text);
	free(l->text);
	l->text = 0;
	l->text_size = 0;
}
/** Constructor.
 @return Success.
 @throws {malloc} errors. */
static struct Literals *Literals(struct LiteralsPool *const lp,
	const char *const text, size_t text_size) {
	struct Literals *l;
	assert(lp && text && text_size);
	if(!(l = LiteralsPoolNew(lp))) return 0;
	Transition(&l->edge.data.info, &literals_vt);
	l->text = 0;
	l->text_size = 0;
	/* Copy the literals; null terminator even thought it's not really used. */
	if(!(l->text = malloc(sizeof *text * (text_size + 1))))
		{ LiteralsPoolPop(lp); return 0; }
	memcpy(l->text, text, text_size);
	l->text[text_size] = '\0';
	l->text_size = text_size;
	printf("Literals %p: <%s>:%lu\n", (void *)l, l->text,
		(unsigned long)l->text_size);
	return l;
}

/**
 * Machine.
 */
struct Machine {
	const char *title;
	struct MachineDigraph graph;
	struct VertexPool v;
	struct EmptyPool empties;
	struct LiteralsPool literals;
};

/*
 * Temporary nesting for compiling.
 * Refers to index in the vertices pool.
 */
#define POOL_NAME Nest
#define POOL_TYPE size_t
#define POOL_STACK
#include "Pool.h"
static size_t *Nest(struct NestPool *const np, const size_t idx) {
	size_t *i;
	assert(np);
	if(!(i = NestPoolNew(np))) return 0;
	*i = idx;
	return i;
}

/** Called from \see{Regex}.
 @return Success, otherwise {errno} will (probably) be set. */
static int m_compile(struct Machine *m, const char *const compile) {
	struct NestPool np;
	size_t *n;
	struct MachineVertex *v;
	const char *c = compile, *start = 0;
	int is_done = 0, is_edge = 0;
	enum { SUCCESS, RESOURCES, SYNTAX } e = SUCCESS;

	NestPool(&np);
	printf("compile: <%s>.\n", compile);
	do { /* try */
		if(!(v = VertexPoolNew(&m->v))
			|| !Nest(&np, VertexPoolIndex(&m->v, v))) break;
		MachineDigraphPutVertex(&m->graph, v);
		printf("vertices: %s.\n", MachineVertexListToString(&m->graph.vertices));
		do {
			printf("%c (%d)\n", *c, (int)*c);
			switch(*c) {
				case '|': is_edge = 1; break;
				case '\0': is_done = is_edge = 1; break;
				default: if(!start) start = c; break;
			}
			if(is_edge) {
				struct MachineEdge *edge;
				struct MachineVertex *v1 = VertexPoolNew(&m->v);
				if(!v1) { e = RESOURCES; break; }
				printf("edge\n");
				MachineDigraphPutVertex(&m->graph, v1);
				is_edge = 0;
				if(start && start < c) {
					struct Literals *lit;
					if(!(lit = Literals(&m->literals, start, c - start)))
						{ e = RESOURCES; break; }
					edge = &lit->edge.data;
				} else {
					struct MachineEdgeLink *emp;
					if(!(emp = Empty(&m->empties)))
						{ e = RESOURCES; break; }
					edge = &emp->data;
				}
				start = 0;
				MachineDigraphPutEdge(edge, v, v1);
			}
		} while(c++, !is_done);
		if(e) break;
		if(!NestPoolPop(&np) || NestPoolPeek(&np)) { printf("shit\n");e = SYNTAX; break; }
	} while(0); if(e == SYNTAX) { /* catch(SYNTAX) */
		errno = EILSEQ;
	} { /* finally */
		NestPoolClear(&np);
	}
	printf("m_compile: e %d\n", e);
	return !e;
}
/** @implements <Machine>Migrate */
static void vertex_migrate(struct Machine *m,
	const struct Migrate *const migrate) {
	assert(m && migrate);
	/*VertexPoolMigrate(&m->graph, migrate); no such thing */
	/*MachineDigraphVertexMigrateAll(&m->graph, migrate);*/ /* no */
	printf("vertex_migrate doom\n");
	printf("%s.\n", MachineVertexListToString(&m->graph.vertices));
}
/** @implements <Machine>Migrate */
static void edge_migrate(struct Machine *m,
	const struct Migrate *const migrate) {
	assert(m && migrate);
	/*MachineDigraphEdgeMigrateAll(&m->graph, migrate);*/ /* no */
	printf("edge_migrate doom\n");
}
/** Destructor. */
static void Machine_(struct Machine **const pm) {
	struct Machine *m;
	if(!pm || !(m = *pm)) return;
	printf("~Machine<%s>.\n", m->title);
	LiteralsPoolForEach(&m->literals, &Literals_);
	LiteralsPool_(&m->literals);
	EmptyPool_(&m->empties);
	VertexPool_(&m->v);
	MachineDigraph_(&m->graph);
	*pm = 0;
}
/** Constructor. */
static struct Machine *Machine(const char *const compile) {
	struct Machine *m;
	if(!compile) return 0;
	if(!(m = malloc(sizeof *m))) return 0;
	m->title = compile;
	MachineDigraph(&m->graph);
	VertexPool(&m->v, &vertex_migrate, m);
	EmptyPool(&m->empties, &edge_migrate, m);
	LiteralsPool(&m->literals, &edge_migrate, m);
	if(!m_compile(m, compile)) Machine_(&m);
	return m;
}

/** @return Error condition. */
int main(void) {
	unsigned seed = (unsigned)clock();
	struct Machine *m = 0;
	const char *const fn = "graphs/machine.gv";
	FILE *fp = 0;
	int is_err = 0;
	srand(seed), rand(), printf("Seed %u.\n", seed);
	BlankDigraphTest();
	ColourDigraphTest();
	/* Custom. */
	if(!(m = Machine("hi(a|b|c)|d(e(f))"))
		|| !(fp = fopen(fn, "w"))
		|| !MachineDigraphOut(&m->graph, fp)) is_err = 1;
	if(is_err) perror(fn), assert(0);
	printf("The machine has %s vertices.\n", MachineVertexListToString(&m->graph.vertices));
	fclose(fp);
	Machine_(&m);
	if(!is_err) printf("Test success.\n\n");
	return is_err ? EXIT_FAILURE : EXIT_SUCCESS;
}
