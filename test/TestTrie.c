/** @license 2020 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 Test Trie.

 @std C89/90 */

#include <stdlib.h> /* EXIT malloc free rand */
#include <stdio.h>  /* *printf */
#include <assert.h> /* assert */
#include <errno.h>  /* errno */
#include <string.h> /* strncpy */
#include <time.h>   /* clock time */
#include <math.h>   /* sqrt NAN? for stddev */
#ifndef NAN
#define NAN (0./0.)
#endif
#include "Orcish.h"


/** Just a placeholder to get `graph()`; <fn:StrTrieTest> will crash. */
static void fill_str(const char *str) { (void)(str); }

#define TRIE_NAME Str
#define TRIE_TO_STRING
#define TRIE_TEST &fill_str
#include "../src/Trie.h"

/** Specific test for str. */
static void test_basic_trie_str(void) {
	struct StrTrie trie = TRIE_IDLE;
	const char *words[] = { "foo", "bar", "baz", "qux", "quux" };
	const size_t words_size = sizeof words / sizeof *words;
	const char *wordsr[] = { "", "foo", "qux", "quxx", "quux", "foo" };
	const size_t wordsr_size = sizeof wordsr / sizeof *wordsr;
	const char *alph[] = { "m", "n", "o", "u", "v", "x", "y", "z", "p", "q",
		"r", "", "å", "a", "b", "g", "h", "i", "j", "k", "l", "c", "d", "e",
		"f", "s", "t", "w" };
	const size_t alph_size = sizeof alph / sizeof *alph;
	size_t i;

	assert(StrTrieRemove(&trie, "") == 0);

	trie_Str_print(&trie);
	trie_Str_graph(&trie, "graph/trie0.gv");
	/*printf("Trie0: %s.\n\n", StrTrieToString(&trie));*/

	if(!StrTrieAdd(&trie, "foo")) goto catch;
	/*trie_Str_print(&trie);*/
	trie_Str_graph(&trie, "graph/trie1.gv");
	/*printf("Trie1: %s.\n\n", StrTrieToString(&trie));*/

	if(!StrTrieAdd(&trie, "bar")) goto catch;
	/*trie_Str_print(&trie);*/
	trie_Str_graph(&trie, "graph/trie2.gv");
	/*printf("Trie2: %s.\n\n", StrTrieToString(&trie));*/

	if(!StrTrieAdd(&trie, "baz")) goto catch;
	/*trie_Str_print(&trie);*/
	trie_Str_graph(&trie, "graph/trie3.gv");
	/*printf("Trie3: %s.\n\n", StrTrieToString(&trie));*/

	if(!StrTrieAdd(&trie, "qux")) goto catch;
	/*trie_Str_print(&trie);*/
	trie_Str_graph(&trie, "graph/trie4.gv");
	/*printf("Trie4: %s.\n\n", StrTrieToString(&trie));*/

	if(!StrTrieAdd(&trie, "quxx")) goto catch;
	/*trie_Str_print(&trie);*/
	trie_Str_graph(&trie, "graph/trie5.gv");
	/*printf("Trie5: %s.\n\n", StrTrieToString(&trie));*/

	if(!StrTrieAdd(&trie, "quxxx")) goto catch;
	/*trie_Str_print(&trie);*/
	trie_Str_graph(&trie, "graph/trie6.gv");
	/*printf("Trie6: %s.\n\n", StrTrieToString(&trie));*/

	assert(StrTrieSize(&trie) == 6);

	if(!StrTrieAdd(&trie, "a")) goto catch;
	trie_Str_graph(&trie, "graph/trie_a.gv");
	if(!StrTrieAdd(&trie, "b")) goto catch;
	trie_Str_graph(&trie, "graph/trie_b.gv");
	/*trie_Str_print(&trie);*/
	if(!StrTrieAdd(&trie, "c")) goto catch;
	trie_Str_print(&trie);
	trie_Str_graph(&trie, "graph/trie_c.gv");
	if(!StrTrieAdd(&trie, "d")
	   || !StrTrieAdd(&trie, "e")
	   || !StrTrieAdd(&trie, "f")
	   || !StrTrieAdd(&trie, "g")
	   || !StrTrieAdd(&trie, "h")
	   || !StrTrieAdd(&trie, "i")
	   || !StrTrieAdd(&trie, "j")
	   || !StrTrieAdd(&trie, "k")
	   || !StrTrieAdd(&trie, "l")
	   || !StrTrieAdd(&trie, "m")
	   || !StrTrieAdd(&trie, "n")
	   || !StrTrieAdd(&trie, "o")
	   || !StrTrieAdd(&trie, "p")
	   || !StrTrieAdd(&trie, "q")
	   || !StrTrieAdd(&trie, "r")
	   || !StrTrieAdd(&trie, "s")
	   || !StrTrieAdd(&trie, "t")
	   || !StrTrieAdd(&trie, "u")
	   || !StrTrieAdd(&trie, "v")
	   || !StrTrieAdd(&trie, "w")
	   || !StrTrieAdd(&trie, "x")
	   || !StrTrieAdd(&trie, "y")
	   || !StrTrieAdd(&trie, "z")) goto catch;
	trie_Str_print(&trie);
	trie_Str_graph(&trie, "graph/trie_z.gv");
	printf("TrieZ: %s.\n\n", StrTrieToString(&trie));
	assert(StrTrieSize(&trie) == 26 + 6);
	if(!StrTrieRemove(&trie, "x")
		|| !StrTrieRemove(&trie, "z")
		|| !StrTrieRemove(&trie, "y")
		|| !StrTrieRemove(&trie, "d")
		|| !StrTrieRemove(&trie, "c")
		|| !StrTrieRemove(&trie, "b")
		|| !StrTrieRemove(&trie, "a")
		|| !StrTrieRemove(&trie, "f")
		|| !StrTrieRemove(&trie, "g")
		|| !StrTrieRemove(&trie, "h")
		|| !StrTrieRemove(&trie, "i")
		|| !StrTrieRemove(&trie, "j")
		|| !StrTrieRemove(&trie, "k")
		|| !StrTrieRemove(&trie, "l")
		|| !StrTrieRemove(&trie, "m")
		|| !StrTrieRemove(&trie, "n")
		|| !StrTrieRemove(&trie, "o")
		|| !StrTrieRemove(&trie, "p")
		|| !StrTrieRemove(&trie, "q")
		|| !StrTrieRemove(&trie, "r")
		|| !StrTrieRemove(&trie, "s")
		|| !StrTrieRemove(&trie, "t")
		|| !StrTrieRemove(&trie, "u")
		|| !StrTrieRemove(&trie, "v")
		|| !StrTrieRemove(&trie, "w")
		|| !StrTrieRemove(&trie, "e")) goto catch;
	trie_Str_graph(&trie, "graph/trie_a-z-delete.gv");
	assert(StrTrieSize(&trie) == 6);
	for(i = 0; i < words_size; i++)
		printf("\"%s\": %s\n", words[i], StrTrieMatch(&trie, words[i]));
	StrTrie_(&trie);

	printf("Trie from array.\n");
	if(!StrTrieFromArray(&trie, words, words_size, 0)) goto catch;
	trie_Str_graph(&trie, "graph/trie_all_at_once.gv");
	StrTrie_(&trie);
	if(!StrTrieFromArray(&trie, alph, alph_size, 0)) goto catch;
	trie_Str_graph(&trie, "graph/alph_all_at_once.gv");
	if(!StrTrieFromArray(&trie, wordsr, wordsr_size, 0)) goto catch;
	trie_Str_graph(&trie, "graph/trie_r_all_at_once.gv");
	StrTrie_(&trie);	
	goto finally;
catch:
	printf("Test failed.\n");
	assert(0);
finally:
	StrTrie_(&trie);
}


#ifndef DEBUG
#define TRIE_BENCHMARK
#endif

#ifdef TRIE_BENCHMARK /* <!-- benchmark */

static void pstr_to_str(const char *const*const pstr, char (*const a)[12])
	{ sprintf(*a, "%.11s", *pstr); }
static int pstr_cmp(const char *const*const pa, const char *const*const pb)
	{ return strcmp(*pa, *pb); }

/* For comparison with sorted array. */
#define ARRAY_NAME Str
#define ARRAY_TYPE const char *
#define ARRAY_UNFINISHED
#include "../src/Array.h"
#define ARRAY_TO_STRING &pstr_to_str
#define ARRAY_UNFINISHED
#include "../src/Array.h"
#define ARRAY_COMPARE &pstr_cmp
#include "../src/Array.h"

/** Fills `strs` with `words` of size `words_size` from `words_start` to
 `words_chosen`. */
static int array_fill(struct StrArray *const strs,
	const char *const*const words, const size_t words_size,
	const size_t words_start, const size_t words_chosen) {
	assert(strs && words && words_chosen && words_chosen <= words_size
		&& words_start < words_size);
	StrArrayClear(strs);
	if(!StrArrayBuffer(strs, words_chosen)) return 0;
	if(words_start + words_chosen > words_size) {
		const size_t size_a = words_size - words_start,
		size_b = words_chosen - size_a;
		memcpy(strs->data, words + words_start, sizeof *words * size_a);
		memcpy(strs->data + size_a, words, sizeof *words * size_b);
	} else {
		memcpy(strs->data, words + words_start, sizeof *words * words_chosen);
	}
	return 1;
}

/** For comparison with string set (hash.) */

/** Perform a 32 bit
 [Fowler/Noll/Vo FNV-1a](http://www.isthe.com/chongo/tech/comp/fnv/) hash on
 `str`. */
static unsigned fnv_32a_str(const char *const str) {
	const unsigned char *s = (const unsigned char *)str;
	/* 32 bit FNV-1 and FNV-1a non-zero initial basis, `FNV1_32A_INIT`. */
	unsigned hval = 0x811c9dc5;
	/* FNV magic prime `FNV_32_PRIME 0x01000193`. */
	while(*s) {
		hval ^= *s++;
		hval += (hval<<1) + (hval<<4) + (hval<<7) + (hval<<8) + (hval<<24);
	}
	return hval;
}
static int string_is_equal(const char *const a, const char *const b) {
	return !strcmp(a, b);
}
static void pointer_to_string(const char *const*const ps, char (*const a)[12]) {
	strncpy(*a, *ps, sizeof(*a) - 1);
	(*a)[sizeof(*a) - 1] = '\0';
}
#define SET_NAME String
#define SET_TYPE const char *
#define SET_HASH &fnv_32a_str
#define SET_IS_EQUAL &string_is_equal
#define SET_TO_STRING &pointer_to_string
#include "Set.h"

#define POOL_NAME StringElement
#define POOL_TYPE struct StringSetElement
#include "Pool.h"

/** Returns a time diffecence in microseconds from `then`. */
static double diff_us(clock_t then)
	{ return 1000000.0 / CLOCKS_PER_SEC * (clock() - then); }

/** On-line numerically stable first-order statistics, <Welford, 1962, Note>. */
struct Measure { size_t count; double mean, ssdm; };

static void m_reset(struct Measure *const measure)
	{ assert(measure); measure->count = 0, measure->mean = measure->ssdm = 0; }

static void m_add(struct Measure *const measure, const double replica) {
	const size_t n = ++measure->count;
	const double delta = replica - measure->mean;
	assert(measure);
	measure->mean += delta / n;
	measure->ssdm += delta * (replica - measure->mean);
}

static double m_mean(const struct Measure *const measure)
	{ assert(measure); return measure->count ? measure->mean : NAN; }

static double m_sample_variance(const struct Measure *const m)
	{ assert(m); return m->count > 1 ? m->ssdm / (m->count - 1) : NAN; }

static double m_stddev(const struct Measure *const measure)
	{ return sqrt(m_sample_variance(measure)); }

/* How many experiments is an X-macro. `gnuplot` doesn't like `_`. */
#define PARAM(A) A
#define STRUCT(A) { #A, 0, { 0, 0, 0 } }
#define ES(X) X(ARRAYINIT), X(ARRAYLOOK), \
	X(TRIEINIT), X(TRIELOOK), \
	X(HASHINIT), X(HASHLOOK)

static int timing_comparison(const char *const *const keys,
	const size_t keys_size) {
	struct StrTrie trie = TRIE_IDLE;
	struct StrArray array = ARRAY_IDLE;
	struct StringSet set = SET_IDLE;
	struct StringElementPool set_pool = POOL_IDLE;
	size_t i, r, n = 1, e, replicas = 5;
	clock_t t, t_total;
	int success = 1, is_full = 0;
	/* How many files we open simultaneously qs.size OR gnu.size. */
	enum { ES(PARAM) };
	struct { const char *name; FILE *fp; struct Measure m; }
		es[] = { ES(STRUCT) }, gnu = { "experiment", 0, {0,0,0} };
	const size_t es_size = sizeof es / sizeof *es;

	assert(keys && keys_size);

	/* Open all graphs for writing. */
	for(e = 0; e < es_size; e++) {
		char fn[64];
		if(sprintf(fn, "graph/%s.tsv", es[e].name) < 0
			|| !(es[e].fp = fopen(fn, "w"))) goto catch;
		fprintf(es[e].fp, "# %s\n"
			"# <items>\t<t (ms)>\t<sample error on t with %lu replicas>\n",
			es[e].name, (unsigned long)replicas);
	}
	for(n = 1; !is_full; n <<= 1) {
		if(n >= keys_size) is_full = 1, n = keys_size;
		for(e = 0; e < es_size; e++) m_reset(&es[e].m);
		for(r = 0; r < replicas; r++) {
			size_t start_i = rand() / (RAND_MAX / keys_size + 1);
			t_total = clock();
			printf("Replica %lu/%lu.\n", r + 1, replicas);

			/* Sorted array; pre-allocate for fair test. */
			array_fill(&array, keys, keys_size, start_i, n);
			t = clock();
			array_fill(&array, keys, keys_size, start_i, n);
			qsort(array.data, array.size, sizeof array.data,
				&array_Str_compar_anonymous);
			StrArrayCompactify(&array, 0);
			m_add(&es[ARRAYINIT].m, diff_us(t));
			printf("Added init array size %lu: %s.\n",
				(unsigned long)array.size, StrArrayToString(&array));
			t = clock();
			printf("Array: %s.\n", StrArrayToString(&array));
			for(i = 0; i < n; i++) {
				const char *const word = keys[(start_i + i) % keys_size],
					**const key = bsearch(&word, array.data, array.size,
					sizeof array.data, &array_Str_compar_anonymous);
				const int cmp = strcmp(word, *key);
				(void)cmp, assert(key && !cmp);
			}
			m_add(&es[ARRAYLOOK].m, diff_us(t));
			printf("Added look array size %lu.\n", (unsigned long)array.size);

			/* Set, (hash map.) */
			StringSetClear(&set);
			StringElementPoolClear(&set_pool);
			t = clock();
			for(i = 0; i < n; i++) {
				struct StringSetElement *elem = StringElementPoolNew(&set_pool);
				elem->key = keys[(start_i + i) % keys_size];
				if(StringSetPolicyPut(&set, elem, 0))
					StringElementPoolRemove(&set_pool, elem);
			}
			m_add(&es[HASHINIT].m, diff_us(t));
			printf("Added init hash size %lu: %s.\n",
				(unsigned long)StringSetSize(&set), StringSetToString(&set));
			t = clock();
			for(i = 0; i < n; i++) {
				const char *const word = keys[(start_i + i) % keys_size];
				const struct StringSetElement *const elem
					= StringSetGet(&set, word);
				const int cmp = strcmp(word, elem->key);
				(void)cmp, assert(elem && !cmp);
			}
			m_add(&es[HASHLOOK].m, diff_us(t));
			printf("Added look hash size %lu.\n",
				(unsigned long)StringSetSize(&set));

			/* Trie. */
			t = clock();
			array_fill(&array, keys, keys_size, start_i, n);
			StrTrieClear(&trie);
			StrTrieFromArray(&trie, array.data, array.size, 0);
			m_add(&es[TRIEINIT].m, diff_us(t));
			printf("Added init trie size %lu: %s.\n",
				(unsigned long)StrTrieSize(&trie), StrTrieToString(&trie));
			t = clock();
			for(i = 0; i < n; i++) {
				const char *const word = keys[(start_i + i) % keys_size],
					*const key = StrTrieGet(&trie, word);
				const int cmp = strcmp(word, key);
				(void)cmp, assert(key && !cmp);
			}
			m_add(&es[TRIELOOK].m, diff_us(t));
			printf("Added look trie size %lu.\n",
				(unsigned long)StrTrieSize(&trie));

			/* Took took much time; decrease the replicas for next time. */
			if(replicas != 1
				&& 10.0 * (clock() - t_total) / CLOCKS_PER_SEC > 1.0 * replicas)
				replicas--;
		}
		for(e = 0; e < es_size; e++) {
			double stddev = m_stddev(&es[e].m);
			if(stddev != stddev) stddev = 0; /* Is nan; happens. */
			fprintf(es[e].fp, "%lu\t%f\t%f\n",
				(unsigned long)n, m_mean(&es[e].m), stddev);
		}
		if(n == 512) trie_Str_graph(&trie, "graph/example.gv");
	}
	printf("Test passed.\n");
	goto finally;
catch:
	success = 0;
	perror("test");
	printf("Test failed.\n");
finally:
	StrArray_(&array);
	StringSet_(&set);
	StringElementPool_(&set_pool);
	StrTrie_(&trie);
	if(gnu.fp && fclose(gnu.fp)) perror(gnu.name);
	for(e = 0; e < es_size; e++)
		if(es[e].fp && fclose(es[e].fp)) perror(es[e].name);
	if(!success) return 0;

	/* Output a `gnuplot` script. */
	{
		char fn[64];
		if(sprintf(fn, "graph/%s.gnu", gnu.name) < 0
			|| !(gnu.fp = fopen(fn, "w"))) goto catch2;
		fprintf(gnu.fp, "set style line 1 lt 5 lw 2 lc rgb '#0072bd'\n"
			"set style line 2 lt 1 lw 3 lc rgb '#0072bd'\n"
			"set style line 3 lt 5 lw 2 lc rgb '#ff0000'\n" /* a2142f */
			"set style line 4 lt 1 lw 3 lc rgb '#ff0000'\n"
			"set style line 5 lt 5 lw 2 lc rgb '#00ac33'\n" /* 30ac77 */
			"set style line 6 lt 1 lw 3 lc rgb '#00ac33'\n");
		fprintf(gnu.fp, "set term postscript eps enhanced color\n"
			/*"set encoding utf8\n" Doesn't work at all; {/Symbol m}. */
			"set output \"graph/%s.eps\"\n"
			"set grid\n"
			"set xlabel \"elements\"\n"
			"set ylabel \"time per element, t (ns)\"\n"
			"set yrange [0:2000]\n"
			"set log x\n"
			"plot", gnu.name);
		for(e = 0; e < es_size; e++) fprintf(gnu.fp,
			"%s \\\n\"graph/%s.tsv\" using 1:($2/$1*1000):($3/$1*1000) "
			"with errorlines title \"%s\" ls %d", e ? "," : "",
			es[e].name, es[e].name, (int)e + 1);
		fprintf(gnu.fp, "\n");
	}
	if(gnu.fp && fclose(gnu.fp)) goto catch2; gnu.fp = 0;
#define ESYSTEM -300 /* C89 std does not define. */
	{
		int result;
		char cmd[64];
		fprintf(stderr, "Running Gnuplot to get a graph of, \"%s,\" "
			"(http://www.gnuplot.info/.)\n", gnu.name);
		if((result = system("/usr/local/bin/gnuplot --version")) == -1)
			goto catch2;
		else if(result != EXIT_SUCCESS) { errno = ESYSTEM; goto catch2; }
		if(sprintf(cmd, "/usr/local/bin/gnuplot graph/%s.gnu", gnu.name) < 0
			|| (result = system(cmd)) == -1) goto catch2;
		else if(result != EXIT_SUCCESS) { errno = ESYSTEM; goto catch2; }
		fprintf(stderr, "Running open.\n");
		if(sprintf(cmd, "open graph/%s.eps", gnu.name) < 0
		   || (result = system(cmd)) == -1) goto catch2;
		else if(result != EXIT_SUCCESS) { errno = ESYSTEM; goto catch2; }
	}
	goto finally2;
catch2:
	errno == ESYSTEM
		? fprintf(stderr, "gnu: automation system returned error\n")
		: (perror(gnu.name), 0);
finally2:
	if(gnu.fp && fclose(gnu.fp)) perror(gnu.name);
	return 1;
}

#endif /* benchmark --> */


struct Dict { char word[64]; int defn; };

static const char *dict_key(const struct Dict *const dict)
	{ return dict->word; }

static void fill_dict(struct Dict *const dict) {
	Orcish(dict->word, sizeof ((struct Dict *)0)->word);
	dict->defn = rand() / (RAND_MAX / 99 + 1);
}

#define TRIE_NAME Dict
#define TRIE_TYPE struct Dict
#define TRIE_KEY &dict_key
#define TRIE_TEST &fill_dict
#include "../src/Trie.h"


int main(void) {
	unsigned seed = (unsigned)clock();
	srand(seed), rand(), printf("Seed %u.\n", seed);
	test_basic_trie_str();
	(void)StrTrieTest; /* <- Not safe; `const char` is not generatable. */
	DictTrieTest();
	printf("\n***\n\n");
#ifdef TRIE_BENCHMARK /* <!-- bench */
	{
#if 0
		/* This is a dictionary defined in `parole_inglesi.c`. */
		extern const char *const parole[];
		extern const size_t parole_size;
		fprintf(stderr, "parole_size %lu\n", (unsigned long)parole_size);
		timing_comparison(parole, parole_size);
#else
		const char **orcs = 0;
		char *content = 0;
		const size_t orcs_size = 10000000, each = 64;
		size_t n;
		if(!(orcs = malloc(sizeof *orcs * orcs_size))
			|| !(content = malloc(sizeof *content * each * orcs_size)))
			goto catch;
		for(n = 0; n < orcs_size; n++) {
			orcs[n] = content + each * n;
			Orcish(content + each * n, each);
			/*printf("orc: %s.\n", orcs[n]);*/
		}
		timing_comparison(orcs, orcs_size);
		goto finally;
	catch:
		perror("orcs");
	finally:
		free(orcs), free(content);
#endif
	}
#endif /* !bench --> */
	return EXIT_SUCCESS;
}
