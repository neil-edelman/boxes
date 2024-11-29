/** @license 2022 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @abstract Stand-alone header <../src/tree.h>; examples <../test/test_tree.c>;
 article <../doc/tree/tree.pdf>.

 @subtitle Ordered tree

 ![Example of an order-3 tree.](../doc/tree/tree.png)

 A <tag:<B>tree> is an ordered set or map contained in a tree. For memory
 locality, this is implemented B-tree, described in
 <Bayer, McCreight, 1972, Large>.

 All operations are fail-fast and will not compromise the integrity of the
 existing tree. As a B-tree, this is not stable.

 @param[TREE_NAME, TREE_KEY]
 `<B>` that satisfies `C` naming conventions when mangled, required, and
 `TREE_KEY`, a type, <typedef:<PB>key>, whose default is `unsigned int`.
 `<PB>` is private, whose names are prefixed in a manner to avoid collisions.

 @param[TREE_VALUE]
 Optional payload to go with the type, <typedef:<PB>value>, thus making it a
 map instead of a set.

 @param[TREE_COMPARE]
 This will define <fn:<B>compare>, a <typedef:<PB>compare_fn> that compares
 keys as integer-types that results in ascending order, `a > b`. If
 `TREE_COMPARE` is specified, the user most specify their own <fn:<B>compare>.

 @param[TREE_ORDER]
 Sets the branching factor, or order as <Knuth, 1998 Art 3>, to the range
 `[3, UINT_MAX+1]`. Default 65 is tuned to an integer to pointer map, and
 should be okay for most variations. 4 is isomorphic to left-leaning red-black
 tree, <Sedgewick, 2008, LLRB>. The above illustration is 5.

 @param[TREE_DEFAULT]
 Default trait which must be set to a <typedef:<PB>value>, used in
 <fn:<B>tree<D>get>.

 @param[TREE_TO_STRING]
 To string trait `<STR>` contained in <src/to_string.h>. Require
 `<name>[<trait>]to_string` be declared as <typedef:<PSTR>to_string_fn>.

 @param[TREE_EXPECT_TRAIT, TREE_TRAIT]
 Named traits are obtained by including `tree.h` multiple times with
 `TREE_EXPECT_TRAIT` and then subsequently including the name that satisfies
 `C` naming conventions when mangled in `TREE_TRAIT`.

 @param[TREE_HEAD, TREE_BODY]
 These go together to allow exporting non-static data between compilation units
 by separating the header head from the code body. `TREE_HEAD` needs identical
 `TREE_NAME`, `TREE_KEY`, `TREE_VALUE`, and `TREE_ORDER`.

 @fixme merge, difference

 @std C89 */

#if !defined(TREE_NAME)
#error Name undefined.
#endif
#if defined(TREE_TRAIT) ^ defined(BOX_TYPE)
#error TREE_TRAIT name must come after TREE_EXPECT_TRAIT.
#endif
#if defined(TREE_TEST) && (!defined(TREE_TRAIT) && !defined(TREE_TO_STRING) \
	|| defined(TREE_TRAIT) && !defined(TREE_HAS_TO_STRING))
#error Test requires to string.
#endif
#if defined TREE_HEAD && (defined TREE_BODY || defined TREE_TRAIT)
#error Can not be simultaneously defined.
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
#if defined(TREE_CAT_) || defined(TREE_CAT) || defined(B_) || defined(PB_)
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
#define TREE_RESULT X(ERROR), X(ABSENT), X(PRESENT)
#define X(n) TREE_##n
/** A result of modifying the tree, of which `TREE_ERROR` is false.

 ![A diagram of the result states.](../doc/tree/result.png) */
enum tree_result { TREE_RESULT };
#undef X
#ifndef TREE_HEAD /* <!-- body */
#define X(n) #n
/** A static array of strings describing the <tag:tree_result>. */
static const char *const tree_result_str[] = { TREE_RESULT };
#undef X
#endif /* body --> */
#undef TREE_RESULT
struct tree_node_count { size_t branches, leaves; };
#endif /* idempotent --> */


#ifndef TREE_TRAIT /* <!-- base code */

#ifndef TREE_ORDER
#define TREE_ORDER 65 /* Maximum branching factor. This sets the granularity. */
#endif
#if TREE_ORDER < 3 || TREE_ORDER > UINT_MAX + 1
#error TREE_ORDER parameter range `[3, UINT_MAX+1]`.
#endif
#ifndef TREE_KEY
#define TREE_KEY unsigned
#endif

#ifndef TREE_BODY /* <!-- head */

/** Ordered type used by <typedef:<PB>compare_fn>; defaults to `unsigned`. */
typedef TREE_KEY PB_(key);

#ifdef TREE_VALUE
/** On `TREE_VALUE`, this creates a map, otherwise a set of
 <typedef:<PB>key>. */
typedef TREE_VALUE PB_(value);
#endif

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
	unsigned size;
	PB_(key) key[TREE_MAX]; /* Cache-friendly lookup. */
#ifdef TREE_VALUE
	PB_(value) value[TREE_MAX];
#endif
};
/* B-tree branch is a <tag:<PB>node> and links to `size + 1` nodes. */
struct PB_(branch) { struct PB_(node) base, *child[TREE_ORDER]; };

/* Node plus height is a [sub]-tree. */
struct PB_(tree) { struct PB_(node) *node; unsigned height; };
/** To initialize it to an idle state, see <fn:<B>tree>, `{0}` (`C99`), or
 being `static`.

 ![States.](../doc/tree/states.png) */
struct B_(tree);
struct B_(tree) { struct PB_(tree) root; };
/* fixme: height 1-based, fix pointer. */

/* Address of a specific key by node. Height might not be used, but there's too
 many structs in this file anyway. */
struct PB_(ref) {
	struct PB_(node) *node; /* If null, others ignored. */
	unsigned height, idx; /* `idx < node.size` means valid. */
};

struct PB_(iterator) { struct PB_(tree) *root; struct PB_(ref) ref; };

/** Adding, deleting, or changes in the topology of the tree invalidate the
 iterator. To modify the tree while iterating, take the <fn:<B>tree_key> and
 restart the iterator with <fn:<B>tree_less> or <fn:<B>tree_more> as
 appropriate. */
struct B_(tree_iterator);
struct B_(tree_iterator) { struct PB_(iterator) _; };

#endif /* head --> */
#ifndef TREE_HEAD /* <!-- body */

/** Returns a positive result if `a` is out-of-order with respect to `b`,
 inducing a strict weak order. This is compatible, but less strict then the
 comparators from `bsearch` and `qsort`; it only needs to divide entries into
 two instead of three categories. */
typedef int (*PB_(compare_fn))(const PB_(key) a, const PB_(key) b);
#ifndef TREE_COMPARE /* <!-- !cmp */
/** The default `TREE_COMPARE` on `a` and `b` is integer comparison that
 results in ascending order, `a > b`. Use `TREE_COMPARE` to supply one's own.
 @implements <typedef:<PB>compare_fn> */
static int B_(compare)(const PB_(key) a, const PB_(key) b)
	{ return a > b; }
#define TREE_COMPARE &PB_(default_compare)
#endif /* !cmp --> */

/** @return Downcasts `as_leaf` to a branch. */
static struct PB_(branch) *PB_(as_branch)(struct PB_(node) *const as_leaf)
	{ return (struct PB_(branch) *)(void *)
	((char *)as_leaf - offsetof(struct PB_(branch), base)); }
/** @return Downcasts `as_node` to a branch. */
static const struct PB_(branch) *PB_(as_branch_c)(const struct PB_(node) *
	const as_node) { return (const struct PB_(branch) *)(const void *)
	((const char *)as_node - offsetof(struct PB_(branch), base)); }

#ifdef TREE_VALUE /* <!-- value */
/** Gets the value of `ref`. */
static PB_(value) *PB_(ref_to_valuep)(const struct PB_(ref) ref)
	{ return ref.node ? ref.node->value + ref.idx : 0; }
#else /* value --><!-- !value */
typedef PB_(key) PB_(value);
/** Gets the value of `ref`. */
static PB_(value) *PB_(ref_to_valuep)(const struct PB_(ref) ref)
	{ return ref.node ? ref.node->key + ref.idx : 0; }
#endif /* !value --> */

/** Iterator for `tree` in empty state. */
static struct PB_(iterator) PB_(iterator)(struct B_(tree) *const tree) {
	struct PB_(iterator) it;
	assert(tree);
	it.root = &tree->root;
	it.ref.node = 0, it.ref.height = 0, it.ref.idx = 0;
	return it;
}
/** @return Dereference the next (pointing to valid element) `it`. */
static struct PB_(ref) *PB_(element)(struct PB_(iterator) *const it)
	{ return &it->ref; }
/** @return Whether `it` pointing to a valid element. */
static int PB_(next)(struct PB_(iterator) *const it) {
	struct PB_(ref) next;
	assert(it && it->root);

	/* Tree empty. */
	if(!it->root || !it->root->node || it->root->height == UINT_MAX) return 0;

	/* Iterator empty; tree non-empty; point at first. */
	if(!it->ref.node) {
		it->ref.height = it->root->height;
		for(it->ref.node = it->root->node; it->ref.height;
			it->ref.node = PB_(as_branch_c)(it->ref.node)->child[0],
			it->ref.height--);
		it->ref.idx = 0;
		return 1;
	}

	/* Next is a copy of the next element. Clip. */
	next = it->ref, next.idx++;
	if(next.height && next.idx > next.node->size) next.idx = next.node->size;
	while(next.height) next.node = PB_(as_branch)(next.node)->child[next.idx],
		next.idx = 0, next.height--; /* Fall from branch. */
	it->ref = next; /* Possibly one beyond bounds. */
	if(next.idx >= next.node->size) { /* Maybe re-descend reveals more keys. */
		struct PB_(tree) tree = *it->root;
		unsigned a0;
		/* Target; this will not work with duplicate keys. */
		const PB_(key) x = next.node->key[next.node->size - 1];
		assert(next.node->size);
		for(next.node = 0; tree.height;
			tree.node = PB_(as_branch)(tree.node)->child[a0], tree.height--) {
			unsigned a1 = tree.node->size;
			a0 = 0;
			while(a0 < a1) {
				const unsigned m = (a0 + a1) / 2;
				if(B_(compare)(x, tree.node->key[m]) > 0) a0 = m + 1;
				else a1 = m;
			}
			if(a0 < tree.node->size) next.node = tree.node,
				next.height = tree.height, next.idx = a0;
		}
		if(!next.node) return it->ref.node = 0, 0; /* Off right. */
	} /* Jumped nodes. */
	it->ref = next;
	return 1;
}
/** @return Whether `it` is pointing to a valid element. */
static int PB_(previous)(struct PB_(iterator) *const it) {
	struct PB_(ref) prd;
	assert(it && it->root);

	/* Tree empty. */
	if(!it->root || !it->root->node || it->root->height == UINT_MAX) return 0;

	/* Iterator empty; tree non-empty; point at last. */
	if(!it->ref.node) {
		it->ref.height = it->root->height;
		for(it->ref.node = it->root->node; it->ref.height; it->ref.node
			= PB_(as_branch)(it->ref.node)->child[it->ref.node->size],
			it->ref.height--);
		/* Did you forget <fn:<N>tree_bulk_load_finish>? */
		if(!it->ref.node->size) return it->ref.node = 0, 0;
		it->ref.idx = it->ref.node->size - 1;
		return 1;
	}

	/* Predecessor? Clip. */
	prd = it->ref;
	if(prd.height && prd.idx > prd.node->size) prd.idx = prd.node->size;
	while(prd.height) prd.height--,
		prd.node = PB_(as_branch)(prd.node)->child[prd.idx],
		prd.idx = prd.node->size;
	if(prd.idx) {
		prd.idx--;
	} else { /* Maybe re-descend reveals more keys. */
		struct PB_(tree) tree = *it->root;
		unsigned a0;
		const PB_(key) x = prd.node->key[0]; /* Target. */
		for(prd.node = 0; tree.height;
			tree.node = PB_(as_branch)(tree.node)->child[a0], tree.height--) {
			unsigned a1 = tree.node->size;
			a0 = 0;
			while(a0 < a1) {
				const unsigned m = (a0 + a1) / 2;
				if(B_(compare)(x, tree.node->key[m]) > 0) a0 = m + 1;
				else a1 = m;
			}
			if(a0) prd.node = tree.node, prd.height = tree.height,
				prd.idx = a0 - 1;
		}
		if(!prd.node) return it->ref.node = 0, 0; /* Off left. */
	} /* Jumped nodes. */
	it->ref = prd;
	return 1;
}

/** Finds `idx` of 'greatest lower-bound' (C++ parlance) minorant of `x` in
 `lo` only in one node at a time. */
static void PB_(node_lb)(struct PB_(ref) *const lo, const PB_(key) x) {
	unsigned hi = lo->node->size; lo->idx = 0;
	assert(lo && lo->node && hi);
	do {
		const unsigned mid = (lo->idx + hi) / 2; /* Will not overflow. */
		if(B_(compare)(x, lo->node->key[mid]) > 0) lo->idx = mid + 1;
		else hi = mid;
	} while(lo->idx < hi);
}
/** Finds `idx` of 'least upper-bound' (C++ parlance) majorant of `x` in `hi`
 only in one node at a time. */
static void PB_(node_ub)(struct PB_(ref) *const hi, const PB_(key) x) {
	unsigned lo = 0;
	assert(hi->node && hi->idx);
	do {
		const unsigned mid = (lo + hi->idx) / 2;
		if(B_(compare)(hi->node->key[mid], x) <= 0) lo = mid + 1;
		else hi->idx = mid;
	} while(lo < hi->idx);
}

/** @return A reference to the greatest key at or less than `x` in `tree`, or
 the reference will be empty if the `x` is less than all `tree`. */
static struct PB_(ref) PB_(less)(const struct PB_(tree) tree,
	const PB_(key) x) {
	struct PB_(ref) hi, found;
	found.node = 0;
	if(!tree.node || tree.height == UINT_MAX) return found;
	for(hi.node = tree.node, hi.height = tree.height; ;
		hi.node = PB_(as_branch_c)(hi.node)->child[hi.idx], hi.height--) {
		if(!(hi.idx = hi.node->size)) continue;
		PB_(node_ub)(&hi, x);
		if(hi.idx) { /* Within bounds to record the current predecessor. */
			found = hi, found.idx--;
			/* Equal. */
			if(B_(compare)(x, found.node->key[found.idx]) <= 0) break;
		}
		if(!hi.height) break; /* Reached the bottom. */
	}
	return found;
}
/** @return A reference to the smallest key at or more than `x` in `tree`, or
 the reference will be empty if the `x` is more than all `tree`. */
static struct PB_(ref) PB_(more)(const struct PB_(tree) tree,
	const PB_(key) x) {
	struct PB_(ref) lo, found;
	found.node = 0;
	if(!tree.node || tree.height == UINT_MAX) return found;
	for(lo.node = tree.node, lo.height = tree.height; ;
		lo.node = PB_(as_branch_c)(lo.node)->child[lo.idx], lo.height--) {
		unsigned hi = lo.node->size; lo.idx = 0;
		if(!hi) continue;
		PB_(node_lb)(&lo, x);
		if(lo.idx < lo.node->size) {
			found = lo;
			if(B_(compare)(x, lo.node->key[lo.idx]) > 0) break;
		}
		if(!lo.height) break;
	}
	return found;
}
/** Finds an exact key `x` in non-empty `tree`. */
static struct PB_(ref) PB_(lookup_find)(const struct PB_(tree) tree,
	const PB_(key) x) {
	struct PB_(ref) lo;
	if(!tree.node || tree.height == UINT_MAX) return lo.node = 0, lo;
	for(lo.node = tree.node, lo.height = tree.height; ;
		lo.node = PB_(as_branch_c)(lo.node)->child[lo.idx], lo.height--) {
		unsigned hi = lo.node->size; lo.idx = 0;
		if(!hi) continue;
		PB_(node_lb)(&lo, x);
		/* Absolutely will not equivalent `x > lo`, investigate? */
		if(lo.idx < lo.node->size && B_(compare)(lo.node->key[lo.idx], x) <= 0)
			break;
		if(!lo.height) { lo.node = 0; break; }
	}
	return lo;
}
/** Finds lower-bound of key `x` in non-empty `tree` while counting the
 non-filled `hole` and `is_equal`. */
static struct PB_(ref) PB_(lookup_insert)(struct PB_(tree) tree,
	const PB_(key) x, struct PB_(ref) *const hole, int *const is_equal) {
	struct PB_(ref) lo;
	hole->node = 0;
	for(lo.node = tree.node, lo.height = tree.height; ;
		lo.node = PB_(as_branch_c)(lo.node)->child[lo.idx], lo.height--) {
		unsigned hi = lo.node->size; lo.idx = 0;
		if(hi < TREE_MAX) *hole = lo;
		if(!hi) continue;
		PB_(node_lb)(&lo, x);
		if(lo.node->size < TREE_MAX) hole->idx = lo.idx;
		if(lo.idx < lo.node->size && B_(compare)(lo.node->key[lo.idx], x) <= 0)
			{ *is_equal = 1; break; }
		if(!lo.height) break;
	}
	return lo;
}
/** Finds exact key `x` in non-empty `tree`. If `node` is found, temporarily,
 the nodes that have `TREE_MIN` keys have
 `as_branch(node).child[TREE_MAX] = parent` or, for leaves, `leaf_parent`,
 which must be set. (Patently terrible for running concurrently; hack, would be
 nice to go down tree maybe.) */
static struct PB_(ref) PB_(lookup_remove)(struct PB_(tree) tree,
	const PB_(key) x, struct PB_(node) **leaf_parent) {
	struct PB_(node) *parent = 0;
	struct PB_(ref) lo;
	for(lo.node = tree.node, lo.height = tree.height; ;
		lo.node = PB_(as_branch_c)(lo.node)->child[lo.idx], lo.height--) {
		unsigned hi = lo.node->size; lo.idx = 0;
		/* Cannot delete bulk add. */
		if(parent && hi < TREE_MIN || !parent && !hi) { lo.node = 0; break; }
		if(hi <= TREE_MIN) { /* Remember the parent temporarily. */
			if(lo.height) PB_(as_branch)(lo.node)->child[TREE_MAX] = parent;
			else *leaf_parent = parent;
		}
		PB_(node_lb)(&lo, x);
		if(lo.idx < lo.node->size && B_(compare)(lo.node->key[lo.idx], x) <= 0)
			break;
		if(!lo.height) { lo.node = 0; break; } /* Was not in. */
		parent = lo.node;
	}
	return lo;
}

/** Zeroed data (not all-bits-zero) is initialized. @return An idle tree.
 @order \Theta(1) @allow */
static struct B_(tree) B_(tree)(void) {
	struct B_(tree) tree;
	tree.root.node = 0; tree.root.height = 0;
	return tree;
}

/** Private: frees non-empty `tree` and it's children recursively, but doesn't
 put it to idle or clear pointers.
 @param[keep] Keep one leaf if non-null. Set to null before. */
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
 full, it remains active, (all except one node are freed.)
 @order \O(|`tree`|) @allow */
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

/** @return Is `x` in `tree` (which can be null)?
 @order \O(\log |`tree`|) @allow */
static int B_(tree_contains)(const struct B_(tree) *const tree,
	const PB_(key) x) { return tree && PB_(lookup_find)(tree->root, x).node; }
/* fixme: entry <B>tree_query -- there is no functionality that returns the
 key, which might be important with distinguishable keys. */

/** @return Get the value of `key` in `tree`, or if no key, `default_value`.
 The map type is `TREE_VALUE` and the set type is `TREE_KEY`.
 @order \O(\log |`tree`|) @allow */
static PB_(value) B_(tree_get_or)(const struct B_(tree) *const tree,
	const PB_(key) key, const PB_(value) default_value) {
	struct PB_(ref) ref;
	return tree && tree->root.node && tree->root.height != UINT_MAX
		&& (ref = PB_(lookup_find)(tree->root, key)).node
		? *PB_(ref_to_valuep)(ref) : default_value;
}

/** For example, `tree = { 10 }`, `x = 5 -> default_value`, `x = 10 -> 10`,
 `x = 11 -> 10`.
 @return Key in `tree` less-then-or-equal to `x` or `default_key` if `x` is
 smaller than all in `tree`. @order \O(\log |`tree`|) @allow */
static PB_(key) B_(tree_less_or)(const struct B_(tree) *const tree,
	const PB_(key) x, const PB_(key) default_key) {
	struct PB_(ref) ref;
	return tree && (ref = PB_(less)(tree->root, x)).node ?
		(assert(ref.idx < ref.node->size), ref.node->key[ref.idx])
		: default_key;
}

/** For example, `tree = { 10 }`, `x = 5 -> 10`, `x = 10 -> 10`,
 `x = 11 -> default_value`.
 @return Key in `tree` greater-than-or-equal to `x` or `default_key` if `x` is
 greater than all in `tree`.
 @order \O(\log |`tree`|) @allow */
static PB_(key) B_(tree_more_or)(const struct B_(tree) *const tree,
	const PB_(key) x, const PB_(key) default_key) {
	struct PB_(ref) ref;
	return tree && (ref = PB_(more)(tree->root, x)).node
		? ref.node->key[ref.idx] : default_key;
}

/* fixme: Notch (add) and nick (delete) are good names for the highest
 non-full node, in spirit with the tree analogy. */

#ifdef TREE_VALUE /* <!-- map */
/** Only if `TREE_VALUE` is set; the set version is <fn:<B>tree_try>. Packs
 `key` on the right side of `tree` without doing the usual restructuring. All
 other topology modification functions should be avoided until followed by
 <fn:<B>tree_bulk_finish>.
 @param[value] A pointer to the key's value which is set by the function on
 returning true. Can be null.
 @return One of <tag:tree_result>: `TREE_ERROR` and `errno` will be set,
 `TREE_PRESENT` if the key is already (the highest) in the tree, and
 `TREE_ABSENT`, added, the `value` (if applicable) is uninitialized.
 @throws[EDOM] `x` is smaller than the largest key in `tree`. @throws[malloc]
 @order \O(\log |`tree`|) @allow */
static enum tree_result B_(tree_bulk_assign)(struct B_(tree) *const tree,
	PB_(key) key, PB_(value) **const value) {
#elif defined TREE_VALUE /* map --><!-- null: For braces matching. */
}
#else /* null --><!-- set */
/** Only if `TREE_VALUE` is not set; see <fn:<B>tree_assign>, which is
 the map version. Packs `key` on the right side of `tree`. @allow */
static enum tree_result B_(tree_bulk_try)(struct B_(tree) *const tree,
	PB_(key) key) {
#endif
	struct PB_(node) *node = 0, *head = 0; /* The original and new. */
	assert(tree);
	if(!tree->root.node) { /* Idle tree. */
		assert(!tree->root.height);
		if(!(node = malloc(sizeof *node))) goto catch;
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
		if(B_(compare)(max, key) > 0) return errno = EDOM, TREE_ERROR;
		if(B_(compare)(key, max) <= 0) {
#ifdef TREE_VALUE
			if(value) {
				struct PB_(ref) max_ref;
				max_ref.node = last, max_ref.idx = last->size - 1;
				*value = PB_(ref_to_valuep)(max_ref);
			}
#endif
			return TREE_PRESENT;
		}

		/* One leaf, and the rest branches. */
		new_nodes = n = unfull.node ? unfull.height : tree->root.height + 2;
		if(!n) {
			node = unfull.node;
		} else {
			if(!(node = tail = malloc(sizeof *tail))) goto catch;
			tail->size = 0;
			while(--n) {
				struct PB_(branch) *b;
				if(!(b = malloc(sizeof *b))) goto catch;
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
		*value = PB_(ref_to_valuep)(max_ref);
	}
#endif
	node->size++;
	return TREE_ABSENT;
catch: /* Didn't work. Reset. */
	free(node);
	while(head) {
		struct PB_(node) *const next = PB_(as_branch)(head)->child[0];
		free(head);
		head = next;
	}
	if(!errno) errno = ERANGE;
	return TREE_ERROR;
}

/** Distributes `tree` (can be null) on the right side so that, after a series
 of <fn:<B>tree_bulk_try> or <fn:<B>tree_bulk_assign>, it will be consistent
 with the minimum number of keys in a node.
 @return The re-distribution was a success and all nodes are within rules.
 (Only when intermixing bulk and regular operations, can the function return
 false.) @order \O(\log |`tree`|) @allow */
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
	struct PB_(ref) add, hole, iterator;
	assert(root);
	if(!(add.node = root->node)) goto idle;
	else if(root->height == UINT_MAX) goto empty;
	goto descend;
idle: /* No reserved memory. */
	assert(!add.node && !root->height);
	if(!(add.node = malloc(sizeof *add.node))) goto catch;
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
		add = PB_(lookup_insert)(*root, key, &hole, &is_equal);
		if(is_equal) {
			if(eject) {
				*eject = add.node->key[add.idx];
				add.node->key[add.idx] = key;
			}
#ifdef TREE_VALUE
			if(value) *value = PB_(ref_to_valuep)(add);
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
	if(value) *value = PB_(ref_to_valuep)(add);
#endif
	return TREE_ABSENT;
grow: /* Leaf is full. */ {
	unsigned new_no = hole.node ? hole.height : root->height + 2;
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
		hole.node = new_head, hole.height = ++root->height, hole.idx = 0;
		new_head = new_root->child[1] = new_root->child[0];
		new_root->child[0] = root->node, root->node = hole.node;
		hole.node->size = 1;
	}
	iterator = hole; /* Go down; (as opposed to doing it on paper.) */
	goto split;
} split: { /* Split between the new and existing nodes. */
	struct PB_(node) *sibling;
	assert(iterator.node && iterator.node->size && iterator.height);
	sibling = new_head;
	/*PB_(graph_usual)(tree, "graph/work.gv");*/
	/* Descend now while split hasn't happened -- easier. */
	new_head = --iterator.height ? PB_(as_branch)(new_head)->child[0] : 0;
	iterator.node = PB_(as_branch)(iterator.node)->child[iterator.idx];
	PB_(node_lb)(&iterator, key);
	assert(!sibling->size && iterator.node->size == TREE_MAX); /* Atomic. */
	/* Expand `iterator`, which is full, to multiple nodes. */
	if(iterator.idx < TREE_SPLIT) { /* Descend hole to `iterator`. */
		memcpy(sibling->key, iterator.node->key + TREE_SPLIT,
			sizeof *sibling->key * (TREE_MAX - TREE_SPLIT));
#ifdef TREE_VALUE
		memcpy(sibling->value, iterator.node->value + TREE_SPLIT,
			sizeof *sibling->value * (TREE_MAX - TREE_SPLIT));
#endif
		hole.node->key[hole.idx] = iterator.node->key[TREE_SPLIT - 1];
#ifdef TREE_VALUE
		hole.node->value[hole.idx] = iterator.node->value[TREE_SPLIT - 1];
#endif
		memmove(iterator.node->key + iterator.idx + 1,
			iterator.node->key + iterator.idx,
			sizeof *iterator.node->key * (TREE_SPLIT - 1 - iterator.idx));
#ifdef TREE_VALUE
		memmove(iterator.node->value + iterator.idx + 1,
			iterator.node->value + iterator.idx,
			sizeof *iterator.node->value * (TREE_SPLIT - 1 - iterator.idx));
#endif
		if(iterator.height) {
			struct PB_(branch) *const cb = PB_(as_branch)(iterator.node),
				*const sb = PB_(as_branch)(sibling);
			struct PB_(node) *temp = sb->child[0];
			memcpy(sb->child, cb->child + TREE_SPLIT,
				sizeof *cb->child * (TREE_MAX - TREE_SPLIT + 1));
			memmove(cb->child + iterator.idx + 2, cb->child + iterator.idx + 1,
				sizeof *cb->child * (TREE_SPLIT - 1 - iterator.idx));
			cb->child[iterator.idx + 1] = temp;
		}
		hole = iterator;
	} else if(iterator.idx > TREE_SPLIT) { /* Descend hole to `sibling`. */
		hole.node->key[hole.idx] = iterator.node->key[TREE_SPLIT];
#ifdef TREE_VALUE
		hole.node->value[hole.idx] = iterator.node->value[TREE_SPLIT];
#endif
		hole.node = sibling, hole.height = iterator.height,
			hole.idx = iterator.idx - TREE_SPLIT - 1;
		memcpy(sibling->key, iterator.node->key + TREE_SPLIT + 1,
			sizeof *sibling->key * hole.idx);
		memcpy(sibling->key + hole.idx + 1, iterator.node->key + iterator.idx,
			sizeof *sibling->key * (TREE_MAX - iterator.idx));
#ifdef TREE_VALUE
		memcpy(sibling->value, iterator.node->value + TREE_SPLIT + 1,
			sizeof *sibling->value * hole.idx);
		memcpy(sibling->value + hole.idx + 1, iterator.node->value
			+ iterator.idx, sizeof *sibling->value * (TREE_MAX - iterator.idx));
#endif
		if(iterator.height) {
			struct PB_(branch) *const cb = PB_(as_branch)(iterator.node),
				*const sb = PB_(as_branch)(sibling);
			struct PB_(node) *temp = sb->child[0];
			memcpy(sb->child, cb->child + TREE_SPLIT + 1,
				sizeof *cb->child * (hole.idx + 1));
			memcpy(sb->child + hole.idx + 2, cb->child + iterator.idx + 1,
				sizeof *cb->child * (TREE_MAX - iterator.idx));
			sb->child[hole.idx + 1] = temp;
		}
	} else { /* Equal split: leave the hole where it is. */
		memcpy(sibling->key, iterator.node->key + TREE_SPLIT,
			sizeof *sibling->key * (TREE_MAX - TREE_SPLIT));
#ifdef TREE_VALUE
		memcpy(sibling->value, iterator.node->value + TREE_SPLIT,
			sizeof *sibling->value * (TREE_MAX - TREE_SPLIT));
#endif
		if(iterator.height) {
			struct PB_(branch) *const cb = PB_(as_branch)(iterator.node),
				*const sb = PB_(as_branch)(sibling);
			memcpy(sb->child + 1, cb->child + TREE_SPLIT + 1,
				sizeof *cb->child * (TREE_MAX - TREE_SPLIT));
		}
	}
	/* Divide `TREE_MAX + 1` into two trees. */
	iterator.node->size = TREE_SPLIT, sibling->size = TREE_MAX - TREE_SPLIT;
	if(iterator.height) goto split; /* Loop max `\log_{TREE_MIN} size`. */
	hole.node->key[hole.idx] = key;
#ifdef TREE_VALUE
	if(value) *value = PB_(ref_to_valuep)(hole);
#endif
	assert(!new_head);
	return TREE_ABSENT;
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

#ifdef TREE_VALUE /* <!-- map */
/** Adds or gets `key` in `tree`. If `key` is already in `tree`, uses the
 old value, _vs_ <fn:<B>tree_update>. (This is only significant in trees with
 distinguishable keys.)
 @param[valuep] Only present if `TREE_VALUE` (map) was specified. If this
 parameter is non-null and a return value other then `TREE_ERROR`, this
 receives the address of the value associated with the `key`. This pointer is
 only guaranteed to be valid only while the `tree` doesn't undergo
 structural changes, (such as potentially calling it again.)
 @return Either `TREE_ERROR` (false) and doesn't touch `tree`, `TREE_ABSENT`
 and adds a new key with `key`, or `TREE_PRESENT` there was already an existing
 key. @throws[malloc] @order \Theta(\log |`tree`|) @allow */
static enum tree_result B_(tree_assign)(struct B_(tree) *const tree,
	const PB_(key) key, PB_(value) **const valuep)
	{ return assert(tree), PB_(update)(&tree->root, key, 0, valuep); }
#else /* map --><!-- set */
/** Only if `TREE_VALUE` is not defined. Adds `key` to `tree` only if it is a
 new value, otherwise returns `TREE_PRESENT`. See <fn:<B>tree_assign>, which is
 the map version. @allow */
static enum tree_result B_(tree_try)(struct B_(tree) *const tree,
	const PB_(key) key)
	{ return assert(tree), PB_(update)(&tree->root, key, 0); }
#endif /* set --> */

#ifdef TREE_VALUE /* <!-- map */
/** Adds or updates `key` in `tree`.
 @param[eject] If this parameter is non-null and a return value of
 `TREE_PRESENT`, the old key is stored in `eject`, replaced by `key`. A null
 value indicates that on conflict, the new key yields to the old key, as
 <fn:<B>tree_try>. This is only significant in trees with distinguishable keys.
 @param[value] Only present if `TREE_VALUE` (map) was specified. If this
 parameter is non-null and a return value other then `TREE_ERROR`, this
 receives the address of the value associated with the key.
 @return Either `TREE_ERROR` (false,) `errno` is set and doesn't touch `tree`;
 `TREE_ABSENT`, adds a new key; or `TREE_PRESENT`, there was already an
 existing key. @throws[malloc] @order \Theta(\log |`tree`|) @allow */
static enum tree_result B_(tree_update)(struct B_(tree) *const tree,
	const PB_(key) key, PB_(key) *const eject, PB_(value) **const value)
	{ return assert(tree), PB_(update)(&tree->root, key, eject, value); }
#else /* map --><!-- set */
/** Replaces `eject` by `key` or adds `key` in `tree`, but in a set. */
static enum tree_result B_(tree_update)(struct B_(tree) *const tree,
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
	if(!(rm = PB_(lookup_remove)(*tree, x, &parent.node)).node) return 0;
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
	PB_(node_lb)(&parent, provisional_x);
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
	struct { struct PB_(node) **head, **fresh, **iterator; } branch, leaf;
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
	const int keep_branch = sc->branch.iterator < sc->branch.fresh;
	assert(ref.node && ref.height && sc);
	if(keep_branch) *sc->branch.iterator = ref.node, sc->branch.iterator++;
	if(ref.height == 1) { /* Children are leaves. */
		unsigned n;
		for(n = 0; n <= ref.node->size; n++) {
			const int keep_leaf = sc->leaf.iterator < sc->leaf.fresh;
			struct PB_(node) *child = branch->child[n];
			if(keep_leaf) *sc->leaf.iterator = child, sc->leaf.iterator++;
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
	sc->branch.iterator = sc->branch.head;
	sc->leaf.iterator = sc->leaf.head;
	if(ref.height) {
		PB_(cannibalize_r)(ref, sc);
	} else { /* Just one leaf. */
		*sc->leaf.iterator = ref.node;
	}
}
/** Do the work of `src` cloned with `sc`. Called from <fn:<PB>clone>. */
static struct PB_(node) *PB_(clone_r)(struct PB_(tree) src,
	struct PB_(scaffold) *const sc) {
	struct PB_(node) *node;
	if(src.height) {
		struct PB_(branch) *const srcb = PB_(as_branch)(src.node),
			*const branch = PB_(as_branch)(node = *sc->branch.iterator++);
		unsigned i;
		struct PB_(tree) child;
		*node = *src.node; /* Copy node. */
		child.height = src.height - 1;
		for(i = 0; i <= src.node->size; i++) { /* Different links. */
			child.node = srcb->child[i];
			branch->child[i] = PB_(clone_r)(child, sc);
		}
	} else { /* Leaves. */
		node = *sc->leaf.iterator++;
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
	sc->branch.iterator = sc->branch.head;
	sc->leaf.iterator = sc->leaf.head;
	sub.node = PB_(clone_r)(*src, sc);
	sub.height = src->height;
	/* Used up all of them. No concurrent modifications, please. */
	assert(sc->branch.iterator == sc->leaf.head
		&& sc->leaf.iterator == sc->data + sc->no);
	return sub;
}
/** `source` is copied to, and overwrites, `tree`.
 @param[source] In the case where it's null or idle, if `tree` is empty, then
 it continues to be.
 @return Success, otherwise `tree` is not modified.
 @throws[malloc] @throws[EDOM] `tree` is null. @throws[ERANGE] The size of
 `source` nodes doesn't fit into `size_t`.
 @order \O(|`source`| + |`tree`|) time and temporary space. @allow */
static int B_(tree_clone)(struct B_(tree) *const tree,
	const struct B_(tree) *const source) {
	struct PB_(scaffold) sc;
	int success = 1;
	sc.data = 0; /* Need to keep this updated to catch. */
	if(!tree) { errno = EDOM; goto catch; }
	/* Count the number of nodes and set up to copy. */
	if(!PB_(nodes)(tree, &sc.victim) || !PB_(nodes)(source, &sc.source)
		|| (sc.no = sc.source.branches + sc.source.leaves) < sc.source.branches)
		{ errno = ERANGE; goto catch; } /* Overflow. */
	if(!sc.no) { PB_(clear)(tree); goto finally; } /* No need to allocate. */
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
		struct PB_(branch) *branch;
		if(!(branch = malloc(sizeof *branch))) goto catch;
		branch->base.size = 0;
		branch->child[0] = 0;
		*sc.branch.iterator++ = &branch->base;
	}
	while(sc.leaf.iterator != sc.data + sc.no) {
		struct PB_(node) *leaf;
		if(!(leaf = malloc(sizeof *leaf))) goto catch;
		leaf->size = 0;
		*sc.leaf.iterator++ = leaf;
	}
	/* Resources acquired; now we don't care about tree. */
	PB_(cannibalize)(tree, &sc);
	/* The scaffold has the exact number of nodes we need. Overwrite. */
	tree->root = PB_(clone)(&source->root, &sc);
	goto finally;
catch:
	success = 0;
	if(!sc.data) goto finally;
	while(sc.leaf.iterator != sc.leaf.fresh) {
		struct PB_(node) *leaf = *(--sc.leaf.iterator);
		assert(leaf);
		free(leaf);
	}
	while(sc.branch.iterator != sc.branch.fresh) {
		struct PB_(branch) *branch = PB_(as_branch)(*(--sc.branch.iterator));
		assert(branch);
		free(branch);
	}
finally:
	free(sc.data); /* Temporary memory. */
	return success;
}


/** @return Cursor at null in valid `tree`. @order \Theta(1) @allow */
static struct B_(tree_iterator) B_(tree_iterator)(struct B_(tree) *const tree)
	{ struct B_(tree_iterator) it; it._ = PB_(iterator)(tree); return it; }
/** @return Cursor in `tree` such that <fn:<B>tree_key> is the greatest key
 that is less-than-or-equal-to `x`, or if `x` is less than all in `tree`,
 <fn:<B>tree_iterator>. @order \Theta(\log |`tree`|) @allow */
static struct B_(tree_iterator) B_(tree_less)(struct B_(tree) *const
	tree, const PB_(key) x) {
	struct B_(tree_iterator) it;
	assert(tree);
	if(!(it._.root = &tree->root)) return it;
	it._.ref = PB_(less)(tree->root, x);
	return it;
}
/** @return Cursor in `tree` such that <fn:<B>tree_more> is the smallest key
 that is greater-than-or-equal-to `x`, or, <fn:<B>tree_iterator> if `x` is
 greater than all in `tree`. @order \Theta(\log |`tree`|) @allow */
static struct B_(tree_iterator) B_(tree_more)(struct B_(tree) *const
	tree, const PB_(key) x) {
	struct B_(tree_iterator) it;
	assert(tree);
	if(!(it._.root = &tree->root)) return it;
	it._.ref = PB_(more)(tree->root, x);
	return it;
}
/** @return Whether valid `it` is pointing to an element. This is the same as
 the return value from <fn:<B>tree_next> and <fn:<B>tree_previous> but intended
 for <fn:<B>tree_less> and <fn:<B>tree_more> because there's no check for
 validity. @allow */
static int B_(tree_has_element)(const struct B_(tree_iterator) *const it) {
	return assert(it), it->_.root && it->_.ref.node
		&& it->_.ref.idx <= it->_.ref.node->size;
}
/** @return Whether `it` still points at a valid index. @allow */
static int B_(tree_next)(struct B_(tree_iterator) *const it)
	{ return assert(it), PB_(next)(&it->_); }
/** @return Whether `it` still points at a valid index. @allow */
static int B_(tree_previous)(struct B_(tree_iterator) *const it)
	{ return assert(it), PB_(previous)(&it->_); }
/** @return Extract the key from `it` when it points at a valid index. @allow */
static PB_(key) B_(tree_key)(const struct B_(tree_iterator) *const it)
	{ return it->_.ref.node->key[it->_.ref.idx]; }
#ifdef TREE_VALUE /* <!-- map */
/** @return Extract the value from `it` when it points at a valid index, if
 `TREE_VALUE`. @allow */
static PB_(value) *B_(tree_value)(const struct B_(tree_iterator) *const it)
	{ return it->_.ref.node->value + it->_.ref.idx; }
#endif /* map --> */


static void PB_(unused_base_coda)(void);
static void PB_(unused_base)(void) {
	PB_(key) k; PB_(value) v; memset(&k, 0, sizeof k); memset(&v, 0, sizeof v);
	PB_(element)(0);
	B_(tree)(); B_(tree_)(0); B_(tree_clear)(0); B_(tree_count)(0);
	B_(tree_contains)(0, k); B_(tree_get_or)(0, k, v);
	B_(tree_less_or)(0, k, k); B_(tree_more_or)(0, k, k);
	B_(tree_next)(0); B_(tree_has_element)(0);
#ifdef TREE_VALUE
	B_(tree_bulk_assign)(0, k, 0); B_(tree_assign)(0, k, 0);
	B_(tree_update)(0, k, 0, 0); B_(tree_value)(0);
#else
	B_(tree_bulk_try)(0, k); B_(tree_try)(0, k);
	B_(tree_update)(0, k, 0);
#endif
	B_(tree_bulk_finish)(0); B_(tree_remove)(0, k); B_(tree_clone)(0, 0);
	B_(tree_iterator)(0); B_(tree_less)(0, k); B_(tree_more)(0, k);
	B_(tree_next)(0); B_(tree_previous)(0); B_(tree_key)(0);
	PB_(unused_base_coda)();
}
static void PB_(unused_base_coda)(void) { PB_(unused_base)(); }

/* Box override information. */
#define BOX_TYPE struct B_(tree)
#define BOX_CONTENT struct PB_(ref)
#define BOX_ PB_
#define BOX_MAJOR_NAME tree
#define BOX_NAME TREE_NAME

#endif /* body --> */

#endif /* base code --> */


#ifdef TREE_TRAIT /* <-- trait: Will be different on different includes. */
#define BOX_TRAIT_NAME TREE_TRAIT
#define PBT_(n) PB_(TREE_CAT(TREE_TRAIT, n))
#define BT_(n) B_(TREE_CAT(TREE_TRAIT, n))
#else /* trait --><!-- !trait */
#define PBT_(n) PB_(n)
#define BT_(n) B_(n)
#endif /* !trait --> */


#ifdef TREE_TO_STRING /* <!-- to string trait */
/** Thunk `r` -> `a`. */
static void PBT_(to_string)(const struct PB_(ref) *const r,
	char (*const a)[12]) {
#ifdef TREE_VALUE
	BT_(to_string)(r->node->key[r->idx], r->node->value + r->idx, a);
#else
	BT_(to_string)(r->node->key[r->idx], a);
#endif
}
#include "to_string.h" /** \include */
#undef TREE_TO_STRING
#ifndef TREE_TRAIT
#define TREE_HAS_TO_STRING
#endif
#endif /* to string trait --> */
#undef PBT_
#undef BT_


#if defined(TREE_TEST) && !defined(TREE_TRAIT) /* <!-- test base */
#include "../test/test_tree.h"
#endif /* test base --> */


#ifdef TREE_DEFAULT /* <!-- default trait */
#ifdef TREE_TRAIT
#define B_D_(n, m) TREE_CAT(B_(n), TREE_CAT(TREE_TRAIT, m))
#define PB_D_(n, m) TREE_CAT(tree, B_D_(n, m))
#else
#define B_D_(n, m) TREE_CAT(B_(n), m)
#define PB_D_(n, m) TREE_CAT(tree, B_D_(n, m))
#endif
/* `TREE_DEFAULT` is a valid <tag:<PB>value>. */
static const PB_(value) PB_D_(default, value) = TREE_DEFAULT;
/** This is functionally identical to <fn:<B>tree_get_or>, but a with a trait
 specifying a constant default value.
 @return The value associated with `key` in `tree`, (which can be null.) If
 no such value exists, the `TREE_DEFAULT` is returned.
 @order \O(\log |`tree`|). @allow */
static PB_(value) B_D_(tree, get)(const struct B_(tree) *const tree,
	const PB_(key) key) {
	struct PB_(ref) ref;
	return tree && tree->root.node && tree->root.height != UINT_MAX
		&& (ref = PB_(lookup_find)(tree->root, key)).node
		? *PB_(ref_to_valuep)(ref) : PB_D_(default, value);
}
static void PB_D_(unused, default_coda)(void);
static void PB_D_(unused, default)(void) {
	PB_(key) k; memset(&k, 0, sizeof k);
	B_D_(tree, get)(0, k); PB_D_(unused, default_coda)();
}
static void PB_D_(unused, default_coda)(void) { PB_D_(unused, default)(); }
#undef B_D_
#undef PB_D_
#undef TREE_DEFAULT
#endif /* default trait --> */


#ifdef TREE_EXPECT_TRAIT /* <!-- more */
#undef TREE_EXPECT_TRAIT
#else /* more --><!-- done */
#undef BOX_TYPE
#undef BOX_CONTENT
#undef BOX_
#undef BOX_MAJOR_NAME
#undef BOX_NAME
#undef TREE_NAME
#undef TREE_KEY
#undef TREE_ORDER
#undef TREE_COMPARE
#ifdef TREE_VALUE
#undef TREE_VALUE
#endif
#ifdef TREE_HAS_TO_STRING
#undef TREE_HAS_TO_STRING
#endif
#ifdef TREE_TEST
#undef TREE_TEST
#endif
#ifdef TREE_BODY
#undef TREE_BODY
#endif
#ifdef TREE_HEAD
#undef TREE_HEAD
#endif
#endif /* done --> */
#ifdef TREE_TRAIT
#undef TREE_TRAIT
#undef BOX_TRAIT_NAME
#endif
