/** @license 2022 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @abstract Stand-alone header <src/tree.h>; examples <test/test_tree.c>. On a
 compatible workstation, `make` creates the test suite of the examples.

 @subtitle Ordered tree

 A <tag:<B>tree> is an ordered collection of read-only <typedef:<PB>key>, and
 an optional <typedef:<PB>value> to go with them. This can be a map or set, but
 in general, it can have identical keys, (a multi-map). Internally, this is a
 B-tree, described in <Bayer, McCreight, 1972 Large>.

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
#include <stddef.h> /* fixme: Wtf? stdlib and string should include it. */
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
#define TREE_MAX 2
#if TREE_MAX < 2 || TREE_MAX > UCHAR_MAX
#error TREE_MAX parameter range `[2, UCHAR_MAX]`.
#endif
#define TREE_ORDER (TREE_MAX + 1) /* Maximum branching factor. */
#endif /* idempotent --> */


#if TREE_TRAITS == 0 /* <!-- base code */


#ifndef TREE_KEY
#define TREE_KEY unsigned
#endif
/** A comparable type, defaults to `unsigned`. Note that `key` is used loosely;
 there can be multiple keys with the same value stored in the same
 <tag:<B>tree>, if one chooses. */
typedef TREE_KEY PB_(key);
/** Read-only <typedef:<PB>key>. */
typedef const TREE_KEY PB_(key_c);

#ifdef TREE_VALUE
/** On `TREE_VALUE`, otherwise just a (multi)-set of <typedef:<PB>key>. */
typedef TREE_VALUE PB_(value);
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

/* B-tree leaf of `TREE_ORDER`, (`TREE_MAX + 1`); the order is the maximum
 branching factor, as <Knuth, 1998 Art 3>. However, Knuth's leaves are
 imaginary in this data structure, so we use the original terminology from
 <Bayer, McCreight, 1972 Large>: leaves are height-zero nodes. */
struct PB_(leaf) {
	unsigned char size; /* `[0, TREE_MAX]`. */
	PB_(key) x[TREE_MAX]; /* Cache-friendly lookup; all but one value. */
#ifdef TREE_VALUE
	PB_(value) value[TREE_MAX];
#endif
};
/* B-tree branch is a <tag:<PB>leaf> and links to `size + 1` nodes. */
struct PB_(branch) { struct PB_(leaf) base, *link[TREE_ORDER]; };
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
 pointer-to-<typedef:<PB>value>. The reason `x` is a pointer is because it has
 to be nullifiable. The reason `value` is a pointer is because the type
 is stored as one contiguous array in the node for caching, it is not connected
 to the value. */
struct B_(tree_entry) { PB_(key) *x; PB_(value) *value; };
/** On `TREE_VALUE`, otherwise it's just an alias for
 pointer-to-<typedef:<PB>key>. */
typedef struct B_(tree_entry) PB_(entry);
static PB_(entry) PB_(null_entry)(void)
	{ const PB_(entry) e = { 0, 0 }; return e; }
static PB_(entry) PB_(to_entry)(struct PB_(leaf) *const leaf,
	const unsigned i) {
	PB_(entry) e;
	e.x = leaf->x + i, e.value = leaf->value + i;
	return e;
}
static PB_(key) PB_(to_x)(const PB_(entry) entry) { return *entry.x; }
static PB_(value) *PB_(to_value)(PB_(entry) entry) { return entry.value; }
#else /* value --><!-- !value */
typedef PB_(key) PB_(value);
typedef PB_(key) *PB_(entry);
static PB_(entry) PB_(null_entry)(void) { return 0; }
static PB_(entry) PB_(to_entry)(struct PB_(leaf) *const leaf,
	const unsigned i) { return leaf->x + i; }
static PB_(key) PB_(to_x)(const PB_(key) *const x) { return *x; }
static PB_(value) *PB_(to_value)(PB_(key) *const x) { return x; }
#endif /* !entry --> */

/** To initialize it to an idle state, see <fn:<U>tree>, `TRIE_IDLE`, `{0}`
 (`C99`), or being `static`.

 ![States.](../doc/states.png) */
struct B_(tree);
struct B_(tree) { struct PB_(leaf) *root; unsigned height; };
/* Top level tree: `node` is root, flag UINT_MAX: empty but non-idle. */

/* It was very difficult to have the key-value entry pair as the contents,
 everywhere I was blocked, because they are not contiguous. One would think
 it's easy to pass around a pair, but we have to make another pair for `const`,
 and then there are two different `struct`, so `is_content` doesn't work. I
 don't think that's fixable without totally duplicating all code in all the
 boxes. It's easier to ignore the value, but less useful. */
#define BOX_CONTENT PB_(key) *
/** Is `x` not null? @implements `is_content` */
static int PB_(is_content)(const PB_(key_c) k) { return !!k; }
/* @implements `forward` */
struct PB_(forward) {
	const struct B_(tree) *tree;
	const struct PB_(leaf) *cur;
	unsigned height, idx;
};
/** @return Whether it is addressing a valid item. */
static int PB_(forward_pin)(struct PB_(forward) *const it) {
	unsigned a0;
	struct B_(tree) t, next;
	PB_(key) x;
	assert(it);
	if(!it->tree || it->tree->height == UINT_MAX) return 0;
	/* Off the left: !it->cur. */
	if(!it->cur) {
		it->cur = it->tree->root, assert(it->cur),
			it->height = it->tree->height, it->idx = 0;
		while(it->height) it->height--,
			it->cur = PB_(branch_c)(it->cur)->link[0], assert(it->cur);
	}
	if(it->idx < it->cur->size) return 1; /* Likely. */
	if(!it->cur->size) return 0; /* The empty nodes are always at the end. */
	/* Go down the tree again and note the next. */
	next.root = 0, x = it->cur->x[it->cur->size - 1];
	for(t = *it->tree; t.height;
		t.root = PB_(branch_c)(t.root)->link[a0], t.height--) {
		int cmp; unsigned a1 = t.root->size; a0 = 0;
		while(a0 < a1) {
			const unsigned m = (a0 + a1) / 2;
			cmp = PB_(compare)(x, t.root->x[m]);
			if(cmp > 0) a0 = m + 1; else a1 = m;
		}
		if(a0 + 1 < t.root->size)
			next.root = PB_(branch_c)(t.root)->link[a0 + 1],
			next.height = t.height - 1;
	}
	/* Off the right: it->idx >= it->cur->size && !next.node */
	if(!next.root) return 0;
	it->cur = next.root, it->height = next.height;
	return 1; /* Jumped nodes. */
}
/** @return Before `tree`. @implements `forward_begin` */
static struct PB_(forward) PB_(forward_begin)(const struct B_(tree) *const
	tree) {
	struct PB_(forward) it;
	it.tree = tree, it.cur = 0, it.height = 0, it.idx = 0;
	return it;
}
/** Advances `it` to the next element. @return A pointer to the current
 element or null. @implements `forward_next` */
static PB_(key_c) *PB_(forward_next)(struct PB_(forward) *const it) {
	printf("_next_\n");
	return assert(it), PB_(forward_pin)(it) ? it->cur->x + it->idx++ : 0;
}

/*#define BOX_ITERATOR <- doesn't define remove yet? */
struct PB_(iterator) {
	struct B_(tree) *tree;
	struct PB_(leaf) *cur;
	unsigned height, idx;
};
/** @return Whether it is addressing a valid item. */
static int PB_(pin)(struct PB_(iterator) *const it) {
	unsigned a0;
	struct B_(tree) t, next;
	PB_(key) x;
	assert(it);
	if(!it->tree || it->tree->height == UINT_MAX) return 0;
	/* Off the left: !it->cur. */
	if(!it->cur) {
		it->cur = it->tree->root, assert(it->cur),
			it->height = it->tree->height, it->idx = 0;
		while(it->height) it->height--,
			it->cur = PB_(branch_c)(it->cur)->link[0], assert(it->cur);
	}
	if(it->idx < it->cur->size) return 1; /* Likely. */
	if(!it->cur->size) return 0; /* The empty nodes are always at the end. */
	/* Go down the tree again and note the next. */
	next.root = 0, x = it->cur->x[it->cur->size - 1];
	for(t = *it->tree; t.height;
		t.root = PB_(branch_c)(t.root)->link[a0], t.height--) {
		int cmp; unsigned a1 = t.root->size; a0 = 0;
		while(a0 < a1) {
			const unsigned m = (a0 + a1) / 2;
			cmp = PB_(compare)(x, t.root->x[m]);
			if(cmp > 0) a0 = m + 1; else a1 = m;
		}
		if(a0 + 1 < t.root->size)
			next.root = PB_(branch_c)(t.root)->link[a0 + 1],
			next.height = t.height - 1;
	}
	/* Off the right: it->idx >= it->cur->size && !next.node */
	if(!next.root) return 0;
	it->cur = next.root, it->height = next.height;
	return 1; /* Jumped nodes. */
}
/** @return Before `tree`. @implements `forward_begin` */
static struct PB_(iterator) PB_(begin)(struct B_(tree) *const tree) {
	struct PB_(iterator) it;
	it.tree = tree, it.cur = 0, it.height = 0, it.idx = 0;
	return it;
}
/** Advances `it` to the next element. @return A pointer to the current
 element or null. @implements `next` */
static PB_(key) *PB_(next)(struct PB_(iterator) *const it) {
	printf("_next_\n");
	assert(it);
	return assert(it), PB_(pin)(it) ? it->cur->x + it->idx++ : 0;
}

/** @param[tree] Can be null. @return Finds the smallest entry in `tree` that
 is not less than `no`. If `no` is higher than any of `tree`, it will be placed
 just passed the end. @order \O(\log |`tree`|)
 @fixme Test with all the same value on multiple levels; I suspect it will
 return the closest lower in the node, not the global lower bound. */
static struct PB_(iterator) PB_(lower)(struct B_(tree) *const tree,
	const PB_(key) x) {
	struct B_(tree) t;
	struct PB_(iterator) it;
	unsigned a0;
	it.tree = 0;
	if(!tree || !tree->root || tree->height == UINT_MAX) return it;
	for(t = *tree; ; t.root = PB_(branch_c)(t.root)->link[a0], t.height--) {
		unsigned a1 = t.root->size; PB_(key) m; a0 = 0;
		if(!a1) continue;
		do {
			const unsigned mi = (a0 + a1) / 2;
			m = t.root->x[mi];
			if(PB_(compare)(x, m) > 0) a0 = mi + 1; else a1 = mi;
		} while(a0 < a1);
		/* [!(x > m) -> x <= m] && [m <= x] -> [x == m]? */
		if(!t.height || PB_(compare)(m, x) <= 0) break;
	}
	it.tree = tree, it.cur = t.root, it.height = t.height, it.idx = a0;
	return it;
}

/** Frees a non-empty `tree` and it's children recursively. */
static void PB_(clear_r)(struct B_(tree) tree) {
	/* FIXME: This doesn't want to clear one. */
	assert(tree.root);
	if(!tree.height) {
		free(PB_(branch)(tree.root));
	} else {
		struct B_(tree) sub;
		unsigned i;
		sub.height = tree.height - 1;
		for(i = 0; i <= tree.root->size; i++)
		sub.root = PB_(branch)(tree.root)->link[i], PB_(clear_r)(sub);
		free(tree.root);
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
		struct B_(tree) t = *tree;
		PB_(clear_r)(t);
	}
	/* fixme: Have another param of <fn:<PB>clear_r> like trie, doesn't delete
	 one. */
	/*struct PB_(outer_tree) *clear_all = (struct PB_(outer_tree) *)1;*/
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
/*static PB_(entry) B_(tree_next)(struct B_(tree_iterator) *const it)
	{ return PB_(next)(&it->_); }*/

#if 0
/** Counts the of the items in initialized `it`. @order \O(|`it`|) @allow */
static size_t B_(trie_size)(const struct B_(trie_iterator) *const it)
	{ return assert(it), PB_(size_r)(&it->i); }
#endif

/** @param[tree] Can be null. @return Finds the smallest entry in `tree` that
 is not less than `x`. If `x` is higher than any of `tree`, it will be placed
 just passed the end. @order \O(\log |`tree`|) @allow */
static struct B_(tree_iterator) B_(tree_lower)(struct B_(tree) *const tree,
	const PB_(key) x)
	{ struct B_(tree_iterator) it; it._ = PB_(lower)(tree, x); return it; }

/** @return Lowest value match for `x` in `tree` or null no such item exists.
 @order \O(\log |`tree`|) @allow */
static PB_(value) *B_(tree_get)(struct B_(tree) *const tree,
	const PB_(key) x) {
	struct PB_(iterator) it = PB_(lower)(tree, x);
	PB_(entry) e;
	assert(0);
	return 0;//PB_(is_content)(e = PB_(next)(&it)) ? PB_(to_value)(e) : 0;
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
	/* `node` is outer and `head` is inner up to the point where they merge. */
	struct PB_(leaf) *node = 0, *head = 0;
	printf("bulk():\n");
	if(!tree) return 0;
	if(!tree->root) { /* Idle tree. */
		assert(!tree->height);
		if(!(node = malloc(sizeof *node))) goto catch;
		node->size = 0;
		tree->root = node;
		printf("Idle tree: new %s.\n", orcify(node));
	} else if(tree->height == UINT_MAX) { /* Empty tree. */
		tree->height = 0;
		node = tree->root;
		node->size = 0;
		printf("Empty tree, %s.\n", orcify(node));
	} else {
		struct B_(tree) space = { 0, 0 }; /* Furthest node with space. */
		PB_(key) *last = 0; /* Key of the last for comparing with arg. */
		unsigned new_nodes, n; /* Count new nodes. */
		struct PB_(leaf) *tail = 0; /* New nodes. */
		struct PB_(branch) *pretail = 0;
		PB_(print)(tree);

		{ /* Figure out where `space` and `last` are in `log size`. */
			struct B_(tree) expl;

			for(expl = *tree; ; expl.root
				= PB_(branch)(expl.root)->link[expl.root->size], expl.height--) {
				printf("dowhile expl %s:%u with %u size\n",
					orcify(expl.root), expl.height, expl.root->size);
				if(expl.root->size < TREE_MAX) space = expl;
				if(expl.root->size) last = expl.root->x + expl.root->size - 1;
				if(!expl.height) break;
			}
			assert(last); /* Else it would be empty and we would not be here. */
			printf("dowhile expl finished %s:%u, last: %u, space %s:%u\n",
				orcify(expl.root), expl.height, 0/**last*/,
				orcify(space.root), space.height);
		}

		/* Verify that the argument is not smaller than the largest in tree. */
		if(PB_(compare)(*last, x) > 0) { errno = EDOM; goto catch; }

		/* One outer, and the rest inner. */
		new_nodes = n = space.root ? space.height : tree->height + 2;
		printf("new_nodes: %u, tree height %u\n", new_nodes, tree->height);
		if(!n) {
			node = space.root;
		} else {
			if(!(node = tail = malloc(sizeof *tail))) goto catch;
			tail->size = 0;
			printf("new tail: %s.\n", orcify(tail));
			while(--n) {
				struct PB_(branch) *inner;
				if(!(inner = malloc(sizeof *inner))) goto catch;
				inner->base.size = 0;
				printf("new inner: %s.\n", orcify(inner));
				if(!head) inner->link[0] = 0, pretail = inner; /* First loop. */
				else inner->link[0] = head; /* Not first loop. */
				head = &inner->base;
			}
		}

		/* Post-error; modify the original as needed. */
		if(pretail) pretail->link[0] = tail;
		else head = node;
		if(!space.root) { /* Add tree to head. */
			struct PB_(branch) *const inner = PB_(branch)(head);
			printf("adding the existing root, %s to %s\n",
				orcify(tree->root), orcify(head));
			assert(new_nodes > 1);
			inner->link[1] = inner->link[0], inner->link[0] = tree->root;
			node = tree->root = head, tree->height++;
		} else if(space.height) { /* Add head to tree. */
			struct PB_(branch) *const inner = PB_(branch)(node = space.root);
			printf("adding the linked list, %s to %s at %u\n",
				orcify(head), orcify(inner), inner->base.size + 1);
			assert(new_nodes);
			inner->link[inner->base.size + 1] = head;
		}
	}
	assert(node && node->size < TREE_MAX);
	node->x[node->size] = x;
	/*PB_(print)(tree); uninitialized */
#ifdef TREE_VALUE
	return node->value + node->size++;
#else
	return node->x + node->size++;
#endif
catch:
	printf("!!! freeing %s\n", orcify(node));
	free(node);
	if(head) for( ; ; ) {
		struct PB_(leaf) *const next = PB_(branch)(head)->link[0];
		printf("!!! freeing %s\n", orcify(head));
		free(head);
		if(!next) break;
		head = next;
	}
	if(!errno) errno = ERANGE;
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
static void (*PB_(to_string))(PB_(key_c), char (*)[12]);
static const char *(*PB_(tree_to_string))(const struct B_(tree) *);
#include "../test/test_tree.h"
#endif /* test --> */

static void PB_(unused_base_coda)(void);
static void PB_(unused_base)(void) {
	PB_(forward_begin)(0); PB_(forward_next)(0);
	B_(tree)(); B_(tree_)(0); B_(tree_begin)(0); //B_(tree_next)(0);
	B_(tree_lower)(0, 0);
	B_(tree_get)(0, 0);
	B_(tree_bulk_add)(0, 0);
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
#endif /* !trait --> */
#undef TREE_TO_STRING_TRAIT
#undef TREE_TRAITS
