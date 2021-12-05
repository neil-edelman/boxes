/* Intended to be included by Map.h on {MAP_TEST}. */

/* Check that {MAP_TO_STRING} is a function implementing {<I>Action}. */
static const PI_(Action) PI_(filler) = (MAP_TEST);


#ifdef I_NAME
#undef I_NAME
#endif
#ifdef QUOTE
#undef QUOTE
#endif
#ifdef QUOTE_
#undef QUOTE_
#endif
#ifdef PTI_
#undef PTI_
#endif
#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)
#define I_NAME QUOTE(MAP_NAME)
#define PTI_(thing) PCAT(list_map, PCAT(MAP_NAME, PCAT(Item, thing)))

/** Count how many are in the {bin}.
 @order O(n) */
static size_t PI_(count)(const struct PI_(ItemList) *const bin) {
	struct PTI_(X) *i;
	size_t c = 0;
	for(i = bin->head.anonymous_next; i->anonymous_next; i = i->anonymous_next)
		c++;
	return c;
}

/** Assertion function for seeing if it is in a valid state.
 @order O(|{map.bins}| + |{map.items}|) */
static void PI_(legit)(const struct I_(Map) *const map) {
	struct PI_(ItemList) *b, *end;
	size_t items = 0;
	assert(map);
	assert(map->log_bins > 0 && map->log_bins < 32);
	for(b = map->bins, end = b + (1 << map->log_bins); b < end; b++)
		items += PI_(count)(b);
	assert(map->items == items);
}

/** Draw a graph of this {Map} to {fn} in Graphviz format.
 @order O(|{map.bins}| + |{map.items}|) */
static void PI_(graph)(const struct I_(Map) *const map, const char *const fn) {
	FILE *fp;
	I *item;
	struct PI_(ItemList) *b, *end;
	struct I_(MapNode) *mn;
	char str[12];
	assert(map && fn);
	if(!(fp = fopen(fn, "w"))) { perror(fn); return; }
	fprintf(fp, "digraph {\n");
	for(b = map->bins, end = b + (1 << map->log_bins); b < end; b++) {
		/* Bins. */
		fprintf(fp, "\tsubgraph cluster_%p {\n"
			"\t\tnode [shape=box];\n"
			"\t\tp%p [label=\"head%u\"];\n"
			"\t\tp%p [label=\"tail%u\"];\n"
			"\t\tnode [shape=oval];\n"
			"\t\tp%p -> p%p;\n"
			"\t\tp%p -> p%p [style=dashed];\n",
			(void *)b, (void *)&b->head, (unsigned)(b - map->bins),
			(void *)&b->tail, (unsigned)(b - map->bins),
			(void *)&b->head, (void *)b->head.anonymous_next,
			(void *)&b->tail, (void *)b->tail.anonymous_prev);
		/* Draw all items. */
		for(item = PI_(ItemListFirst(b); item; item = PI_(ItemListNext)(item))){
			PI_(to_string)(item, &str);
			mn = PI_(node_holds_item)(item);
			fprintf(fp, "\t\tp%p [label=\"%u\\l%s\\l\"];\n"
				"\t\tp%p -> p%p;\n"
				"\t\tp%p -> p%p [style=dashed];\n",
				(void *)&mn->node.x, mn->hash, str,
				(void *)&mn->node.x, (void *)mn->node.x.anonymous_next,
				(void *)&mn->node.x, (void *)mn->node.x.anonymous_prev);
		}
		fprintf(fp, "\t}\n");
	}
	fprintf(fp, "\tnode [colour=red, style=filled];\n");
	fprintf(fp, "}\n");
	fclose(fp);
}

static void PI_(test_basic)(void) {
	struct I_(Map) *m = 0;
	struct Test {
		int is_in;
		struct I_(MapNode) node;
	} test[3000], *t, *end;
	I *eject;
	const size_t test_size = sizeof test / sizeof *test;
	char a[12];
	m = I_(Map)();
	assert(m);
	PI_(legit)(m);
	/* Put in map. */
	for(t = test, end = t + test_size; t < end; t++) {
		PI_(filler)(&t->node.node.data);
		PI_(to_string)(&t->node.node.data, &a);
		/*printf("About to put %s into map.\n", a);*/
		PI_(legit)(m);
		eject = I_(MapPut)(m, &t->node.node.data);
		t->is_in = 1;
		if(eject) {
			/*PI_(to_string)(eject, &a), printf("Ejected %s", a);*/
			((struct Test *)(void *)
				((char *)eject
				- offsetof(struct Test, node)
				- offsetof(struct I_(MapNode), node)
				- offsetof(struct PI_(ItemListNode), data)))->is_in = 0;
		}
		if((t - test) == 150) PI_(graph)(m, "graph/" I_NAME ".gv");
		/*printf("m: %s.\n", I_(MapToString(m)));*/
		PI_(legit)(m);
	}
	/* Collect stats. */
	for() {
	}
	I_(Map_)(&m);
	assert(!m);
}

/** The list will be tested on stdout. */
static void I_(MapTest)(void) {
	printf("Map<" I_NAME ">: of type <" QUOTE(MAP_TYPE) "> was created using: "
		"MAP_KEY <" QUOTE(MAP_KEY) ">; "
		"MAP_TYPE_TO_KEY <" QUOTE(MAP_TYPE_TO_KEY) ">; "
		"MAP_IS_EQUAL <" QUOTE(MAP_IS_EQUAL) ">; "
		"MAP_HASH <" QUOTE(MAP_HASH) ">; "
		"TYPE_TO_STRING<" QUOTE(MAP_TO_STRING) ">; "
		"MAP_TEST<" QUOTE(MAP_TEST) ">; "
		"testing:\n");
	PI_(test_basic)();
	fprintf(stderr, "Done tests of Set<" I_NAME ">.\n\n");
}

#undef I_NAME
#undef QUOTE
#undef QUOTE_
#undef PTI_
