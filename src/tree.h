/** @license 2022 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @abstract Stand-alone header <src/tree.h>; examples <test/test_tree.c>. On a
 compatible workstation, `make` creates the test suite of the examples.

 @subtitle Ordered tree

 A <tag:<B>tree> is an ordered collection of read-only <typedef:<PB>key>, and
 an optional <typedef:<PB>value> to go with them. This can be a map or set, but
 in general, it can have identical keys, (a multi-map).

 @param[TREE_NAME, TREE_KEY]
 `<B>` that satisfies `C` naming conventions when mangled, required, and an
 integral type, <typedef:<PB>key>, whose default is `unsigned int`. `<PB>` is
 private, whose names are prefixed in a manner to avoid collisions.

 @param[TREE_VALUE]
 `TRIE_VALUE` is an optional payload to go with the type, <typedef:<PB>value>.

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

 @fixme Either TREE_KEY or TREE_UNIQUE_KEY.
 @fixme It would be probably easy to turn this into an order statistic tree,
 (but annoying.)
 @fixme merge, difference

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
#define TREE_MAX 3
/* 2 is the theoretical minimum, but this does pre-emptive splitting/merging.
 That means it must have a middle element to promote _before_ insertion; it's
 independent of the value added. This means that odd orders, (even `TREE_MAX`,)
 instead of balance 0, it's either 0 or 2, and would not work. Even order it's
 always 1-unbalanced, (which is better; I don't feel like doing virtual
 functions for each case.) */
#if TREE_MAX < 3 || TREE_MAX > UCHAR_MAX
#error TREE_MAX parameter range `[3, UCHAR_MAX]`.
#endif
/* Usually this is `⌊TREE_MAX/2⌋`, the maximum, corresponding to
 `⌈TREE_ORDER/2⌉` children. This provides hysteresis at small occupancies, in
 the spirit of <Johnson, Shasha, 1990, Free-at-Empty>. */
#define TREE_MIN (TREE_MAX / 3)
#if TREE_MIN == 0 || TREE_MIN > TREE_MAX / 2
#error TREE_MIN parameter range `[1, \floor(TREE_MAX / 2)]`.
#endif
#define TREE_ORDER (TREE_MAX + 1) /* Maximum branching factor (degree). */
#endif /* idempotent --> */


#if TREE_TRAITS == 0 /* <!-- base code */


#ifndef TREE_KEY
#define TREE_KEY unsigned
#endif
/** A comparable type, defaults to `unsigned`. Note that `key` is used loosely;
 there can be multiple keys with the same value stored in the same
 <tag:<B>tree>, if one chooses. */
typedef TREE_KEY PB_(key);
typedef const TREE_KEY PB_(key_c);

#ifdef TREE_VALUE
/** On `TREE_VALUE`, otherwise just a (multi)-set of <typedef:<PB>key>. */
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
 <Johnson, Shasha, 1990, Free-at-Empty>, but lookup is potentially slower after
 deleting; this is a design decision that nodes are not cached. In the
 terminology of <Knuth, 1998 Art 3>,
 * Every branch has at most `TREE_ORDER == TREE_MAX + 1` children, which is at
   minimum three, (four with pre-emptive operations.)
 * Every non-root and non-bulk-loaded node has at least `⎣TREE_MAX/3⎦` keys.
 * Every branch has at least one child, `k`, and contains `k - 1` keys, (this
   is a consequence of the fact that they are implicitly storing a complete
   binary sub-tree.)
 * All leaves are at the maximum depth and height zero; they do'n't carry links
   to other nodes. (The height is one less then the original paper, as
   <Knuth, 1998 Art 3>, for computational simplicity.)
 * There are two empty B-trees to facilitate allocation hysteresis between
   0 -- 1: idle `{ 0, 0 }`, and `{ garbage leaf, UINT_MAX }`, one could test,
   `!root | height == UINT_MAX`.
 * Bulk-loading always is on the right side. */
struct PB_(leaf) {
	unsigned char size; /* `[0, TREE_MAX]`. */
	PB_(key) x[TREE_MAX]; /* Cache-friendly lookup; all but one value. */
#ifdef TREE_VALUE
	PB_(value) value[TREE_MAX];
#endif
};
/* B-tree branch is a <tag:<PB>leaf> and links to `size + 1` nodes. */
struct PB_(branch) { struct PB_(leaf) base, *child[TREE_ORDER]; };
/** @return Upcasts `as_leaf` to a branch. */
static struct PB_(branch) *PB_(branch)(struct PB_(leaf) *const as_leaf)
	{ return (struct PB_(branch) *)(void *)
	((char *)as_leaf - offsetof(struct PB_(branch), base)); }
/** @return Upcasts `as_leaf` to a branch. */
static const struct PB_(branch) *PB_(branch_c)(const struct PB_(leaf) *
	const as_leaf) { return (const struct PB_(branch) *)(const void *)
	((const char *)as_leaf - offsetof(struct PB_(branch), base)); }

#ifdef TREE_VALUE /* <!-- value */

/** On `TREE_VALUE`, creates a map from pointer-to-<typedef:<PB>key> to
 pointer-to-<typedef:<PB>value>. The reason these are pointers is because it
 is not connected in memory. */
struct B_(tree_entry) { PB_(key) *x; PB_(value) *value; };
struct B_(tree_entry_c) { PB_(key_c) *x; PB_(value_c) *value; };
/** On `TREE_VALUE`, otherwise it's just an alias for
 pointer-to-<typedef:<PB>key>. */
typedef struct B_(tree_entry) PB_(entry);
typedef struct B_(tree_entry_c) PB_(entry_c);
static PB_(entry) PB_(null_entry)(void)
	{ const PB_(entry) e = { 0, 0 }; return e; }
static PB_(entry_c) PB_(null_entry_c)(void)
	{ const PB_(entry_c) e = { 0, 0 }; return e; }
static PB_(entry) PB_(to_entry)(struct PB_(leaf) *const leaf,
	const unsigned i) { PB_(entry) e;
	e.x = leaf->x + i, e.value = leaf->value + i; return e; }
static PB_(entry_c) PB_(to_entry_c)(const struct PB_(leaf) *const leaf,
	const unsigned i) { PB_(entry_c) e;
	e.x = leaf->x + i, e.value = leaf->value + i; return e; }
/*static PB_(key) PB_(to_x)(const PB_(entry) entry) { return *entry.x; }*/
static PB_(value) *PB_(to_value)(PB_(entry) entry) { return entry.value; }

#else /* value --><!-- !value */

typedef PB_(key) PB_(value);
typedef PB_(key) *PB_(entry);
typedef PB_(key_c) *PB_(entry_c);
static PB_(entry_c) PB_(null_entry_c)(void) { return 0; }
static PB_(entry) PB_(null_entry)(void) { return 0; }
static PB_(entry) PB_(to_entry)(struct PB_(leaf) *const leaf,
	const unsigned i) { return leaf->x + i; }
static PB_(entry_c) PB_(to_entry_c)(const struct PB_(leaf) *const leaf,
	const unsigned i) { return leaf->x + i; }
/*static PB_(key) PB_(to_x)(const PB_(key) *const x) { return *x; }*/
static PB_(value) *PB_(to_value)(PB_(key) *const x) { return x; }

#endif /* !entry --> */

/** To initialize it to an idle state, see <fn:<B>tree>, `TRIE_IDLE`, `{0}`
 (`C99`), or being `static`. This is a B-tree, as
 <Bayer, McCreight, 1972 Large>.

 ![States.](../doc/states.png) */
struct B_(tree);
struct B_(tree) { struct PB_(leaf) *root; unsigned height; };

#define BOX_CONTENT PB_(entry_c)
/** Is `e` not null? @implements `is_element_c` */
static int PB_(is_element_c)(PB_(entry_c) e) {
#ifdef TREE_VALUE
	return !!e.x;
#else
	return !!e;
#endif
}
/* A constant iterator. @implements `forward` */
struct PB_(forward) {
	const struct B_(tree) *tree;
	struct PB_(forward_end) { const struct PB_(leaf) *node;
		unsigned height, idx; } end;
};
/** Start the iteration or, if off the end of a node, go to the next node.
 @return Whether it is addressing a valid item. */
static int PB_(forward_pin)(struct PB_(forward) *const it) {
	unsigned a0;
	struct B_(tree) t;
	struct PB_(forward_end) next;
	PB_(key) x;
	assert(it);
	if(!it->tree || it->tree->height == UINT_MAX) return 0;
	/* Special case: off the left. */
	if(!it->end.node) it->end.node = it->tree->root, assert(it->end.node),
		it->end.height = it->tree->height, it->end.idx = 0;
	/* Descend the tree. */
	while(it->end.height) it->end.height--,
		it->end.node = PB_(branch_c)(it->end.node)->child[it->end.idx],
		it->end.idx = 0, assert(it->end.node);
	if(it->end.idx < it->end.node->size) return 1; /* Likely. */
	if(!it->end.node->size) return 0; /* Empty nodes are always at the end. */
	/* Go down the tree again and note the next. */
	next.node = 0, x = it->end.node->x[it->end.node->size - 1];
	for(t = *it->tree; t.height;
		t.root = PB_(branch_c)(t.root)->child[a0], t.height--) {
		int cmp; unsigned a1 = t.root->size; a0 = 0;
		while(a0 < a1) {
			const unsigned m = (a0 + a1) / 2;
			cmp = PB_(compare)(x, t.root->x[m]);
			if(cmp > 0) a0 = m + 1; else a1 = m;
		}
		if(a0 < t.root->size)
			next.node = t.root, next.height = t.height, next.idx = a0;
	}
	if(!next.node) return 0; /* Off the right. */
	it->end = next;
	return 1; /* Jumped nodes. */
}
/** @return Before `tree`. @implements `forward_begin` */
static struct PB_(forward) PB_(forward_begin)(const struct B_(tree) *const
	tree) {
	struct PB_(forward) it;
	it.tree = tree, it.end.node = 0, it.end.height = 0, it.end.idx = 0;
	return it;
}
/** Advances `it` to the next element. @return A pointer to the current
 element or null. @implements `forward_next` */
static PB_(entry_c) PB_(forward_next)(struct PB_(forward) *const it)
	{ return assert(it), PB_(forward_pin)(it)
	? PB_(to_entry_c)(it->end.node, it->end.idx++) : PB_(null_entry_c)(); }

#define BOX_ITERATOR PB_(entry)
/** Is `x` not null? @implements `is_element` */
static int PB_(is_element)(const PB_(entry) e) {
#ifdef TREE_VALUE
	return !!e.x;
#else
	return !!e;
#endif
}
/* A certain position and the top level tree for backtracking.
 @implements `iterator` */
struct PB_(iterator) {
	struct B_(tree) *tree;
	struct PB_(end) { struct PB_(leaf) *node; unsigned height, idx; } end;
};
/** @return Whether it is addressing a valid item. */
static int PB_(pin)(struct PB_(iterator) *const it) {
	unsigned a0;
	struct B_(tree) t;
	struct PB_(end) next;
	PB_(key) x;
	assert(it);
	if(!it->tree || it->tree->height == UINT_MAX) return 0;
	if(!it->end.node) it->end.node = it->tree->root,
		it->end.height = it->tree->height, it->end.idx = 0;
	while(it->end.height) it->end.height--,
		it->end.node = PB_(branch_c)(it->end.node)->child[it->end.idx],
		it->end.idx = 0;
	if(it->end.idx < it->end.node->size) return 1; /* Likely. */
	if(!it->end.node->size) return 0; /* Empty nodes are always at the end. */
	next.node = 0, x = it->end.node->x[it->end.node->size - 1];
	for(t = *it->tree; t.height;
		t.root = PB_(branch_c)(t.root)->child[a0], t.height--) {
		int cmp; unsigned a1 = t.root->size; a0 = 0;
		while(a0 < a1) {
			const unsigned m = (a0 + a1) / 2;
			cmp = PB_(compare)(x, t.root->x[m]);
			if(cmp > 0) a0 = m + 1; else a1 = m;
		}
		if(a0 < t.root->size)
			next.node = t.root, next.height = t.height, next.idx = a0;
	}
	if(!next.node) return 0; /* Off the right. */
	it->end = next;
	return 1; /* Jumped nodes. */
}
/** @return Before `tree`. @implements `forward_begin` */
static struct PB_(iterator) PB_(begin)(struct B_(tree) *const tree) {
	struct PB_(iterator) it;
	it.tree = tree, it.end.node = 0, it.end.height = 0, it.end.idx = 0;
	return it;
}
/** Advances `it` to the next element. @return A pointer to the current
 element or null. @implements `next` */
static PB_(entry) PB_(next)(struct PB_(iterator) *const it)
	{ return assert(it), PB_(pin)(it)
	? PB_(to_entry)(it->end.node, it->end.idx++) : PB_(null_entry)(); }

#include "../test/orcish.h"

/** Assume `tree` and `x` are checked for non-empty validity. */
static struct PB_(end) PB_(lower_r)(struct B_(tree) *const tree,
	const PB_(key) x, struct PB_(end) *const unfull) {
	struct PB_(end) lo;
	for(lo.node = tree->root, lo.height = tree->height; ;
		lo.node = PB_(branch_c)(lo.node)->child[lo.idx], lo.height--) {
		unsigned hi = lo.node->size;
		lo.idx = 0;
		if(unfull && hi < TREE_MAX) *unfull = lo;
		if(!hi) continue; /* No nodes; bulk-add? */
		do {
			const unsigned mid = lo.idx + (hi - lo.idx) / 2; /* Un-needed op? */
			if(PB_(compare)(x, lo.node->x[mid]) > 0) lo.idx = mid + 1;
			else hi = mid;
		} while(lo.idx < hi);
		if(!lo.height) break; /* Leaf node. */
		if(lo.idx == lo.node->size) continue; /* Off the end. */
		if(PB_(compare)(lo.node->x[lo.idx], x) <= 0) { /* Total order equals. */
#ifdef TREE_UNIQUE_KEY
#error TREE_UNIQUE_KEY doesn't exist yet.
#else
			/* Lower indices with the same value in the left child? */
			struct PB_(end) res;
			struct B_(tree) sub;
			sub.root = PB_(branch_c)(lo.node)->child[lo.idx];
			sub.height = lo.height - 1;
			if((res = PB_(lower_r)(&sub, x, unfull)).idx < res.node->size)
				lo = res;
#endif
			break;
		}
	}
	return lo;
}

/** @param[tree] Can be null. @return Lower bound of `x` in `tree`.
 @order \O(\log |`tree`|) */
static struct PB_(iterator) PB_(lower)(struct B_(tree) *const tree,
	const PB_(key) x) {
	struct PB_(iterator) it;
	if(!tree || !tree->root || tree->height == UINT_MAX) return it.tree = 0, it;
	return it.tree = tree, it.end = PB_(lower_r)(tree, x, 0), it;
}

/** Clears non-empty `tree` and it's children recursively, but doesn't put it
 to idle or clear pointers. If `one` is valid, tries to keep one leaf. */
static void PB_(clear_r)(struct B_(tree) tree, struct PB_(leaf) **const one) {
	assert(tree.root);
	if(!tree.height) {
		if(one && !*one) *one = tree.root;
		else free(tree.root);
	} else {
		struct B_(tree) sub;
		unsigned i;
		sub.height = tree.height - 1;
		for(i = 0; i <= tree.root->size; i++)
			sub.root = PB_(branch)(tree.root)->child[i], PB_(clear_r)(sub, one);
		free(PB_(branch)(tree.root));
	}
}

/* Box override information. */
#define BOX_ PB_
#define BOX struct B_(tree)

/** Initializes `tree` to idle. @order \Theta(1) @allow */
static struct B_(tree) B_(tree)(void)
	{ struct B_(tree) tree; tree.root = 0; tree.height = 0; return tree; }

/** Returns an initialized `tree` to idle, `tree` can be null. @allow */
static void B_(tree_)(struct B_(tree) *const tree) {
	if(!tree) return; /* Null. */
	if(!tree->root) { /* Idle. */
		assert(!tree->height);
	} else if(tree->height == UINT_MAX) { /* Empty. */
		assert(tree->root); free(tree->root);
	} else {
		PB_(clear_r)(*tree, 0);
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

#if 0
/** Counts the of the items in initialized `it`. @order \O(|`it`|) @allow */
static size_t B_(trie_size)(const struct B_(trie_iterator) *const it)
	{ return assert(it), PB_(size_r)(&it->i); }
#endif

/** @param[tree] Can be null. @return Finds the smallest entry in `tree` that
 is at the lower bound of `x`. If `x` is higher than any of `tree`, it will be
 placed just passed the end. @order \O(\log |`tree`|) @allow */
static struct B_(tree_iterator) B_(tree_lower)(struct B_(tree) *const tree,
	const PB_(key) x)
	{ struct B_(tree_iterator) it; it._ = PB_(lower)(tree, x); return it; }

/** @return Lowest value match for `x` in `tree` or null no such item exists.
 @order \O(\log |`tree`|) @allow */
static PB_(value) *B_(tree_get)(struct B_(tree) *const tree,
	const PB_(key) x) {
	struct PB_(iterator) it = PB_(lower)(tree, x);
	PB_(entry) e = PB_(next)(&it);
	return PB_(is_element)(e) ? PB_(to_value)(e) : 0;
}

#include "../test/orcish.h"
static void PB_(print)(const struct B_(tree) *const tree);
#ifndef TREE_TEST
static void PB_(print)(const struct B_(tree) *const tree)
	{ (void)tree, printf("not printable\n"); }
#endif

/** `x` must be not less than the largest key in `tree`.
 @throws[EDOM] `x` is smaller than the largest key in `tree`.
 @throws[malloc] */
static PB_(value) *B_(tree_bulk_add)(struct B_(tree) *const tree, PB_(key) x) {
	struct PB_(leaf) *node = 0, *head = 0;
	/*printf("bulk():\n");*/
	if(!tree) return 0;
	if(!tree->root) { /* Idle tree. */
		assert(!tree->height);
		if(!(node = malloc(sizeof *node))) goto catch;
		node->size = 0;
		tree->root = node;
		/*printf("Idle tree: new %s.\n", orcify(node));*/
	} else if(tree->height == UINT_MAX) { /* Empty tree. */
		tree->height = 0;
		node = tree->root;
		node->size = 0;
		/*printf("Empty tree, %s.\n", orcify(node));*/
	} else {
		struct B_(tree) space = { 0, 0 }; /* Furthest node with space. */
		PB_(key) *last = 0; /* Key of the last for comparing with arg. */
		unsigned new_nodes, n; /* Count new nodes. */
		struct PB_(leaf) *tail = 0; /* New nodes. */
		struct PB_(branch) *pretail = 0;
		PB_(print)(tree);

		{ /* Figure out where `space` and `last` are in `log size`. */
			struct B_(tree) expl;
			for(expl = *tree; ; expl.root = PB_(branch)(expl.root)
				->child[expl.root->size], expl.height--) {
				/*printf("dowhile expl %s:%u with %u size\n",
					orcify(expl.root), expl.height, expl.root->size);*/
				if(expl.root->size < TREE_MAX) space = expl;
				if(expl.root->size) last = expl.root->x + expl.root->size - 1;
				if(!expl.height) break;
			}
			assert(last); /* Else it would be empty and we would not be here. */
			/*printf("dowhile expl finished %s:%u, space %s:%u\n",
				orcify(expl.root), expl.height,
				orcify(space.root), space.height);*/
		}

		/* Verify that the argument is not smaller than the largest in tree. */
		if(PB_(compare)(*last, x) > 0) { errno = EDOM; goto catch; }

		/* One leaf, and the rest branches. */
		new_nodes = n = space.root ? space.height : tree->height + 2;
		/*printf("new_nodes: %u, tree height %u\n", new_nodes, tree->height);*/
		if(!n) {
			node = space.root;
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
		if(!space.root) { /* Add tree to head. */
			struct PB_(branch) *const inner = PB_(branch)(head);
			/*printf("adding the existing root, %s to %s\n",
				orcify(tree->root), orcify(head));*/
			assert(new_nodes > 1);
			inner->child[1] = inner->child[0], inner->child[0] = tree->root;
			node = tree->root = head, tree->height++;
		} else if(space.height) { /* Add head to tree. */
			struct PB_(branch) *const inner = PB_(branch)(node = space.root);
			/*printf("adding the linked list, %s to %s at %u\n",
				orcify(head), orcify(inner), inner->base.size + 1);*/
			assert(new_nodes);
			inner->child[inner->base.size + 1] = head;
		}
	}
	assert(node && node->size < TREE_MAX);
	node->x[node->size] = x;
#ifdef TREE_VALUE
	return node->value + node->size++;
#else
	return node->x + node->size++;
#endif
catch:
	/*printf("!!! freeing %s\n", orcify(node));*/
	free(node);
	if(head) for( ; ; ) {
		struct PB_(leaf) *const next = PB_(branch)(head)->child[0];
		/*printf("!!! freeing %s\n", orcify(head));*/
		free(head);
		if(!next) break;
		head = next;
	}
	if(!errno) errno = ERANGE;
	return 0;
}

static void B_(tree_bulk_finish)(struct B_(tree) *const tree) {
	struct B_(tree) p;
	struct PB_(leaf) *right;
	printf("tree_bulk_finalize(%s) number of nodes [%u, %u]\n",
		orcify(tree), TREE_MIN, TREE_MAX);
	if(!tree || !tree->root || tree->height == UINT_MAX) return; /* Empty. */
	for(p = *tree; p.height; p.root = right, p.height--) {
		unsigned distribute, right_want, take_sibling;
		struct PB_(branch) *parent = PB_(branch)(p.root);
		struct PB_(leaf) *sibling = (assert(parent->base.size),
			parent->child[parent->base.size - 1]);
		right = parent->child[parent->base.size];
		printf("initial parent node %s:%u with %u size, children %s and %s.\n",
			orcify(p.root), p.height, p.root->size,
			orcify(sibling), orcify(right));
		if(right->size >= TREE_MIN) { printf("cool\n"); continue; }
		distribute = sibling->size + right->size;
		right_want = distribute / 2; /* Minimum balancing. */
		take_sibling = right_want - right->size - 1;
		/* Either the right has met the properties of a B-tree node, (covered
		 above,) or the left sibling is full from bulk-loading. */
		assert(sibling->size == TREE_MAX
			&& distribute >= 2 * TREE_MIN && right_want >= TREE_MIN
			&& sibling->size - take_sibling >= TREE_MIN + 1);
		/* Move one node from the parent. */
		memcpy(right->x + right->size, parent->base.x + parent->base.size - 1,
			sizeof *right->x);
#ifdef TREE_VALUE
		memcpy(right->value + right->size,
			parent->base.value + parent->base.size - 1, sizeof *right->value);
#endif
		right->size++;
		/* Move the others from the sibling. */
		memcpy(right->x + right->size, sibling->x + take_sibling,
			sizeof *right->x * take_sibling);
#ifdef TREE_VALUE
		memcpy(right->value + right->size, sibling->value + take_sibling,
			sizeof *right->value * take_sibling);
#endif
		right->size += take_sibling, assert(right->size == right_want);
		sibling->size -= take_sibling;
		/* The parent borrows from the sibling's key. */
		memcpy(parent->base.x + parent->base.size - 1,
			sibling->x + sibling->size - 1, sizeof *right->x);
#ifdef TREE_VALUE
		memcpy(parent->base.value + parent->base.size - 1,
			sibling->value + sibling->size - 1, sizeof *right->value);
#endif
		sibling->size--;
		printf("redistributed\n");
	}
}






static PB_(value) *B_(tree_add)(struct B_(tree) *const tree, PB_(key) x) {
	struct PB_(leaf) *node = 0, *head = 0;
	if(!tree) return 0;
	if(!tree->root) { /* Idle tree. */
		assert(!tree->height);
		if(!(node = malloc(sizeof *node))) goto catch;
		node->size = 0;
		tree->root = node;
		/*printf("Idle tree: new %s.\n", orcify(node));*/
	} else if(tree->height == UINT_MAX) { /* Empty tree. */
		tree->height = 0;
		node = tree->root;
		node->size = 0;
		/*printf("Empty tree, %s.\n", orcify(node));*/
	} else {
#if 0
		struct B_(tree) space = { 0, 0 }; /* Furthest node with space. */
		PB_(key) *last = 0; /* Key of the last for comparing with arg. */
		unsigned new_nodes, n; /* Count new nodes. */
		struct PB_(leaf) *tail = 0; /* New nodes. */
		struct PB_(branch) *pretail = 0;
		PB_(print)(tree);
		struct PB_(end) end, unfull;
		unfull.leaf = 0; /* Start off at the top. */
		end = PB_(lower_r)(tree, x, &unfull);
#endif
		head = 0;
		(void)x;
	}
catch:
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
	PB_(is_element_c); PB_(forward_begin)(0); PB_(forward_next)(0);
	B_(tree)(); B_(tree_)(0); B_(tree_begin)(0); B_(tree_next)(0);
	B_(tree_lower)(0, k);
	B_(tree_get)(0, k);
	B_(tree_bulk_add)(0, k); B_(tree_bulk_finish)(0); B_(tree_add)(0, k);
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
