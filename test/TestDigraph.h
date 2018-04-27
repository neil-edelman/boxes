/* Intended to be included by {../src/Digraph.h} on {DIGRAPH_TEST}. */

#ifdef DIGRAPH_VDATA /* <-- vdata */
static const PG_(VDataAction) PG_(vdata_filler) = (DIGRAPH_VDATA_TEST);
#endif /* vdata --> */

#ifdef DIGRAPH_EDATA /* <-- edata */
static const PG_(EDataAction) PG_(edata_filler) = (DIGRAPH_EDATA_TEST);
#endif /* edata --> */



static void PG_(valid_state)(const struct G_(Digraph) *const g) {
	G_(DigraphValid)(g);
}

static void PG_(test_basic)(void) {
	struct G_(Digraph) g;
	PG_(valid_state)(0);
	printf("Constructor:\n");
	G_(Digraph)(&g);
	printf("Destructor:\n");
	G_(Digraph_)(&g);
	PG_(valid_state)(&g);
}

static void PG_(test_random)(void) {
	const char *const fn = "graphs/" QUOTE(DIGRAPH_NAME) ".gv";
	FILE *const fp = fopen(fn, "w");
	struct G_(Digraph) g;
	struct G_(VertexLink) vns[100], *vn, *vn1;
	const size_t vns_size = sizeof vns / sizeof *vns;
	struct G_(EdgeLink) ens[200], *en, *en1;
	const size_t ens_size = sizeof ens / sizeof *ens;
	struct G_(Vertex) *v0, *v1;
	int done = 0;
	printf("Random:\n");
	G_(Digraph)(&g);
	do {
		if(!fp) break;
		for(vn = vns, vn1 = vn + vns_size; vn < vn1; vn++) {
#ifdef DIGRAPH_VDATA /* <-- vdata */
			PG_(vdata_filler)(G_(DigraphVertexData)(&vn->data));
#endif /* vdata --> */
			G_(DigraphVertex)(&g, &vn->data);
		}
		for(en = ens, en1 = en + ens_size; en < en1; en++) {
			size_t idx0 = (size_t)(rand() / (1.0 + RAND_MAX) * vns_size);
			size_t idx1 = (size_t)(rand() / (1.0 + RAND_MAX) * vns_size);
			v0 = &vns[idx0].data;
			v1 = &vns[idx1].data;
#ifdef DIGRAPH_EDATA /* <-- edata */
			PG_(edata_filler)(G_(DigraphEdgeData)(&en->data));
#endif /* !edata --> */
			G_(DigraphEdge)(&en->data, v0, v1);
		}
		/*PG_(valid_state)(&g);*/
		if(!G_(DigraphOut)(&g, fp)) break;
		done = 1;
	} while(0); if(!done) perror(fn); {
		G_(Digraph_)(&g);
		fclose(fp);
		if(!done) assert(0);
	}
	printf("See <%s>.\n", fn);
}

/** The list will be tested on stdout. */
static void G_(DigraphTest)(void) {
	printf("Digraph<" QUOTE(DIGRAPH_NAME) ">: was created using: "
#ifdef DIGRAPH_TEST
		"DIGRAPH_TEST; "
#endif
#ifdef DIGRAPH_VDATA
		"DIGRAPH_VDATA<" QUOTE(DIGRAPH_VDATA) ">; "
#endif
#ifdef DIGRAPH_VDATA_TO_STRING
		"DIGRAPH_VDATA_TO_STRING<" QUOTE(DIGRAPH_VDATA_TO_STRING) ">; "
#endif
#ifdef DIGRAPH_VDATA_TEST
		"DIGRAPH_VDATA_TEST<" QUOTE(DIGRAPH_VDATA_TEST) ">; "
#endif
#ifdef DIGRAPH_EDATA
		"DIGRAPH_EDATA<" QUOTE(DIGRAPH_EDATA) ">; "
#endif
#ifdef DIGRAPH_EDATA_TO_STRING
		"DIGRAPH_EDATA_TO_STRING<" QUOTE(DIGRAPH_EDATA_TO_STRING) ">; "
#endif
#ifdef DIGRAPH_EDATA_TEST
		"DIGRAPH_EDATA_TEST<" QUOTE(DIGRAPH_EDATA_TEST) ">; "
#endif
		"testing:\n");
	PG_(test_basic)();
	PG_(test_random)();
	fprintf(stderr, "Done tests of Digraph<" QUOTE(DIGRAPH_NAME) ">.\n\n");
}
