#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "orcish.h"

struct str32 { char str[32]; };
#define POOL_NAME str32
#define POOL_TYPE struct str32
#include "../../../test/pool.h"

#define ARRAY_NAME str
#define ARRAY_TYPE char *
#include "../../../test/array.h"

#define TRIE_NAME str
#include "../../../src/trie.h"

static size_t djb2_hash(const char *s) {
	const unsigned char *str = (const unsigned char *)s;
	size_t hash = 5381, c;
	while(c = *str++) hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	return hash;
}
static int str_is_equal(const char *const a, const char *const b)
	{ return !strcmp(a, b); }
#define TABLE_NAME str
#define TABLE_KEY char *
#define TABLE_HASH &djb2_hash
#define TABLE_IS_EQUAL &str_is_equal
#include "table.h"

/* FIXME: this should not be necessary, obv. */
#undef B_
#undef PB_

#define TREE_NAME str
#define TREE_KEY char *
#define TREE_COMPARE &strcmp
#include "../src/tree.h"

#include <math.h>
#ifndef NAN
#define NAN (0.0/0.0)
#endif
#include <time.h>
/** Returns a time difference in microseconds from `then`. */
static double diff_us(clock_t then)
	{ return 1000000.0 / CLOCKS_PER_SEC * (clock() - then); }
/** On-line numerically stable first-order statistics, <Welford, 1962, Note>. */
struct measure { size_t count; double mean, ssdm; };
static void m_reset(struct measure *const m)
	{ m->count = 0, m->mean = m->ssdm = 0; }
static void m_add(struct measure *const m, const double replica) {
	const size_t n = ++m->count;
	const double delta = replica - m->mean;
	m->mean += delta / n;
	m->ssdm += delta * (replica - m->mean);
}
static double m_mean(const struct measure *const m)
	{ return m->count ? m->mean : (double)NAN; }
static double m_sample_variance(const struct measure *const m)
	{ return m->count > 1 ? m->ssdm / (m->count - 1) : (double)NAN; }
static double m_stddev(const struct measure *const m)
	{ return sqrt(m_sample_variance(m)); }

#define EXPS \
	X(TRIE_ADD, trie-add), \
	X(TRIE_LOOK, trie-look), \
	X(TREE_ADD, tree-add), \
	X(TREE_LOOK, tree-look), \
	X(TABLE_ADD, table-add), \
	X(TABLE_LOOK, table-look)

/** <https://theunixzoo.co.uk/blog/2021-10-14-preventing-optimisations.html> */
static inline void DoNotOptimize(void *value) {
	asm volatile("" : "+r,m"(value) : : "memory");
}

int main(void) {
	const unsigned replicas = 5;
	unsigned n, e;
#define X(n, m) n
	enum { EXPS };
#undef X
#define X(n, m) { #m, 0, { 0, 0.0, 0.0 } }
	struct { const char *name; FILE *fp; struct measure m; }
		exp[] = { EXPS };
	const size_t exp_size = sizeof exp / sizeof *exp;
#undef X
	struct str32_pool str_pool = str32_pool();
	struct str_array orcs = str_array();
	struct str_table table = str_table();
	struct str_tree tree = str_tree();
	struct str_trie trie = str_trie();
	int success = EXIT_SUCCESS;
	/* Open all graphs for writing. */
	for(e = 0; e < exp_size; e++) {
		char fn[64];
		if(sprintf(fn, "graph/%s.tsv", exp[e].name) < 0
			|| !(exp[e].fp = fopen(fn, "w"))) goto catch;
		fprintf(exp[e].fp, "# %s\n"
			"# <items>\t<t (ms)>\t<sample error on t with %u replicas>\n",
			exp[e].name, replicas);
	}
	/* Do experiment. */
	for(n = 1; n < 10000000; n <<= 1) {
		unsigned r;
		for(e = 0; e < exp_size; e++) m_reset(&exp[e].m);
		for(r = 0; r < replicas; r++) {
			clock_t t;
			char **o, **o_end;
			{ /* Set up a random test beforehand. */
				struct str32 *orc;
				unsigned i;
				for(i = 0; i < n; i++) {
					if(!(orc = str32_pool_new(&str_pool))
						|| !(o = str_array_new(&orcs))) goto catch;
					orcish(orc->str, sizeof orc->str);
					*o = orc->str;
					/*printf("Stored: %s\n", *o);*/
				}
			}
			fprintf(stderr, "Replica %u/%u of %u elements.\n",
				r + 1, replicas, n);

			/* Time table. */
			printf("Table.\n");
			t = clock();
			for(o = orcs.data, o_end = o + orcs.size; o < o_end; o++) {
				switch(str_table_try(&table, *o)) {
				case TABLE_ERROR:
				case TABLE_REPLACE: goto catch;
				case TABLE_UNIQUE: /*printf("%s.\n", *o);*/ break;
				case TABLE_YIELD: /*printf("%s already.\n", *o);*/ break;
				}
			}
			m_add(&exp[TABLE_ADD].m, diff_us(t));
			t = clock();
			{
				struct str_table_iterator it = str_table_begin(&table);
				char *str, *look = 0;
				while(str_table_next(&it, &str)) {
					/*fprintf(stderr, "->%s\n", str);*/
					DoNotOptimize(look);
					look = str_table_get_or(&table, str, 0);
					DoNotOptimize(look);
					assert(str == look);
				}
			}
			m_add(&exp[TABLE_LOOK].m, diff_us(t));
			str_table_clear(&table);
			str_table_(&table);

			/* Time trie. */
			printf("Trie.\n");
			t = clock();
			for(o = orcs.data, o_end = o + orcs.size; o < o_end; o++) {
				switch(str_trie_try(&trie, *o)) {
				case TRIE_ERROR: goto catch;
				case TRIE_UNIQUE: /*printf("%s.\n", *o);*/ break;
				case TRIE_PRESENT: /*printf("%s already.\n", *o);*/ break;
				}
			}
			m_add(&exp[TRIE_ADD].m, diff_us(t));
			t = clock();
			{
				struct str_trie_iterator it = str_trie_prefix(&trie, "");
				const char **str, **look = 0;
				while(str = str_trie_next(&it)) {
					/*fprintf(stderr, "->%s\n", *str);*/
					DoNotOptimize(look);
					look = str_trie_get(&trie, *str);
					DoNotOptimize(look);
					assert(*str == *look);
				}
			}
			m_add(&exp[TRIE_LOOK].m, diff_us(t));
			str_trie_clear(&trie);

			/* Time tree. */
			printf("Tree.\n");
			t = clock();
			for(o = orcs.data, o_end = o + orcs.size; o < o_end; o++) {
				switch(str_tree_try(&tree, *o)) {
				case TREE_ERROR: goto catch;
				case TREE_UNIQUE: /*printf("%s.\n", *o);*/ break;
				case TREE_PRESENT: /*printf("%s already.\n", *o);*/ break;
				}
			}
			m_add(&exp[TREE_ADD].m, diff_us(t));
			t = clock();
			{
				struct str_tree_cursor it = str_tree_begin(&tree);
				char **str, *look = 0;
				while(str = str_tree_next(&it)) {
					/*fprintf(stderr, "->%s\n", *str);*/
					DoNotOptimize(look);
					look = str_tree_get_or(&tree, *str, 0);
					DoNotOptimize(look);
					assert(*str == look);
				}
			}
			m_add(&exp[TREE_LOOK].m, diff_us(t));
			str_tree_clear(&tree);

			str_array_clear(&orcs);
			str32_pool_clear(&str_pool);
		}
		for(e = 0; e < exp_size; e++) {
			double stddev = m_stddev(&exp[e].m), mean = m_mean(&exp[e].m);
			if(stddev != stddev) stddev = 0; /* Is nan; happens. */
			/* # attempted size, mean (us), stddev (us)\n */
			fprintf(exp[e].fp, "%u\t%f\t%f\n", n, mean, stddev);
		}
	}
	goto finally;
catch:
	success = EXIT_FAILURE;
	perror("timing");
finally:
	str_trie_(&trie);
	str_tree_(&tree);
	str_table_(&table);
	str_array_(&orcs);
	str32_pool_(&str_pool);
	return success;
}
