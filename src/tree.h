/** @license 2022 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @abstract Stand-alone header <src/tree.h>; examples <test/test_tree.c>. On a
 compatible workstation, `make` creates the test suite of the examples.

 @subtitle Ordered tree

 A <tag:<B>tree> is an ordered collection of read-only <typedef:<PB>type>, and
 an optional <typedef:<PB>value> to go with them. One can make this a map or
 set, but in general, it can have identical keys, (a multi-map). Internally,
 this is a B-tree, described in <Bayer, McCreight, 1972 Large>.

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


#ifndef TREE_TYPE
#define TREE_TYPE unsigned
#endif
/** A comparable type, defaults to `unsigned`. */
typedef TREE_TYPE PB_(type);

#ifdef TREE_VALUE
/** On `TREE_VALUE`, otherwise just a set of integers. */
typedef TREE_VALUE PB_(value);
#endif

/** Returns a positive result if `a` is out-of-order with respect to `b`,
 inducing a total order. This is compatible, but less strict then the
 comparators from `bsearch` and `qsort`; it only needs to divide entries into
 two instead of three categories. */
typedef int (*PB_(compare_fn))(const PB_(type) a, const PB_(type) b);

#ifndef TREE_COMPARE /* <!-- !cmp */
/** The default `TREE_COMPARE` on `a` and `b` is integer comparison that
 results in ascending order. @implements <typedef:<PH>compare_fn> */
static int PB_(default_compare)(const PB_(type) a, const PB_(type) b)
	{ return a > b; }
#define TREE_COMPARE &PB_(default_compare)
#endif /* !cmp --> */

/* Check that `TREE_COMPARE` is a function implementing
 <typedef:<PB>compare_fn>, if defined. */
static const PB_(compare_fn) PB_(compare) = (TREE_COMPARE);

/* In <Knuth, 1998 Art 3> terminology, external, height zero, B-tree node of
 `TREE_ORDER`. Since the leaves carry no information, we just don't include
 them. With this optimization, it looks a lot like a leaf. */
struct PB_(outer) {
	unsigned char size;
	PB_(type) x[TREE_MAX]; /* Cache-friendly. */
#ifdef TREE_VALUE
	PB_(value) value[TREE_MAX]; /* Don't care about the value except the end. */
#endif
};
/* Internal node of `TRIE_ORDER` inherits from <tag:<PB>outer>. Since
 `height > 0`, there are `size + 1` links to other nodes. */
struct PB_(inner) { struct PB_(outer) base, *link[TREE_ORDER]; };
/** @return Upcasts `outer` to an inner node. */
static struct PB_(inner) *PB_(inner)(struct PB_(outer) *const outer)
	{ return (struct PB_(inner) *)(void *)
	((char *)outer - offsetof(struct PB_(inner), base)); }
/** @return Upcasts `outer` to a constant inner node. */
static const struct PB_(inner) *PB_(inner_c)(const struct PB_(outer) *
	const outer) { return (const struct PB_(inner) *)(const void *)
	((const char *)outer - offsetof(struct PB_(inner), base)); }

#if defined(TREE_VALUE) /* <!-- value */
/** On `TREE_VALUE`, creates a map from type to pointer-to-value. */
struct B_(tree_entry) { PB_(type) x; PB_(value) *value; };
/** On `TREE_VALUE`, otherwise it's just an alias for <typedef:<PB>type>. */
typedef struct B_(tree_entry) PB_(entry);
static void PB_(fill_entry)(PB_(entry) *const entry,
	struct PB_(outer) *const outer, const unsigned i)
	{ entry->x = outer->x[i], entry->value = outer->value + i; }
static PB_(type) PB_(to_x)(const PB_(entry) *const entry)
	{ return entry->x; }
#else /* value --><!-- !value */
typedef PB_(type) PB_(value);
typedef PB_(value) PB_(entry);
static void PB_(fill_entry)(PB_(entry) *const entry,
	struct PB_(outer) *const outer, const unsigned i)
	{ *entry = outer->x[i]; }
static PB_(type) PB_(to_x)(const PB_(type) *const x) { return *x; }
#endif /* !entry --> */

/** To initialize it to an idle state, see <fn:<U>tree>, `TRIE_IDLE`, `{0}`
 (`C99`), or being `static`.

 ![States.](../doc/states.png) */
struct B_(tree);
struct B_(tree) {
	struct PB_(outer) *node; /* Root if top level. */
	unsigned height; /* Top level flag UINT_MAX: empty but non-idle. */
};

/* <!-- iterate interface */

struct PB_(iterator) {
	const struct B_(tree) *tree;
	struct B_(tree) n;
	unsigned idx; /* Could pack it, but probably not worth the extra code. */
};

/** Corrects `it` for going off the end.
 @return Whether it is addressing a valid item. */
static int PB_(pin)(struct PB_(iterator) *const it) {
	unsigned a0;
	struct B_(tree) t, next;
	PB_(type) x;
	assert(it);
	if(!it->tree || it->tree->height == UINT_MAX) return 0; /* Empty. */
	if(!it->n.node) { /* Begin. */
		it->n.node = it->tree->node, assert(it->n.node),
			it->n.height = it->tree->height, it->idx = 0;
		while(it->n.height) it->n.height--,
			it->n.node = PB_(inner_c)(it->n.node)->link[0], assert(it->n.node);
	}
	if(it->idx < it->n.node->size) return 1; /* Likely. */
	if(!it->n.node->size) return 0; /* The empty nodes are always at the end. */
	next.node = 0, x = it->n.node->x[it->n.node->size - 1];
	for(t = *it->tree; t.height;
		t.node = PB_(inner_c)(t.node)->link[a0], t.height--) {
		int cmp; unsigned a1 = t.node->size; a0 = 0;
		while(a0 < a1) {
			const unsigned m = (a0 + a1) / 2;
			cmp = PB_(compare)(x, t.node->x[m]);
			if(cmp > 0) a0 = m + 1; else a1 = m;
		}
		if(a0 + 1 < t.node->size)
			next.node = PB_(inner_c)(t.node)->link[a0 + 1],
			next.height = t.height - 1;
	}
	if(!next.node) return 0; /* The end, normally. */
	it->n = next;
	return 1; /* Jumped nodes. */
}

/** Loads the first element of `tree` (can be null) into `it`.
 @implements begin */
static void PB_(begin)(struct PB_(iterator) *const it,
	const struct B_(tree) *const tree)
	{ assert(it); it->tree = tree, it->n.node = 0; }

/** Advances `it` and returns the last or null in a static entry.
 @implements next */
static const PB_(entry) *PB_(next)(struct PB_(iterator) *const it) {
	static PB_(entry) e;
	assert(it);
	printf("_next_\n");
	return PB_(pin)(it) ? (PB_(fill_entry)(&e, it->n.node, it->idx++), &e) : 0;
}

/* iterate --> */

/** @param[tree] Can be null. @return Finds the smallest entry in `tree` that
 is not less than `no`. If `no` is higher than any of `tree`, it will be placed
 just passed the end. @order \O(\log |`tree`|) */
static struct PB_(iterator) PB_(lower)(const struct B_(tree) *const tree,
	const PB_(type) x) {
	struct B_(tree) t;
	struct PB_(iterator) it;
	unsigned a0;
	it.tree = 0;
	if(!tree || !tree->node || tree->height == UINT_MAX) return it;
	for(t = *tree; ; t.node = PB_(inner_c)(t.node)->link[a0], t.height--) {
		unsigned a1 = t.node->size; PB_(type) m; a0 = 0;
		if(!a1) continue;
		do {
			const unsigned mi = (a0 + a1) / 2;
			m = t.node->x[mi];
			if(PB_(compare)(x, m) > 0) a0 = mi + 1; else a1 = mi;
		} while(a0 < a1);
		/* [!(x > m) -> x <= m] && [m <= x] -> [x == m]? */
		if(!t.height || PB_(compare)(m, x) <= 0) break;
	}
	it.tree = tree, it.n = t, it.idx = a0;
	return it;
}

/** Frees a non-empty `tree` and it's children recursively. */
static void PB_(clear_r)(struct B_(tree) tree) {
	/* FIXME: This doesn't want to clear one. */
	assert(tree.node);
	if(!tree.height) {
		free(PB_(inner)(tree.node));
	} else {
		struct B_(tree) sub;
		unsigned i;
		sub.height = tree.height - 1;
		for(i = 0; i <= tree.node->size; i++)
		sub.node = PB_(inner)(tree.node)->link[i], PB_(clear_r)(sub);
		free(tree.node);
	}
}

/** Initializes `tree` to idle. @order \Theta(1) @allow */
static struct B_(tree) B_(tree)(void)
	{ struct B_(tree) tree; tree.node = 0; tree.height = 0; return tree; }

/** Returns an initialized `tree` to idle, `tree` can be null. @allow */
static void B_(tree_)(struct B_(tree) *const tree) {
	if(!tree) return; /* Null. */
	if(!tree->node) { /* Idle. */
		assert(!tree->height);
	} else if(tree->height == UINT_MAX) { /* Empty. */
		assert(tree->node); free(tree->node);
	} else {
		struct B_(tree) t = *tree;
		PB_(clear_r)(t);
	}
	/*struct PB_(outer_tree) *clear_all = (struct PB_(outer_tree) *)1;*/
	*tree = B_(tree)();
}

/** Stores an iteration in a tree. Generally, changes in the topology of the
 tree invalidate it. */
struct B_(tree_iterator) { struct PB_(iterator) it; };

/** Loads the first element of `tree` (can be null) into `it`. */
static struct B_(tree_iterator) B_(tree_begin)(const struct B_(tree) *const
	tree)
	{ struct B_(tree_iterator) it; PB_(begin)(&it.it, tree); return it; }

/** Advances `it`.
 @return If the iteration is finished, null, otherwise, if `TREE_VALUE`, a
 static <typedef:<B>tree_entry> of a copy of the key and pointer to the value,
 otherwise, a pointer to key. @allow */
static const PB_(entry) *B_(tree_next)(struct B_(tree_iterator) *const it)
	{ return PB_(next)(&it->it); }

#if 0
/** Counts the of the items in initialized `it`. @order \O(|`it`|) @allow */
static size_t B_(trie_size)(const struct B_(trie_iterator) *const it)
	{ return assert(it), PB_(size_r)(&it->i); }
#endif

/** @param[tree] Can be null. @return Finds the smallest entry in `tree` that
 is not less than `x`. If `x` is higher than any of `tree`, it will be placed
 just passed the end. @order \O(\log |`tree`|) @allow */
static struct B_(tree_iterator) B_(tree_lower)(const struct B_(tree)
	*const tree, const PB_(type) x)
	{ struct B_(tree_iterator) it; it.it = PB_(lower)(tree, x); return it; }

/** @return Lowest match for `x` in `tree` or null no such item exists.
 @order \O(\log |`tree`|) @allow */
static const PB_(entry) *B_(tree_get)(const struct B_(tree) *const tree,
	const PB_(type) x) {
	struct PB_(iterator) it = PB_(lower)(tree, x);
	/*return it.tree && it.n.node ? it.n.node->x + it.idx : 0;*/
	return PB_(next)(&it);
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
static PB_(value) *B_(tree_bulk_add)(struct B_(tree) *const tree, PB_(type) x) {
	/* `node` is outer and `head` is inner up to the point where they merge. */
	struct PB_(outer) *node = 0, *head = 0;
	printf("bulk():\n");
	if(!tree) return 0;
	if(!tree->node) { /* Idle tree. */
		assert(!tree->height);
		if(!(node = malloc(sizeof *node))) goto catch;
		node->size = 0;
		tree->node = node;
		printf("Idle tree: new %s.\n", orcify(node));
	} else if(tree->height == UINT_MAX) { /* Empty tree. */
		tree->height = 0;
		node = tree->node;
		node->size = 0;
		printf("Empty tree, %s.\n", orcify(node));
	} else {
		struct B_(tree) space = { 0, 0 }; /* Furthest node with space. */
		PB_(type) *last = 0; /* Key of the last for comparing with arg. */
		unsigned new_nodes, n; /* Count new nodes. */
		struct PB_(outer) *tail = 0; /* New nodes. */
		struct PB_(inner) *pretail = 0;
		PB_(print)(tree);

		{ /* Figure out where `space` and `last` are in `log size`. */
			struct B_(tree) expl;

			for(expl = *tree; ; expl.node
				= PB_(inner)(expl.node)->link[expl.node->size], expl.height--) {
				printf("dowhile expl %s:%u with %u size\n",
					orcify(expl.node), expl.height, expl.node->size);
				if(expl.node->size < TREE_MAX) space = expl;
				if(expl.node->size) last = expl.node->x + expl.node->size - 1;
				if(!expl.height) break;
			}
			assert(last); /* Else it would be empty and we would not be here. */
			printf("dowhile expl finished %s:%u, last: %u, space %s:%u\n",
				orcify(expl.node), expl.height, 0/**last*/,
				orcify(space.node), space.height);
		}

		/* Verify that the argument is not smaller than the largest in tree. */
		if(PB_(compare)(*last, x) > 0) { errno = EDOM; goto catch; }

		/* One outer, and the rest inner. */
		new_nodes = n = space.node ? space.height : tree->height + 2;
		printf("new_nodes: %u, tree height %u\n", new_nodes, tree->height);
		if(!n) {
			node = space.node;
		} else {
			if(!(node = tail = malloc(sizeof *tail))) goto catch;
			tail->size = 0;
			printf("new tail: %s.\n", orcify(tail));
			while(--n) {
				struct PB_(inner) *inner;
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
		if(!space.node) { /* Add tree to head. */
			struct PB_(inner) *const inner = PB_(inner)(head);
			printf("adding the existing root, %s to %s\n",
				orcify(tree->node), orcify(head));
			assert(new_nodes > 1);
			inner->link[1] = inner->link[0], inner->link[0] = tree->node;
			node = tree->node = head, tree->height++;
		} else if(space.height) { /* Add head to tree. */
			struct PB_(inner) *const inner = PB_(inner)(node = space.node);
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
		struct PB_(outer) *const next = PB_(inner)(head)->link[0];
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

/* <!-- box */
#define BOX_ PB_
#define BOX_CONTAINER struct B_(tree)
#define BOX_CONTENTS PB_(entry)

#ifdef TREE_TEST /* <!-- test */
/* Forward-declare. */
static void (*PB_(to_string))(const PB_(entry) *, char (*)[12]);
static const char *(*PB_(tree_to_string))(const struct B_(tree) *);
#include "../test/test_tree.h"
#endif /* test --> */

static void PB_(unused_base_coda)(void);
static void PB_(unused_base)(void) {
	B_(tree)(); B_(tree_)(0); B_(tree_begin)(0); B_(tree_next)(0);
	B_(tree_lower)(0, 0);
	B_(tree_get)(0, 0);
	B_(tree_bulk_add)(0, 0);
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
#undef TREE_TO_STRING_TRAIT
#undef TREE_TRAITS
