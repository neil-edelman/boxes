/* Intended to be included by {../src/Digraph.h} on {DIGRAPH_TEST}. */

#ifdef DIGRAPH_VERTEX /* <-- vertex */
static const PG_(VertexAction) PG_(v_filler) = (DIGRAPH_VERTEX_TEST);
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
	const char *const fn = QUOTE(DIGRAPH_NAME) ".gv";
	FILE *const fp = fopen(fn, "w");
	int done = 0;
	do {
		struct G_(Digraph) g;
		struct G_(VertexListNode) vns[100], *vn, *vn1;
		const size_t vns_size = sizeof vns / sizeof *vns;
		struct G_(EdgeListNode) ens[200], *en, *en1;
		const size_t ens_size = sizeof ens / sizeof *ens;
		struct G_(Vertex) *v0, *v1;

		if(!fp) break;
		G_(Digraph)(&g);
		for(vn = vns, vn1 = vn + vns_size; vn < vn1; vn++) {
#ifdef DIGRAPH_VERTEX /* <-- vertex */
			PG_(v_filler)(G_(DigraphVertexInit)(&vn->data));
#else /* vertex --><-- !vertex */
			G_(DigraphVertexInit(&vn->data));
#endif /* !vertex --> */
			G_(DigraphAdd)(&g, &vn->data);
		}
		for(en = ens, en1 = en + ens_size; en < en1; en++) {
			size_t idx0 = (size_t)(rand() / (1.0 + RAND_MAX) * vns_size);
			size_t idx1 = (size_t)(rand() / (1.0 + RAND_MAX) * vns_size);
			v0 = &vns[idx0].data;
			v1 = &vns[idx1].data;
			printf("v0: %lu; v1: %lu.\n", idx0, idx1);
#ifdef DIGRAPH_EDGE /* <-- edge */
			PG_(e_filler)(G_(DigraphEdgeInit)(&en->data, v1));
#else /* edge --><-- !edge */
			G_(DigraphEdgeInit(&en->data, v1));
#endif /* !edge --> */
			printf("adding edge to v%lu\n", (struct G_(VertexListNode) *)v1 - vns);
			G_(DigraphVertexAdd)(v0, &en->data);
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
