/** @license 2020 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @abstract Header <../src/trie.h> requires <../src/bmp.h>; examples
 <../test/test_trie.c>; article <../doc/trie/trie.pdf>.

 @subtitle Prefix tree

 ![Example of trie.](../doc/trie/trie.png)

 A <tag:<t>trie> is a prefix-tree, digital-tree, or trie: an ordered set or map
 of byte null-terminated immutable key strings allowing efficient prefix
 queries. The implementation is as <Morrison, 1968 PATRICiA>: a compact
 [binary radix trie](https://en.wikipedia.org/wiki/Radix_tree) that acts as an
 index, only storing the where the key bits are different. The keys are grouped
 in fixed-size nodes in a relaxed version of a B-tree, as
 <Bayer, McCreight, 1972 Large>, where the height is no longer fixed.

 While the worse-case run-time of querying or modifying is bounded by
 \O(|`string`|), <Tong, Goebel, Lin, 2015, Smoothed> show that, in an iid
 model, a better fit is \O(\log |`trie`|), which is seen and reported here. It
 is not stable.

 ![Bit view of the trie.](../doc/trie/trie-bits.png)

 @param[TRIE_NAME]
 Required `<t>` that satisfies `C` naming conventions when mangled.

 @param[TRIE_KEY]
 Optional <typedef:<pT>key>, the default of which is `const char *`. Requires
 implementation of <typedef:<pT>string_fn> `<t>string` to convert
 <typedef:<pT>key> to a `const char *`.

 @param[TRIE_ENTRY]
 Optional <typedef:<pT>entry> that contains the key, the default of which is
 the entry is the key. Requires <typedef:<pT>key_fn> `<t>key`, that picks out
 <typedef:<pT>key> from <typedef:<pT>entry>.

 @param[TRIE_TO_STRING]
 To string trait contained in <src/to_string.h>. The unnamed trait is
 automatically supplied by the string, but others require
`<name><trait>to_string` be declared as <typedef:<pT>to_string_fn>.

 @param[TRIE_EXPECT_TRAIT, TRIE_TRAIT]
 Named traits are obtained by including `trie.h` multiple times with
 `TRIE_EXPECT_TRAIT` and then subsequently including the name in `TRIE_TRAIT`.

 @param[TRIE_DECLARE_ONLY]
 For headers in different compilation units.

 @std C89 (Specifically, ISO/IEC 9899/AMD1:1995 because it uses EILSEQ.) */

#ifndef TRIE_NAME
#	error Name TRIE_NAME undefined.
#endif
#if defined(TRIE_TRAIT) ^ defined(BOX_TYPE)
#	error TRIE_TRAIT name must come after TRIE_EXPECT_TRAIT.
#endif
#if defined(TRIE_TEST) && (!defined(TRIE_TRAIT) && !defined(TRIE_TO_STRING) \
	|| defined(TRIE_TRAIT) && !defined(TRIE_HAS_TO_STRING))
#	error Test requires to string.
#endif
#if defined TRIE_DECLARE_ONLY && (defined TRIE_BODY || defined TRIE_TRAIT)
#	error Can not be simultaneously defined.
#endif

#ifdef TRIE_TRAIT
#	define BOX_TRAIT TRIE_TRAIT /* Ifdef in <box.h>. */
#endif
#define BOX_START
#include "box.h"

#ifndef TRIE_H /* Idempotent. */
#	define TRIE_H
#	include <stdlib.h>
#	include <string.h>
#	include <errno.h>
#	include <assert.h>
#	include <limits.h>
/* <http://c-faq.com/misc/bitsets.html>, except reversed for msb-first. */
#	define TRIE_MASK(n) ((1 << CHAR_BIT - 1) >> (n) % CHAR_BIT)
#	define TRIE_SLOT(n) ((n) / CHAR_BIT)
#	define TRIE_QUERY(a, n) ((a)[TRIE_SLOT(n)] & TRIE_MASK(n))
#	define TRIE_DIFF(a, b, n) \
		(((a)[TRIE_SLOT(n)] ^ (b)[TRIE_SLOT(n)]) & TRIE_MASK(n))
/* Maximum branching factor/leaves. Prefer alignment `4n`; cache `32n`. */
#	define TRIE_ORDER 256
#	if TRIE_ORDER - 2 < 1 || TRIE_ORDER - 2 > UCHAR_MAX /* Max left. */
#		error Non-zero maximum left parameter range `[1, UCHAR_MAX]`.
#	endif
/* `⌈(2n-1)/3⌉` nodes. */
#	define TRIE_SPLIT ((2 * (TRIE_ORDER + TRIE_ORDER - 1) - 1 + 2) / 3)

#	define TRIE_RESULT X(ERROR), X(ABSENT), X(PRESENT)
#	define X(n) TRIE_##n
/** A result of modifying the table, of which `TRIE_ERROR` is false.

 ![A diagram of the result states.](../doc/trie/result.png) */
enum trie_result { TRIE_RESULT };
#	undef X
#	ifndef TRIE_DECLARE_ONLY
#		define X(n) #n
/** A static array of strings describing the <tag:trie_result>. */
static const char *const trie_result_str[] = { TRIE_RESULT };
#		undef X
#	endif
#	undef TRIE_RESULT

struct trie_branch { unsigned char left, skip; };
#	ifndef TRIE_DECLARE_ONLY
/* Construct `struct trie_bmp`. */
#		define BMP_NAME trie
#		define BMP_BITS TRIE_ORDER
#		include "bmp.h"
/** @return Whether `prefix` is the prefix of `word`.
 Used in <fn:<T>prefix>. */
static int trie_is_prefix(const char *prefix, const char *word) {
	for( ; ; prefix++, word++) {
		if(*prefix == '\0') return 1;
		if(*prefix != *word) return 0;
	}
}
#	endif
#endif

#ifndef TRIE_TRAIT /* <!-- base trie */

#	define BOX_MINOR TRIE_NAME
#	define BOX_MAJOR trie

#	ifdef TRIE_KEY
/** The default is `const char *`. If one sets `TRIE_KEY` to a different type,
 then one must also declare `<t>string` as a <typedef:<pT>string_fn>. */
typedef TRIE_KEY pT_(key);
#	else
typedef const char *pT_(key);
#	endif

#	ifdef TRIE_ENTRY
/** If `TRIE_ENTRY` is set, one must provide `<t>key` as a
 <typedef:<pT>key_fn>; otherwise a set and <typedef:<pT>entry> and
 <typedef:<pT>key> are the same. */
typedef TRIE_ENTRY pT_(entry);
/** Remit is either an extra indirection on <typedef:<PT>entry> on `TRIE_ENTRY`
 or not. */
typedef pT_(entry) *pT_(remit);
#	else
typedef pT_(key) pT_(entry);
typedef pT_(key) pT_(remit);
#	endif

#	if 0 /* <!-- documentation */
/** Transforms a <typedef:<pT>key> into a `const char *`. */
typedef const char *(*pT_(string_fn))(pT_(key));
/** Extracts <typedef:<pT>key> from <typedef:<pT>entry>. */
typedef pT_(key) (*pT_(key_fn))(const pT_(entry) *);
#	endif /* documentation --> */

union pT_(leaf) { pT_(entry) as_entry; struct pT_(tree) *as_link; };
/* In a B-tree described using <Knuth, 1998 Art 3>, this is a node of
 `TRIE_ORDER`. Node already has conflicting meaning with the individual
 entries. We use tree, such that a trie is a forest of non-empty complete
 binary trees. */
struct pT_(tree) {
	unsigned short bsize;
	struct trie_branch branch[TRIE_ORDER - 1];
	struct trie_bmp bmp;
	union pT_(leaf) leaf[TRIE_ORDER];
};
/** To initialize it to an idle state, see <fn:<T>trie>, `{0}`, or being
 `static`.

 ![States.](../doc/trie/states.png) */
struct t_(trie);
struct t_(trie) { struct pT_(tree) *root; };
typedef struct t_(trie) pT_(box);

struct pT_(ref) { struct pT_(tree) *tree; unsigned lf; };

/* A range of words. */
struct T_(cursor) {
	struct pT_(tree) *root;
	struct pT_(ref) cur, end;
};

#	ifndef TRIE_DECLARE_ONLY /* <!-- body */

#		ifndef TRIE_KEY
/** @return The string of `key` is itself, by default. We supply this function
 if `TRIE_KEY` has not been defined. @implements <typedef:<pT>string_fn> */
static const char *t_(string)(const char *const key) { return key; }
#		endif

/** @return Given `ref`, get the remit, which is either a pointer or the entry
 itself. */
static pT_(remit) pT_(ref_to_remit)(const struct pT_(ref) *const ref) {
#		ifdef TRIE_ENTRY
	return &ref->tree->leaf[ref->lf].as_entry;
#		else
	return ref->tree->leaf[ref->lf].as_entry;
#		endif
}
/** @return Given `ref`, get the string. */
static const char *pT_(ref_to_string)(const struct pT_(ref) *const ref) {
#		ifdef TRIE_ENTRY
	/* <fn:<t>string> defined by the user iff `TRIE_KEY`, but <fn:<t>key> must
	 be defined by the user. */
	return t_(string)(t_(key)(&ref->tree->leaf[ref->lf].as_entry));
#		else
	return t_(string)(ref->tree->leaf[ref->lf].as_entry);
#		endif
}
/** Fall through `ref` until hit the first entry. Must be pointing at
 something. */
static void pT_(lower_entry)(struct pT_(ref) *ref) {
	while(trie_bmp_test(&ref->tree->bmp, ref->lf))
		ref->tree = ref->tree->leaf[ref->lf].as_link, ref->lf = 0;
}
/** Fall through `ref` until hit the last entry. Must be pointing at
 something. */
static void pT_(higher_entry)(struct pT_(ref) *ref) {
	while(trie_bmp_test(&ref->tree->bmp, ref->lf))
		ref->tree = ref->tree->leaf[ref->lf].as_link,
		ref->lf = ref->tree->bsize;
}
/** This is a convince function.
 @return The leftmost entry string at `lf` of `tree`. */
static const char *pT_(sample)(struct pT_(tree) *const tree,
	const unsigned lf) {
	struct pT_(ref) ref; ref.tree = tree, ref.lf = lf;
	pT_(lower_entry)(&ref);
	return pT_(ref_to_string)(&ref);
}

/** Looks at only the index of `trie` (non-null) for potential `prefix`
 matches, and stores them in `it`. */
static struct T_(cursor) pT_(match_prefix)
	(const struct t_(trie) *const trie, const char *const prefix) {
	struct T_(cursor) cur;
	struct pT_(tree) *tree;
	size_t bit;
	struct { size_t cur, next; } byte;
	assert(trie && prefix);
	cur.root = 0;
	if(!(tree = trie->root) || tree->bsize == USHRT_MAX) return cur;
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
		cur.root = trie->root;
		cur.cur.tree = cur.end.tree = tree;
		/* Such that <fn:<PT>next> is the first and end is greater than. */
		cur.cur.lf = lf, pT_(lower_entry)(&cur.cur), cur.cur.lf--;
		cur.end.lf = lf + br1 - br0, pT_(higher_entry)(&cur.end), cur.end.lf++;
		break;
	}
	return cur;
}
/** @return The first element of a non-null `trie`. */
static struct T_(cursor) T_(begin)(const struct t_(trie) *const trie)
	{ return pT_(match_prefix)(trie, ""); }
/** @return Extracts the reference from a valid, non-null `cur`. */
static struct pT_(ref) T_(look)(const struct T_(cursor) *const cur)
	{ return cur->cur; }
/** Advancing `cur` to the next element.
 @order \O(\log |`trie`|) @allow */
static void T_(next)(struct T_(cursor) *const cur) {
	assert(cur);
	if(!cur->root || cur->root->bsize == USHRT_MAX)
		{ cur->root = 0; return; } /* Empty. */
	assert(cur->cur.tree && cur->end.tree
		&& cur->end.lf < cur->end.tree->bsize + 2 /* +1 plus [) */);
	/* Stop when getting to the end of the range. */
	if(cur->cur.tree == cur->end.tree && cur->cur.lf + 1 >= cur->end.lf)
		{ cur->root = 0; return; }
	if(cur->cur.lf + 1 <= cur->cur.tree->bsize) { /* It's in the same tree. */
		cur->cur.lf++;
	} else { /* Going to go off the end. */
		const char *const sample = pT_(sample)(cur->cur.tree, cur->cur.lf);
		const struct pT_(tree) *old = cur->cur.tree;
		struct pT_(tree) *next = cur->root;
		size_t bit = 0;
		cur->cur.tree = 0;
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
			if(lf < next->bsize) cur->cur.tree = next, cur->cur.lf = lf + 1;
			assert(trie_bmp_test(&next->bmp, lf)); /* The old. */
			next = next->leaf[lf].as_link;
		}
		/* End of iteration. Should not get here because iteration will stop
		 one before the end. */
		if(!cur->cur.tree)
			{ cur->root = 0; return; }
	}
	pT_(lower_entry)(&cur->cur);
}
/** @return A set to strings that start with `prefix` in `trie`.
 It is valid until a topological change to `trie`. Calling <fn:<T>next> will
 iterate them in order.
 @param[prefix] To fill with the entire `trie`, use the empty string.
 @order \O(\log |`trie`|) @allow */
static struct T_(cursor) T_(prefix)(struct t_(trie) *const trie,
	const char *const prefix) {
	struct T_(cursor) cur;
	assert(trie && prefix);
	cur = pT_(match_prefix)(trie, prefix);
	/* Make sure actually a prefix. */
	if(cur.root) {
		struct pT_(ref) next = cur.cur;
		next.lf++;
		assert(cur.cur.tree && cur.end.tree);
		if(next.tree == cur.end.tree && next.lf >= cur.end.lf /* Empty. */
			|| !trie_is_prefix(prefix, pT_(ref_to_string)(&next)))
			cur.root = 0;
	}
	return cur;
}

/** @return The entry at a valid, non-null `cur`. @allow */
static pT_(remit) T_(entry)(const struct T_(cursor) *const cur)
	{ return pT_(ref_to_remit)(&cur->cur); }

/** Destroys `tree`'s children and sets invalid state.
 @order \O(|`tree`|) both time and space. */
static void pT_(clear_r)(struct pT_(tree) *const tree) {
	unsigned i;
	assert(tree && tree->bsize != USHRT_MAX);
	for(i = 0; i <= tree->bsize; i++) if(trie_bmp_test(&tree->bmp, i))
		pT_(clear_r)(tree->leaf[i].as_link), free(tree->leaf[i].as_link);
}

/** @return Is a candidate match for `string` in `trie`, (which must both be
 non-null) stored in `ref`? Or `string` is definitely not in the trie. */
static int pT_(match)(const struct t_(trie) *const trie,
	const char *const string, struct pT_(ref) *const ref) {
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
static int pT_(get)(const struct t_(trie) *const trie,
	const char *const string, struct pT_(ref) *const ref) {
	assert(trie && string && ref);
	return pT_(match)(trie, string, ref)
		&& !strcmp(pT_(ref_to_string)(ref), string);
}

/** Splits `tree` into two trees. Used in <fn:<pT>add>. @throws[malloc] */
static int pT_(split)(struct pT_(tree) *const tree) {
	unsigned br0, br1, lf;
	struct pT_(tree) *kid;
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
static unsigned pT_(open)(struct pT_(tree) *const tree,
	size_t tree_bit, const char *const key, size_t diff_bit) {
	unsigned br0, br1, lf;
	struct trie_branch *branch;
	union pT_(leaf) *leaf;
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
static enum trie_result pT_(add)(struct t_(trie) *const trie, pT_(key) key,
	struct pT_(ref) *const found) {
	unsigned br0, br1;
	struct { size_t cur, next; } byte; /* `key` null checks. */
	struct pT_(ref) exemplar, ref;
	const char *const key_string = t_(string)(key), *exemplar_string;
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
					pT_(lower_entry)(&ref); /* Arbitrarily pick the first. */
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
	exemplar_string = pT_(ref_to_string)(&exemplar), assert(exemplar_string);
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
		if(!pT_(split)(ref.tree)) goto catch; /* Takes memory. */
		bit1 = tree_bit1;
		goto tree; /* Start again from the top of the first tree. */
	}
	/* Tree is not full. */
	ref.lf = pT_(open)(ref.tree, tree_bit1, key_string, diff);
assign:
#		ifndef TRIE_ENTRY
	ref.tree->leaf[ref.lf].as_entry = key;
#		else
	/* Do not have enough information; rely on user to do it. */
#		endif
	if(found) *found = ref;
	return TRIE_ABSENT;
catch:
	if(!errno) errno = ERANGE; /* `malloc` only has to set it in POSIX. */
	return TRIE_ERROR;
}

/** Try to remove `string` from `trie`. @fixme Join when combined-half is less
 than a quarter? Probably crucial to performance in some corner cases. */
static int pT_(remove)(struct t_(trie) *const trie, const char *const string) {
	struct pT_(tree) *tree;
	size_t bit; /* In bits of `key`. */
	struct { size_t cur, next; } byte; /* `key` null checks. */
	struct { unsigned br0, br1, lf; } ye, no, up;
	unsigned parent_br = 0; /* Same tree. Useless initialization. */
	struct { struct pT_(tree) *tree; unsigned lf; } prev = { 0, 0 }; /* Diff. */
	struct pT_(ref) rm;
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
	if(strcmp(pT_(ref_to_string)(&rm), string)) return 0;
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
		struct pT_(tree) *const downstream = tree->leaf[no.lf].as_link;
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
		struct pT_(tree) *const next = tree->leaf[0].as_link;
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
static struct t_(trie) t_(trie)(void)
	{ struct t_(trie) trie = { 0 }; return trie; }

#		if 0 /* Fixme? */
/** Initializes `trie` from an `array` of pointers-to-`<T>` of `array_size`.
 @return Success. @throws[realloc] @order \O(`array_size`) @allow
 @fixme Write this function, somehow. */
static int T_(from_array)(struct T_(trie) *const trie,
	pT_(type) *const*const array, const size_t array_size) {
	return assert(trie && array && array_size),
		pT_(init)(trie, array, array_size);
}
#		endif

/** Returns any initialized `trie` (can be null) to idle.
 @order \O(|`trie`|) @allow */
static void t_(trie_)(struct t_(trie) *const trie) {
	if(!trie || !trie->root) return; /* Null or idle. */
	if(trie->root->bsize != USHRT_MAX) pT_(clear_r)(trie->root); /* Contents. */
	free(trie->root); /* Empty. */
	*trie = t_(trie)();
}

/** Clears every entry in a valid `trie` (can be null), but it continues to be
 active if it is not idle. @order \O(|`trie`|) @allow */
static void T_(clear)(struct t_(trie) *const trie) {
	if(!trie || !trie->root) return; /* Null or idle. */
	if(trie->root->bsize != USHRT_MAX) pT_(clear_r)(trie->root); /* Contents. */
	trie->root->bsize = USHRT_MAX; /* Hysteresis. */
}

#		if defined(TREE_ENTRY) || !defined(TRIE_KEY) /* <!-- pointer */

/** Looks at only the index of `trie` for potential `string` (can both be null)
 matches. Does not access the string itself, thus will ignore the bits that are
 not in the index. If may not have a null, the `remit` is stuck as a pointer on
 the end and a `trie_result` is returned.
 @return A candidate match for `string` or null. @order \O(|`string`|) @allow */
static pT_(remit) T_(match)(const struct t_(trie) *const trie,
	const char *const string) {
	struct pT_(ref) ref;
	return trie && string && pT_(match)(trie, string, &ref)
		? pT_(ref_to_remit)(&ref) : 0;
}

/** If may not have a null, the `remit` is stuck as a pointer on the end and a
 `trie_result` is returned.
 @return Exact `string` match for `trie` or null, (both can be null.)
 @order \O(\log |`trie`|) iid @allow */
static pT_(remit) T_(get)(const struct t_(trie) *const trie,
	const char *const string) {
	struct pT_(ref) ref;
	return trie && string && pT_(get)(trie, string, &ref)
		? pT_(ref_to_remit)(&ref) : 0;
}

		#else /* pointer --><!-- enum? */

/** `string` match for `trie` -> `remit`. */
static enum trie_result T_(match)(const struct T_(trie) *const trie,
	const char *const string, pT_(remit) *const remit) {
	struct pT_(ref) ref;
	if(trie && string && pT_(match)(trie, string, &ref)) {
		if(remit) *remit = pT_(ref_to_remit)(&ref);
		return TRIE_PRESENT;
	}
	return TRIE_ABSENT;
}

/** `string` exact match for `trie` -> `remit`. */
static enum trie_result T_(get)(const struct T_(trie) *const trie,
	const char *const string, pT_(remit) *const remit) {
	struct pT_(ref) ref;
	if(trie && string && pT_(get)(trie, string, &ref)) {
		if(remit) *remit = pT_(ref_to_remit)(&ref);
		return TRIE_PRESENT;
	}
	return TRIE_ABSENT;
}

#		endif /* enum? --> */

#		ifndef TRIE_ENTRY /* <!-- set */
/** Adds `key` to `trie` (which must both exist) if it doesn't exist. */
static enum trie_result T_(try)(struct t_(trie) *const trie,
	const pT_(key) key) {
	assert(trie && t_(string)(key));
	return pT_(add)(trie, key, 0);
}
#		else /* set --><!-- entry */
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
static enum trie_result T_(try)(struct t_(trie) *const trie,
	const pT_(key) key, pT_(entry) **const entry) {
	enum trie_result result;
	struct pT_(ref) r;
	assert(trie && T_(string)(key));
	if(result = pT_(add)(trie, key, &r)) *entry = &r.tree->leaf[r.lf].as_entry;
	return result;
}
#		endif /* entry --> */

/** Tries to remove `string` from `trie`.
 @return Success. If either parameter is null or the `string` is not in `trie`,
 returns false without setting `errno`.
 @throws[EILSEQ] The deletion of `string` would cause an overflow with the rest
 of the strings.
 @order \O(\log |`trie`|) @allow */
static int T_(remove)(struct t_(trie) *const trie,
	const char *const string)
	{ return trie && string && pT_(remove)(trie, string); }

#		if 0 /* fixme: Haven't figured out the best way to do this. */
/** @return The number of elements in `it`. */
static size_t pT_(size_r)(const struct pT_(iterator) *const it) {
	return it->end.lf - it->cur.lf; /* Fixme. */
}
/** Counts the of the items in `it`. @order \O(|`it`|)
 @fixme Doesn't work at all. */
static size_t T_(trie_size)(const struct T_(trie_iterator) *const it)
	{ return assert(it), pT_(size_r)(&it->_); }
#		endif

static void pT_(unused_base_coda)(void);
static void pT_(unused_base)(void) {
	t_(trie)(); t_(trie_)(0); T_(clear)(0);
	T_(next)(0);
#		if defined(TREE_ENTRY) || !defined(TRIE_KEY)
	T_(match)(0, 0); T_(get)(0, 0);
#		else
	T_(match)(0, 0, 0); T_(get)(0, 0, 0);
#		endif
#		ifdef TRIE_ENTRY
	T_(try)(0, 0, 0);
#		else
	T_(try)(0, 0);
#		endif
	T_(remove)(0, 0); T_(entry)(0); T_(prefix)(0, 0);
	/*T_(trie_size)(0);*/
	pT_(unused_base_coda)();
}
static void pT_(unused_base_coda)(void) { pT_(unused_base)(); }
#	endif /* body --> */
#endif /* base code --> */

#ifndef TRIE_DECLARE_ONLY /* Produce code. */

#	ifdef TRIE_TO_STRING
#		undef TRIE_TO_STRING
/** Thunk because `pT_(ref)` should not be visible. */
static void pTR_(to_string)(const struct pT_(ref) r,
	char (*const a)[12]) {
	const char *from = pT_(ref_to_string)(&r);
	unsigned i;
	char *to = *a;
	assert(a);
	for(i = 0; i < 11; from++, i++) {
		*to++ = *from;
		if(*from == '\0') return;
	}
	*to = '\0';
}
#		define BOX_THUNK
#		include "box.h"
#		include "to_string.h" /** \include */
#		define BOX_UNTHUNK
#		include "box.h"
#		ifndef TRIE_TRAIT
#			define TRIE_HAS_TO_STRING /* Warning about tests. */
#		endif
#	endif

#	if defined(TRIE_TEST) && !defined(TRIE_TRAIT)
#		include "../test/test_trie.h"
#	endif

#endif /* Produce code. */
#ifdef TRIE_TRAIT
#	undef TRIE_TRAIT
#	undef BOX_TRAIT
#endif


#ifdef TRIE_EXPECT_TRAIT
#	undef TRIE_EXPECT_TRAIT
#else
#	undef BOX_MINOR
#	undef BOX_MAJOR
#	undef TRIE_NAME
#	ifdef TRIE_ENTRY
#		undef TRIE_ENTRY
#	endif
#	ifdef TRIE_KEY /*?*/
#		undef TRIE_KEY
#	endif
#	ifdef TRIE_HAS_TO_STRING
#		undef TRIE_HAS_TO_STRING
#	endif
#	ifdef TRIE_TEST
#		undef TRIE_TEST
#	endif
#	ifdef TRIE_DECLARE_ONLY
#		undef TRIE_DECLARE_ONLY
#	endif
#endif
#define BOX_END
#include "box.h"
