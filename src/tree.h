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
#define TREE_MAX 2
#if TREE_MAX < 2 || TREE_MAX > UCHAR_MAX
#error TREE_MAX parameter range `[2, UCHAR_MAX]`.
#endif
#define TREE_ORDER (TREE_MAX + 1) /* Maximum branching factor. */
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
	PB_(type) x[TREE_MAX];
#ifdef TREE_VALUE
	PB_(value) value[TREE_MAX];
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
struct B_(tree) { struct PB_(outer) *root; unsigned height; }; /* Height +1. */

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
		unsigned a1 = trunk->size; a0 = 0, assert(a1);
		do {
			const unsigned m = (a0 + a1) / 2;
			cmp = trunk->x[m];
			if(PB_(compare)(no, cmp) > 0) a0 = m + 1; else a1 = m;
		} while(a0 < a1);
		if(!--h || no == cmp) break; /* +1 differentiate between empty. */
	}
	it.tree = tree, it.trunk = trunk, it.idx = a0;
	return it;
}

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
		if(!--h) break; /* Height +1. */
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
			if(--h) break; /* +1 */
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

/** Frees `tr` at `h` and it's children recursively. Actual `height`. */
static void PB_(clear_r)(struct PB_(outer) *const tree, const size_t height) {
	/* FIXME: This doesn't want to clear one. */
	assert(tree && height);
	if(height) {
		unsigned i;
		for(i = 0; i <= tree->size; i++)
			PB_(clear_r)(PB_(inner)(tree)->link[i], height - 1);
		free(PB_(inner)(tree));
	} else {
		free(tree);
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
	if(tree->height) PB_(clear_r)(tree->root, tree->height - 1); /* +1 */
	else free(tree->root);
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

#include "orcish.h"
static void PB_(print)(const struct B_(tree) *const tree);

/** `x` must be not less than the largest element in `tree`.
 @throws[EDOM] `x` is smaller than the largest element in `tree`.
 @throws[malloc] */
static PB_(value) *B_(bulk_add)(struct B_(tree) *const tree, PB_(type) x) {
	struct B_(tree) t;
	struct PB_(outer) *outer = 0;
	struct PB_(inner) *inner = 0;
	assert(tree);
	printf("bulk()\n");
	if(t.height = tree->height) {
		struct B_(tree) back = { 0, 0 };
		PB_(type) *last = 0;
		struct PB_(inner) *tail = 0;
		struct PB_(outer) *head;
		unsigned new_nodes, n;
		/* Figure out where the end is. */
		t.root = tree->root, assert(t.root);
		do {
			if(t.root->size < TREE_MAX)
				back.root = t.root, back.height = t.height;
			if(!t.root->size) break;
			last = t.root->x + t.root->size - 1;
		} while(t.root = PB_(inner)(t.root)->link[t.root->size - 1], --t.height);
		printf("dowhile %s height %u\n", orcify(t.root), t.height);
		assert(last); printf("non-empty last: %u\n", *last);
		if(PB_(compare)(*last, x)) { errno = EDOM; return 0; } /* Not max. */
		if(!t.height) t.root = back.root, t.height = back.height;
		new_nodes = n = t.root ? t.height : tree->height + 1;
		printf("new nodes: %u; ptr %s\n", new_nodes, orcify(t.root));
		/* New nodes: one outer, and the rest inner. */
		if(n) {
			if(!(outer = malloc(sizeof *outer))) goto catch;
			outer->size = 0;
			n--;
		}
		while(n) {
			struct PB_(inner) *a;
			if(!(a = malloc(sizeof *a))) goto catch;
			a->base.size = 0;
			if(inner) a->link[0] = &inner->base; else a->link[0] = 0, tail = a;
			inner = a;
			n--;
		}
		/* Now we are past the catch; modify the original. */
		if(tail) tail->link[0] = outer, head = &inner->base;
		else head = outer;
		if(!t.root) { /* Adding level. */
			printf("adding maximal\n");
			assert(new_nodes > 1);
			inner->link[1] = inner->link[0], inner->link[0] = tree->root;
			tree->root = t.root = &inner->base, tree->height++;
			printf("now tree root size is %u.\n", t.root->size);
			assert(!t.root->size);
		} else if(t.height > 1) { /* Adding side. */
			printf("more nodes\n");
			assert(t.root->size < TREE_MAX);
			inner = PB_(inner)(t.root);
			inner->link[inner->base.size] = head;
			assert(0);
		}
	} else {
		/* Empty tree. */
		if(!(t.root = tree->root) && !(t.root = malloc(sizeof *t.root)))
			goto catch;
		t.root->size = 0, tree->root = t.root, tree->height = t.height = 1;
		printf("empty root node\n");
	}
	assert(t.root && t.root->size < TREE_MAX);
	t.root->x[t.root->size++] = x;
	PB_(print)(tree);
#ifdef TREE_VALUE
	return t.root->value + t.root->size;
#else
	return t.root->x + t.root->size;
#endif
catch:
	free(outer);
	if(inner) for( ; ; ) {
		outer = inner->link[0]; free(inner);
		if(!outer) break; inner = PB_(inner)(outer);
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
