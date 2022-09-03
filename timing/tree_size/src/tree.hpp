/** @license 2022 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @abstract Stand-alone header <src/tree.h>; examples <test/test_tree.c>;
 article <doc/tree.pdf>. On a compatible workstation, `make` creates the test
 suite of the examples.

 @subtitle Ordered tree

 ![Example of Tree](../doc/tree.png)

 A <tag:<B>tree> is an ordered set or map contained in a tree. For memory
 locality, this is implemented B-tree, described in
 <Bayer, McCreight, 1972, Large>.

 @param[TREE_NAME, TREE_KEY]
 `<B>` that satisfies `C` naming conventions when mangled, required, and
 `TREE_KEY`, a type, <typedef:<PB>key>, whose default is `unsigned int`.
 `<PB>` is private, whose names are prefixed in a manner to avoid collisions.

 @param[TREE_VALUE]
 `TRIE_VALUE` is an optional payload to go with the type, <typedef:<PB>value>.
 The makes it a map of <tag:<B>tree_entry> instead of a set.

 @param[TREE_COMPARE]
 A function satisfying <typedef:<PB>compare_fn>. Defaults to ascending order.
 Required if `TREE_KEY` is changed to an incomparable type.

 @param[TREE_ORDER]
 Sets the branching factor, or order as <Knuth, 1998 Art 3>, to the range
 `[3, UINT_MAX+1]`. Default is most likely fine except when specific
 constraints have to be met; for example, an isomorphism to red-black trees
 sets `TREE_ORDER` to 4.

 @param[TREE_EXPECT_TRAIT]
 Do not un-define certain variables for subsequent inclusion in a parameterized
 trait.

 @param[TREE_TO_STRING_NAME, TREE_TO_STRING]
 To string trait contained in <src/to_string.h>; an optional unique `<SZ>`
 that satisfies `C` naming conventions when mangled and function implementing
 <typedef:<PSTR>to_string_fn>.

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
#include <stddef.h> /* That's weird. */
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
#define TREE_SPLIT (TREE_ORDER / 2) /* Split index: even order left-leaning. */
#define TREE_RESULT X(ERROR), X(UNIQUE), X(PRESENT)
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
struct tree_node_count { size_t branches, leaves; };
#endif /* idempotent --> */


#if TREE_TRAITS == 0 /* <!-- base code */


#ifndef TREE_ORDER
#define TREE_ORDER 33 /* Maximum branching factor. fixme: experiment. */
#endif
#if TREE_ORDER < 3 || TREE_ORDER > UINT_MAX + 1
#error TREE_ORDER parameter range `[3, UINT_MAX+1]`.
#endif
#ifndef TREE_KEY
#define TREE_KEY unsigned
#endif

/** Ordered type used by <typedef:<PB>compare_fn>; defaults to `unsigned`. */
typedef TREE_KEY PB_(key);
typedef const TREE_KEY PB_(key_c);

#ifdef TREE_VALUE
/** On `TREE_VALUE`, this creates a map, otherwise a set of
 <typedef:<PB>key>. */
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
 results in ascending order, `a > b`. @implements <typedef:<PB>compare_fn> */
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
	unsigned size; /* `[0, TREE_MAX]`. */
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
/* Address of a specific key by node. There is a need for node plus index
 without height, but we'll just let height be unused. */
struct PB_(ref) { struct PB_(node) *node; unsigned height, idx; };
struct PB_(ref_c) { const struct PB_(node) *node; unsigned height, idx; };
/* Node plus height is a sub-tree. A <tag:<B>tree> is a sub-tree of the tree. */
struct PB_(tree) { struct PB_(node) *node; unsigned height; };
/** To initialize it to an idle state, see <fn:<B>tree>, `{0}` (`C99`), or
 being `static`.

 ![States.](../doc/states.png) */
struct B_(tree);
struct B_(tree) { struct PB_(tree) root; };

#ifdef TREE_VALUE /* <!-- value */

/** On `TREE_VALUE`, creates a map from pointer-to-<typedef:<PB>key> to
 pointer-to-<typedef:<PB>value>. The reason these are pointers is because it
 is not contiguous in memory. */
struct B_(tree_entry) { PB_(key) *key; PB_(value) *value; };
struct B_(tree_entry_c) { PB_(key_c) *key; const PB_(value) *value; };
/** On `TREE_VALUE`, otherwise it's just an alias for
 pointer-to-<typedef:<PB>key>. */
typedef struct B_(tree_entry) PB_(entry);
typedef struct B_(tree_entry_c) PB_(entry_c);
static PB_(entry) PB_(null_entry)(void)
	{ const PB_(entry) e = { 0, 0 }; return e; }
static PB_(entry_c) PB_(null_entry_c)(void)
	{ const PB_(entry_c) e = { 0, 0 }; return e; }
/** Constructs entry from `node` and `i`. */
static PB_(entry) PB_(cons_entry)(struct PB_(node) *const node,
	const unsigned i) { PB_(entry) e;
	e.key = node->key + i, e.value = node->value + i; return e; }
/** Constructs entry from `node` and `i`. */
static PB_(entry_c) PB_(cons_entry_c)(const struct PB_(node) *const node,
	const unsigned i) { PB_(entry_c) e;
	e.key = node->key + i, e.value = node->value + i; return e; }
/** Gets the value of `ref`. */
static PB_(value) *PB_(ref_to_value)(const struct PB_(ref) ref)
	{ return ref.node ? ref.node->value + ref.idx : 0; }

#else /* value --><!-- !value */

typedef PB_(key) PB_(value);
typedef PB_(key) *PB_(entry);
typedef PB_(key_c) *PB_(entry_c);
static PB_(entry_c) PB_(null_entry_c)(void) { return 0; }
static PB_(entry) PB_(null_entry)(void) { return 0; }
/** Constructs entry from `node` and `i`. */
static PB_(entry) PB_(cons_entry)(struct PB_(node) *const node,
	const unsigned i) { return node->key + i; }
/** Constructs entry from `node` and `i`. */
static PB_(entry_c) PB_(cons_entry_c)(const struct PB_(node) *const node,
	const unsigned i) { return node->key + i; }
/** Gets the value of `ref`. */
static PB_(value) *PB_(ref_to_value)(const struct PB_(ref) ref)
	{ return ref.node ? ref.node->key + ref.idx : 0; }

#endif /* !value --> */

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
		unsigned a1 = tree.node->size; /* This is repeated code; sigh. */
		a0 = 0;
		while(a0 < a1) {
			const unsigned m = (a0 + a1) / 2;
			if(PB_(compare)(x, tree.node->key[m]) > 0) a0 = m + 1; else a1 = m;
		}
		if(a0)
			prev.node = tree.node, prev.height = tree.height, prev.idx = a0 - 1;
	}
	if(!prev.node) return 0; /* Off left. */
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
{ /* Re-descend; pick the minimum height node that has a next key. */ \
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
	if(!next.node) return 0; /* Off right. */ \
	*ref = next; \
}	return 1; /* Jumped nodes. */ \
}
TREE_TO_SUCCESSOR(to_successor, ref) /* For cursor. */
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
static struct PB_(forward) PB_(forward)(const struct B_(tree) *const tree) {
	struct PB_(forward) it;
	it.root = tree ? &tree->root : 0, it.next.node = 0;
	return it;
}
/** Move to next `it`. @return Element or null. @implements `next_c` */
static PB_(entry_c) PB_(next_c)(struct PB_(forward) *const it) {
	return assert(it), PB_(to_successor_c)(*it->root, &it->next) ?
		PB_(cons_entry_c)(it->next.node, it->next.idx) : PB_(null_entry_c)();
}

#define BOX_ITERATOR PB_(entry)
/** Is `e` not null? @implements `is_element` */
static int PB_(is_element)(const PB_(entry) e) {
#ifdef TREE_VALUE
	return !!e.key;
#else
	return !!e;
#endif
}

/* @implements `cursor` */
struct PB_(cursor) { struct PB_(tree) *root; struct PB_(ref) ref; int seen; };

/** Eliminates code-re-use from <fn:<PB>begin> and <fn:<PB>end>.
 @return Fills `it` and returns if `tree` has contents, in which case, `idx`
 is uninitialized. */
static int PB_(cursor_fill_part)(struct PB_(cursor) *const it,
	struct B_(tree) *const tree) {
	assert(it);
	it->seen = 0;
	if(!(it->root = tree ? &tree->root : 0)
		|| !(it->ref.node = tree->root.node)
		|| (it->ref.height = tree->root.height) == UINT_MAX) {
		it->ref.node = 0;
		it->ref.height = 0;
		it->ref.idx = 0;
		return 0;
	}
	return 1;
}
/** @return Before the start of `tree`, (can be null.) @implements `begin` */
static struct PB_(cursor) PB_(begin)(struct B_(tree) *const tree) {
	struct PB_(cursor) it;
	if(PB_(cursor_fill_part)(&it, tree)) {
		for(it.ref.node = tree->root.node; it.ref.height;
			it.ref.node = PB_(as_branch)(it.ref.node)->child[0], it.ref.height--);
		it.ref.idx = 0;
	}
	return it;
}
/** @return Iterator after the end of `tree`, (can be null.)
 @implements `end` */
static struct PB_(cursor) PB_(end)(struct B_(tree) *const tree) {
	struct PB_(cursor) it;
	if(PB_(cursor_fill_part)(&it, tree)) {
		for(it.ref.node = tree->root.node; it.ref.height;
			it.ref.node = PB_(as_branch)(it.ref.node)->child[it.ref.node->size],
			it.ref.height--);
		it.ref.idx = it.ref.node->size;
	}
	return it;
}

/** Advances `it`. @return Element or null. @implements `next` */
static PB_(entry) PB_(next)(struct PB_(cursor) *const it) {
	assert(it);
	if(!it->root || (it->seen || !it->ref.node)
		&& !PB_(to_successor)(*it->root, &it->ref))
		return it->ref.node = 0, it->seen = 0, PB_(null_entry)();
	assert(it->ref.node);
	return it->ref.idx < it->ref.node->size
		? (it->seen = 1, PB_(cons_entry)(it->ref.node, it->ref.idx))
		: (it->seen = 0, PB_(null_entry)());
}
/** Move to previous `it`. @return Element or null. @implements `previous` */
static PB_(entry) PB_(previous)(struct PB_(cursor) *const it) {
	assert(it);
	if(!it->root || !PB_(to_predecessor)(*it->root, &it->ref))
		return it->ref.node = 0, it->seen = 0, PB_(null_entry)();
	return it->seen = 1, PB_(cons_entry)(it->ref.node, it->ref.idx);
}

/* Want to find slightly different things; code re-use is bad. Confusing.
 This is the lower-bound. */
#define TREE_FORTREE(i) i.node = tree->node, i.height = tree->height; ; \
	i.node = PB_(as_branch_c)(i.node)->child[i.idx], i.height--
#define TREE_START(i) unsigned hi = i.node->size; i.idx = 0;
#define TREE_FORNODE(i) do { \
	const unsigned m = (i.idx + hi) / 2; \
	if(PB_(compare)(key, i.node->key[m]) > 0) i.idx = m + 1; \
	else hi = m; \
} while(i.idx < hi);
#define TREE_FLIPPED(i) PB_(compare)(i.node->key[i.idx], key) <= 0
/** Finds `key` in `lo` one node at a time. */
static void PB_(find_idx)(struct PB_(ref) *const lo, const PB_(key) key) {
	TREE_START((*lo))
	if(!lo) return;
	TREE_FORNODE((*lo))
}
/** Finds lower-bound of `key` in non-empty `tree`, or, if `key` is greater
 than all `tree`, one off the end. */
static struct PB_(ref) PB_(lower_r)(struct PB_(tree) *const tree,
	const PB_(key) key) {
	struct PB_(ref) i, lo = { 0, 0, 0 };
	for(TREE_FORTREE(i)) {
		TREE_START(i)
		if(!hi) continue;
		TREE_FORNODE(i)
		if(i.idx < i.node->size) {
			lo = i;
			if(TREE_FLIPPED(i)) break; /* Multi-keys go here. */
		}
		if(!i.height) {
			if(!lo.node) lo = i; /* Want one-off-end if last. */
			break;
		}
	}
	return lo;
}
/** @return Lower bound of `x` in `tree`. @order \O(\log |`tree`|) */
static struct PB_(ref) PB_(lower)(struct PB_(tree) tree, const PB_(key) x) {
	if(!tree.node || tree.height == UINT_MAX) {
		struct PB_(ref) ref; ref.node = 0; return ref;
	} else {
		return PB_(lower_r)(&tree, x);
	}
}
/** Finds an exact `key` in non-empty `tree`. */
static struct PB_(ref) PB_(find)(const struct PB_(tree) *const tree,
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
/** Finds lower-bound of `key` in non-empty `tree` while counting the
 non-filled `hole` and `is_equal`. */
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
/** Finds exact `key` in non-empty `tree`. If `node` is found, temporarily, the
 nodes that have `TREE_MIN` keys have
 `as_branch(node).child[TREE_MAX] = parent` or, for leaves, `leaf_parent`,
 which must be set. (Patently terrible for running concurrently; hack, would be
 nice to go down tree maybe.) */
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


/** Zeroed data (not all-bits-zero) is initialized. @return An idle tree.
 @order \Theta(1) @allow */
static struct B_(tree) B_(tree)(void) {
	struct B_(tree) tree;
	tree.root.node = 0; tree.root.height = 0;
	return tree;
}

/** Private: frees non-empty `tree` and it's children recursively, but doesn't
 put it to idle or clear pointers.
 @param[keep] Tries to keep one leaf if non-null. Set to null before. */
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
/** Private: `tree` can be null. */
static void PB_(clear)(struct B_(tree) *tree) {
	struct PB_(node) *one = 0;
	/* Already not there/idle/empty. */
	if(!tree || !tree->root.node || tree->root.height == UINT_MAX) return;
	PB_(clear_r)(tree->root, &one), assert(one);
	/* This is a special state where the tree has one leaf, but it is empty.
	 This state exists because it gives hysteresis to 0 -- 1 transition because
	 we have no advanced memory management. */
	tree->root.node = one;
	tree->root.height = UINT_MAX;
}
/** Returns an initialized `tree` to idle, `tree` can be null.
 @order \O(|`tree`|) @allow */
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

/** Clears `tree`, which can be null, idle, empty, or full. If it is empty or
 full, it remains active. @order \O(|`tree`|) @allow */
static void B_(tree_clear)(struct B_(tree) *const tree) { PB_(clear)(tree); }

/** Private: counts a sub-tree, `tree`. */
static size_t PB_(count_r)(const struct PB_(tree) tree) {
	size_t c = tree.node->size;
	if(tree.height) {
		const struct PB_(branch) *const branch = PB_(as_branch)(tree.node);
		struct PB_(tree) sub;
		size_t i;
		sub.height = tree.height - 1;
		for(i = 0; i <= tree.node->size; i++) {
			sub.node = branch->child[i];
			c += PB_(count_r)(sub);
		}
	}
	return c;
}
/** Counts all the keys on `tree`, which can be null.
 @order \O(|`tree`|) @allow */
static size_t B_(tree_count)(const struct B_(tree) *const tree) {
	return tree && tree->root.height != UINT_MAX
		? PB_(count_r)(tree->root) : 0;
}

/** @return Is `x` in `tree`? @order \O(\log |`tree`|) @allow */
static int B_(tree_contains)(const struct B_(tree) *const tree,
	const PB_(key) x) {
	return tree && tree->root.node && tree->root.height != UINT_MAX
		&& PB_(find)(&tree->root, x).node ? 1 : 0;
}

/** @return Get the value of `x` in `tree`, or if no `x`, null. The map type is
 a pointer to `TREE_VALUE` and the set type is a pointer to `TREE_KEY`.
 @order \O(\log |`tree`|) @allow */
static PB_(value) *B_(tree_get)(const struct B_(tree) *const tree,
	const PB_(key) x) {
	struct PB_(ref) ref;
	if(!tree || !tree->root.node || tree->root.height == UINT_MAX
		|| !(ref = PB_(find)(&tree->root, x)).node) return 0;
	return PB_(ref_to_value)(ref);
}

/** For example, `tree = { 10 }`, `x = 5 -> 10`, `x = 10 -> 10`,
 `x = 11 -> null`. (There is no upper value.)
 @return Lower-bound value match for `x` in `tree` or null if `x` is greater
 than all in `tree`.  The map type is a pointer to `TREE_VALUE` and the set
 type is a pointer to `TREE_KEY`. @order \O(\log |`tree`|) @allow */
static PB_(value) *B_(tree_at)(struct B_(tree) *const tree,
	const PB_(key) x) {
	struct PB_(ref) ref;
	if(!tree) return 0;
	ref = PB_(lower)(tree->root, x);
	return ref.node && ref.idx < ref.node->size ? PB_(ref_to_value)(ref) : 0;
}

#ifdef TREE_VALUE /* <!-- map */
/** Packs `key` on the right side of `tree` without doing the usual
 restructuring. All other topology modification functions should be avoided
 until followed by <fn:<B>tree_bulk_finish>.
 @param[value] A pointer to the key's value which is set by the function on
 returning true. A null pointer in this parameter causes the value to go
 uninitialized. This parameter is not there if one didn't specify `TREE_VALUE`.
 @return One of <tag:tree_result>: `TREE_ERROR` and `errno` will be set,
 `TREE_PRESENT` if the key is already (the highest) in the tree, and
 `TREE_UNIQUE`, added, the `value` (if applicable) is uninitialized.
 @throws[EDOM] `x` is smaller than the largest key in `tree`. @throws[malloc]
 @order \O(\log |`tree`|) @allow */
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
		if(!(node = (struct PB_(node) *)malloc(sizeof *node))) goto cat3h;
		node->size = 0;
		tree->root.node = node;
	} else if(tree->root.height == UINT_MAX) { /* Empty tree. */
		node = tree->root.node;
		tree->root.height = 0;
		node->size = 0;
	} else {
		struct PB_(tree) unfull = { 0, 0 };
		unsigned new_nodes, n; /* Count new nodes. */
		struct PB_(node) *tail = 0, *last = 0;
		struct PB_(branch) *pretail = 0;
		struct PB_(tree) scout;
		PB_(key) max;
		/* Right side bottom: `last` node with any keys, `unfull` not full. */
		for(scout = tree->root; ; scout.node = PB_(as_branch)(scout.node)
			->child[scout.node->size], scout.height--) {
			if(scout.node->size < TREE_MAX) unfull = scout;
			if(scout.node->size) last = scout.node;
			if(!scout.height) break;
		}
		assert(last), max = last->key[last->size - 1];
		if(PB_(compare)(max, key) > 0) return errno = EDOM, TREE_ERROR;
		if(PB_(compare)(key, max) <= 0) {
#ifdef TREE_VALUE
			if(value) {
				struct PB_(ref) max_ref;
				max_ref.node = last, max_ref.idx = last->size - 1;
				*value = PB_(ref_to_value)(max_ref);
			}
#endif
			return TREE_PRESENT;
		}

		/* One leaf, and the rest branches. */
		new_nodes = n = unfull.node ? unfull.height : tree->root.height + 2;
		if(!n) {
			node = unfull.node;
		} else {
			if(!(node = tail = (struct PB_(node) *)malloc(sizeof *tail))) goto cat3h;
			tail->size = 0;
			while(--n) {
				struct PB_(branch) *b;
				if(!(b = (struct PB_(branch) *)malloc(sizeof *b))) goto cat3h;
				b->base.size = 0;
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
			assert(new_nodes > 1);
			branch->child[1] = branch->child[0];
			branch->child[0] = tree->root.node;
			node = tree->root.node = head, tree->root.height++;
		} else if(unfull.height) { /* Add head to tree. */
			struct PB_(branch) *const branch
				= PB_(as_branch)(node = unfull.node);
			assert(new_nodes);
			branch->child[branch->base.size + 1] = head;
		}
	}
	assert(node && node->size < TREE_MAX);
	node->key[node->size] = key;
#ifdef TREE_VALUE
	if(value) {
		struct PB_(ref) max_ref;
		max_ref.node = node, max_ref.idx = node->size;
		*value = PB_(ref_to_value)(max_ref);
	}
#endif
	node->size++;
	return TREE_UNIQUE;
cat3h: /* Didn't work. Reset. */
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

/** Distributes `tree` (can be null) on the right side so that, after a series
 of <fn:<B>tree_bulk_add>, it will be consistent with the minimum number of
 keys in a node. @return The re-distribution was a success and all nodes are
 within rules. (Only when intermixing bulk and regular operations, can the
 function return false.) @order \O(\log |`tree`|) @allow */
static int B_(tree_bulk_finish)(struct B_(tree) *const tree) {
	struct PB_(tree) s;
	struct PB_(node) *right;
	if(!tree || !tree->root.node || tree->root.height == UINT_MAX) return 1;
	for(s = tree->root; s.height; s.node = right, s.height--) {
		unsigned distribute, right_want, right_move, take_sibling;
		struct PB_(branch) *parent = PB_(as_branch)(s.node);
		struct PB_(node) *sibling = (assert(parent->base.size),
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
#ifdef TREE_VALUE
		memmove(right->value + right_move, right->value,
			sizeof *right->value * right->size);
#endif
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
		memcpy(right->key + take_sibling,
			parent->base.key + parent->base.size - 1, sizeof *right->key);
#ifdef TREE_VALUE
		memcpy(right->value + take_sibling,
			parent->base.value + parent->base.size - 1, sizeof *right->value);
#endif
		/* Move the others from the sibling. */
		memcpy(right->key, sibling->key + sibling->size - take_sibling,
			sizeof *right->key * take_sibling);
#ifdef TREE_VALUE
		memcpy(right->value, sibling->value + sibling->size - take_sibling,
			sizeof *right->value * take_sibling);
#endif
		sibling->size -= take_sibling;
		/* Sibling's key is now the parent's. */
		memcpy(parent->base.key + parent->base.size - 1,
			sibling->key + sibling->size - 1, sizeof *right->key);
#ifdef TREE_VALUE
		memcpy(parent->base.value + parent->base.size - 1,
			sibling->value + sibling->size - 1, sizeof *right->value);
#endif
		sibling->size--;
	}
	return 1;
}

#ifdef TREE_VALUE /* <!-- map */
/** Adds or updates `key` in `root`. If not-null, `eject` will be the replaced
 key, otherwise don't replace. If `value` is not-null, sticks the associated
 value. */
static enum tree_result PB_(update)(struct PB_(tree) *const root,
	PB_(key) key, PB_(key) *const eject, PB_(value) **const value) {
#else /* map --><!-- set */
static enum tree_result PB_(update)(struct PB_(tree) *const root,
	PB_(key) key, PB_(key) *const eject) {
#endif /* set --> */
	struct PB_(node) *new_head = 0;
	struct PB_(ref) add, hole, cursor;
	assert(root);
	if(!(add.node = root->node)) goto idle;
	else if(root->height == UINT_MAX) goto empty;
	goto descend;
idle: /* No reserved memory. */
	assert(!add.node && !root->height);
	if(!(add.node = (struct PB_(node) *)malloc(sizeof *add.node))) goto cat3h;
	root->node = add.node;
	root->height = UINT_MAX;
	goto empty;
empty: /* Reserved dynamic memory, but tree is empty. */
	assert(add.node && root->height == UINT_MAX);
	add.height = root->height = 0;
	add.node->size = 0;
	add.idx = 0;
	goto insert;
descend: /* Record last node that has space. */
	{
		int is_equal = 0;
		add = PB_(lookup_insert)(root, key, &hole, &is_equal);
		if(is_equal) {
			if(eject) {
				*eject = add.node->key[add.idx];
				add.node->key[add.idx] = key;
			}
#ifdef TREE_VALUE
			if(value) *value = PB_(ref_to_value)(add);
#endif
			return TREE_PRESENT;
		}
	}
	if(hole.node == add.node) goto insert; else goto grow;
insert: /* Leaf has space to spare; usually end up here. */
	assert(add.node && add.idx <= add.node->size && add.node->size < TREE_MAX);
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
	unsigned new_no = hole.node ? hole.height : root->height + 2;
	struct PB_(node) **new_next = &new_head, *new_leaf;
	struct PB_(branch) *new_branch;
	assert(new_no);
	/* Allocate new nodes in succession. */
	while(new_no != 1) { /* All branches except one. */
		if(!(new_branch = (struct PB_(branch) *)malloc(sizeof *new_branch))) goto cat3h;
		new_branch->base.size = 0;
		new_branch->child[0] = 0;
		*new_next = &new_branch->base, new_next = new_branch->child;
		new_no--;
	}
	/* Last point of potential failure; (don't need to have entry in catch.) */
	if(!(new_leaf = (struct PB_(node) *)malloc(sizeof *new_leaf))) goto cat3h;
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
		hole.node = new_head, hole.height = ++root->height, hole.idx = 0;
		new_head = new_root->child[1] = new_root->child[0];
		new_root->child[0] = root->node, root->node = hole.node;
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
} cat3h: /* Didn't work. Reset. */
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

#ifdef TREE_VALUE /* <!-- map */
/** Adds or gets `key` in `tree`. If `key` is already in `tree`, uses the
 old value, _vs_ <fn:<B>tree_assign>. (This is only significant in trees with
 distinguishable keys.)
 @param[value] Only present if `TREE_VALUE` (map) was specified. If this
 parameter is non-null and a return value other then `TREE_ERROR`, this
 receives the address of the value associated with the `key`. This pointer is
 only guaranteed to be valid only while the `tree` doesn't undergo
 structural changes, (such as calling <fn:<B>tree_try> with `TREE_UNIQUE`
 again.)
 @return Either `TREE_ERROR` (false) and doesn't touch `tree`, `TREE_UNIQUE`
 and adds a new key with `key`, or `TREE_PRESENT` there was already an existing
 key. @throws[malloc] @order \Theta(\log |`tree`|) @allow */
static enum tree_result B_(tree_try)(struct B_(tree) *const tree,
	const PB_(key) key, PB_(value) **const value)
	{ return assert(tree), PB_(update)(&tree->root, key, 0, value); }
#else /* map --><!-- set */
/** Adds `key` to `tree` but in a set. */
static enum tree_result B_(tree_try)(struct B_(tree) *const tree,
	const PB_(key) key)
	{ return assert(tree), PB_(update)(&tree->root, key, 0); }
#endif /* set --> */

#ifdef TREE_VALUE /* <!-- map */
/** Adds or updates `key` in `tree`.
 @param[eject] If this parameter is non-null and a return value of
 `TREE_PRESENT`, the old key is stored in `eject`, replaced by `key`. A null
 value indicates that on conflict, the new key yields to the old key, as <fn:<B>tree_try>. This is only significant in trees with distinguishable keys.
 @param[value] Only present if `TREE_VALUE` (map) was specified. If this
 parameter is non-null and a return value other then `TREE_ERROR`, this
 receives the address of the value associated with the key.
 @return Either `TREE_ERROR` (false,) `errno` is set and doesn't touch `tree`;
 `TREE_UNIQUE`, adds a new key; or `TREE_PRESENT`, there was already an existing key. @throws[malloc] @order \Theta(\log |`tree`|) @allow */
static enum tree_result B_(tree_assign)(struct B_(tree) *const tree,
	const PB_(key) key, PB_(key) *const eject, PB_(value) **const value)
	{ return assert(tree), PB_(update)(&tree->root, key, eject, value); }
#else /* map --><!-- set */
/** Replaces `eject` by `key` or adds `key` in `tree`, but in a set. */
static enum tree_result B_(tree_assign)(struct B_(tree) *const tree,
	const PB_(key) key, PB_(key) *const eject)
	{ return assert(tree), PB_(update)(&tree->root, key, eject); }
#endif /* set --> */

/** Removes `x` from `tree` which must have contents. */
static int PB_(remove)(struct PB_(tree) *const tree, const PB_(key) x) {
	struct PB_(ref) rm, parent /* Only if `key.size <= TREE_MIN`. */;
	struct PB_(branch) *parentb;
	struct { struct PB_(node) *less, *more; } sibling;
	PB_(key) provisional_x = x;
	parent.node = 0;
	assert(tree && tree->node && tree->height != UINT_MAX);
	/* Traverse down the tree until `key`, leaving breadcrumbs for parents of
	 minimum key nodes. */
	if(!(rm = PB_(lookup_remove)(tree, x, &parent.node)).node) return 0;
	/* Important when `rm = parent`; `find_idx` later. */
	parent.height = rm.height + 1;
	assert(rm.idx < rm.node->size);
	if(rm.height) goto branch; else goto upward;
branch: {
	struct { struct PB_(ref) leaf; struct PB_(node) *parent; unsigned top; }
		pred, succ, chosen;
	assert(rm.height);
	/* Predecessor leaf. */
	pred.leaf = rm, pred.top = UINT_MAX;
	do {
		struct PB_(node) *const up = pred.leaf.node;
		pred.leaf.node = PB_(as_branch_c)(pred.leaf.node)->child[pred.leaf.idx];
		pred.leaf.idx = pred.leaf.node->size;
		pred.leaf.height--;
		if(pred.leaf.node->size < TREE_MIN) /* Possible in bulk-add? */
			{ pred.leaf.node = 0; goto no_pred; }
		else if(pred.leaf.node->size > TREE_MIN) pred.top = pred.leaf.height;
		else if(pred.leaf.height)
			PB_(as_branch)(pred.leaf.node)->child[TREE_MAX] = up;
		else pred.parent = up;
	} while(pred.leaf.height);
	pred.leaf.idx--;
no_pred:
	/* Successor leaf. */
	succ.leaf = rm, succ.top = UINT_MAX;
	succ.leaf.idx++;
	do {
		struct PB_(node) *const up = succ.leaf.node;
		succ.leaf.node = PB_(as_branch_c)(succ.leaf.node)->child[succ.leaf.idx];
		succ.leaf.idx = 0;
		succ.leaf.height--;
		if(succ.leaf.node->size < TREE_MIN)
			{ succ.leaf.node = 0; goto no_succ; }
		else if(succ.leaf.node->size > TREE_MIN) succ.top = succ.leaf.height;
		else if(succ.leaf.height)
			PB_(as_branch)(succ.leaf.node)->child[TREE_MAX] = up;
		else succ.parent = up;
	} while(succ.leaf.height);
no_succ:
	/* Choose the predecessor or successor. */
	if(!pred.leaf.node) {
		assert(succ.leaf.node);
		chosen = succ;
	} else if(!succ.leaf.node) {
		assert(pred.leaf.node);
		chosen = pred;
	} else if(pred.leaf.node->size < succ.leaf.node->size) {
		chosen = succ;
	} else if(pred.leaf.node->size > succ.leaf.node->size) {
		chosen = pred;
	} else if(pred.top > succ.top) {
		chosen = succ;
	} else {
		chosen = pred;
	}
	/* Replace `rm` with the predecessor or the successor leaf. */
	provisional_x = rm.node->key[rm.idx]
		= chosen.leaf.node->key[chosen.leaf.idx];
#ifdef TREE_VALUE
	rm.node->value[rm.idx] = chosen.leaf.node->value[chosen.leaf.idx];
#endif
	rm = chosen.leaf;
	if(chosen.leaf.node->size <= TREE_MIN) parent.node = chosen.parent;
	parent.height = 1;
	goto upward;
} upward: /* The first iteration, this will be a leaf. */
	assert(rm.node);
	if(!parent.node) goto space;
	assert(rm.node->size <= TREE_MIN); /* Condition on `parent.node`. */
	/* Retrieve forgotten information about the index in parent. (This is not
	 as fast at it could be, but holding parent data in minimum keys allows it
	 to be in place, if a hack. We could go down, but new problems arise.) */
	PB_(find_idx)(&parent, provisional_x);
	parentb = PB_(as_branch)(parent.node);
	assert(parent.idx <= parent.node->size
		&& parentb->child[parent.idx] == rm.node);
	/* Sibling edges. */
	sibling.less = parent.idx ? parentb->child[parent.idx - 1] : 0;
	sibling.more = parent.idx < parent.node->size
		? parentb->child[parent.idx + 1] : 0;
	assert(sibling.less || sibling.more);
	/* It's not clear which of `{ <, <= }` would be better. */
	if((sibling.more ? sibling.more->size : 0)
		> (sibling.less ? sibling.less->size : 0)) goto balance_more;
	else goto balance_less;
balance_less: {
	const unsigned combined = rm.node->size + sibling.less->size;
	unsigned promote, more, transfer;
	assert(parent.idx);
	if(combined < 2 * TREE_MIN + 1) goto merge_less; /* Don't have enough. */
	assert(sibling.less->size > TREE_MIN); /* Since `rm.size <= TREE_MIN`. */
	promote = (combined - 1 + 1) / 2, more = promote + 1;
	transfer = sibling.less->size - more;
	assert(transfer < TREE_MAX && rm.node->size <= TREE_MAX - transfer);
	/* Make way for the keys from the less. */
	memmove(rm.node->key + rm.idx + 1 + transfer, rm.node->key + rm.idx + 1,
		sizeof *rm.node->key * (rm.node->size - rm.idx - 1));
	memmove(rm.node->key + transfer + 1, rm.node->key,
		sizeof *rm.node->key * rm.idx);
	rm.node->key[transfer] = parent.node->key[parent.idx - 1];
	memcpy(rm.node->key, sibling.less->key + more,
		sizeof *sibling.less->key * transfer);
	parent.node->key[parent.idx - 1] = sibling.less->key[promote];
#ifdef TREE_VALUE
	memmove(rm.node->value + rm.idx + 1 + transfer, rm.node->value + rm.idx + 1,
		sizeof *rm.node->value * (rm.node->size - rm.idx - 1));
	memmove(rm.node->value + transfer + 1, rm.node->value,
		sizeof *rm.node->value * rm.idx);
	rm.node->value[transfer] = parent.node->value[parent.idx - 1];
	memcpy(rm.node->value, sibling.less->value + more,
		sizeof *sibling.less->value * transfer);
	parent.node->value[parent.idx - 1] = sibling.less->value[promote];
#endif
	if(rm.height) {
		struct PB_(branch) *const lessb = PB_(as_branch)(sibling.less),
			*const rmb = PB_(as_branch)(rm.node);
		unsigned transferb = transfer + 1;
		/* This is already moved; inefficient. */
		memmove(rmb->child + transferb, rmb->child,
			sizeof *rmb->child * (rm.node->size + 1 - 1));
		memcpy(rmb->child, lessb->child + promote + 1,
			sizeof *lessb->child * transferb);
	}
	rm.node->size += transfer;
	sibling.less->size = promote;
	goto end;
} balance_more: {
	const unsigned combined = rm.node->size + sibling.more->size;
	unsigned promote;
	assert(rm.node->size);
	if(combined < 2 * TREE_MIN + 1) goto merge_more; /* Don't have enough. */
	assert(sibling.more->size > TREE_MIN); /* Since `rm.size <= TREE_MIN`. */
	promote = (combined - 1) / 2 - rm.node->size; /* In `more`. Could be +1. */
	assert(promote < TREE_MAX && rm.node->size <= TREE_MAX - promote);
	/* Delete key. */
	memmove(rm.node->key + rm.idx, rm.node->key + rm.idx + 1,
		sizeof *rm.node->key * (rm.node->size - rm.idx - 1));
	/* Demote into hole. */
	rm.node->key[rm.node->size - 1] = parent.node->key[parent.idx];
	/* Transfer some keys from more to child. */
	memcpy(rm.node->key + rm.node->size, sibling.more->key,
		sizeof *sibling.more->key * promote);
	/* Promote one key from more. */
	parent.node->key[parent.idx] = sibling.more->key[promote];
	/* Move back in more. */
	memmove(sibling.more->key, sibling.more->key + promote + 1,
		sizeof *sibling.more->key * (sibling.more->size - promote - 1));
#ifdef TREE_VALUE
	memmove(rm.node->value + rm.idx, rm.node->value + rm.idx + 1,
		sizeof *rm.node->value * (rm.node->size - rm.idx - 1));
	rm.node->value[rm.node->size - 1] = parent.node->value[parent.idx];
	memcpy(rm.node->value + rm.node->size, sibling.more->value,
		sizeof *sibling.more->value * promote);
	parent.node->value[parent.idx] = sibling.more->value[promote];
	memmove(sibling.more->value, sibling.more->value + promote + 1,
		sizeof *sibling.more->value * (sibling.more->size - promote - 1));
#endif
	if(rm.height) {
		struct PB_(branch) *const moreb = PB_(as_branch)(sibling.more),
			*const rmb = PB_(as_branch)(rm.node);
		unsigned transferb = promote + 1;
		/* This is already moved; inefficient. */
		memcpy(rmb->child + rm.node->size, moreb->child,
			sizeof *moreb->child * transferb);
		memmove(moreb->child, moreb->child + transferb,
			sizeof *rmb->child * (moreb->base.size + 1 - transferb));
	}
	rm.node->size += promote;
	sibling.more->size -= promote + 1;
	goto end;
} merge_less:
	assert(parent.idx && parent.idx <= parent.node->size && parent.node->size
		&& rm.idx < rm.node->size && rm.node->size == TREE_MIN
		&& sibling.less->size == TREE_MIN
		&& sibling.less->size + rm.node->size <= TREE_MAX);
	/* There are (maybe) two spots that we can merge, this is the less. */
	parent.idx--;
	/* Bring down key from `parent` to append to `less`. */
	sibling.less->key[sibling.less->size] = parent.node->key[parent.idx];
	/* Copy the keys, leaving out deleted. */
	memcpy(sibling.less->key + sibling.less->size + 1, rm.node->key,
		sizeof *rm.node->key * rm.idx);
	memcpy(sibling.less->key + sibling.less->size + 1 + rm.idx,
		rm.node->key + rm.idx + 1,
		sizeof *rm.node->key * (rm.node->size - rm.idx - 1));
#ifdef TREE_VALUE
	sibling.less->value[sibling.less->size] = parent.node->value[parent.idx];
	memcpy(sibling.less->value + sibling.less->size + 1, rm.node->value,
		sizeof *rm.node->value * rm.idx);
	memcpy(sibling.less->value + sibling.less->size + 1 + rm.idx,
		rm.node->value + rm.idx + 1,
		sizeof *rm.node->value * (rm.node->size - rm.idx - 1));
#endif
	if(rm.height) { /* The `parent` links will have one less. Copying twice. */
		struct PB_(branch) *const lessb = PB_(as_branch)(sibling.less),
			*const rmb = PB_(as_branch)(rm.node);
		memcpy(lessb->child + sibling.less->size + 1, rmb->child,
			sizeof *rmb->child * rm.node->size); /* _Sic_. */
	}
	sibling.less->size += rm.node->size;
	/* Remove references to `rm` from `parent`. The parent will have one less
	 link than key (_ie_, an equal number.) This is by design. */
	memmove(parentb->child + parent.idx + 1, parentb->child + parent.idx + 2,
		sizeof *parentb->child * (parent.node->size - parent.idx - 1));
	/* This is the same pointer, but future-proof. */
	if(rm.height) free(PB_(as_branch)(rm.node)); else free(rm.node);
	goto ascend;
merge_more:
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
#ifdef TREE_VALUE
	memmove(rm.node->value + rm.idx, rm.node->value + rm.idx + 1,
		sizeof *rm.node->value * (rm.node->size - rm.idx - 1));
	rm.node->value[rm.node->size - 1] = parent.node->value[parent.idx];
	memcpy(rm.node->value + rm.node->size, sibling.more->value,
		sizeof *sibling.more->value * sibling.more->size);
#endif
	if(rm.height) { /* The `parent` links will have one less. */
		struct PB_(branch) *const rmb = PB_(as_branch)(rm.node),
			*const moreb = PB_(as_branch)(sibling.more);
		memcpy(rmb->child + rm.node->size, moreb->child,
			sizeof *moreb->child * (sibling.more->size + 1));
	}
	rm.node->size += sibling.more->size;
	/* Remove references to `more` from `parent`. The parent will have one less
	 link than key (_ie_, an equal number.) This is by design. */
	memmove(parentb->child + parent.idx + 1, parentb->child + parent.idx + 2,
		sizeof *parentb->child * (parent.node->size - parent.idx - 1));
	/* This is the same pointer, but future-proof. */
	if(rm.height) free(PB_(as_branch)(sibling.more)); else free(sibling.more);
	goto ascend;
ascend:
	/* Fix the hole by moving it up the tree. */
	rm = parent;
	if(rm.node->size <= TREE_MIN) {
		if(!(parent.node = PB_(as_branch)(rm.node)->child[TREE_MAX])) {
			assert(tree->height == rm.height);
		} else {
			parent.height++;
		}
	} else {
		parent.node = 0;
	}
	goto upward;
space: /* Node is root or has more than `TREE_MIN`; branches taken care of. */
	assert(rm.node);
	assert(rm.idx < rm.node->size);
	assert(rm.node->size > TREE_MIN || rm.node == tree->node);
	memmove(rm.node->key + rm.idx, rm.node->key + rm.idx + 1,
		sizeof *rm.node->key * (rm.node->size - rm.idx - 1));
#ifdef TREE_VALUE
	memmove(rm.node->value + rm.idx, rm.node->value + rm.idx + 1,
		sizeof *rm.node->value * (rm.node->size - rm.idx - 1));
#endif
	if(!--rm.node->size) {
		assert(rm.node == tree->node);
		if(tree->height) {
			tree->node = PB_(as_branch)(rm.node)->child[0];
			tree->height--;
			free(PB_(as_branch)(rm.node));
		} else { /* Just deleted the last one. Set flag for zero container. */
			tree->height = UINT_MAX;
		}
	}
	goto end;
end:
	return 1;
}
/** Tries to remove `key` from `tree`. @return Success, otherwise it was not in
 `tree`. @order \Theta(\log |`tree`|) @allow */
static int B_(tree_remove)(struct B_(tree) *const tree,
	const PB_(key) key) { return !!tree && !!tree->root.node
	&& tree->root.height != UINT_MAX && PB_(remove)(&tree->root, key); }

/* All these are used in clone; it's convenient to use `\O(\log size)` stack
 space. [existing branches][new branches][existing leaves][new leaves] no */
struct PB_(scaffold) {
	struct tree_node_count victim, source;
	size_t no;
	struct PB_(node) **data;
	struct { struct PB_(node) **head, **fresh, **cursor; } branch, leaf;
};
/** Counts the nodes `no` in `tree` for <fn:<PB>nodes>. */
static int PB_(nodes_r)(struct PB_(tree) tree,
	struct tree_node_count *const no) {
	assert(tree.node && tree.height);
	if(!++no->branches) return 0;
	if(tree.height == 1) {
		/* Overflow; aren't guaranteed against this. */
		if(no->leaves + tree.node->size + 1 < no->leaves) return 0;
		no->leaves += tree.node->size + 1;
	} else {
		unsigned i;
		for(i = 0; i <= tree.node->size; i++) {
			struct PB_(tree) child;
			child.node = PB_(as_branch)(tree.node)->child[i];
			child.height = tree.height - 1;
			if(!PB_(nodes_r)(child, no)) return 0;
		}
	}
	return 1;
}
/** Counts the nodes `no` in `tree`. */
static int PB_(nodes)(const struct B_(tree) *const tree,
	struct tree_node_count *const no) {
	assert(tree && no);
	no->branches = no->leaves = 0;
	if(!tree->root.node) { /* Idle. */
	} else if(tree->root.height == UINT_MAX || !tree->root.height) {
		no->leaves = 1;
	} else { /* Complex. */
		struct PB_(tree) sub = tree->root;
		if(!PB_(nodes_r)(sub, no)) return 0;
	}
	return 1;
}
/** `ref` with `sc` work under <fn:<PB>cannibalize>. */
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
/** Disassemble `tree` and put in into `sc`. */
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
/** Do the work of `src` cloned with `sc`. Called from <fn:<PB>clone>. */
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
/** `src` is copied with the cloning scaffold `sc`. */
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
 `source` nodes doesn't fit into `size_t`.
 @order \O(|`source`| + |`tree`|) @allow */
static int B_(tree_clone)(struct B_(tree) *const tree,
	const struct B_(tree) *const source) {
	struct PB_(scaffold) sc;
	int success = 1;
	sc.data = 0; /* Need to keep this updated to catch. */
	if(!tree) { errno = EDOM; goto cat3h; }
	/* Count the number of nodes and set up to copy. */
	if(!PB_(nodes)(tree, &sc.victim) || !PB_(nodes)(source, &sc.source)
		|| (sc.no = sc.source.branches + sc.source.leaves) < sc.source.branches)
		{ errno = ERANGE; goto cat3h; } /* Overflow. */
	if(!sc.no) { PB_(clear)(tree); goto finally; } /* No need to allocate. */
	if(!(sc.data = (struct PB_(node) **)malloc(sizeof *sc.data * sc.no)))
		{ if(!errno) errno = ERANGE; goto cat3h; }
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
		if(!(branch = (struct PB_(branch) *)malloc(sizeof *branch))) goto cat3h;
		branch->base.size = 0;
		branch->child[0] = 0;
		*sc.branch.cursor++ = &branch->base;
	}
	while(sc.leaf.cursor != sc.data + sc.no) {
		struct PB_(node) *leaf;
		if(!(leaf = (struct PB_(node) *)malloc(sizeof *leaf))) goto cat3h;
		leaf->size = 0;
		*sc.leaf.cursor++ = leaf;
	}
	/* Resources acquired; now we don't care about tree. */
	PB_(cannibalize)(tree, &sc);
	/* The scaffold has the exact number of nodes we need. Overwrite. */
	tree->root = PB_(clone)(&source->root, &sc);
	goto finally;
cat3h:
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


/* Box override information. */
#define BOX_ PB_
#define BOX struct B_(tree)


/** Adding, deleting, or changes in the topology of the tree invalidate it. */
struct B_(tree_cursor);
struct B_(tree_cursor) { struct PB_(cursor) _; };


/** @return Cursor before the first element of `tree`. Can be null.
 @order \Theta(\log |`tree`|) @allow */
static struct B_(tree_cursor) B_(tree_begin)(struct B_(tree) *const tree)
	{ struct B_(tree_cursor) cur; cur._ = PB_(begin)(tree); return cur; }
/** @param[tree] Can be null. @return Cursor in `tree` between elements, such
 that if <fn:<B>tree_next> is called, it will be smallest key that is not
 smaller than `x`, or, <fn:<B>tree_end> if `x` is greater than all in `tree`.
 @order \Theta(\log |`tree`|) @allow */
static struct B_(tree_cursor) B_(tree_begin_at)(struct B_(tree) *const tree,
	const PB_(key) x) {
	struct B_(tree_cursor) cur;
	if(!tree) return cur._.root = 0, cur;
	cur._.ref = PB_(lower)(tree->root, x);
	cur._.root = &tree->root;
	cur._.seen = 0;
	return cur;
}
/** @return Cursor after the last element of `tree`. Can be null.
 @order \Theta(\log |`tree`|) @allow */
static struct B_(tree_cursor) B_(tree_end)(struct B_(tree) *const tree)
	{ struct B_(tree_cursor) cur; cur._ = PB_(end)(tree); return cur; }
/** Advances `cur` to the next element. @return A pointer to the current
 element, or null if it ran out of elements. The type is either a set
 pointer-to-key or a map <tag:<B>tree_entry> (with `TREE_VALUE`, both fields
 are null if null). @order \O(\log |`tree`|) @allow */
static PB_(entry) B_(tree_next)(struct B_(tree_cursor) *const cur)
	{ return PB_(next)(&cur->_); }
/** Reverses `cur` to the previous element. @return A pointer to the previous
 element, or null if it ran out of elements. The type is either a set
 pointer-to-key or a map <tag:<B>tree_entry> (with `TREE_VALUE`, both fields
 are null if null). @order \O(\log |`tree`|) @allow */
static PB_(entry) B_(tree_previous)(struct B_(tree_cursor) *const cur)
	{ return PB_(previous)(&cur->_); }

#ifdef TREE_VALUE /* <!-- map */
/** Adds `key` and returns `value` to tree in cursor `cur`. See
 <fn:<B>tree_try>. @return If `cur` is not pointing at a valid tree, returns
 `TREE_ERROR` and doesn't set `errno`, otherwise the same. */
static enum tree_result B_(tree_cursor_try)(struct B_(tree_cursor) *const
	cur, const PB_(key) key, PB_(value) **const value) {
#else /* map --><!-- set */
static enum tree_result B_(tree_cursor_try)(struct B_(tree_cursor) *const
	cur, const PB_(key) key) {
#endif /* set --> */
	enum { NONODE, ITERATING, END } where;
	PB_(key) anchor;
	enum tree_result ret;
	memset(&anchor, 0, sizeof anchor); /* Silence warnings. */
	if(!cur || !cur->_.root) return TREE_ERROR; /* No tree. */
	if(cur->_.ref.node && cur->_.root->height != UINT_MAX) {
		where = (cur->_.ref.idx < cur->_.ref.node->size) ? ITERATING : END;
	} else {
		where = NONODE;
	}
	if(where == ITERATING) anchor = cur->_.ref.node->key[cur->_.ref.idx];
	if(where == NONODE || where == END) cur->_.seen = 0; /* Should be already. */
#ifdef TREE_VALUE
	ret = PB_(update)(cur->_.root, key, 0, value);
#else
	ret = PB_(update)(cur->_.root, key, 0);
#endif
	if(ret == TREE_ERROR) return TREE_ERROR;
	assert(cur->_.root->height != UINT_MAX); /* Can't be empty. */
	switch(where) {
	case NONODE: cur->_.ref.node = 0; cur->_.seen = 0; break;
	case ITERATING: cur->_.ref = PB_(lower)(*cur->_.root, anchor); break;
	case END:
		assert(cur->_.root->node);
		cur->_.ref.node = cur->_.root->node;
		cur->_.ref.height = cur->_.root->height;
		cur->_.ref.idx = cur->_.root->node->size;
		while(cur->_.ref.height) {
			cur->_.ref.node
				= PB_(as_branch_c)(cur->_.ref.node)->child[cur->_.ref.idx];
			cur->_.ref.idx = cur->_.ref.node->size;
			cur->_.ref.height--;
		}
		cur->_.seen = 0;
		break;
	}
	return ret;
#ifdef TREE_VALUE
}
#else
}
#endif

/** Removes the last entry returned by a valid `cur`. All other cursors on the
 same object are invalidated, but `cur` is now between on the removed node.
 @return Success, otherwise `cur` is not at a valid element.
 @order \Theta(\log |`tree`|) */
static int B_(tree_cursor_remove)(struct B_(tree_cursor) *const cur) {
	PB_(key) remove;
	if(!cur || !cur->_.seen || !cur->_.root || !cur->_.ref.node
		|| cur->_.root->height == UINT_MAX
		|| cur->_.ref.idx >= cur->_.ref.node->size
		|| (remove = cur->_.ref.node->key[cur->_.ref.idx],
		!PB_(remove)(cur->_.root, remove))) return 0;
	/* <fn:<B>tree_begin_at>. */
	cur->_.ref = PB_(lower)(*cur->_.root, remove);
	cur->_.seen = 0;
	return 1;
}

#ifdef TREE_TEST /* <!-- test */
/* Forward-declare. */
static void (*PB_(to_string))(PB_(entry_c), char (*)[12]);
static const char *(*PB_(tree_to_string))(const struct B_(tree) *);
#include "test_tree.h"
#endif /* test --> */


static void PB_(unused_base_coda)(void);
static void PB_(unused_base)(void) {
	PB_(key) k;
	memset(&k, 0, sizeof k);
	PB_(is_element_c); PB_(forward); PB_(next_c); PB_(is_element);
	B_(tree)(); B_(tree_)(0); B_(tree_clear)(0); B_(tree_count)(0);
	B_(tree_contains)(0, k); B_(tree_get)(0, k); B_(tree_at)(0, k);
#ifdef TREE_VALUE
	B_(tree_bulk_add)(0, k, 0); B_(tree_try)(0, k, 0);
	B_(tree_assign)(0, k, 0, 0); B_(tree_cursor_try)(0, k, 0);
#else
	B_(tree_bulk_add)(0, k); B_(tree_try)(0, k);
	B_(tree_assign)(0, k, 0); B_(tree_cursor_try)(0, k);
#endif
	B_(tree_bulk_finish)(0); B_(tree_remove)(0, k); B_(tree_clone)(0, 0);
	B_(tree_begin)(0); B_(tree_begin_at)(0, k); B_(tree_end)(0);
	B_(tree_previous)(0); B_(tree_next)(0);
	B_(tree_cursor_remove)(0);
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
/*static PSTR_(to_string_fn) PB_(to_string) = PSTR_(to_string);
static const char *(*PB_(tree_to_string))(const struct B_(tree) *)
	= &STR_(to_string); ??? */
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
