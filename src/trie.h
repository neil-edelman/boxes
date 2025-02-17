/** @license 2020 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @abstract Header <../../src/trie.h> requires <../../src/bmp.h>; examples
 <../../test/test_trie.c>; article <../trie/trie.pdf>.

 @subtitle Prefix tree

 ![Example of trie.](../doc/trie/trie.png)

 A <tag:<t>trie> is a prefix-tree, digital-tree, or trie: an ordered set or map
 of byte null-terminated immutable key strings allowing efficient prefix
 queries. The implementation is as <Morrison, 1968 PATRICiA>: a compact
 [binary radix trie](https://en.wikipedia.org/wiki/Radix_tree) that acts as an
 index, only storing the where the key bits are different. The keys are grouped
 in fixed-size nodes in a relaxed version of a B-tree, as
 <Bayer, McCreight, 1972 Large>, where the height is no longer fixed.

 In general, multiple trees are equivalent by rotations. A trie is single one
 of the trees that aligns with the data. Thus, instead of using binary search,
 one uses the bits of the key directly; there is just one entry that has to be
 compared. Where the fine-grained structure of the B-tree is implicit, a trie
 needs a specific shape. This is given constant size (16 bytes) _per_ entry
 cache.

 ![Bit view of the trie.](../doc/trie/trie-bits.png)

 While the worse-case run-time of querying or modifying is bounded by
 \O(|`string`|), <Tong, Goebel, Lin, 2015, Smoothed> show that, in an iid
 model, a better fit is \O(\log |`trie`|), which is seen and reported here. It
 is not stable. It does not need `<t>less`. In practice, most applications will
 not see a difference between the tree and trie, but may find one more
 convenient.

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

 @param[TRIE_TO_STRING, TRIE_KEY_TO_STRING]
 To string trait contained in <src/to_string.h>. For `TRIE_TO_STRING`, see
 <typedef:<pT>to_string_fn>; alternately, for `TRIE_KEY_TO_STRING`, the key is
 suppled to the string directly, under the assumption that it can be truncated
 at the last code-point. (If one's data is not a subset of utf-8 and has the
 highest-bit set, it may be prematurely and unpredictably truncated if one uses
 `TRIE_KEY_TO_STRING`.)

 @param[TRIE_EXPECT_TRAIT, TRIE_TRAIT]
 Named traits are obtained by including `trie.h` multiple times with
 `TRIE_EXPECT_TRAIT` and then subsequently including the name in `TRIE_TRAIT`.

 @param[TRIE_DECLARE_ONLY, TRIE_NON_STATIC]
 For headers in different compilation units.

 @depend [box](../../src/box.h)
 @depend [bmp](../../src/bmp.h)
 @std C89 (Specifically, ISO/IEC 9899/AMD1:1995 because it uses EILSEQ.) */

#ifndef TRIE_NAME
#	error Name undefined.
#endif
#if !defined BOX_ENTRY1 && (defined TRIE_TRAIT ^ defined BOX_MAJOR)
#	error Trait name must come after expect trait.
#endif
#if defined TRIE_TO_STRING && defined TRIE_KEY_TO_STRING
#	error Exclusive.
#endif
#if defined TRIE_TEST && (!defined TRIE_TRAIT \
	&& !(defined TRIE_TO_STRING || defined TRIE_KEY_TO_STRING) \
	|| defined TRIE_TRAIT && !defined TRIE_HAS_TO_STRING)
#	error Test requires to string.
#endif
#if defined BOX_TRAIT && !defined TRIE_TRAIT
#	error Unexpected flow.
#endif

#ifdef TRIE_TRAIT
#	define BOX_TRAIT TRIE_TRAIT /* Ifdef in <box.h>. */
#endif
#ifdef TRIE_NON_STATIC
#	define BOX_NON_STATIC
#endif
#ifdef TRIE_DECLARE_ONLY
#	define BOX_DECLARE_ONLY
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
/** Remit is either an extra indirection on <typedef:<pT>entry> on `TRIE_ENTRY`
 or not in the case of a string—it already has a star, and we can't modify it
 anyway, so it would be awkward to return a pointer to a string. */
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

/* A leaf in a bough can either be an entry or a link to another bough. */
union pT_(leaf) { pT_(entry) as_entry; struct pT_(bough) *as_link; };
/* In a B-tree described using <Knuth, 1998 Art 3>, this is a node of
 `TRIE_ORDER`. Node already has conflicting meaning with the individual
 entries, so we avoid using this terminology. Instead, a trie is made up of
 boughs: non-empty full binary trees contained in contiguous memory. The only
 exception is the trunk (root) could have leaves equal zero in the case of an
 empty—but not idle—trie to provide hysteresis. That is, switching from 0 to 1
 entry repeatedly should not release and `malloc` resources every time (it is
 lazy.) As a full binary tree, the branches is `leaves - 1`, with the exception
 being empty. */
struct pT_(bough) {
	unsigned short leaves;
	struct trie_branch branch[TRIE_ORDER - 1];
	struct trie_bmp bmp;
	union pT_(leaf) leaf[TRIE_ORDER];
};
/** To initialize it to an idle state, see <fn:<t>trie>, `{0}`, or being
 `static`.

 ![States.](../doc/trie/states.png) */
struct t_(trie);
struct t_(trie) { struct pT_(bough) *trunk; };
typedef struct t_(trie) pT_(box);

struct pT_(ref) { struct pT_(bough) *bough; unsigned lf; };

/* A range of words. fixme: Have a cursor and a range; this is a range. */
struct T_(cursor) { /* fixme: This is very wasteful? at least re-arrange? */
	struct pT_(bough) *root;
	struct pT_(ref) start, end;
};

#	ifdef BOX_NON_STATIC /* Public functions. */
struct T_(cursor) T_(begin)(const struct t_(trie) *);
int T_(exists)(const struct T_(cursor) *);
pT_(remit) T_(entry)(const struct T_(cursor) *);
void T_(next)(struct T_(cursor) *);
struct T_(cursor) T_(prefix)(struct t_(trie) *, const char *);
struct t_(trie) t_(trie)(void);
void t_(trie_)(struct t_(trie) *);
void T_(clear)(struct t_(trie) *);
#		if defined(TREE_ENTRY) || !defined(TRIE_KEY) /* Pointer. */
pT_(remit) T_(match)(const struct t_(trie) *, const char *);
pT_(remit) T_(get)(const struct t_(trie) *const trie, const char *);
#		else
enum trie_result T_(match)(const struct t_(trie) *, const char *, pT_(remit) *);
enum trie_result T_(get)(const struct t_(trie) *, const char *, pT_(remit) *);
#		endif
#		ifndef TRIE_ENTRY
enum trie_result T_(add)(struct t_(trie) *, pT_(key));
#		else
enum trie_result T_(add)(struct t_(trie) *, pT_(key), pT_(entry) **);
#		endif
int T_(remove)(struct t_(trie) *, const char *);
#	endif
#	ifndef BOX_DECLARE_ONLY /* <!-- body */

#		ifndef TRIE_KEY
/** @return The string of `key` is itself, by default. We supply this function
 if `TRIE_KEY` has not been defined. @implements <typedef:<pT>string_fn> */
static const char *t_(string)(const char *const key) { return key; }
#		endif

/* fixme: This is terrible! */
/** @return Given `ref`, get the remit, which is either a pointer or the entry
 itself. */
static pT_(remit) pT_(ref_to_remit)(const struct pT_(ref) *const ref) {
#		ifdef TRIE_ENTRY
	return &ref->bough->leaf[ref->lf].as_entry;
#		else
	return ref->bough->leaf[ref->lf].as_entry;
#		endif
}
/** @return Given `ref`, get the string. */
static const char *pT_(ref_to_string)(const struct pT_(ref) *const ref) {
#		ifdef TRIE_ENTRY
	/* <fn:<t>string> defined by the user iff `TRIE_KEY`, but <fn:<t>key> must
	 be defined by the user. */
	return t_(string)(t_(key)(&ref->bough->leaf[ref->lf].as_entry));
#		else
	return t_(string)(ref->bough->leaf[ref->lf].as_entry);
#		endif
}
/** Fall through `ref` until hit the first entry. Must be pointing at
 something. */
static void pT_(lower_entry)(struct pT_(ref) *ref) {
	while(trie_bmp_test(&ref->bough->bmp, ref->lf))
		ref->bough = ref->bough->leaf[ref->lf].as_link, ref->lf = 0;
}
/** Fall through `ref` until hit the last entry. Must be pointing at
 something. */
static void pT_(higher_entry)(struct pT_(ref) *ref) {
	while(trie_bmp_test(&ref->bough->bmp, ref->lf))
		ref->bough = ref->bough->leaf[ref->lf].as_link,
		ref->lf = ref->bough->leaves - 1;
}
/** This is a convince function for <fn:<pT>match_prefix>.
 @return The leftmost entry string at `lf` of `tree`. */
static const char *pT_(sample)(const struct pT_(bough) *const tree,
	const unsigned lf) {
	union { const struct pT_(bough) *readonly; struct pT_(bough) *promise; }
		slybox;
	struct pT_(ref) ref;
	slybox.readonly = tree, ref.bough = slybox.promise, ref.lf = lf;
	pT_(lower_entry)(&ref);
	return pT_(ref_to_string)(&ref);
}

/** @return Looks at only the index of `trie` (non-null) for potential `prefix`
 matches. */
static struct T_(cursor) pT_(match_prefix)
	(const struct t_(trie) *const trie, const char *const prefix) {
	struct T_(cursor) cur;
	struct pT_(bough) *bough;
	size_t bit;
	struct { size_t cur, next; } byte;
	assert(trie && prefix);
	cur.root = 0;
	if(!(bough = trie->trunk) || !bough->leaves) return cur;
	for(bit = 0, byte.cur = 0; ; ) {
		unsigned br0 = 0, br1 = bough->leaves - 1, lf = 0;
		while(br0 < br1) {
			const struct trie_branch *const branch = bough->branch + br0;
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
		if(trie_bmp_test(&bough->bmp, lf))
			{ bough = bough->leaf[lf].as_link; continue; } /* Link. */
finally:
		assert(br0 <= br1 && lf - br0 + br1 < bough->leaves);
		cur.root = trie->trunk;
		cur.start.bough = cur.end.bough = bough;
		/* Such that <fn:<T>next> is the first and end is greater than. */
		cur.start.lf = lf, pT_(lower_entry)(&cur.start);
		cur.end.lf = lf + br1 - br0, pT_(higher_entry)(&cur.end);
		break;
	}
	return cur;
}

/** Destroys `bough`'s children and sets invalid state.
 @order \O(|`bough`|) both time and space. */
static void pT_(clear_r)(struct pT_(bough) *const bough) {
	unsigned i;
	assert(bough && bough->leaves);
	for(i = 0; i < bough->leaves; i++) if(trie_bmp_test(&bough->bmp, i))
		pT_(clear_r)(bough->leaf[i].as_link), free(bough->leaf[i].as_link);
}

/** @return Is a candidate match for `string` in `trie`, (which must both be
 non-null) stored in `ref`? Or `string` is definitely not in the trie. */
static int pT_(match)(const struct t_(trie) *const trie,
	const char *const string, struct pT_(ref) *const ref) {
	size_t bit; /* In bits of `key`. */
	struct { size_t cur, next; } byte; /* `key` null checks. */
	assert(trie && string && ref);
	/* Empty. */
	if(!(ref->bough = trie->trunk) || !ref->bough->leaves) return 0;
	for(bit = 0, byte.cur = 0; ; ref->bough = ref->bough->leaf[ref->lf].as_link) {
		unsigned br0 = 0, br1 = ref->bough->leaves - 1;
		ref->lf = 0;
		while(br0 < br1) {
			const struct trie_branch *const branch = ref->bough->branch + br0;
			for(byte.next = (bit += branch->skip) / CHAR_BIT;
				byte.cur < byte.next; byte.cur++)
				if(string[byte.cur] == '\0') return 0; /* Too short. */
			if(!TRIE_QUERY(string, bit))
				br1 = ++br0 + branch->left;
			else
				br0 += branch->left + 1, ref->lf += branch->left + 1;
			bit++;
		}
		if(!trie_bmp_test(&ref->bough->bmp, ref->lf)) break;
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

/** Splits `bough` into two. Used in <fn:<pT>add>. @throws[malloc] */
static int pT_(split)(struct pT_(bough) *const bough) {
	unsigned br0, br1, lf;
	struct pT_(bough) *kid;
	assert(bough && bough->leaves == TRIE_ORDER);
	/* Mitosis; more info added on error in <fn:<PT>add_unique>. */
	if(!(kid = malloc(sizeof *kid))) return 0;
	/* Where should we split it? <https://cs.stackexchange.com/q/144928> */
	br0 = 0, br1 = bough->leaves - 1, lf = 0;
	do {
		const struct trie_branch *const branch = bough->branch + br0;
		const unsigned right = br1 - br0 - 1 - branch->left;
		assert(br0 < br1);
		if(branch->left > right) /* Prefer right; it's less work copying. */
			br1 = ++br0 + branch->left;
		else
			br0 += branch->left + 1, lf += branch->left + 1;
	} while(2 * (br1 - br0) + 1 > TRIE_SPLIT);
	/* Copy data rooted at the current node. */
	kid->leaves = (unsigned char)(br1 - br0) + 1;
	memcpy(kid->branch, bough->branch + br0,
		sizeof *bough->branch * (kid->leaves - 1));
	memcpy(kid->leaf, bough->leaf + lf, sizeof *bough->leaf * kid->leaves);
	/* Subtract `tree` left branches; (right branches are implicit.) */
	br0 = 0, br1 = bough->leaves - 1, lf = 0;
	do {
		struct trie_branch *const branch = bough->branch + br0;
		const unsigned right = br1 - br0 - 1 - branch->left;
		assert(br0 < br1);
		if(branch->left > right)
			br1 = ++br0 + branch->left, branch->left -= kid->leaves - 1;
		else
			br0 += branch->left + 1, lf += branch->left + 1;
	} while(2 * (br1 - br0) + 1 > TRIE_SPLIT);
	/* Delete from `tree`. */
	memmove(bough->branch + br0, bough->branch + br1,
		sizeof *bough->branch * (bough->leaves - 1 - br1));
	memmove(bough->leaf + lf + 1, bough->leaf + lf + kid->leaves,
		sizeof *bough->leaf * (bough->leaves - lf - kid->leaves));
	/* Move the bits. */
	memcpy(&kid->bmp, &bough->bmp, sizeof bough->bmp);
	trie_bmp_remove(&kid->bmp, 0, lf);
	trie_bmp_remove(&kid->bmp, kid->leaves,
		bough->leaves - lf - kid->leaves);
	trie_bmp_remove(&bough->bmp, lf + 1, kid->leaves - 1);
	trie_bmp_set(&bough->bmp, lf);
	bough->leaf[lf].as_link = kid;
	bough->leaves -= kid->leaves - 1;
	return 1;
}

/** Open up an uninitialized space in a non-full `bough`. Used in <fn:<pT>add>.
 @param[bough, bough_bit] The start of the tree.
 @param[key, diff_bit] New key and where the new key differs from the tree.
 @return The index of the uninitialized leaf. */
static unsigned pT_(open)(struct pT_(bough) *const bough,
	size_t bough_bit, const char *const key, size_t diff_bit) {
	unsigned br0, br1, lf;
	struct trie_branch *branch;
	union pT_(leaf) *leaf;
	size_t bit1;
	unsigned is_right;
	assert(key && bough && bough->leaves && bough->leaves < TRIE_ORDER);
	/* Modify the tree's left branches to account for the new leaf. */
	br0 = 0, br1 = bough->leaves - 1, lf = 0;
	while(br0 < br1) {
		branch = bough->branch + br0;
		bit1 = bough_bit + branch->skip;
		/* Decision bits can never be the site of a difference. */
		if(diff_bit <= bit1) { assert(diff_bit < bit1); break; }
		if(!TRIE_QUERY(key, bit1))
			br1 = ++br0 + branch->left++;
		else
			br0 += branch->left + 1, lf += branch->left + 1;
		bough_bit = bit1 + 1;
	}
	assert(bough_bit <= diff_bit && diff_bit - bough_bit <= UCHAR_MAX);
	/* Should be the same as the first descent. */
	if(is_right = !!TRIE_QUERY(key, diff_bit)) lf += br1 - br0 + 1;
	/* Make room in leaves. */
	assert(lf <= bough->leaves);
	leaf = bough->leaf + lf;
	memmove(leaf + 1, leaf, sizeof *leaf * (bough->leaves - lf));
	trie_bmp_insert(&bough->bmp, lf, 1);
	/* Add a branch. */
	branch = bough->branch + br0;
	if(br0 != br1) { /* Split with existing branch. */
		assert(br0 < br1 && diff_bit + 1 <= bough_bit + branch->skip);
		branch->skip -= diff_bit - bough_bit + 1;
	}
	memmove(branch + 1, branch, sizeof *branch * (bough->leaves - 1 - br0));
	branch->left = is_right ? (unsigned char)(br1 - br0) : 0;
	branch->skip = (unsigned char)(diff_bit - bough_bit);
	bough->leaves++;
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
	size_t bit1, diff, bough_bit1;
	assert(trie && key_string);
	if(!(ref.bough = trie->trunk)) { /* Idle. */
		if(!(ref.bough = malloc(sizeof *ref.bough))) goto catch;
		ref.bough->leaves = 0;
		trie->trunk = ref.bough;
	} /* Fall-through. */
	if(!ref.bough->leaves) { /* Empty: special case. */
		ref.bough->leaves = 1, ref.lf = 0;
		trie_bmp_clear_all(&ref.bough->bmp);
		goto assign;
	}
	/* Otherwise we will be able to find an exemplar: a neighbouring key to the
	 new key up to the difference, (after that, it doesn't matter.) */
	for(bit1 = 0, byte.cur = 0; ;
		ref.bough = ref.bough->leaf[ref.lf].as_link) {
		br0 = 0, br1 = ref.bough->leaves - 1, ref.lf = 0; /* Leaf. */
		while(br0 < br1) {
			const struct trie_branch *const branch = ref.bough->branch + br0;
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
		if(!trie_bmp_test(&ref.bough->bmp, ref.lf)) break; /* One exemplar. */
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
	for(bit1 = 0, ref.bough = trie->trunk; ;
		ref.bough = ref.bough->leaf[ref.lf].as_link) {
		bough_bit1 = bit1;
tree:
		br0 = 0, br1 = ref.bough->leaves - 1, ref.lf = 0;
		while(br0 < br1) {
			const struct trie_branch *const branch = ref.bough->branch + br0;
			if(diff <= (bit1 += branch->skip)) goto found_diff;
			if(!TRIE_QUERY(key_string, bit1))
				br1 = ++br0 + branch->left;
			else
				br0 += branch->left + 1, ref.lf += branch->left + 1;
			bit1++;
		}
		if(!trie_bmp_test(&ref.bough->bmp, ref.lf)) goto found_diff;
	}
found_diff:
	/* Account for choosing the right leaf. */
	if(TRIE_QUERY(key_string, diff)) ref.lf += br1 - br0 + 1;
	/* Split. Agnostic of the key, also inefficient in that it moves data one
	 time for split and a subset of the data a second time for insert. Having
	 `TREE_ORDER` be more makes this matter less. */
	assert(ref.bough->leaves <= TRIE_ORDER);
	if(ref.bough->leaves == TRIE_ORDER) {
		if(!pT_(split)(ref.bough)) goto catch; /* Takes memory. */
		bit1 = bough_bit1;
		goto tree; /* Start again from the top of the first tree. */
	}
	/* Tree is not full. */
	ref.lf = pT_(open)(ref.bough, bough_bit1, key_string, diff);
assign:
#		ifndef TRIE_ENTRY
	ref.bough->leaf[ref.lf].as_entry = key;
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
	struct pT_(bough) *bough;
	size_t bit; /* In bits of `key`. */
	struct { size_t cur, next; } byte; /* `key` null checks. */
	struct { unsigned br0, br1, lf; } ye, no, up;
	unsigned parent_br = 0; /* Same tree. Useless initialization. */
	struct pT_(ref) prev = { 0, 0 } /* Diff. */, rm;
	assert(trie && string);
	/* Same as match, except keep track of more stuff. */
	if(!(bough = trie->trunk) || !bough->leaves) return 0; /* Empty. */
	for(bit = 0, byte.cur = 0; ; ) {
		ye.br0 = no.br0 = 0, ye.br1 = no.br1 = bough->leaves - 1, ye.lf = no.lf = 0;
		while(ye.br0 < ye.br1) {
			const struct trie_branch *const branch
				= bough->branch + (parent_br = ye.br0);
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
		if(!trie_bmp_test(&bough->bmp, ye.lf)) break;
		prev.bough = bough, prev.lf = ye.lf;
		bough = bough->leaf[ye.lf].as_link; /* Jumped trees. */
	}
	rm.bough = bough, rm.lf = ye.lf;
	if(strcmp(pT_(ref_to_string)(&rm), string)) return 0;
	/* If a branch, branch not taken's skip merges with the parent. */
	if(no.br0 < no.br1) {
		struct trie_branch *const parent = bough->branch + parent_br,
			*const no_child = bough->branch + no.br0;
		/* Would cause overflow. */
		if(parent->skip == UCHAR_MAX
			|| no_child->skip > UCHAR_MAX - parent->skip - 1)
			return errno = EILSEQ, 0;
		no_child->skip += parent->skip + 1;
	} else if(no.br0 == no.br1 && trie_bmp_test(&bough->bmp, no.lf)) {
		/* Branch not taken is a link leaf. */
		struct trie_branch *const parent = bough->branch + parent_br;
		struct pT_(bough) *const downstream = bough->leaf[no.lf].as_link;
		assert(downstream);
		if(downstream->leaves > 1) {
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
	up.br0 = 0, up.br1 = bough->leaves - 1, up.lf = ye.lf;
	if(!up.br1) goto erased_bough;
	for( ; ; ) {
		struct trie_branch *const branch = bough->branch + up.br0;
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
	memmove(bough->branch + parent_br, bough->branch
		+ parent_br + 1, sizeof *bough->branch
		* (bough->leaves - 1 - parent_br - 1));
	memmove(bough->leaf + ye.lf, bough->leaf + ye.lf + 1,
		sizeof *bough->leaf * (bough->leaves - 1 - ye.lf));
	bough->leaves--;
	/* Remove the bit. */
	trie_bmp_remove(&bough->bmp, ye.lf, 1);
	if(bough->leaves > 1) return 1; /* We are done. */
	/* Just making sure. */
	assert(!prev.bough || trie_bmp_test(&prev.bough->bmp, prev.lf));
	if(trie_bmp_test(&bough->bmp, 0)) { /* A single link on it's own tree. */
		struct pT_(bough) *const next = bough->leaf[0].as_link;
		if(prev.bough) prev.bough->leaf[prev.lf].as_link = next;
		else assert(trie->trunk == bough), trie->trunk = next;
	} else if(prev.bough) { /* Single entry might as well go to previous tree. */
		prev.bough->leaf[prev.lf].as_entry = bough->leaf[0].as_entry;
		trie_bmp_clear(&prev.bough->bmp, prev.lf);
	} else {
		return 1; /* Just one entry; leave it be. */
	}
	free(bough);
	return 1;
erased_bough:
	assert(trie->trunk == bough && bough->leaves == 1 && !trie_bmp_test(&bough->bmp, 0));
	bough->leaves = 0;
	return 1;
}

#		define BOX_PUBLIC_OVERRIDE
#		include "box.h"

/** @return The first element of a non-null `trie`. */
static struct T_(cursor) T_(begin)(const struct t_(trie) *const trie)
	{ return pT_(match_prefix)(trie, ""); }
/** @return Is `cur` valid. */
static int T_(exists)(const struct T_(cursor) *const cur)
	{ return cur && cur->root; }
/* @return Extracts the reference from a valid, non-null `cur`. */
/*static struct pT_(ref) T_(entry)(const struct T_(cursor) *const cur)
	{ return cur->start; }*/
/** @return The entry at a valid, non-null `cur`. @allow */
static pT_(remit) T_(entry)(const struct T_(cursor) *const cur)
	{ return pT_(ref_to_remit)(&cur->start); }
#		ifdef TRIE_ENTRY
/* fixme? */
/*static const char *T_(key)(const struct T_(cursor) *const cur)
	{ return pT_(ref_to_string)(&cur->start); }*/
#		endif
/** Advancing `cur` to the next element.
 @order \O(\log |`trie`|) @allow */
static void T_(next)(struct T_(cursor) *const cur) {
	assert(cur);
	if(!cur->root || !cur->root->leaves)
		{ cur->root = 0; return; } /* Empty. */
	assert(cur->start.bough && cur->end.bough
		&& cur->end.lf < cur->end.bough->leaves);
	/* Stop when getting to the end of the range. */
	if(cur->start.bough == cur->end.bough && cur->start.lf >= cur->end.lf)
		{ cur->root = 0; return; }
	if(cur->start.lf + 1 < cur->start.bough->leaves) {
		cur->start.lf++; /* It's in the same tree. */
	} else { /* Going to go off the end. */
		const char *const sample = pT_(sample)(cur->start.bough, cur->start.lf);
		const struct pT_(bough) *old = cur->start.bough;
		struct pT_(bough) *next = cur->root;
		size_t bit = 0;
		cur->start.bough = 0;
		while(next != old) {
			unsigned br0 = 0, br1 = next->leaves - 1, lf = 0;
			while(br0 < br1) {
				const struct trie_branch *const branch = next->branch + br0;
				bit += branch->skip;
				if(!TRIE_QUERY(sample, bit))
					br1 = ++br0 + branch->left;
				else
					br0 += branch->left + 1, lf += branch->left + 1;
				bit++;
			}
			if(lf < next->leaves - 1) cur->start.bough = next, cur->start.lf = lf + 1;
			assert(trie_bmp_test(&next->bmp, lf)); /* The old. */
			next = next->leaf[lf].as_link;
		}
		/* End of iteration. Should not get here—all ranged iterators. */
		if(!cur->start.bough)
			{ cur->root = 0; return; }
	}
	pT_(lower_entry)(&cur->start);
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
	/* Make sure actually a prefix by choosing one of the words and testing. */
	if(cur.root && !trie_is_prefix(prefix, pT_(ref_to_string)(&cur.start)))
		cur.root = 0;
	return cur;
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
	if(!trie || !trie->trunk) return; /* Null or idle. */
	if(trie->trunk->leaves) pT_(clear_r)(trie->trunk); /* Contents. */
	free(trie->trunk); /* Empty. */
	*trie = t_(trie)();
}

/** Clears every entry in a valid `trie` (can be null), but it continues to be
 active if it is not idle. @order \O(|`trie`|) @allow */
static void T_(clear)(struct t_(trie) *const trie) {
	if(!trie || !trie->trunk) return; /* Null or idle. */
	if(trie->trunk->leaves) pT_(clear_r)(trie->trunk); /* Contents. */
	trie->trunk->leaves = 0; /* Keep the resources for hysteresis. */
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
static enum trie_result T_(match)(const struct t_(trie) *const trie,
	const char *const string, pT_(remit) *const remit) {
	struct pT_(ref) ref;
	if(trie && string && pT_(match)(trie, string, &ref)) {
		if(remit) *remit = pT_(ref_to_remit)(&ref);
		return TRIE_PRESENT;
	}
	return TRIE_ABSENT;
}

/** `string` exact match for `trie` -> `remit`. */
static enum trie_result T_(get)(const struct t_(trie) *const trie,
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
static enum trie_result T_(add)(struct t_(trie) *const trie,
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
 <typedef:<pT>key> is not invertible in this case, it is agnostic of the method
 of setting the key.)
 @order \O(\log |`trie`|)
 @throws[EILSEQ] The string has a distinguishing run of bytes with a
 neighbouring string that is too long. On most platforms, this is about
 32 bytes the same. @throws[malloc] @allow */
static enum trie_result T_(add)(struct t_(trie) *const trie,
	const pT_(key) key, pT_(entry) **const put_entry_here) {
	enum trie_result result;
	struct pT_(ref) r;
	assert(trie && t_(string)(key) && put_entry_here);
	if(result = pT_(add)(trie, key, &r))
		*put_entry_here = &r.bough->leaf[r.lf].as_entry;
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

#		define BOX_PRIVATE_AGAIN
#		include "box.h"

static void pT_(unused_base_coda)(void);
static void pT_(unused_base)(void) {
	T_(begin)(0); T_(exists)(0); T_(entry)(0); /*T_(key)(0);*/ T_(next)(0);
	t_(trie)(); t_(trie_)(0); T_(clear)(0);
	T_(next)(0);
#		if defined(TREE_ENTRY) || !defined(TRIE_KEY)
	T_(match)(0, 0); T_(get)(0, 0);
#		else
	T_(match)(0, 0, 0); T_(get)(0, 0, 0);
#		endif
#		ifdef TRIE_ENTRY
	T_(add)(0, 0, 0);
#		else
	T_(add)(0, 0);
#		endif
	T_(remove)(0, 0); T_(entry)(0); T_(prefix)(0, 0);
	pT_(unused_base_coda)();
}
static void pT_(unused_base_coda)(void) { pT_(unused_base)(); }
#	endif /* body --> */
#endif /* base code --> */


#if defined TRIE_TO_STRING || defined TRIE_KEY_TO_STRING
#	ifdef TRIE_TO_STRING
#		ifndef TRIE_DECLARE_ONLY
#			ifndef TRIE_TRAIT
/** The type of the required `<tr>to_string`. Responsible for turning the
 read-only argument into a 12-max-`char` output string. `<pT>value` is omitted
 when it's a set. */
typedef void (*pT_(to_string_fn))(const pT_(entry) *, char (*)[12]);
/* fixme: Or maybe it's `<pT>remit`? this needs to be refactored a lot. */
#			endif
/** Thunk(`cur`, `a`). One must implement `<tr>to_string` or switch to
 `TRIE_KEY_TO_STRING`, which uses the key as the string automatically.
 fixme: Should have ref ∈ cursor, ∈ range separated. */
static void pTR_(to_string)(const struct T_(cursor) *const cur,
	char (*const a)[12]) {
	/* We know the record will be `as_entry` because the cursor never stops on
	 a link. */
	tr_(to_string)(pT_(ref_to_remit)(&cur->start), a);
}
#		endif
#	else
/** Thunk(`cur`, `a`), transforming a cursor to the key string.
 Where is this even used? */
static void pTR_(to_string)(const struct T_(cursor) *const cur,
	char (*const a)[12]) {
 	const char *from = pT_(ref_to_string)(&cur->start);
	unsigned i;
	char *to = *a;
	assert(cur && cur->root && a);
	for(i = 0; i < 11 - 3; from++, i++) {
		*to++ = *from;
		if(*from == '\0') return;
	}
	/* Utf-8 assumed. Split at code-point. Still could be grapheme clusters,
	 but… we take what we can get without a full-functioning text engine. */
	for( ; i < 11; from++, i++) {
		const unsigned left = 11 - i;
		const unsigned char f = (const unsigned char)*from;
		if(f < 0x80) goto encode;
		if((f & 0xe0) == 0xc0) if(left < 2) break; else goto encode;
		if((f & 0xf0) == 0xe0) if(left < 3) break; else goto encode;
		if((f & 0xf8) == 0xf0) break;
encode:
		/* Very permissive otherwise; we don't actually know anything about the
		 encoding. */
		*to++ = *from;
		if(*from == '\0') return;
	}
	*to = '\0';
}
#	endif
#	include "to_string.h" /** \include */
#	ifndef TRIE_TRAIT
#		define TRIE_HAS_TO_STRING /* Warning about tests. */
#	endif
#endif

#if defined HAS_GRAPH_H && defined TRIE_HAS_TO_STRING && !defined TRIE_TRAIT
#	include "graph.h" /** \include */
#endif

#if defined TRIE_TEST && !defined TRIE_TRAIT \
	&& defined TRIE_HAS_TO_STRING && defined HAS_GRAPH_H
#	include "../test/test_trie.h"
#endif

#ifdef TRIE_TO_STRING /* Need these in test. */
#	undef TRIE_TO_STRING
#endif
#ifdef TRIE_KEY_TO_STRING
#	undef TRIE_KEY_TO_STRING
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
#	ifdef TRIE_KEY
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
#ifdef TRIE_TRAIT
#	undef TRIE_TRAIT
#	undef BOX_TRAIT
#endif
#define BOX_END
#include "box.h"
