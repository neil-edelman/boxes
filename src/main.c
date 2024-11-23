#include "../test/orcish.h"
#include <stdlib.h>
#include <time.h> /* clock */
#include <math.h> /* NAN */
#include <stdio.h>
#include <assert.h>

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

struct fixed_string { char str[12/*32*/]; };

/** Unused—just want to get gv output. */
static void str_filler(const char **const key) { (void)key; }

#define TRIE_NAME str
#define TRIE_TO_STRING /* Uses the keys as strings. For test. */
#define TRIE_TEST
#include "trie.h"

static int str_compare(const char *const a, const char *const b)
	{ return strcmp(a, b); }
static void str_to_string(const char *const x, char (*const z)[12])
	{ strncpy(*z, x, sizeof *z - 1); }
#define TREE_NAME str
#define TREE_KEY const char *
#define TREE_COMPARE
#define TREE_DEFAULT 0
#define TREE_ORDER 3
#define TREE_TO_STRING
#define TREE_TEST
#include "tree.h"

/** @return Compares up to `prefix`-length lexicographically with `word`. */
static int str_compare_prefix(const char *const prefix, const char *const word) {
	const unsigned char *uprefix = (const char unsigned *)prefix,
		*uword = (const char unsigned *)word;
	unsigned char up, uw;
	for( ; ; ) {
		if((up = *uprefix) == '\0') return 0; /* It's a prefix. */
		if(up != (uw = *uword)) return (up > uw) - (up < uw);
		uprefix++, uword++;
	}
}

/** Finds `idx` of greatest lower-bound minorant of `x` in `lo` only in one
 node at a time. */
static void tree_str_prefix_node_lower(struct tree_str_ref *const lo, const tree_str_key x) {
	unsigned hi = lo->node->size; lo->idx = 0;
	assert(lo && lo->node && hi);
	do {
		const unsigned mid = (lo->idx + hi) / 2; /* Will not overflow. */
		if(str_compare_prefix(x, lo->node->key[mid]) > 0) lo->idx = mid + 1;
		else hi = mid;
	} while(lo->idx < hi);
}
/** Finds `idx` of least upper-bound majorant of `x` in `hi` only in one node
 at a time. */
static void tree_str_prefix_node_upper(struct tree_str_ref *const hi, const tree_str_key x) {
	unsigned lo = 0;
	assert(hi->node && hi->idx);
	do {
		const unsigned mid = (lo + hi->idx) / 2;
		if(str_compare_prefix(x, hi->node->key[mid]) >= 0) lo = mid + 1;
		else hi->idx = mid;
	} while(lo < hi->idx);
}
static struct tree_str_ref tree_prefix_lower(const struct tree_str_tree tree,
	const char *const x) {
	struct tree_str_ref lo, found;
	found.node = 0;
	if(!tree.node || tree.height == UINT_MAX) return found;
	for(lo.node = tree.node, lo.height = tree.height; ;
		lo.node = tree_str_as_branch_c(lo.node)->child[lo.idx], lo.height--) {
		unsigned hi = lo.node->size; lo.idx = 0;
		if(!hi) continue;
		tree_str_prefix_node_lower(&lo, x);
		if(lo.idx < lo.node->size) found = lo;
		if(!lo.height) break;
	}
	return found;
}
static struct tree_str_ref tree_prefix_upper(const struct tree_str_tree tree,
	const char *const x) {
	struct tree_str_ref hi, found;
	found.node = 0;
	if(!tree.node || tree.height == UINT_MAX) return found;
	for(hi.node = tree.node, hi.height = tree.height; ;
		hi.node = tree_str_as_branch_c(hi.node)->child[hi.idx], hi.height--) {
		if(!(hi.idx = hi.node->size)) continue;
		tree_str_prefix_node_upper(&hi, x);
		if(hi.idx) found = hi, found.idx--;
		if(!hi.height) break; /* Reached the bottom. */
	}
	return found;
}

// iterator cursor look range subset [view] span: unbounded laden full [entire], laden occupied [exists], distance size count, get acquire look first [front], last back, shift [pop_front], pop pop_back… random?, delete, insert
// for(view v = entire(&container) or v = prefix(&tree, "foo"); exists(&v); pop_front(&v)) print front(&v);
/* A subset of contiguous elements built on top of a container, stored
 in O(1) space, valid until a topological modification of the
 container. */
struct str_tree_view {
	const struct tree_str_tree *root;
	struct tree_str_ref front, back;
	/* !root -> dead, root -> front_node && front < front_node.size && (!back_node || back_node && back < back_node.size && front_node[front] <= back_node[back]) */
};

static struct str_tree_view str_tree_prefix(const struct str_tree *const tree,
	const char *const prefix) {
	struct str_tree_view view;
	assert(tree);
	view.front = tree_prefix_lower(tree->root, prefix);
	/* We have done this comparison already but it is lost to the others. */
	if(!view.front.node
		|| str_compare_prefix(prefix, view.front.node->key[view.front.idx]))
		return view.root = 0, view;
	view.back = tree_prefix_upper(tree->root, prefix);
	return view.root = &tree->root, view;
}

#define OUT

int main(void) {
	int success = 0;
	struct measure m;
	clock_t t;
	const size_t word_array_size = 50;
	struct fixed_string *word_array = 0;
	struct str_trie trie = str_trie();
	struct str_tree tree = str_tree();

	(void)str_trie_test, (void)str_tree_test;

	unsigned seed = (unsigned)clock();
	srand(seed), rand(), fprintf(stderr, "Seed %u.\n", seed);
	errno = 0;

	/* Create a random word array. */
	if(!(word_array = malloc(sizeof *word_array * word_array_size))) goto catch;
	for(struct fixed_string *word = word_array,
		*const word_end = word_array + word_array_size;
		word < word_end; word++)
		orcish(word->str, sizeof word->str);

	/* Trie tests. */
	m_reset(&m), t = clock();
	for(struct fixed_string *word = word_array,
		*const word_end = word_array + word_array_size;
		word < word_end; word++)
		if(!str_trie_try(&trie, word->str)) goto catch;
	m_add(&m, diff_us(t));
	printf("trie add: %f(%f)µs/%zu\n", m_mean(&m), m_stddev(&m), word_array_size);
	trie_str_graph(&trie, "trie.gv", 1);
	m_reset(&m), t = clock();
	for(struct fixed_string *word = word_array,
		*const word_end = word_array + word_array_size;
		word < word_end; word++) {
		const char *result;
		result = str_trie_get(&trie, word->str);
#ifdef OUT
		fprintf(stderr, "%s: %s\n", word->str, result);
#endif
	}
	m_add(&m, diff_us(t));
	printf("trie look: %f(%f)µs/%zu\n", m_mean(&m), m_stddev(&m), word_array_size);
	m_reset(&m), t = clock();
	for(char letter = 'A', letter_end = 'Z' + 1;
		letter < letter_end;
		letter++) {
		const char build[] = { letter, '\0' };
		struct str_trie_iterator it = str_trie_prefix(&trie, build);
		while(str_trie_next(&it))
#ifdef OUT
			printf("%s: %s\n", build, str_trie_entry(&it))
#endif
			;
	}
	m_add(&m, diff_us(t));
	printf("trie prefix: %f(%f)µs/%zu\n", m_mean(&m), m_stddev(&m), word_array_size);
	str_trie_clear(&trie);

	/* Tree tests. */
	m_reset(&m);
	for(struct fixed_string *word = word_array,
		*const word_end = word_array + word_array_size;
		word < word_end; word++)
		if(!str_tree_try(&tree, word->str)) goto catch;
	m_add(&m, diff_us(t));
	printf("tree add: %f(%f)µs/%zu\n", m_mean(&m), m_stddev(&m), word_array_size);
	tree_str_graph(&tree, "tree.gv");
	m_reset(&m), t = clock();
	for(struct fixed_string *word = word_array,
		*const word_end = word_array + word_array_size;
		word < word_end; word++) {
		const char *result;
		result = str_tree_get(&tree, word->str);
#ifdef OUT
		fprintf(stderr, "%s: %s\n", word->str, result);
#endif
	}
	m_add(&m, diff_us(t));
	printf("tree look: %f(%f)µs/%zu\n", m_mean(&m), m_stddev(&m), word_array_size);
	m_reset(&m), t = clock();
	for(char letter = 'A', letter_end = 'Z';
		letter <= letter_end;
		letter++) {
		const char build[] = { letter, '\0' };
		struct str_tree_view view = str_tree_prefix(&tree, build);
		printf("%s: ", build);
		if(!view.root) printf("null\n");
		else printf("[%s, %s]\n", view.front.node->key[view.front.idx], view.back.node->key[view.back.idx]);
	}
	m_add(&m, diff_us(t));
	printf("tree prefix: %f(%f)µs/%zu\n", m_mean(&m), m_stddev(&m), word_array_size);
	str_tree_clear(&tree);

	success = 1;
	goto finally;
catch:
	perror("trie vs tree");
finally:
	str_tree_(&tree);
	str_trie_(&trie);
	free(word_array);
	return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
