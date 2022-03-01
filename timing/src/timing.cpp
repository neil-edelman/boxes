#include <unordered_set>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
/* "C and C++ are the same," hahahaha. C++ is a mess. */
extern "C" {
#include "orcish.h"
}

/* Inline-chained string set. */

/** <http://www.cse.yorku.ca/~oz/hash.html> @implements <string>hash_fn */
static size_t djb2_hash(const char *s) {
	const unsigned char *str = (const unsigned char *)s;
	size_t hash = 5381, c;
	while(c = *str++) hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	return hash;
}
/** @implements <string>is_equal_fn */
static int string_is_equal(const char *const a, const char *const b)
	{ return !strcmp(a, b); }
/** @implements <string>to_string_fn */
static void string_to_string(const char *const s, char (*const a)[12])
	{ strncpy(*a, s, sizeof(*a) - 1), (*a)[sizeof(*a) - 1] = '\0'; }
#define TABLE_NAME string
#define TABLE_KEY char *
#define TABLE_HASH &djb2_hash
#define TABLE_IS_EQUAL &string_is_equal
#define TABLE_EXPECT_TRAIT
#include "table.hpp"
#define TABLE_TO_STRING &string_to_string
#include "table.hpp"
struct str16 { char str[16]; };
#define POOL_NAME str16
#define POOL_TYPE struct str16
#include "pool.hpp"
static char *str16_from_pool(struct str16_pool *const s16s) {
	struct str16 *s16 = str16_pool_new(s16s);
	if(!s16) return 0;
	orcish(s16->str, sizeof s16->str);
	return s16->str;
}
/** @implements <string>test_new_fn */
static int str16_from_void(void *const s16s, char **const string) {
	return !!(*string = (char *)str16_from_pool((struct str16_pool *)s16s));
}


/* Set up a closed hash table for comparison. With optimizations, I get that
 the run-time is very close to the same. Performance-wise, the simplicity of
 the closed hash is the winner. However, open tables are a big improvement in
 usability over the inscrutable closed set. (Like why is there a linked-list?
 No one has time to read or understand the documentation.)

 This relies on the closed set branch being in a certain directory outside the
 project, which it isn't in general, (_ie_, one would have to make changes.) I
 expect strings are the basis of most use cases. */

/** This was before we solved pointers-pointers. */
static void pstring_to_string(char *const*const ps,
	char (*const a)[12]) { string_to_string(*ps, a); }
/** Bogus `a`. */
static void closed_fill(const char **const a) { assert(a && !a); }
static size_t djb2_var(char *const s) { return djb2_hash(s); }
static int string_is_var(char *const a, char *const b)
	{ return string_is_equal(a, b); }
#define SET_NAME closed
#define SET_TYPE char *
#define SET_UINT size_t
#define SET_HASH &djb2_var
#define SET_IS_EQUAL &string_is_var
/*#define SET_TEST &closed_fill*/
#define SET_EXPECT_TRAIT
#include "set.hpp"
#define SET_TO_STRING &pstring_to_string
#include "set.hpp"
#define ARRAY_NAME closed
#define ARRAY_TYPE struct closed_setlink
#include "array.hpp"
/** Pair of elements stuck together for passing to tests. */
struct backing { struct str16_pool str16s; struct closed_array closed; };

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
	X(CLOSED, closed), \
	X(OPEN, open), \
	X(UNORDERED, unordered)

struct str_eq { bool operator()(const char *a, const char *b)
	/* noexcept: 'noexcept' is a keyword, yes, and I want to use that keyword,
	 wtv.*/ const { return !strcmp(a, b); } };
struct str_djb2hash { std::size_t operator()(const char* s) const {
	return djb2_hash(s); } };

int main(void) {
	FILE *gnu = 0;
	const char *name = "timing";
	size_t i, n = 1, e, replicas = 5;
#define X(n, m) n
	enum { EXPS };
#undef X
#define X(n, m) { #m, 0, { 0, 0.0, 0.0 } }
	struct { const char *name; FILE *fp; struct measure m; }
		exp[] = { EXPS };
	const size_t exp_size = sizeof exp / sizeof *exp;
#undef X
	struct closed_set cs = SET_IDLE;
	struct string_table os = TABLE_IDLE;
	struct backing backing = { POOL_IDLE, ARRAY_IDLE };
	closed_set(&cs);
	/* Open all graphs for writing. */
	for(e = 0; e < exp_size; e++) {
		char fn[64];
		if(sprintf(fn, "graph/%s.tsv", exp[e].name) < 0
			|| !(exp[e].fp = fopen(fn, "w"))) goto catch_;
		fprintf(exp[e].fp, "# %s\n"
			"# <items>\t<t (ms)>\t<sample error on t with %lu replicas>\n",
			exp[e].name, (unsigned long)replicas);
	}
	/* Do experiment. */
	for(n = 1; n < 10000000; n <<= 1) {
		clock_t t_total;
		size_t r;
		for(e = 0; e < exp_size; e++) m_reset(&exp[e].m);
		for(r = 0; r < replicas; r++) {
			std::unordered_set<char *, str_djb2hash, str_eq> us;
			clock_t t;
			struct str16 *s16;
			struct closed_setlink *link;
			t_total = clock();
			printf("Replica %lu/%lu.\n", r + 1, replicas);

			/* It crashes if I don't have this, but no idea why. */
			closed_set(&cs);
			string_table(&os);

			/* Sorted array; pre-allocate for fair test. Don't worry about
			 unused references. */
			str16_pool_clear(&backing.str16s);
			closed_array_clear(&backing.closed);
			for(i = 0; i < n; i++) {
				if(!(s16 = str16_pool_new(&backing.str16s))
				   || !(link = closed_array_new(&backing.closed))) goto catch_;
				orcish(s16->str, sizeof s16->str);
				link->key = s16->str;
				/*printf("Word pool %s\n", link->key); Spaammm. */
			}

			/* Set, (closed hash set.) (Don't put I/O in the test.) */
			t = clock();
			for(i = 0; i < n; i++) {
				link = backing.closed.data + i;
				if(closed_set_policy_put(&cs, link, 0))
					/*printf("Closed %s already.\n", link->key)*/;
			}
			m_add(&exp[CLOSED].m, diff_us(t));
			printf("Closed size %lu: %s.\n", (unsigned long)cs.size,
				closed_set_to_string(&cs));

			/* Table, (open hash set.) */
			t = clock();
			for(i = 0; i < n; i++) {
				char *const word = backing.closed.data[i].key;
				string_table_try(&os, word);
				/*switch(string_table_try(&os, word)) {
				case TABLE_ERROR: case TABLE_REPLACE: goto catch;
				case TABLE_YIELD: printf("Open %s already.\n", word); break;
				case TABLE_UNIQUE: printf("Open %s.\n", word); break;
				}*/
			}
			m_add(&exp[OPEN].m, diff_us(t));
			printf("Open size %lu: %s.\n", (unsigned long)os.size,
				string_table_to_string(&os));

			t = clock();
			for(i = 0; i < n; i++) us.insert(backing.closed.data[i].key);
			m_add(&exp[UNORDERED].m, diff_us(t));
			printf("Unordered size %lu.\n", (unsigned long)os.size);
			//for(const char *w : us) printf("word: %s\n", w);

			/* Took took much time; decrease the replicas for next time. */
			if(replicas != 1
				&& 10.0 * (clock() - t_total) / CLOCKS_PER_SEC > 1.0 * replicas)
				replicas--;

			/* Cut a slice to see if it's actually working. */
			/*if(n == 1024) {
				char fn[64];
				sprintf(fn, "graph/%s.gv", exp[0].name);
				set_closed_graph(&closed, fn);
				sprintf(fn, "graph/%s.gv", exp[1].name);
				table_string_graph(&os, fn);
			}*/
			/*closed_set_clear(&closed);
			string_table_clear(&os);*/
			closed_set_(&cs);
			string_table_(&os);
		}
		for(e = 0; e < exp_size; e++) {
			double stddev = m_stddev(&exp[e].m);
			if(stddev != stddev) stddev = 0; /* Is nan; happens. */
			fprintf(exp[e].fp, "%lu\t%f\t%f\n",
				(unsigned long)n, m_mean(&exp[e].m), stddev);
		}
	}
	goto finally;
catch_:
	perror("timing"), assert(0);
finally:
	for(e = 0; e < exp_size; e++)
		if(exp[e].fp && fclose(exp[e].fp)) perror(exp[e].name);

	/* Output a `gnuplot` script. */
	{
		char fn[64];
		if(sprintf(fn, "graph/%s.gnu", name) < 0
			|| !(gnu = fopen(fn, "w"))) goto catch2;
		fprintf(gnu,
			"set style line 1 lt 5 lw 2 lc rgb '#0072bd'\n"
			"set style line 2 lt 5 lw 2 lc rgb '#ff0000'\n"
			"set style line 3 lt 5 lw 2 lc rgb '#00ac33'\n"
			"set style line 4 lt 5 lw 2 lc rgb '#19d3f5'\n");
		fprintf(gnu, "set term postscript eps enhanced color\n"
			/*"set encoding utf8\n" Doesn't work at all; {/Symbol m}. */
			"set output \"graph/%s.eps\"\n"
			"set grid\n"
			"set xlabel \"elements\"\n"
			"set ylabel \"time per element, t (ns)\"\n"
			"set yrange [0:]\n"
			"set log x\n"
			"plot", name);
		for(e = 0; e < exp_size; e++) fprintf(gnu,
			"%s \\\n\"graph/%s.tsv\" using 1:($2/$1*1000):($3/$1*1000) "
			"with errorlines title \"%s\" ls %d", e ? "," : "",
			exp[e].name, exp[e].name, (int)e + 1);
		fprintf(gnu, "\n");
	}
	if(gnu && fclose(gnu)) goto catch2; gnu = 0;
	{
		int result;
		char cmd[64];
		fprintf(stderr, "Running Gnuplot to get a graph of, \"%s,\" "
			"(http://www.gnuplot.info/.)\n", name);
		if((result = system("/usr/local/bin/gnuplot --version")) == -1)
			goto catch2;
		else if(result != EXIT_SUCCESS) { errno = EDOM; goto catch2; }
		if(sprintf(cmd, "/usr/local/bin/gnuplot graph/%s.gnu", name) < 0
			|| (result = system(cmd)) == -1) goto catch2;
		else if(result != EXIT_SUCCESS) { errno = EDOM; goto catch2; }
		fprintf(stderr, "Running open.\n");
		if(sprintf(cmd, "open graph/%s.eps", name) < 0
		   || (result = system(cmd)) == -1) goto catch2;
		else if(result != EXIT_SUCCESS) { errno = EDOM; goto catch2; }
	}
	goto finally2;
catch2:
	perror(name);
finally2:
	if(gnu && fclose(gnu)) perror(name);
	printf("\n");
	return EXIT_SUCCESS;
}
