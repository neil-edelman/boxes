/** @license 2022 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @abstract Header <../../src/tree.h>; examples <../../test/test_tree.c>;
 article <../tree/tree.pdf>.

 @subtitle Ordered tree

 ![Example of an order-3 tree.](../doc/tree/tree.png)

 A <tag:<t>tree> is an ordered set or map contained in an unstable tree; the
 order is suppled by <fn:<t>less>. For memory locality, this is implemented
 B-tree, described in <Bayer, McCreight, 1972, Large>.

 All operations are fail-fast and will not compromise the integrity of any
 existing tree.

 @param[TREE_NAME, TREE_KEY]
 `<t>` that satisfies `C` naming conventions when mangled, and `TREE_KEY`, a
 type, <typedef:<pT>key>, required.

 @param[TREE_VALUE]
 Optional payload to go with the type, <typedef:<pT>value>, thus making it a
 map instead of a set.

 @param[TREE_ORDER]
 Sets the branching factor, or order as <Knuth, 1998 Art 3>, to the range
 `[3, UINT_MAX+1]`. Default 65 is tuned to an integer to pointer map, and
 should be okay for most variations. 4 is isomorphic to left-leaning red-black
 tree, <Sedgewick, 2008, LLRB>. The above illustration is 5.

 @param[TREE_DEFAULT]
 Default trait which must be set to a <typedef:<pT>value>, used in
 <fn:<T>tree<R>get>.

 @param[TREE_TO_STRING]
 To string trait contained in <../../src/to_string.h>. See
 <typedef:<pT>to_string_fn>.

 @param[TREE_EXPECT_TRAIT, TREE_TRAIT]
 Named traits are obtained by including `tree.h` multiple times with
 `TREE_EXPECT_TRAIT` and then subsequently including the name that satisfies
 `C` naming conventions when mangled in `TREE_TRAIT`.

 @param[TREE_DECLARE_ONLY, TREE_NON_STATIC]
 For headers in different compilation units.

 @fixme merge, difference, trie
 @depend [box](../../src/box.h)
 @std C89 */

#if !defined TREE_NAME || !defined TREE_KEY
#	error Name or key undefined.
#endif
#if !defined BOX_ENTRY1 && (defined TREE_TRAIT ^ defined BOX_MAJOR)
#	error Trait name must come after expect trait.
#endif
#if defined TREE_TEST && (!defined TREE_TRAIT && !defined TREE_TO_STRING \
	|| defined TREE_TRAIT && !defined TREE_HAS_TO_STRING)
#error Test requires to string.
#endif
#if defined BOX_TRAIT && !defined TREE_TRAIT
#	error Unexpected flow.
#endif

#ifdef TREE_TRAIT
#	define BOX_TRAIT TREE_TRAIT /* Ifdef in <box.h>. */
#endif
#ifdef TREE_NON_STATIC
#	define BOX_NON_STATIC
#endif
#ifdef TREE_DECLARE_ONLY
#	define BOX_DECLARE_ONLY
#endif
#define BOX_START
#include "box.h"

#ifndef TREE_H
#	define TREE_H
/* Leaf: `TREE_MAX type`; branch: `TREE_MAX type + TREE_ORDER pointer`. In
 <Goodrich, Tamassia, Mount, 2011, Data>, these are (a,b)-trees as
 (TREE_MIN+1,TREE_MAX+1)-trees. */
#	define TREE_MAX (TREE_ORDER - 1)
/* This is the worst-case branching factor; the performance will be
 \O(log_{`TREE_MIN`+1} `size`). Usually this is `⌈(TREE_MAX+1)/2⌉-1`. However,
 smaller values are less-eager; in the extreme,
 <Johnson, Shasha, 1993, Free-at-Empty>, show good results. */
#	define TREE_MIN (TREE_MAX / 3 ? TREE_MAX / 3 : 1)
#	define TREE_SPLIT (TREE_ORDER / 2) /* Split index: even order left-leaning. */
#	define TREE_RESULT X(ERROR), X(ABSENT), X(PRESENT)
#	define X(n) TREE_##n
/** A result of modifying the tree, of which `TREE_ERROR` is false.

 ![A diagram of the result states.](../doc/tree/result.png) */
enum tree_result { TREE_RESULT };
#	undef X
#	ifndef TREE_DECLARE_ONLY
#		define X(n) #n
/** A static array of strings describing the <tag:tree_result>. */
static const char *const tree_result_str[] = { TREE_RESULT };
#		undef X
#	endif
#	undef TREE_RESULT
struct tree_node_count { size_t branches, leaves; };
#endif

#ifndef TREE_TRAIT /* <!-- base code */
#	include <stddef.h> /* That's weird. */
#	include <stdlib.h>
#	include <string.h>
#	include <errno.h>
#	include <assert.h>
#	include <limits.h>

#	ifndef TREE_ORDER
#		define TREE_ORDER 65 /* Maximum branching factor. Sets granularity. */
#	endif
#	if TREE_ORDER < 3 || TREE_ORDER > UINT_MAX + 1
#		error TREE_ORDER parameter range `[3, UINT_MAX+1]`.
#	endif

#	define BOX_MINOR TREE_NAME
#	define BOX_MAJOR tree

/** Ordered type used by <typedef:<pT>less_fn>; defaults to `unsigned`. */
typedef TREE_KEY pT_(key);

#	ifdef TREE_VALUE
/** On `TREE_VALUE`, this creates a map, otherwise a set of
 <typedef:<pT>key>. */
typedef TREE_VALUE pT_(value);
#	endif

/* These rules are lazier than the original—described in <Knuth, 1998 Art 3>—so
 as to not exhibit worst-case behaviour in small trees, as
 <Johnson, Shasha, 1993, Free-at-Empty>.

 In an effort to work with the same terminology as tries—from
 <https://bugwoodcloud.org/resource/files/15277.pdf>
 <Coder, Kim D, 2018, Tree anatomy>—what are usually "nodes" are boughs. There
 are 2 kinds of bough: leaf-boughs and branch-boughs.

 * Every branch-bough (stem) has at most `TREE_ORDER == TREE_MAX + 1` children.
 * Every non-trunk and non-bulk-loaded bough has at least `TREE_MIN` keys,
   (`⎣TREE_MAX/3⎦`.)
 * Every branch-bough (stem) also has the number of keys plus one children;
   (that is, it is an implicit full binary tree.)
 * All leaf-boughs are at the height one; they do'n't carry links to other
   boughs.
 * Bulk-loading always is ascending. */
struct pT_(bough) {
	unsigned size;
	pT_(key) key[TREE_MAX]; /* Cache-friendly lookup. */
#	ifdef TREE_VALUE
	pT_(value) value[TREE_MAX];
#	endif
};
/* A branch-bough is a specialization of leaf-bough that has links to
 lower-level boughs. */
struct pT_(branch_bough) { struct pT_(bough) base, *child[TREE_ORDER]; };

/* fixme: Notch (add) and nick (delete) are good names for the highest
 non-full node, in spirit with the tree analogy. Replace `<pT>node` with a
 uniform terminology between `tree` and `tree`. This should be made of
 fixed-size `<pT>bought`, with a `tree` as—for tree—an implicit structure
 realized by binary search. The root bought is the `trunk`. */

/* A pointer to a bough plus height is a [sub]-tree. */
struct pT_(tree) { struct pT_(bough) *bough; unsigned height; };
/** See <fn:<t>tree>.

 ![States.](../doc/tree/states.png) */
struct t_(tree);
struct t_(tree) { struct pT_(tree) trunk; };
typedef struct t_(tree) pT_(box);

/* Address of a specific key by node. Height might not be used, but there's too
 many structs in this file anyway. (That's <tag:<pT>tree>…merge this) */
struct pT_(ref) {
	struct pT_(bough) *bough; /* If null, others ignored. */
	unsigned height, idx; /* `idx < node.size` means valid. */
};

struct T_(cursor) { struct pT_(tree) *trunk; struct pT_(ref) ref; };

struct pT_(entry) {
	pT_(key) key;
#	ifdef TREE_VALUE
	pT_(value) *value;
#	endif
};

/** Adding, deleting, or changes in the topology of the tree invalidate the
 iterator. To modify the tree while iterating, take the <fn:<T>key> and restart
 the iterator with <fn:<T>less> or <fn:<T>more> as appropriate. */

#	ifdef BOX_NON_STATIC /* Public functions. */
struct T_(cursor) T_(begin)(const struct t_(tree) *);
int T_(exists)(struct T_(cursor) *);
pT_(ref) *T_(entry)(struct T_(cursor) *);
pT_(key) T_(key)(const struct T_(cursor) *);
#		ifdef TREE_VALUE
pT_(value) *T_(value)(const struct T_(cursor) *);
#		endif
void T_(next)(struct T_(cursor) *);
void T_(previous)(struct T_(cursor) *);
struct T_(cursor) T_(less)(struct t_(tree) *, pT_(key));
struct T_(cursor) T_(more)(struct t_(tree) *, pT_(key));
struct t_(tree) t_(tree)(void);
void t_(tree_)(struct t_(tree) *);
void T_(clear)(struct t_(tree) *);
size_t T_(count)(const struct t_(tree) *);
int T_(contains)(const struct t_(tree) *, pT_(key));
pT_(value) T_(get_or)(const struct t_(tree) *, pT_(key), pT_(value));
pT_(key) T_(less_or)(const struct t_(tree) *, pT_(key), pT_(key));
pT_(key) T_(more_or)(const struct t_(tree) *, pT_(key), pT_(key));
#		ifdef TREE_VALUE
enum tree_result T_(assign)(struct t_(tree) *, pT_(key), pT_(value) **);
enum tree_result T_(update)(struct t_(tree) *, pT_(key), pT_(key) *, pT_(value) **);
enum tree_result T_(bulk_assign)(struct t_(tree) *, pT_(key), pT_(value) **);
#		else
enum tree_result T_(add)(struct t_(tree) *, pT_(key));
enum tree_result T_(update)(struct t_(tree) *, pT_(key), pT_(key) *);
enum tree_result T_(bulk_add)(struct t_(tree) *, pT_(key));
#		endif
int T_(bulk_finish)(struct t_(tree) *);
int T_(remove)(struct t_(tree) *, pT_(key));
int T_(clone)(struct t_(tree) *restrict, const struct t_(tree) *restrict);
#	endif
#	ifndef BOX_DECLARE_ONLY /* <!-- body */

/** Inducing a strict weak order by returning a positive result if `a` is
 out-of-order with respect to `b`. Compatible, but less strict then the
 comparators from `bsearch` and `qsort`: it only needs to divide entries into
 two instead of three categories.  For example, `return a > b` or
 `return strcmp(a, b)` would give an ascending tree. */
typedef int (*pT_(less_fn))(const pT_(key) a, const pT_(key) b);

/** @return Downcasts `as_leaf` to a branch. */
static struct pT_(branch_bough) *pT_(as_branch)(struct pT_(bough) *const bough)
	{ return (struct pT_(branch_bough) *)(void *)
	((char *)bough - offsetof(struct pT_(branch_bough), base)); }
/** @return Downcasts `as_node` to a branch. FIXME! this should not be necessary. */
static const struct pT_(branch_bough) *pT_(as_branch_c)(const struct pT_(bough) *
	const bough) { return (const struct pT_(branch_bough) *)(const void *)
	((const char *)bough - offsetof(struct pT_(branch_bough), base)); }
#		ifdef TREE_VALUE /* <!-- value */
/** Gets the value of `ref`. */
static pT_(value) *pT_(ref_to_valuep)(const struct pT_(ref) ref)
	{ return ref.bough ? ref.bough->value + ref.idx : 0; }
#		else /* value --><!-- !value */
typedef pT_(key) pT_(value);
/** Gets the value of `ref`. */
static pT_(value) *pT_(ref_to_valuep)(const struct pT_(ref) ref)
	{ return ref.bough ? ref.bough->key + ref.idx : 0; }
#		endif /* !value --> */

/** Finds greatest lower-bound of `x` in `lo` only in one bough. */
static void pT_(node_lb)(struct pT_(ref) *const lo, const pT_(key) x) {
	unsigned hi = lo->bough->size; lo->idx = 0;
	assert(lo && lo->bough && hi);
	do {
		const unsigned mid = (lo->idx + hi) / 2; /* Will not overflow. */
		/* Make sure one has declared <typedef:<pT>less_fn> `<t>less`. */
		if(t_(less)(x, lo->bough->key[mid]) > 0) lo->idx = mid + 1;
		else hi = mid;
	} while(lo->idx < hi);
}
/** Finds `idx` of 'least upper-bound' (C++ parlance) majorant of `x` in `hi`
 only in one node at a time. */
static void pT_(node_ub)(struct pT_(ref) *const hi, const pT_(key) x) {
	unsigned lo = 0;
	assert(hi->bough && hi->idx);
	do {
		const unsigned mid = (lo + hi->idx) / 2;
		if(t_(less)(hi->bough->key[mid], x) <= 0) lo = mid + 1;
		else hi->idx = mid;
	} while(lo < hi->idx);
}
/** @return A reference to the greatest key at or less than `x` in `tree`, or
 the reference will be empty if the `x` is less than all `tree`. */
static struct pT_(ref) pT_(less)(const struct pT_(tree) tree,
	const pT_(key) x) {
	struct pT_(ref) hi, found;
	found.bough = 0;
	if(!tree.bough || !tree.height) return found;
	for(hi.bough = tree.bough, hi.height = tree.height; ;
		hi.bough = pT_(as_branch_c)(hi.bough)->child[hi.idx], hi.height--) {
		if(!(hi.idx = hi.bough->size)) continue;
		pT_(node_ub)(&hi, x);
		if(hi.idx) { /* Within bounds to record the current predecessor. */
			found = hi, found.idx--;
			/* Equal. */
			if(t_(less)(x, found.bough->key[found.idx]) <= 0) break;
		}
		if(hi.height <= 1) break; /* Reached the bottom. */
	}
	return found;
}
/** @return A reference to the smallest key at or more than `x` in `tree`, or
 the reference will be empty if the `x` is more than all `tree`. */
static struct pT_(ref) pT_(more)(const struct pT_(tree) tree,
	const pT_(key) x) {
	struct pT_(ref) lo, found;
	found.bough = 0;
	if(!tree.bough || !tree.height) return found;
	for(lo.bough = tree.bough, lo.height = tree.height; ;
		lo.bough = pT_(as_branch_c)(lo.bough)->child[lo.idx], lo.height--) {
		unsigned hi = lo.bough->size; lo.idx = 0;
		if(!hi) continue;
		pT_(node_lb)(&lo, x);
		if(lo.idx < lo.bough->size) {
			found = lo;
			if(t_(less)(x, lo.bough->key[lo.idx]) > 0) break;
		}
		if(lo.height <= 1) break;
	}
	return found;
}
/** Finds an exact key `x` in non-empty `tree`. */
static struct pT_(ref) pT_(lookup_find)(const struct pT_(tree) tree,
	const pT_(key) x) {
	struct pT_(ref) lo;
	if(!tree.bough || !tree.height) return lo.bough = 0, lo;
	for(lo.bough = tree.bough, lo.height = tree.height; ;
		lo.bough = pT_(as_branch_c)(lo.bough)->child[lo.idx], lo.height--) {
		unsigned hi = lo.bough->size; lo.idx = 0;
		if(!hi) continue;
		pT_(node_lb)(&lo, x);
		/* Absolutely will not equivalent `x > lo`, investigate? */
		if(lo.idx < lo.bough->size && t_(less)(lo.bough->key[lo.idx], x) <= 0)
			break;
		if(lo.height <= 1) { lo.bough = 0; break; }
	}
	return lo;
}
/** Finds lower-bound of key `x` in non-empty `tree` while counting the
 non-filled `hole` and `is_equal`. Used in insert. */
static struct pT_(ref) pT_(lookup_hole)(struct pT_(tree) tree,
	const pT_(key) x, struct pT_(ref) *const hole, int *const is_equal) {
	struct pT_(ref) lo;
	hole->bough = 0;
	assert(tree.height >= 1);
	for(lo.bough = tree.bough, lo.height = tree.height; ;
		lo.bough = pT_(as_branch_c)(lo.bough)->child[lo.idx], lo.height--) {
		unsigned hi = lo.bough->size; lo.idx = 0;
		if(hi < TREE_MAX) *hole = lo;
		if(!hi) continue;
		pT_(node_lb)(&lo, x);
		if(lo.bough->size < TREE_MAX) hole->idx = lo.idx;
		if(lo.idx < lo.bough->size && t_(less)(lo.bough->key[lo.idx], x) <= 0)
			{ *is_equal = 1; break; }
		if(lo.height <= 1) break;
	}
	return lo;
}
/** Finds exact key `x` in non-empty `tree`. If `node` is found, temporarily,
 the nodes that have `TREE_MIN` keys have
 `as_branch(node).child[TREE_MAX] = parent` or, for leaves, `leaf_parent`,
 which must be set. (Patently terrible for running concurrently; hack, would be
 nice to go down tree maybe.) */
static struct pT_(ref) pT_(lookup_remove)(struct pT_(tree) tree,
	const pT_(key) x, struct pT_(bough) **leaf_parent) {
	struct pT_(bough) *parent = 0;
	struct pT_(ref) lo;
	for(lo.bough = tree.bough, lo.height = tree.height; ;
		lo.bough = pT_(as_branch_c)(lo.bough)->child[lo.idx], lo.height--) {
		unsigned hi = lo.bough->size; lo.idx = 0;
		/* Cannot delete bulk add. */
		if(parent && hi < TREE_MIN || !parent && !hi) break;
		if(hi <= TREE_MIN) { /* Remember the parent temporarily. */
			if(lo.height > 1) pT_(as_branch)(lo.bough)->child[TREE_MAX] = parent;
			else *leaf_parent = parent;
		}
		pT_(node_lb)(&lo, x);
		if(lo.idx < lo.bough->size && t_(less)(lo.bough->key[lo.idx], x) <= 0)
			goto finally;
		if(lo.height <= 1) break;
		parent = lo.bough;
	}
	lo.bough = 0;
finally:
	return lo;
}
/** Removes `x` from `tree` which must have contents. */
static int pT_(remove)(struct pT_(tree) *const tree, const pT_(key) x) {
	struct pT_(ref) rm, parent /* Only if `key.size <= TREE_MIN`. */;
	struct pT_(branch_bough) *parentb;
	struct { struct pT_(bough) *less, *more; } sibling;
	pT_(key) provisional_x = x;
	parent.bough = 0;
	assert(tree && tree->bough && tree->height);
	/* Traverse down the tree until `key`, leaving breadcrumbs for parents of
	 minimum key nodes. */
	if(!(rm = pT_(lookup_remove)(*tree, x, &parent.bough)).bough) return 0;
	/* Important when `rm = parent`; `find_idx` later. */
	parent.height = rm.height + 1;
	assert(rm.idx < rm.bough->size);
	if(rm.height > 1) goto branch; else goto upward;
branch: {
	struct { struct pT_(ref) leaf; struct pT_(bough) *parent; unsigned top; }
		pred, succ, chosen;
	assert(rm.height > 1);
	/* Predecessor leaf. */
	pred.leaf = rm, pred.top = UINT_MAX /* Not sure what this means? */;
	do {
		struct pT_(bough) *const up = pred.leaf.bough;
		pred.leaf.bough = pT_(as_branch_c)(pred.leaf.bough)->child[pred.leaf.idx];
		pred.leaf.idx = pred.leaf.bough->size;
		pred.leaf.height--;
		if(pred.leaf.bough->size < TREE_MIN) /* Possible in bulk-add? */
			{ pred.leaf.bough = 0; goto no_pred; }
		else if(pred.leaf.bough->size > TREE_MIN) pred.top = pred.leaf.height;
		else if(pred.leaf.height)
			pT_(as_branch)(pred.leaf.bough)->child[TREE_MAX] = up;
		else pred.parent = up;
	} while(pred.leaf.height > 1);
	pred.leaf.idx--;
no_pred:
	/* Successor leaf. */
	succ.leaf = rm, succ.top = UINT_MAX /* Not sure what this means? */;
	succ.leaf.idx++;
	do {
		struct pT_(bough) *const up = succ.leaf.bough;
		succ.leaf.bough = pT_(as_branch_c)(succ.leaf.bough)->child[succ.leaf.idx];
		succ.leaf.idx = 0;
		succ.leaf.height--;
		if(succ.leaf.bough->size < TREE_MIN)
			{ succ.leaf.bough = 0; goto no_succ; }
		else if(succ.leaf.bough->size > TREE_MIN) succ.top = succ.leaf.height;
		else if(succ.leaf.height)
			pT_(as_branch)(succ.leaf.bough)->child[TREE_MAX] = up;
		else succ.parent = up;
	} while(succ.leaf.height > 1);
no_succ:
	/* Choose the predecessor or successor. */
	if(!pred.leaf.bough) {
		assert(succ.leaf.bough);
		chosen = succ;
	} else if(!succ.leaf.bough) {
		assert(pred.leaf.bough);
		chosen = pred;
	} else if(pred.leaf.bough->size < succ.leaf.bough->size) {
		chosen = succ;
	} else if(pred.leaf.bough->size > succ.leaf.bough->size) {
		chosen = pred;
	} else if(pred.top > succ.top) {
		chosen = succ;
	} else {
		chosen = pred;
	}
	/* Replace `rm` with the predecessor or the successor leaf. */
	provisional_x = rm.bough->key[rm.idx]
		= chosen.leaf.bough->key[chosen.leaf.idx];
#		ifdef TREE_VALUE
	rm.bough->value[rm.idx] = chosen.leaf.bough->value[chosen.leaf.idx];
#		endif
	rm = chosen.leaf;
	if(chosen.leaf.bough->size <= TREE_MIN) parent.bough = chosen.parent;
	parent.height = 2;
	goto upward;
} upward: /* The first iteration, this will be a leaf. */
	assert(rm.bough);
	if(!parent.bough) goto space;
	assert(rm.bough->size <= TREE_MIN); /* Condition on `parent.node`. */
	/* Retrieve forgotten information about the index in parent. (This is not
	 as fast at it could be, but holding parent data in minimum keys allows it
	 to be in place, if a hack. We could go down, but new problems arise.) */
	pT_(node_lb)(&parent, provisional_x);
	parentb = pT_(as_branch)(parent.bough);
	assert(parent.idx <= parent.bough->size
		&& parentb->child[parent.idx] == rm.bough);
	/* Sibling edges. */
	sibling.less = parent.idx ? parentb->child[parent.idx - 1] : 0;
	sibling.more = parent.idx < parent.bough->size
		? parentb->child[parent.idx + 1] : 0;
	assert(sibling.less || sibling.more);
	/* It's not clear which of `{ <, <= }` would be better. */
	if((sibling.more ? sibling.more->size : 0)
		> (sibling.less ? sibling.less->size : 0)) goto balance_more;
	else goto balance_less;
balance_less: {
	const unsigned combined = rm.bough->size + sibling.less->size;
	unsigned promote, more, transfer;
	assert(parent.idx);
	if(combined < 2 * TREE_MIN + 1) goto merge_less; /* Don't have enough. */
	assert(sibling.less->size > TREE_MIN); /* Since `rm.size <= TREE_MIN`. */
	promote = (combined - 1 + 1) / 2, more = promote + 1;
	transfer = sibling.less->size - more;
	assert(transfer < TREE_MAX && rm.bough->size <= TREE_MAX - transfer);
	/* Make way for the keys from the less. */
	memmove(rm.bough->key + rm.idx + 1 + transfer, rm.bough->key + rm.idx + 1,
		sizeof *rm.bough->key * (rm.bough->size - rm.idx - 1));
	memmove(rm.bough->key + transfer + 1, rm.bough->key,
		sizeof *rm.bough->key * rm.idx);
	rm.bough->key[transfer] = parent.bough->key[parent.idx - 1];
	memcpy(rm.bough->key, sibling.less->key + more,
		sizeof *sibling.less->key * transfer);
	parent.bough->key[parent.idx - 1] = sibling.less->key[promote];
#		ifdef TREE_VALUE
	memmove(rm.bough->value + rm.idx + 1 + transfer, rm.bough->value + rm.idx + 1,
		sizeof *rm.bough->value * (rm.bough->size - rm.idx - 1));
	memmove(rm.bough->value + transfer + 1, rm.bough->value,
		sizeof *rm.bough->value * rm.idx);
	rm.bough->value[transfer] = parent.bough->value[parent.idx - 1];
	memcpy(rm.bough->value, sibling.less->value + more,
		sizeof *sibling.less->value * transfer);
	parent.bough->value[parent.idx - 1] = sibling.less->value[promote];
#		endif
	if(rm.height > 1) {
		struct pT_(branch_bough) *const lessb = pT_(as_branch)(sibling.less),
			*const rmb = pT_(as_branch)(rm.bough);
		unsigned transferb = transfer + 1;
		/* This is already moved; inefficient. */
		memmove(rmb->child + transferb, rmb->child,
			sizeof *rmb->child * (rm.bough->size + 1 - 1));
		memcpy(rmb->child, lessb->child + promote + 1,
			sizeof *lessb->child * transferb);
	}
	rm.bough->size += transfer;
	sibling.less->size = promote;
	goto end;
} balance_more: {
	const unsigned combined = rm.bough->size + sibling.more->size;
	unsigned promote;
	assert(rm.bough->size);
	if(combined < 2 * TREE_MIN + 1) goto merge_more; /* Don't have enough. */
	assert(sibling.more->size > TREE_MIN); /* Since `rm.size <= TREE_MIN`. */
	promote = (combined - 1) / 2 - rm.bough->size; /* In `more`. Could be +1. */
	assert(promote < TREE_MAX && rm.bough->size <= TREE_MAX - promote);
	/* Delete key. */
	memmove(rm.bough->key + rm.idx, rm.bough->key + rm.idx + 1,
		sizeof *rm.bough->key * (rm.bough->size - rm.idx - 1));
	/* Demote into hole. */
	rm.bough->key[rm.bough->size - 1] = parent.bough->key[parent.idx];
	/* Transfer some keys from more to child. */
	memcpy(rm.bough->key + rm.bough->size, sibling.more->key,
		sizeof *sibling.more->key * promote);
	/* Promote one key from more. */
	parent.bough->key[parent.idx] = sibling.more->key[promote];
	/* Move back in more. */
	memmove(sibling.more->key, sibling.more->key + promote + 1,
		sizeof *sibling.more->key * (sibling.more->size - promote - 1));
#		ifdef TREE_VALUE
	memmove(rm.bough->value + rm.idx, rm.bough->value + rm.idx + 1,
		sizeof *rm.bough->value * (rm.bough->size - rm.idx - 1));
	rm.bough->value[rm.bough->size - 1] = parent.bough->value[parent.idx];
	memcpy(rm.bough->value + rm.bough->size, sibling.more->value,
		sizeof *sibling.more->value * promote);
	parent.bough->value[parent.idx] = sibling.more->value[promote];
	memmove(sibling.more->value, sibling.more->value + promote + 1,
		sizeof *sibling.more->value * (sibling.more->size - promote - 1));
#		endif
	if(rm.height > 1) {
		struct pT_(branch_bough) *const moreb = pT_(as_branch)(sibling.more),
			*const rmb = pT_(as_branch)(rm.bough);
		unsigned transferb = promote + 1;
		/* This is already moved; inefficient. */
		memcpy(rmb->child + rm.bough->size, moreb->child,
			sizeof *moreb->child * transferb);
		memmove(moreb->child, moreb->child + transferb,
			sizeof *rmb->child * (moreb->base.size + 1 - transferb));
	}
	rm.bough->size += promote;
	sibling.more->size -= promote + 1;
	goto end;
} merge_less:
	assert(parent.idx && parent.idx <= parent.bough->size && parent.bough->size
		&& rm.idx < rm.bough->size && rm.bough->size == TREE_MIN
		&& sibling.less->size == TREE_MIN
		&& sibling.less->size + rm.bough->size <= TREE_MAX);
	/* There are (maybe) two spots that we can merge, this is the less. */
	parent.idx--;
	/* Bring down key from `parent` to append to `less`. */
	sibling.less->key[sibling.less->size] = parent.bough->key[parent.idx];
	/* Copy the keys, leaving out deleted. */
	memcpy(sibling.less->key + sibling.less->size + 1, rm.bough->key,
		sizeof *rm.bough->key * rm.idx);
	memcpy(sibling.less->key + sibling.less->size + 1 + rm.idx,
		rm.bough->key + rm.idx + 1,
		sizeof *rm.bough->key * (rm.bough->size - rm.idx - 1));
#		ifdef TREE_VALUE
	sibling.less->value[sibling.less->size] = parent.bough->value[parent.idx];
	memcpy(sibling.less->value + sibling.less->size + 1, rm.bough->value,
		sizeof *rm.bough->value * rm.idx);
	memcpy(sibling.less->value + sibling.less->size + 1 + rm.idx,
		rm.bough->value + rm.idx + 1,
		sizeof *rm.bough->value * (rm.bough->size - rm.idx - 1));
#		endif
	if(rm.height > 1) { /* The `parent` links will have one less. Copying twice. */
		struct pT_(branch_bough) *const lessb = pT_(as_branch)(sibling.less),
			*const rmb = pT_(as_branch)(rm.bough);
		memcpy(lessb->child + sibling.less->size + 1, rmb->child,
			sizeof *rmb->child * rm.bough->size); /* _Sic_. */
	}
	sibling.less->size += rm.bough->size;
	/* Remove references to `rm` from `parent`. The parent will have one less
	 link than key (_ie_, an equal number.) This is by design. */
	memmove(parentb->child + parent.idx + 1, parentb->child + parent.idx + 2,
		sizeof *parentb->child * (parent.bough->size - parent.idx - 1));
	/* This is the same pointer, but future-proof. */
	if(rm.height > 1) free(pT_(as_branch)(rm.bough)); else free(rm.bough);
	goto ascend;
merge_more:
	assert(parent.idx < parent.bough->size && parent.bough->size
		&& rm.idx < rm.bough->size && rm.bough->size == TREE_MIN
		&& sibling.more->size == TREE_MIN
		&& rm.bough->size + sibling.more->size <= TREE_MAX); /* Violated bulk? */
	/* Remove `rm`. */
	memmove(rm.bough->key + rm.idx, rm.bough->key + rm.idx + 1,
		sizeof *rm.bough->key * (rm.bough->size - rm.idx - 1));
	/* Bring down key from `parent` to append to `rm`. */
	rm.bough->key[rm.bough->size - 1] = parent.bough->key[parent.idx];
	/* Merge `more` into `rm`. */
	memcpy(rm.bough->key + rm.bough->size, sibling.more->key,
		sizeof *sibling.more->key * sibling.more->size);
#		ifdef TREE_VALUE
	memmove(rm.bough->value + rm.idx, rm.bough->value + rm.idx + 1,
		sizeof *rm.bough->value * (rm.bough->size - rm.idx - 1));
	rm.bough->value[rm.bough->size - 1] = parent.bough->value[parent.idx];
	memcpy(rm.bough->value + rm.bough->size, sibling.more->value,
		sizeof *sibling.more->value * sibling.more->size);
#		endif
	if(rm.height > 1) { /* The `parent` links will have one less. */
		struct pT_(branch_bough) *const rmb = pT_(as_branch)(rm.bough),
			*const moreb = pT_(as_branch)(sibling.more);
		memcpy(rmb->child + rm.bough->size, moreb->child,
			sizeof *moreb->child * (sibling.more->size + 1));
	}
	rm.bough->size += sibling.more->size;
	/* Remove references to `more` from `parent`. The parent will have one less
	 link than key (_ie_, an equal number.) This is by design. */
	memmove(parentb->child + parent.idx + 1, parentb->child + parent.idx + 2,
		sizeof *parentb->child * (parent.bough->size - parent.idx - 1));
	/* This is the same pointer, but future-proof. */
	if(rm.height > 1) free(pT_(as_branch)(sibling.more)); else free(sibling.more);
	goto ascend;
ascend:
	/* Fix the hole by moving it up the tree. */
	rm = parent;
	if(rm.bough->size <= TREE_MIN) {
		if(!(parent.bough = pT_(as_branch)(rm.bough)->child[TREE_MAX])) {
			assert(tree->height == rm.height);
		} else {
			parent.height++;
		}
	} else {
		parent.bough = 0;
	}
	goto upward;
space: /* Node is root or has more than `TREE_MIN`; branches taken care of. */
	assert(rm.bough);
	assert(rm.idx < rm.bough->size);
	assert(rm.bough->size > TREE_MIN || rm.bough == tree->bough);
	memmove(rm.bough->key + rm.idx, rm.bough->key + rm.idx + 1,
		sizeof *rm.bough->key * (rm.bough->size - rm.idx - 1));
#		ifdef TREE_VALUE
	memmove(rm.bough->value + rm.idx, rm.bough->value + rm.idx + 1,
		sizeof *rm.bough->value * (rm.bough->size - rm.idx - 1));
#		endif
	if(!--rm.bough->size) {
		assert(rm.bough == tree->bough);
		if(tree->height /**/>1) {
			tree->bough = pT_(as_branch)(rm.bough)->child[0];
			tree->height--;
			free(pT_(as_branch)(rm.bough));
		} else { /* Just deleted the last one. */
			tree->height = 0;
		}
	}
	goto end;
end:
	return 1;
}

/** Private: frees non-empty `tree` and it's children recursively, but doesn't
 put it to idle or clear pointers.
 @param[keep] Keep one leaf if non-null. Set to null before. */
static void pT_(clear_r)(struct pT_(tree) tree, struct pT_(bough) **const keep) {
	assert(tree.bough && tree.height);
	if(tree.height > 1) {
		if(keep && !*keep) *keep = tree.bough;
		else free(tree.bough);
	} else {
		struct pT_(tree) child;
		unsigned i;
		child.height = tree.height - 1;
		for(i = 0; i <= tree.bough->size; i++)
			child.bough = pT_(as_branch)(tree.bough)->child[i],
			pT_(clear_r)(child, keep);
		free(pT_(as_branch)(tree.bough));
	}
}
/** Private: `tree` can be null. */
static void pT_(clear)(struct t_(tree) *tree) {
	struct pT_(bough) *one = 0;
	/* Already not there/idle/empty. */
	if(!tree || !tree->trunk.bough || !tree->trunk.height) return;
	pT_(clear_r)(tree->trunk, &one), assert(one);
	/* This is a special state where the tree has one leaf, but it is empty.
	 This state exists because it gives hysteresis to 0–1 transition because
	 we have no advanced memory management. */
	tree->trunk.bough = one;
	tree->trunk.height = 0;
}

/** Private: counts a sub-tree, `tree`. */
static size_t pT_(count_r)(const struct pT_(tree) tree) {
	size_t c = tree.bough->size;
	if(tree.height > 1) {
		const struct pT_(branch_bough) *const branch = pT_(as_branch)(tree.bough);
		struct pT_(tree) sub;
		size_t i;
		sub.height = tree.height - 1;
		for(i = 0; i <= tree.bough->size; i++) {
			sub.bough = branch->child[i];
			c += pT_(count_r)(sub);
		}
	}
	return c;
}

/* All these are used in clone; it's convenient to use `\O(\log size)` stack
 space. [existing branches][new branches][existing leaves][new leaves] no */
struct pT_(scaffold) {
	struct tree_node_count victim, source;
	size_t no;
	struct pT_(bough) **data;
	struct { struct pT_(bough) **head, **fresh, **iterator; } branch, leaf;
};
/** Counts the nodes `no` in `tree` for <fn:<pT>nodes>. */
static int pT_(nodes_r)(struct pT_(tree) tree,
	struct tree_node_count *const no) {
	assert(tree.bough && tree.height > 1);
	if(!++no->branches) return 0;
	if(tree.height == 2) {
		/* Overflow; aren't guaranteed against this. */
		if(no->leaves + tree.bough->size + 1 < no->leaves) return 0;
		no->leaves += tree.bough->size + 1;
	} else {
		unsigned i;
		for(i = 0; i <= tree.bough->size; i++) {
			struct pT_(tree) child;
			child.bough = pT_(as_branch)(tree.bough)->child[i];
			child.height = tree.height - 1;
			if(!pT_(nodes_r)(child, no)) return 0;
		}
	}
	return 1;
}
/** Counts the nodes `no` in `tree`. */
static int pT_(nodes)(const struct t_(tree) *const tree,
	struct tree_node_count *const no) {
	assert(tree && no);
	no->branches = no->leaves = 0;
	if(!tree->trunk.bough) { /* Idle. */
	} else if(tree->trunk.height <= 1) {
		no->leaves = 1;
	} else { /* Complex. */
		struct pT_(tree) sub = tree->trunk;
		if(!pT_(nodes_r)(sub, no)) return 0;
	}
	return 1;
}

////////// FIXME
#include "orcish.h"
#		ifdef TREE_TO_STRING
static void pT_(graph)(const struct pT_(tree) *, FILE *);
#		endif

#		ifdef TREE_VALUE /* <!-- map */
/** Adds or updates `key` in `root`. If not-null, `eject` will be the replaced
 key, otherwise don't replace. If `value` is not-null, sticks the associated
 value. */
static enum tree_result pT_(update)(struct pT_(tree) *const trunk,
	pT_(key) key, pT_(key) *const eject, pT_(value) **const value) {
#		else /* map --><!-- set */
static enum tree_result pT_(update)(struct pT_(tree) *const trunk,
	pT_(key) key, pT_(key) *const eject) {
#		endif /* set --> */
	/* <https://github.com/neil-edelman/boxes/blob/master/doc/tree/tree.pdf>.
	 Figure 2. */
	struct pT_(bough) *new_head = 0;
	struct pT_(ref) add, hole, cur;
	assert(trunk);
#		ifdef TREE_TO_STRING
	{
		FILE *fp = fopen("graph/tree/work1.gv", "w");
		pT_(graph)(trunk, fp);
		fclose(fp);
	}
#		endif
	if(!(add.bough = trunk->bough)) goto idle;
	else if(!trunk->height) goto empty;
	goto descend;
idle: /* No reserved memory; reserve memory. */
	assert(!add.bough && !trunk->height);
	if(!(add.bough = malloc(sizeof *add.bough))) goto catch;
	trunk->bough = add.bough;
	trunk->height = 0;
	goto empty;
empty: /* Tree is empty with one bought. */
	assert(add.bough && !trunk->height);
	add.height = trunk->height = 1;
	add.bough->size = 0;
	add.idx = 0;
	goto insert;
descend: /* Record last node that has space. */
	{
		int is_equal = 0;
		add = pT_(lookup_hole)(*trunk, key, &hole, &is_equal);
		if(is_equal) {
			if(eject) {
				*eject = add.bough->key[add.idx];
				add.bough->key[add.idx] = key;
			}
#		ifdef TREE_VALUE
			if(value) *value = pT_(ref_to_valuep)(add);
#		endif
			return TREE_PRESENT;
		}
	}
	//printf("hole %s; add %s\n", orcify(hole.bough), orcify(add.bough));
	if(hole.bough == add.bough) goto insert; else goto grow;
insert: /* Leaf has space to spare; usually end up here. */
	assert(add.bough && add.idx <= add.bough->size && add.bough->size < TREE_MAX);
	memmove(add.bough->key + add.idx + 1, add.bough->key + add.idx,
		sizeof *add.bough->key * (add.bough->size - add.idx));
#		ifdef TREE_VALUE
	memmove(add.bough->value + add.idx + 1, add.bough->value + add.idx,
		sizeof *add.bough->value * (add.bough->size - add.idx));
#		endif
	add.bough->size++;
	add.bough->key[add.idx] = key;
#		ifdef TREE_VALUE
	if(value) *value = pT_(ref_to_valuep)(add);
#		endif
	return TREE_ABSENT;
grow: /* Leaf is full. */ {
	unsigned new_no = hole.bough ? hole.height - 1: trunk->height + 1;
	struct pT_(bough) **new_next = &new_head, *new_leaf;
	struct pT_(branch_bough) *new_branch;
	assert(new_no);
	while(new_no != 1) { /* Branch-boughs and one leaf-bough. */
		if(!(new_branch = malloc(sizeof *new_branch))) goto catch;
		new_branch->base.size = 0;
		new_branch->child[0] = 0;
		*new_next = &new_branch->base, new_next = new_branch->child;
		new_no--;
	}
	if(!(new_leaf = malloc(sizeof *new_leaf))) goto catch;
	new_leaf->size = 0;
	*new_next = new_leaf;
	if(hole.bough) { /* New nodes are a sub-structure of the tree. */
		struct pT_(branch_bough) *holeb = pT_(as_branch)(hole.bough);
		memmove(hole.bough->key + hole.idx + 1, hole.bough->key + hole.idx,
			sizeof *hole.bough->key * (hole.bough->size - hole.idx));
#		ifdef TREE_VALUE
		memmove(hole.bough->value + hole.idx + 1, hole.bough->value + hole.idx,
			sizeof *hole.bough->value * (hole.bough->size - hole.idx));
#		endif
		memmove(holeb->child + hole.idx + 2, holeb->child + hole.idx + 1,
			sizeof *holeb->child * (hole.bough->size - hole.idx));
		holeb->child[hole.idx + 1] = new_head;
		hole.bough->size++;
	} else { /* New nodes raise tree height. */
		struct pT_(branch_bough) *const new_root = pT_(as_branch)(new_head);
		hole.bough = new_head, hole.height = ++trunk->height, hole.idx = 0;
		new_head = new_root->child[1] = new_root->child[0];
		new_root->child[0] = trunk->bough, trunk->bough = hole.bough;
		hole.bough->size = 1;
	}
	cur = hole; /* Go down; (as opposed to doing it on paper.) */
	goto split;
} split: { /* Split between the new and existing nodes. */
	struct pT_(bough) *sibling;
	sibling = new_head;
#		ifdef TREE_TO_STRING
	{
		FILE *fp = fopen("graph/tree/work2.gv", "w");
		pT_(graph)(trunk, fp);
		fclose(fp);
	}
#		endif
	assert(cur.bough && cur.bough->size && cur.height);
	/* Easier to descend now while split hasn't happened. */
	new_head = --cur.height > 1 ? pT_(as_branch)(new_head)->child[0] : 0;
	cur.bough = pT_(as_branch)(cur.bough)->child[cur.idx];
	pT_(node_lb)(&cur, key);
	//printf("sibling is %s, cur is %s.\n", orcify(sibling), orcify(cur.bough));
	assert(!sibling->size && cur.bough->size == TREE_MAX);
	/* Expand `cur`, which is full, to multiple nodes. */
	if(cur.idx < TREE_SPLIT) { /* Descend hole to `cur`. */
		memcpy(sibling->key, cur.bough->key + TREE_SPLIT,
			sizeof *sibling->key * (TREE_MAX - TREE_SPLIT));
#		ifdef TREE_VALUE
		memcpy(sibling->value, cur.bough->value + TREE_SPLIT,
			sizeof *sibling->value * (TREE_MAX - TREE_SPLIT));
#		endif
		hole.bough->key[hole.idx] = cur.bough->key[TREE_SPLIT - 1];
#		ifdef TREE_VALUE
		hole.bough->value[hole.idx] = cur.bough->value[TREE_SPLIT - 1];
#		endif
		memmove(cur.bough->key + cur.idx + 1,
			cur.bough->key + cur.idx,
			sizeof *cur.bough->key * (TREE_SPLIT - 1 - cur.idx));
#		ifdef TREE_VALUE
		memmove(cur.bough->value + cur.idx + 1,
			cur.bough->value + cur.idx,
			sizeof *cur.bough->value * (TREE_SPLIT - 1 - cur.idx));
#		endif
		if(cur.height > 1) {
			struct pT_(branch_bough) *const cb = pT_(as_branch)(cur.bough),
				*const sb = pT_(as_branch)(sibling);
			struct pT_(bough) *temp = sb->child[0];
			memcpy(sb->child, cb->child + TREE_SPLIT,
				sizeof *cb->child * (TREE_MAX - TREE_SPLIT + 1));
			memmove(cb->child + cur.idx + 2, cb->child + cur.idx + 1,
				sizeof *cb->child * (TREE_SPLIT - 1 - cur.idx));
			cb->child[cur.idx + 1] = temp;
		}
		hole = cur;
	} else if(cur.idx > TREE_SPLIT) { /* Descend hole to `sibling`. */
		hole.bough->key[hole.idx] = cur.bough->key[TREE_SPLIT];
#		ifdef TREE_VALUE
		hole.bough->value[hole.idx] = cur.bough->value[TREE_SPLIT];
#		endif
		hole.bough = sibling, hole.height = cur.height,
			hole.idx = cur.idx - TREE_SPLIT - 1;
		memcpy(sibling->key, cur.bough->key + TREE_SPLIT + 1,
			sizeof *sibling->key * hole.idx);
		memcpy(sibling->key + hole.idx + 1, cur.bough->key + cur.idx,
			sizeof *sibling->key * (TREE_MAX - cur.idx));
#		ifdef TREE_VALUE
		memcpy(sibling->value, cur.bough->value + TREE_SPLIT + 1,
			sizeof *sibling->value * hole.idx);
		memcpy(sibling->value + hole.idx + 1, cur.bough->value
			+ cur.idx, sizeof *sibling->value * (TREE_MAX - cur.idx));
#		endif
		if(cur.height > 1) {
			struct pT_(branch_bough) *const cb = pT_(as_branch)(cur.bough),
				*const sb = pT_(as_branch)(sibling);
			struct pT_(bough) *temp = sb->child[0];
			memcpy(sb->child, cb->child + TREE_SPLIT + 1,
				sizeof *cb->child * (hole.idx + 1));
			memcpy(sb->child + hole.idx + 2, cb->child + cur.idx + 1,
				sizeof *cb->child * (TREE_MAX - cur.idx));
			sb->child[hole.idx + 1] = temp;
		}
	} else { /* Equal split: leave the hole where it is. */
		memcpy(sibling->key, cur.bough->key + TREE_SPLIT,
			sizeof *sibling->key * (TREE_MAX - TREE_SPLIT));
#		ifdef TREE_VALUE
		memcpy(sibling->value, cur.bough->value + TREE_SPLIT,
			sizeof *sibling->value * (TREE_MAX - TREE_SPLIT));
#		endif
		if(cur.height > 1) {
			struct pT_(branch_bough) *const cb = pT_(as_branch)(cur.bough),
				*const sb = pT_(as_branch)(sibling);
			memcpy(sb->child + 1, cb->child + TREE_SPLIT + 1,
				sizeof *cb->child * (TREE_MAX - TREE_SPLIT));
		}
	}
	/* Divide `TREE_MAX + 1` into two trees. */
	cur.bough->size = TREE_SPLIT, sibling->size = TREE_MAX - TREE_SPLIT;
	if(cur.height > 1) goto split; /* Loop max `\log_{TREE_MIN} size`. */
	hole.bough->key[hole.idx] = key;
#		ifdef TREE_VALUE
	if(value) *value = pT_(ref_to_valuep)(hole);
#		endif
#		ifdef TREE_TO_STRING
	{
		FILE *fp = fopen("graph/tree/work3.gv", "w");
		pT_(graph)(trunk, fp);
		fclose(fp);
	}
#		endif
	assert(!new_head);
	return TREE_ABSENT;
} catch: /* Didn't work. Reset. */
	while(new_head) {
		struct pT_(branch_bough) *const top = pT_(as_branch)(new_head);
		new_head = top->child[0];
		free(top);
	}
	if(!errno) errno = ERANGE; /* Non-POSIX OSs not mandated to set errno. */
	return TREE_ERROR;
#		ifdef TREE_VALUE /* Code editor is confused; leave in. */
}
#		else
}
#		endif
/** `ref` with `sc` work under <fn:<pT>cannibalize>. */
static void pT_(cannibalize_r)(struct pT_(ref) ref,
	struct pT_(scaffold) *const sc) {
	struct pT_(branch_bough) *branch = pT_(as_branch)(ref.bough);
	const int keep_branch = sc->branch.iterator < sc->branch.fresh;
	assert(ref.bough && ref.height > 1 && sc);
	if(keep_branch) *sc->branch.iterator = ref.bough, sc->branch.iterator++;
	if(ref.height == 2) { /* Children are leaves. */
		unsigned n;
		for(n = 0; n <= ref.bough->size; n++) {
			const int keep_leaf = sc->leaf.iterator < sc->leaf.fresh;
			struct pT_(bough) *child = branch->child[n];
			if(keep_leaf) *sc->leaf.iterator = child, sc->leaf.iterator++;
			else free(child);
		}
	} else while(ref.idx <= ref.bough->size) {
		struct pT_(ref) child;
		child.bough = pT_(as_branch)(ref.bough)->child[ref.idx];
		child.height = ref.height - 1;
		child.idx = 0;
		pT_(cannibalize_r)(child, sc);
		ref.idx++;
	}
	if(!keep_branch) free(branch);
}
/** Disassemble `tree` and put in into `sc`. */
static void pT_(cannibalize)(const struct t_(tree) *const tree,
	struct pT_(scaffold) *const sc) {
	struct pT_(ref) ref;
	assert(tree && tree->trunk.height && sc);
	/* Nothing to cannibalize. */
	if(!sc->victim.branches && !sc->victim.leaves) return;
	assert(tree->trunk.bough);
	ref.bough = tree->trunk.bough, ref.height = tree->trunk.height, ref.idx = 0;
	sc->branch.iterator = sc->branch.head;
	sc->leaf.iterator = sc->leaf.head;
	if(ref.height > 1) {
		pT_(cannibalize_r)(ref, sc);
	} else { /* Just one leaf. */
		*sc->leaf.iterator = ref.bough;
	}
}
/** Do the work of `src` cloned with `sc`. Called from <fn:<pT>clone>. */
static struct pT_(bough) *pT_(clone_r)(struct pT_(tree) src,
	struct pT_(scaffold) *const sc) {
	struct pT_(bough) *node;
	if(src.height > 1) {
		struct pT_(branch_bough) *const srcb = pT_(as_branch)(src.bough),
			*const branch = pT_(as_branch)(node = *sc->branch.iterator++);
		unsigned i;
		struct pT_(tree) child;
		*node = *src.bough; /* Copy node. */
		child.height = src.height - 1;
		for(i = 0; i <= src.bough->size; i++) { /* Different links. */
			child.bough = srcb->child[i];
			branch->child[i] = pT_(clone_r)(child, sc);
		}
	} else { /* Leaves. */
		node = *sc->leaf.iterator++;
		*node = *src.bough;
	}
	return node;
}
/** `src` is copied with the cloning scaffold `sc`. */
static struct pT_(tree) pT_(clone)(const struct pT_(tree) *const src,
	struct pT_(scaffold) *const sc) {
	struct pT_(tree) sub;
	assert(src && src->bough && sc);
	/* Go back to the beginning of the scaffold and pick off one by one. */
	sc->branch.iterator = sc->branch.head;
	sc->leaf.iterator = sc->leaf.head;
	sub.bough = pT_(clone_r)(*src, sc);
	sub.height = src->height;
	/* Used up all of them. No concurrent modifications, please. */
	assert(sc->branch.iterator == sc->leaf.head
		&& sc->leaf.iterator == sc->data + sc->no);
	return sub;
}

#		define BOX_PUBLIC_OVERRIDE
#		include "box.h"

/** Iterator for `tree` in empty state. @allow */
static struct T_(cursor) T_(begin)(const struct t_(tree) *const tree) {
	union { const struct t_(tree) *readonly; struct t_(tree) *promise; } sly;
	struct T_(cursor) cur;
	assert(tree);
	cur.trunk = &(sly.readonly = tree, sly.promise)->trunk;
	cur.ref.bough = 0, cur.ref.height = 0, cur.ref.idx = 0;
	return cur;
}
/** @return Whether the `cur` points to an element. @allow */
static int T_(exists)(struct T_(cursor) *const cur) {
	assert(cur);
	if(!cur->trunk || !cur->trunk->height) return 0;
	assert(cur->trunk->bough);
	/* Iterator empty; tree non-empty; fall to first data. */
	if(!cur->ref.bough) {
		cur->ref.height = cur->trunk->height;
		for(cur->ref.bough = cur->trunk->bough; cur->ref.height > 1;
			cur->ref.bough = pT_(as_branch_c)(cur->ref.bough)->child[0],
			cur->ref.height--);
		cur->ref.idx = 0;
	}
	return 1;
}
/** @return Dereference the element pointed to by `cur` that <fn:<T>exists>. */
static struct pT_(ref) *T_(entry)(struct T_(cursor) *const cur) {
	return &cur->ref;
}
/** @return Extract the key from `cur` that <fn:<T>exists>. @allow */
static pT_(key) T_(key)(const struct T_(cursor) *const cur)
	{ return cur->ref.bough->key[cur->ref.idx]; }
#		ifdef TREE_VALUE
/** @return Extract the value from `cur` that <fn:<T>exists>, if `TREE_VALUE`.
 @allow */
static pT_(value) *T_(value)(const struct T_(cursor) *const cur)
	{ return cur->ref.bough->value + cur->ref.idx; }
#		endif
/** Move `cur` that <fn:<T>exists> to the next element. @allow */
static void T_(next)(struct T_(cursor) *const cur) {
	struct pT_(ref) next;
	assert(cur && cur->trunk && cur->trunk->bough
		&& cur->trunk->height && cur->ref.bough && cur->ref.bough->size);
	/* Next is a copy of the next element. */
	next = cur->ref, next.idx++, assert(next.idx <= next.bough->size);
	if(next.height > 1) { /* Fall from branch. */
		do next.bough = pT_(as_branch)(next.bough)->child[next.idx],
			next.idx = 0, next.height--; while(next.height > 1);
	} else if(next.idx >= next.bough->size) { /* Re-descend. */
		struct pT_(tree) descend = *cur->trunk;
		unsigned a0;
		/* Target; this will not work with duplicate keys. */
		const pT_(key) x = next.bough->key[next.bough->size - 1];
		assert(next.bough->size);
		for(next.bough = 0; descend.height > 1; descend.bough
			= pT_(as_branch)(descend.bough)->child[a0], descend.height--) {
			unsigned a1 = descend.bough->size;
			a0 = 0;
			while(a0 < a1) {
				const unsigned m = (a0 + a1) / 2;
				/* <t>less must be declared. */
				if(t_(less)(x, descend.bough->key[m]) > 0) a0 = m + 1;
				else a1 = m;
			}
			if(a0 < descend.bough->size) next.bough = descend.bough,
				next.height = descend.height, next.idx = a0;
		}
		if(!next.bough) /* Off right. */
			{ cur->ref.bough = 0, cur->trunk = 0; return; }
	} /* Jumped nodes. */
	cur->ref = next;
}
/** Move `cur` that <fn:<T>exists> to the previous element. @allow */
static void T_(previous)(struct T_(cursor) *const cur) {
	struct pT_(ref) prd;
	assert(cur && cur->trunk);

	/* Tree empty. */
	if(!cur->trunk || !cur->trunk->bough || !cur->trunk->height) return;

	/* Iterator empty; tree non-empty; point at last. */
	if(!cur->ref.bough) {
		cur->ref.height = cur->trunk->height;
		for(cur->ref.bough = cur->trunk->bough; cur->ref.height > 1;
			cur->ref.bough
			= pT_(as_branch)(cur->ref.bough)->child[cur->ref.bough->size],
			cur->ref.height--);
		/* Did you forget <fn:<T>bulk_load_finish>? */
		if(!cur->ref.bough->size) { cur->ref.bough = 0; return; }
		cur->ref.idx = cur->ref.bough->size - 1;
	}

	/* Predecessor? Clip. */
	prd = cur->ref;
	if(prd.height /**/>1 && prd.idx > prd.bough->size) prd.idx = prd.bough->size;
	while(prd.height) prd.height--,
		prd.bough = pT_(as_branch)(prd.bough)->child[prd.idx],
		prd.idx = prd.bough->size;
	if(prd.idx) {
		prd.idx--;
	} else { /* Maybe re-descend reveals more keys. */
		struct pT_(tree) tree = *cur->trunk;
		unsigned a0;
		const pT_(key) x = prd.bough->key[0]; /* Target. */
		for(prd.bough = 0; tree.height > 1;
			tree.bough = pT_(as_branch)(tree.bough)->child[a0], tree.height--) {
			unsigned a1 = tree.bough->size;
			a0 = 0;
			while(a0 < a1) {
				const unsigned m = (a0 + a1) / 2;
				if(t_(less)(x, tree.bough->key[m]) > 0) a0 = m + 1;
				else a1 = m;
			}
			if(a0) prd.bough = tree.bough, prd.height = tree.height,
				prd.idx = a0 - 1;
		}
		if(!prd.bough) { cur->ref.bough = 0; return; } /* Off left. */
	} /* Jumped nodes. */
	cur->ref = prd;
}

/** @return Cursor in `tree` such that <fn:<T>key> is the greatest key that is
 less-than-or-equal-to `x`, or if `x` is less than all in `tree`,
 <fn:<T>begin>. @order \Theta(\log |`tree`|) @fixme Update. @allow */
static struct T_(cursor) T_(less)(struct t_(tree) *const tree,
	const pT_(key) x) {
	struct T_(cursor) cur;
	assert(tree);
	if(!(cur.trunk = &tree->trunk)) return cur;
	/* This ensures that it doesn't start again. */
	if(!(cur.ref = pT_(less)(tree->trunk, x)).bough) cur.trunk = 0;
	return cur;
}

/** @return Cursor in `tree` such that <fn:<T>more> is the smallest key that is
 greater-than-or-equal-to `x`, or, (…) if `x` is greater than all in
 `tree`. @order \Theta(\log |`tree`|) @fixme Update. @allow */
static struct T_(cursor) T_(more)(struct t_(tree) *const tree,
	const pT_(key) x) {
	struct T_(cursor) cur;
	assert(tree);
	if(!(cur.trunk = &tree->trunk)) return cur;
	/* This ensures that it doesn't start again. */
	if(!(cur.ref = pT_(more)(tree->trunk, x)).bough) cur.trunk = 0;
	return cur;
}

/** Zeroed data (not all-bits-zero) is initialized. @return An idle tree.
 @order \Theta(1) @allow */
static struct t_(tree) t_(tree)(void) {
	struct t_(tree) tree;
	tree.trunk.bough = 0; tree.trunk.height = 0;
	return tree;
}

/** Returns an initialized `tree` to idle, `tree` can be null.
 @order \O(|`tree`|) @allow */
static void t_(tree_)(struct t_(tree) *const tree) {
	if(!tree) return; /* Null. */
	if(!tree->trunk.bough) { /* Idle. */
		assert(!tree->trunk.height);
	} else if(!tree->trunk.height) { /* Empty with space. */
		free(tree->trunk.bough);
	} else {
		pT_(clear_r)(tree->trunk, 0);
	}
	*tree = t_(tree)();
}

/** Clears `tree`, which can be null, idle, empty, or full. If it is empty or
 full, it remains active, (all except one node are freed.)
 @order \O(|`tree`|) @allow */
static void T_(clear)(struct t_(tree) *const tree) { pT_(clear)(tree); }

/** Counts all the keys on `tree`, which can be null.
 @order \O(|`tree`|) @allow */
static size_t T_(count)(const struct t_(tree) *const tree)
	{ return tree && tree->trunk.height ? pT_(count_r)(tree->trunk) : 0; }

/** @return Is `x` in `tree` (which can be null)?
 @order \O(\log |`tree`|) @allow */
static int T_(contains)(const struct t_(tree) *const tree, const pT_(key) x)
	{ return tree && pT_(lookup_find)(tree->trunk, x).bough; }
/* fixme: entry <T>query—there is no functionality that returns the
 key, which might be important with distinguishable keys. */

/** @return Get the value of `key` in `tree`, or if no key, `default_value`.
 The map type is `TREE_VALUE` and the set type is `TREE_KEY`.
 @order \O(\log |`tree`|) @allow */
static pT_(value) T_(get_or)(const struct t_(tree) *const tree,
	const pT_(key) key, const pT_(value) default_value) {
	struct pT_(ref) ref;
	return tree && tree->trunk.bough && tree->trunk.height
		&& (ref = pT_(lookup_find)(tree->trunk, key)).bough
		? *pT_(ref_to_valuep)(ref) : default_value;
}

/** For example, `tree = { 10 }`, `x = 5 -> default_value`, `x = 10 -> 10`,
 `x = 11 -> 10`.
 @return Key in `tree` less-then-or-equal to `x` or `default_key` if `x` is
 smaller than all in `tree`. @order \O(\log |`tree`|) @allow */
static pT_(key) T_(less_or)(const struct t_(tree) *const tree,
	const pT_(key) x, const pT_(key) default_key) {
	struct pT_(ref) ref;
	return tree && (ref = pT_(less)(tree->trunk, x)).bough ?
		(assert(ref.idx < ref.bough->size), ref.bough->key[ref.idx])
		: default_key;
}

/** For example, `tree = { 10 }`, `x = 5 -> 10`, `x = 10 -> 10`,
 `x = 11 -> default_value`.
 @return Key in `tree` greater-than-or-equal to `x` or `default_key` if `x` is
 greater than all in `tree`.
 @order \O(\log |`tree`|) @allow */
static pT_(key) T_(more_or)(const struct t_(tree) *const tree,
	const pT_(key) x, const pT_(key) default_key) {
	struct pT_(ref) ref;
	return tree && (ref = pT_(more)(tree->trunk, x)).bough
		? ref.bough->key[ref.idx] : default_key;
}

#		ifdef TREE_VALUE /* <!-- map */
/** Only if `TREE_VALUE` is set; the set version is <fn:<T>add>. Packs `key` on
 the right side of `tree` without doing the usual restructuring. All other
 topology modification functions should be avoided until followed by
 <fn:<T>bulk_finish>.
 @param[value] A pointer to the key's value which is set by the function on
 returning true. Can be null.
 @return One of <tag:tree_result>: `TREE_ERROR` and `errno` will be set,
 `TREE_PRESENT` if the key is already (the highest) in the tree, and
 `TREE_ABSENT`, added, the `value` (if applicable) is uninitialized.
 @throws[EDOM] `x` is smaller than the largest key in `tree`. @throws[malloc]
 @order \O(\log |`tree`|) @allow */
static enum tree_result T_(bulk_assign)(struct t_(tree) *const tree,
	pT_(key) key, pT_(value) **const value) {
#		elif defined TREE_VALUE /* map --><!-- null: For braces matching. */
}
#		else /* null --><!-- set */
/** Only if `TREE_VALUE` is not set; see <fn:<T>assign>, which is the map
 version. Packs `key` on the right side of `tree`. @allow */
static enum tree_result T_(bulk_add)(struct t_(tree) *const tree, pT_(key) key)
{
#		endif
	struct pT_(bough) *bough = 0, *head = 0; /* The original and new. */
	assert(tree);
	if(!tree->trunk.bough) { /* Idle tree. */
		assert(!tree->trunk.height);
		if(!(bough = malloc(sizeof *bough))) goto catch;
		bough->size = 0;
		tree->trunk.height = 1;
		tree->trunk.bough = bough;
	} else if(!tree->trunk.height) { /* Empty tree. */
		bough = tree->trunk.bough;
		tree->trunk.height = 1;
		bough->size = 0;
	} else {
		struct pT_(tree) unfull = { 0, 0 };
		unsigned new_nodes, n; /* Count new nodes. */
		struct pT_(bough) *tail = 0, *last = 0;
		struct pT_(branch_bough) *pretail = 0;
		struct pT_(tree) scout;
		pT_(key) max;
		/* Right side bottom: `last` node with any keys, `unfull` not full. */
		for(scout = tree->trunk; ; scout.bough = pT_(as_branch)(scout.bough)
			->child[scout.bough->size], scout.height--) {
			if(scout.bough->size < TREE_MAX) unfull = scout;
			if(scout.bough->size) last = scout.bough;
			if(scout.height <= 1) break;
		}
		assert(last), max = last->key[last->size - 1];
		if(t_(less)(max, key) > 0) return errno = EDOM, TREE_ERROR;
		if(t_(less)(key, max) <= 0) {
#		ifdef TREE_VALUE
			if(value) {
				struct pT_(ref) max_ref;
				max_ref.bough = last, max_ref.idx = last->size - 1;
				*value = pT_(ref_to_valuep)(max_ref);
			}
#		endif
			return TREE_PRESENT;
		}

		/* One leaf, and the rest branches. */
		new_nodes = n = unfull.bough ? unfull.height : tree->trunk.height + 2;
		if(!n) {
			bough = unfull.bough;
		} else {
			if(!(bough = tail = malloc(sizeof *tail))) goto catch;
			tail->size = 0;
			while(--n) {
				struct pT_(branch_bough) *b;
				if(!(b = malloc(sizeof *b))) goto catch;
				b->base.size = 0;
				if(!head) b->child[0] = 0, pretail = b; /* First loop. */
				else b->child[0] = head; /* Not first loop. */
				head = &b->base;
			}
		}

		/* Post-error; modify the original as needed. */
		if(pretail) pretail->child[0] = tail;
		else head = bough;
		if(!unfull.bough) { /* Add tree to head. */
			struct pT_(branch_bough) *const branch = pT_(as_branch)(head);
			assert(new_nodes > 1);
			branch->child[1] = branch->child[0];
			branch->child[0] = tree->trunk.bough;
			bough = tree->trunk.bough = head, tree->trunk.height++;
		} else if(unfull.height) { /* Add head to tree. */
			struct pT_(branch_bough) *const branch
				= pT_(as_branch)(bough = unfull.bough);
			assert(new_nodes);
			branch->child[branch->base.size + 1] = head;
		}
	}
	assert(bough && bough->size < TREE_MAX);
	bough->key[bough->size] = key;
#		ifdef TREE_VALUE
	if(value) {
		struct pT_(ref) max_ref;
		max_ref.bough = bough, max_ref.idx = bough->size;
		*value = pT_(ref_to_valuep)(max_ref);
	}
#		endif
	bough->size++;
	return TREE_ABSENT;
catch: /* Didn't work. Reset. */
	free(bough);
	while(head) {
		struct pT_(bough) *const next = pT_(as_branch)(head)->child[0];
		free(head);
		head = next;
	}
	if(!errno) errno = ERANGE;
	return TREE_ERROR;
}

/** Distributes `tree` (can be null) on the right side so that, after a series
 of <fn:<T>bulk_add> or <fn:<T>bulk_assign>, it will be consistent with the
 minimum number of keys in a node.
 @return The re-distribution was a success and all nodes are within rules.
 (Only when intermixing bulk and regular operations, can the function return
 false.) @order \O(\log |`tree`|) @allow */
static int T_(bulk_finish)(struct t_(tree) *const tree) {
	struct pT_(tree) s;
	struct pT_(bough) *right;
	if(!tree || !tree->trunk.bough || !tree->trunk.height) return 1;
	for(s = tree->trunk; s.height/**/>1; s.bough = right, s.height--) {
		unsigned distribute, right_want, right_move, take_sibling;
		struct pT_(branch_bough) *parent = pT_(as_branch)(s.bough);
		struct pT_(bough) *sibling = (assert(parent->base.size),
			parent->child[parent->base.size - 1]);
		right = parent->child[parent->base.size];
		/* Should this be increased to max/2 instead of max/3 to make a more
		 balanced tree? Otoh, why? */
		if(TREE_MIN <= right->size) continue; /* Has enough. */
		distribute = sibling->size + right->size;
		/* Should have at least `TREE_MAX` on left. */
		if(distribute < 2 * TREE_MIN) return 0;
		right_want = distribute / 2;
		right_move = right_want - right->size;
		take_sibling = right_move - 1;
		/* Either the right has met the properties of a B-tree node, (covered
		 above,) or the left sibling is full from bulk-loading (relaxed.) */
		assert(right->size < right_want && right_want >= TREE_MIN
			&& sibling->size - take_sibling >= TREE_MIN + 1);
		/* Move the right node to accept more keys. */
		memmove(right->key + right_move, right->key,
			sizeof *right->key * right->size);
#		ifdef TREE_VALUE
		memmove(right->value + right_move, right->value,
			sizeof *right->value * right->size);
#		endif
		if(s.height > 2) { /* (Parent height.) */
			struct pT_(branch_bough) *rbranch = pT_(as_branch)(right),
				*sbranch = pT_(as_branch)(sibling);
			memmove(rbranch->child + right_move, rbranch->child,
				sizeof *rbranch->child * (right->size + 1));
			memcpy(rbranch->child, sbranch->child + sibling->size + 1
				- right_move, sizeof *sbranch->child * right_move);
		}
		right->size += right_move;
		/* Move one node from the parent. */
		memcpy(right->key + take_sibling,
			parent->base.key + parent->base.size - 1, sizeof *right->key);
#		ifdef TREE_VALUE
		memcpy(right->value + take_sibling,
			parent->base.value + parent->base.size - 1, sizeof *right->value);
#		endif
		/* Move the others from the sibling. */
		memcpy(right->key, sibling->key + sibling->size - take_sibling,
			sizeof *right->key * take_sibling);
#		ifdef TREE_VALUE
		memcpy(right->value, sibling->value + sibling->size - take_sibling,
			sizeof *right->value * take_sibling);
#		endif
		sibling->size -= take_sibling;
		/* Sibling's key is now the parent's. */
		memcpy(parent->base.key + parent->base.size - 1,
			sibling->key + sibling->size - 1, sizeof *right->key);
#		ifdef TREE_VALUE
		memcpy(parent->base.value + parent->base.size - 1,
			sibling->value + sibling->size - 1, sizeof *right->value);
#		endif
		sibling->size--;
	}
	return 1;
}

#		ifdef TREE_VALUE /* <!-- map */
/** Adds or gets `key` in `tree`. If `key` is already in `tree`, uses the
 old value, _vs_ <fn:<T>update>. (This is only significant in trees with
 distinguishable keys.)
 @param[valuep] Only present if `TREE_VALUE` (map) was specified. If this
 parameter is non-null and a return value other then `TREE_ERROR`, this
 receives the address of the value associated with the `key`. This pointer is
 only guaranteed to be valid only while the `tree` doesn't undergo
 structural changes, (such as potentially calling it again.)
 @return Either `TREE_ERROR` (false) and doesn't touch `tree`, `TREE_ABSENT`
 and adds a new key with `key`, or `TREE_PRESENT` there was already an existing
 key. @throws[malloc] @order \Theta(\log |`tree`|) @allow */
static enum tree_result T_(assign)(struct t_(tree) *const tree,
	const pT_(key) key, pT_(value) **const valuep)
	{ return assert(tree), pT_(update)(&tree->trunk, key, 0, valuep); }
#		else /* map --><!-- set */
/** Only if `TREE_VALUE` is not defined. Adds `key` to `tree` only if it is a
 new value, otherwise returns `TREE_PRESENT`. See <fn:<T>assign>, which is the
 map version. @allow */
static enum tree_result T_(add)(struct t_(tree) *const tree,
	const pT_(key) key)
	{ return assert(tree), pT_(update)(&tree->trunk, key, 0); }
#		endif /* set --> */

#		ifdef TREE_VALUE /* <!-- map */
/** Adds or updates `key` in `tree`.
 @param[eject] If this parameter is non-null and a return value of
 `TREE_PRESENT`, the old key is stored in `eject`, replaced by `key`. A null
 value indicates that on conflict, the new key yields to the old key, as
 <fn:<T>add>. This is only significant in trees with distinguishable keys.
 @param[value] Only present if `TREE_VALUE` (map) was specified. If this
 parameter is non-null and a return value other then `TREE_ERROR`, this
 receives the address of the value associated with the key.
 @return Either `TREE_ERROR` (false,) `errno` is set and doesn't touch `tree`;
 `TREE_ABSENT`, adds a new key; or `TREE_PRESENT`, there was already an
 existing key. @throws[malloc] @order \Theta(\log |`tree`|) @allow */
static enum tree_result T_(update)(struct t_(tree) *const tree,
	const pT_(key) key, pT_(key) *const eject, pT_(value) **const value)
	{ return assert(tree), pT_(update)(&tree->trunk, key, eject, value); }
#		else /* map --><!-- set */
/** Replaces `eject` by `key` or adds `key` in `tree`, but in a set. */
static enum tree_result T_(update)(struct t_(tree) *const tree,
	const pT_(key) key, pT_(key) *const eject)
	{ return assert(tree), pT_(update)(&tree->trunk, key, eject); }
#		endif /* set --> */

/** Tries to remove `key` from `tree`. @return Success, otherwise it was not in
 `tree`. @order \Theta(\log |`tree`|) @allow */
static int T_(remove)(struct t_(tree) *const tree, const pT_(key) key)
	{ return assert(tree), !!tree->trunk.bough
	&& tree->trunk.height /**/> 1 && pT_(remove)(&tree->trunk, key); }

/** `source` is copied to, and overwrites, `tree`.
 @param[source] In the case where it's null or idle, if `tree` is empty, then
 it continues to be.
 @return Success, otherwise `tree` is not modified.
 @throws[malloc] @throws[EDOM] `tree` is null. @throws[ERANGE] The size of
 `source` nodes doesn't fit into `size_t`.
 @order \O(|`source`| + |`tree`|) time and temporary space. @allow */
static int T_(clone)(struct t_(tree) *const restrict tree,
	const struct t_(tree) *const restrict source) {
	struct pT_(scaffold) sc;
	int success = 1;
	sc.data = 0; /* Need to keep this updated to catch. */
	if(!tree) { errno = EDOM; goto catch; }
	/* Count the number of nodes and set up to copy. */
	if(!pT_(nodes)(tree, &sc.victim) || !pT_(nodes)(source, &sc.source)
		|| (sc.no = sc.source.branches + sc.source.leaves) < sc.source.branches)
		{ errno = ERANGE; goto catch; } /* Overflow. */
	if(!sc.no) { pT_(clear)(tree); goto finally; } /* No need to allocate. */
	if(!(sc.data = malloc(sizeof *sc.data * sc.no)))
		{ if(!errno) errno = ERANGE; goto catch; }
	{ /* Makes debugging easier; not necessary. */
		size_t i;
		for(i = 0; i < sc.no; i++) sc.data[i] = 0;
	}
	{ /* Ready scaffold. */
		struct tree_node_count need;
		need.leaves = sc.source.leaves > sc.victim.leaves
			? sc.source.leaves - sc.victim.leaves : 0;
		need.branches = sc.source.branches > sc.victim.branches
			? sc.source.branches - sc.victim.branches : 0;
		sc.branch.head = sc.data;
		sc.branch.fresh = sc.branch.iterator
			= sc.branch.head + sc.source.branches - need.branches;
		sc.leaf.head = sc.branch.fresh + need.branches;
		sc.leaf.fresh = sc.leaf.iterator
			= sc.leaf.head + sc.source.leaves - need.leaves;
		assert(sc.leaf.fresh + need.leaves == sc.data + sc.no);
	}
	/* Add new nodes. */
	while(sc.branch.iterator != sc.leaf.head) {
		struct pT_(branch_bough) *branch;
		if(!(branch = malloc(sizeof *branch))) goto catch;
		branch->base.size = 0;
		branch->child[0] = 0;
		*sc.branch.iterator++ = &branch->base;
	}
	while(sc.leaf.iterator != sc.data + sc.no) {
		struct pT_(bough) *leaf;
		if(!(leaf = malloc(sizeof *leaf))) goto catch;
		leaf->size = 0;
		*sc.leaf.iterator++ = leaf;
	}
	/* Resources acquired; now we don't care about tree. */
	pT_(cannibalize)(tree, &sc);
	/* The scaffold has the exact number of nodes we need. Overwrite. */
	tree->trunk = pT_(clone)(&source->trunk, &sc);
	goto finally;
catch:
	success = 0;
	if(!sc.data) goto finally;
	while(sc.leaf.iterator != sc.leaf.fresh) {
		struct pT_(bough) *leaf = *(--sc.leaf.iterator);
		assert(leaf);
		free(leaf);
	}
	while(sc.branch.iterator != sc.branch.fresh) {
		struct pT_(branch_bough) *branch
			= pT_(as_branch)(*(--sc.branch.iterator));
		assert(branch);
		free(branch);
	}
finally:
	free(sc.data); /* Temporary memory. */
	return success;
}

#		define BOX_PRIVATE_AGAIN
#		include "box.h"

static void pT_(unused_base_coda)(void);
static void pT_(unused_base)(void) {
	pT_(key) k; pT_(value) v; memset(&k, 0, sizeof k); memset(&v, 0, sizeof v);
	T_(begin)(0); T_(exists)(0); T_(entry)(0); T_(key)(0);
	T_(next)(0); T_(previous)(0); T_(less)(0, k); T_(more)(0, k);
	t_(tree)(); t_(tree_)(0); T_(clear)(0); T_(count)(0);
	T_(contains)(0, k); T_(get_or)(0, k, v);
	T_(less_or)(0, k, k); T_(more_or)(0, k, k);
#		ifdef TREE_VALUE
	T_(bulk_assign)(0, k, 0); T_(assign)(0, k, 0);
	T_(update)(0, k, 0, 0); T_(value)(0);
#		else
	T_(bulk_add)(0, k); T_(add)(0, k); T_(update)(0, k, 0);
#		endif
	T_(bulk_finish)(0); T_(remove)(0, k); T_(clone)(0, 0);
	pT_(unused_base_coda)();
}
static void pT_(unused_base_coda)(void) { pT_(unused_base)(); }
#	endif /* body --> */
#endif /* base code --> */

#ifdef TREE_TO_STRING
#	undef TREE_TO_STRING
#	ifndef TREE_DECLARE_ONLY
#		ifndef TREE_TRAIT
#			ifdef TREE_VALUE
/** The type of the required `<tr>to_string`. Responsible for turning the
 read-only argument into a 12-max-`char` output string. `<pT>value` is omitted
 when it's a set. */
typedef void (*pT_(to_string_fn))(const pT_(key), const pT_(value) *,
	char (*)[12]);
#			else
typedef void (*pT_(to_string_fn))(const pT_(key), char (*)[12]);
#			endif
#		endif
/** Thunk(`cur`, `a`). One must implement `<tr>to_string`. */
static void pTR_(to_string)(const struct T_(cursor) *const cur,
	char (*const a)[12]) {
#		ifdef TREE_VALUE
	tr_(to_string)(cur->ref.bough->key[cur->ref.idx],
		cur->ref.bough->value + cur->ref.idx, a);
#		else
	tr_(to_string)(cur->ref.bough->key[cur->ref.idx], a);
#		endif
}
#	endif
#	include "to_string.h" /** \include */
#	ifndef TREE_TRAIT
#		define TREE_HAS_TO_STRING /* Warning about tests. */
#	endif
#endif

#if defined HAS_GRAPH_H && defined TREE_HAS_TO_STRING && !defined TREE_TRAIT
#	include "graph.h" /** \include */
#endif

#if defined TREE_TEST && !defined TREE_TRAIT \
	&& defined TREE_HAS_TO_STRING && defined HAS_GRAPH_H
#	include "../test/test_tree.h"
#endif

#ifdef TREE_DEFAULT
#	define BOX_PUBLIC_OVERRIDE
#	include "box.h"
/** This is functionally identical to <fn:<T>get_or>, but a with a trait
 specifying a constant default value.
 @return The value associated with `key` in `tree`, (which can be null.) If
 no such value exists, the `TREE_DEFAULT` is returned.
 @order \O(\log |`tree`|). @allow */
static pT_(value) T_R_(tree, get)(const struct t_(tree) *const tree,
	const pT_(key) key) {
	const pT_(value) pTR_(default_value) = TREE_DEFAULT;
	struct pT_(ref) ref; /* `TREE_DEFAULT` is a valid <tag:<pT>value>. */
	return tree && tree->trunk.bough && tree->trunk.height
		&& (ref = pT_(lookup_find)(tree->trunk, key)).bough
		? *pT_(ref_to_valuep)(ref) : pTR_(default_value);
}
#	define BOX_PRIVATE_AGAIN
#	include "box.h"
static void pTR_(unused_default_coda)(void);
static void pTR_(unused_default)(void) {
	pT_(key) k; memset(&k, 0, sizeof k);
	T_R_(tree, get)(0, k); pTR_(unused_default_coda)();
}
static void pTR_(unused_default_coda)(void) { pTR_(unused_default)(); }
#	undef TREE_DEFAULT
#endif


#ifdef TREE_EXPECT_TRAIT
#	undef TREE_EXPECT_TRAIT
#else
#	undef BOX_MINOR
#	undef BOX_MAJOR
#	undef TREE_NAME
#	undef TREE_KEY
#	undef TREE_ORDER
#	ifdef TREE_VALUE
#		undef TREE_VALUE
#	endif
#	ifdef TREE_LESS
#		undef TREE_LESS
#	endif
#	ifdef TREE_HAS_TO_STRING
#		undef TREE_HAS_TO_STRING
#	endif
#	ifdef TREE_TEST
#		undef TREE_TEST
#	endif
#	ifdef TREE_DECLARE_ONLY
#		undef TREE_DECLARE_ONLY
#	endif
#endif
#ifdef TREE_TRAIT
#	undef TREE_TRAIT
#	undef BOX_TRAIT
#endif
#define BOX_END
#include "box.h"
