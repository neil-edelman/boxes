/* Intended to be included by {../src/Digraph.h} on {DIGRAPH_TEST}. */

#ifdef DIGRAPH_VERTEX /* <-- vertex */
static const PG_(VertexAction) PG_(e_filler) = (DIGRAPH_EDGE_TEST);
#endif /* vertex --> */

#ifdef DIGRAPH_EDGE /* <-- edge */
static const PG_(EdgeAction) PG_(e_filler) = (DIGRAPH_EDGE_TEST);
#endif /* edge --> */



static void PG_(valid_state)(const struct G_(Digraph) *const g) {
	if(!g) return; /* null is valid */
}

static void PG_(test_basic)(void) {
	struct G_(Digraph) g;
	PG_(valid_state)(0);
	G_(Digraph)(&g);
	printf("Destructor:\n");
}

static void PG_(test_random)(void) {
	struct G_(Digraph) g;
	struct G_(Vertex) vs[7], *v, *v1;
	const size_t vs_size = sizeof vs / sizeof *vs;
	const char *const fn = QUOTE(DIGRAPH_NAME) ".gv";
	FILE *const fp = fopen(fn, "w");
	int done = 0;
	do {
		if(!fp) break;
		G_(Digraph)(&g);
		for(v = vs, v1 = v + vs_size; v < v1; v++) {
			G_(DigraphVertexInit(v)); /* @fixme add DIGRAPH_VERTEX */
		}
		for(v = vs, v1 = v + vs_size; v < v1 - 1; v++) {
			G_(DigraphAdd)(&g, v);
			printf("added %s\n", G_(VertexListToString)(&g.vertices));
		}
		if(!G_(DigraphOut)(&g, fp)) break;
		done = 1;
	} while(0);
	if(!done) perror(fn);
	fclose(fp);
	if(!done) assert(0);
}

/** The list will be tested on stdout. */
static void G_(DigraphTest)(void) {
	printf("Digraph<" QUOTE(DIGRAPH_NAME) ">: was created using: "
#ifdef DIGRAPH_TEST
		"DIGRAPH_TEST; "
#endif
#ifdef DIGRAPH_VERTEX
		"DIGRAPH_VERTEX<" QUOTE(DIGRAPH_VERTEX) ">; "
#endif
#ifdef DIGRAPH_VERTEX_TO_STRING
		"DIGRAPH_VERTEX_TO_STRING<" QUOTE(DIGRAPH_VERTEX_TO_STRING) ">; "
#endif
#ifdef DIGRAPH_VERTEX_TEST
		"DIGRAPH_VERTEX_TEST<" QUOTE(DIGRAPH_VERTEX_TEST) ">; "
#endif
#ifdef DIGRAPH_EDGE
		"DIGRAPH_EDGE<" QUOTE(DIGRAPH_EDGE) ">; "
#endif
#ifdef DIGRAPH_EDGE_TO_STRING
		"DIGRAPH_EDGE_TO_STRING<" QUOTE(DIGRAPH_EDGE_TO_STRING) ">; "
#endif
#ifdef DIGRAPH_EDGE_TEST
		"DIGRAPH_EDGE_TEST<" QUOTE(DIGRAPH_EDGE_TEST) ">; "
#endif
		"testing:\n");
	PG_(test_basic)();
	PG_(test_random)();
	fprintf(stderr, "Done tests of Digraph<" QUOTE(DIGRAPH_NAME) ">.\n\n");
}

/* Un-define all macros. */
