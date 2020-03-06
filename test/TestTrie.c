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
#include <math.h>   /* sqrt NAN? */
#ifndef NAN
#define NAN (0./0.)
#endif
#include "Orcish.h"


extern const char *const parole[];
extern const size_t parole_size;

/** For now, it's just a placeholder to get `graph()`. */
static void fill_str(const char *str) {
	/* nothing */ (void)(str);
}

#define TRIE_NAME Str
#define TRIE_TEST &fill_str
#include "../src/Trie.h"



/** For comparison with sorted array. */

static void str_to_str(const char *const*str, char(*const a)[12]) {
	sprintf(*a, "%.11s", *str);
}

#define ARRAY_NAME Str
#define ARRAY_TYPE const char *
#define ARRAY_TO_STRING &str_to_str
#include "../src/Array.h"

static size_t lower_bound(const struct StrArray *const array,
	const char *const value) {
	const char **const data = array->data;
	size_t low = 0, mid, high = array->size;
	assert(array && value);
	while(low < high) {
		mid = low + ((high - low) >> 1);
		/*printf("[%lu, %lu) cmp (%s) to %s\n", low, high, value, data[mid]);*/
		if(strcmp(value, data[mid]) <= 0) {
			high = mid;
		} else {
			low = mid + 1;
		}
	}
	return low;
}

static int array_insert(struct StrArray *const array,
	const char *const data) {
	size_t b;
	assert(array && data);
	b = lower_bound(array, data);
	/*printf("lb(%s) = %lu.\n", data, b);*/
	StrArrayBuffer(array, 1);
	memmove(array->data + b + 1, array->data + b,
		sizeof *array->data * (array->size - b - 1));
	array->data[b] = data;
	return 1;
}

static int array_cmp(const void *a, const void *b) {
	return strcmp((const char *)a, *(const char **)b);
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
static void pointer_to_string(const char *const*const ps,
	char (*const a)[12]) {
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



static void test_basic_trie_str() {
	struct StrTrie trie = TRIE_IDLE;
	union trie_Str_TrieNode *n;

	trie_Str_print(&trie);
	trie_Str_graph(&trie, "graph/trie0.gv");
	/*printf("Trie0: %s.\n\n", StrTrieToString(&trie));*/

	if(!StrTrieAdd(&trie, "foo")) goto catch;
	trie_Str_print(&trie);
	trie_Str_graph(&trie, "graph/trie1.gv");
	/*printf("Trie1: %s.\n\n", StrTrieToString(&trie));*/

	if(!StrTrieAdd(&trie, "bar")) goto catch;
	trie_Str_print(&trie);
	trie_Str_graph(&trie, "graph/trie2.gv");
	/*printf("Trie2: %s.\n\n", StrTrieToString(&trie));*/

	if(!StrTrieAdd(&trie, "baz")) goto catch;
	trie_Str_print(&trie);
	trie_Str_graph(&trie, "graph/trie3.gv");
	/*printf("Trie3: %s.\n\n", StrTrieToString(&trie));*/

	if(!StrTrieAdd(&trie, "qux")) goto catch;
	trie_Str_print(&trie);
	trie_Str_graph(&trie, "graph/trie4.gv");
	/*printf("Trie4: %s.\n\n", StrTrieToString(&trie));*/

	if(!StrTrieAdd(&trie, "quxx")) goto catch;
	trie_Str_print(&trie);
	trie_Str_graph(&trie, "graph/trie5.gv");
	/*printf("Trie5: %s.\n\n", StrTrieToString(&trie));*/

	if(!StrTrieAdd(&trie, "quxxx")) goto catch;
	trie_Str_print(&trie);
	trie_Str_graph(&trie, "graph/trie6.gv");
	/*printf("Trie6: %s.\n\n", StrTrieToString(&trie));*/

	if(!StrTrieAdd(&trie, "a")) goto catch;
	trie_Str_graph(&trie, "graph/trie_a.gv");
	if(!StrTrieAdd(&trie, "b")) goto catch;
	trie_Str_graph(&trie, "graph/trie_b.gv");
	trie_Str_print(&trie);
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

	n = trie_Str_match(&trie, "");
	printf("\"\": %s\n", n ? n->leaf : "null");
	n = trie_Str_match(&trie, "foo");
	printf("\"foo\": %s\n", n ? n->leaf : "null");
	n = trie_Str_match(&trie, "qux");
	printf("\"qux\": %s\n", n ? n->leaf : "null");
	n = trie_Str_match(&trie, "quxx");
	printf("\"quxx\": %s\n", n ? n->leaf : "null");
	n = trie_Str_match(&trie, "quux");
	printf("\"quux\": %s\n", n ? n->leaf : "null");
	goto finally;
catch:
	printf("Test failed.\n");
	assert(0);
finally:
	StrTrie_(&trie);
}

/** Returns a time diffecence in microseconds from `then`. */
static double diff_us(clock_t then) {
	return 1000000.0 / CLOCKS_PER_SEC * (clock() - then);
}

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
#define STRUCT(A) { #A, 0, { 0, 0, 0 }, 0 }
#define ES(X) X(ARRAYINIT), X(ARRAYLOOK), \
	X(TRIEINIT), X(TRIELOOK), \
	X(SETINIT), X(SETLOOK)

/* fixme: All the data should be the same and permuted randomly. */
static int test(void) {
	struct StrTrie trie = TRIE_IDLE;
	struct StrArray array = ARRAY_IDLE;
	/* Linked hash map -- too much code.
	struct Str24EntryPool entries = POOL_IDLE;
	struct Str24Set set = SET_IDLE;
	struct Str24List list;*/
	struct StringSet set = SET_IDLE;
	struct StringElementPool set_pool = POOL_IDLE;
	size_t i, r, s = 1, e;
	const size_t replicas = 3;
	clock_t t;
	int success = 1, is_full = 0;
	/* How many files we open simultaneously qs.size OR gnu.size. */
	enum { ES(PARAM) };
	struct { const char *name; FILE *fp; struct Measure m; int is_long; }
		es[] = { ES(STRUCT) }, gnu = { "experiment", 0, {0,0,0}, 0 };
	const size_t es_size = sizeof es / sizeof *es;

	fprintf(stderr, "parole_size %lu\n", (unsigned long)parole_size);

	/* Open all graphs for writing. */
	for(e = 0; e < es_size; e++) {
		char fn[64];
		if(sprintf(fn, "graph/%s.tsv", es[e].name) < 0
			|| !(es[e].fp = fopen(fn, "w"))) goto catch;
		fprintf(es[e].fp, "# %s\n"
			"# <items>\t<t (ms)>\t<sample error on t with %lu replicas>\n",
			es[e].name, (unsigned long)replicas);
	}
	for(s = 1; !is_full; s <<= 1) {
		/*if(s > 10000) break;*/
		if(s >= parole_size) is_full = 1, s = parole_size;
		for(e = 0; e < es_size; e++) m_reset(&es[e].m);
		for(r = 0; r < replicas; r++) {
			printf("Replica %lu.\n", r + 1);

			/* Trie. */

			StrTrieClear(&trie);
			t = clock();
			for(i = 0; i < s; i++)
				if(!StrTriePut(&trie, parole[i], 0)) goto catch;
			m_add(&es[TRIEINIT].m, diff_us(t));
			printf("Added init trie size %lu: %s.\n",
				(unsigned long)StrTrieSize(&trie), StrTrieToString(&trie));
			t = clock();
			for(i = 0; i < s; i++) {
				volatile const char *const str = StrTrieGet(&trie, parole[i]);
				assert(str);
				(void)str;
			}
			m_add(&es[TRIELOOK].m, diff_us(t));
			printf("Added look trie size %lu.\n",
				(unsigned long)StrTrieSize(&trie));

			/* Sorted array. */

			if(es[ARRAYINIT].is_long || es[ARRAYLOOK].is_long) goto array_end;
			StrArrayClear(&array);
			t = clock();
			for(i = 0; i < s; i++) array_insert(&array, parole[i]);
			m_add(&es[ARRAYINIT].m, diff_us(t));
			printf("Added init array size %lu: %s.\n",
				(unsigned long)StrArraySize(&array), StrArrayToString(&array));
			if(diff_us(t) > 20000) es[ARRAYINIT].is_long = 1;
			t = clock();
			for(i = 0; i < s; i++) {
				/* On systems which have differing pointer sizes, the casts are
				 problematic, but lazy. */
				volatile const char **const str = bsearch(parole[i],
					array.data, array.size, sizeof(array.data), array_cmp);
				assert(str);
			}
			m_add(&es[ARRAYLOOK].m, diff_us(t));
			printf("Added look array size %lu.\n",
				(unsigned long)StrArraySize(&array));
array_end:

			/* Set, (hash map.) */
			
			StringSetClear(&set);
			StringElementPoolClear(&set_pool);
			t = clock();
			for(i = 0; i < s; i++) {
				struct StringSetElement *elem = StringElementPoolNew(&set_pool);
				elem->key = parole[i];
				if(StringSetPolicyPut(&set, elem, 0))
					StringElementPoolRemove(&set_pool, elem);
			}
			m_add(&es[SETINIT].m, diff_us(t));
			printf("Added init set size %lu: %s.\n",
				(unsigned long)StringSetSize(&set), StringSetToString(&set));
			t = clock();
			for(i = 0; i < s; i++) {
				struct StringSetElement *const elem
					= StringSetGet(&set, parole[i]);
				assert(elem);
				/*printf("looking for %s.\n", parole[i]);*/
			}
			m_add(&es[SETLOOK].m, diff_us(t));
			printf("Added look set size %lu.\n",
				(unsigned long)StringSetSize(&set));
			
			
#if 0
			/* Linked set. */

			Str24SetClear(&set);
			Str24ListClear(&list);
			Str24EntryPoolClear(&entries);
			t = clock();
			for(i = 0; i < s; i++) {
				struct Str24Entry *const n = Str24EntryPoolNew(&entries);
				strcpy(n->s24e.key.a, parole[i]); /* Should pause the timing? */
				if(Str24SetPolicyPut(&set, &n->s24e, 0)) /* Duplicate. */
					{ Str24EntryPoolRemove(&entries, n); continue; }
				Str24ListPush(&list, &n->s24n);
			}
			Str24ListSort(&list);
			m_add(&es[SETINIT].m, diff_us(t));
			printf("Added init set size %lu: %s.\n",
				(unsigned long)Str24SetSize(&set), Str24SetToString(&set));
			t = clock();
			for(i = 0; i < s; i++) {
				printf("looking for %s.\n", parole[i]);
				struct Str24SetElement *const s24 = Str24SetGet(&set, parole[i]);
				assert(s24);
			}
			m_add(&es[SETLOOK].m, diff_us(t));
			printf("Added look set size %lu.\n",
				(unsigned long)StrSetSize(&set));
#endif

		}
		for(e = 0; e < es_size; e++) fprintf(es[e].fp, "%lu\t%f\t%f\n",
			(unsigned long)s, m_mean(&es[e].m), m_stddev(&es[e].m));
	}
	printf("Test passed.\n");
	goto finally;
catch:
	success = 0;
	perror("test");
	printf("Test failed.\n");
finally:
	StrArray_(&array);
	/*EntryPool_(&entries);
	StrSet_(&set);*/
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
		/*"set style line 1 lt 1 lw 3 lc rgb '#0072bd' # blue\n"
		"set style line 2 lt 1 lw 3 lc rgb '#d95319' # orange\n"
		"set style line 3 lt 1 lw 3 lc rgb '#edb120' # yellow\n"
		"set style line 4 lt 1 lw 3 lc rgb '#7e2f8e' # purple\n"
		"set style line 5 lt 1 lw 3 lc rgb '#77ac30' # green\n"
		"set style line 6 lt 1 lw 3 lc rgb '#4dbeee' # light-blue\n"
		"set style line 7 lt 1 lw 3 lc rgb '#a2142f' # red\n"*/
		fprintf(gnu.fp, "set style line 1 lw 3 lc rgb '#0072ff'\n"
			"set style line 2 lw 3 lc rgb '#ff7200'\n"
			"set style line 3 lw 3 lc rgb '#0072cc'\n"
			"set style line 4 lw 3 lc rgb '#cc7200'\n"
			"set style line 5 lw 3 lc rgb '#007299'\n"
			"set style line 6 lw 3 lc rgb '#997200'\n");
		fprintf(gnu.fp, "set term postscript eps enhanced color\n"
			/*"set encoding utf8\n" Doesn't work at all; {/Symbol m}. */
			"set output \"graph/%s.eps\"\n"
			"set grid\n"
			"set xlabel \"elements\"\n"
			"set ylabel \"amortised time per unit, t (ns)\"\n"
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

int main(void) {
	unsigned seed = (unsigned)clock();
	srand(seed), rand(), printf("Seed %u.\n", seed);
	fprintf(stderr, "TrieInternal %lu\n"
		"size_t %lu\n"
		"Type * %lu\n"
		"union <PN>TrieNode %lu\n",
		sizeof(struct TrieInternal),
		sizeof(size_t),
		sizeof(trie_Str_Type *),
		sizeof(union trie_Str_TrieNode));
	test_basic_trie_str();
	printf("\n***\n\n");
	test();
	return EXIT_SUCCESS;
}
