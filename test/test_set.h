#if defined(QUOTE) || defined(QUOTE_)
#error QUOTE_? cannot be defined.
#endif
#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)

#include <stddef.h> /* offsetof */
#include <stdio.h>  /* fprintf FILE */
#include <math.h>   /* sqrt NAN? */
#ifndef NAN /* <https://stackoverflow.com/questions/5714131/nan-literal-in-c> */
#define NAN (0. / 0.)
#endif
#include <string.h> /* memset */

/** Operates by side-effects. Used for `SET_TEST`. */
typedef void (*PS_(action_fn))(PS_(type));

/* Check that `SET_TEST` is a function implementing `<PS>action_fn`. */
//static const PS_(action_fn) PS_(filler) = (SET_TEST); ****

static size_t PS_(count_bucket)(const struct S_(set) *const set,
	PS_(uint) idx) {
	struct PS_(entry) *entry;
	PS_(uint) next;
	size_t no = 0;
	assert(set && idx < PS_(capacity)(set));
	entry = set->entries + idx;
	if((next = entry->next) == SETm2
		|| set->top != SETm1 && set->top <= idx /* In range of stack. */
		&& idx != PS_(hash_to_bucket)(set, PS_(entry_hash)(entry))) return 0;
	for( ; ; ) {
		no++;
		if(next == SETm1) return no;
		idx = next;
		assert(set->top <= idx && idx < 1 << set->log_capacity);
		entry = set->entries + idx;
		next = entry->next;
		assert(next != SETm2); /* -2 null: linked-list integrity. */
	}
}

/** Mean: `mean`, population variance: `ssdm/n`, sample variance: `ssdm/(n-1)`.
 <Welford1962Note>. */
static struct { size_t n, max; double mean, ssdm; }
	PS_(stats) = { 0, 0, 0.0, 0.0 };
static void PS_(reset)(void) {
	PS_(stats).n = PS_(stats).max = 0;
	PS_(stats).mean = PS_(stats).ssdm = 0.0;
}
/** Update one sample point of `value`. */
static void PS_(update)(const size_t value) {
	double d, v = value;
	if(PS_(stats).max < value) PS_(stats).max = value;
	d = v - PS_(stats).mean;
	PS_(stats).mean += d / ++PS_(stats).n;
	PS_(stats).ssdm += d * (v - PS_(stats).mean);
}
/** Collect stats on `set`. */
static void PS_(collect)(const struct S_(set) *const set) {
	size_t size = 0;
	PS_(reset)();
	if(set && set->entries) {
		PS_(uint) idx, idx_end = 1 << set->log_capacity;
		for(idx = 0; idx < idx_end; idx++) {
			size_t no;
			/* I'm sure there's a cheaper way to do it. */
			for(no = PS_(count_bucket)(set, idx); no; no--) PS_(update)(no);
		}
		size = set->size;
	}
}

/** Draw a diagram of `set` written to `fn` in
 [Graphviz](https://www.graphviz.org/) format.
 @order \O(|`set.bins`| + |`set.items`|) */
static void PS_(graph)(const struct S_(set) *const set, const char *const fn) {
	FILE *fp;
	size_t i, i_end;
	assert(set && fn);
	if(!(fp = fopen(fn, "w"))) { perror(fn); return; }
	printf("*** %s\n", fn);
	fprintf(fp, "digraph {\n"
		"\trankdir=LR;\n"
		"\tfontface=modern;\n");
	if(!set->entries) { fprintf(fp, "\tidle [shape=none]\n"); goto end; }
	PS_(collect)(set);
	//assert((size_t)set->size == PS_(stats).n);
	fprintf(fp,
		"\tnode [shape=box, style=filled, fillcolor=\"Gray95\"];\n"
		"\tset [label=<<TABLE BORDER=\"0\">\n"
		"\t<TR><TD COLSPAN=\"3\" ALIGN=\"LEFT\"><FONT COLOR=\"Gray85\">&lt;" QUOTE(SET_NAME) "&gt;set: " QUOTE(SET_TYPE) "</FONT></TD></TR>\n"
		"\t<TR>\n"
		"\t\t<TD>&nbsp;</TD>\n"
		"\t\t<TD BORDER=\"0\" ALIGN=\"RIGHT\" BGCOLOR=\"Gray90\">load factor</TD>\n"
		"\t\t<TD BORDER=\"0\" ALIGN=\"RIGHT\" BGCOLOR=\"Gray90\">%lu/%lu</TD>\n"
		"\t</TR>\n"
		"\t<TR>\n"
		"\t\t<TD>&nbsp;</TD>\n"
		"\t\t<TD BORDER=\"0\" ALIGN=\"RIGHT\">E[no bucket]</TD>\n"
		"\t\t<TD BORDER=\"0\" ALIGN=\"RIGHT\">%.2f(%.1f)</TD>\n"
		"\t</TR>\n"
		"\t<TR>\n"
		"\t\t<TD>&nbsp;</TD>\n"
		"\t\t<TD BORDER=\"0\" ALIGN=\"RIGHT\" BGCOLOR=\"Gray90\">max bucket</TD>\n"
		 "\t\t<TD BORDER=\"0\" ALIGN=\"RIGHT\" BGCOLOR=\"Gray90\">%lu</TD>\n"
		 "\t</TR>\n",
		(unsigned long)PS_(stats).n,
		set->entries ? 1ul << set->log_capacity : 0,
		PS_(stats).n ? PS_(stats).mean : (double)NAN, PS_(stats).n > 1
		? sqrt(PS_(stats).ssdm / (double)(PS_(stats).n - 1)) : (double)NAN,
		(unsigned long)PS_(stats).max);
	fprintf(fp, "\t<TR>\n"
		"\t\t<TD BORDER=\"0\"><FONT FACE=\"Times-Italic\">i</FONT></TD>\n"
		"\t\t<TD BORDER=\"0\"><FONT FACE=\"Times-Italic\">hash</FONT></TD>"
		"\n"
		"\t\t<TD BORDER=\"0\"><FONT FACE=\"Times-Italic\">key</FONT></TD>"
		"\n"
		"\t</TR>\n");
	for(i = 0, i_end = 1 << set->log_capacity; i < i_end; i++) {
		const char *const bgc = i & 1 ? "" : " BGCOLOR=\"Gray90\"",
			*const top = set->top == i ? " BORDER=\"1\"" : "";
		struct PS_(entry) *e = set->entries + i;
		fprintf(fp, "\t<TR>\n"
			"\t\t<TD ALIGN=\"RIGHT\"%s%s>0x%lx</TD>\n",
			top, bgc, (unsigned long)i);
		if(e->next != SETm2) {
			const char *const closed
				= PS_(hash_to_bucket)(set, PS_(entry_hash)(e)) == i
				? "⬤" : "◯";
			char z[12];
			PS_(to_string)(&e->key, &z);
			fprintf(fp, "\t\t<TD ALIGN=\"RIGHT\"%s>0x%lx</TD>\n"
				"\t\t<TD ALIGN=\"LEFT\"%s>%s</TD>\n"
				"\t\t<TD PORT=\"%lu\"%s>%s</TD>\n",
				bgc, (unsigned long)e->hash,
				bgc, z,
				(unsigned long)i, bgc, closed);
		}
		fprintf(fp, "\t</TR>\n");
	}
	fprintf(fp, "</TABLE>>];\n");
	fprintf(fp, "\tnode [shape=plain, fillcolor=none]\n");
	for(i = 0, i_end = 1 << set->log_capacity; i < i_end; i++) {
		struct PS_(entry) *e = set->entries + i;
		PS_(uint) left, right;
		if((right = e->next) >= SETm2
		   || PS_(hash_to_bucket)(set, PS_(entry_hash)(e)) != i) continue;
		fprintf(fp,
			"\te%lu [label=\"0x%lx\"];\n"
			"\tset:%lu -> e%lu [tailclip=false];\n",
			(unsigned long)right, (unsigned long)right,
			(unsigned long)i, (unsigned long)right);
		while(left = right, e = set->entries + left,
			(right = e->next) != SETm1) {
			assert(right != SETm2);
			fprintf(fp,
				"\te%lu [label=\"0x%lx\"];\n"
				"\te%lu -> e%lu;\n",
				(unsigned long)right, (unsigned long)right,
				(unsigned long)left, (unsigned long)right);
		}
	}
end:
	fprintf(fp, "\tnode [color=red];\n"
		"}\n");
	fclose(fp);
}

/** Draw a histogram of `set` written to `fn` in
 [Gnuplot](http://www.gnuplot.info/) format.
 @order \O(|`set.bins`| + |`set.items`|) */
static void PS_(histogram)(const struct S_(set) *const set,
	const char *const fn) {
	FILE *fp;
	size_t histogram[64], hs, h;
	const size_t histogram_size = sizeof histogram / sizeof *histogram;
	assert(set && fn);
	memset(histogram, 0, sizeof histogram);
	if(!(fp = fopen(fn, "w"))) { perror(fn); return; }
	if(set->entries) {
		PS_(uint) i, i_end = 1 << set->log_capacity;
		struct PS_(entry) *e;
		for(i = 0; i < i_end; i++) {
			size_t items;
			e = set->entries + i;
			if(e->next == SETm2 || set->top != SETm1 && set->top <= i
				&& i != PS_(hash_to_bucket)(set, PS_(entry_hash)(e)))
				continue;
			//= PS_(count)(set, b);
			continue;
			/* The bins are `0,1,2,...,[histogram_size - 1, \infty]`. */
			if(items >= histogram_size) items = histogram_size - 1;
			histogram[items]++;
		}
	}
	/* Hopefully `historgram_size` is much larger then it has to be. */
	for(hs = histogram_size - 1; !(histogram[hs] && (hs++, 1)) && hs; hs--);
	fprintf(fp, "# Size: %lu.\n"
		"set term postscript eps enhanced color\n"
		"set output \"%s.eps\"\n"
		"set grid\n"
		"set xlabel \"entry occupancy\"\n"
		"set ylabel \"frequency\"\n"
		"set style histogram\n"
		"set xrange [0:]\n"
		"plot \"-\" using 1:2 with boxes lw 3 title \"Histogram\"\n",
		(unsigned long)set->size, fn);
	for(h = 0; h < hs; h++) fprintf(fp, "%lu\t%lu\n",
		(unsigned long)h, (unsigned long)histogram[h]);
	fclose(fp);
}

/** Assertion function for seeing if `set` is in a valid state.
 @order \O(|`set.bins`| + |`set.items`|) */
static void PS_(legit)(const struct S_(set) *const set) {
	struct PS_(entry) *e, *e_end;
	size_t size = 0;
	if(!set) return; /* Null state. */
	if(!set->entries) { /* Empty state. */
		assert(!set->log_capacity && !set->size);
		return;
	}
	assert(set->log_capacity >= 3);
	for(e = set->entries, e_end = e + (1 << set->log_capacity); e < e_end; e++)
		if(e->next != SETm2) size++;
	assert(set->size == size);
}

/** Passed `parent_new` and `parent` from <fn:<S>set_test>. */
static void PS_(test_basic)(PS_(type) (*const parent_new)(void *),
	void *const parent) {
	struct test { PS_(type) elem; int is_in, unused; } test[10000], *t, *t_end;
	const size_t test_size = sizeof test / sizeof *test;
	int success;
	char z[12];
	size_t removed = 0, collision = 0;
	struct PS_(entry) *b, *b_end;
	struct S_(set) set = SET_IDLE;
	PS_(type) eject;
	assert(test_size > 1);
	memset(&test, 0, sizeof test);
	/* Test empty. */
	PS_(legit)(&set);
	S_(set)(&set);
	assert(!set.entries && !set.log_capacity && !set.size);
	PS_(legit)(&set);
	PS_(graph)(&set, "graph/" QUOTE(SET_NAME) "-0.gv");
	/* Test placing items. */
	for(t = test, t_end = t + test_size; t < t_end; t++) {
		size_t n = (size_t)(t - test);
		if(parent_new && !(t->elem = parent_new(parent))) { assert(0); return; }
		/*PS_(filler)(t->elem);*/
		PS_(to_string)(&t->elem, &z);
		printf("%lu: came up with %s.\n", (unsigned long)n, z);
		/*success = S_(set_reserve)(&set, 1);
		assert(success && set.entries);
		if(n == 0) assert(set.log_capacity == 3 && !set.size
			&& !set.entries[0].first && !set.entries[1].first
			&& !set.entries[2].first && !set.entries[3].first
			&& !set.entries[4].first && !set.entries[5].first
			&& !set.entries[6].first && !set.entries[7].first); */
		eject = S_(set_put)(&set, t->elem);
		if(n == 0) assert(!eject && set.size == 1);
#if 0
		else if(eject) {
			if(!parent_new) {
				((struct Test *)(void *)((char *)eject
					- offsetof(struct Test, space)))->is_in = 0;
			} else {
				struct Test *sub_t, *sub_t_end;
				/* Slow way; we have got a one-way `test -> elem` so it
				 necessitates a linear search if we want to clear `is_in`. */
				for(sub_t = test, sub_t_end = t; sub_t < sub_t_end; sub_t++) {
					if(!sub_t->is_in
						|| !PS_(equal)(PS_(pointer)(&eject->key),
						PS_(pointer)(&sub_t->elem->key))) continue;
					sub_t->is_in = 0;
					break;
				}
				if(t == t_end) assert(0);
			}
		}
		t->is_in = 1;
#endif
		if(set.size == 130 || set.size == 129
			|| set.size < 1000000 && !(n & (n - 1))) {
			char fn[64];
			sprintf(fn, "graph/" QUOTE(SET_NAME) "-%u.gv",
				(unsigned)n + 1);
			printf("*** graph %s: set %s.\n", fn, PS_(set_to_string)(&set));
			PS_(graph)(&set, fn);
		}
		PS_(legit)(&set);
	}
	{
		char fn[64];
		/*PS_(stats)(&set, "\n", stdout);*/
		printf("\n");
		sprintf(fn, "graph/" QUOTE(SET_NAME) "-%u-final.gv",
			(unsigned)test_size + 1);
		PS_(graph)(&set, fn);
		sprintf(fn, "graph/histogram-" QUOTE(SET_NAME) "-%u.gnu",
			(unsigned)test_size + 1);
		PS_(histogram)(&set, fn);
	}
	printf("Testing get from set.\n");
	/* This is more debug info.
	printf("[ ");
	for(t = test, t_end = t + test_size; t < t_end; t++) {
		PS_(to_string)(&t->elem->data, &a);
		printf("%s[%lu-%s]%s", t == test ? "" : ", ",
			(unsigned long)(t - test), t->is_in ? "yes" : "no", a);
	}
	printf(" ]\n");*/
#if 0
	for(t = test, t_end = t + test_size; t < t_end; t++) {
		const size_t n = (size_t)(t - test);
		struct S_(setlink) *r;
		if(!(n & (n - 1))) {
			PS_(to_string)(&t->elem->key, &a);
			fprintf(stderr, "%lu: retrieving %s.\n", (unsigned long)n, a);
		}
		element = S_(set_get)(&set, PS_(pointer)(&t->elem->key));
		assert(element);
		if(t->is_in) {
			assert(element == t->elem);
			if(rand() < RAND_MAX / 8) {
				removed++;
				r = S_(set_remove)(&set, PS_(pointer)(&t->elem->key));
				assert(r);
				r = S_(set_remove)(&set, PS_(pointer)(&t->elem->key));
				assert(!r);
				r = S_(set_policy_put)(&set, t->elem, 0);
				assert(!r);
				r = S_(set_policy_put)(&set, t->elem, 0);
				assert(r == t->elem);
				r = S_(set_remove)(&set, PS_(pointer)(&t->elem->key));
				assert(r);
			}
		} else {
			const size_t count = set.size;
			collision++;
			assert(t && element != t->elem);
			r = S_(set_policy_put)(&set, t->elem, 0);
			assert(r == t->elem && count == set.size);
		}
	}
	printf("Collisions: %lu; removed: %lu.\n",
		(unsigned long)collision, (unsigned long)removed);
	PS_(legit)(&set);
	PS_(stats)(&set, "\n", stdout);
	S_(set_clear)(&set);
	for(b = set.entries, b_end = b + (1 << set.log_capacity); b < b_end; b++)
		assert(!PS_(count)(b));
	assert(set.size == 0);
#endif
	printf("Clear: %s.\n", PS_(set_to_string)(&set));
	S_(set_)(&set);
	assert(!set.entries && !set.log_capacity && !set.size);
}

/** The list will be tested on `stdout`. Requires `SET_TEST` to be a
 <typedef:<PS>action_fn> and `SET_TO_STRING`.
 @param[parent_new] Specifies the dynamic up-level creator of the parent
 `struct`. Could be null; then testing will be done statically on an array of
 <tag:<S>setlink> and `SET_TEST` is not allowed to go over the limits of the
 data type. @param[parent] The parameter passed to `parent_new`. Ignored if
 `parent_new` is null. @allow */
static void S_(set_test)(PS_(type) (*const parent_new)(void *),
	void *const parent) {
	printf("<" QUOTE(SET_NAME) ">set of type <" QUOTE(SET_TYPE)
		"> was created using: SET_HASH <" QUOTE(SET_HASH) ">; "
		"SET_IS_EQUAL <" QUOTE(SET_IS_EQUAL) ">; "
#ifdef SET_RECALCULATE
		"SET_RECALCULATE; "
#endif
		"SET_TEST<" QUOTE(SET_TEST) ">; "
		"%stesting:\n", parent_new ? "parent type specified; " : "");
	PS_(test_basic)(parent_new, parent);
	fprintf(stderr, "Done tests of <" QUOTE(SET_NAME) ">set.\n\n");
}

#undef QUOTE
#undef QUOTE_
