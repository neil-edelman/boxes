/** @license 2020 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @abstract Header <../src/trie.h> requires <../src/bmp.h>; examples
 <../test/test_trie.c>; article <../doc/trie/trie.pdf>.

 @subtitle Prefix tree

 ![Example of trie.](../doc/trie/trie.png)

 A <tag:<T>trie> is a prefix-tree, digital-tree, or trie: an ordered set or map
 of byte null-terminated immutable key strings allowing efficient prefix
 queries. The implementation is as <Morrison, 1968 PATRICiA>: a compact
 [binary radix trie](https://en.wikipedia.org/wiki/Radix_tree) that acts as an
 index, only storing the where the key bits are different. The keys are grouped
 in fixed-size nodes in a relaxed version of a B-tree, as
 <Bayer, McCreight, 1972 Large>, where the height is no longer fixed.

 While the worse-case run-time of querying or modifying is bounded by
 \O(|`string`|), <Tong, Goebel, Lin, 2015, Smoothed> show that, in an iid
 model, a better fit is \O(\log |`trie`|), which is reported here.

 ![Bit view of the trie.](../doc/trie/trie-bits.png)

 @param[TRIE_NAME]
 Required `<T>` that satisfies `C` naming conventions when mangled. `<PT>` is
 private, whose names are prefixed in a manner to avoid collisions.

 @param[TRIE_KEY]
 Optional <typedef:<PT>key>, the default of which is `const char *`. Requires
 implementation of <typedef:<PT>string_fn> `<T>string` to convert
 <typedef:<PT>key> to a `const char *`.

 @param[TRIE_ENTRY]
 Optional <typedef:<PT>entry> that contains the key, the default of which is
 the entry is the key. Requires <typedef:<PT>key_fn> `<T>key`, that picks out
 <typedef:<PT>key> from <typedef:<PT>entry>.

 @param[TRIE_TO_STRING]
 To string trait `<STR>` contained in <src/to_string.h>. The unnamed trait is
 automatically supplied by the string, but others require
`<name><trait>to_string` be declared as <typedef:<PSTR>to_string_fn>.

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
/* Maximum branching factor/leaves. Prefer alignment `4n`; cache `32n`. */
#define TRIE_ORDER 256
#if TRIE_ORDER - 2 < 1 || TRIE_ORDER - 2 > UCHAR_MAX /* Max left. */
#error Non-zero maximum left parameter range `[1, UCHAR_MAX]`.
#endif
/* `⌈(2n-1)/3⌉` nodes. */
#define TRIE_SPLIT ((2 * (TRIE_ORDER + TRIE_ORDER - 1) - 1 + 2) / 3)
#define TRIE_RESULT X(ERROR), X(ABSENT), X(PRESENT)
#define X(n) TRIE_##n
/** A result of modifying the table, of which `TRIE_ERROR` is false.

 ![A diagram of the result states.](../doc/trie/result.png) */
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

#ifdef TRIE_KEY /* <!-- indirect */
/** The default is `const char *`. If one sets `TRIE_KEY` to a different type,
 then one must also declare `<P>string` as a <typedef:<PT>string_fn>. */
typedef TRIE_KEY PT_(key);
#else /* indirect --><!-- !indirect */
typedef const char *PT_(key);
/** @return The string of `key` is itself, by default. We supply this function
 if `TRIE_KEY` has not been defined. @implements <typedef:<PT>string_fn> */
static const char *T_(string)(const char *const key) { return key; }
#endif /* !indirect --> */

#ifdef TRIE_ENTRY /* <!-- entry */
/** If `TRIE_ENTRY` is set, one must provide `<T>key` as a
 <typedef:<PT>key_fn>; otherwise a set and <typedef:<PT>entry> and
 <typedef:<PT>key> are the same. */
typedef TRIE_ENTRY PT_(entry);
/** Remit is either an extra indirection on <typedef:<PT>entry> on `TRIE_ENTRY`
 or not. */
typedef PT_(entry) *PT_(remit);
#else /* entry --><!-- set */
typedef PT_(key) PT_(entry);
typedef PT_(key) PT_(remit);
#endif /* set --> */

#if 0 /* <!-- documentation */
/** Transforms a <typedef:<PT>key> into a `const char *`. */
typedef const char *(*PT_(string_fn))(PT_(key));
/** Extracts <typedef:<PT>key> from <typedef:<PT>entry>. */
typedef PT_(key) (*PT_(key_fn))(const PT_(entry) *);
#endif /* documentation --> */

union PT_(leaf) { PT_(entry) as_entry; struct PT_(tree) *as_link; };
/* In a B-tree described using <Knuth, 1998 Art 3>, this is a node of
 `TRIE_ORDER`. Node already has conflicting meaning with the individual
 entries. We use tree, such that a trie is a forest of non-empty complete
 binary trees. */
struct PT_(tree) {
	unsigned short bsize;
	struct trie_branch branch[TRIE_ORDER - 1];
	struct trie_bmp bmp;
	union PT_(leaf) leaf[TRIE_ORDER];
};
/** To initialize it to an idle state, see <fn:<T>trie>, `{0}`, or being
 `static`.

 ![States.](../doc/trie/states.png) */
struct T_(trie);
struct T_(trie) { struct PT_(tree) *root; };

struct PT_(ref) { struct PT_(tree) *tree; unsigned lf; };

/** @return Given `ref`, get the remit, which is either a pointer or the entry
 itself. */
static PT_(remit) PT_(ref_to_remit)(const struct PT_(ref) *const ref) {
#ifdef TRIE_ENTRY
	return &ref->tree->leaf[ref->lf].as_entry;
#else
	return ref->tree->leaf[ref->lf].as_entry;
#endif
}
/** @return Given `ref`, get the string. */
static const char *PT_(ref_to_string)(const struct PT_(ref) *const ref) {
#ifdef TRIE_ENTRY
	/* <fn:<T>string> defined by the user iff `TRIE_KEY`, but <fn:<T>key> must
	 be defined by the user. */
	return T_(string)(T_(key)(&ref->tree->leaf[ref->lf].as_entry));
#else
	return T_(string)(ref->tree->leaf[ref->lf].as_entry);
#endif
}
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
	return PT_(ref_to_string)(&ref);
}

/* A range of words from `[cur + 1, end)` such that <fn:<PT>next> makes it
 `[cur, end)`. */
struct PT_(iterator) {
	struct PT_(tree) *root;
	struct PT_(ref) cur, end;
};
/** Looks at only the index of `trie` (non-null) for potential `prefix`
 matches, and stores them in `it`. */
static struct PT_(iterator) PT_(match_prefix)
	(const struct T_(trie) *const trie, const char *const prefix) {
	struct PT_(iterator) it;
	struct PT_(tree) *tree;
	size_t bit;
	struct { size_t cur, next; } byte;
	assert(trie && prefix);
	it.root = 0;
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
		it.root = trie->root;
		it.cur.tree = it.end.tree = tree;
		/* Such that <fn:<PT>next> is the first and end is greater than. */
		it.cur.lf = lf, PT_(lower_entry)(&it.cur), it.cur.lf--;
		it.end.lf = lf + br1 - br0, PT_(higher_entry)(&it.end), it.end.lf++;
		break;
	}
	return it;
}
/** Loads the first element of a non-null `trie` into `it`. */
static struct PT_(iterator) PT_(iterator)(const struct T_(trie) *const trie)
	{ return PT_(match_prefix)(trie, ""); }
static struct PT_(ref) PT_(element)(const struct PT_(iterator) *const it)
	{ return it->cur; }
/** @return If `it` was advanced to the successor? @implements next */
static int PT_(next)(struct PT_(iterator) *const it) {
	assert(it);
	if(!it->root || it->root->bsize == USHRT_MAX) return 0; /* Empty. */
	assert(it->cur.tree && it->end.tree
		&& it->end.lf < it->end.tree->bsize + 2 /* +1 plus [) */);
	/* Stop when getting to the end of the range. */
	if(it->cur.tree == it->end.tree && it->cur.lf + 1 >= it->end.lf) return 0;
	if(it->cur.lf + 1 <= it->cur.tree->bsize) { /* It's in the same tree. */
		it->cur.lf++;
	} else { /* Going to go off the end. */
		const char *const sample = PT_(sample)(it->cur.tree, it->cur.lf);
		const struct PT_(tree) *old = it->cur.tree;
		struct PT_(tree) *next = it->root;
		size_t bit = 0;
		it->cur.tree = 0;
		while(next != old) {
			unsigned br0 = 0, br1 = next->bsize, lf = 0;
			while(br0 < br1) {
				const struct trie_branch *const branch = next->branch + br0;
				bit += branch->skip;
				if(!TRIE_QUERY(sample, bit))
					br1 = ++br0 + branch->left;
				else
					br0 += branch->left + 1, lf += branch->left + 1;
				bit++;
			}
			if(lf < next->bsize) it->cur.tree = next, it->cur.lf = lf + 1;
			assert(trie_bmp_test(&next->bmp, lf)); /* The old. */
			next = next->leaf[lf].as_link;
		}
		/* End of iteration. Should not get here because iteration will stop
		 one before the end. */
		if(!it->cur.tree) return 0;
	}
	PT_(lower_entry)(&it->cur);
	return 1;
}
/** Stores all `prefix` matches in `trie` in `it`. */
static struct PT_(iterator) PT_(prefix)(struct T_(trie) *const trie,
	const char *const prefix) {
	struct PT_(iterator) it;
	assert(trie && prefix);
	it = PT_(match_prefix)(trie, prefix);
	/* Make sure actually a prefix. */
	if(it.root) {
		struct PT_(ref) next = it.cur;
		next.lf++;
		assert(it.cur.tree && it.end.tree);
		if(next.tree == it.end.tree && next.lf >= it.end.lf /* Empty. */
			|| !trie_is_prefix(prefix, PT_(ref_to_string)(&next)))
			it.root = 0;
	}
	return it;
}

/** Destroys `tree`'s children and sets invalid state.
 @order \O(|`tree`|) both time and space. */
static void PT_(clear_r)(struct PT_(tree) *const tree) {
	unsigned i;
	assert(tree && tree->bsize != USHRT_MAX);
	for(i = 0; i <= tree->bsize; i++) if(trie_bmp_test(&tree->bmp, i))
		PT_(clear_r)(tree->leaf[i].as_link), free(tree->leaf[i].as_link);
}

/** @return Is a candidate match for `string` in `trie`, (which must both be
 non-null) stored in `ref`? Or `string` is definitely not in the trie. */
static int PT_(match)(const struct T_(trie) *const trie,
	const char *const string, struct PT_(ref) *const ref) {
	size_t bit; /* In bits of `key`. */
	struct { size_t cur, next; } byte; /* `key` null checks. */
	assert(trie && string && ref);
	/* Empty. */
	if(!(ref->tree = trie->root) || ref->tree->bsize == USHRT_MAX) return 0;
	for(bit = 0, byte.cur = 0; ; ref->tree = ref->tree->leaf[ref->lf].as_link) {
		unsigned br0 = 0, br1 = ref->tree->bsize;
		ref->lf = 0;
		while(br0 < br1) {
			const struct trie_branch *const branch = ref->tree->branch + br0;
			for(byte.next = (bit += branch->skip) / CHAR_BIT;
				byte.cur < byte.next; byte.cur++)
				if(string[byte.cur] == '\0') return 0; /* Too short. */
			if(!TRIE_QUERY(string, bit))
				br1 = ++br0 + branch->left;
			else
				br0 += branch->left + 1, ref->lf += branch->left + 1;
			bit++;
		}
		if(!trie_bmp_test(&ref->tree->bmp, ref->lf)) break;
	}
	return 1;
}

/** @return Is `ref` storing an exact match for `string` in `trie`. */
static int PT_(get)(const struct T_(trie) *const trie,
	const char *const string, struct PT_(ref) *const ref) {
	assert(trie && string && ref);
	return PT_(match)(trie, string, ref)
		&& !strcmp(PT_(ref_to_string)(ref), string);
}

/** Splits `tree` into two trees. Used in <fn:<PT>add>. @throws[malloc] */
static int PT_(split)(struct PT_(tree) *const tree) {
	unsigned br0, br1, lf;
	struct PT_(tree) *kid;
	assert(tree && tree->bsize == TRIE_ORDER - 1);
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
 @return The index of the uninitialized leaf. */
static unsigned PT_(open)(struct PT_(tree) *const tree,
	size_t tree_bit, const char *const key, size_t diff_bit) {
	unsigned br0, br1, lf;
	struct trie_branch *branch;
	union PT_(leaf) *leaf;
	size_t bit1;
	unsigned is_right;
	assert(key && tree && tree->bsize < TRIE_ORDER - 1);
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
	return lf;
}

/** Adds `key` to `trie` and stores it in `found`, if not null. On unique,
 fills in the `key` unless map, then one is on their own. @return A result. */
static enum trie_result PT_(add)(struct T_(trie) *const trie, PT_(key) key,
	struct PT_(ref) *const found) {
	unsigned br0, br1;
	struct { size_t cur, next; } byte; /* `key` null checks. */
	struct PT_(ref) exemplar, ref;
	const char *const key_string = T_(string)(key), *exemplar_string;
	size_t bit1, diff, tree_bit1;
	assert(trie && key_string);
	if(!(ref.tree = trie->root)) { /* Idle. */
		if(!(ref.tree = malloc(sizeof *ref.tree))) goto catch;
		ref.tree->bsize = USHRT_MAX;
		trie->root = ref.tree;
	} /* Fall-through. */
	if(ref.tree->bsize == USHRT_MAX) { /* Empty: special case. */
		ref.tree->bsize = 0, ref.lf = 0;
		trie_bmp_clear_all(&ref.tree->bmp);
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
	exemplar = ref;
	exemplar_string = PT_(ref_to_string)(&exemplar), assert(exemplar_string);
	/* Do a string comparison to find difference, first bytes, then bits. */
	{
		const char *k = key_string, *e = exemplar_string;
		for(diff = 0; *k == *e; k++, e++) {
			if(*k == '\0') { if(found) *found = exemplar; return TRIE_PRESENT; }
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
	assert(ref.tree->bsize <= TRIE_ORDER - 1);
	if(ref.tree->bsize == TRIE_ORDER - 1) {
		if(!PT_(split)(ref.tree)) goto catch; /* Takes memory. */
		bit1 = tree_bit1;
		goto tree; /* Start again from the top of the first tree. */
	}
	/* Tree is not full. */
	ref.lf = PT_(open)(ref.tree, tree_bit1, key_string, diff);
assign:
#ifndef TRIE_ENTRY /* <!-- set */
	ref.tree->leaf[ref.lf].as_entry = key;
#else /* set --><!-- entry */
	/* Do not have enough information; rely on user to do it. */
#endif /* entry --> */
	if(found) *found = ref;
	return TRIE_ABSENT;
catch:
	if(!errno) errno = ERANGE; /* `malloc` only has to set it in POSIX. */
	return TRIE_ERROR;
}

/** Try to remove `string` from `trie`. @fixme Join when combined-half is less
 than a quarter? Probably crucial to performance in some corner cases. */
static int PT_(remove)(struct T_(trie) *const trie, const char *const string) {
	struct PT_(tree) *tree;
	size_t bit; /* In bits of `key`. */
	struct { size_t cur, next; } byte; /* `key` null checks. */
	struct { unsigned br0, br1, lf; } ye, no, up;
	unsigned parent_br = 0; /* Same tree. Useless initialization. */
	struct { struct PT_(tree) *tree; unsigned lf; } prev = { 0, 0 }; /* Diff. */
	struct PT_(ref) rm;
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
	rm.tree = tree, rm.lf = ye.lf;
	if(strcmp(PT_(ref_to_string)(&rm), string)) return 0;
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

#if defined(TREE_ENTRY) || !defined(TRIE_KEY) /* <!-- pointer */

/** Looks at only the index of `trie` for potential `string` (can both be null)
 matches. Does not access the string itself, thus will ignore the bits that are
 not in the index. If may not have a null, the `remit` is stuck as a pointer on
 the end and a `trie_result` is returned.
 @return A candidate match for `string` or null. @order \O(|`string`|) @allow */
static PT_(remit) T_(trie_match)(const struct T_(trie) *const trie,
	const char *const string) {
	struct PT_(ref) ref;
	return trie && string && PT_(match)(trie, string, &ref)
		? PT_(ref_to_remit)(&ref) : 0;
}

/** If may not have a null, the `remit` is stuck as a pointer on the end and a
 `trie_result` is returned.
 @return Exact `string` match for `trie` or null, (both can be null.)
 @order \O(\log |`trie`|) iid @allow */
static PT_(remit) T_(trie_get)(const struct T_(trie) *const trie,
	const char *const string) {
	struct PT_(ref) ref;
	return trie && string && PT_(get)(trie, string, &ref)
		? PT_(ref_to_remit)(&ref) : 0;
}

#else /* pointer --><!-- enum? */

/** `string` match for `trie` -> `remit`. */
static enum trie_result T_(trie_match)(const struct T_(trie) *const trie,
	const char *const string, PT_(remit) *const remit) {
	struct PT_(ref) ref;
	if(trie && string && PT_(match)(trie, string, &ref)) {
		if(remit) *remit = PT_(ref_to_remit)(&ref);
		return TRIE_PRESENT;
	}
	return TRIE_ABSENT;
}

/** `string` exact match for `trie` -> `remit`. */
static enum trie_result T_(trie_get)(const struct T_(trie) *const trie,
	const char *const string, PT_(remit) *const remit) {
	struct PT_(ref) ref;
	if(trie && string && PT_(get)(trie, string, &ref)) {
		if(remit) *remit = PT_(ref_to_remit)(&ref);
		return TRIE_PRESENT;
	}
	return TRIE_ABSENT;
}

#endif /* enum? --> */

#ifndef TRIE_ENTRY /* <!-- set */
/** Adds `key` to `trie` (which must both exist) if it doesn't exist. */
static enum trie_result T_(trie_try)(struct T_(trie) *const trie,
	const PT_(key) key) {
	assert(trie && T_(string)(key));
	return PT_(add)(trie, key, 0);
}
#else /* set --><!-- entry */
/** Adds `key` to `trie` if it doesn't exist already.
 @param[entry] Output pointer. Only if `TRIE_ENTRY` is set will this parameter
 exist.
 @return One of, `TRIE_ERROR`, `errno` is set and `entry` is not;
 `TRIE_ABSENT`, `key` is added to `trie`; `TRIE_PRESENT`, the value associated
 with `key`.

 If `TRIE_ENTRY` was specified and the return is `TRIE_ABSENT`, the trie is in
 an invalid state until filling in the key with an equivalent `key`. (Because
 <typedef:<PT>key> is not invertible in this case, it is agnostic of the method
 of setting the key.)
 @order \O(\log |`trie`|)
 @throws[EILSEQ] The string has a distinguishing run of bytes with a
 neighbouring string that is too long. On most platforms, this is about
 32 bytes the same. @throws[malloc] @allow */
static enum trie_result T_(trie_try)(struct T_(trie) *const trie,
	const PT_(key) key, PT_(entry) **const entry) {
	enum trie_result result;
	struct PT_(ref) r;
	assert(trie && T_(string)(key));
	if(result = PT_(add)(trie, key, &r)) *entry = &r.tree->leaf[r.lf].as_entry;
	return result;
}
#endif /* entry --> */

/** Tries to remove `string` from `trie`.
 @return Success. If either parameter is null or the `string` is not in `trie`,
 returns false without setting `errno`.
 @throws[EILSEQ] The deletion of `string` would cause an overflow with the rest
 of the strings.
 @order \O(\log |`trie`|) @allow */
static int T_(trie_remove)(struct T_(trie) *const trie,
	const char *const string)
	{ return trie && string && PT_(remove)(trie, string); }

/** Represents a range of in-order keys in \O(1) space. */
struct T_(trie_iterator);
struct T_(trie_iterator) { struct PT_(iterator) _; };

#if 0
/* Haven't figured out the best way to do this. */
/** @return The number of elements in `it`. */
static size_t PT_(size_r)(const struct PT_(iterator) *const it) {
	return it->end.lf - it->cur.lf; /* Fixme. */
}
/** Counts the of the items in `it`. @order \O(|`it`|) @allow
 @fixme Doesn't work at all. */
static size_t T_(trie_size)(const struct T_(trie_iterator) *const it)
	{ return assert(it), PT_(size_r)(&it->_); }
#endif

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

/** @return Whether advancing `it` to the next element is successful.
 @order \O(\log |`trie`|) @allow */
static int T_(trie_next)(struct T_(trie_iterator) *const it)
	{ return PT_(next)(&it->_); }

static void PT_(unused_base_coda)(void);
static void PT_(unused_base)(void) {
	T_(trie)(); T_(trie_)(0); T_(trie_clear)(0); T_(trie_next)(0);
#if defined(TREE_ENTRY) || !defined(TRIE_KEY)
	T_(trie_match)(0, 0); T_(trie_get)(0, 0);
#else
	T_(trie_match)(0, 0, 0); T_(trie_get)(0, 0, 0);
#endif
#ifdef TRIE_ENTRY
	T_(trie_try)(0, 0, 0);
#else
	T_(trie_try)(0, 0);
#endif
	T_(trie_remove)(0, 0); T_(trie_prefix)(0, 0); /*T_(trie_size)(0);*/
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
/** Uses the natural `r` -> `z` that is defined by the key string. */
static void PTT_(to_string)(const struct PT_(ref) r,
	char (*const z)[12]) {
	const char *from = PT_(ref_to_string)(&r);
	unsigned i;
	char *to = *z;
	assert(z);
	for(i = 0; i < 11; from++, i++) {
		*to++ = *from;
		if(*from == '\0') return;
	}
	*to = '\0';
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


#ifdef TRIE_EXPECT_TRAIT /* <!-- more */
#undef TRIE_EXPECT_TRAIT
#else /* more --><!-- done */
#undef BOX_TYPE
#undef BOX_CONTENT
#undef BOX_
#undef BOX_MAJOR_NAME
#undef BOX_MINOR_NAME
#undef TRIE_NAME
#ifdef TRIE_ENTRY
#undef TRIE_ENTRY
#endif
#ifdef TRIE_KEY
#undef TRIE_KEY
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
