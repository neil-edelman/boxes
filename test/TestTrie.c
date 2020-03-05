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
/*typedef void(*<PT>ToString)(const T *, char(*)[12]);*/

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

static void foo(void) {
	struct StrArray array = ARRAY_IDLE;
	const char *const list[] = { "a", "b", "c", "d", "e", "f" };
	const char **pstr;
	size_t i;
	for(i = 0; i < sizeof list / sizeof *list; i++)
		pstr = StrArrayNew(&array), *pstr = list[i];
	printf("array: %s.\n", StrArrayToString(&array));
	array_insert(&array, "");
	array_insert(&array, "aa");
	array_insert(&array, "ba");
	array_insert(&array, "ca");
	array_insert(&array, "da");
	array_insert(&array, "ea");
	array_insert(&array, "fa");
	printf("array: %s.\n", StrArrayToString(&array));
	array_insert(&array, "a");
	array_insert(&array, "a");
	array_insert(&array, "a");
	array_insert(&array, "a");
	array_insert(&array, "a");
	array_insert(&array, "a");
	array_insert(&array, "a");
	printf("array: %s.\n", StrArrayToString(&array));
	printf("lb(foo)=%lu; lb(a)=%lu; lb()=%lu; lb(b)=%lu\n",
		lower_bound(&array, "foo"),
		   lower_bound(&array, "a"),
		   lower_bound(&array, ""),
		   lower_bound(&array, "b"));
}

/** For comparison with linked hash. */

static int string_is_equal(const char *const a, const char *const b) {
	return !strcmp(a, b);
}
static void pstring_to_string(const char *const*const ps, char (*const a)[12]) {
	strncpy(*a, *ps, sizeof(*a) - 1);
	(*a)[sizeof(*a) - 1] = '\0';
}
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
#define SET_NAME Str
#define SET_TYPE const char *
#define SET_HASH &fnv_32a_str
#define SET_IS_EQUAL &string_is_equal
#define SET_TO_STRING &pstring_to_string
#include "Set.h"

struct StrListNode;
static int str_list_compare(const struct StrListNode *,
	const struct StrListNode *);
static void str_list_to_string(const struct StrListNode *, char (*)[12]);
#define LIST_NAME Str
#define LIST_COMPARE &str_list_compare
#define LIST_TO_STRING &str_list_to_string
#include "List.h"

struct Entry {
	struct StrSetElement elem;
	struct StrListNode node;
	char str[24];
};
static const struct Entry *node_upcast_c(const struct StrListNode *const node) {
	return (const struct Entry *)(const void *)((const char *)node
		- offsetof(struct Entry, node));
}
static void str_list_to_string(const struct StrListNode *s,
	char (*const a)[12]) {
	sprintf(*a, "%.11s", node_upcast_c(s)->str);
}
/** @implements <KeyListNode>Compare */
static int str_list_compare(const struct StrListNode *const a,
	const struct StrListNode *const b) {
	return strcmp(node_upcast_c(a)->elem.key, node_upcast_c(b)->elem.key);
}

#define POOL_NAME Entry
#define POOL_TYPE struct Entry
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
	X(SETINIT), X(SETLOOK)

static int test(void) {
	struct StrTrie trie = TRIE_IDLE;
	struct StrArray array = ARRAY_IDLE;
	struct EntryPool entries = POOL_IDLE;
	struct StrSet set = SET_IDLE;
	struct StrList list;
	size_t i, r, s = 1, e;
	const size_t replicas = 3;
	clock_t t;
	int success = 1, is_full = 0;
	/* How many files we open simultaneously qs.size OR gnu.size. */
	enum { ES(PARAM) };
	struct { const char *name; FILE *fp; struct Measure m; }
		es[] = { ES(STRUCT) }, gnu = { "experiment", 0, {0,0,0} };
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
		if(s >= parole_size) is_full = 1, s = parole_size;
		for(e = 0; e < es_size; e++) m_reset(&es[e].m);
		for(r = 0; r < replicas; r++) {
			printf("replica %lu\n", r + 1);

			/* Trie. */

			t = clock();
			for(i = 0; i < s; i++)
				if(!StrTriePut(&trie, parole[i], 0)) goto catch;
			t = clock() - t;
			m_add(&es[TRIEINIT].m, 1000.0 / CLOCKS_PER_SEC * t);
			printf("trie size %lu initialisation %fms; %s.\n",
				(unsigned long)StrTrieSize(&trie), 1000.0 / CLOCKS_PER_SEC * t,
				StrTrieToString(&trie));
			t = clock();
			for(i = 0; i < s; i++) {
				const char *const str = StrTrieGet(&trie, parole[i]);
				assert(str);
			}
			t = clock() - t;
			m_add(&es[TRIELOOK].m, 1000.0 / CLOCKS_PER_SEC * t);
			printf("trie size %lu lookup all %fms.\n",
				(unsigned long)StrTrieSize(&trie), 1000.0 / CLOCKS_PER_SEC * t);
			/*trie_Str_graph(&inglesi, "graph/inglesi.gv"); -- 31MB. */
			/*trie_Str_print(&inglesi);*/
			StrTrieClear(&trie);

			/* Sorted array. */

			t = clock();
			for(i = 0; i < s; i++) array_insert(&array, parole[i]);
			t = clock() - t;
			m_add(&es[ARRAYINIT].m, 1000.0 / CLOCKS_PER_SEC * t);
			t = clock();
			for(i = 0; i < s; i++) {
				/* On systems which have differing pointer sizes, this is
				 problematic. */
				bsearch(parole[i], array.data, StrArraySize(&array),
					sizeof(array.data),
					(int (*)(const void *, const void *))&strcmp);
			}
			t = clock() - t;
			m_add(&es[ARRAYLOOK].m, 1000.0 / CLOCKS_PER_SEC * t);

			/* Linked set. */

			StrListClear(&list);
			t = clock();
			for(i = 0; i < s; i++) {
				struct Entry *const n = EntryPoolNew(&entries);
				n->elem.key = n->str;
				strcpy(n->str, parole[i]);
				if(StrSetPolicyPut(&set, &n->elem, 0))
					{ EntryPoolRemove(&entries, n); continue; } /* Duplicate. */
				StrListPush(&list, &n->node);
			}
			StrListSort(&list);
			t = clock() - t;
			m_add(&es[SETINIT].m, 1000.0 / CLOCKS_PER_SEC * t);
			printf("set size %lu initialisation %fms; %s.\n",
				(unsigned long)StrSetSize(&set), 1000.0 / CLOCKS_PER_SEC * t,
				StrListToString(&list));
			t = clock();
			for(i = 0; i < s; i++) {
				struct StrSetElement *const str = StrSetGet(&set, parole[i]);
				assert(str);
			}
			t = clock() - t;
			m_add(&es[SETLOOK].m, 1000.0 / CLOCKS_PER_SEC * t);
			printf("set size %lu lookup all %fms.\n",
				(unsigned long)StrSetSize(&set), 1000.0 / CLOCKS_PER_SEC * t);
			EntryPoolClear(&entries);
			StrSetClear(&set);
		}
		for(e = 0; e < es_size; e++) fprintf(es[e].fp, "%lu\t%f\t%f\n",
			(unsigned long)s, m_mean(&es[e].m), m_stddev(&es[e].m));
		/*if(parole_size >= s) break;
		s <<= 2;
		if(parole_size >= s) s = parole_size;*/
	}
	printf("Test passed.\n");
	goto finally;
catch:
	success = 0;
	perror("test");
	printf("Test failed.\n");
finally:
	StrArray_(&array);
	EntryPool_(&entries);
	StrSet_(&set);
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
		fprintf(gnu.fp, "set term postscript eps enhanced color\n"
			"set output \"graph/%s.eps\"\n"
			"set grid\n"
			"set xlabel \"elements\"\n"
			"set ylabel \"time, t (ms)\"\n"
			"set yrange [0:2000]\n"
			"set log x\n"
			"plot", gnu.name);
		for(e = 0; e < es_size; e++) fprintf(gnu.fp,
			"%s \\\n\"graph/%s.tsv\" using 1:($2/$1*1000000):($3/$1*1000000) "
			"with errorlines lw 3 title \"%s\"", e ? "," : "",
			es[e].name, es[e].name);
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
	/*test_basic_trie_str();
	printf("\n***\n\n");
	test();*/
	foo();
	return EXIT_SUCCESS;
}
