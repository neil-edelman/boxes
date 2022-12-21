/** @license 2020 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @abstract Header <src/trie.h> requires <src/bmp.h>; examples
 <test/test_trie.c>; article <doc/trie.pdf>. On a compatible workstation,
 `make` creates the test suite of the examples.

 @subtitle Prefix tree

 ![Example of trie.](../doc/trie.png)

 A <tag:<T>trie> is a prefix-tree, digital-tree, or trie: an ordered set or map
 of byte null-terminated immutable key strings allowing efficient prefix
 queries. The implementation is as <Morrison, 1968 PATRICiA>: a compact
 [binary radix trie](https://en.wikipedia.org/wiki/Radix_tree) that acts as an
 index, only storing the where the key bits are different. The keys are grouped
 in fixed-size nodes in a relaxed version of a B-tree, as
 <Bayer, McCreight, 1972 Large>, where the height is no longer fixed.

 The worse-case run-time of querying or modifying, \O(|`string`|); however,
 this presumes that the string is packed with decision bits. In reality, the
 bottleneck is more the density of looked-at bits. In an iid model,
 <Tong, Goebel, Lin, 2015, Smoothed> showed that the performance was
 \O(\log |`trie`|).

 ![Bit view of the trie.](../doc/trie-bits.png)

 @param[TRIE_NAME]
 Required `<T>` that satisfies `C` naming conventions when mangled. `<PT>` is
 private, whose names are prefixed in a manner to avoid collisions.

 @param[TRIE_KEY]
 Normally, the key is directly compatible with `const char *`. Optionally, one
 can set `TRIE_KEY` to a custom type,
 <typedef:<PT>key>, needing <typedef:<PT>string_fn> `<T>string`.

 @param[TRIE_VALUE, TRIE_KEY_IN_VALUE]
 `TRIE_VALUE` is an optional payload type to go with the key. Further, defining
 `TRIE_KEY_IN_VALUE` says that <typedef:<PT>key_fn> `<T>key` picks out
 <typedef:<PT>key> from <typedef:<PT>value>. Otherwise it is an associative
 array from a key to value, <tag:<T>trie_entry>.

 @param[TRIE_TO_STRING]
 To string trait `<STR>` contained in <src/to_string.h>. The unnamed trait is
 automatically supplied by the string, but others require
`<name><trait>to_string` be declared as <typedef:<PSTR>to_string_fn>.

 @param[TRIE_DEFAULT]
 Get or default set default. (fixme: upcoming.)

 @param[TRIE_EXPECT_TRAIT, TRIE_TRAIT]
 Named traits are obtained by including `trie.h` multiple times with
 `TRIE_EXPECT_TRAIT` and then subsequently including the name in `TRIE_TRAIT`.

 @std C89 (Specifically, ISO/IEC 9899/AMD1:1995 because it uses EILSEQ.) */

#ifndef TRIE_NAME
#error Name TRIE_NAME undefined.
#endif
#if defined(TRIE_TRAIT) ^ defined(BOX_TYPE)
#error TRIE_TRAIT name must come after TRIE_EXPECT_TRAIT.
#endif
#if defined(TRIE_KEY_IN_VALUE) && !defined(TRIE_VALUE)
#error TRIE_KEY_IN_VALUE requires TRIE_VALUE.
#endif
#if defined(TRIE_TEST) && (!defined(TRIE_TRAIT) && !defined(TRIE_TO_STRING) \
	|| defined(TRIE_TRAIT) && !defined(TRIE_HAS_TO_STRING))
#error Test requires to string.
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
/* Worst-case all-branches-left root. Parameter sets the maximum tree size.
 Prefer alignment `4n - 2`; cache `32n - 2`, (`(left + 1) * 2 + 2`.) */
#define TRIE_MAX_LEFT 254
#if TRIE_MAX_LEFT < 1 || TRIE_MAX_LEFT > UCHAR_MAX
#error TRIE_MAX_LEFT parameter range `[1, UCHAR_MAX]`.
#endif
#define TRIE_BRANCHES (TRIE_MAX_LEFT + 1) /* Maximum branches. */
#define TRIE_ORDER (TRIE_BRANCHES + 1) /* Maximum branching factor/leaves. */
/* `⌈(2n-1)/3⌉` nodes. */
#define TRIE_SPLIT ((2 * (TRIE_ORDER + TRIE_BRANCHES) - 1 + 2) / 3)
#define TRIE_RESULT X(ERROR), X(UNIQUE), X(PRESENT)
#define X(n) TRIE_##n
/** A result of modifying the table, of which `TRIE_ERROR` is false.

 ![A diagram of the result states.](../doc/result.png) */
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
/** @return Whether `prefix` is the prefix of `word`.
 Used in <fn:<T>trie_prefix>. */
static int trie_is_prefix(const char *prefix, const char *word) {
	for( ; ; prefix++, word++) {
		if(*prefix == '\0') return 1;
		if(*prefix != *word) return 0;
	}
}
#endif /* idempotent --> */


#ifndef TRIE_TRAIT /* <!-- base trie */

#ifdef TRIE_KEY /* <!-- custom key */

/** The default is assignable `const char *`. If one sets `TRIE_KEY` to
 something other than that, then one must also declare `<P>string` as a
 <typedef:<PT>string_fn>. */
typedef TRIE_KEY PT_(key);

#else /* custom key --><!-- string key */

typedef const char *PT_(key);

#endif /* string key --> */

/** Transforms a <typedef:<PT>key> into a `const char *`, if `TRIE_KEY` has
 been set. */
typedef const char *(*PT_(string_fn))(PT_(key));

#ifdef TRIE_KEY /* <!-- custom key */

/* Valid <typedef:<PT>key_to_string_fn>. */
static PT_(string_fn) PT_(key_string) = &T_(string);

#else /* custom key --><!-- string key */

/** @return The string of `key` is itself, by default.
 @implements <typedef:<PT>string_fn> */
static const char *PT_(string_string)(const char *const key) { return key; }
static PT_(string_fn) PT_(key_string) = &PT_(string_string);

#endif /* string key --> */

#ifndef TRIE_VALUE /* <!-- key set */
typedef PT_(key) PT_(entry);
/** @return The key of `e` is itself in a set. */
static PT_(key) PT_(entry_key)(const PT_(entry) *const e) { return *e; }
#elif !defined(TRIE_KEY_IN_VALUE) /* key set --><!-- key value map */
typedef TRIE_VALUE PT_(value);
/** On `KEY_VALUE` but not `KEY_KEY_IN_VALUE`, defines an entry. */
struct T_(trie_entry) { PT_(key) key; PT_(value) value; };
typedef struct T_(trie_entry) PT_(entry);
/** @return Key of `e` in a map. */
static PT_(key) PT_(entry_key)(const struct T_(trie_entry) *const e)
	{ return e->key; }
#else /* key value map --><!-- key in value */
/* The entry is the value. */
typedef TRIE_VALUE PT_(value);
typedef PT_(value) PT_(entry);
/** If `TRIE_KEY_IN_VALUE`, extracts the key from `TRIE_VALUE`; in this case,
 the user makes a contract to set the key on new entries before using the trie
 again, (mostly, can still match, but not reliably modify the topology.) */
typedef PT_(key) (*PT_(key_fn))(const PT_(value) *);
/* Valid <typedef:<PT>key_fn>. */
static PT_(key_fn) PT_(read_key) = &T_(key);
/** @return A key-in-value `v`. */
static const char *PT_(entry_key)(const PT_(value) *const v)
	{ return PT_(read_key)(v); }
#endif /* key in value --> */

union PT_(leaf) { PT_(entry) as_entry; struct PT_(tree) *as_link; };
/* Node already has conflicting meaning, so we use tree. Such that a trie is a
 forest of non-empty complete binary trees. In a B-tree, described using
 <Knuth, 1998 Art 3> terminology, this is a node of `TRIE_ORDER`. */
struct PT_(tree) {
	unsigned short bsize;
	struct trie_branch branch[TRIE_BRANCHES];
	struct trie_bmp bmp;
	union PT_(leaf) leaf[TRIE_ORDER];
};
/** To initialize it to an idle state, see <fn:<T>trie>, `{0}`, or being
 `static`.

 ![States.](../doc/states.png) */
struct T_(trie);
struct T_(trie) { struct PT_(tree) *root; };

struct PT_(ref) { struct PT_(tree) *tree; unsigned lf; };

/** Fall through `ref` until hit the first entry. Must be pointing at
 something. */
static void PT_(lower_entry)(struct PT_(ref) *ref) {
	while(trie_bmp_test(&ref->tree->bmp, ref->lf))
		ref->tree = ref->tree->leaf[ref->lf].as_link, ref->lf = 0;
}
/** Fall through `ref` until hit the last entry. Must be pointing at
 something. */
static void PT_(higher_entry)(struct PT_(ref) *ref) {
	while(trie_bmp_test(&ref->tree->bmp, ref->lf))
		ref->tree = ref->tree->leaf[ref->lf].as_link,
		ref->lf = ref->tree->bsize;
}
/** This is a convince function.
 @return The leftmost entry string at `lf` of `tree`. */
static const char *PT_(sample)(struct PT_(tree) *const tree,
	const unsigned lf) {
	struct PT_(ref) ref; ref.tree = tree, ref.lf = lf;
	PT_(lower_entry)(&ref);
	return PT_(key_string)(PT_(entry_key)(&ref.tree->leaf[ref.lf].as_entry));
}

/* A range of words from `[cur, end]`. */
struct PT_(iterator) {
	const struct T_(trie) *trie;
	struct PT_(ref) cur, end;
	int seen;
};
/** Looks at only the index of `trie` (which can be null) for potential
 `prefix` matches, and stores them in `it`. */
static struct PT_(iterator) PT_(match_prefix)
	(const struct T_(trie) *const trie, const char *const prefix) {
	struct PT_(iterator) it;
	struct PT_(tree) *tree;
	size_t bit;
	struct { size_t cur, next; } byte;
	assert(trie && prefix);
	it.trie = 0;
	if(!(tree = trie->root) || tree->bsize == USHRT_MAX) return it;
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
		it.trie = trie;
		it.cur.tree = it.end.tree = tree;
		it.cur.lf = lf, PT_(lower_entry)(&it.cur);
		it.end.lf = lf + br1 - br0, PT_(higher_entry)(&it.end);
		it.seen = 0;
		break;
	}
	return it;
}
/** Loads the first element of `trie` (can be null) into `it`.
 @implements begin */
static struct PT_(iterator) PT_(begin)(const struct T_(trie) *const trie)
	{ return PT_(match_prefix)(trie, ""); }

/** If `ref.tree` is null, starts iteration.
 @return Does `ref` have a successor in `root`? If yes, sets it to that. */
static int PT_(to_successor)(struct PT_(tree) *const root,
	struct PT_(ref) *const ref) {
	assert(ref);
	if(!root || root->bsize == USHRT_MAX) return 0; /* Empty. */
	if(!ref->tree) { ref->tree = root, ref->lf = 0; } /* Start. */
	else if(++ref->lf > ref->tree->bsize) { /* Gone off the end. */
		struct PT_(tree) *const old = ref->tree;
		const char *const sample = PT_(sample)(old, ref->lf - 1);
		struct PT_(tree) *tree = root;
		size_t bit = 0;
		for(ref->tree = 0, ref->lf = 0; tree != old; ) {
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
			if(lf < tree->bsize) ref->tree = tree, ref->lf = lf + 1;
			assert(trie_bmp_test(&tree->bmp, lf));
			tree = tree->leaf[lf].as_link;
		}
		if(!ref->tree) return 0; /* End of iteration. */
	}
	PT_(lower_entry)(ref);
	return 1;
}
/** Advances `it`. @return The previous value or null. @implements next */
static int PT_(next)(struct PT_(iterator) *const it,
	struct PT_(ref) **const ref) {
	assert(it);
	/* Possibly this is still valid? */
	if(!it->trie || !it->cur.tree || it->cur.tree->bsize < it->cur.lf
		|| !it->end.tree || it->end.tree->bsize < it->end.lf) return 0;
	if(it->seen) {
		/* We have reached the planned end or concurrent modification. */
		if(it->cur.tree == it->end.tree && it->cur.lf >= it->end.lf
			|| !PT_(to_successor)(it->trie->root, &it->cur)) return 0;
	} else {
		it->seen = 1;
	}
	assert(!trie_bmp_test(&it->cur.tree->bmp, it->cur.lf));
	if(ref) *ref = &it->cur;
	return 1;
}


/** Zeroed data (not all-bits-zero) is initialized. @return An idle tree.
 @order \Theta(1) @allow */
static struct T_(trie) T_(trie)(void)
	{ struct T_(trie) trie = { 0 }; return trie; }
#if 0
/** Initializes `trie` from an `array` of pointers-to-`<T>` of `array_size`.
 @return Success. @throws[realloc] @order \O(`array_size`) @allow
 @fixme Write this function, somehow. */
static int T_(trie_from_array)(struct T_(trie) *const trie,
	PT_(type) *const*const array, const size_t array_size) {
	return assert(trie && array && array_size),
		PT_(init)(trie, array, array_size);
}
#endif


/* Clearing entries. */

/** Destroys `tree`'s children and sets invalid state.
 @order \O(|`tree`|) both time and space. */
static void PT_(clear_r)(struct PT_(tree) *const tree) {
	unsigned i;
	assert(tree && tree->bsize != USHRT_MAX);
	for(i = 0; i <= tree->bsize; i++) if(trie_bmp_test(&tree->bmp, i))
		PT_(clear_r)(tree->leaf[i].as_link), free(tree->leaf[i].as_link);
}
/** Returns any initialized `trie` (can be null) to idle.
 @order \O(|`trie`|) @allow */
static void T_(trie_)(struct T_(trie) *const trie) {
	if(!trie || !trie->root) return; /* Null or idle. */
	if(trie->root->bsize != USHRT_MAX) PT_(clear_r)(trie->root); /* Contents. */
	free(trie->root); /* Empty. */
	*trie = T_(trie)();
}
/** Clears every entry in a valid `trie` (can be null), but it continues to be
 active if it is not idle. @order \O(|`trie`|) @allow */
static void T_(trie_clear)(struct T_(trie) *const trie) {
	if(!trie || !trie->root) return; /* Null or idle. */
	if(trie->root->bsize != USHRT_MAX) PT_(clear_r)(trie->root); /* Contents. */
	trie->root->bsize = USHRT_MAX; /* Hysteresis. */
}


/* Lookup. Match just looks at the index. */

/** @return A candidate match for `string` in `trie`, (which must both be
 non-null), or null, if `key` is definitely not in the trie. */
static PT_(entry) *PT_(match)(const struct T_(trie) *const trie,
	const char *const string) {
	struct PT_(tree) *tree;
	size_t bit; /* In bits of `key`. */
	struct { size_t cur, next; } byte; /* `key` null checks. */
	assert(trie && string);
	if(!(tree = trie->root) || tree->bsize == USHRT_MAX) return 0; /* Empty. */
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
			return &tree->leaf[lf].as_entry;
		tree = tree->leaf[lf].as_link; /* Jumped trees. */
	}
}
/** Looks at only the index of `trie` for potential `string` (can both
 be null) matches. Does not access the string itself, thus will ignore the
 bits that are not in the index.
 @return A candidate match for `string` or null. @order \O(|`string`|) @allow */
static PT_(entry) *T_(trie_match)(const struct T_(trie) *const trie,
	const char *const string)
	{ return trie && string ? PT_(match)(trie, string) : 0; }
/** @return An exact match for `string` in `trie`. */
static PT_(entry) *PT_(get)(const struct T_(trie) *const trie,
	const char *const string) {
	PT_(entry) *entry;
	assert(trie && string);
	return (entry = PT_(match)(trie, string))
		&& !strcmp(PT_(key_string)(PT_(entry_key)(entry)), string)
		? entry : 0;
}
/** @return Exact `string` match for `trie` or null, (both can be null.)
 @order \O(\log |`trie`|) iid @allow */
static PT_(entry) *T_(trie_get)(const struct T_(trie) *const trie,
	const char *const string)
	{ return trie && string ? PT_(get)(trie, string) : 0; }
/** Stores all `prefix` matches in `trie` in `it`. */
static struct PT_(iterator) PT_(prefix)(struct T_(trie) *const trie,
	const char *const prefix) {
	struct PT_(iterator) it;
	assert(trie && prefix);
	it = PT_(match_prefix)(trie, prefix);
	/* Make sure actually a prefix. */
	if(it.trie && !trie_is_prefix(prefix,
		PT_(key_string)(PT_(entry_key)(
		&it.cur.tree->leaf[it.cur.lf].as_entry))))
		it.trie = 0;
	return it;
}
/** Represents a range of in-order keys in \O(1) space. */
struct T_(trie_iterator);
struct T_(trie_iterator) { struct PT_(iterator) _; };
/** @return An iterator set to strings that start with `prefix` in `trie`.
 It is valid until a topological change to `trie`. Calling <fn:<T>trie_next>
 will iterate them in order.
 @param[prefix] To fill with the entire `trie`, use the empty string.
 @order \O(|`prefix`|) @allow */
static struct T_(trie_iterator) T_(trie_prefix)(struct T_(trie) *const trie,
	const char *const prefix) {
	struct T_(trie_iterator) it;
	it._ = PT_(prefix)(trie, prefix);
	return it;
}

#ifdef TRIE_VALUE /* <!-- map */
/** @return Whether advancing `it` to the next element and filling `k`, (and
 `v` if a map, otherwise absent,) if not-null.
 @order \O(\log |`trie`|) @allow */
static int T_(trie_next)(struct T_(trie_iterator) *const it,
	PT_(key) *const k, PT_(value) **v) {
#else /* map --><!-- set */
static int T_(trie_next)(struct T_(trie_iterator) *const it,
	PT_(key) *const k) {
#endif /* set --> */
	struct PT_(ref) *r;
	if(!PT_(next)(&it->_, &r)) return 0;
	if(k) *k = PT_(entry_key)(&r->tree->leaf[r->lf].as_entry);
#ifdef TRIE_VALUE
#ifdef TRIE_KEY_IN_VALUE
	if(v) *v = &r->tree->leaf[r->lf].as_entry;
#else
	if(v) *v = &r->tree->leaf[r->lf].as_entry.value;
#endif
#endif
	return 1;
#ifdef TRIE_VALUE
}
#else
}
#endif


#if 0
/** @return The number of elements in `it`. */
static size_t PT_(size_r)(const struct PT_(iterator) *const it) {
	return it->end.lf - it->cur.lf; /* Fixme. */
}
/** Counts the of the items in `it`. @order \O(|`it`|) @allow
 @fixme Doesn't work at all. */
static size_t T_(trie_size)(const struct T_(trie_iterator) *const it)
	{ return assert(it), PT_(size_r)(&it->_); }
#endif


/* Adding new entries. */

/** Splits `tree` into two trees. Used in <fn:<PT>add>. @throws[malloc] */
static int PT_(split)(struct PT_(tree) *const tree) {
	unsigned br0, br1, lf;
	struct PT_(tree) *kid;
	assert(tree && tree->bsize == TRIE_BRANCHES);
	/* Mitosis; more info added on error in <fn:<PT>add_unique>. */
	if(!(kid = malloc(sizeof *kid))) return 0;
	/* Where should we split it? <https://cs.stackexchange.com/q/144928> */
	br0 = 0, br1 = tree->bsize, lf = 0;
	do {
		const struct trie_branch *const branch = tree->branch + br0;
		const unsigned right = br1 - br0 - 1 - branch->left;
		assert(br0 < br1);
		if(branch->left > right) /* Prefer right; it's less work copying. */
			br1 = ++br0 + branch->left;
		else
			br0 += branch->left + 1, lf += branch->left + 1;
	} while(2 * (br1 - br0) + 1 > TRIE_SPLIT);
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
			br1 = ++br0 + branch->left, branch->left -= kid->bsize;
		else
			br0 += branch->left + 1, lf += branch->left + 1;
	} while(2 * (br1 - br0) + 1 > TRIE_SPLIT);
	/* Delete from `tree`. */
	memmove(tree->branch + br0, tree->branch + br1,
		sizeof *tree->branch * (tree->bsize - br1));
	memmove(tree->leaf + lf + 1, tree->leaf + lf + kid->bsize + 1,
		sizeof *tree->leaf * (tree->bsize + 1 - lf - kid->bsize - 1));
	/* Move the bits. */
	memcpy(&kid->bmp, &tree->bmp, sizeof tree->bmp);
	trie_bmp_remove(&kid->bmp, 0, lf);
	trie_bmp_remove(&kid->bmp, kid->bsize + 1,
		tree->bsize - lf - kid->bsize);
	trie_bmp_remove(&tree->bmp, lf + 1, kid->bsize);
	trie_bmp_set(&tree->bmp, lf);
	tree->leaf[lf].as_link = kid;
	tree->bsize -= kid->bsize;
	return 1;
}
/** Open up an uninitialized space in a non-full `tree`. Used in <fn:<PT>add>.
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
/** Adds `key` to `trie` and stores it in `entry`, if it is not null. On
 unique, fills in the `key` unless `TRIE_KEY_IN_VALUE` (because it doesn't have
 enough information, in that case.) @return A result. */
static enum trie_result PT_(add)(struct T_(trie) *const trie, PT_(key) key,
	PT_(entry) **const entry) {
	struct PT_(ref) ref;
	unsigned br0, br1;
	union PT_(leaf) *leaf;
	struct { size_t cur, next; } byte; /* `key` null checks. */
	PT_(entry) *exemplar;
	const char *const key_string = PT_(key_string)(key), *exemplar_string;
	size_t bit1, diff, tree_bit1;
	assert(trie && PT_(key_string)(key));
	if(!(ref.tree = trie->root)) { /* Idle. */
		if(!(ref.tree = malloc(sizeof *ref.tree))) goto catch;
		ref.tree->bsize = USHRT_MAX;
		trie->root = ref.tree;
	} /* Fall-through. */
	if(ref.tree->bsize == USHRT_MAX) { /* Empty: special case. */
		ref.tree->bsize = 0;
		trie_bmp_clear_all(&ref.tree->bmp);
		leaf = ref.tree->leaf + 0;
		goto assign;
	}
	/* Otherwise we will be able to find an exemplar: a neighbouring key to the
	 new key up to the difference, (after that, it doesn't matter.) */
	for(bit1 = 0, byte.cur = 0; ;
		ref.tree = ref.tree->leaf[ref.lf].as_link) {
		br0 = 0, br1 = ref.tree->bsize, ref.lf = 0; /* Leaf. */
		while(br0 < br1) {
			const struct trie_branch *const branch = ref.tree->branch + br0;
			for(byte.next = (bit1 += branch->skip) / CHAR_BIT;
				byte.cur < byte.next; byte.cur++)
				if(key_string[byte.cur] == '\0') {
					bit1++;
					PT_(lower_entry)(&ref); /* Arbitrarily pick the first. */
					goto found_exemplar;
				}
			if(!TRIE_QUERY(key_string, bit1))
				br1 = ++br0 + branch->left;
			else
				br0 += branch->left + 1, ref.lf += branch->left + 1;
			bit1++;
		}
		if(!trie_bmp_test(&ref.tree->bmp, ref.lf)) break; /* One exemplar. */
	}
found_exemplar:
	exemplar = &ref.tree->leaf[ref.lf].as_entry;
	exemplar_string = PT_(key_string)(PT_(entry_key)(exemplar));
	/* Do a string comparison to find difference, first bytes, then bits. */
	{
		const char *k = key_string, *e = exemplar_string;
		for(diff = 0; *k == *e; k++, e++) {
			if(*k == '\0') { if(entry) *entry = exemplar; return TRIE_PRESENT; }
			diff += CHAR_BIT;
			/* Both one ahead at this point, (they might not exist.) */
			if(bit1 + UCHAR_MAX < diff) return errno = EILSEQ, TRIE_ERROR;
		}
	}
	/* `diff` switched from ahead; now actual; there is a difference. */
	while(!TRIE_DIFF(key_string, exemplar_string, diff)) diff++;
	assert(!bit1 || diff != bit1 - 1);
	/* Too much similarity to fit in a limited skip, (~32 characters.) */
	if((bit1 ? bit1 - 1 : 0) + UCHAR_MAX < diff) { errno = EILSEQ; goto catch; }
	/* Restart and go to the difference. */
	for(bit1 = 0, ref.tree = trie->root; ;
		ref.tree = ref.tree->leaf[ref.lf].as_link) {
		tree_bit1 = bit1;
tree:
		br0 = 0, br1 = ref.tree->bsize, ref.lf = 0;
		while(br0 < br1) {
			const struct trie_branch *const branch = ref.tree->branch + br0;
			if(diff <= (bit1 += branch->skip)) goto found_diff;
			if(!TRIE_QUERY(key_string, bit1))
				br1 = ++br0 + branch->left;
			else
				br0 += branch->left + 1, ref.lf += branch->left + 1;
			bit1++;
		}
		if(!trie_bmp_test(&ref.tree->bmp, ref.lf)) goto found_diff;
	}
found_diff:
	/* Account for choosing the right leaf. */
	if(TRIE_QUERY(key_string, diff)) ref.lf += br1 - br0 + 1;
	/* Split. Agnostic of the key, also inefficient in that it moves data one
	 time for split and a subset of the data a second time for insert. Having
	 `TREE_ORDER` be more makes this matter less. */
	assert(ref.tree->bsize <= TRIE_BRANCHES);
	if(ref.tree->bsize == TRIE_BRANCHES) {
		if(!PT_(split)(ref.tree)) goto catch; /* Takes memory. */
		bit1 = tree_bit1;
		goto tree; /* Start again from the top of the first tree. */
	}
	/* Tree is not full. */
	leaf = PT_(tree_open)(ref.tree, tree_bit1, key_string, diff);
assign:
#ifndef TRIE_VALUE /* <!-- key set */
	leaf->as_entry = key;
#elif !defined(TRIE_KEY_IN_VALUE) /* key set --><!-- key value map */
	leaf->as_entry.key = key;
#else /* key value map --><!-- key in value */
	/* Do not have enough information; rely on user to do it. */
#endif /* key in value --> */
	if(entry) *entry = &leaf->as_entry;
	return TRIE_UNIQUE;
catch:
	if(!errno) errno = ERANGE; /* `malloc` only has to set it in POSIX. */
	return TRIE_ERROR;
}
#ifndef TRIE_VALUE /* <!-- key set */
/** Adds `key` to `trie` (which must both exist) if it doesn't exist. */
static enum trie_result T_(trie_try)(struct T_(trie) *const trie,
	const PT_(key) key) {
	assert(trie && PT_(key_string)(key));
	return PT_(add)(trie, key, 0);
}
#else /* key set --><!-- key value map OR key in value */
/** Adds `key` to `trie` if it doesn't exist already.
 @param[value] Only if `TRIE_VALUE` is set will this parameter exist. Output
 pointer. Can be null only if `TRIE_KEY_IN_VALUE` was not defined.
 @return One of, `TRIE_ERROR`, `errno` is set and `value` is not;
 `TRIE_UNIQUE`, added to `trie`, and uninitialized `value` is associated with
 `key`; `TRIE_PRESENT`, the value associated with `key`. If `TRIE_IN_VALUE`,
 was specified and the return is `TRIE_UNIQUE`, the trie is in an invalid state
 until filling in the key in value by `key`.
 @order \O(\log |`trie`|)
 @throws[EILSEQ] The string has a distinguishing run of bytes with a
 neighbouring string that is too long. On most platforms, this is about
 32 bytes the same. @throws[malloc] @allow */
static enum trie_result T_(trie_try)(struct T_(trie) *const trie,
	const PT_(key) key, PT_(value) **const value) {
	enum trie_result res;
	PT_(entry) *e;
	assert(trie && PT_(key_string)(key));
	if(res = PT_(add)(trie, key, &e)) {
#ifndef TRIE_KEY_IN_VALUE /* <!-- key value map */
		if(value) *value = &e->value;
#else /* key value map --><!-- key in value */
		assert(value); /* One _has_ to set the key in the value. */
		*value = e;
#endif /* key in value --> */
	}
	return res;
}
#endif /* trie value map OR key in value --> */


/* Deleting entries. */

/** Try to remove `string` from `trie`. @fixme Join when combined-half is less
 than a quarter? Probably crucial to performance in some corner cases. */
static int PT_(remove)(struct T_(trie) *const trie, const char *const string) {
	struct PT_(tree) *tree;
	size_t bit; /* In bits of `key`. */
	struct { size_t cur, next; } byte; /* `key` null checks. */
	struct { unsigned br0, br1, lf; } ye, no, up;
	unsigned parent_br = 0; /* Same tree. Useless initialization. */
	struct { struct PT_(tree) *tree; unsigned lf; } prev = { 0, 0 }; /* Diff. */
	PT_(entry) *rm;
	assert(trie && string);
	/* Same as match, except keep track of more stuff. */
	if(!(tree = trie->root) || tree->bsize == USHRT_MAX) return 0; /* Empty. */
	for(bit = 0, byte.cur = 0; ; ) {
		ye.br0 = no.br0 = 0, ye.br1 = no.br1 = tree->bsize, ye.lf = no.lf = 0;
		while(ye.br0 < ye.br1) {
			const struct trie_branch *const branch
				= tree->branch + (parent_br = ye.br0);
			for(byte.next = (bit += branch->skip) / CHAR_BIT;
				byte.cur < byte.next; byte.cur++)
				if(string[byte.cur] == '\0') return 0; /* `key` too short. */
			if(!TRIE_QUERY(string, bit))
				no.lf = ye.lf + branch->left + 1,
				no.br1 = ye.br1,
				no.br0 = ye.br1 = ++ye.br0 + branch->left;
			else
				no.br0 = ++ye.br0,
				no.br1 = (ye.br0 += branch->left),
				no.lf = ye.lf, ye.lf += branch->left + 1;
			bit++;
		}
		if(!trie_bmp_test(&tree->bmp, ye.lf)) break;
		prev.tree = tree, prev.lf = ye.lf;
		tree = tree->leaf[ye.lf].as_link; /* Jumped trees. */
	}
	rm = &tree->leaf[ye.lf].as_entry;
	if(strcmp(PT_(key_string)(PT_(entry_key)(rm)), string)) return 0;
	/* If a branch, branch not taken's skip merges with the parent. */
	if(no.br0 < no.br1) {
		struct trie_branch *const parent = tree->branch + parent_br,
			*const no_child = tree->branch + no.br0;
		/* Would cause overflow. */
		if(parent->skip == UCHAR_MAX
			|| no_child->skip > UCHAR_MAX - parent->skip - 1)
			return errno = EILSEQ, 0;
		no_child->skip += parent->skip + 1;
	} else if(no.br0 == no.br1 && trie_bmp_test(&tree->bmp, no.lf)) {
		/* Branch not taken is a link leaf. */
		struct trie_branch *const parent = tree->branch + parent_br;
		struct PT_(tree) *const downstream = tree->leaf[no.lf].as_link;
		assert(downstream);
		if(downstream->bsize) {
			if(parent->skip == UCHAR_MAX
				|| downstream->branch[0].skip > UCHAR_MAX - parent->skip - 1)
				return errno = EILSEQ, 0;
			downstream->branch[0].skip += parent->skip + 1;
		} else {
			/* Don't allow links to be the single entry in a tree. */
			assert(!trie_bmp_test(&downstream->bmp, 0));
		}
	}
	/* Update `left` values for the path to the deleted branch. */
	up.br0 = 0, up.br1 = tree->bsize, up.lf = ye.lf;
	if(!up.br1) goto erased_tree;
	for( ; ; ) {
		struct trie_branch *const branch = tree->branch + up.br0;
		if(branch->left >= up.lf) {
			if(!branch->left) break;
			up.br1 = ++up.br0 + branch->left;
			branch->left--;
		} else {
			if((up.br0 += branch->left + 1) >= up.br1) break;
			up.lf -= branch->left + 1;
		}
	}
	/* Remove the actual memory. */
	memmove(tree->branch + parent_br, tree->branch
		+ parent_br + 1, sizeof *tree->branch
		* (tree->bsize - parent_br - 1));
	memmove(tree->leaf + ye.lf, tree->leaf + ye.lf + 1,
		sizeof *tree->leaf * (tree->bsize - ye.lf));
	tree->bsize--;
	/* Remove the bit. */
	trie_bmp_remove(&tree->bmp, ye.lf, 1);
	if(tree->bsize) return 1; /* We are done. */
	/* Just making sure. */
	assert(!prev.tree || trie_bmp_test(&prev.tree->bmp, prev.lf));
	if(trie_bmp_test(&tree->bmp, 0)) { /* A single link on it's own tree. */
		struct PT_(tree) *const next = tree->leaf[0].as_link;
		if(prev.tree) prev.tree->leaf[prev.lf].as_link = next;
		else assert(trie->root == tree), trie->root = next;
	} else if(prev.tree) { /* Single entry might as well go to previous tree. */
		prev.tree->leaf[prev.lf].as_entry = tree->leaf[0].as_entry;
		trie_bmp_clear(&prev.tree->bmp, prev.lf);
	} else {
		return 1; /* Just one entry; leave it be. */
	}
	free(tree);
	return 1;
erased_tree:
	assert(trie->root == tree && !tree->bsize && !trie_bmp_test(&tree->bmp, 0));
	tree->bsize = USHRT_MAX;
	return 1;
}
/** Tries to remove `string` from `trie`.
 @return Success. If either parameter is null or the `string` is not in `trie`,
 returns false without setting `errno`.
 @throws[EILSEQ] The deletion of `string` would cause an overflow with the rest
 of the strings.
 @order \O(\log |`trie`|) @allow */
static int T_(trie_remove)(struct T_(trie) *const trie,
	const char *const string)
	{ return trie && string && PT_(remove)(trie, string); }

static void PT_(unused_base_coda)(void);
static void PT_(unused_base)(void) {
	T_(trie)(); T_(trie_)(0); T_(trie_clear)(0);
	T_(trie_match)(0, 0); T_(trie_get)(0, 0);
#ifndef TRIE_VALUE
	T_(trie_try)(0, 0); T_(trie_next)(0, 0);
#else
	T_(trie_try)(0, 0, 0); T_(trie_next)(0, 0, 0);
#endif
	T_(trie_remove)(0, 0);
	T_(trie_prefix)(0, 0); /*T_(trie_size)(0);*/
	PT_(unused_base_coda)();
}
static void PT_(unused_base_coda)(void) { PT_(unused_base)(); }

/* Box override information. */
#define BOX_TYPE struct T_(trie)
#define BOX_CONTENT struct PT_(ref)
#define BOX_ PT_
#define BOX_MAJOR_NAME trie
#define BOX_MINOR_NAME TRIE_NAME

#endif /* base code --> */


#ifdef TRIE_TRAIT /* <-- trait: Will be different on different includes. */
#define BOX_TRAIT_NAME TRIE_TRAIT
#define PTT_(n) PT_(TRIE_CAT(TRIE_TRAIT, n))
#define TT_(n) T_(TRIE_CAT(TRIE_TRAIT, n))
#else /* trait --><!-- !trait */
#define PTT_(n) PT_(n)
#define TT_(n) T_(n)
#endif /* !trait --> */


#ifdef TRIE_TO_STRING /* <!-- to string trait */
#ifndef TREE_TRAIT /* <!-- natural default */
/** Uses the natural `e` -> `z` that is defined by the key string. */
static void PTT_(to_string)(const struct PT_(ref) *const r,
	char (*const z)[12]) {
	const char *string
		= PT_(key_string)(PT_(entry_key)(&r->tree->leaf[r->lf].as_entry));
	unsigned i;
	char *y = *z;
	assert(r && z);
	for(i = 0; i < 11; string++, i++) {
		*y++ = *string;
		if(*string == '\0') return;
	}
	*y = '\0';
}
#endif /* natural --> */
#define TO_STRING_LEFT '{'
#define TO_STRING_RIGHT '}'
#include "to_string.h" /** \include */
#undef TRIE_TO_STRING
#ifndef TRIE_TRAIT
#define TRIE_HAS_TO_STRING
#endif
#endif /* to string trait --> */
#undef PTT_
#undef TT_


#if defined(TRIE_TEST) && !defined(TRIE_TRAIT) /* <!-- test */
#include "../test/test_trie.h"
#endif /* test --> */


#ifdef TRIE_DEFAULT /* <!-- default trait */
#error Unfinished.
#ifdef TREE_TRAIT
#define T_D_(n, m) TREE_CAT(T_(n), TREE_CAT(TREE_TRAIT, m))
#define PT_D_(n, m) TREE_CAT(tree, B_T_(n, m))
#else
#define T_D_(n, m) TREE_CAT(T_(n), m)
#define PT_D_(n, m) TREE_CAT(tree, T_D_(n, m))
#endif
/** This is functionally identical to <fn:<B>tree_get_or>, but a with a trait
 specifying a constant default value.
 @return The value associated with `key` in `trie`, (which can be null.) If
 no such value exists, the `TREE_DEFAULT` is returned.
 @order \O(\log |`tree`|). @allow */
static PT_(value) T_D_(tree, get)(const struct T_(trie) *const trie,
	const PT_(key) key) {
	struct PT_(ref) ref;
	/* `TREE_DEFAULT` is a valid <tag:<PB>value>. */
	static const PT_(value) PT_D_(default, value) = (TREE_DEFAULT);
	return tree && tree->root.node && tree->root.height != UINT_MAX
		&& (ref = PT_(find)(&tree->root, key)).node
		? *PT_(ref_to_valuep)(ref) : PT_D_(default, value);
}
#endif /* default trait --> */


#ifdef TRIE_EXPECT_TRAIT /* <!-- more */
#undef TRIE_EXPECT_TRAIT
#else /* more --><!-- done */
#undef BOX_TYPE
#undef BOX_CONTENT
#undef BOX_
#undef BOX_MAJOR_NAME
#undef BOX_MINOR_NAME
#undef TRIE_NAME
#ifdef TRIE_VALUE
#undef TRIE_VALUE
#endif
#ifdef TRIE_KEY
#undef TRIE_KEY
#endif
#ifdef TRIE_KEY_IN_VALUE
#undef TRIE_KEY_IN_VALUE
#endif
#ifdef TRIE_HAS_TO_STRING
#undef TRIE_HAS_TO_STRING
#endif
#ifdef TRIE_TEST
#undef TRIE_TEST
#endif
#endif /* done --> */
#ifdef TRIE_TRAIT
#undef TRIE_TRAIT
#undef BOX_TRAIT_NAME
#endif
