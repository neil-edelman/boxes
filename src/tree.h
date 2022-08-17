/** @license 2022 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @abstract Stand-alone header <src/tree.h>; examples <test/test_tree.c>. On a
 compatible workstation, `make` creates the test suite of the examples.

 @subtitle Ordered tree

 A <tag:<B>tree> is an ordered set or map contained in a tree. For memory
 locality, this is implemented B-tree, described in
 <Bayer, McCreight, 1972, Large>.

 @param[TREE_NAME, TREE_KEY]
 `<B>` that satisfies `C` naming conventions when mangled, required, and
 `TREE_KEY`, a comparable type, <typedef:<PB>key>, whose default is
 `unsigned int`. `<PB>` is private, whose names are prefixed in a manner to
 avoid collisions.

 @param[TREE_VALUE]
 `TRIE_VALUE` is an optional payload to go with the type, <typedef:<PB>value>.
 The makes it a map of <tag:<B>tree_entry> instead of a set.

 @param[TREE_COMPARE]
 A function satisfying <typedef:<PB>compare_fn>. Defaults to ascending order.
 Required if `TREE_KEY` is changed to an incomparable type.

 @param[TREE_ORDER]
 Sets the branching factor, or order as <Knuth, 1998 Art 3>, to the range
 `[3, UCHAR_MAX+1]`. Default is most likely fine except when specific
 constraints have to be met. (That is, setting `TREE_ORDER` to 4 is an
 isomorphism to red-black trees.)

 @param[TREE_EXPECT_TRAIT]
 Do not un-define certain variables for subsequent inclusion in a parameterized
 trait.

 @param[TREE_TO_STRING_NAME, TREE_TO_STRING]
 To string trait contained in <to_string.h>; an optional unique `<SZ>`
 that satisfies `C` naming conventions when mangled and function implementing
 <typedef:<PSTR>to_string_fn>.

 @fixme multi-key; implementation of order statistic tree?
 @fixme merge, difference

 @std C89 */

#if !defined(TREE_NAME)
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
#include <stddef.h> /* fixme: stdlib, string should do it; what is going on? */
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
/* Leaf: `TREE_MAX type`; branch: `TREE_MAX type + TREE_ORDER pointer`. In
 <Goodrich, Tamassia, Mount, 2011, Data>, these are (a,b)-trees as
 (TREE_MIN+1,TREE_MAX+1)-trees. */
#define TREE_MAX (TREE_ORDER - 1)
/* This is the worst-case branching factor; the performance will be
 \O(log_{`TREE_MIN`+1} `size`). Usually this is `⌈(TREE_MAX+1)/2⌉-1`. However,
 smaller values are less-eager; in the extreme,
 <Johnson, Shasha, 1993, Free-at-Empty>, show good results; this has been
 chosen to provide hysteresis. (Except `TREE_MAX 2`, it's fixed.) */
#define TREE_MIN (TREE_MAX / 3 ? TREE_MAX / 3 : 1)
/* FIXME: Is this good? */
#define TREE_SPLIT (TREE_ORDER / 2) /* Split index: even order left-leaning. */
#define TREE_RESULT X(ERROR), X(UNIQUE), X(YIELD)
#define X(n) TREE_##n
/** A result of modifying the tree, of which `TREE_ERROR` is false.
 ![A diagram of the result states.](../doc/put.png) */
enum tree_result { TREE_RESULT };
#undef X
#define X(n) #n
/** A static array of strings describing the <tag:tree_result>. */
static const char *const tree_result_str[] = { TREE_RESULT };
#undef X
#undef TREE_RESULT
struct tree_count { size_t branches, leaves; };
#endif /* idempotent --> */


#if TREE_TRAITS == 0 /* <!-- base code */


#ifndef TREE_ORDER
#define TREE_ORDER 3 /* Maximum degree, (branching factor.) */
#endif
#if TREE_ORDER < 3 || TREE_ORDER > UCHAR_MAX + 1
#error TREE_ORDER parameter range `[3, UCHAR_MAX+1]`.
#endif
#ifndef TREE_KEY
#define TREE_KEY unsigned
#endif

/** A comparable type, defaults to `unsigned`. */
typedef TREE_KEY PB_(key);
typedef const TREE_KEY PB_(key_c);

#ifdef TREE_VALUE
/** On `TREE_VALUE`, otherwise just a set of <typedef:<PB>key>. */
typedef TREE_VALUE PB_(value);
typedef const TREE_VALUE PB_(value_c);
#endif

/** Returns a positive result if `a` is out-of-order with respect to `b`,
 inducing a strict weak order. This is compatible, but less strict then the
 comparators from `bsearch` and `qsort`; it only needs to divide entries
 into two instead of three categories. */
typedef int (*PB_(compare_fn))(PB_(key_c) a, PB_(key_c) b);

#ifndef TREE_COMPARE /* <!-- !cmp */
/** The default `TREE_COMPARE` on `a` and `b` is integer comparison that
 results in ascending order. @implements <typedef:<PB>compare_fn> */
static int PB_(default_compare)(PB_(key_c) a, PB_(key_c) b)
	{ return a > b; }
#define TREE_COMPARE &PB_(default_compare)
#endif /* !cmp --> */

/* Check that `TREE_COMPARE` is a function implementing
 <typedef:<PB>compare_fn>, if defined. */
static const PB_(compare_fn) PB_(compare) = (TREE_COMPARE);

/* These rules are more lazy than the original so as to not exhibit worst-case
 behaviour in small trees, as <Johnson, Shasha, 1993, Free-at-Empty>, (lookup
 is potentially slower after deleting.) In the terminology of
 <Knuth, 1998 Art 3>,
 * Every branch has at most `TREE_ORDER == TREE_MAX + 1` children, which is at
   minimum three.
 * Every non-root and non-bulk-loaded node has at least `TREE_MIN` keys,
   (`⎣TREE_MAX/3⎦`.)
 * Every branch has at least one child, `k`, and contains `k - 1` keys, (this
   is a consequence of the fact that they are implicitly storing a complete
   binary sub-tree.)
 * All leaves are at the maximum depth and height zero; they do'n't carry links
   to other nodes, (hence, leaf.) In this code, a branch node is a
   specialization of a (leaf) node with children. One can tell if it's a branch
   by keeping track of the height.
 * There are two empty B-trees to facilitate allocation hysteresis between
   0 -- 1: idle `{ 0, 0 }`, and `{ garbage leaf, UINT_MAX }`, one could test,
   `!root || height == UINT_MAX`.
 * Bulk-loading always is on the right side. */
struct PB_(node) {
	unsigned char size; /* `[0, TREE_MAX]`. */
	PB_(key) key[TREE_MAX]; /* Cache-friendly lookup. */
#ifdef TREE_VALUE
	PB_(value) value[TREE_MAX];
#endif
};
/* B-tree branch is a <tag:<PB>node> and links to `size + 1` nodes. */
struct PB_(branch) { struct PB_(node) base, *child[TREE_ORDER]; };
/** @return Downcasts `as_leaf` to a branch. */
static struct PB_(branch) *PB_(as_branch)(struct PB_(node) *const as_leaf)
	{ return (struct PB_(branch) *)(void *)
	((char *)as_leaf - offsetof(struct PB_(branch), base)); }
/** @return Downcasts `as_node` to a branch. */
static const struct PB_(branch) *PB_(as_branch_c)(const struct PB_(node) *
	const as_node) { return (const struct PB_(branch) *)(const void *)
	((const char *)as_node - offsetof(struct PB_(branch), base)); }
/* Address specific entry. */
struct PB_(ref) { struct PB_(node) *node; unsigned height, idx; };
struct PB_(ref_c) { const struct PB_(node) *node; unsigned height, idx; };
struct PB_(tree) { struct PB_(node) *node; unsigned height; };
/** To initialize it to an idle state, see <fn:<B>tree>, `TRIE_IDLE`, `{0}`
 (`C99`), or being `static`.

 ![States.](../doc/states.png) */
struct B_(tree);
struct B_(tree) { struct PB_(tree) root; };

#ifdef TREE_VALUE /* <!-- value */

/** On `TREE_VALUE`, creates a map from pointer-to-<typedef:<PB>key> to
 pointer-to-<typedef:<PB>value>. The reason these are pointers is because it
 is not connected in memory. (Does `key` still have to be?) */
struct B_(tree_entry) { PB_(key) *key; PB_(value) *value; };
struct B_(tree_entry_c) { PB_(key_c) *key; PB_(value_c) *value; };
/** On `TREE_VALUE`, otherwise it's just an alias for
 pointer-to-<typedef:<PB>key>. */
typedef struct B_(tree_entry) PB_(entry);
typedef struct B_(tree_entry_c) PB_(entry_c);
static PB_(entry) PB_(null_entry)(void)
	{ const PB_(entry) e = { 0, 0 }; return e; }
static PB_(entry_c) PB_(null_entry_c)(void)
	{ const PB_(entry_c) e = { 0, 0 }; return e; }
static PB_(entry) PB_(leaf_to_entry)(struct PB_(node) *const leaf,
	const unsigned i) { PB_(entry) e;
	e.key = leaf->key + i, e.value = leaf->value + i; return e; }
static PB_(entry_c) PB_(leaf_to_entry_c)(const struct PB_(node) *const leaf,
	const unsigned i) { PB_(entry_c) e;
	e.key = leaf->key + i, e.value = leaf->value + i; return e; }
static PB_(value) *PB_(ref_to_value)(const struct PB_(ref) ref)
	{ return ref.node ? ref.node->value + ref.idx : 0; }

#else /* value --><!-- !value */

typedef PB_(key) PB_(value);
typedef PB_(key) *PB_(entry);
typedef PB_(key_c) *PB_(entry_c);
static PB_(entry_c) PB_(null_entry_c)(void) { return 0; }
static PB_(entry) PB_(null_entry)(void) { return 0; }
static PB_(entry) PB_(leaf_to_entry)(struct PB_(node) *const leaf,
	const unsigned i) { return leaf->key + i; }
static PB_(entry_c) PB_(leaf_to_entry_c)(const struct PB_(node) *const leaf,
	const unsigned i) { return leaf->key + i; }
static PB_(value) *PB_(ref_to_value)(const struct PB_(ref) ref)
	{ return ref.node ? ref.node->key + ref.idx : 0; }

#endif /* !value --> */

#include "../test/orcish.h"
static void (*PB_(to_string))(PB_(entry_c), char (*)[12]);

/** @return If `ref` in `tree` has a predecessor, then it decrements. */
static int PB_(to_predecessor)(struct PB_(tree) tree,
	struct PB_(ref) *const ref) {
	assert(ref);
	if(!tree.node || tree.height == UINT_MAX) return 0; /* Empty. */
	if(!ref->node) { /* Null: `ref` is the last key. */
		struct PB_(tree) descend = tree;
		while(descend.height) descend.height--, descend.node
			= PB_(as_branch)(descend.node)->child[descend.node->size];
		/* While bulk-loading, could have empty right. */
		if(descend.node->size) ref->node = descend.node,
			ref->height = 0, ref->idx = descend.node->size - 1;
		else assert(tree.node->size), ref->node = tree.node,
			ref->height = tree.height, ref->idx = tree.node->size - 1;
		return 1;
	}
	while(ref->height) ref->height--,
		ref->node = PB_(as_branch_c)(ref->node)->child[ref->idx],
		ref->idx = ref->node->size;
	if(ref->idx) return ref->idx--, 1; /* Likely. */
{ /* Re-descend; pick the minimum height node that has a previous key. */
	struct PB_(ref) prev;
	unsigned a0;
	PB_(key) x;
	for(prev.node = 0, x = ref->node->key[0]; tree.height;
		tree.node = PB_(as_branch_c)(tree.node)->child[a0], tree.height--) {
		/* fixme: This is repeated. */
		unsigned a1 = tree.node->size;
		a0 = 0;
		while(a0 < a1) {
			const unsigned m = (a0 + a1) / 2;
			if(PB_(compare)(x, tree.node->key[m]) > 0) a0 = m + 1; else a1 = m;
		}
		if(a0)
			prev.node = tree.node, prev.height = tree.height, prev.idx = a0 - 1;
	}
	if(!prev.node) return 0; /* Off the left. */
	*ref = prev;
}	return 1; /* Jumped nodes. */
}
/* @return If `ref_c` in `tree` has a successor, then it increments. */
#define TREE_TO_SUCCESSOR(to_successor_c, ref_c) \
static int PB_(to_successor_c)(struct PB_(tree) tree, \
	struct PB_(ref_c) *const ref) { \
	assert(ref); \
	if(!tree.node || tree.height == UINT_MAX) return 0; /* Empty. */ \
	if(!ref->node) \
		ref->node = tree.node, ref->height = tree.height, ref->idx = 0; \
	else \
		ref->idx++; \
	while(ref->height) ref->height--, \
		ref->node = PB_(as_branch_c)(ref->node)->child[ref->idx], ref->idx = 0; \
	if(ref->idx < ref->node->size) return 1; /* Likely. */ \
	if(!ref->node->size) return 0; /* When bulk-loading. */ \
{	/* Re-descend; pick the minimum height node that has a next key. */ \
	struct PB_(ref_c) next; \
	unsigned a0; \
	PB_(key) x; \
	for(next.node = 0, x = ref->node->key[ref->node->size - 1]; tree.height; \
		tree.node = PB_(as_branch_c)(tree.node)->child[a0], tree.height--) { \
		unsigned a1 = tree.node->size; \
		a0 = 0; \
		while(a0 < a1) { \
			const unsigned m = (a0 + a1) / 2; \
			if(PB_(compare)(x, tree.node->key[m]) > 0) a0 = m + 1; else a1 = m;\
		} \
		if(a0 < tree.node->size) \
			next.node = tree.node, next.height = tree.height, next.idx = a0; \
	} \
	if(!next.node) return 0; /* Off the right. */ \
	*ref = next; \
}	return 1; /* Jumped nodes. */ \
}
TREE_TO_SUCCESSOR(to_successor, ref)
TREE_TO_SUCCESSOR(to_successor_c, ref_c) /* For forward iteration. */
#undef TREE_TO_SUCCESSOR

#define BOX_CONTENT PB_(entry_c)
/** Is `e` not null? @implements `is_element_c` */
static int PB_(is_element_c)(PB_(entry_c) e) {
#ifdef TREE_VALUE
	return !!e.key;
#else
	return !!e;
#endif
}
/* @implements `forward` */
struct PB_(forward) { const struct PB_(tree) *root; struct PB_(ref_c) next; };
/** @return Before `tree`. @implements `forward` */
static struct PB_(forward) PB_(forward)(const struct B_(tree) *const
	tree) { struct PB_(forward) it;
	it.root = tree ? &tree->root : 0, it.next.node = 0;
	return it;
}
/** Move to next `it`. @return Element or null. @implements `next_c` */
static PB_(entry_c) PB_(next_c)(struct PB_(forward) *const it) {
	return assert(it), PB_(to_successor_c)(*it->root, &it->next) ?
		PB_(leaf_to_entry_c)(it->next.node, it->next.idx) : PB_(null_entry_c)();
}

#define BOX_ITERATOR PB_(entry)
/** Is `x` not null? @implements `is_element` */
static int PB_(is_element)(const PB_(entry) e) {
#ifdef TREE_VALUE
	return !!e.key;
#else
	return !!e;
#endif
}
/* @implements `iterator` */
struct PB_(iterator) { struct PB_(tree) *root; struct PB_(ref) i; int seen; };
/** @return Iterator to null in `tree`. @implements `iterator` */
static struct PB_(iterator) PB_(iterator)(struct B_(tree) *const tree) {
	struct PB_(iterator) it;
	it.root = tree ? &tree->root : 0, it.i.node = 0, it.seen = 0;
	return it;
}
/** Advances `it`. @return Element or null. @implements `next` */
static PB_(entry) PB_(next)(struct PB_(iterator) *const it) {
	assert(it);
	if(!it->root
		|| (it->seen || !it->i.node) && !PB_(to_successor)(*it->root, &it->i)) {
		it->i.node = 0, it->seen = 0;
		return PB_(null_entry)();
	}
	it->seen = 1;
	return PB_(leaf_to_entry)(it->i.node, it->i.idx);
}
/** Move to previous `it`. @return Element or null. @implements `previous`
 @fixme */
static PB_(entry) PB_(previous)(struct PB_(iterator) *const it) {
	assert(it && 0);
	if(!it->root || !it->i.node && it->seen != -1
		|| it->seen && !PB_(to_predecessor)(*it->root, &it->i))
		return PB_(null_entry)();
	it->seen = -1;
	return PB_(leaf_to_entry)(it->i.node, it->i.idx);
}

/* Want to find slightly different things; code re-use is bad. Confusing. */
#define TREE_FORTREE(i) i.node = tree->node, i.height = tree->height; ; \
	i.node = PB_(as_branch_c)(i.node)->child[i.idx], i.height--
#define TREE_START(i) unsigned hi = i.node->size; i.idx = 0;
#define TREE_FORNODE(i) do { \
	const unsigned m = (i.idx + hi) / 2; \
	if(PB_(compare)(key, i.node->key[m]) > 0) i.idx = m + 1; \
	else hi = m; \
} while(i.idx < hi);
#define TREE_FLIPPED(i) PB_(compare)(i.node->key[i.idx], key) <= 0
/** One height at a time. */
static void PB_(find_idx)(struct PB_(ref) *const lo, const PB_(key) key) {
	TREE_START((*lo))
	if(!lo) return;
	TREE_FORNODE((*lo))
}
/** Finds lower-bound of `key` in `tree`. */
static struct PB_(ref) PB_(lower_r)(struct PB_(tree) *const tree,
	const PB_(key) key) {
	struct PB_(ref) i, lo = { 0, 0, 0 };
	for(TREE_FORTREE(i)) {
		TREE_START(i)
		if(!hi) continue;
		TREE_FORNODE(i)
		if(i.idx < i.node->size) {
			lo = i;
			/* Might be useful expanding this to multi-keys. */
			if(TREE_FLIPPED(i)) break;
		}
		if(!i.height) break;
	}
	return lo;
}
/** Finds (one?) exact `key` in `tree`. This is what not settling on a
 definition of a tree gets you. */
static struct PB_(ref) PB_(find)(struct PB_(tree) *const tree,
	const PB_(key) key) {
	struct PB_(ref) i;
	for(TREE_FORTREE(i)) {
		TREE_START(i)
		if(!hi) continue;
		TREE_FORNODE(i)
		if(i.idx < i.node->size && TREE_FLIPPED(i)) break;
		if(!i.height) { i.node = 0; return i; }
	}
	return i;
}
/** Finds lower-bound of `key` in `tree` while counting the non-filled `hole`
 and `is_equal`. (fixme: `is_equal` useless) */
static struct PB_(ref) PB_(lookup_insert)(struct PB_(tree) *const tree,
	const PB_(key) key, struct PB_(ref) *const hole, int *const is_equal) {
	struct PB_(ref) lo;
	hole->node = 0;
	for(TREE_FORTREE(lo)) {
		TREE_START(lo)
		if(hi < TREE_MAX) *hole = lo;
		if(!hi) continue;
		TREE_FORNODE(lo)
		if(lo.node->size < TREE_MAX) hole->idx = lo.idx;
		if(lo.idx < lo.node->size && TREE_FLIPPED(lo)) { *is_equal = 1; break; }
		if(!lo.height) break;
	}
	return lo;
}
/** Finds exact `key` in `tree`. If `node` is found, temporarily, the nodes
 that have `TREE_MIN` keys have `as_branch(node).child[TREE_MAX] = parent` or,
 for leaves, `leaf_parent`, which must be set. */
static struct PB_(ref) PB_(lookup_remove)(struct PB_(tree) *const tree,
	const PB_(key) key, struct PB_(node) **leaf_parent) {
	struct PB_(node) *parent = 0;
	struct PB_(ref) lo;
	for(TREE_FORTREE(lo)) {
		TREE_START(lo)
		/* Cannot delete bulk add. */
		if(parent && hi < TREE_MIN || !parent && !hi) { lo.node = 0; break; }
		if(hi <= TREE_MIN) { /* Remember the parent temporarily. */
			if(lo.height) PB_(as_branch)(lo.node)->child[TREE_MAX] = parent;
			else *leaf_parent = parent;
		}
		TREE_FORNODE(lo)
		if(lo.idx < lo.node->size && TREE_FLIPPED(lo)) break;
		if(!lo.height) { lo.node = 0; break; } /* Was not in. */
		parent = lo.node;
	}
	return lo;
}
#undef TREE_FORTREE
#undef TREE_START
#undef TREE_FORNODE
#undef TREE_FLIPPED

/** @param[tree] Can be null. @return Lower bound of `x` in `tree`.
 @order \O(\log |`tree`|) */
static struct PB_(ref) PB_(lower)(struct PB_(tree) tree, const PB_(key) x) {
	if(!tree.node || tree.height == UINT_MAX) {
		struct PB_(ref) ref; ref.node = 0; return ref;
	} else {
		return PB_(lower_r)(&tree, x);
	}
}

/** Frees non-empty `tree` and it's children recursively, but doesn't put it
 to idle or clear pointers.
 @param[one] If `one` is valid, tries to keep one leaf. Set to null before. */
static void PB_(clear_r)(struct PB_(tree) tree, struct PB_(node) **const keep) {
	assert(tree.node);
	if(!tree.height) {
		if(keep && !*keep) *keep = tree.node;
		else free(tree.node);
	} else {
		struct PB_(tree) child;
		unsigned i;
		child.height = tree.height - 1;
		for(i = 0; i <= tree.node->size; i++)
			child.node = PB_(as_branch)(tree.node)->child[i],
			PB_(clear_r)(child, keep);
		free(PB_(as_branch)(tree.node));
	}
}
/** `tree` can be null. */
static void PB_(clear)(struct B_(tree) *tree) {
	struct PB_(node) *one = 0;
	/* Already not there/idle/empty. */
	if(!tree || !tree->root.node || tree->root.height == UINT_MAX) return;
	PB_(clear_r)(tree->root, &one), assert(one);
	/* This is a special state where the tree has one leaf, but it is empty.
	 This state exists because it gives hysteresis to 0 -- 1 transition. */
	tree->root.node = one;
	tree->root.height = UINT_MAX;
}

/* Box override information. */
#define BOX_ PB_
#define BOX struct B_(tree)

/** @return Initializes `tree` to idle. @order \Theta(1) @allow */
static struct B_(tree) B_(tree)(void) {
	struct B_(tree) tree;
	tree.root.node = 0; tree.root.height = 0;
	return tree;
}

/** Returns an initialized `tree` to idle, `tree` can be null. @allow */
static void B_(tree_)(struct B_(tree) *const tree) {
	if(!tree) return; /* Null. */
	if(!tree->root.node) { /* Idle. */
		assert(!tree->root.height);
	} else if(tree->root.height == UINT_MAX) { /* Empty. */
		assert(tree->root.node), free(tree->root.node);
	} else {
		PB_(clear_r)(tree->root, 0);
	}
	*tree = B_(tree)();
}

/** Stores an iteration in a tree. Generally, changes in the topology of the
 tree invalidate it. (Future: have insert and delete with iterators.) */
struct B_(tree_iterator) { struct PB_(iterator) _; };
/** @return An iterator before the first element of `tree`. Can be null.
 @allow */
static struct B_(tree_iterator) B_(tree_iterator)(struct B_(tree) *const tree)
	{ struct B_(tree_iterator) it; it._ = PB_(iterator)(tree); return it; }
/** Advances `it` to the next element. @return A pointer to the current
 element or null. @allow */
static PB_(entry) B_(tree_next)(struct B_(tree_iterator) *const it)
	{ return PB_(next)(&it->_); }

/** @param[tree] Can be null. @return Finds the smallest entry in `tree` that
 is at the lower bound of `x`. If `x` is higher than any of `tree`, it will be
 placed just passed the end. @order \O(\log |`tree`|) @allow */
static struct B_(tree_iterator) B_(tree_lower_iterator)
	(struct B_(tree) *const tree, const PB_(key) x) {
	struct B_(tree_iterator) it;
	if(!tree) return it._.root = 0, it;
	it._.i = PB_(lower)(tree->root, x);
	it._.root = &tree->root;
	it._.seen = 0;
	return it;
}

/** For example, `tree = { 10 }`, `x = 5 -> 10`, `x = 10 -> 10`,
 `x = 11 -> null`.
 @return Lower-bound value match for `x` in `tree` or null if `x` is greater
 than all in `tree`. @order \O(\log |`tree`|) @allow */
static PB_(value) *B_(tree_lower_value)(struct B_(tree) *const tree,
	const PB_(key) x)
	{ return tree ? PB_(ref_to_value)(PB_(lower)(tree->root, x)) : 0; }

/** Clears `tree`, which can be null, idle, empty, or full. If it is empty or
 full, it remains active. */
static void B_(tree_clear)(struct B_(tree) *const tree) { PB_(clear)(tree); }

#include "../test/orcish.h"
static void PB_(print)(const struct B_(tree) *const tree);
#ifndef TREE_TEST
static void PB_(print)(const struct B_(tree) *const tree)
	{ (void)tree, printf("not printable\n"); }
#endif

/** Contains. */
static int B_(tree_contains)(struct B_(tree) *const tree, const PB_(key) x) {
	assert(tree);
	return PB_(find)(&tree->root, x).node ? 1 : 0;
}

#ifdef TREE_VALUE /* <!-- map */
/** TREE_ERROR should be set if it's not >= high. */
/** Packs `key` on the right side of `tree` without doing the usual
 restructuring. All other topology modification functions should be avoided
 until followed by <fn:<B>tree_bulk_finish>.
 @param[value] A pointer to the key's value which is set by the function on
 returning true. A null pointer in this parameter causes the value to go
 uninitialized. This parameter is not there if one didn't specify `TREE_VALUE`.
 @return One of <tag:tree_result>: `TREE_ERROR` and `errno` will be set,
 `TREE_YIELD` if the key is already (the highest) in the tree, and
 `TREE_UNIQUE`, added, the `value` (if specified) is uninitialized.
 @throws[EDOM] `x` is smaller than the largest key in `tree`. @throws[malloc] */
static enum tree_result B_(tree_bulk_add)(struct B_(tree) *const tree,
	PB_(key) key, PB_(value) **const value) {
#else /* map --><!-- set */
static enum tree_result B_(tree_bulk_add)(struct B_(tree) *const tree,
	PB_(key) key) {
#endif
	struct PB_(node) *node = 0, *head = 0; /* The original and new. */
	assert(tree);
	if(!tree->root.node) { /* Idle tree. */
		assert(!tree->root.height);
		if(!(node = malloc(sizeof *node))) goto catch;
		node->size = 0;
		tree->root.node = node;
		printf("bulk: idle\n");
	} else if(tree->root.height == UINT_MAX) { /* Empty tree. */
		node = tree->root.node;
		tree->root.height = 0;
		node->size = 0;
		printf("bulk: empty\n");
	} else {
		struct PB_(tree) unfull = { 0, 0 };
		unsigned new_nodes, n; /* Count new nodes. */
		struct PB_(node) *tail = 0, *last = 0;
		struct PB_(branch) *pretail = 0;
		struct PB_(tree) scout;
		PB_(key) i;
		printf("bulk: tree...\n"), PB_(print)(tree);
		for(scout = tree->root; ; scout.node = PB_(as_branch)(scout.node)
			->child[scout.node->size], scout.height--) {
			if(scout.node->size < TREE_MAX) unfull = scout;
			if(scout.node->size) last = scout.node;
			if(!scout.height) break;
		}
		assert(last), i = last->key[last->size - 1];
		/* Verify that the argument is not smaller than the largest. */
		if(PB_(compare)(i, key) > 0) return errno = EDOM, TREE_ERROR;
		if(PB_(compare)(key, i) <= 0) {
#ifdef TREE_VALUE
			if(value) { /* Last value in the last node. */
				struct PB_(ref) ref;
				ref.node = last, ref.idx = last->size - 1;
				*value = PB_(ref_to_value)(ref);
			}
#endif
			return TREE_YIELD;
		}

		/* One leaf, and the rest branches. */
		new_nodes = n = unfull.node ? unfull.height : tree->root.height + 2;
		/*printf("new_nodes: %u, tree height %u\n", new_nodes, tree->height);*/
		if(!n) {
			node = unfull.node;
		} else {
			if(!(node = tail = malloc(sizeof *tail))) goto catch;
			tail->size = 0;
			/*printf("new tail: %s.\n", orcify(tail));*/
			while(--n) {
				struct PB_(branch) *b;
				if(!(b = malloc(sizeof *b))) goto catch;
				b->base.size = 0;
				/*printf("new branch: %s.\n", orcify(b));*/
				if(!head) b->child[0] = 0, pretail = b; /* First loop. */
				else b->child[0] = head; /* Not first loop. */
				head = &b->base;
			}
		}

		/* Post-error; modify the original as needed. */
		if(pretail) pretail->child[0] = tail;
		else head = node;
		if(!unfull.node) { /* Add tree to head. */
			struct PB_(branch) *const branch = PB_(as_branch)(head);
			/*printf("adding the existing root, %s to %s\n",
				orcify(tree->root), orcify(head));*/
			assert(new_nodes > 1);
			branch->child[1] = branch->child[0];
			branch->child[0] = tree->root.node;
			node = tree->root.node = head, tree->root.height++;
		} else if(unfull.height) { /* Add head to tree. */
			struct PB_(branch) *const branch
				= PB_(as_branch)(node = unfull.node);
			/*printf("adding the linked list, %s to %s at %u\n",
				orcify(head), orcify(inner), inner->base.size + 1);*/
			assert(new_nodes);
			branch->child[branch->base.size + 1] = head;
		}
	}
	assert(node && node->size < TREE_MAX);
	node->key[node->size] = key;
#ifdef TREE_VALUE
	if(value) {
		struct PB_(ref) ref;
		ref.node = node, ref.idx = node->size;
		*value = PB_(ref_to_value)(ref);
	}
#endif
	node->size++;
	return TREE_UNIQUE;
catch: /* Didn't work. Reset. */
	free(node);
	while(head) {
		struct PB_(node) *const next = PB_(as_branch)(head)->child[0];
		free(head);
		head = next;
	}
	if(!errno) errno = ERANGE;
	return TREE_ERROR;
#ifdef TREE_VALUE
}
#else
}
#endif

/** Distributes `tree` on the right side so that, after a series of
 <fn:<B>tree_bulk_add>, it will be consistent with the minimum number of keys
 in a node. @return The re-distribution was a success and all nodes are within
 rules. The only time that it would be false is if a regular operation was
 performed interspersed with a bulk insertion without calling this function.
 (Are you sure?)
 (Maybe we should up the minimum to 1/2 for this function?)
 @order \O(\log `size`) */
static int B_(tree_bulk_finish)(struct B_(tree) *const tree) {
	struct PB_(tree) s;
	struct PB_(node) *right;
	printf("tree_bulk_finish(%s) number of nodes [%u, %u]\n",
		orcify(tree), TREE_MIN, TREE_MAX);
	if(!tree || !tree->root.node || tree->root.height == UINT_MAX) return 1;
	for(s = tree->root; s.height; s.node = right, s.height--) {
		unsigned distribute, right_want, right_move, take_sibling;
		struct PB_(branch) *parent = PB_(as_branch)(s.node);
		struct PB_(node) *sibling = (assert(parent->base.size),
			parent->child[parent->base.size - 1]);
		right = parent->child[parent->base.size];
		printf("initial parent node %s:%u with %u size, children %s and %s.\n",
			orcify(s.node), s.height, s.node->size,
			orcify(sibling), orcify(right));
		if(TREE_MIN <= right->size)
			{ printf("cool\n"); continue; } /* Has enough. */
		distribute = sibling->size + right->size;
		/* Should have at least `TREE_MAX` on left. */
		if(distribute < 2 * TREE_MIN) return 0;
		right_want = distribute / 2;
		right_move = right_want - right->size;
		take_sibling = right_move - 1;
		printf("distributing %u, of which the right wants %u and will"
			" be move %u and take %u from sibling.\n", distribute, right_want,
			right_move, take_sibling);
		/* Either the right has met the properties of a B-tree node, (covered
		 above,) or the left sibling is full from bulk-loading (relaxed.) */
		assert(right->size < right_want && right_want >= TREE_MIN
			&& sibling->size - take_sibling >= TREE_MIN + 1);
		/* Move the right node to accept more keys. */
		printf("right (%u) -> right at %u\n",
			right->size, right_move);
		memmove(right->key + right_move, right->key,
			sizeof *right->key * right->size);
#ifdef TREE_VALUE
		memmove(right->value + right_move, right->value,
			sizeof *right->value * right->size);
#endif
		printf("height %u\n", s.height);
		if(s.height > 1) { /* (Parent height.) */
			struct PB_(branch) *rbranch = PB_(as_branch)(right),
				*sbranch = PB_(as_branch)(sibling);
			memmove(rbranch->child + right_move, rbranch->child,
				sizeof *rbranch->child * (right->size + 1));
			memcpy(rbranch->child, sbranch->child + sibling->size + 1
				- right_move, sizeof *sbranch->child * right_move);
		}
		right->size += right_move;
		/* Move one node from the parent. */
		printf("right:%u <- parent:%u (1)\n",
			take_sibling, parent->base.size - 1);
		memcpy(right->key + take_sibling,
			parent->base.key + parent->base.size - 1, sizeof *right->key);
#ifdef TREE_VALUE
		memcpy(right->value + take_sibling,
			parent->base.value + parent->base.size - 1, sizeof *right->value);
#endif
		/* Move the others from the sibling. */
		printf("right <- sibling(%u) down to %u\n",
			sibling->size, take_sibling);
		memcpy(right->key, sibling->key + sibling->size - take_sibling,
			sizeof *right->key * take_sibling);
#ifdef TREE_VALUE
		memcpy(right->value, sibling->value + sibling->size - take_sibling,
			sizeof *right->value * take_sibling);
#endif
		sibling->size -= take_sibling;
		/* Sibling's key is now the parent's. */
		printf("parent:%u <- sibling:%u (1)\n",
			parent->base.size - 1, sibling->size - 1);
		memcpy(parent->base.key + parent->base.size - 1,
			sibling->key + sibling->size - 1, sizeof *right->key);
#ifdef TREE_VALUE
		memcpy(parent->base.value + parent->base.size - 1,
			sibling->value + sibling->size - 1, sizeof *right->value);
#endif
		sibling->size--;
		/* fixme: Also take the children. This is backwards in right. */
		printf("redistributed, %s:%u, %s:%u, %s:%u\n", orcify(s.node),
			parent->base.size, orcify(right), right->size, orcify(sibling),
			sibling->size);
	}
	return 1;
}

static void PB_(graph_usual)(const struct B_(tree) *const tree,
							 const char *const fn);

#ifdef TREE_VALUE /* <!-- map */
/** @param[value] If non-null and successful, a pointer that receives the
 address of the value associated with the key. Only present if `TREE_VALUE`
 (map) was specified.
 @return Either `TREE_ERROR` (false) and doesn't touch `tree`, `TREE_UNIQUE`
 and adds a new key, or `TREE_YIELD` and updates an existing key.
 @throws[malloc] */
static enum tree_result B_(tree_add)(struct B_(tree) *const tree,
	PB_(key) key, PB_(value) **const value) {
#else /* map --><!-- set */
static enum tree_result B_(tree_add)(struct B_(tree) *const tree,
	PB_(key) key) {
#endif /* set --> */
	struct PB_(node) *new_head = 0;
	struct PB_(ref) add, hole, cursor;
	int is_growing = 0;
	assert(tree);
	if(!(add.node = tree->root.node)) goto idle;
	else if(tree->root.height == UINT_MAX) goto empty;
	goto descend;
idle: /* No reserved memory. */
	assert(!add.node && !tree->root.height);
	if(!(add.node = malloc(sizeof *add.node))) goto catch;
	tree->root.node = add.node;
	tree->root.height = UINT_MAX;
	goto empty;
empty: /* Reserved dynamic memory, but tree is empty. */
	assert(add.node && tree->root.height == UINT_MAX);
	add.height = tree->root.height = 0;
	add.node->size = 0;
	add.idx = 0;
	goto insert;
descend: /* Record last node that has space. */
	{
		int is_equal = 0;
		add = PB_(lookup_insert)(&tree->root, key, &hole, &is_equal);
		if(is_equal) {
			/* Assumes key is unique; we might not want this for multi-maps,
			 but that is not implemented yet. */
#ifdef TREE_VALUE
			if(value) *value = PB_(ref_to_value)(add);
#endif
			return TREE_YIELD;
		}
	}
	if(hole.node == add.node) goto insert; else goto grow;
insert: /* Leaf has space to spare; usually end up here. */
	assert(add.node && add.idx <= add.node->size && add.node->size < TREE_MAX
		&& (!add.height || is_growing));
	memmove(add.node->key + add.idx + 1, add.node->key + add.idx,
		sizeof *add.node->key * (add.node->size - add.idx));
#ifdef TREE_VALUE
	memmove(add.node->value + add.idx + 1, add.node->value + add.idx,
		sizeof *add.node->value * (add.node->size - add.idx));
#endif
	add.node->size++;
	add.node->key[add.idx] = key;
#ifdef TREE_VALUE
	if(value) *value = PB_(ref_to_value)(add);
#endif
	return TREE_UNIQUE;
grow: /* Leaf is full. */ {
	unsigned new_no = hole.node ? hole.height : tree->root.height + 2;
	struct PB_(node) **new_next = &new_head, *new_leaf;
	struct PB_(branch) *new_branch;
	assert(new_no);
	/* Allocate new nodes in succession. */
	while(new_no != 1) { /* All branches except one. */
		if(!(new_branch = malloc(sizeof *new_branch))) goto catch;
		new_branch->base.size = 0;
		new_branch->child[0] = 0;
		*new_next = &new_branch->base, new_next = new_branch->child;
		new_no--;
	}
	/* Last point of potential failure; (don't need to have entry in catch.) */
	if(!(new_leaf = malloc(sizeof *new_leaf))) goto catch;
	new_leaf->size = 0;
	*new_next = new_leaf;
	/* Attach new nodes to the tree. The hole is now an actual hole. */
	if(hole.node) { /* New nodes are a sub-structure of the tree. */
		struct PB_(branch) *holeb = PB_(as_branch)(hole.node);
		memmove(hole.node->key + hole.idx + 1, hole.node->key + hole.idx,
			sizeof *hole.node->key * (hole.node->size - hole.idx));
#ifdef TREE_VALUE
		memmove(hole.node->value + hole.idx + 1, hole.node->value + hole.idx,
			sizeof *hole.node->value * (hole.node->size - hole.idx));
#endif
		memmove(holeb->child + hole.idx + 2, holeb->child + hole.idx + 1,
			sizeof *holeb->child * (hole.node->size - hole.idx));
		holeb->child[hole.idx + 1] = new_head;
		hole.node->size++;
	} else { /* New nodes raise tree height. */
		struct PB_(branch) *const new_root = PB_(as_branch)(new_head);
		hole.node = new_head, hole.height = ++tree->root.height, hole.idx = 0;
		new_head = new_root->child[1] = new_root->child[0];
		new_root->child[0] = tree->root.node, tree->root.node = hole.node;
		hole.node->size = 1;
	}
	cursor = hole; /* Go down; (as opposed to doing it on paper.) */
	goto split;
} split: { /* Split between the new and existing nodes. */
	struct PB_(node) *sibling;
	assert(cursor.node && cursor.node->size && cursor.height);
	sibling = new_head;
	/*PB_(graph_usual)(tree, "graph/work.gv");*/
	/* Descend now while split hasn't happened -- easier. */
	new_head = --cursor.height ? PB_(as_branch)(new_head)->child[0] : 0;
	cursor.node = PB_(as_branch)(cursor.node)->child[cursor.idx];
	PB_(find_idx)(&cursor, key);
	assert(!sibling->size && cursor.node->size == TREE_MAX); /* Atomic. */
	/* Expand `cursor`, which is full, to multiple nodes. */
	if(cursor.idx < TREE_SPLIT) { /* Descend hole to `cursor`. */
		memcpy(sibling->key, cursor.node->key + TREE_SPLIT,
			sizeof *sibling->key * (TREE_MAX - TREE_SPLIT));
#ifdef TREE_VALUE
		memcpy(sibling->value, cursor.node->value + TREE_SPLIT,
			sizeof *sibling->value * (TREE_MAX - TREE_SPLIT));
#endif
		hole.node->key[hole.idx] = cursor.node->key[TREE_SPLIT - 1];
#ifdef TREE_VALUE
		hole.node->value[hole.idx] = cursor.node->value[TREE_SPLIT - 1];
#endif
		memmove(cursor.node->key + cursor.idx + 1,
			cursor.node->key + cursor.idx,
			sizeof *cursor.node->key * (TREE_SPLIT - 1 - cursor.idx));
#ifdef TREE_VALUE
		memmove(cursor.node->value + cursor.idx + 1,
			cursor.node->value + cursor.idx,
			sizeof *cursor.node->value * (TREE_SPLIT - 1 - cursor.idx));
#endif
		if(cursor.height) {
			struct PB_(branch) *const cb = PB_(as_branch)(cursor.node),
				*const sb = PB_(as_branch)(sibling);
			struct PB_(node) *temp = sb->child[0];
			memcpy(sb->child, cb->child + TREE_SPLIT,
				sizeof *cb->child * (TREE_MAX - TREE_SPLIT + 1));
			memmove(cb->child + cursor.idx + 2, cb->child + cursor.idx + 1,
				sizeof *cb->child * (TREE_SPLIT - 1 - cursor.idx));
			cb->child[cursor.idx + 1] = temp;
		}
		hole = cursor;
	} else if(cursor.idx > TREE_SPLIT) { /* Descend hole to `sibling`. */
		hole.node->key[hole.idx] = cursor.node->key[TREE_SPLIT];
#ifdef TREE_VALUE
		hole.node->value[hole.idx] = cursor.node->value[TREE_SPLIT];
#endif
		hole.node = sibling, hole.height = cursor.height,
			hole.idx = cursor.idx - TREE_SPLIT - 1;
		memcpy(sibling->key, cursor.node->key + TREE_SPLIT + 1,
			sizeof *sibling->key * hole.idx);
		memcpy(sibling->key + hole.idx + 1, cursor.node->key + cursor.idx,
			sizeof *sibling->key * (TREE_MAX - cursor.idx));
#ifdef TREE_VALUE
		memcpy(sibling->value, cursor.node->value + TREE_SPLIT + 1,
			sizeof *sibling->value * hole.idx);
		memcpy(sibling->value + hole.idx + 1, cursor.node->value + cursor.idx,
			sizeof *sibling->value * (TREE_MAX - cursor.idx));
#endif
		if(cursor.height) {
			struct PB_(branch) *const cb = PB_(as_branch)(cursor.node),
				*const sb = PB_(as_branch)(sibling);
			struct PB_(node) *temp = sb->child[0];
			memcpy(sb->child, cb->child + TREE_SPLIT + 1,
				sizeof *cb->child * (hole.idx + 1));
			memcpy(sb->child + hole.idx + 2, cb->child + cursor.idx + 1,
				sizeof *cb->child * (TREE_MAX - cursor.idx));
			sb->child[hole.idx + 1] = temp;
		}
	} else { /* Equal split: leave the hole where it is. */
		memcpy(sibling->key, cursor.node->key + TREE_SPLIT,
			sizeof *sibling->key * (TREE_MAX - TREE_SPLIT));
#ifdef TREE_VALUE
		memcpy(sibling->value, cursor.node->value + TREE_SPLIT,
			sizeof *sibling->value * (TREE_MAX - TREE_SPLIT));
#endif
		if(cursor.height) {
			struct PB_(branch) *const cb = PB_(as_branch)(cursor.node),
				*const sb = PB_(as_branch)(sibling);
			memcpy(sb->child + 1, cb->child + TREE_SPLIT + 1,
				sizeof *cb->child * (TREE_MAX - TREE_SPLIT));
		}
	}
	/* Divide `TREE_MAX + 1` into two trees. */
	cursor.node->size = TREE_SPLIT, sibling->size = TREE_MAX - TREE_SPLIT;
	if(cursor.height) goto split; /* Loop max `\log_{TREE_MIN} size`. */
	hole.node->key[hole.idx] = key;
#ifdef TREE_VALUE
	if(value) *value = PB_(ref_to_value)(hole);
#endif
	assert(!new_head);
	return TREE_UNIQUE;
} catch: /* Didn't work. Reset. */
	while(new_head) {
		struct PB_(branch) *const top = PB_(as_branch)(new_head);
		new_head = top->child[0];
		free(top);
	}
	if(!errno) errno = ERANGE; /* Non-POSIX OSs not mandated to set errno. */
	return TREE_ERROR;
#ifdef TREE_VALUE
}
#else
}
#endif

/****************************/
static void PB_(graph)(const struct B_(tree) *const tree,
					   const char *const fn);




/** Tries to remove `key` from `tree`. @return Success. */
static int B_(tree_remove)(struct B_(tree) *const tree,
	const PB_(key) key) {
	struct PB_(ref) rm, parent /* Only if `key.size <= TREE_MIN`. */;
	struct PB_(branch) *parentb;
	struct { struct PB_(node) *less, *more; } sibling;
	parent.node = 0;
	assert(tree);
	printf("<remove> MIN %u, MAX %u, ORDER %u\n",
		TREE_MIN, TREE_MAX, TREE_ORDER);
	/* Traverse down the tree until `key`, leaving breadcrumbs for parents of
	 minimum key nodes. */
	if(!tree->root.node || tree->root.height == UINT_MAX
		|| !(rm = PB_(lookup_remove)(&tree->root, key, &parent.node)).node)
		{ printf(" didn't match any nodes.\n"); return 0; }
	/* Important when `rm = parent`; `find_idx` later. */
	parent.height = rm.height + 1;
	assert(rm.idx < rm.node->size);
	printf(" %s(%u):%u <%u>; parent %s(%u)\n",
		orcify(rm.node), rm.height, rm.idx, rm.node->key[rm.idx],
		orcify(parent.node), parent.height);
	if(rm.height) goto branch; else goto upward;
branch: {
	struct { struct PB_(ref) leaf; struct PB_(node) *parent; unsigned top; }
		pred, succ, chosen;
	assert(rm.height);
	printf("<branch> %s; replace by successor or predecessor.\n",
		orcify(rm.node));
	/* Predecessor leaf. */
	pred.leaf = rm, pred.top = UINT_MAX;
	do {
		struct PB_(node) *const up = pred.leaf.node;
		pred.leaf.node = PB_(as_branch_c)(pred.leaf.node)->child[pred.leaf.idx];
		pred.leaf.idx = pred.leaf.node->size;
		pred.leaf.height--;
		if(pred.leaf.node->size < TREE_MIN)
			{ pred.leaf.node = 0; goto no_pred; }
		else if(pred.leaf.node->size > TREE_MIN) pred.top = pred.leaf.height;
		else if(pred.leaf.height)
			PB_(as_branch)(pred.leaf.node)->child[TREE_MAX] = up;
		else pred.parent = up;
	} while(pred.leaf.height);
	pred.leaf.idx--;
	printf(" pred %s(%u):%u\n",
		orcify(pred.leaf.node), pred.leaf.height, pred.leaf.idx);
no_pred:
	/* Successor leaf. */
	succ.leaf = rm, succ.top = UINT_MAX;
	succ.leaf.idx++;
	do {
		struct PB_(node) *const up = succ.leaf.node;
		succ.leaf.node = PB_(as_branch_c)(succ.leaf.node)->child[succ.leaf.idx],
		succ.leaf.idx = 0;
		succ.leaf.height--;
		if(succ.leaf.node->size < TREE_MIN)
			{ succ.leaf.node = 0; goto no_succ; }
		else if(succ.leaf.node->size > TREE_MIN) succ.top = succ.leaf.height;
		else if(succ.leaf.height)
			PB_(as_branch)(succ.leaf.node)->child[TREE_MAX] = up;
		else succ.parent = up;
	} while(succ.leaf.height);
	printf(" succ %s(%u):%u\n",
		orcify(succ.leaf.node), succ.leaf.height, succ.leaf.idx);
no_succ:
	/* Choose the predecessor or successor. */
	if(!pred.leaf.node) {
		assert(succ.leaf.node);
		printf(" succ, pred null\n");
		chosen = succ;
	} else if(!succ.leaf.node) {
		assert(pred.leaf.node);
		printf(" pred, succ null\n");
		chosen = pred;
	} else if(pred.leaf.node->size < succ.leaf.node->size) {
		printf(" succ, has more keys\n");
		chosen = succ;
	} else if(pred.leaf.node->size > succ.leaf.node->size) {
		printf(" pred, has more keys\n");
		chosen = pred;
	} else if(pred.top > succ.top) {
		printf(" succ, less iterations\n");
		chosen = succ;
	} else {
		printf(" pred, less or equal iterations\n");
		chosen = pred;
	}
	/* Replace `rm` with the predecessor or the successor leaf. */
	rm.node->key[rm.idx] = chosen.leaf.node->key[chosen.leaf.idx];
	rm = chosen.leaf;
	goto upward;
} upward: /* The first iteration, this will be a leaf. */
	assert(rm.node);
	printf("<upward> %s\n", orcify(rm.node));
	if(!parent.node) goto space; /* Same predicate. */
	/* Retrieve forgot information about the index in parent. (This is not as
	 fast at it could be, but holding parent data in minimum keys allows it to
	 be in place.) */
	PB_(find_idx)(&parent, key);
	parentb = PB_(as_branch)(parent.node);
	assert(parent.idx <= parent.node->size
		&& parentb->child[parent.idx] == rm.node);
	/* Sibling edges. */
	sibling.less = parent.idx ? parentb->child[parent.idx - 1] : 0;
	sibling.more = parent.idx < parent.node->size
		? parentb->child[parent.idx + 1] : 0;
	assert(sibling.less || sibling.more);
	printf(" parent %s:%u, siblings %s (size %u) and %s (size %u).\n",
		orcify(parent.node), parent.idx,
		orcify(sibling.less), sibling.less ? sibling.less->size : 0,
		orcify(sibling.more), sibling.more ? sibling.more->size : 0);
	/* It's not clear which of `{ <, <= }` would be better. */
	if((sibling.more ? sibling.more->size : 0)
		> (sibling.less ? sibling.less->size : 0)) goto balance_more;
	else goto balance_less;
balance_less: {
	const unsigned combined = rm.node->size + sibling.less->size;
	unsigned promote, more, transfer;
	printf("<balance_less> %s; less %s; combined %u\n",
		orcify(rm.node), orcify(sibling.less), combined);
	assert(parent.idx);
	if(combined < 2 * TREE_MIN + 1) goto merge_less; /* Don't have enough. */
	promote = (combined - 1 + 1) / 2, more = promote + 1;
	transfer = sibling.less->size - more;
	assert(transfer < TREE_MAX && rm.node->size <= TREE_MAX - transfer);
	printf(" promote %u, more %u, transfer %u\n", promote, more, transfer);
	/* Make way for the keys from the less. */
	printf(" rm %s from %u keys %u -> %u\n", orcify(rm.node), rm.idx + 1,
		rm.node->size - rm.idx - 1, rm.idx + 1 + transfer);
	memmove(rm.node->key + rm.idx + 1 + transfer, rm.node->key + rm.idx + 1,
		sizeof *rm.node->key * (rm.node->size - rm.idx - 1));
	/* fixme: And... */
	printf(" rm %s from 0 keys %u -> %u\n", orcify(rm.node), rm.idx,
		transfer + 1);
	memmove(rm.node->key + transfer + 1, rm.node->key,
		sizeof *rm.node->key * rm.idx);
	printf(" parent %s from %u one key <%u> -> %s:%u\n",
		orcify(parent.node), parent.idx - 1,
		parent.node->key[parent.idx - 1], orcify(rm.node), transfer);
	rm.node->key[transfer] = parent.node->key[parent.idx - 1];
	printf(" less %s from %u keys %u -> %s:%u\n",
		orcify(sibling.less), more, transfer, orcify(rm.node), 0);
	memcpy(rm.node->key, sibling.less->key + more,
		sizeof *sibling.less->key * transfer);
	printf(" less %s from %u one key <%u> -> %s:%u\n",
		orcify(sibling.less), promote, sibling.less->key[promote],
		orcify(parent.node), parent.idx - 1);
	parent.node->key[parent.idx - 1] = sibling.less->key[promote];
	if(rm.height) {
		/* THIS IS THE PROBLEM! RIGHT HERE! We are not moving it to the
		 right. */
		struct PB_(branch) *const lessb = PB_(as_branch)(sibling.less),
			*const rmb = PB_(as_branch)(rm.node);
		unsigned transferb = transfer + 1,
			leftbsize = rm.node->size - rm.idx - 1;
		printf(" transferring %u from %s to %s, leftbsize %u.\n",
			transferb, orcify(lessb), orcify(rmb), leftbsize);
		memmove(rmb->child + transferb + rm.idx,
			rmb->child + rm.idx + 1, sizeof *rmb->child * leftbsize);
		memmove(rmb->child + transferb, rmb->child,
			sizeof *lessb->child * rm.idx);
		memcpy(rmb->child, lessb->child + promote + 1,
			sizeof *lessb->child * transferb);
	}
	rm.node->size += transfer;
	sibling.less->size = (unsigned char)promote;
	goto end;
} balance_more: {
	const unsigned combined = rm.node->size + sibling.more->size;
	unsigned promote;
	printf("<balance_more> %s; more %s; combined %u\n",
		orcify(rm.node), orcify(sibling.more), combined);
	assert(rm.node->size);
	if(combined < 2 * TREE_MIN + 1) goto merge_more; /* Don't have enough. */
	promote = (combined - 1) / 2 - rm.node->size;
	assert(promote < TREE_MAX && rm.node->size <= TREE_MAX - promote);
	printf(" promote %u from more\n", promote);
	/* Delete key. */
	memmove(rm.node->key + rm.idx, rm.node->key + rm.idx + 1,
		sizeof *rm.node->key * (rm.node->size - rm.idx - 1));
	/* Demote into hole. */
	printf(" parent %s from %u one key <%u> -> rm %s %u\n", orcify(parent.node),
		parent.idx, parent.node->key[parent.idx],
		orcify(rm.node), rm.node->size);
	rm.node->key[rm.node->size - 1] = parent.node->key[parent.idx];
	/* Transfer some keys from more to child. */
	memcpy(rm.node->key + rm.node->size, sibling.more->key,
		sizeof *sibling.more->key * promote);
	/* Promote one key from more. */
	parent.node->key[parent.idx] = sibling.more->key[promote];
	/* Move back in more. */
	memmove(sibling.more->key, sibling.more->key + promote + 1,
		sizeof *sibling.more->key * (sibling.more->size - promote - 1));
	if(rm.height) {
		struct PB_(branch) *const moreb = PB_(as_branch)(sibling.more);
		assert(0);
	}
	rm.node->size += promote;
	sibling.more->size -= promote + 1;
	goto end;
} merge_less:
	printf("<merge_less> (%s, %s) through %s with %u keys <%d>\n",
		orcify(sibling.less), orcify(rm.node), orcify(parent.node),
		parent.node->size, parent.node->key[parent.idx - 1]);
	assert(parent.idx && parent.idx <= parent.node->size && parent.node->size
		&& rm.idx < rm.node->size && rm.node->size == TREE_MIN
		&& sibling.less->size == TREE_MIN
		&& sibling.less->size + rm.node->size <= TREE_MAX);
	/* There are (maybe) two spots that we can merge, this is the less. */
	parent.idx--;
	/* Bring down key from `parent` to append to `less`. */
	sibling.less->key[sibling.less->size] = parent.node->key[parent.idx];
	printf(" ori %u\n", sibling.less->key[sibling.less->size]);
	/* Copy the keys, leaving out deleted. */
	memcpy(sibling.less->key + sibling.less->size + 1, rm.node->key,
		sizeof *rm.node->key * rm.idx);
	memcpy(sibling.less->key + sibling.less->size + 1 + rm.idx,
		rm.node->key + rm.idx + 1,
		sizeof *rm.node->key * (rm.node->size - rm.idx - 1));
	printf(" now %u\n", sibling.less->key[sibling.less->size]);
	if(rm.height) { /* The `parent` links will have one less. Copying twice. */
		struct PB_(branch) *const lessb = PB_(as_branch)(sibling.less),
			*const rmb = PB_(as_branch)(rm.node);
		memcpy(lessb->child + sibling.less->size + 1, rmb->child,
			sizeof *rmb->child * rm.node->size); /* _Sic_. */
	}
	sibling.less->size += rm.node->size;
	/* Remove references to `rm` from `parent`. The parent will have one less
	 link than key (_ie_, an equal number.) This is good. */
	printf(" %u<-%u(%u)\n", parent.idx + 1, parent.idx + 2, parent.node->size - parent.idx - 1);
	memmove(parentb->child + parent.idx + 1, parentb->child + parent.idx + 2,
		sizeof *parentb->child * (parent.node->size - parent.idx - 1));
	printf(" ***FREE %s\n", orcify(rm.node)); /* WE FREE THE WRONG ONE? No. */
	free(rm.node);
	goto ascend;
merge_more:
	printf("<merge_more> (%s, %s) through %s with %u keys <%d>\n",
		orcify(rm.node), orcify(sibling.more), orcify(parent.node),
		parent.node->size, parent.node->key[parent.idx]);
	assert(parent.idx < parent.node->size && parent.node->size
		&& rm.idx < rm.node->size && rm.node->size == TREE_MIN
		&& sibling.more->size == TREE_MIN
		&& rm.node->size + sibling.more->size <= TREE_MAX); /* Violated bulk? */
	/* Remove `rm`. */
	memmove(rm.node->key + rm.idx, rm.node->key + rm.idx + 1,
		sizeof *rm.node->key * (rm.node->size - rm.idx - 1));
	/* Bring down key from `parent` to append to `rm`. */
	rm.node->key[rm.node->size - 1] = parent.node->key[parent.idx];
	/* Merge `more` into `rm`. */
	memcpy(rm.node->key + rm.node->size, sibling.more->key,
		sizeof *sibling.more->key * sibling.more->size);
	if(rm.height) { /* The `parent` links will have one less. */
		struct PB_(branch) *const rmb = PB_(as_branch)(rm.node),
			*const moreb = PB_(as_branch)(sibling.more);
		memcpy(rmb->child + rm.node->size, moreb->child,
			sizeof *moreb->child * (sibling.more->size + 1));
	}
	rm.node->size += sibling.more->size;
	/* Remove references to `more` from `parent`. The parent will have one less
	 link than key (_ie_, an equal number.) This is good. */
	printf(" %u<-%u(%u)\n", parent.idx + 1, parent.idx + 2, parent.node->size - parent.idx - 1);
	memmove(parentb->child + parent.idx + 1, parentb->child + parent.idx + 2,
		sizeof *parentb->child * (parent.node->size - parent.idx - 1));
	printf(" ***FREE %s\n", orcify(sibling.more));
	free(sibling.more);
	goto ascend;
ascend:
	printf("<ascend>\n");
	/* Fix the hole by moving it up the tree. */
	PB_(graph_usual)(tree, "graph/work1.gv");
	rm = parent;
	if(rm.node->size <= TREE_MIN) {
		if(!(parent.node = PB_(as_branch)(rm.node)->child[TREE_MAX])) {
			assert(tree->root.height == rm.height);
		} else {
			parent.height++;
		}
	} else {
		parent.node = 0;
	}
	goto upward;
space: /* Node is root or has more than `TREE_MIN`; branches taken care of. */
	printf("<space> %s\n", orcify(rm.node));
	assert(rm.node && rm.idx < rm.node->size
		&& (rm.node->size > TREE_MIN || rm.node == tree->root.node));
	memmove(rm.node->key + rm.idx, rm.node->key + rm.idx + 1,
		sizeof *rm.node->key * (rm.node->size - rm.idx - 1));
#ifdef TREE_VALUE
	memmove(rm.node->value + rm.idx, rm.node->value + rm.idx + 1,
		sizeof *rm.node->value * (rm.node->size - rm.idx - 1));
#endif
	if(!--rm.node->size) {
		assert(rm.node == tree->root.node);
		if(tree->root.height) {
			tree->root.node = PB_(as_branch)(rm.node)->child[0];
			tree->root.height--;
			printf(" ***freeing branch %s\n", orcify(rm.node));
			free(PB_(as_branch)(rm.node));
		} else { /* Just deleted the last one. Set flag for zero container. */
			printf(" empty %s\n", orcify(rm.node));
			tree->root.height = UINT_MAX;
		}
	}
	goto end;
end:
	printf("<end>\n");
	return 1;
}



/****************************/

/* All these are used in clone; it's convenient to use `\O(\log size)` stack
 space. [existing branches][new branches][existing leaves][new leaves] no */
struct PB_(scaffold) {
	struct tree_count victim, source;
	size_t no;
	struct PB_(node) **data;
	struct { struct PB_(node) **head, **fresh, **cursor; } branch, leaf;
};
static int PB_(count_r)(struct PB_(tree) tree, struct tree_count *const no) {
	assert(tree.node && tree.height);
	if(!++no->branches) return 0;
	if(tree.height == 1) {
		/* Overflow; aren't guaranteed against this. */
		if(no->leaves + tree.node->size + 1 < no->leaves) return 0;
		no->leaves += tree.node->size + 1;
	} else {
		unsigned char i;
		for(i = 0; i <= tree.node->size; i++) {
			struct PB_(tree) child;
			child.node = PB_(as_branch)(tree.node)->child[i];
			child.height = tree.height - 1;
			if(!PB_(count_r)(child, no)) return 0;
		}
	}
	return 1;
}
static int PB_(count)(const struct B_(tree) *const tree,
	struct tree_count *const no) {
	assert(tree && no);
	no->branches = no->leaves = 0;
	if(!tree->root.node) { /* Idle. */
	} else if(tree->root.height == UINT_MAX || !tree->root.height) {
		no->leaves = 1;
	} else { /* Complex. */
		struct PB_(tree) sub = tree->root;
		if(!PB_(count_r)(sub, no)) return 0;
	}
	return 1;
}
static void PB_(cannibalize_r)(struct PB_(ref) ref,
	struct PB_(scaffold) *const sc) {
	struct PB_(branch) *branch = PB_(as_branch)(ref.node);
	const int keep_branch = sc->branch.cursor < sc->branch.fresh;
	assert(ref.node && ref.height && sc);
	if(keep_branch) *sc->branch.cursor = ref.node, sc->branch.cursor++;
	if(ref.height == 1) { /* Children are leaves. */
		unsigned n;
		for(n = 0; n <= ref.node->size; n++) {
			const int keep_leaf = sc->leaf.cursor < sc->leaf.fresh;
			struct PB_(node) *child = branch->child[n];
			if(keep_leaf) *sc->leaf.cursor = child, sc->leaf.cursor++;
			else free(child);
		}
	} else while(ref.idx <= ref.node->size) {
		struct PB_(ref) child;
		child.node = PB_(as_branch)(ref.node)->child[ref.idx];
		child.height = ref.height - 1;
		child.idx = 0;
		PB_(cannibalize_r)(child, sc);
		ref.idx++;
	}
	if(!keep_branch) free(branch);
}
static void PB_(cannibalize)(const struct B_(tree) *const tree,
	struct PB_(scaffold) *const sc) {
	struct PB_(ref) ref;
	assert(tree && tree->root.height != UINT_MAX && sc);
	/* Nothing to cannibalize. */
	if(!sc->victim.branches && !sc->victim.leaves) return;
	assert(tree->root.node);
	ref.node = tree->root.node, ref.height = tree->root.height, ref.idx = 0;
	sc->branch.cursor = sc->branch.head;
	sc->leaf.cursor = sc->leaf.head;
	if(ref.height) {
		PB_(cannibalize_r)(ref, sc);
	} else { /* Just one leaf. */
		*sc->leaf.cursor = ref.node;
	}
}
static struct PB_(node) *PB_(clone_r)(struct PB_(tree) src,
	struct PB_(scaffold) *const sc) {
	struct PB_(node) *node;
	if(src.height) {
		struct PB_(branch) *const srcb = PB_(as_branch)(src.node),
			*const branch = PB_(as_branch)(node = *sc->branch.cursor++);
		unsigned i;
		struct PB_(tree) child;
		*node = *src.node; /* Copy node. */
		child.height = src.height - 1;
		for(i = 0; i <= src.node->size; i++) { /* Different links. */
			child.node = srcb->child[i];
			branch->child[i] = PB_(clone_r)(child, sc);
		}
	} else { /* Leaves. */
		node = *sc->leaf.cursor++;
		*node = *src.node;
	}
	return node;
}
static struct PB_(tree) PB_(clone)(const struct PB_(tree) *const src,
	struct PB_(scaffold) *const sc) {
	struct PB_(tree) sub;
	assert(src && src->node && sc);
	/* Go back to the beginning of the scaffold and pick off one by one. */
	sc->branch.cursor = sc->branch.head;
	sc->leaf.cursor = sc->leaf.head;
	sub.node = PB_(clone_r)(*src, sc);
	sub.height = src->height;
	/* Used up all of them. No concurrent modifications, please. */
	assert(sc->branch.cursor == sc->leaf.head
		&& sc->leaf.cursor == sc->data + sc->no);
	return sub;
}
/** `source` is copied to, and overwrites, `tree`.
 @param[source] In the case where it's null or idle, if `tree` is empty, then
 it continues to be.
 @return Success, otherwise `tree` is not modified.
 @throws[malloc] @throws[EDOM] `tree` is null. @throws[ERANGE] The size of
 `source` doesn't fit into `size_t`. @allow */
static int B_(tree_clone)(struct B_(tree) *const tree,
	const struct B_(tree) *const source) {
	struct PB_(scaffold) sc;
	int success = 1;
	sc.data = 0; /* Need to keep this updated to catch. */
	if(!tree) { errno = EDOM; goto catch; }
	/* Count the number of nodes and set up to copy. */
	if(!PB_(count)(tree, &sc.victim) || !PB_(count)(source, &sc.source)
		|| (sc.no = sc.source.branches + sc.source.leaves) < sc.source.branches)
		{ errno = ERANGE; goto catch; } /* Overflow. */
	printf("<B>tree_clone: victim.branches %zu; victim.leaves %zu; "
		"source.branches %zu; source.leaves %zu.\n", sc.victim.branches,
		sc.victim.leaves, sc.source.branches, sc.source.leaves);
	if(!sc.no) { PB_(clear)(tree); goto finally; } /* No need to allocate. */
	if(!(sc.data = malloc(sizeof *sc.data * sc.no)))
		{ if(!errno) errno = ERANGE; goto catch; }
	{ /* Makes debugging easier; not necessary. */
		size_t i;
		for(i = 0; i < sc.no; i++) sc.data[i] = 0;
	}
	{ /* Ready scaffold. */
		struct tree_count need;
		need.leaves = sc.source.leaves > sc.victim.leaves
			? sc.source.leaves - sc.victim.leaves : 0;
		need.branches = sc.source.branches > sc.victim.branches
			? sc.source.branches - sc.victim.branches : 0;
		sc.branch.head = sc.data;
		sc.branch.fresh = sc.branch.cursor
			= sc.branch.head + sc.source.branches - need.branches;
		sc.leaf.head = sc.branch.fresh + need.branches;
		sc.leaf.fresh = sc.leaf.cursor
			= sc.leaf.head + sc.source.leaves - need.leaves;
		assert(sc.leaf.fresh + need.leaves == sc.data + sc.no);
	}
	/* Add new nodes. */
	while(sc.branch.cursor != sc.leaf.head) {
		struct PB_(branch) *branch;
		if(!(branch = malloc(sizeof *branch))) goto catch;
		branch->base.size = 0;
		branch->child[0] = 0;
		*sc.branch.cursor++ = &branch->base;
	}
	while(sc.leaf.cursor != sc.data + sc.no) {
		struct PB_(node) *leaf;
		if(!(leaf = malloc(sizeof *leaf))) goto catch;
		leaf->size = 0;
		*sc.leaf.cursor++ = leaf;
	}
	/* Resources acquired; now we don't care about tree. */
	PB_(cannibalize)(tree, &sc);
	/* The scaffold has the exact number of nodes we need. Overwrite. */
	tree->root = PB_(clone)(&source->root, &sc);
	goto finally;
catch:
	success = 0;
	if(!sc.data) goto finally;
	while(sc.leaf.cursor != sc.leaf.fresh) {
		struct PB_(node) *leaf = *(--sc.leaf.cursor);
		assert(leaf);
		free(leaf);
	}
	while(sc.branch.cursor != sc.branch.fresh) {
		struct PB_(branch) *branch = PB_(as_branch)(*(--sc.branch.cursor));
		assert(branch);
		free(branch);
	}
finally:
	free(sc.data); /* Temporary memory. */
	return success;
}

#ifdef TREE_TEST /* <!-- test */
/* Forward-declare. */
static void (*PB_(to_string))(PB_(entry_c), char (*)[12]);
static const char *(*PB_(tree_to_string))(const struct B_(tree) *);
#include "../test/test_tree.h"
#endif /* test --> */

static void PB_(unused_base_coda)(void);
static void PB_(unused_base)(void) {
	PB_(key) k;
	memset(&k, 0, sizeof k);
	PB_(is_element_c); PB_(forward); PB_(next_c); PB_(is_element);
	B_(tree)(); B_(tree_)(0); B_(tree_iterator)(0); B_(tree_next)(0);
	B_(tree_clear)(0);
	B_(tree_lower_iterator)(0, k); B_(tree_lower_value)(0, k);
#ifdef TREE_VALUE
	B_(tree_bulk_add)(0, k, 0); B_(tree_add)(0, k, 0);
#else
	B_(tree_bulk_add)(0, k); B_(tree_add)(0, k);
#endif
	B_(tree_bulk_finish)(0); B_(tree_remove)(0, k); B_(tree_clone)(0, 0);
	PB_(unused_base_coda)();
}
static void PB_(unused_base_coda)(void) { PB_(unused_base)(); }


#elif defined(TREE_TO_STRING) /* base code --><!-- to string trait */


#ifdef TREE_TO_STRING_NAME
#define STR_(n) TREE_CAT(B_(tree), TREE_CAT(TREE_TO_STRING_NAME, n))
#else
#define STR_(n) TREE_CAT(B_(tree), n)
#endif
#define TO_STRING TREE_TO_STRING
#define TO_STRING_LEFT '{'
#define TO_STRING_RIGHT '}'
#include "to_string.h" /** \include */
#ifdef TREE_TEST /* <!-- expect: greedy satisfy forward-declared. */
#undef TREE_TEST
static PSTR_(to_string_fn) PB_(to_string) = PSTR_(to_string);
static const char *(*PB_(tree_to_string))(const struct B_(tree) *)
	= &STR_(to_string);
#endif /* expect --> */
#undef STR_
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
#undef TREE_ORDER
#undef TREE_NAME
#undef TREE_KEY
#undef TREE_COMPARE
#ifdef TREE_VALUE
#undef TREE_VALUE
#endif
#ifdef TREE_TEST
#undef TREE_TEST
#endif
#undef BOX_
#undef BOX
#undef BOX_CONTENT
#undef BOX_ITERATOR
#endif /* !trait --> */
#undef TREE_TO_STRING_TRAIT
#undef TREE_TRAITS
