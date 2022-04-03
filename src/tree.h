/** @license 2022 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @abstract Stand-alone header <src/tree.h>; examples <test/test_tree.c>. On a
 compatible workstation, `make` creates the test suite of the examples.

 @subtitle Integral tree

 A <tag:<B>tree> is an ordered collection of read-only <typedef:<PB>type>, and
 an optional <typedef:<PB>value> to go with them. One can make this a map, but
 in general, it can have identical keys. Internally, this is a B-tree,
 described in <Bayer, McCreight, 1972 Large>.

 @param[TREE_NAME, TREE_TYPE]
 `<B>` that satisfies `C` naming conventions when mangled, required, and an
 integral type, <typedef:<PB>type>, whose default is `unsigned int`. `<PB>` is
 private, whose names are prefixed in a manner to avoid collisions.

 @param[TREE_VALUE]
 `TRIE_VALUE` is an optional payload to go with the type, <typedef:<PB>value>.

 @param[TREE_COMPARE]
 A function satisfying <typedef:<PB>compare_fn>. Defaults to ascending order.
 Required if `TREE_TYPE` is changed to an incomparable type.

 @param[TREE_EXPECT_TRAIT]
 Do not un-define certain variables for subsequent inclusion in a parameterized
 trait.

 @param[TREE_TO_STRING_NAME, TREE_TO_STRING]
 To string trait contained in <to_string.h>; an optional unique `<SZ>`
 that satisfies `C` naming conventions when mangled and function implementing
 <typedef:<PSZ>to_string_fn>.

 @std C89 */

#ifndef TREE_NAME
#error Name TREE_NAME undefined.
#endif
#if defined(TREE_TO_STRING_NAME) || defined(TREE_TO_STRING)
#define TREE_TO_STRING_TRAIT 1
#else
#define TREE_TO_STRING_TRAIT 0
#endif
#define TREE_TRAITS TREE_TO_STRING_TRAIT
#if TREE_TRAITS > 1
#error Only one trait per include is allowed; use TREE_EXPECT_TRAIT.
#endif
#if defined(TREE_TO_STRING_NAME) && !defined(TREE_TO_STRING)
#error TREE_TO_STRING_NAME requires TREE_TO_STRING.
#endif

#ifndef TREE_H /* <!-- idempotent */
#define TREE_H
#include <stddef.h> /* ? */
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <limits.h>
/* <Kernighan and Ritchie, 1988, p. 231>. */
#if defined(TREE_CAT_) || defined(TREE_CAT) || defined(B_) || defined(PB_) \
	|| defined(TREE_IDLE)
#error Unexpected defines.
#endif
#define TREE_CAT_(n, m) n ## _ ## m
#define TREE_CAT(n, m) TREE_CAT_(n, m)
#define B_(n) TREE_CAT(TREE_NAME, n)
#define PB_(n) TREE_CAT(tree, B_(n))
#define TREE_ORDER 2 /* Maximum branching factor. */
#if TREE_ORDER < 2 || TREE_ORDER > UCHAR_MAX
#error TREE_ORDER parameter range `[2, UCHAR_MAX]`.
#endif
#define TREE_IDLE { 0, 0 }
#endif /* idempotent --> */


#if TREE_TRAITS == 0 /* <!-- base code */


#ifndef TREE_TYPE
#define TREE_TYPE unsigned
#endif
/** An integral type, defaults to `unsigned int`. (We couldh've made this a
 more general comparable type with a comparison function, but we wanted the
 extra cache coherency.) */
typedef TREE_TYPE PB_(type);

#ifdef TREE_VALUE
/** On `TREE_VALUE`, otherwise just a set of integers. */
typedef TREE_VALUE PB_(value);
#endif

/** Returns a positive result if `a` is out-of-order with respect to `b`,
 inducing a strict pre-order. This is compatible, but less strict then the
 comparators from `bsearch` and `qsort`; it only needs to divide entries into
 two instead of three categories. */
typedef int (*PB_(compare_fn))(const PB_(type) a, const PB_(type) b);

#ifndef TREE_COMPARE /* <!-- !cmp */
/** The default `TREE_COMPARE` on `a` and `b` is `a > b`, which makes ascending
 order. @implements <typedef:<PH>compare_fn> */
static int PB_(default_compare)(const PB_(type) a, const PB_(type) b)
	{ return a > b; }
#define TREE_COMPARE &PB_(default_compare)
#endif /* !cmp --> */

/* Check that `TREE_COMPARE` is a function implementing
 <typedef:<PB>compare_fn>, if defined. */
static const PB_(compare_fn) PB_(compare) = (TREE_COMPARE);

/* In <Knuth, 1998 Art 3> terminology, leaf B-tree node (external trunk) of
 `TREE_ORDER`. Drawing a B-tree, the links on the outer nodes are all null. We
 just don't include them. */
struct PB_(outer) {
	unsigned char size;
	PB_(type) x[TREE_ORDER - 1];
#ifdef TREE_VALUE
	PB_(value) value[TREE_ORDER - 1];
#endif
};
/* Internal node of `TRIE_ORDER` inherits from <tag:<PB>outer>, and links.
 Collectively, <tag:<PB>inner> and <tag:<PB>outer> are nodes called trunks. */
struct PB_(inner) { struct PB_(outer) base, *link[TREE_ORDER]; };
/** @return Upcasts `outer` to an inner trunk. */
static struct PB_(inner) *PB_(inner)(struct PB_(outer) *const outer)
	{ return (struct PB_(inner) *)(void *)
	((char *)outer - offsetof(struct PB_(inner), base)); }
/** @return Upcasts `outer` to an inner trunk. */
static const struct PB_(inner) *PB_(inner_c)(const struct PB_(outer) *
	const outer) { return (const struct PB_(inner) *)(const void *)
	((const char *)outer - offsetof(struct PB_(inner), base)); }

#if defined(TREE_VALUE) /* <!-- value */
/** On `TREE_VALUE`, creates a map from type to pointer-to-value. (One can
 modify the `value`, but not the `x`.) */
struct B_(tree_entry) { PB_(type) x; PB_(value) *value; }
/** On `TREE_VALUE`, otherwise it's just an alias for <typedef:<PB>type>. */
typedef struct B_(tree_entry) PB_(entry);
static PB_(type) PB_(to_x)(const PB_(entry) *const entry)
	{ return entry->x; }
#else /* value --><!-- !value */
typedef PB_(type) PB_(value);
typedef PB_(value) PB_(entry);
static PB_(type) PB_(to_x)(const PB_(type) *const x) { return *x; }
#endif /* !entry --> */

/** To initialize it to an idle state, see <fn:<U>tree>, `TRIE_IDLE`, `{0}`
 (`C99`), or being `static`.

 ![States.](../doc/states.png) */
struct B_(tree);
struct B_(tree) { struct PB_(outer) *root; unsigned height; };

struct PB_(iterator) {
	const struct B_(tree) *tree;
	struct PB_(outer) *trunk;
	unsigned idx;
};

/** @param[tree] Can be null. @return Finds the smallest entry in `tree` that
 is not less than `no`. If `no` is higher than any of `tree`, it will be placed
 just passed the end. @order \O(\log |`tree`|) */
static struct PB_(iterator) PB_(lower)(const struct B_(tree) *const tree,
	const PB_(type) no) {
	unsigned h, a0;
	struct PB_(outer) *trunk;
	struct PB_(iterator) it = { 0, 0, 0 };
	if(!tree || !(h = tree->height)) return it;
	for(trunk = tree->root, assert(trunk); ;
		trunk = PB_(inner_c)(trunk)->link[a0]) {
		PB_(type) cmp;
		unsigned a1 = trunk->size;
		h--, a0 = 0, assert(a1);
		do {
			const unsigned m = (a0 + a1) / 2;
			cmp = trunk->x[m];
			if(/* no > cmp */ PB_(compare)(no, cmp) > 0) a0 = m + 1;
			else a1 = m;
		} while(a0 < a1);
		if(!h || no == cmp) break;
	}
	it.tree = tree, it.trunk = trunk, it.idx = a0;
	return it;
}

#if 0
/** Right side of `left`, which must be full, moves to `right`, (which is
 clobbered.) The root of `left` is also clobbered. */
static void PB_(split)(struct PB_(outer) *const left,
	struct PB_(outer) *const right) {
	assert(0);
	unsigned char leaves_split = left->trunk.branch[0].left + 1;;
	assert(left && right && left->trunk.bsize == TRIE_BRANCHES);
	right->trunk.bsize = left->trunk.bsize - leaves_split;
	right->trunk.skip = left->trunk.skip; /* Maybe? */
	memcpy(right->trunk.branch, left->trunk.branch + leaves_split,
		sizeof *left->trunk.branch * right->trunk.bsize);
	memcpy(right->leaf, left->leaf + leaves_split,
		sizeof *left->leaf * (right->trunk.bsize + 1));
	/* Move back the branches of the left to account for the promotion. */
	left->trunk.bsize = leaves_split - 1;
	memmove(left->trunk.branch, left->trunk.branch + 1,
		sizeof *left->trunk.branch * (left->trunk.bsize + 1));
	memmove(left->leaf, left->leaf + 1,
		sizeof *left->leaf * (left->trunk.bsize + 2));
}

/** Open up a spot in the tree. Used in <fn:<PT>add_unique>. This is no longer
 well-defined if any parameters are off.
 @param[key] New <fn:<PT>to_key>.
 @param[diff] Calculated bit where the difference occurs. Has to be zero on
 `type = TRIE_INNER`.
 @param[trunk] Tree-trunk in which the difference occurs. Cannot be full.
 @param[bit0] Tree start bit.
 @param[type] Inner (link) or outer (leaf) type of the `trunk`.
 @return The uninitialized leaf/link. */
static union PB_(leaf_ptr) PB_(tree_open)(enum trie_tree_type type,
	const char *const key, const size_t diff, struct trie_trunk *const trunk,
	size_t bit0, union PB_(leaf_ptr) spot_for_tree_root) {
	struct { unsigned br0, br1, lf; } t;
	struct trie_branch *branch;
	size_t tr1;
	unsigned is_right;
	union PB_(leaf_ptr) ret;
	assert(key && trunk && trunk->bsize < TRIE_BRANCHES
		&& (type == TRIE_OUTER || type == TRIE_INNER)
		&& (type != TRIE_OUTER || bit0 <= diff && spot_for_tree_root.outer)
		&& (type != TRIE_INNER || !bit0 && spot_for_tree_root.inner));
	/* Modify the tree's left branches to account for the new leaf. */
	t.br0 = 0, t.br1 = trunk->bsize, t.lf = 0;
	while(t.br0 < t.br1) { /* Tree. */
		branch = trunk->branch + t.br0;
		tr1 = bit0 + branch->skip;
		/* Decision bits can never be the site of a difference. */
		if(type == TRIE_OUTER && diff <= tr1) { assert(diff < tr1); break; }
		if(!TRIE_QUERY(key, tr1))
			t.br1 = ++t.br0 + branch->left++;
		else
			t.br0 += branch->left + 1, t.lf += branch->left + 1;
		bit0 = tr1 + 1;
	}
	assert(bit0 <= diff && diff - bit0 <= UCHAR_MAX);
	/* Should be the same as the first descent. */
	if(is_right = type == TRIE_OUTER && !!TRIE_QUERY(key, diff))
		t.lf += t.br1 - t.br0 + 1;

	/************************ also promote the root ****************/
	/* Expand the tree to include one more leaf and branch. */
	assert(t.lf <= trunk->bsize + 1);
	if(type == TRIE_INNER) {
		struct trie_inner_leaf *const leaf = trie_inner(trunk)->leaf + t.lf;
		memmove(leaf + 1, leaf, sizeof *leaf * ((trunk->bsize + 1) - t.lf));
		ret.inner = leaf;
	} else {
		PB_(entry) *const leaf = PB_(outer)(trunk)->leaf + t.lf;
		memmove(leaf + 1, leaf, sizeof *leaf * ((trunk->bsize + 1) - t.lf));
		ret.outer = leaf;
	}
	branch = trunk->branch + t.br0;
	if(t.br0 != t.br1) { /* Split with existing branch. */
		assert(t.br0 < t.br1 && diff + 1 <= bit0 + branch->skip);
		branch->skip -= diff - bit0 + 1;
	}
	memmove(branch + 1, branch, sizeof *branch * (trunk->bsize - t.br0));
	branch->left = is_right ? (unsigned char)(t.br1 - t.br0) : 0;
	branch->skip = (unsigned char)(diff - bit0);
	trunk->bsize++;
	return ret;
}
struct PB_(iterator) it;
if(!(it = PB_(lower)(tree, x)).tree) return 0;
assert(it.trunk && it.idx < it.trunk->size);
if(it.trunk->size >= TRIE_ORDER - 1) {
	assert(0);
} else {

}
#endif

#if 0
/** Adds `x` in the first spot on `tree`. @return A pointer to the new value.
 @throw[malloc, ERANGE] */
static PB_(value) *PB_(add)(struct B_(tree) *const tree, PB_(type) x) {
	struct { /* Last inner trie that is not full. */
		struct { struct trie_trunk *trunk; size_t height, diff; } unfull;
		size_t full; /* Count after the last. */
	} history;
	PB_(type) cmp;
	struct PB_(inner) *inner = 0;
	struct PB_(outer) *outer = 0;
	struct trie_inner_tree *new_root = 0;
	assert(tree && x && x);

	printf("unique: adding \"%s\".\n", x);
start:
	if(!(d.h = trie->node_height)) { /* Solitary. */
		if(tree->root) outer = PB_(outer)(tree->root);
		else if(outer = malloc(sizeof *outer)) trie->root = &outer->trunk;
		else goto catch;
		outer->trunk.bsize = 0, outer->trunk.skip = 0, outer->leaf[0] = x;
		trie->node_height = 1;
		printf("add: new outer %s-tree that holds \"%s\"=>\"%s\".\n",
			orcify(outer), PB_(to_key)(x), PB_(to_key)(outer->leaf[0]));
		return 1;
	}

	/* Find the first bit not in the tree. */
	history.unfull.trunk = 0, history.unfull.height = 0,
		history.unfull.diff = 0, history.full = 0;
	for(d.trunk = trie->root, assert(d.trunk), d.diff = 0; ;
		d.trunk = trie_inner(d.trunk)->leaf[d.lf].link) {
		const int is_full = TRIE_BRANCHES <= d.trunk->bsize;
		trunk_diff = d.diff;
		assert(d.trunk->skip < d.h), d.h -= 1 + d.trunk->skip;
		history.full = is_full ? history.full + 1 : 0;
		sample = PB_(sample)(d.trunk, d.h, 0);
		printf("add: find, %s-tree, sample %s.\n", orcify(d.trunk), sample);
		d.br0 = 0, d.br1 = d.trunk->bsize, d.lf = 0;
		while(d.br0 < d.br1) {
			const struct trie_branch *const branch = d.trunk->branch + d.br0;
			const size_t bit1 = d.diff + branch->skip;
			for( ; d.diff < bit1; d.diff++)
				if(TRIE_DIFF(key, sample, d.diff)) goto found;
			if(!TRIE_QUERY(key, d.diff)) {
				d.br1 = ++d.br0 + branch->left;
			} else {
				d.br0 += branch->left + 1, d.lf += branch->left + 1;
				sample = PB_(sample)(d.trunk, d.h, d.lf);
			}
			d.diff++;
		}
		if(!d.h) break;
		if(!is_full) history.unfull.trunk = d.trunk,
			history.unfull.height = d.h, history.unfull.diff = trunk_diff;
	}
	{ /* Got to a leaf without getting a difference. */
		const size_t limit = d.diff + UCHAR_MAX;
		while(!TRIE_DIFF(key, sample, d.diff))
			if(++d.diff > limit) return errno = EILSEQ, 0;
	}
found:
	/* Account for choosing the right leaf. */
	if(!!TRIE_QUERY(key, d.diff)) d.lf += d.br1 - d.br0 + 1;

	/* If the tree is full, backtrack and split. */
	if(history.full) goto split;
split_end:

	{ /* Insert into unfilled tree. */
		union PB_(leaf_ptr) dumb;
		dumb.outer = &x;
		union PB_(leaf_ptr) ptr = PB_(tree_open)(TRIE_OUTER, key, d.diff,
			d.trunk, trunk_diff, dumb);
		memcpy(ptr.outer, &x, sizeof x);
	}
	return 1;

split:
	do {
		size_t add_outer = 1,
			add_inner = history.full - !d.h + !history.unfull.trunk;
		int is_above = d.br0 == 0 && d.br1 == d.trunk->bsize;
		printf("add: history last unfull, %s-tree, followed by %lu full.\n",
			orcify(history.unfull.trunk),
			(unsigned long)history.full);
		printf("add: we will need an additional %lu outer tree"
			" and %lu inner trees.\n", add_outer, add_inner);
		printf("add: is above %s.\n", is_above ? "yes" : "no");
		printf("add: %s-tree, height %lu.\n", orcify(d.trunk), d.h);
		if(!history.unfull.trunk) { /* Trie is full -- increase height. */
			if(!(new_root = malloc(sizeof *new_root))) goto catch;
			printf("add: new root %s.\n", orcify(&new_root->trunk));
		} else { /* Add to not-full. */
			struct trie_inner_leaf dumb;
			union PB_(leaf_ptr) ptr;
			ptr.inner = &dumb;
			/********* I want a state machine ***************/
			/* machine parameters: key, trunk, diff, height */
			PB_(tree_open)(TRIE_INNER, key, 0, history.unfull.trunk,
				history.unfull.diff, ptr);
			assert(0);
		}
		assert(0);
	} while(--history.full);
	//trunk = history.unfull.trunk, trunk_diff = history.unfull.trunk_diff;
	assert(0);
	/* It was in the promoted bit's skip and "Might be full now," was true.
	 Don't have enough information to recover, but ca'n't get here twice. */
	if(TRIE_BRANCHES <= d.trunk->bsize) { assert(!restarts++); goto start; }
	goto split_end;

catch:
	free(inner), free(outer), free(new_root);
	if(!errno) errno = ERANGE;
	return 0;
}
#endif

/* <!-- iterate interface */

/** Loads the first element of `tree` (can be null) into `it`.
 @implements begin */
static void PB_(begin)(struct PB_(iterator) *const it,
	const struct B_(tree) *const tree) {
	unsigned h;
	struct PB_(outer) *t;
	assert(it);
	it->tree = 0, it->trunk = 0, it->idx = 0;
	if(!tree || !(h = tree->height)) return;
	for(t = tree->root; ; t = PB_(inner_c)(t)->link[0]) {
		if(!t->size) return; /* Was bulk loading? */
		if(!--h) break;
	}
	it->tree = tree, it->trunk = t, it->idx = 0;
}

/** Advances `it`. @return The previous value or null. @implements next */
const static PB_(type) *PB_(next)(struct PB_(iterator) *const it) {
	assert(it);
	printf("_next_\n");
	if(!it->tree || !it->trunk || !it->trunk->size) return 0;
	if(it->idx > it->trunk->size) {
		/* Off the end of the trunk; keep track of the next trunk. */
		unsigned h, a0, next_h;
		struct PB_(outer) *trunk, *next = 0;
		PB_(type) prev;
		if(!(h = it->tree->height)) goto finish; /* Empty now? */
		prev = it->trunk->x[it->trunk->size - 1];
		for(trunk = it->tree->root; ; trunk = PB_(inner_c)(trunk)->link[a0]) {
			PB_(type) cmp;
			unsigned a1 = trunk->size;
			if(!a1) goto finish; /* Non-valid concurrent modification? */
			if(--h) break;
			a0 = 0;
			do {
				const unsigned m = (a0 + a1) / 2;
				cmp = trunk->x[m];
				if(PB_(compare)(prev, cmp)) a0 = m + 1; else a1 = m;
			} while(a0 < a1);
			if(a0 < trunk->size - 1)
				next = PB_(inner_c)(trunk)->link[a0 + 1], next_h = h;
		}
		if(!next) goto finish;
		assert(0);
	}
	return it->trunk->x + it->idx++;
finish:
	{ it->trunk = 0; return 0; }
}

/* iterate --> */

/** Frees `tr` at `h` and it's children recursively. `height` is the node
 height, (height plus one.) */
static void PB_(clear_r)(struct PB_(outer) *const tr, size_t height) {
	/* This doesn't want to clear one. */
	assert(tr && height);
	if(height--) {
		unsigned i;
		for(i = 0; i <= tr->size; i++)
			PB_(clear_r)(PB_(inner)(tr)->link[i], height);
		free(PB_(inner)(tr));
	} else {
		free(tr);
	}
}

/** Stores an iteration in a tree. Generally, changes in the topology of the
 tree invalidate it. */
struct B_(tree_iterator) { struct PB_(iterator) i; };

/** Initializes `tree` to idle. @order \Theta(1) @allow */
static void B_(tree)(struct B_(tree) *const tree)
	{ assert(tree); tree->root = 0; tree->height = 0; }

/** Returns an initialized `tree` to idle. @allow */
static void B_(tree_)(struct B_(tree) *const tree) {
	/*struct PB_(outer_tree) *clear_all = (struct PB_(outer_tree) *)1;*/
	assert(tree);
	PB_(clear_r)(tree->root, tree->height);
	B_(tree)(tree);
}

/** @param[tree] Can be null. @return Finds the smallest entry in `tree` that
 is not less than `x`. If `x` is higher than any of `tree`, it will be placed
 just passed the end. @order \O(\log |`tree`|) @allow */
static struct B_(tree_iterator) B_(tree_lower)(const struct B_(tree)
	*const tree, const PB_(type) x)
	{ struct B_(tree_iterator) it; it.i = PB_(lower)(tree, x); return it; }

/** @return Lowest match for `x` in `tree` or null no such item exists.
 @order \O(\log |`tree`|) @allow */
static PB_(value) *B_(tree_get)(const struct B_(tree) *const tree,
	const PB_(type) x) {
	const struct PB_(iterator) i = PB_(lower)(tree, x);
	return i.tree && i.trunk ? i.trunk->x + i.idx : 0;
}

/** `x` must be not less than the largest element in `tree`. */
static PB_(value) *PB_(bulk_add)(struct B_(tree) *const tree, PB_(type) x) {
	struct PB_(iterator) it;
	if(!(it = PB_(lower)(tree, x)).tree) return 0;
	assert(it.trunk && it.idx < it.trunk->size);
	if(it.trunk->size >= TREE_ORDER - 1) {
		assert(0);
	} else {

	}
	return 0;
}

#if 0
/** Updates or adds a pointer to `x` into `trie`.
 @param[eject] If not null, on success it will hold the overwritten value or
 a pointer-to-null if it did not overwrite any value.
 @return Success. @throws[realloc, ERANGE] @order \O(|`key`|) @allow */
static int B_(trie_put)(struct B_(trie) *const trie, const PB_(entry) x,
	PB_(entry) */*const fixme*/eject)
	{ return assert(trie && x), PB_(put)(trie, x, &eject, 0); }

/** Adds a pointer to `x` to `trie` only if the entry is absent or if calling
 `replace` returns true or is null.
 @param[eject] If not null, on success it will hold the overwritten value or
 a pointer-to-null if it did not overwrite any value. If a collision occurs and
 `replace` does not return true, this will be a pointer to `x`.
 @param[replace] Called on collision and only replaces it if the function
 returns true. If null, it is semantically equivalent to <fn:<T>trie_put>.
 @return Success. @throws[realloc, ERANGE] @order \O(|`key`|) @allow */
static int B_(trie_policy)(struct B_(trie) *const trie, const PB_(entry) x,
	PB_(entry) */*const*/ eject, const PB_(replace_fn) replace)
	{ return assert(trie && x), PB_(put)(trie, x, &eject, replace); }

/** Tries to remove `key` from `trie`. @return Success. */
static int B_(trie_remove)(struct B_(trie) *const trie,
	const char *const key) { return PB_(remove)(trie, key); }
#endif

/** Advances `it`. @return The previous value or null. @allow */
static const PB_(entry) *B_(trie_next)(struct B_(tree_iterator) *const it)
	{ return PB_(next)(&it->i); }

#if 0
/** Counts the of the items in initialized `it`. @order \O(|`it`|) @allow */
static size_t B_(trie_size)(const struct B_(trie_iterator) *const it)
	{ return assert(it), PB_(size_r)(&it->i); }
#endif

/* <!-- box */
#define BOX_ PB_
#define BOX_CONTAINER struct B_(tree)
#define BOX_CONTENTS PB_(value)

#ifdef TREE_TEST /* <!-- test */
/* Forward-declare. */
static void (*PB_(to_string))(const PB_(type) *, char (*)[12]);
static const char *(*PB_(tree_to_string))(const struct B_(tree) *);
#include "../test/test_tree.h"
#endif /* test --> */

static void PB_(unused_base_coda)(void);
static void PB_(unused_base)(void) {
	PB_(begin)(0, 0);
	PB_(unused_base_coda)();
}
static void PB_(unused_base_coda)(void) { PB_(unused_base)(); }


#elif defined(TREE_TO_STRING) /* base code --><!-- to string trait */


#ifdef TREE_TO_STRING_NAME
#define SZ_(n) TREE_CAT(B_(tree), TREE_CAT(TREE_TO_STRING_NAME, n))
#else
#define SZ_(n) TREE_CAT(B_(tree), n)
#endif
#define TO_STRING TREE_TO_STRING
#define TO_STRING_LEFT '{'
#define TO_STRING_RIGHT '}'
#include "to_string.h" /** \include */
#ifdef TREE_TEST /* <!-- expect: greedy satisfy forward-declared. */
#undef TREE_TEST
static PSZ_(to_string_fn) PB_(to_string) = PSZ_(to_string);
static const char *(*PB_(tree_to_string))(const struct B_(tree) *)
	= &SZ_(to_string);
#endif /* expect --> */
#undef SZ_
#undef TREE_TO_STRING
#ifdef TREE_TO_STRING_NAME
#undef TREE_TO_STRING_NAME
#endif


#endif /* traits --> */


#ifdef TREE_EXPECT_TRAIT /* <!-- trait */
#undef TREE_EXPECT_TRAIT
#else /* trait --><!-- !trait */
#ifdef TREE_TEST
#error No TREE_TO_STRING traits defined for TREE_TEST.
#endif
#undef TREE_NAME
#undef TREE_TYPE
#undef TREE_COMPARE
#ifdef TREE_VALUE
#undef TREE_VALUE
#endif
#ifdef TREE_TEST
#undef TREE_TEST
#endif
#undef BOX_
#undef BOX_CONTAINER
#undef BOX_CONTENTS
/* box --> */
#endif /* !trait --> */
#undef ARRAY_TO_STRING_TRAIT
#undef ARRAY_TRAITS
