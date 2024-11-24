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

struct fixed_string { char str[128/*12*//*32*/]; };

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
//#define TREE_ORDER 3
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

static int str_tree_exists(struct str_tree_view *const v)
	{ return assert(v), !!v->root; }

static const char *str_tree_front(const struct str_tree_view *const v) {
	assert(v && v->root && v->front.node
		&& v->front.idx <= v->front.node->size);
	return v->front.node->key[v->front.idx];
}

/** @return Whether `it` pointing to a valid element. */
static void str_tree_pop_front(struct str_tree_view *const v) {
	struct tree_str_ref next;
	assert(v && v->root && v->root->node && v->root->height != UINT_MAX &&
		(!v->front.node || v->front.idx <= v->front.node->size) &&
		(!v->back.node || v->back.idx <= v->back.node->size));

	/* Next is a copy of the next element. Clip. */
	next = v->front, next.idx++;
	//if(next.height && next.idx > next.node->size) next.idx = next.node->size;
	if(next.idx > next.node->size /* Concurrent modification? */
		|| v->front.node == v->back.node && v->front.idx == v->back.idx)
		{ v->root = 0; return; }
	while(next.height) next.node = tree_str_as_branch(next.node)->child[next.idx],
		next.idx = 0, next.height--; /* Fall from branch. */
	v->front = next; /* Possibly one beyond bounds.[?!] */
	if(next.idx >= next.node->size) { /* Maybe re-descend reveals more keys. */
		struct tree_str_tree tree = *v->root;
		unsigned a0;
		/* Target; this will not work with duplicate keys: we can't have any. */
		const tree_str_key x = next.node->key[next.node->size - 1];
		assert(next.node->size);
		for(next.node = 0; tree.height;
			tree.node = tree_str_as_branch(tree.node)->child[a0],
			tree.height--) {
			unsigned a1 = tree.node->size;
			a0 = 0;
			while(a0 < a1) {
				const unsigned m = (a0 + a1) / 2;
				if(strcmp(x, tree.node->key[m]) > 0) a0 = m + 1;
				else a1 = m;
			}
			if(a0 < tree.node->size) next.node = tree.node,
				next.height = tree.height, next.idx = a0;
		}
		if(!next.node) { v->root = 0; return; } /* Off right. */
	} /* Jumped nodes. */
	v->front = next;
}

extern void optimization_inhibitor(const char *);
extern void optimization_inhibitor(const char *const dum) { (void)dum; }

//#define OUT

int main(void) {
	int success = 0;
	clock_t t;
	struct measures { struct measure trie_add, trie_look, trie_prefix,
		tree_add, tree_look, tree_prefix; } m_array[22];
	const size_t m_size = sizeof m_array / sizeof *m_array,
		words_size = 1 << m_size;
	struct fixed_string *words_array = 0;
	struct str_trie trie = str_trie();
	struct str_tree tree = str_tree();

	(void)str_trie_test, (void)str_tree_test;

	unsigned seed = (unsigned)clock();
	srand(seed), rand(), printf("# Seed %u.\n", seed);
	errno = 0;

	/* Set up memory to run the experiment. */
	if(!(words_array = malloc(sizeof *words_array * words_size))) goto catch;
	for(struct measures *m = m_array, *const m_end = m + m_size; m < m_end; m++)
		m_reset(&m->trie_add), m_reset(&m->trie_look), m_reset(&m->trie_prefix),
		m_reset(&m->tree_add), m_reset(&m->tree_look), m_reset(&m->tree_prefix);
	if(!str_trie_try(&trie, "init") || !str_tree_try(&tree, "init"))
		goto catch;
	str_trie_clear(&trie), str_tree_clear(&tree);

	/* We are going to repeat measurements this many times… */
	for(unsigned replicas = 0; replicas < 3; replicas++) {
		/* …with these exponential differences in problem… */
		for(unsigned m = 0, m_words = 2; m < m_size; m++, m_words <<= 1) {
			assert(m_words <= words_size);
			/* …with different random words. */
			for(struct fixed_string *word = words_array,
				*const word_end = words_array + m_words;
				word < word_end; word++)
				orcish(word->str, sizeof word->str);
			//fprintf(stderr, "m: %u; m_words: %u; word_size: %zu\n", m, m_words, words_size);

			/* Trie tests. */
			t = clock();
			for(struct fixed_string *word = words_array,
				*const word_end = words_array + m_words;
				word < word_end; word++)
				if(!str_trie_try(&trie, word->str)) goto catch;
			m_add(&m_array[m].trie_add, diff_us(t));
			//trie_str_graph(&trie, "trie.gv", 1);
			t = clock();
			for(struct fixed_string *word = words_array,
				*const word_end = words_array + m_words;
				word < word_end; word++) {
				const char *result = str_trie_get(&trie, word->str);
#ifdef OUT
				fprintf(stderr, "look trie %s: %s\n", word->str, result);
#else
				optimization_inhibitor(result);
#endif
			}
			m_add(&m_array[m].trie_look, diff_us(t));
			t = clock();
			for(char letter = 'A', letter_end = 'Z' + 1;
				letter < letter_end; letter++) {
				const char build[] = { letter, '\0' };
				struct str_trie_iterator it = str_trie_prefix(&trie, build);
#ifdef OUT
				while(str_trie_next(&it))
					printf("%s: %s\n", build, str_trie_entry(&it));
#else
				str_trie_next(&it);
#endif
			}
			m_add(&m_array[m].trie_prefix, diff_us(t));
			str_trie_clear(&trie);

			/* Tree tests. */
			t = clock();
			for(struct fixed_string *word = words_array,
				*const word_end = words_array + m_words;
				word < word_end; word++)
				if(!str_tree_try(&tree, word->str)) goto catch;
			double fl;
			m_add(&m_array[m].tree_add, fl = diff_us(t));
			//tree_str_graph(&tree, "tree.gv");
			t = clock();
			for(struct fixed_string *word = words_array,
				*const word_end = words_array + m_words;
				word < word_end; word++) {
				const char *result;
				result = str_tree_get(&tree, word->str);
#ifdef OUT
				fprintf(stderr, "%s: %s\n", word->str, result);
#else
				optimization_inhibitor(result);
#endif
			}
			m_add(&m_array[m].tree_look, diff_us(t));
			t = clock();
			for(char letter = 'A', letter_end = 'Z';
				letter <= letter_end;
				letter++) {
				const char build[] = { letter, '\0' };
				struct str_tree_view view = str_tree_prefix(&tree, build);
#ifdef OUT
				printf("range %s: ", build);
				if(!view.root) printf("null\n");
				else printf("[%s, %s]\n", view.front.node->key[view.front.idx], view.back.node->key[view.back.idx]);
				for( ; str_tree_exists(&view); str_tree_pop_front(&view))
					printf("%s: %s\n", build, str_tree_front(&view));
#else
				str_tree_exists(&view);
#endif
			}
			m_add(&m_array[m].tree_prefix, diff_us(t));
			str_tree_clear(&tree);
		}
	}
	/* This could be a macro. */
	printf("set term postscript eps enhanced\n"
		"set encoding utf8\n"
		"set output \"plot.eps\"\n"
		"set xlabel \"Elements\"\n"
		"set ylabel \"Time (µs)\"\n"
		"set xrange [0:%u*1.01]\n"
		"set yrange [0:*]\n"
		"set grid\n"
		"set key autotitle columnhead\n"
		"set multiplot layout 3,1\n"
		"#set size ratio 1\n"
		"\n"
		"$trie_add<<EOD\n"
		"trie-add\n"
		"# size\ttime µs\tstddev µs\n", 1 << m_size);
	for(unsigned m = 0, m_words = 2; m < m_size; m++, m_words <<= 1)
		printf("%u\t%f\t%f\n", m_words, m_mean(&m_array[m].trie_add),
		m_stddev(&m_array[m].trie_add));
	printf("EOD\n"
		"$trie_look<<EOD\n"
		"trie-look\n"
		"# size\ttime µs\tstddev µs\n");
	for(unsigned m = 0, m_words = 2; m < m_size; m++, m_words <<= 1)
		printf("%u\t%f\t%f\n", m_words, m_mean(&m_array[m].trie_look),
		m_stddev(&m_array[m].trie_look));
	printf("EOD\n"
		"$trie_prefix<<EOD\n"
		"trie-prefix\n"
		"# size\ttime µs\tstddev µs\n");
	for(unsigned m = 0, m_words = 2; m < m_size; m++, m_words <<= 1)
		printf("%u\t%f\t%f\n", m_words, m_mean(&m_array[m].trie_prefix),
			m_stddev(&m_array[m].trie_prefix));
	printf("EOD\n"
		"$tree_add<<EOD\n"
		"tree-add\n"
		"# size\ttime µs\tstddev µs\n");
	for(unsigned m = 0, m_words = 2; m < m_size; m++, m_words <<= 1)
		printf("%u\t%f\t%f\n", m_words, m_mean(&m_array[m].tree_add),
			m_stddev(&m_array[m].tree_add));
	printf("EOD\n"
		"$tree_look<<EOD\n"
		"tree-look\n"
		"# size\ttime µs\tstddev µs\n");
	for(unsigned m = 0, m_words = 2; m < m_size; m++, m_words <<= 1)
		printf("%u\t%f\t%f\n", m_words, m_mean(&m_array[m].tree_look),
			m_stddev(&m_array[m].tree_look));
	printf("EOD\n"
		"$tree_prefix<<EOD\n"
		"tree-prefix\n"
		"# size\ttime µs\tstddev µs\n");
	for(unsigned m = 0, m_words = 2; m < m_size; m++, m_words <<= 1)
		printf("%u\t%f\t%f\n", m_words, m_mean(&m_array[m].tree_prefix),
			m_stddev(&m_array[m].tree_prefix));
	printf("EOD\n"
		"\n"
		"unset xlabel\n"
		"set format x \"\"\n"
		"plot $trie_add with yerrorlines, $tree_add with yerrorlines\n"
		"plot $trie_look with yerrorlines, $tree_look with yerrorlines\n"
		"set xlabel\n"
		"set format x \"%%.0f\"\n"
		"plot $trie_prefix with yerrorlines, $tree_prefix with yerrorlines\n");

	success = 1;
	goto finally;
catch:
	perror("trie vs tree");
finally:
	str_tree_(&tree);
	str_trie_(&trie);
	free(words_array);
	return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
