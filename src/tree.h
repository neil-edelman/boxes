/** @license 2022 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @abstract Stand-alone header <src/tree.h>; examples <test/test_tree.c>. On a
 compatible workstation, `make` creates the test suite of the examples.

 @subtitle Ordered tree

 A <tag:<B>tree> is an ordered set or map.

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

 @param[TREE_EXPECT_TRAIT]
 Do not un-define certain variables for subsequent inclusion in a parameterized
 trait.

 @param[TREE_TO_STRING_NAME, TREE_TO_STRING]
 To string trait contained in <to_string.h>; an optional unique `<SZ>`
 that satisfies `C` naming conventions when mangled and function implementing
 <typedef:<PSZ>to_string_fn>.

 @fixme multi-key; implementation of order statistic tree
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
/* Leaf: `TREE_MAX type`; branch: `TREE_MAX type + TREE_ORDER pointer`. */
#define TREE_MAX 2
#if TREE_MAX < 2 || TREE_MAX > UCHAR_MAX
#error TREE_MAX parameter range `[3, UCHAR_MAX]`.
#endif
/* This is the worst-case branching factor; the performance will be
 \O(log_{`TREE_MIN`+1} `size`). Usually this is `⌈(TREE_MAX+1)/2⌉-1`. However,
 smaller values are less-eager; this has been chosen to provide hysteresis. In
 the extreme, <Johnson, Shasha, 1993, Free-at-Empty> show good results. (Except
 `TREE_MAX 2`, one can be the only value.) */
#define TREE_MIN (TREE_MAX / 3 ? TREE_MAX / 3 : 1)
#if TREE_MIN == 0 || TREE_MIN > TREE_MAX / 2
#error TREE_MIN parameter range `[1, \floor(TREE_MAX / 2)]`.
#endif
#define TREE_ORDER (TREE_MAX + 1) /* Maximum degree, (branching factor.) */
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
#endif /* idempotent --> */


#if TREE_TRAITS == 0 /* <!-- base code */


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
 results in ascending order. @implements <typedef:<PH>compare_fn> */
static int PB_(default_compare)(PB_(key_c) a, PB_(key_c) b)
	{ return a > b; }
#define TREE_COMPARE &PB_(default_compare)
#endif /* !cmp --> */

/* Check that `TREE_COMPARE` is a function implementing
 <typedef:<PB>compare_fn>, if defined. */
static const PB_(compare_fn) PB_(compare) = (TREE_COMPARE);

/* B-tree node, as <Bayer, McCreight, 1972, Large>. These rules are more lazy
 than the original so as to not exhibit worst-case behaviour in small trees, as
 <Johnson, Shasha, 1993, Free-at-Empty>, but lookup is potentially slower after
 deleting; this is a design decision that nodes are not cached. In the
 terminology of <Knuth, 1998 Art 3>,
 * Every branch has at most `TREE_ORDER == TREE_MAX + 1` children, which is at
   minimum three.
 * Every non-root and non-bulk-loaded node has at least `TREE_MIN` keys,
   (`⎣TREE_MAX/3⎦`.)
 * Every branch has at least one child, `k`, and contains `k - 1` keys, (this
   is a consequence of the fact that they are implicitly storing a complete
   binary sub-tree.)
 * All leaves are at the maximum depth and height zero; they do'n't carry links
   to other nodes. (The height is one less then the original paper, as
   <Knuth, 1998 Art 3>, for computational simplicity.)
 * There are two empty B-trees to facilitate allocation hysteresis between
   0 -- 1: idle `{ 0, 0 }`, and `{ garbage leaf, UINT_MAX }`, one could test,
   `!root || height == UINT_MAX`.
 * Bulk-loading always is on the right side.
 * A branch node is a specialization of a (leaf) node with children. One can
   tell if it's a branch by the non-zero height. */
struct PB_(node) {
	unsigned char size; /* `[0, TREE_MAX]`. */
	PB_(key) key[TREE_MAX]; /* Cache-friendly lookup. */
#ifdef TREE_VALUE
	PB_(value) value[TREE_MAX];
#endif
};
/* B-tree branch is a <tag:<PB>node> and links to `size + 1` nodes. */
struct PB_(branch) { struct PB_(node) base, *child[TREE_ORDER]; };
/** @return Upcasts `as_node` to a branch. */
static struct PB_(branch) *PB_(branch)(struct PB_(node) *const as_leaf)
	{ return (struct PB_(branch) *)(void *)
	((char *)as_leaf - offsetof(struct PB_(branch), base)); }
/** @return Upcasts `as_node` to a branch. */
static const struct PB_(branch) *PB_(branch_c)(const struct PB_(node) *
	const as_node) { return (const struct PB_(branch) *)(const void *)
	((const char *)as_node - offsetof(struct PB_(branch), base)); }

/* Subtree is a node with a height. */
struct PB_(sub) { struct PB_(node) *node; unsigned height; };
/* Address specific entry. */
struct PB_(ref) { struct PB_(node) *node; unsigned height, idx; };
struct PB_(ref_c) { const struct PB_(node) *node; unsigned height, idx; };

#ifdef TREE_VALUE /* <!-- value */

/** On `TREE_VALUE`, creates a map from pointer-to-<typedef:<PB>key> to
 pointer-to-<typedef:<PB>value>. The reason these are pointers is because it
 is not connected in memory. */
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

/** To initialize it to an idle state, see <fn:<B>tree>, `TRIE_IDLE`, `{0}`
 (`C99`), or being `static`. This is a B-tree, as
 <Bayer, McCreight, 1972 Large>.

 ![States.](../doc/states.png) */
struct B_(tree);
struct B_(tree) { struct PB_(sub) root; };

#define BOX_CONTENT PB_(entry_c)
/** Is `e` not null? @implements `is_element_c` */
static int PB_(is_element_c)(PB_(entry_c) e) {
#ifdef TREE_VALUE
	return !!e.key;
#else
	return !!e;
#endif
}
/* Two copies of the same code, with and without `const`.
 @param[sub] A copy of the tree's root.
 @param[ref] If it has a null node, starts at the first key; if it's past the
 node's limits, uses `sub` to go to the next node.
 @return True unless there are no more `ref`. */
#define TREE_PIN(pin_c, ref_c) \
static int PB_(pin_c)(struct PB_(sub) sub, struct PB_(ref_c) *const ref) { \
	struct PB_(ref_c) next; \
	unsigned a0; \
	PB_(key) x; \
	assert(ref); \
	if(!sub.node || sub.height == UINT_MAX) return 0; \
	/* Start. */ \
	if(!ref->node) \
		ref->node = sub.node, ref->height = sub.height, ref->idx = 0; \
	/* Descend. */ \
	while(ref->height) ref->height--, \
		ref->node = PB_(branch_c)(ref->node)->child[ref->idx], ref->idx = 0; \
	if(ref->idx < ref->node->size) return 1; /* Likely. */ \
	/* Empty nodes are always at the end, (when bulk loading.) */ \
	if(!ref->node->size) return 0; \
	/* Re-descend tree and note the minimum height node that has a next key. */\
	for(next.node = 0, x = ref->node->key[ref->node->size - 1]; sub.height; \
		sub.node = PB_(branch_c)(sub.node)->child[a0], sub.height--) { \
		unsigned a1 = sub.node->size; a0 = 0; \
		while(a0 < a1) { \
			const unsigned m = (a0 + a1) / 2; \
			if(PB_(compare)(x, sub.node->key[m]) > 0) a0 = m + 1; else a1 = m; \
		} \
		if(a0 < sub.node->size) \
			next.node = sub.node, next.height = sub.height, next.idx = a0; \
	} \
	if(!next.node) return 0; /* Off the right. */ \
	*ref = next; \
	return 1; /* Jumped nodes. */ \
}
TREE_PIN(pin_c, ref_c)
TREE_PIN(pin, ref)
#undef TREE_PIN
/* This could be expanded! */

/* A constant iterator. @implements `forward` */
struct PB_(forward) { const struct PB_(sub) *root; struct PB_(ref_c) ref; };
/** @return Before `tree`. @implements `forward_begin` */
static struct PB_(forward) PB_(forward_begin)(const struct B_(tree) *const
	tree) {
	struct PB_(forward) it;
	it.root = tree ? &tree->root : 0, it.ref.node = 0,
		it.ref.height = 0, it.ref.idx = 0;
	return it;
}
/** Advances `it` to the next element. @return A pointer to the current
 element or null. @implements `forward_next` */
static PB_(entry_c) PB_(forward_next)(struct PB_(forward) *const it) {
	return assert(it), PB_(pin_c)(*it->root, &it->ref) ?
		PB_(leaf_to_entry_c)(it->ref.node, it->ref.idx++) : PB_(null_entry_c)();
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
/* A certain position and the top level tree for backtracking.
 @implements `iterator` */
struct PB_(iterator) { struct PB_(sub) *root; struct PB_(ref) ref; };
/** @return Before `tree`. @implements `forward_begin` */
static struct PB_(iterator) PB_(begin)(struct B_(tree) *const tree) {
	struct PB_(iterator) it;
	it.root = tree ? &tree->root : 0, it.ref.node = 0,
		it.ref.height = 0, it.ref.idx = 0;
	return it;
}
/** Advances `it` to the next element. @return A pointer to the current
 element or null. @implements `next` */
static PB_(entry) PB_(next)(struct PB_(iterator) *const it) {
	return assert(it), PB_(pin)(*it->root, &it->ref) ?
		PB_(leaf_to_entry)(it->ref.node, it->ref.idx++) : PB_(null_entry)();
}

#include "../test/orcish.h"

static void PB_(find_idx)(struct PB_(ref) *const lo, const PB_(key) key) {
	unsigned hi = lo->node->size;
	lo->idx = 0;
	if(!hi) return;
	do {
		const unsigned m = (lo->idx + hi) / 2;
		if(PB_(compare)(key, lo->node->key[m]) > 0) lo->idx = m + 1;
		else hi = m;
	} while(lo->idx < hi);
}

/** Assume `tree` and `x` are checked for non-empty validity. */
static struct PB_(ref) PB_(lower_r)(struct PB_(sub) *const tree,
	const PB_(key) key, struct PB_(ref) *const hole, int *const is_equal) {
	struct PB_(ref) lo;
	for(lo.node = tree->node, lo.height = tree->height; ;
		lo.node = PB_(branch_c)(lo.node)->child[lo.idx], lo.height--) {
		unsigned hi = lo.node->size;
		lo.idx = 0;
		if(hole && hi < TREE_MAX) *hole = lo;
		if(!hi) continue; /* No nodes; bulk-add? */
		do {
			const unsigned m = (lo.idx + hi) / 2;
			if(PB_(compare)(key, lo.node->key[m]) > 0) lo.idx = m + 1;
			else hi = m;
		} while(lo.idx < hi);
		if(hole && hi < TREE_MAX) hole->idx = lo.idx; /* Update. */
		if(!lo.height) break; /* Leaf node. */
		if(lo.idx == lo.node->size) continue; /* Off the end. */
		/* Total order and monotonic, otherwise have to check right. */
		if(PB_(compare)(lo.node->key[lo.idx], key) > 0) continue;
		if(is_equal) *is_equal = 1; /* Check right, multi-key, not yet. */
		break;
	}
	return lo;
}

/** @param[tree] Can be null. @return Lower bound of `x` in `tree`.
 @order \O(\log |`tree`|) */
static struct PB_(ref) PB_(lower)(struct PB_(sub) sub,
	const PB_(key) x, struct PB_(ref) *const unfull, int *const is_equal) {
	if(!sub.node || sub.height == UINT_MAX) {
		struct PB_(ref) ref; ref.node = 0; return ref;
	} else {
		return PB_(lower_r)(&sub, x, unfull, is_equal);
	}
}

/** Clears non-empty `tree` and it's children recursively, but doesn't put it
 to idle or clear pointers. If `one` is valid, tries to keep one leaf. */
static void PB_(clear_r)(struct PB_(sub) sub, struct PB_(node) **const one) {
	assert(sub.node);
	if(!sub.height) {
		if(one && !*one) *one = sub.node;
		else free(sub.node);
	} else {
		struct PB_(sub) child;
		unsigned i;
		child.height = sub.height - 1;
		for(i = 0; i <= sub.node->size; i++)
			child.node = PB_(branch)(sub.node)->child[i],
			PB_(clear_r)(child, one);
		free(PB_(branch)(sub.node));
	}
}

/* Box override information. */
#define BOX_ PB_
#define BOX struct B_(tree)

/** Initializes `tree` to idle. @order \Theta(1) @allow */
static struct B_(tree) B_(tree)(void)
	{ struct B_(tree) tree; tree.root.node = 0; tree.root.height = 0;
	return tree; }

/** Returns an initialized `tree` to idle, `tree` can be null. @allow */
static void B_(tree_)(struct B_(tree) *const tree) {
	if(!tree) return; /* Null. */
	if(!tree->root.node) { /* Idle. */
		assert(!tree->root.height);
	} else if(tree->root.height == UINT_MAX) { /* Empty. */
		assert(tree->root.node); free(tree->root.node);
	} else {
		PB_(clear_r)(tree->root, 0);
	}
	*tree = B_(tree)();
}

/** Stores an iteration in a tree. Generally, changes in the topology of the
 tree invalidate it. */
struct B_(tree_iterator) { struct PB_(iterator) _; };
/** @return An iterator before the first element of `tree`. Can be null.
 @allow */
static struct B_(tree_iterator) B_(tree_begin)(struct B_(tree) *const tree)
	{ struct B_(tree_iterator) it; it._ = PB_(begin)(tree); return it; }
/** Advances `it` to the next element. @return A pointer to the current
 element or null. @allow */
static PB_(entry) B_(tree_next)(struct B_(tree_iterator) *const it)
	{ return PB_(next)(&it->_); }

/** @param[tree] Can be null. @return Finds the smallest entry in `tree` that
 is at the lower bound of `x`. If `x` is higher than any of `tree`, it will be
 placed just passed the end. @order \O(\log |`tree`|) @allow */
static struct B_(tree_iterator) B_(tree_lower)(struct B_(tree) *const tree,
	const PB_(key) x) {
	struct B_(tree_iterator) it;
	if(!tree) return it._.root = 0, it;
	it._.ref = PB_(lower)(tree->root, x, 0, 0);
	it._.root = &tree->root;
	return it;
}

/** For example, `tree = { 10 }`, `x = 5 -> 10`, `x = 10 -> 10`,
 `x = 11 -> null`.
 @return Lower-bound value match for `x` in `tree` or null if `x` is greater
 than all in `tree`. @order \O(\log |`tree`|) @allow */
static PB_(value) *B_(tree_get_next)(struct B_(tree) *const tree,
	const PB_(key) x) {
	struct PB_(ref) ref;
	return tree && (ref = PB_(lower)(tree->root, x, 0, 0),
		PB_(pin)(tree->root, &ref)) ? PB_(ref_to_value)(ref) : 0;
}

#include "../test/orcish.h"
static void PB_(print)(const struct B_(tree) *const tree);
#ifndef TREE_TEST
static void PB_(print)(const struct B_(tree) *const tree)
	{ (void)tree, printf("not printable\n"); }
#endif

#ifdef TREE_VALUE /* <!-- map */
/** Packs `key` on the right side of `tree` without doing the usual
 restructuring. This is best followed by <fn:<B>tree_bulk_finish>.
 @param[value] A pointer to the key's value which is set by the function on
 returning true. A null pointer in this parameter causes the value to go
 uninitialized. This parameter is not there if one didn't specify `TREE_VALUE`.
 @return One of <tag:tree_result>: `TREE_ERROR` and `errno` will be set,
 `TREE_YIELD` if the key is already (the highest) in the tree, and
 `TREE_UNIQUE`, added, the `value` (if specified) is uninitialized.
 @throws[EDOM] `x` is smaller than the largest key in `tree`. @throws[malloc] */
static enum tree_result B_(tree_bulk_add)(struct B_(tree) *const tree,
	PB_(key) key, PB_(value) **const value)
#else /* map --><!-- set */
static enum tree_result B_(tree_bulk_add)(struct B_(tree) *const tree,
	PB_(key) key)
#endif
{
	struct PB_(node) *node = 0, *head = 0; /* The original and new. */
	assert(tree);
	if(!tree->root.node) { /* Idle tree. */
		assert(!tree->root.height);
		if(!(node = malloc(sizeof *node))) goto catch;
		node->size = 0;
		tree->root.node = node;
		printf("bulk: idle\n");
	} else if(tree->root.height == UINT_MAX) { /* Empty tree. */
		tree->root.height = 0;
		tree->root.node->size = 0;
		printf("bulk: empty\n");
	} else {
		struct PB_(sub) unfull = { 0, 0 };
		unsigned new_nodes, n; /* Count new nodes. */
		struct PB_(node) *tail = 0, *last = 0;
		struct PB_(branch) *pretail = 0;
		struct PB_(sub) scout;
		PB_(key) i;
		printf("bulk: tree...\n"), PB_(print)(tree);
		for(scout = tree->root; ; scout.node = PB_(branch)(scout.node)
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
			struct PB_(branch) *const branch = PB_(branch)(head);
			/*printf("adding the existing root, %s to %s\n",
				orcify(tree->root), orcify(head));*/
			assert(new_nodes > 1);
			branch->child[1] = branch->child[0];
			branch->child[0] = tree->root.node;
			node = tree->root.node = head, tree->root.height++;
		} else if(unfull.height) { /* Add head to tree. */
			struct PB_(branch) *const branch = PB_(branch)(node = unfull.node);
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
catch:
	free(node); /* Didn't work out. */
	while(head) {
		struct PB_(node) *const next = PB_(branch)(head)->child[0];
		free(head);
		head = next;
	}
	if(!errno) errno = ERANGE;
	return TREE_ERROR;
}

/** Distributes `tree` on the right side so that, after a series of
 <fn:<B>tree_bulk_add>, it will be consistent with the minimum number of keys
 in a node. @return The distribution was a success and all nodes are within
 rules. The only time that it would be false is if, maybe, a regular insertion
 instead of a bulk insertion was performed interspersed with a bulk insertion
 without calling this function. */
static int B_(tree_bulk_finish)(struct B_(tree) *const tree) {
	struct PB_(sub) s;
	struct PB_(node) *right;
	printf("tree_bulk_finish(%s) number of nodes [%u, %u]\n",
		orcify(tree), TREE_MIN, TREE_MAX);
	if(!tree || !tree->root.node || tree->root.height == UINT_MAX) return 1;
	for(s = tree->root; s.height; s.node = right, s.height--) {
		unsigned distribute, right_want, right_move, take_sibling;
		struct PB_(branch) *parent = PB_(branch)(s.node);
		struct PB_(node) *sibling = (assert(parent->base.size),
			parent->child[parent->base.size - 1]);
		right = parent->child[parent->base.size];
		printf("initial parent node %s:%u with %u size, children %s and %s.\n",
			orcify(s.node), s.height, s.node->size,
			orcify(sibling), orcify(right));
		if(TREE_MIN <= right->size)
			{ printf("cool\n"); continue; } /* Has enough. */
		distribute = sibling->size + right->size;
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
			struct PB_(branch) *rbranch = PB_(branch)(right),
				*sbranch = PB_(branch)(sibling);
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

static void PB_(graph)(const struct B_(tree) *const tree,
	const char *const fn);
#ifndef TREE_TEST
static void PB_(graph)(const struct B_(tree) *const tree,
	const char *const fn) { (void)tree, (void)fn; }
#endif

#ifdef TREE_VALUE /* <!-- map */
static enum tree_result B_(tree_add)(struct B_(tree) *const tree,
	PB_(key) key, PB_(value) **const value)
#else /* map --><!-- set */
static enum tree_result B_(tree_add)(struct B_(tree) *const tree,
	PB_(key) key)
#endif /* set --> */
{
	struct PB_(node) *new_head = 0;
	unsigned i;
	struct PB_(ref) add, hole, cursor;
	int is_equal;
	assert(tree);
	if(!(add.node = tree->root.node)) goto idle;
	else if(tree->root.height == UINT_MAX) goto empty;
	goto descend;
idle: /* No reserved memory. */
	assert(!add.node && !tree->root.height);
	if(!(add.node = malloc(sizeof *add.node))) goto catch;
	tree->root.node = add.node;
	tree->root.height = UINT_MAX;
	printf("add: idle tree, new %s.\n", orcify(add.node));
	goto empty;
empty: /* Reserved dynamic memory, but tree is empty. */
	assert(add.node && tree->root.height == UINT_MAX);
	tree->root.height = 0;
	add.node->size = 0;
	add.idx = 0;
	printf("add: empty tree, %s.\n", orcify(add.node));
	goto insert;
descend: /* Record last node that has space. */
	printf("add: contents...\n"), PB_(print)(tree);
	hole.node = 0, is_equal = 0;
	add = PB_(lower_r)(&tree->root, key, &hole, &is_equal);
	if(is_equal) {
		/* Assumes key is unique; we might not want this for multi-maps, but
		 that is not implemented yet. */
#ifdef TREE_VALUE
		if(value) *value = PB_(ref_to_value)(add);
#endif
		return TREE_YIELD;
	}
	printf("add: fill %s(%u):%u, add %s(%u):%u.\n",
		orcify(hole.node), hole.height, hole.idx,
		orcify(add.node), add.height, add.idx);
	if(hole.node == add.node) goto insert; else goto grow;
insert: /* `add` has space to spare; usually end up here. */
	assert(add.node && add.idx <= add.node->size && add.node->size < TREE_MAX
		&& !add.height);
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
	const unsigned new_no = hole.node ? hole.height + 1 : tree->root.height + 2;
	struct PB_(node) **new_next = &new_head, *new_leaf;
	struct PB_(branch) *new_branch;
	/* Allocate new nodes in succession. */
	for(i = 0; i < new_no - 1; i++) {
		if(!(new_branch = malloc(sizeof *new_branch))) goto catch;
		new_branch->base.size = 0;
		new_branch->child[0] = 0;
		*new_next = &new_branch->base, new_next = new_branch->child;
	}
	/* Last point of potential failure; (don't need to catch this.) */
	if(!(new_leaf = malloc(sizeof *new_leaf))) goto catch;
	new_leaf->size = 0;
	*new_next = new_leaf;
	printf("add: new { ");
	for(i = 0, cursor.node = new_head; i < new_no; i++, cursor.node
		= PB_(branch)(cursor.node)->child[0])
		printf("%s%s", i ? ", " : "", orcify(cursor.node));
	printf(" }.\n");
	/* Attach new nodes to the tree. The hole is now an actual hole. */
	if(hole.node) { /* New nodes are a sub-structure of the tree. */
		assert(0);
	} else { /* New nodes raise tree height. */
		struct PB_(branch) *const new_root = PB_(branch)(new_head);
		hole.node = new_head, hole.height = ++tree->root.height, hole.idx = 0;
		new_head = new_root->child[1] = new_root->child[0];
		new_root->child[0] = tree->root.node, tree->root.node = hole.node;
		hole.node->size = 1;
		printf("grow, now root %s.\n", orcify(new_root));
	}
	cursor = hole; /* Start at the hole, go down. */
	goto split;
} split: { /* Split between the new and existing nodes. */
	{
		char fn[64];
		sprintf(fn, "graph/topology-%u.gv", cursor.height);
		PB_(graph)(tree, fn);
	}
	struct PB_(node) *sibling;
	assert(cursor.node && cursor.node->size && cursor.height);
	sibling = new_head;
	/* Descend now while split hasn't happened -- easier. */
	new_head = --cursor.height ? PB_(branch)(new_head)->child[0] : 0;
	printf("new_head moved down to %s.\n", orcify(new_head));
	cursor.node = PB_(branch)(cursor.node)->child[cursor.idx];
	PB_(find_idx)(&cursor, key);
	printf("cursor %s:%u, idx %u; hole %s:%u, idx %u; sibling %s.\n",
		orcify(cursor.node), cursor.height, cursor.idx,
		orcify(hole.node), hole.height, hole.idx, orcify(sibling));
	assert(!sibling->size && cursor.node->size == TREE_MAX); /* Atomic. */
	if(cursor.idx == TREE_SPLIT) { /* Down the middle. */
		printf("down middle\n");
		sibling->size = TREE_MAX - TREE_SPLIT - 1;
		memcpy(sibling->key, cursor.node->key + TREE_SPLIT,
			sizeof *sibling->key * (TREE_MAX - TREE_SPLIT));
#ifdef TREE_VALUE
		memcpy(sibling->value, cursor.node->value + TREE_SPLIT,
			sizeof *sibling->value * (TREE_MAX - TREE_SPLIT));
#endif
		if(cursor.height) {
			struct PB_(branch) *const cbranch = PB_(branch)(cursor.node),
				*const sbranch = PB_(branch)(sibling);
			memcpy(sbranch->child + 1, cbranch->child + TREE_SPLIT + 1,
				sizeof *cbranch->child * (TREE_MAX - TREE_SPLIT));
		}
		cursor.node->size = TREE_SPLIT;
		sibling->size = TREE_MAX - TREE_SPLIT;
	} else if(cursor.idx < TREE_SPLIT) {
		assert(0);
	} else /* Greater than. */ {
		assert(0);
	}
	if(cursor.height) goto split;
	hole.node->key[hole.idx] = key;
	PB_(graph)(tree, "graph/topology-0.gv");
#ifdef TREE_VALUE
	if(value) *value = PB_(ref_to_value)(hole);
#endif
	return TREE_UNIQUE;
} catch:
	while(new_head) {
		struct PB_(branch) *const top = PB_(branch)(new_head);
		new_head = top->child[0];
		printf("free branch %s\n", orcify(top));
		free(top);
	}
	if(!errno) errno = ERANGE; /* Non-POSIX OS are not mandated to set errno. */
	return TREE_ERROR;
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
	PB_(is_element_c); PB_(forward_begin); PB_(forward_next);
	PB_(is_element);
	B_(tree)(); B_(tree_)(0); B_(tree_begin)(0); B_(tree_next)(0);
	B_(tree_lower)(0, k);
	B_(tree_get_next)(0, k);
#ifdef TREE_VALUE
	B_(tree_bulk_add)(0, k, 0);
	B_(tree_add)(0, k, 0);
#else
	B_(tree_bulk_add)(0, k);
	B_(tree_add)(0, k);
#endif
	B_(tree_bulk_finish)(0);
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
