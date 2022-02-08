/** @license 2020 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @abstract Source <src/trie.h>; examples <test/test_trie.c>.

 @subtitle Prefix tree

 ![Example of trie.](../web/trie.png)

 A <tag:<T>trie> is a prefix-tree, digital-tree, or trie. Specifically,
 <Morrison, 1968 PATRICiA>: a compact
 [binary radix trie](https://en.wikipedia.org/wiki/Radix_tree), only
 storing the where the key bits are different. To increase cache-coherence
 while allowing for insertion and deletion in \O(\log `size`), it uses some
 B-tree techniques described in <Bayer, McCreight, 1972 Large>. Practically,
 this is an ordered set or map of immutable key strings, which can be any
 encoding with a byte null-terminator, including
 [modified UTF-8](https://en.wikipedia.org/wiki/UTF-8#Modified_UTF-8).

 @param[TRIE_NAME, TRIE_VALUE]
 <typedef:<PT>type> that satisfies `C` naming conventions when mangled and an
 optional returnable type that is declared, (it is used by reference only
 except if `TRIE_TEST`.) `<PT>` is private, whose names are prefixed in a
 manner to avoid collisions.

 @param[TRIE_KEY]
 A function that satisfies <typedef:<PT>key_fn>. Must be defined if and only if
 `TRIE_VALUE` is defined. (This imbues it with the properties of a string
 associative array.)

 @param[TRIE_TO_STRING]
 Defining this includes <to_string.h>, with the keys as the string.

 @param[TRIE_TEST]
 Unit testing framework <fn:<T>trie_test>, included in a separate header,
 <../test/test_trie.h>. Must be defined equal to a (random) filler function,
 satisfying <typedef:<PT>action_fn>. Requires `TRIE_TO_STRING` and that
 `NDEBUG` not be defined.

 @depend [bmp](https://github.com/neil-edelman/bmp)
 @std C89 */

#ifndef TRIE_NAME
#error Name TRIE_NAME undefined.
#endif
#if defined(TRIE_VALUE) ^ defined(TRIE_KEY)
#error TRIE_VALUE and TRIE_KEY have to be mutually defined.
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
#if defined(TRIE_CAT_) || defined(TRIE_CAT) || defined(T_) || defined(PT_) \
    || defined(TRIE_IDLE)
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
 Prefer alignment `4n - 2`; cache `32n - 2`. (Easily, `{a, b, ..., A}`). */
#define TRIE_MAX_LEFT /*1*//*6*/254
#if TRIE_MAX_LEFT < 1 || TRIE_MAX_LEFT > UCHAR_MAX
#error TRIE_MAX_LEFT parameter range `[1, UCHAR_MAX]` without modifications.
#endif
#define TRIE_BRANCHES (TRIE_MAX_LEFT + 1) /* Maximum branches. */
#define TRIE_ORDER (TRIE_BRANCHES + 1) /* Maximum branching factor/leaves. */
struct trie_branch { unsigned char left, skip; };
/* Dependency on `bmp.h`. */
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

#ifndef TRIE_VALUE /* <!-- !type */
#define TRIE_SET /* Testing purposes; `const char *` is not really testable. */
/** Default `char` uses `a` as the key, which makes it a set of strings. */
static const char *PT_(raw)(const char *a) { return assert(a), a; }
#define TRIE_VALUE const char
#define TRIE_KEY &PT_(raw)
#endif /* !type --> */
/** Declared type of the trie; `char` default. */
typedef TRIE_VALUE PT_(type);

/** A leaf is either data or another tree; the `children` of <tag:<PT>tree> is
 a bitmap that tells which. */
union PT_(leaf) { PT_(type) *data; struct PT_(tree) *child; };

/** A trie is a forest of non-empty complete binary trees. In
 <Knuth, 1998 Art 3> terminology, this structure is similar to a B-tree node of
 `TRIE_ORDER`, but 'node' already has conflicting meaning. */
struct PT_(tree) {
	unsigned char bsize, skip;
	struct trie_branch branch[TRIE_BRANCHES];
	struct trie_bmp is_child;
	union PT_(leaf) leaf[TRIE_ORDER];
};

/** To initialize it to an idle state, see <fn:<T>trie>, `TRIE_IDLE`, `{0}`
 (`C99`), or being `static`.

 ![States.](../web/states.png) */
struct T_(trie) { struct PT_(tree) *root; };
#ifndef TRIE_IDLE /* <!-- !zero */
#define TRIE_IDLE { 0 }
#endif /* !zero --> */

/* Contains all iteration parameters; satisfies box interface iteration. This
 is a private version of the <tag:<T>trie_iterator> that does all the work, but
 it can only iterate through the entire trie. */
struct PT_(iterator)
	{ struct PT_(tree) *root, *next; unsigned leaf, unused; };

/** Stores a range in the trie. Any changes in the topology of the trie
 invalidate it. @fixme Replacing `root` with `bit` would make it faster and
 allow size remaining; just have to fiddle with `end` to `above`. That makes it
 incompatible with private, but could merge. */
struct T_(trie_iterator);
struct T_(trie_iterator)
	{ struct PT_(tree) *root, *next, *end; unsigned leaf, leaf_end; };

/** Responsible for picking out the null-terminated string. Modifying the
 string key in the original <typedef:<PT>type> while in any trie causes the
 entire trie to go into an undefined state. */
typedef const char *(*PT_(key_fn))(const PT_(type) *);

/* Check that `TRIE_KEY` is a function satisfying <typedef:<PT>key_fn>. */
static PT_(key_fn) PT_(to_key) = (TRIE_KEY);

/** @return The address of a index candidate match for `key` in `trie`, or
 null, if `key` is definitely not in `trie`. @order \O(|`key`|) */
static PT_(type) **PT_(leaf_match)(const struct T_(trie) *const trie,
	const char *const key) {
	struct PT_(tree) *tree;
	size_t bit; /* `bit \in key`.  */
	struct { unsigned br0, br1, lf; } t;
	struct { size_t cur, next; } byte; /* `key` null checks. */
	assert(trie && key);
	if(!(tree = trie->root)) return 0; /* Empty. */
	for(byte.cur = 0, bit = 0; ; ) { /* Forest. */
		t.br0 = 0, t.br1 = tree->bsize, t.lf = 0;
		while(t.br0 < t.br1) { /* Tree. */
			const struct trie_branch *const branch = tree->branch + t.br0;
			for(byte.next = (bit += branch->skip) / CHAR_BIT;
				byte.cur < byte.next; byte.cur++)
				if(key[byte.cur] == '\0') return 0; /* Too short. */
			if(!TRIE_QUERY(key, bit))
				t.br1 = ++t.br0 + branch->left;
			else
				t.br0 += branch->left + 1, t.lf += branch->left + 1;
			bit++;
		}
		if(!trie_bmp_test(&tree->is_child, t.lf)) break;
		tree = tree->leaf[t.lf].child;
	}
	return &tree->leaf[t.lf].data;
}

/** @return An index candidate match for `key` in `trie`. */
static PT_(type) *PT_(match)(const struct T_(trie) *const trie,
	const char *const key)
	{ PT_(type) **const x = PT_(leaf_match)(trie, key); return x ? *x : 0; }

/** @return The address of the exact match for `key` in `trie` or null. */
static PT_(type) **PT_(leaf_get)(const struct T_(trie) *const trie,
	const char *const key) {
	PT_(type) **const x = PT_(leaf_match)(trie, key);
	return x && !strcmp(PT_(to_key)(*x), key) ? x : 0;
}

/** @return Exact match for `key` in `trie` or null. */
static PT_(type) *PT_(get)(const struct T_(trie) *const trie,
	const char *const key) {
	PT_(type) *const x = PT_(match)(trie, key);
	return x && !strcmp(PT_(to_key)(x), key) ? x : 0;
}

/** Looks at only the index of `trie` for potential `prefix` matches,
 and stores them in `it`. @order \O(|`prefix`|) */
static void PT_(match_prefix)(const struct T_(trie) *const trie,
	const char *const prefix, struct T_(trie_iterator) *it) {
	struct PT_(tree) *tree;
	size_t bit; /* `bit \in key`.  */
	struct { unsigned br0, br1, lf; } t;
	struct { size_t cur, next; } byte; /* `key` null checks. */
	assert(prefix && it);
	it->root = it->next = it->end = 0;
	it->leaf = it->leaf_end = 0;
	if(!trie || !(tree = trie->root)) return;
	for(byte.cur = 0, bit = 0; ; ) { /* Forest. */
		t.br0 = 0, t.br1 = tree->bsize, t.lf = 0;
		while(t.br0 < t.br1) { /* Tree. */
			const struct trie_branch *const branch = tree->branch + t.br0;
			/* _Sic_; '\0' is _not_ included for partial match. */
			for(byte.next = (bit += branch->skip) / CHAR_BIT;
				byte.cur <= byte.next; byte.cur++)
				if(prefix[byte.cur] == '\0') goto finally;
			if(!TRIE_QUERY(prefix, bit))
				t.br1 = ++t.br0 + branch->left;
			else
				t.br0 += branch->left + 1, t.lf += branch->left + 1;
			bit++;
		}
		if(!trie_bmp_test(&tree->is_child, t.lf)) break;
		tree = tree->leaf[t.lf].child;
	};
finally:
	assert(t.br0 <= t.br1
		&& t.lf - t.br0 + t.br1 <= tree->bsize);
	it->root = trie->root;
	it->next = it->end = tree;
	it->leaf = t.lf;
	it->leaf_end = t.lf + t.br1 - t.br0 + 1;
}

/** @return The leftmost key `lf` of `tree`. */
static const char *PT_(sample)(const struct PT_(tree) *tree,
	unsigned lf) {
	assert(tree);
	while(trie_bmp_test(&tree->is_child, lf))
		tree = tree->leaf[lf].child, lf = 0;
	return PT_(to_key)(tree->leaf[lf].data);
}

/** Stores all `prefix` matches in `trie` and stores them in `it`.
 @param[it] Output remains valid until the topology of the trie changes.
 @order \O(|`prefix`|) */
static void PT_(prefix)(const struct T_(trie) *const trie,
	const char *const prefix, struct T_(trie_iterator) *it) {
	assert(trie && prefix && it);
	PT_(match_prefix)(trie, prefix, it);
	if(it->leaf_end <= it->leaf) return;
	assert(it->root && it->next && it->next == it->end
		&& it->leaf_end <= it->end->bsize + 1); /* fixme: what? */
	/* Makes sure the trie matches the string. */
	if(!trie_is_prefix(prefix, PT_(sample)(it->end, it->leaf_end - 1)))
		it->leaf_end = it->leaf;
}

/** @return Allocate a new tree with one undefined leaf. @throws[malloc] */
static struct PT_(tree) *PT_(tree)(void) {
	struct PT_(tree) *tree;
	if(!(tree = malloc(sizeof *tree))) { if(!errno) errno = ERANGE; return 0; }
	tree->bsize = 0, tree->skip = 0, trie_bmp_clear_all(&tree->is_child);
	/* fixme: doesn't need clear? */
	return tree;
}

#if 0 /* <!-- forward declare debugging tools */

#ifdef TRIE_TO_STRING
static const char *T_(trie_to_string)(const struct T_(trie) *);
#endif

/** Returns a string of `trie`. */
static const char *PT_(str)(const struct T_(trie) *const trie) {
#ifdef TRIE_TO_STRING
	return T_(trie_to_string)(trie);
#else
	return "[not to string]"
#endif
}

#ifdef TRIE_TEST
static void PT_(graph)(const struct T_(trie) *, const char *);
static void PT_(print)(const struct PT_(tree) *);
#endif

/** Graphs `trie` in `fn`. */
static void PT_(grph)(const struct T_(trie) *const trie, const char *const fn) {
	assert(trie && fn);
#ifdef TRIE_TEST
	PT_(graph)(trie, fn);
#endif
}

/** Prints `tree`. */
static void PT_(prnt)(const struct PT_(tree) *const tree) {
	assert(tree);
#ifdef TRIE_TEST
	PT_(print)(tree);
#endif
}

#endif /* forward --> */

#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)

/** Adds `x` to `trie`, which must not be present. @return Success.
 @throw[malloc, ERANGE]
 @throw[ERANGE] There is too many bytes similar for the data-type. */
static int PT_(add_unique)(struct T_(trie) *const trie,
	PT_(type) *const x) {
	const char *const key = PT_(to_key)(x);
	struct { unsigned br0, br1, lf; } t;
	struct { struct PT_(tree) *tr; struct { size_t tr, diff; } bit; } i;
	struct { struct { struct PT_(tree) *tr; size_t bit; } a; size_t n; } full;
	const char *sample; /* Only used in Find. */
	int restarts = 0; /* Debug: make sure we only go through twice. */
	assert(trie && x && key);

start:
	/* <!-- Solitary. ********************************************************/
	if(!(i.tr = trie->root)) return (i.tr = PT_(tree)())
		&& (i.tr->leaf[0].data = x, trie->root = i.tr, 1);
	/* Solitary. --> */

	/* <!-- Find the first bit not in the tree. ******************************/
	/* Backtracking information; anchor is the first not-full tree. */
	full.a.tr = 0, full.a.bit = 0, full.n = 0;
	assert(i.tr);
	for(i.bit.diff = 0; ; i.tr = i.tr->leaf[t.lf].child) { /* Forest. */
		const int is_full = TRIE_BRANCHES <= i.tr->bsize;
		full.n = is_full ? full.n + 1 : 0;
		i.bit.tr = i.bit.diff;
		sample = PT_(sample)(i.tr, 0);
		t.br0 = 0, t.br1 = i.tr->bsize, t.lf = 0;
		while(t.br0 < t.br1) { /* Tree. */
			const struct trie_branch *const branch = i.tr->branch + t.br0;
			const size_t bit1 = i.bit.diff + branch->skip;
			for( ; i.bit.diff < bit1; i.bit.diff++)
				if(TRIE_DIFF(key, sample, i.bit.diff)) goto found;
			if(!TRIE_QUERY(key, i.bit.diff)) {
				t.br1 = ++t.br0 + branch->left;
			} else {
				t.br0 += branch->left + 1, t.lf += branch->left + 1;
				sample = PT_(sample)(i.tr, t.lf);
			}
			i.bit.diff++;
		} /* Tree. */
		assert(t.br0 == t.br1 && t.lf <= i.tr->bsize);
		if(!trie_bmp_test(&i.tr->is_child, t.lf)) break;
		if(!is_full) full.a.tr = i.tr, full.a.bit = i.bit.tr;
	} /* Forest. */
	{ /* Got to a leaf. */
		const size_t limit = i.bit.diff + UCHAR_MAX;
		while(!TRIE_DIFF(key, sample, i.bit.diff))
			if(++i.bit.diff > limit) return errno = EILSEQ, 0;
	}
found:
	/* Account for choosing the right leaf, (not strictly necessary here?) */
	if(!!TRIE_QUERY(key, i.bit.diff)) t.lf += t.br1 - t.br0 + 1;
	/* Find. --> */

	/* <!-- Backtrack and split. *********************************************/
	if(!full.n) goto insert;
	do { /* Split a tree. */
		struct PT_(tree) *up, *left = 0, *right = 0;
		unsigned char leaves_split;
		struct trie_branch *branch;
		union PT_(leaf) *leaf;
		size_t with_promote_bit;
		/* Allocate one or two if the root-tree is being split. This is a
		 sequence point in splitting where the trie is valid. */
		if(!(up = full.a.tr) && !(up = PT_(tree)()) || !(right = PT_(tree)()))
			{ free(right); if(!full.a.tr) free(up);
			if(!errno) errno = ERANGE; return 0; }
		if(full.a.tr) { /* Expand the parent to hold the promoted root. */
			assert(up == full.a.tr && up->bsize < TRIE_BRANCHES);
			t.br0 = 0, t.br1 = up->bsize, t.lf = 0;
			while(t.br0 < t.br1) { /* Tree. */
				branch = up->branch + t.br0;
				full.a.bit += branch->skip, assert(full.a.bit < i.bit.diff);
				if(!TRIE_QUERY(key, full.a.bit))
					t.br1 = ++t.br0 + branch->left++;
				else
					t.br0 += branch->left + 1, t.lf += branch->left + 1;
				full.a.bit++;
			}
			/* Expand the tree to include one more leaf and branch. */
			left = (leaf = up->leaf + t.lf)->child,
				assert(t.lf <= up->bsize + 1
				&& trie_bmp_test(&up->is_child, t.lf));
			memmove(leaf + 1, leaf, sizeof *leaf * ((up->bsize + 1) - t.lf));
			branch = up->branch + t.br0;
			trie_bmp_insert(&up->is_child, t.lf, 1);
			memmove(branch + 1, branch, sizeof *branch * (up->bsize - t.br0));
			up->bsize++; /* Might be full, now. */
		} else { /* Raise depth of forest for the promoted branch. */
			assert(!full.a.bit);
			left = trie->root;
			trie->root = up;
			t.br0 = 0, t.br1 = up->bsize = 1, t.lf = 0;
			trie_bmp_set(&up->is_child, 1);
		}
		/* Promote the root of left to the parent's unfilled. */
		assert(left && left->bsize);
		branch = up->branch + t.br0;
		branch->left = 0;
		branch->skip = left->branch[0].skip;
		leaf = up->leaf + t.lf;
		leaf->child = left, trie_bmp_set(&up->is_child, t.lf);
		(leaf + 1)->child = right,
			assert(trie_bmp_test(&up->is_child, t.lf + 1));
		/* Advance the cursor to the next tree. */
		leaves_split = left->branch[0].left + 1;
		if((with_promote_bit = full.a.bit + branch->skip) <= i.bit.diff) {
			assert(with_promote_bit < i.bit.diff);
			full.a.bit = with_promote_bit;
			full.a.tr = !(TRIE_QUERY(key, full.a.bit)) ? left : right;
			full.a.bit++;
		} else {
			assert(full.n == 1);
			full.a.tr = up;
		}
		/* Copy the right part of the left to the new right. */
		right->bsize = left->bsize - leaves_split;
		memcpy(right->branch, left->branch + leaves_split,
			sizeof *left->branch * right->bsize);
		memcpy(right->leaf, left->leaf + leaves_split,
			sizeof *left->leaf * (right->bsize + 1));
		memcpy(&right->is_child, &left->is_child, sizeof left->is_child);
		trie_bmp_remove(&right->is_child, 0, leaves_split);
		/* Move back the branches of the left to account for the promotion. */
		left->bsize = leaves_split - 1;
		memmove(left->branch, left->branch + 1,
			sizeof *left->branch * (left->bsize + 1));
	} while(--full.n);
	i.tr = full.a.tr, i.bit.tr = full.a.bit;
	/* It was in the promoted bit's skip and "Might be full now," was true.
	 Don't have enough information to recover, but ca'n't get here twice. */
	if(TRIE_BRANCHES <= i.tr->bsize) { assert(!restarts++); goto start; }
	/* Split. --> */

insert: /* Insert into unfilled tree. ****************************************/
	{
		union PT_(leaf) *leaf;
		struct trie_branch *branch;
		size_t bit0, bit1;
		unsigned is_right;
		assert(key && i.tr && i.tr->bsize < TRIE_BRANCHES
			&& i.bit.tr <= i.bit.diff);
		/* Modify the tree's left branches to account for the new leaf. */
		t.br0 = 0, t.br1 = i.tr->bsize, t.lf = 0;
		bit0 = i.bit.tr;
		while(t.br0 < t.br1) { /* Tree. */
			branch = i.tr->branch + t.br0;
			bit1 = bit0 + branch->skip;
			/* Decision bits can never be the site of a difference. */
			if(i.bit.diff <= bit1) { assert(i.bit.diff < bit1); break; }
			if(!TRIE_QUERY(key, bit1))
				t.br1 = ++t.br0 + branch->left++;
			else
				t.br0 += branch->left + 1, t.lf += branch->left + 1;
			bit0 = bit1 + 1;
		}
		assert(bit0 <= i.bit.diff && i.bit.diff - bit0 <= UCHAR_MAX);
		/* Should be the same as the first descent. */
		if(is_right = !!TRIE_QUERY(key, i.bit.diff)) t.lf += t.br1 - t.br0 + 1;

		/* Expand the tree to include one more leaf and branch. */
		leaf = i.tr->leaf + t.lf, assert(t.lf <= i.tr->bsize + 1);
		memmove(leaf + 1, leaf, sizeof *leaf * ((i.tr->bsize + 1) - t.lf));
		branch = i.tr->branch + t.br0;
		if(t.br0 != t.br1) { /* Split with existing branch. */
			assert(t.br0 < t.br1 && i.bit.diff + 1 <= bit0 + branch->skip);
			branch->skip -= i.bit.diff - bit0 + 1;
		}
		trie_bmp_insert(&i.tr->is_child, t.lf, 1);
		memmove(branch + 1, branch, sizeof *branch * (i.tr->bsize - t.br0));
		branch->left = is_right ? (unsigned char)(t.br1 - t.br0) : 0;
		branch->skip = (unsigned char)(i.bit.diff - bit0);
		i.tr->bsize++;
		leaf->data = x;
	}
	/* PT_(grph)(trie, "graph/" QUOTE(TRIE_NAME) "-add.gv"); */
	return 1;
}

/** A bi-predicate; returns true if the `replace` replaces the `original`; used
 in <fn:<T>trie_policy_put>. */
typedef int (*PT_(replace_fn))(PT_(type) *original, PT_(type) *replace);

/** Adds `x` to `trie` and, if `eject` is non-null, stores the collided
 element, if any, as long as `replace` is null or returns true.
 @param[eject] If not-null, the ejected datum. If `replace` returns false, then
 `*eject == datum`, but it will still return true.
 @return Success. @throws[realloc, ERANGE] */
static int PT_(put)(struct T_(trie) *const trie, PT_(type) *const x,
	PT_(type) **const eject, const PT_(replace_fn) replace) {
	const char *key;
	PT_(type) **leaf;
	assert(trie && x);
	key = PT_(to_key)(x);
	/* Add if absent. */
	if(!(leaf = PT_(leaf_get)(trie, key)))
		{ if(eject) *eject = 0; return PT_(add_unique)(trie, x); }
	/* Collision policy. */
	if(replace && !replace(*leaf, x)) {
		if(eject) *eject = x;
	} else {
		if(eject) *eject = *leaf;
		*leaf = x;
	}
	return 1;
}

/** Try to remove `key` from `trie`. */
static PT_(type) *PT_(remove)(struct T_(trie) *const trie,
	const char *const key) {
	/* @fixme Join when combined-half <= ~TRIE_BRANCH / 2 */
	struct {
		struct PT_(tree) *tr;
		unsigned parent_br, unused;
		struct { unsigned br0, br1, lf; } me, twin;
		size_t empty_followers;
	} full;
	struct PT_(tree) *tree;
	struct trie_branch *twin;
	unsigned lf;
	size_t bit;
	struct { size_t cur, next; } byte;
	PT_(type) *rm;
	assert(trie && key);

	/* Empty. */
	if(!(tree = trie->root)) return 0;

	/* Preliminary exploration. */
	full.tr = 0, full.empty_followers = 0;
	for(bit = 0; ; tree = tree->leaf[lf].child) {
		if(!tree->bsize) { /* Tree is only one leaf: will be freed. */
			full.empty_followers++;
			lf = 0;
		} else { /* Restart with non-empty (`full`) tree, `me`, and `twin`. */
			full.empty_followers = 0;
			full.tr = tree;
			full.me.br0 = 0, full.me.br1 = tree->bsize, full.me.lf = 0;
			do {
				struct trie_branch *const branch
					= full.tr->branch + (full.parent_br = full.me.br0);
				for(byte.next = (bit += branch->skip) / CHAR_BIT;
					byte.cur < byte.next; byte.cur++)
					if(key[byte.cur] == '\0') return 0;
				if(!TRIE_QUERY(key, bit))
					full.twin.lf = full.me.lf + branch->left + 1,
					full.twin.br1 = full.me.br1,
					full.twin.br0 = full.me.br1 = ++full.me.br0 +branch->left;
				else
					full.twin.br0 = ++full.me.br0,
					full.twin.br1 = (full.me.br0 += branch->left),
					full.twin.lf = full.me.lf, full.me.lf += branch->left + 1;
				bit++;
			} while(full.me.br0 < full.me.br1);
			assert(full.me.br0 == full.me.br1
				&& full.me.lf <= full.tr->bsize);
			lf = full.me.lf;
		}
		if(!trie_bmp_test(&tree->is_child, lf)) break;
	}
	/* We have the candidate leaf; check and see if it is a match. */
	if(strcmp(key, PT_(to_key)(rm = tree->leaf[lf].data))) return 0;
	/* Removed the whole trie. Fixme: 1/0/1/0... makes a lot of `malloc`. */
	if(!full.tr) {
		assert(full.empty_followers);
		tree = trie->root, trie->root = 0;
		goto free;
	}

	/* Branch we are deleting could have it's skip value taken up by another. */
	twin = 0;
	if(full.twin.br0 == full.twin.br1) { /* Twin is a leaf. */
		/* If twin continues down another tree. */
		if(trie_bmp_test(&full.tr->is_child, full.twin.lf)) {
			struct PT_(tree) *next = full.tr->leaf[full.twin.lf].child;
			while(!next->bsize && trie_bmp_test(&next->is_child, 0))
				next = next->leaf[0].child;
			if(next->bsize) twin = next->branch + 0;
		}
		/* Fall-through: reduce the size of the trie, twin is data-leaf-like. */
	} else { /* Twin is a branch in the same tree. */
		assert(full.twin.br0 < full.twin.br1);
		twin = full.tr->branch + full.twin.br0;
	}
	if(twin) { /* Collapsing, as determined previously. */
		const unsigned collapse_br = full.tr->branch[full.parent_br].skip + 1
			+ twin->skip;
		/* Removing the data would cause an overflow in `skip`. */
		if(collapse_br > UCHAR_MAX) { errno = EILSEQ; return 0; }
		twin->skip = (unsigned char)collapse_br;
	}

	/* Save the future empty tree for freeing. */
	tree = full.empty_followers ?
		(assert(trie_bmp_test(&full.tr->is_child, full.me.lf)),
		full.tr->leaf[full.me.lf].child) : 0;

	{ /* Go down a second time and modify the tree. Now `lf` goes down. */
		struct { unsigned br0, br1, lf; } mod;
		mod.br0 = 0, mod.br1 = full.tr->bsize, mod.lf = full.me.lf;
		for( ; ; ) {
			struct trie_branch *const branch = full.tr->branch + mod.br0;
			if(branch->left >= mod.lf) {
				if(!branch->left) break;
				mod.br1 = ++mod.br0 + branch->left;
				branch->left--;
			} else {
				if((mod.br0 += branch->left + 1) >= mod.br1) break;
				mod.lf -= branch->left + 1;
			}
		}
	}
	memmove(full.tr->branch + full.parent_br, full.tr->branch
		+ full.parent_br + 1, sizeof *full.tr->branch
		* (full.tr->bsize - full.parent_br - 1));
	memmove(full.tr->leaf + full.me.lf, full.tr->leaf + full.me.lf + 1,
		sizeof *full.tr->leaf * (full.tr->bsize - full.me.lf));
	trie_bmp_remove(&full.tr->is_child, full.me.lf, 1);
	full.tr->bsize--;

free: /* Free all the unused trees. */
	if(full.empty_followers) for( ; ; ) {
		union PT_(leaf) leaf;
		printf("(Freeing %s.)\n", orcify(tree));
		assert(tree && !tree->bsize && !!(full.empty_followers - 1)
			== !!trie_bmp_test(&tree->is_child, 0));
		leaf = tree->leaf[0];
		free(tree);
		if(!--full.empty_followers) break;
		tree = leaf.child;
	}
	return rm;
}

#undef QUOTE
#undef QUOTE_

/** Frees `tree` and it's children recursively. */
static void PT_(clear)(struct PT_(tree) *const tree) {
	unsigned i;
	assert(tree);
	for(i = 0; i <= tree->bsize; i++) if(trie_bmp_test(&tree->is_child, i))
		PT_(clear)(tree->leaf[i].child);
	free(tree);
}

/** Counts the sub-tree `tree`. @order \O(|`any`|) */
static size_t PT_(sub_size)(const struct PT_(tree) *const tree) {
	unsigned i;
	size_t size;
	assert(tree);
	size = tree->bsize + 1;
	/* This is extremely inefficient, but processor agnostic. */
	for(i = 0; i <= tree->bsize; i++) if(trie_bmp_test(&tree->is_child, i))
		size += PT_(sub_size)(tree->leaf[i].child) - 1;
	return size;
}

/** Counts the new iterator `it`. @order \O(|`it`|) */
static size_t PT_(size)(const struct T_(trie_iterator) *const it) {
	struct PT_(tree) *next;
	size_t size;
	unsigned i;
	assert(it);
	if(!it->root || !(next = it->next)) return 0;
	assert(next == it->end
		&& it->leaf <= it->leaf_end && it->leaf_end <= next->bsize + 1);
	size = it->leaf_end - it->leaf;
	for(i = it->leaf; i < it->leaf_end; i++)
		if(trie_bmp_test(&next->is_child, i))
		size += PT_(sub_size)(next->leaf[i].child) - 1;
	return size;
}

/* <!-- iterate interface */

/** Loads the first element of `trie` into `it`. @implements begin */
static void PT_(begin)(struct PT_(iterator) *const it,
	const struct T_(trie) *const trie)
	{ assert(it && trie); it->root = it->next = trie->root; it->leaf = 0; }

/** Advances `it`. @return The previous value or null. @implements next */
static PT_(type) *PT_(next)(struct PT_(iterator) *const it) {
	struct PT_(tree) *tree;
	assert(it);
	/*printf("_next_\n");*/
	if(!it->root || !(tree = it->next)) return 0;
	/* Off the end of the tree. */
	if(it->leaf > tree->bsize) {
		/* Definitely a data leaf or else we would have fallen thought.
		 Unless it had a concurrent modification. That would be bad; don't. */
		const char *key = PT_(to_key)(tree->leaf[tree->bsize].data);
		const struct PT_(tree) *tree1 = it->next;
		struct PT_(tree) *tree2 = it->root;
		size_t bit2 = 0;
		const struct trie_branch *branch2;
		struct { unsigned br0, br1, lf; } in_tree2;
		assert(key && tree2 && !trie_bmp_test(&tree->is_child, tree->bsize));
		/*printf("next: over the end of the tree on %s.\n",
			PT_(to_key)(tree.leaves[it->leaf - 1].data));*/
		for(it->next = 0; ; ) { /* Forest. */
			if(tree2 == tree1) break; /* Reached the tree. */
			in_tree2.br0 = 0, in_tree2.br1 = tree2->bsize, in_tree2.lf = 0;
			while(in_tree2.br0 < in_tree2.br1) { /* Tree. */
				branch2 = tree2->branch + in_tree2.br0;
				bit2 += branch2->skip;
				if(!TRIE_QUERY(key, bit2))
					in_tree2.br1 = ++in_tree2.br0 + branch2->left;
				else
					in_tree2.br0 += branch2->left + 1,
					in_tree2.lf += branch2->left + 1;
				bit2++;
			}
			/* Set it to the next value. */
			if(in_tree2.lf < tree2->bsize)
				it->next = tree2, it->leaf = in_tree2.lf + 1/*,
				printf("next: continues in tree %p, leaf %u.\n",
					(void *)store2.key, it->i)*/;
			/* We never reach the bottom, since it breaks up above. */
			assert(trie_bmp_test(&tree2->is_child, in_tree2.lf));
			tree2 = tree2->leaf[in_tree2.lf].child;
		}
		if(!it->next) { /*printf("next: fin\n");*/ it->leaf = 0; return 0; } /* No more. */
		tree = it->next; /* Update tree. */
	}
	/* Fall through the trees. */
	while(trie_bmp_test(&tree->is_child, it->leaf))
		tree = it->next = tree->leaf[it->leaf].child, it->leaf = 0
		/*, printf("next: fall though.\n")*/; /* !!! */
	/* Until we hit data. */
	/*printf("next: more data\n");*/
	return tree->leaf[it->leaf++].data;
}

/* iterate --> */

/** Initializes `trie` to idle. @order \Theta(1) @allow */
static void T_(trie)(struct T_(trie) *const trie)
	{ assert(trie); trie->root = 0; }

/** Returns an initialized `trie` to idle. @allow */
static void T_(trie_)(struct T_(trie) *const trie) {
	assert(trie);
	if(trie->root) PT_(clear)(trie->root), T_(trie)(trie);
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
#endif

/** @return Looks at only the index of `trie` for potential `key` matches,
 but will ignore the values of the bits that are not in the index.
 @order \O(|`key`|) @allow */
static PT_(type) *T_(trie_match)(const struct T_(trie) *const trie,
	const char *const key) { return PT_(match)(trie, key); }

/** @return Exact match for `key` in `trie` or null no such item exists.
 @order \O(|`key`|), <Thareja 2011, Data>. @allow */
static PT_(type) *T_(trie_get)(const struct T_(trie) *const trie,
	const char *const key) { return PT_(get)(trie, key); }

/** Tries to remove `key` from `trie`. */
static PT_(type) *T_(trie_remove)(struct T_(trie) *const trie,
	const char *const key) { return PT_(remove)(trie, key); }

/** Adds a pointer to `x` into `trie` if the key doesn't exist already.
 @return If the key did not exist and it was created, returns true. If the key
 of `x` is already in `trie`, or an error occurred, returns false.
 @throws[realloc, ERANGE] Set `errno = 0` before to tell if the operation
 failed due to error. @order \O(|`key`|) @allow */
static int T_(trie_add)(struct T_(trie) *const trie, PT_(type) *const x)
	{ return assert(trie && x),
	PT_(get)(trie, PT_(to_key)(x)) ? 0 : PT_(add_unique)(trie, x); }

/** Updates or adds a pointer to `x` into `trie`.
 @param[eject] If not null, on success it will hold the overwritten value or
 a pointer-to-null if it did not overwrite any value.
 @return Success. @throws[realloc, ERANGE] @order \O(|`key`|) @allow */
static int T_(trie_put)(struct T_(trie) *const trie, PT_(type) *const x,
	PT_(type) **const eject)
	{ return assert(trie && x), PT_(put)(trie, x, eject, 0); }

/** Adds a pointer to `x` to `trie` only if the entry is absent or if calling
 `replace` returns true or is null.
 @param[eject] If not null, on success it will hold the overwritten value or
 a pointer-to-null if it did not overwrite any value. If a collision occurs and
 `replace` does not return true, this will be a pointer to `x`.
 @param[replace] Called on collision and only replaces it if the function
 returns true. If null, it is semantically equivalent to <fn:<T>trie_put>.
 @return Success. @throws[realloc, ERANGE] @order \O(|`key`|) @allow */
static int T_(trie_policy_put)(struct T_(trie) *const trie, PT_(type) *const x,
	PT_(type) **const eject, const PT_(replace_fn) replace)
	{ return assert(trie && x), PT_(put)(trie, x, eject, replace); }

/** Fills `it` with iteration parameters that find values of keys that start
 with `prefix` in `trie`.
 @param[prefix] To fill `it` with the entire `trie`, use the empty string.
 @param[it] A pointer to an iterator that gets filled. It is valid until a
 topological change to `trie`. Calling <fn:<T>trie_next> will iterate them in
 order. @order \O(|`prefix`|) */
static void T_(trie_prefix)(const struct T_(trie) *const trie,
	const char *const prefix, struct T_(trie_iterator) *const it)
	{ PT_(prefix)(trie, prefix, it); }

/** Counts the of the items in the new `it`; iterator must be new,
 (calling <fn:<T>trie_next> causes it to become undefined.)
 @order \O(|`it`|) @allow @fixme Allow `<T>trie_next`. */
static size_t T_(trie_size)(const struct T_(trie_iterator) *const it)
	{ return PT_(size)(it); }

/** Advances `it`. @return The previous value or null. @allow */
static PT_(type) *T_(trie_next)(struct T_(trie_iterator) *const it) {
	struct PT_(iterator) shunt;
	PT_(type) *x;
	assert(it && (it->next && it->root || !it->next));
	/* This adds another constraint: instead of ending when the trie has no
	 more entries like <fn:<PT>next>, we check if it has passed the point. */
	if(it->next == it->end && it->leaf >= it->leaf_end) return 0;
	shunt.root = it->root, shunt.next = it->next,
		shunt.leaf = it->leaf, x = PT_(next)(&shunt);
	it->next = shunt.next, it->leaf = shunt.leaf;
	return x;
}

/* <!-- box: Define these for traits. */
#define BOX_ PT_
#define BOX_CONTAINER struct T_(trie)
#define BOX_CONTENTS PT_(type)

#ifdef TRIE_TO_STRING /* <!-- str */
/** Uses the natural `a` -> `z` that is defined by `TRIE_KEY`. */
static void PT_(to_string)(const PT_(type) *const a, char (*const z)[12])
	{ assert(a && z); sprintf(*z, "%.11s", PT_(to_key)(a)); }
#define SZ_(n) TRIE_CAT(T_(trie), n)
#define TO_STRING &PT_(to_string)
#define TO_STRING_LEFT '{'
#define TO_STRING_RIGHT '}'
#include "to_string.h" /** \include */
#undef SZ_
#undef TRIE_TO_STRING
#endif /* str --> */

#ifdef TRIE_TEST /* <!-- test */
#include "../test/test_trie.h" /** \include */
#endif /* test --> */

#undef BOX_
#undef BOX_CONTAINER
#undef BOX_CONTENTS
/* box --> */

static void PT_(unused_base_coda)(void);
static void PT_(unused_base)(void) {
	PT_(begin)(0, 0);
	T_(trie)(0); T_(trie_)(0);
	T_(trie_match)(0, 0); T_(trie_get)(0, 0);
	T_(trie_remove)(0, 0);
	T_(trie_add)(0, 0); T_(trie_put)(0, 0, 0); T_(trie_policy_put)(0, 0, 0, 0);
	T_(trie_prefix)(0, 0, 0); T_(trie_size)(0); T_(trie_next)(0);
	PT_(unused_base_coda)();
}
static void PT_(unused_base_coda)(void) { PT_(unused_base)(); }

#undef TRIE_NAME
#undef TRIE_VALUE
#undef TRIE_KEY
#ifdef TRIE_TEST
#undef TRIE_TEST
#endif
#ifdef TRIE_SET
#undef TRIE_SET
#endif
