/**
 TRIE_NAME: required prefix name
 TRIE_KEY, TRIE_KEY_TO_STRING: must both be defined or not; the type
 <typedef:<PT>key> and function <typedef:<PT>key_to_string_fn> that creates an
 indirect key that maps to `const char *`, (such as an `enum` prefix map.)
 TRIE_VALUE: optional, makes it a map from <typedef:<PT>key> to
 <typedef:<PT>value>; prefer small size.
 TRIE_KEY_IN_VALUE: requires TRIE_VALUE. Function that chooses key
 from <PT>value.
 TRIE_TO_STRING: optional no arguments, uses keys and <to_string.h>
 TRIE_DEFAULT_NAME, TRIE_DEFAULT: get or default set default */
#ifndef TRIE_NAME
#error Name TRIE_NAME undefined.
#endif
#if defined(TRIE_DEFAULT_NAME) || defined(TRIE_DEFAULT)
#define TRIE_DEFAULT_TRAIT 1
#else
#define TRIE_DEFAULT_TRAIT 0
#endif
#define TRIE_TRAITS TRIE_DEFAULT_TRAIT
#if TRIE_TRAITS > 1
#error Only one trait per include is allowed; use TRIE_EXPECT_TRAIT.
#endif
#if defined(TRIE_KEY) ^ defined(TRIE_KEY_TO_STRING)
#error TRIE_KEY and TRIE_KEY_TO_STRING have to be mutually defined.
#endif
#if defined(TRIE_KEY_IN_VALUE) && !defined(TRIE_VALUE)
#error TRIE_KEY_IN_VALUE requires TRIE_VALUE.
#endif
#if defined(TRIE_DEFAULT_NAME) && !defined(TRIE_DEFAULT)
#error TRIE_DEFAULT_NAME requires TRIE_DEFAULT.
#endif
#if defined(TRIE_TEST) && !defined(TRIE_TO_STRING)
#error TRIE_TEST requires TRIE_TO_STRING.
#endif

#ifndef TRIE_H /* <!-- idempotent */
#define TRIE_H
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <limits.h>
/* <Kernighan and Ritchie, 1988, p. 231>. */
#if defined(TRIE_CAT_) || defined(TRIE_CAT) || defined(T_) || defined(PT_)
#error Unexpected defines.
#endif
#define TRIE_CAT_(n, m) n ## _ ## m
#define TRIE_CAT(n, m) TRIE_CAT_(n, m)
#define T_(n) TRIE_CAT(TRIE_NAME, n)
#define PT_(n) TRIE_CAT(trie, T_(n))
/* <http://c-faq.com/misc/bitsets.html>, except reversed for msb-first. */
#define TRIE_MASK(n) ((1 << CHAR_BIT - 1) >> (n) % CHAR_BIT)
#define TRIE_SLOT(n) ((n) / CHAR_BIT)
#define TRIE_QUERY(a, n) ((a)[TRIE_SLOT(n)] & TRIE_MASK(n))
#define TRIE_DIFF(a, b, n) \
	(((a)[TRIE_SLOT(n)] ^ (b)[TRIE_SLOT(n)]) & TRIE_MASK(n))
/* Worst-case all-branches-left root (`{a, aa, aaa, ...}`). Parameter sets the
 maximum tree size. Prefer alignment `4n - 2`; cache `32n - 2`. */
#define TRIE_MAX_LEFT 3/*6*//*253*/
#if TRIE_MAX_LEFT < 1 || TRIE_MAX_LEFT > UCHAR_MAX - 2 /* This is not ideal. */
#error TRIE_MAX_LEFT parameter range `[1, UCHAR_MAX - 2]`.
#endif
#define TRIE_BRANCHES (TRIE_MAX_LEFT + 1) /* Maximum branches. */
#define TRIE_ORDER (TRIE_BRANCHES + 1) /* Maximum branching factor/leaves. */
/* `⌈(2n-1)/3⌉` nodes. */
#define TRIE_SPLIT ((2 * (TRIE_ORDER + TRIE_BRANCHES) - 1 + 2) / 3)
#define TRIE_RESULT X(ERROR), X(UNIQUE), X(PRESENT)
#define X(n) TRIE_##n
/** A result of modifying the table, of which `TRIE_ERROR` is false.

 ![A diagram of the result states.](../doc/put.png) */
enum trie_result { TRIE_RESULT };
#undef X
#define X(n) #n
/** A static array of strings describing the <tag:trie_result>. */
static const char *const trie_result_str[] = { TRIE_RESULT };
#undef X
#undef TRIE_RESULT
struct trie_branch { unsigned char left, skip; };
/* Construct `struct trie_bmp`. */
#define BMP_NAME trie
#define BMP_BITS TRIE_ORDER
#include "bmp.h"
/** @return Whether `a` and `b` are equal up to the minimum of their lengths'.
 Used in <fn:<T>trie_prefix>. */
static int trie_is_prefix(const char *a, const char *b) {
	for( ; ; a++, b++) {
		if(*a == '\0') return 1;
		if(*a != *b) return *b == '\0';
	}
}
#endif /* idempotent --> */


#if TRIE_TRAITS == 0 /* <!-- base trie */


#ifdef TRIE_KEY /* <!-- custom key */
/** The default is `const char *`. (Any type that decays to this will be fine
 to use the default, but once in the trie, it must not change.) If one sets
 `TRIE_KEY`, then one must also set <typedef:<PT>key_to_string_fn> by
 `TRIE_KEY_TO_STRING`. */
typedef TRIE_KEY PT_(key);
#else /* custom key --><!-- string key */
typedef const char *PT_(key);
#endif /* string key --> */
/** Transforms a key into a `const char *`, (if it isn't already.) */
typedef const char *(*PT_(key_to_string_fn))(PT_(key));
#ifdef TRIE_KEY_TO_STRING /* <!-- custom key */
/* Valid <typedef:<PT>key_to_string_fn>. */
static PT_(key_to_string_fn) PT_(key_string) = (TRIE_KEY_TO_STRING);
#else /* custom key --><!-- string key */
/** The string of `key` is itself by default.
 @implements `<PT>key_to_string_fn` */
static const char *PT_(string_to_string)(const char *const s) { return s; }
static PT_(key_to_string_fn) PT_(key_string) = &PT_(string_to_string);
#endif /* string key --> */


#ifndef TRIE_VALUE /* <!-- key set */

typedef PT_(key) PT_(entry);
static PT_(key) PT_(entry_key)(const PT_(key) *const key) { return *key; }

#elif !defined(TRIE_KEY_IN_VALUE) /* key set --><!-- key value map */

typedef TRIE_VALUE PT_(value);
/** On `KEY_VALUE` but not `KEY_KEY_IN_VALUE`, defines an entry. */
struct T_(trie_entry) { PT_(key) key; PT_(value) value; };
typedef struct T_(trie_entry) PT_(entry);
static PT_(key) PT_(entry_key)(const struct T_(trie_entry) *const e)
	{ return e->key; }
static PT_(value) *PT_(entry_value)(struct T_(trie_entry) *const e)
	{ return &e->value; }

#else /* key value map --><!-- key in value */

/* The entry is the value. */
typedef TRIE_VALUE PT_(value);
typedef PT_(value) PT_(entry);
/** If `TRIE_KEY_IN_VALUE`, extracts the key from `TRIE_VALUE`. */
typedef PT_(key) (*PT_(key_fn))(const PT_(value) *);
/* Verify `TRIE_KEY_IN_VALUE` is a function satisfying <typedef:<PT>key_fn>. */
static PT_(key_fn) PT_(read_key) = (TRIE_KEY_IN_VALUE);
static const char *PT_(entry_key)(const PT_(value) *const v)
	{ return PT_(read_key)(v); }
static PT_(value) *PT_(entry_value)(PT_(value) *const v) { return v; }

#endif /* key in value --> */


struct PT_(tree);
struct PT_(ref) { struct PT_(tree) *tree; unsigned idx; };
struct PT_(ref_c) { const struct PT_(tree) *tree; unsigned idx; };
union PT_(leaf) { PT_(entry) as_entry; struct PT_(tree) *as_link; };
struct PT_(tree) {
	unsigned char bsize;
	struct trie_branch branch[TRIE_BRANCHES];
	struct trie_bmp bmp;
	union PT_(leaf) leaf[TRIE_ORDER];
};
struct T_(trie) { struct PT_(tree) *root; };

/* Fall through `ref` until hit an entry. Must be pointing at something. */
static void PT_(fall_through_to_entry)(struct PT_(ref_c) *ref) {
	while(trie_bmp_test(&ref->tree->bmp, ref->idx))
		ref->tree = ref->tree->leaf[ref->idx].as_link, ref->idx = 0;
}

/** If `ref.tree` is null, starts iteration.
 @return Does `ref` have a successor in `root`? If yes, sets it to that. */
static int PT_(to_successor_c)(const struct PT_(tree) *const root,
	struct PT_(ref_c) *const ref) {
	assert(ref);
	if(!root || root->bsize == UCHAR_MAX) return 0; /* Empty. */
	if(!ref->tree) { ref->tree = root, ref->idx = 0; } /* Start. */
	else if(++ref->idx > ref->tree->bsize) { /* Gone off the end. */
		const struct PT_(tree) *const old = ref->tree;
		const char *const sample =
			PT_(key_string)(PT_(entry_key)(&old->leaf[ref->idx - 1].as_entry));
		const struct PT_(tree) *tree = root;
		size_t bit = 0;
		for(ref->tree = 0, ref->idx = 0; tree != old; ) {
			unsigned br0 = 0, br1 = tree->bsize, lf = 0;
			while(br0 < br1) {
				const struct trie_branch *const branch = tree->branch + br0;
				bit += branch->skip;
				if(!TRIE_QUERY(sample, bit))
					br1 = ++br0 + branch->left;
				else
					br0 += branch->left + 1, lf += branch->left + 1;
				bit++;
			}
			if(lf < tree->bsize) ref->tree = tree, ref->idx = lf + 1;
			assert(trie_bmp_test(&tree->bmp, lf));
			tree = tree->leaf[lf].as_link;
		}
		if(!ref->tree) return 0; /* End of iteration. */
	}
	PT_(fall_through_to_entry)(ref);
	return 1;
}

#define BOX_CONTENT const PT_(entry) *
static int PT_(is_element_c)(const PT_(entry) *const e) { return !!e; }
struct PT_(forward) { const struct PT_(tree) *root; struct PT_(ref_c) cur; };
static struct PT_(forward) PT_(forward)(const struct T_(trie) *const trie) {
	struct PT_(forward) it;
	it.root = trie ? trie->root : 0, it.cur.tree = 0;
	return it;
}
static const PT_(entry) *PT_(next_c)(struct PT_(forward) *const it) {
	return assert(it), PT_(to_successor_c)(it->root, &it->cur)
		? &it->cur.tree->leaf[it->cur.idx].as_entry : 0;
}

#define BOX_ITERATOR PT_(entry) *
static int PT_(is_element)(const PT_(entry) *const e) { return !!e; }
/* A range of words from `[t0:lf0, t1:lf1)` fixme: ]. */
struct PT_(cursor) {
	const struct T_(trie) *trie; /* Valid, rest must be, too, or ignore rest. */
	struct PT_(ref_c) cur, end;
};
/** Loads the first element of `trie` (can be null) into `it`.
 @implements begin */
static void PT_(begin)(struct PT_(cursor) *const it,
	const struct T_(trie) *const trie) {
	/* This is exactly what is happening, but we can shorten it.
	 PT_(match_prefix)(trie, "", cur); (not code, file size.) */
	if(it->trie = trie) {
		if(it->cur.tree = it->end.tree = trie->root) {
			it->cur.idx = 0, it->end.idx = trie->root->bsize + 1;
			PT_(fall_through_to_entry)(&it->cur);
		}
	} else {
		it->cur.tree = 0;
	}
}
/** Advances `it`. @return The previous value or null. @implements next */
const static PT_(entry) *PT_(next)(struct PT_(cursor) *const it) {
	struct PT_(ref_c) maybe;
	assert(it);
	if(!it->trie) return 0;
	maybe = it->cur;
	/* fixme: No. First do thing +1; hmm. hmmmmm. */
	if(!PT_(to_successor_c)(it->trie->root, &maybe)
		|| maybe.tree == it->end.tree && maybe.idx <= it->end.idx) return 0;
	return &(it->cur.tree = maybe.tree)
		->leaf[it->cur.idx = maybe.idx].as_entry;
}




/** Looks at only the index of `trie` (which can be null) for potential
 `prefix` matches, and stores them in `cur` as `[key1, key2)`. */
static void PT_(match_prefix)(const struct T_(trie) *const trie,
	const char *const prefix, struct PT_(cursor) *it) {
	struct PT_(tree) *tree;
	size_t bit;
	struct { size_t cur, next; } byte;
	assert(trie && prefix && it);
	it->trie = 0;
	if(!(tree = trie->root)) return;
	for(bit = 0, byte.cur = 0; ; ) {
		unsigned br0 = 0, br1 = tree->bsize, lf = 0;
		while(br0 < br1) {
			const struct trie_branch *const branch = tree->branch + br0;
			/* _Sic_; '\0' is _not_ included for partial match. */
			for(byte.next = (bit += branch->skip) / CHAR_BIT;
				byte.cur <= byte.next; byte.cur++)
				if(prefix[byte.cur] == '\0') goto finally;
			if(!TRIE_QUERY(prefix, bit))
				br1 = ++br0 + branch->left;
			else
				br0 += branch->left + 1, lf += branch->left + 1;
			bit++;
		}
		if(trie_bmp_test(&tree->bmp, lf))
			{ tree = tree->leaf[lf].as_link; continue; } /* Link. */
finally:
		assert(br0 <= br1 && lf - br0 + br1 <= tree->bsize);
		it->trie = trie;
		it->cur.tree = it->end.tree = tree;
		it->cur.idx = lf; /* and this? */
		it->end.idx = lf + br1 - br0; /* fixme: What's stopping this from being a link? */
		break;
	}
	printf("<%s>.match_prefix: [%s:%u<%s>..%s:%u)\n", prefix,
		orcify(it->cur.tree), it->cur.idx,
		PT_(key_string)(PT_(entry_key)(
		&it->cur.tree->leaf[it->cur.idx].as_entry)),
		orcify(it->end.tree), it->end.idx);
	/* fixme: I'm not convinced that leaf will always be an entry . . . */
	/* We need another fn to fall-trough. */
}
/** Stores all `prefix` matches in `trie` in `it`. @order \O(|`prefix`|) */
static void PT_(prefix)(struct T_(trie) *const trie,
	const char *const prefix, struct PT_(cursor) *it) {
	assert(trie && prefix && it);
	PT_(match_prefix)(trie, prefix, it);
	/* Make sure actually a prefix. */
	if(it->trie && !trie_is_prefix(prefix,
		PT_(key_string)(PT_(entry_key)(
		&it->cur.tree->leaf[it->cur.idx].as_entry)))) it->cur.tree = 0;
}








/** @return A candidate match for `key`, non-null, in `tree`, which is the
 valid root, or null, if `key` is definitely not in the trie. */
static int PT_(match)(struct PT_(tree) *tree,
	const char *const string, PT_(entry) *const match) {
	size_t bit; /* In bits of `key`. */
	struct { size_t cur, next; } byte; /* `key` null checks. */
	assert(tree && string && match);
	for(bit = 0, byte.cur = 0; ; ) {
		unsigned br0 = 0, br1 = tree->bsize, lf = 0;
		while(br0 < br1) {
			const struct trie_branch *const branch = tree->branch + br0;
			for(byte.next = (bit += branch->skip) / CHAR_BIT;
				byte.cur < byte.next; byte.cur++)
				if(string[byte.cur] == '\0') return 0; /* `key` too short. */
			if(!TRIE_QUERY(string, bit))
				br1 = ++br0 + branch->left;
			else
				br0 += branch->left + 1, lf += branch->left + 1;
			bit++;
		}
		if(!trie_bmp_test(&tree->bmp, lf))
			return *match = tree->leaf[lf].as_entry, 1;
		tree = tree->leaf[lf].as_link; /* Jumped trees. */
	}
}













/** Exact `string` in `trie`.
 @return Success; `result` holds entry if not null. */
static int PT_(query)(const struct T_(trie) *const trie,
	const char *const string, PT_(entry) *result) {
	PT_(entry) entry;
	if(trie && string && trie->root && trie->root->bsize != UCHAR_MAX
		&& PT_(match)(trie->root, string, &entry)
		&& !strcmp(PT_(key_string)(PT_(entry_key)(&entry)), string)) {
		if(result) *result = entry;
		return 1;
	} else {
		return 0;
	}
}


/** @return The leftmost key `lf` of `tree`. */
static const char *PT_(sample)(const struct PT_(tree) *tree, unsigned lf) {
	while(trie_bmp_test(&tree->bmp, lf)) tree = tree->leaf[lf].as_link, lf = 0;
	return PT_(key_string)(PT_(entry_key)(&tree->leaf[lf].as_entry));
}

#if 0
/** Given a `trie`, calculate the bit at the start of `trunk`.
 @order \O(\log `trie.size`) */
static size_t PT_(trunk_diff)(const struct T_(trie) *trie,
	const struct trie_trunk *trunk, const size_t height) {
	struct trie_trunk_descend d;
	const char *sample = PT_(sample)(trunk, height, 0);
	assert(trie && sample);
	d.h = trie->node_height, assert(d.h);
	for(d.trunk = trie->root, assert(d.trunk), d.diff = 0; ;
		d.trunk = trie_inner(d.trunk)->link[d.lf]) {
		assert(d.trunk->skip < d.h), d.h -= 1 + trunk->skip;
		if(trunk == d.trunk) break;
		d.br0 = 0, d.br1 = trunk->bsize, d.lf = 0;
		while(d.br0 < d.br1) {
			const struct trie_branch *const branch = d.trunk->branch + d.br0;
			if(!TRIE_QUERY(sample, d.diff))
				d.br1 = ++d.br0 + branch->left;
			else
				d.br0 += branch->left + 1, d.lf += branch->left + 1;
			d.diff++;
		}
		assert(d.h); if(!d.h) return 0; /* Corrupted? */
	}
	return d.diff;
}
#endif

/** @throws[malloc] */
static int PT_(split)(struct PT_(tree) *const tree) {
	unsigned br0, br1, lf;
	struct PT_(tree) *kid;
	assert(tree && tree->bsize == TRIE_BRANCHES);
	/* Mitosis; more info added on error in <fn:<PT>add_unique>. */
	if(!(kid = malloc(sizeof *kid))) return 0;
	/* Where should we split it? <https://cs.stackexchange.com/q/144928> */
	printf("split at root, order %u, split %u\n", TRIE_ORDER, TRIE_SPLIT);
	for(lf = 0; lf <= tree->bsize; lf++)
		if(trie_bmp_test(&tree->bmp, lf)) printf("leaf %u is a link\n", lf);
	br0 = 0, br1 = tree->bsize, lf = 0;
	do {
		const struct trie_branch *const branch = tree->branch + br0;
		const unsigned right = br1 - br0 - 1 - branch->left;
		assert(br0 < br1);
		if(branch->left > right) /* Prefer right; it's less work copying. */
			br1 = ++br0 + branch->left, printf("left\n");
		else
			br0 += branch->left + 1, lf += branch->left + 1, printf("right\n");
	} while(2 * (br1 - br0) + 1 > TRIE_SPLIT);
	printf("cut at [%u..%u:%u]\n", br0, br1, lf);
	/* Copy data rooted at the current node. */
	kid->bsize = (unsigned char)(br1 - br0);
	memcpy(kid->branch, tree->branch + br0, sizeof *tree->branch * kid->bsize);
	memcpy(kid->leaf, tree->leaf + lf, sizeof *tree->leaf * (kid->bsize + 1));
	/* Subtract `tree` left branches; (right branches are implicit.) */
	br0 = 0, br1 = tree->bsize, lf = 0;
	do {
		struct trie_branch *const branch = tree->branch + br0;
		const unsigned right = br1 - br0 - 1 - branch->left;
		assert(br0 < br1);
		if(branch->left > right)
			br1 = ++br0 + branch->left, branch->left -= kid->bsize, printf("left\n");
		else
			br0 += branch->left + 1, lf += branch->left + 1, printf("right\n");
	} while(2 * (br1 - br0) + 1 > TRIE_SPLIT);
	/* Delete from `tree`. */
	memmove(tree->branch + br0, tree->branch + br1,
		sizeof *tree->branch * (tree->bsize - br1));
	memmove(tree->leaf + lf + 1, tree->leaf + lf + kid->bsize + 1,
		sizeof *tree->leaf * (tree->bsize + 1 - lf - kid->bsize - 1));
	/* Move the bits. */
	memcpy(&kid->bmp, &tree->bmp, sizeof tree->bmp);
	printf("there were %u leaves.\n", tree->bsize + 1);
	printf("moving %u leaves starting at %u leaving %u.\n",
		kid->bsize + 1, lf, tree->bsize - lf - kid->bsize);
	trie_bmp_remove(&kid->bmp, 0, lf);
	trie_bmp_remove(&kid->bmp, kid->bsize + 1,
		tree->bsize - lf - kid->bsize);
	trie_bmp_remove(&tree->bmp, lf + 1, kid->bsize);
	trie_bmp_set(&tree->bmp, lf);
	tree->leaf[lf].as_link = kid;
	tree->bsize -= kid->bsize;
	return 1;
}

/** Open up an uninitialized space in a non-full `tree`. Used in
 <fn:<PT>add_unique>.
 @param[tree, tree_bit] The start of the tree.
 @param[key, diff_bit] New key and where the new key differs from the tree.
 @return The uninitialized leaf as data. */
static union PT_(leaf) *PT_(tree_open)(struct PT_(tree) *const tree,
	size_t tree_bit, const char *const key, size_t diff_bit) {
	unsigned br0, br1, lf;
	struct trie_branch *branch;
	union PT_(leaf) *leaf;
	size_t bit1;
	unsigned is_right;
	assert(key && tree && tree->bsize < TRIE_BRANCHES);
	/* Modify the tree's left branches to account for the new leaf. */
	br0 = 0, br1 = tree->bsize, lf = 0;
	while(br0 < br1) {
		branch = tree->branch + br0;
		bit1 = tree_bit + branch->skip;
		/* Decision bits can never be the site of a difference. */
		if(diff_bit <= bit1) { assert(diff_bit < bit1); break; }
		if(!TRIE_QUERY(key, bit1))
			br1 = ++br0 + branch->left++;
		else
			br0 += branch->left + 1, lf += branch->left + 1;
		tree_bit = bit1 + 1;
	}
	assert(tree_bit <= diff_bit && diff_bit - tree_bit <= UCHAR_MAX);
	/* Should be the same as the first descent. */
	if(is_right = !!TRIE_QUERY(key, diff_bit)) lf += br1 - br0 + 1;
	/* Make room in leaves. */
	assert(lf <= tree->bsize + 1);
	leaf = tree->leaf + lf;
	memmove(leaf + 1, leaf, sizeof *leaf * ((tree->bsize + 1) - lf));
	trie_bmp_insert(&tree->bmp, lf, 1);
	/* Add a branch. */
	branch = tree->branch + br0;
	if(br0 != br1) { /* Split with existing branch. */
		assert(br0 < br1 && diff_bit + 1 <= tree_bit + branch->skip);
		branch->skip -= diff_bit - tree_bit + 1;
	}
	memmove(branch + 1, branch, sizeof *branch * (tree->bsize - br0));
	branch->left = is_right ? (unsigned char)(br1 - br0) : 0;
	branch->skip = (unsigned char)(diff_bit - tree_bit);
	tree->bsize++;
	return leaf;
}

static int PT_(add_unique)(struct T_(trie) *const trie, PT_(key) key,
	PT_(entry) **const entry) {
	struct PT_(tree) *tree;
	size_t tree_bit, bit;
	struct { size_t cur, next; } byte;
	unsigned br0, br1, lf;
	const char *sample;
	union PT_(leaf) *leaf;
	const char *key_string = PT_(key_string)(key);
	assert(trie && PT_(key_string)(key) && entry);
	if(!(tree = trie->root)) { /* Idle. */
		if(!(tree = malloc(sizeof *tree))) goto catch;
		tree->bsize = UCHAR_MAX;
		trie->root = tree;
	} /* Fall-through. */
	if(tree->bsize == UCHAR_MAX) { /* Empty. */
		tree->bsize = 0;
		trie_bmp_clear_all(&trie->root->bmp); /* Technically only need 1 bit. */
		leaf = tree->leaf + 0;
		goto assign;
	}
	/* Find the first bit not in the tree. */
	for(tree_bit = 0, bit = 0, byte.cur = 0; ;
		tree = tree->leaf[lf].as_link, tree_bit = bit) {
tree:
		br0 = 0, br1 = tree->bsize, lf = 0;
		sample = PT_(sample)(tree, 0);
		while(br0 < br1) {
			const struct trie_branch *const branch = tree->branch + br0;
			const size_t branch_bit = bit + branch->skip;
			for( ; bit < branch_bit; bit++)
				if(TRIE_DIFF(key_string, sample, bit)) goto found;
			if(!TRIE_QUERY(key_string, bit)) {
				br1 = ++br0 + branch->left;
			} else {
				br0 += branch->left + 1, lf += branch->left + 1;
				sample = PT_(sample)(tree, lf);
			}
			bit++;
		}
		/* Got to a leaf without getting a difference. */
		if(!trie_bmp_test(&tree->bmp, lf)) break;
	}
	{ /* Too much similarity to fit in a byte, (~32 characters.) */
		const size_t limit = bit + UCHAR_MAX;
		while(!TRIE_DIFF(key_string, sample, bit))
			if(++bit > limit) { errno = EILSEQ; goto catch; }
	}
found:
	/* Account for choosing the right leaf. */
	if(!!TRIE_QUERY(key_string, bit)) lf += br1 - br0 + 1;
	/* Split. This is inefficient in that it moves data one time for split and
	 a subset of the data a second time for insert. It also is agnostic of the
	 key that we are going to put in. Having `TREE_ORDER` be more makes this
	 matter less. */
	assert(tree->bsize <= TRIE_BRANCHES);
	if(tree->bsize == TRIE_BRANCHES) {
		if(!PT_(split)(tree)) goto catch;
		bit = tree_bit;
		goto tree; /* Start again from the top of the first tree. */
	}
	/* Tree is not full. */
	leaf = PT_(tree_open)(tree, tree_bit, key_string, bit);
assign:
#ifndef TRIE_VALUE /* <!-- key set */
	leaf->as_entry = key;
#elif !defined(TRIE_KEY_IN_VALUE) /* key set --><!-- key value map */
	leaf->as_entry.key = key;
#else /* key value map --><!-- key in value */
	/* Will rely on user to do it; oy. We potentially do not have all the
	 information to do it here. */
#endif /* key in value --> */
	*entry = &leaf->as_entry;
	return 1;
catch:
	/* `malloc` doesn't have to set it according to `C89`. */
	if(!errno) errno = ERANGE;
	return 0;
}

/** Destroys `tree`'s children and sets invalid state.
 @order \O(|`tree`|) both time and space. */
static void PT_(clear_r)(struct PT_(tree) *const tree) {
	unsigned i;
	assert(tree && tree->bsize != UCHAR_MAX);
	for(i = 0; i <= tree->bsize; i++) if(trie_bmp_test(&tree->bmp, i))
		PT_(clear_r)(tree->leaf[i].as_link), free(tree->leaf[i].as_link);
}

struct T_(trie_cursor) { struct PT_(cursor) _; };

/** Zeroed data (not all-bits-zero) is initialized. @return An idle tree.
 @order \Theta(1) @allow */
static struct T_(trie) T_(trie)(void)
	{ struct T_(trie) trie = { 0 }; return trie; }

/** Returns any initialized `trie` (can be null) to idle.
 @order \O(|`trie`|) @allow */
static void T_(trie_)(struct T_(trie) *const trie) {
	if(!trie || !trie->root) return; /* Null or idle. */
	if(trie->root->bsize != UCHAR_MAX) PT_(clear_r)(trie->root); /* Contents. */
	free(trie->root); /* Empty. */
	*trie = T_(trie)();
}

/** Clears every entry in a valid `trie`, but it continues to be active if it
 is not idle. @order \O(|`trie`|) @allow */
static void T_(trie_clear)(struct T_(trie) *const trie) {
	if(!trie || !trie->root) return; /* Null or idle. */
	if(trie->root->bsize != UCHAR_MAX) PT_(clear_r)(trie->root); /* Contents. */
	trie->root->bsize = UCHAR_MAX; /* Hysteresis. */
}




#if 0
/** Initializes `trie` from an `array` of pointers-to-`<T>` of `array_size`.
 @return Success. @throws[realloc] @order \O(`array_size`) @allow
 @fixme Write this function, somehow. */
static int T_(trie_from_array)(struct T_(trie) *const trie,
	PT_(type) *const*const array, const size_t array_size) {
	return assert(trie && array && array_size),
		PT_(init)(trie, array, array_size);
}
/** @return Looks at only the index of `trie` for potential `key` matches,
 but will ignore the values of the bits that are not in the index.
 @order \O(|`key`|) @allow */
static PT_(entry) T_(trie_match)(const struct T_(trie) *const trie,
	const char *const string)
	{ return trie && trie->root && string
		? PT_(match)(trie->root, string) : 0; }
#endif

/** @return Exact match for `key` in `trie` or null no such item exists. If
 either is null, returns a null entry, that is, key or key in value, null,
 entry both are null. @order \O(|`key`|), <Thareja 2011, Data>. @allow */
/** @param[result] If null, behaves like <fn:<T>trie_is>, otherwise, a
 <typedef:<PT>entry> which gets filled on true.
 @return Whether `key` is in `table` (which can be null.) @allow */
static int T_(trie_query)(const struct T_(trie) *const trie,
	const char *const string, PT_(entry) *const result)
	{ return trie && string ? PT_(query)(trie, string, result) : 0; }

#ifndef TRIE_VALUE /* <!-- key set */

/** Adds `key` to `trie` if it doesn't exist. */
static enum trie_result T_(trie_try)(struct T_(trie) *const trie,
	const PT_(key) key) {
	PT_(entry) *e;
	assert(trie && PT_(key_string)(key));
	printf("try(%s)\n", PT_(key_string)(key));
	return PT_(query)(trie, PT_(key_string)(key), 0) ? TRIE_PRESENT
		: (PT_(add_unique)(trie, key, &e) ? TRIE_UNIQUE : TRIE_ERROR);
}

#else /* key set --><!-- key value map OR key in value */

/** Adds `key` to `trie` if it doesn't exist already.
 @param[value] Only if `TRIE_VALUE` is set will this parameter exist; output
 pointer. Can be null only if `TRIE_KEY_IN_VALUE` was not defined.
 @return One of, `TRIE_ERROR`, `errno` is set and `value` is not;
 `TRIE_UNIQUE`, added to `trie`, and uninitialized `value` is associated with
 `key`; `TRIE_PRESENT`, the value associated with `key`. If `TRIE_IN_VALUE`,
 was specified and the return is `TRIE_UNIQUE`, the trie is in an invalid state
 until filling in the key in value by `key`.
 @order \O(max(|`trie.keys`|)) */
static enum trie_result T_(trie_try)(struct T_(trie) *const trie,
	const PT_(key) key, PT_(value) **const value) {
	PT_(entry) *e, result;
	assert(trie && PT_(key_string)(key));
	printf("try(%s)\n", PT_(key_string)(key));
	if(PT_(query)(trie, PT_(key_string)(key), &result)) {
		if(value) *value = PT_(entry_value)(&result);
		return TRIE_PRESENT;
	}
	if(!PT_(add_unique)(trie, key, &e)) return TRIE_ERROR;
#ifndef TRIE_KEY_IN_VALUE /* <!-- key value map */
	if(value) *value = &e->value;
#else /* key value map --><!-- key in value */
	assert(value);
	*value = e;
#endif /* key in value --> */
	return TRIE_UNIQUE;
}

#endif /* trie value map OR key in value --> */

/* "set" that's the JavaScript way, I like that.
 "assign" is also JS.
 trie.try(key)
 trie.try(key, &value)
 //? trie.try(&value)

 trie.assign(key, &eject)
 trie.assign(key, &eject, &value)
 trie.assign(&value, &eject) */


#if 0
/** Updates or adds a pointer to `x` into `trie`.
 @param[eject] If not null, on success it will hold the overwritten value or
 a pointer-to-null if it did not overwrite any value.
 @return Success. @throws[realloc, ERANGE] @order \O(|`key`|) @allow */
static int T_(trie_put)(struct T_(trie) *const trie, const PT_(entry) x,
	PT_(entry) */*const fixme*/eject)
	{ return PT_(put)(trie, x, &eject, 0); }

/** Adds a pointer to `x` to `trie` only if the entry is absent or if calling
 `replace` returns true or is null.
 @param[eject] If not null, on success it will hold the overwritten value or
 a pointer-to-null if it did not overwrite any value. If a collision occurs and
 `replace` does not return true, this will be a pointer to `x`.
 @param[replace] Called on collision and only replaces it if the function
 returns true. If null, it is semantically equivalent to <fn:<T>trie_put>.
 @return Success. @throws[realloc, ERANGE] @order \O(|`key`|) @allow */
static int T_(trie_policy)(struct T_(trie) *const trie, const PT_(entry) x,
	PT_(entry) *eject, const PT_(replace_fn) replace)
	{ return assert(trie && x), PT_(put)(trie, x, &eject, replace); }

/** Tries to remove `key` from `trie`. @return Success. */
static int T_(trie_remove)(struct T_(trie) *const trie,
	const char *const key) { return PT_(remove)(trie, key); }
#endif

/** Fills `it` with iteration parameters that find values of keys that start
 with `prefix` in `trie`.
 @param[prefix] To fill `it` with the entire `trie`, use the empty string.
 @param[it] A pointer to an iterator that gets filled. It is valid until a
 topological change to `trie`. Calling <fn:<T>trie_next> will iterate them in
 order. @order \O(\log `trie.size`) or \O(|`prefix`|) @allow */
static void T_(trie_prefix)(struct T_(trie) *const trie,
	const char *const prefix, struct T_(trie_cursor) *const cur)
	{ assert(cur); PT_(prefix)(trie, prefix, &cur->_); }

/** Advances `it`. @return The previous value or null. @allow */
static const PT_(entry) *T_(trie_next)(struct T_(trie_cursor) *const it)
	{ return PT_(next)(&it->_); }

static size_t PT_(size_r)(const struct PT_(cursor) *const it) {
	return it->end.idx - it->cur.idx; /* Fixme. */
}

/** Counts the of the items in initialized `it`. @order \O(|`it`|) @allow */
static size_t T_(trie_size)(const struct T_(trie_cursor) *const cur)
	{ return assert(cur), PT_(size_r)(&cur->_); }


/* Box override information. */
#define BOX_ PT_
#define BOX struct T_(trie)


/* cursor... */


#ifdef TRIE_TEST /* <!-- test */
#include "../test/test_trie.h" /** \include */
#endif /* test --> */

#ifdef TRIE_TO_STRING /* <!-- str: _sic_, have a natural string. */
#define STR_(n) TRIE_CAT(T_(trie), n)
/** Uses the natural `a` -> `z` that is defined by `TRIE_KEY_IN_VALUE`.
 @fixme `sprintf` is large and cumbersome when a case statement will do. */
static void PT_(to_string)(const PT_(entry) *const e,
	char (*const z)[12]) {
	assert(e && z), sprintf(*z, "%.11s", PT_(key_string(PT_(entry_key)(e))));
}
#define TO_STRING &PT_(to_string)
#define TO_STRING_LEFT '{'
#define TO_STRING_RIGHT '}'
#include "to_string.h" /** \include */
#ifdef TRIE_TEST /* <!-- expect: nothing is forward-declared. */
#undef TRIE_TEST
#endif /* expect --> */
#undef STR_
#undef TRIE_TO_STRING
#endif /* str --> */

/*static void PT_(unused_base_coda)(void);
static void PT_(unused_base)(void) {
	PT_(begin)(0, 0);
	T_(trie)(0); T_(trie_)(0); T_(trie_clear)(0);
	T_(trie_is)(0, 0); T_(trie_match)(0, 0); T_(trie_get)(0, 0);
	T_(trie_try)(0, 0); T_(trie_put)(0, 0, 0); T_(trie_policy)(0, 0, 0, 0);
	T_(trie_remove)(0, 0);
	T_(trie_prefix)(0, 0, 0); T_(trie_next)(0); T_(trie_size)(0);
	PT_(unused_base_coda)();
}
static void PT_(unused_base_coda)(void) { PT_(unused_base)(); }*/


#elif defined(TABLE_DEFAULT) /* base --><!-- default */


#error Not there yet.


#endif /* traits --> */


#ifdef TRIE_EXPECT_TRAIT /* <!-- trait */
#undef TRIE_EXPECT_TRAIT
#else /* trait --><!-- !trait */
#ifdef TRIE_TEST
#error No TRIE_TO_STRING trait defined for TRIE_TEST.
#endif
#undef TRIE_NAME
#ifdef TRIE_VALUE
#undef TRIE_VALUE
#endif
#ifdef TRIE_KEY
#undef TRIE_KEY
#undef TRIE_KEY_TO_STRING
#endif
#ifdef TRIE_KEY_IN_VALUE
#undef TRIE_KEY_IN_VALUE
#endif
#undef BOX_
#undef BOX
#undef BOX_CONTENT
#undef BOX_ITERATOR
#endif /* !trait --> */
#undef TRIE_DEFAULT_TRAIT
#undef TRIE_TRAITS
