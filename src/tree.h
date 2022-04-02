/** @license 2022 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @abstract Stand-alone header <src/tree.h>; examples <test/test_tree.c>. On a
 compatible workstation, `make` creates the test suite of the examples.

 @subtitle Integral tree

 A <tag:<U>tree> is a collection of ordered integers
 <typedef:<PU>key> to an optional <typedef:<PU>value>, stored in a B-tree,
 described in <Bayer, McCreight, 1972 Large>.

 @param[TREE_NAME, TREE_KEY]
 `<U>` that satisfies `C` naming conventions when mangled, required, and an
 optional integral type, <typedef:<PU>key>, whose default is `unsigned int`.
 `<PU>` is private, whose names are prefixed in a manner to avoid collisions.

 @param[TREE_VALUE, TREE_KEY_IN_VALUE]
 `TRIE_VALUE` is an optional payload type to go with the key.
 `TRIE_KEY_IN_VALUE` is an optional <typedef:<PU>key_fn> that picks out the key
 from the of value, otherwise it is an associative array from key to value,
 <tag:<PU>entry>.

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
#if defined(TREE_KEY_IN_VALUE) && !defined(TREE_VALUE)
#error TREE_KEY_IN_VALUE needs TREE_VALUE.
#endif
#if defined(TREE_TEST) && !defined(TREE_TO_STRING)
#error TREE_TEST requires TREE_TO_STRING.
#endif
#if defined(TREE_TO_STRING_NAME) && !defined(TREE_TO_STRING)
#error TREE_TO_STRING_NAME requires TREE_TO_STRING.
#endif

#ifndef TREE_H /* <!-- idempotent */
#define TREE_H
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <limits.h>
/* <Kernighan and Ritchie, 1988, p. 231>. */
#if defined(TREE_CAT_) || defined(TREE_CAT) || defined(U_) || defined(PU_) \
	|| defined(TREE_IDLE)
#error Unexpected defines.
#endif
#define TREE_CAT_(n, m) n ## _ ## m
#define TREE_CAT(n, m) TREE_CAT_(n, m)
#define U_(n) TREE_CAT(TREE_NAME, n)
#define PU_(n) TREE_CAT(tree, U_(n))
#define TREE_KEYS 2
#if TREE_KEYS < 1 || TREE_KEYS > UCHAR_MAX
#error TREE_KEYS parameter range `[1, UCHAR_MAX]`.
#endif
#define TREE_ORDER (TREE_KEYS + 1) /* Maximum branching factor/leaves. */
#define TREE_RESULT X(ERROR), X(UNIQUE), X(YIELD), X(REPLACE)
#define X(n) TREE_##n
/** A result of modifying the table, of which `TREE_ERROR` is false.
 ![A diagram of the result states.](../doc/put.png) */
enum tree_result { TREE_RESULT };
#undef X
#define X(n) #n
/** A static array of strings describing the <tag:tree_result>. */
static const char *const tree_result_str[] = { TREE_RESULT };
#undef X
#undef TREE_RESULT
#define TREE_IDLE { 0, 0 }
#endif /* idempotent --> */

#ifndef TREE_KEY
#define TREE_KEY unsigned
#endif
/** An unsigned type used as the key, defaults to `unsigned int`. */
typedef TREE_KEY PU_(key);

#ifdef TREE_VALUE
/** On `TREE_VALUE`, otherwise just a set of numbers. */
typedef TREE_VALUE PU_(value);
#endif

/* In <Knuth, 1998 Art 3> terminology, external B-tree node of `TREE_ORDER`,
 `TREE_KEYS + 1`. */
struct PU_(outer) {
	unsigned char keys;
	PU_(key) key[TREE_KEYS];
#ifdef TREE_VALUE
	PU_(value) value[TREE_KEYS];
#endif
};
/* B-tree internal node of `TRIE_ORDER` inherits from external. */
struct PU_(inner) { struct PU_(outer) base, *link[TREE_ORDER]; };
/** @return Upcasts `outer` to an inner tree. */
static struct PU_(inner) *PU_(inner)(struct PU_(outer) *const outer)
	{ return (struct PU_(inner) *)(void *)
	((char *)outer - offsetof(struct PU_(inner), base)); }
/** @return Upcasts `outer` to an inner tree. */
static const struct PU_(inner) *PU_(inner_c)(const struct PU_(outer) *
	const outer) { return (const struct PU_(inner) *)(const void *)
	((const char *)outer - offsetof(struct PU_(inner), base)); }

#if defined(TREE_VALUE) && !defined(TREE_KEY_IN_VALUE) /* <!-- entry */
/** On `TREE_VALUE` but not `TREE_KEY_IN_VALUE`, creates a map from key to
 value as an associative array. */
struct U_(tree_entry) { PU_(key) key; PU_(value) value; }
typedef struct U_(tree_entry) PU_(entry);
#else /* entry --><!-- !entry */
typedef PU_(key) PU_(value);
/** On `TREE_VALUE` and not `TREE_KEY_IN_VALUE`, otherwise it's just an alias
 for <typedef:<PU>value>. */
typedef PU_(value) PU_(entry);
#endif /* !entry --> */

/** If `TREE_KEY_IN_VALUE` is set, responsible for picking out the
 null-terminated string from the <typedef:<PU>value>, (in which case, the same
 as <typedef:<PU>entry>.) */
typedef PU_(key) (*PU_(key_fn))(PU_(entry));

#ifdef TREE_KEY_IN_VALUE
static PU_(key_fn) PU_(to_key) = (TREE_KEY_IN_VALUE);
#elif defined(TREE_VALUE)
static const char *PU_(entry_key)(const struct PU_(entry) entry)
	{ return entry.key; }
static PU_(key_fn) PU_(to_key) = &PU_(entry_key);
#else
static PU_(key) PU_(id_key)(const PU_(key) key) { return key; }
static PU_(key_fn) PU_(to_key) = &PU_(id_key);
#endif

/** To initialize it to an idle state, see <fn:<U>tree>, `TRIE_IDLE`, `{0}`
 (`C99`), or being `static`.

 ![States.](../doc/states.png) */
struct U_(tree);
struct U_(tree) { struct PU_(outer) *root; unsigned height; };

struct PU_(iterator) {
	const struct U_(trie) *tree;
	struct PU_(outer) *current, *end;
	unsigned leaf, leaf_end;
};

/** <Bottenbruch, 1962 ALGOL>.
 @return Finds `key` in `tree`, or null. @order \O(\log |`tree`|) */
static PU_(entry) *PU_(get)(const struct U_(tree) *const tree,
	const PU_(key) key) {
	struct { unsigned h, a0; } a;
	const struct PU_(outer) *trunk;
	assert(tree && key);
	if(!(a.h = tree->height)) return 0;
	for(trunk = tree->root, assert(trunk); ;
		trunk = PU_(inner_c)(trunk)->link[a.a0]) {
		unsigned a1 = trunk->keys;
		a.h--, a.a0 = 0;
		while(a.a0 < a1) {
			const unsigned m = (a.a0 + a1) / 2;
			const PU_(key) k = trunk->key[m];
			if(k < key) a.a0 = m + 1;
			else a1 = m;
		}
		if(!d.h) break;
	}
	return PT_(outer)(d.trunk)->leaf + d.lf;
}

/** @return Exact match for `key` in `trie` or null. */
static PT_(entry) PT_(get)(const struct T_(trie) *const trie,
	const char *const key) {
	printf("get \"%s\"\n", key);
	PT_(entry) *const x = PT_(match)(trie, key);
	/*printf("get \"%s\" -> \n", key);
	printf("\"%s\"\n", x ? PT_(to_key)(x) : "(null)");*/
	printf("get \"%s\" -> \"%s\"\n", key, x ? PT_(to_key)(*x) : "(null)");
	return x && !strcmp(PT_(to_key)(*x), key) ? *x : 0;
}

/** Looks at only the index of `trie` (which can be null) for potential
 `prefix` matches, and stores them in `it`. @order \O(|`prefix`|) */
static void PT_(match_prefix)(const struct T_(trie) *const trie,
	const char *const prefix, struct PT_(iterator) *it) {
	struct trie_trunk_descend d;
	struct { size_t cur, next; } byte;
	assert(prefix && it);
	it->trie = 0;
	if(!trie || !(d.h = trie->node_height)) return;
	for(d.trunk = trie->root, assert(d.trunk), d.diff = 0, byte.cur = 0; ;
		d.trunk = trie_inner(d.trunk)->leaf[d.lf].link) {
		assert(d.trunk->skip < d.h), d.h -= 1 + d.trunk->skip;
		d.br0 = 0, d.br1 = d.trunk->bsize, d.lf = 0;
		while(d.br0 < d.br1) {
			const struct trie_branch *const branch = d.trunk->branch + d.br0;
			/* _Sic_; '\0' is _not_ included for partial match. */
			for(byte.next = (d.diff += branch->skip) / CHAR_BIT;
				byte.cur <= byte.next; byte.cur++)
				if(prefix[byte.cur] == '\0') goto finally;
			if(!TRIE_QUERY(prefix, d.diff))
				d.br1 = ++d.br0 + branch->left;
			else
				d.br0 += branch->left + 1,
				d.lf += branch->left + 1;
			d.diff++;
		}
		if(!d.h) break;
	}
finally:
	assert(d.br0 <= d.br1 && d.lf - d.br0 + d.br1 <= d.trunk->bsize);
	it->end = d.trunk;
	it->leaf_end = d.lf + d.br1 - d.br0 + 1;
	while(d.h) d.trunk = trie_inner_c(d.trunk)->leaf[d.lf].link, d.lf = 0,
		assert(d.trunk->skip < d.h), d.h -= 1 + d.trunk->skip;
	it->current = PT_(outer)(d.trunk);
	it->leaf = d.lf;
	it->trie = trie;
}

/** Stores all `prefix` matches in `trie` in `it`. @order \O(|`prefix`|) */
static void PT_(prefix)(struct T_(trie) *const trie,
	const char *const prefix, struct PT_(iterator) *it) {
	assert(trie && prefix && it);
	PT_(match_prefix)(trie, prefix, it);
	/* Make sure actually a prefix. */
	if(it->trie && !trie_is_prefix(prefix,
		PT_(to_key)(it->current->leaf[it->leaf]))) it->current = 0;
}

#if 0 /* <!-- forward declare debugging tools */

#ifdef TRIE_TO_STRING
static const char *T_(trie_to_string)(const struct T_(trie) *);
#endif
/** Returns a string of `trie`. */
static const char *PT_(str)(const struct T_(trie) *const trie) {
#ifdef TRIE_TO_STRING
	return T_(trie_to_string)(trie);
#else
	return "[not to string]"
#endif
}
#ifdef TRIE_TO_STRING
#endif
/** Returns a string of `trie`. */
static const char *PT_(str)(const struct T_(trie) *const trie) {
#ifdef TRIE_TO_STRING
	return T_(trie_to_string)(trie);
#else
	return "[not to string]"
#endif
}

#ifdef TRIE_TEST
static void PT_(graph)(const struct T_(trie) *, const char *);
static void PT_(print)(const struct PT_(tree) *);
#endif
/** Graphs `trie` in `fn`. */
static void PT_(grph)(const struct T_(trie) *const trie, const char *const fn) {
	assert(trie && fn);
#ifdef TRIE_TEST
	PT_(graph)(trie, fn);
#endif
}
/** Prints `tree`. */
static void PT_(prnt)(const struct PT_(tree) *const tree) {
	assert(tree);
#ifdef TRIE_TEST
	PT_(print)(tree);
#endif
}

#endif /* forward --> */

#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)

/** @return The leftmost key `lf` of `trunk` of edge `height`. */
static const char *PT_(sample)(const struct trie_trunk *trunk,
	size_t height, unsigned lf) {
	assert(trunk);
	while(height) trunk = trie_inner_c(trunk)->leaf[lf].link, lf = 0,
		height -= 1 + trunk->skip;
	return PT_(to_key)(PT_(outer_c)(trunk)->leaf[lf]);
}

#if 0
/** Given a `trie`, calculate the bit at the start of `trunk`.
 @order \O(\log `trie.size`) */
static size_t PT_(trunk_diff)(const struct T_(trie) *trie,
	const struct trie_trunk *trunk, const size_t height) {
	struct trie_trunk_descend d;
	const char *sample = PT_(sample)(trunk, height, 0);
	assert(trie && sample);
	d.h = trie->node_height, assert(d.h);
	for(d.trunk = trie->root, assert(d.trunk), d.diff = 0; ;
		d.trunk = trie_inner(d.trunk)->link[d.lf]) {
		assert(d.trunk->skip < d.h), d.h -= 1 + trunk->skip;
		if(trunk == d.trunk) break;
		d.br0 = 0, d.br1 = trunk->bsize, d.lf = 0;
		while(d.br0 < d.br1) {
			const struct trie_branch *const branch = d.trunk->branch + d.br0;
			if(!TRIE_QUERY(sample, d.diff))
				d.br1 = ++d.br0 + branch->left;
			else
				d.br0 += branch->left + 1,
				d.lf += branch->left + 1;
			d.diff++;
		}
		assert(d.h); if(!d.h) return 0; /* Corrupted? */
	}
	return d.diff;
}
#endif

/** Right side of `left`, which must be full, moves to `right`, (which is
 clobbered.) The root of `left` is also clobbered. */
static void PT_(split)(struct PT_(outer_tree) *const left,
	struct PT_(outer_tree) *const right, enum trie_tree_type type) {
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
static union PT_(leaf_ptr) PT_(tree_open)(enum trie_tree_type type,
	const char *const key, const size_t diff, struct trie_trunk *const trunk,
	size_t bit0, union PT_(leaf_ptr) spot_for_tree_root) {
	struct { unsigned br0, br1, lf; } t;
	struct trie_branch *branch;
	size_t tr1;
	unsigned is_right;
	union PT_(leaf_ptr) ret;
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
		PT_(entry) *const leaf = PT_(outer)(trunk)->leaf + t.lf;
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


/** Adds `x` to `trie`, which must not be present. @return Success.
 @throw[malloc, ERANGE]
 @throw[EILSEQ] There are too many bits similar for it to placed in the trie at
 the moment. */
static int PT_(add_unique)(struct T_(trie) *const trie, PT_(entry) x) {
	const char *const key = PT_(to_key)(x);
	struct trie_trunk_descend d;
	size_t trunk_diff;
	struct { /* Last inner trie that is not full. */
		struct { struct trie_trunk *trunk; size_t height, diff; } unfull;
		size_t full; /* Count after the last. */
	} history;
	const char *sample;
	int restarts = 0; /* Debug: make sure we only go through twice. */
	struct trie_inner_tree *inner = 0;
	struct PT_(outer_tree) *outer = 0;
	struct trie_inner_tree *new_root = 0;
	assert(trie && x && key);

	printf("unique: adding \"%s\".\n", key);
start:
	if(!(d.h = trie->node_height)) { /* Solitary. */
		if(trie->root) outer = PT_(outer)(trie->root);
		else if(outer = malloc(sizeof *outer)) trie->root = &outer->trunk;
		else goto catch;
		outer->trunk.bsize = 0, outer->trunk.skip = 0, outer->leaf[0] = x;
		trie->node_height = 1;
		printf("add: new outer %s-tree that holds \"%s\"=>\"%s\".\n",
			orcify(outer), PT_(to_key)(x), PT_(to_key)(outer->leaf[0]));
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
		sample = PT_(sample)(d.trunk, d.h, 0);
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
				sample = PT_(sample)(d.trunk, d.h, d.lf);
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
		union PT_(leaf_ptr) dumb;
		dumb.outer = &x;
		union PT_(leaf_ptr) ptr = PT_(tree_open)(TRIE_OUTER, key, d.diff,
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
			union PT_(leaf_ptr) ptr;
			ptr.inner = &dumb;
			/********* I want a state machine ***************/
			/* machine parameters: key, trunk, diff, height */
			PT_(tree_open)(TRIE_INNER, key, 0, history.unfull.trunk,
				history.unfull.diff, ptr);
			assert(0);
		}
		assert(0);
#if 0
		struct PT_(tree) *up, *left = 0, *right = 0;
		unsigned char leaves_split;
		struct trie_branch *branch;
		union PT_(leaf) *leaf;
		size_t with_promote_bit;
		/* Allocate one or two if the root-tree is being split. This is a
		 sequence point in splitting where the trie is valid. */
		if(!(up = full.a.tr) && !(up = PT_(tree)()) || !(right = PT_(tree)()))
			{ free(right); if(!full.a.tr) free(up);
			if(!errno) errno = ERANGE; return 0; }
		if(full.a.tr) { /* Expand the parent to hold the promoted root. */
			assert(up == full.a.tr && up->bsize < TRIE_BRANCHES);
			t.br0 = 0, t.br1 = up->bsize, t.lf = 0;
			while(t.br0 < t.br1) { /* Tree. */
				branch = up->branch + t.br0;
				full.a.bit += branch->skip, assert(full.a.bit < diff);
				if(!TRIE_QUERY(key, full.a.bit))
					t.br1 = ++t.br0 + branch->left++;
				else
					t.br0 += branch->left + 1, t.lf += branch->left + 1;
				full.a.bit++;
			}
			/* Expand the tree to include one more leaf and branch. */
			left = (leaf = up->leaf + t.lf)->child,
				assert(t.lf <= up->bsize + 1
				&& trie_bmp_test(&up->is_child, t.lf));
			memmove(leaf + 1, leaf, sizeof *leaf * ((up->bsize + 1) - t.lf));
			branch = up->branch + t.br0;
			trie_bmp_insert(&up->is_child, t.lf, 1);
			memmove(branch + 1, branch, sizeof *branch * (up->bsize - t.br0));
			up->bsize++; /* Might be full, now. */
		} else { /* Raise depth of forest for the promoted branch. */
			assert(!full.a.bit);
			left = trie->root;
			trie->root = up;
			t.br0 = 0, t.br1 = up->bsize = 1, t.lf = 0;
			trie_bmp_set(&up->is_child, 1);
		}
		/* Promote the root of left to the parent's unfilled. */
		assert(left && left->bsize);
		branch = up->branch + t.br0;
		branch->left = 0;
		branch->skip = left->branch[0].skip;
		leaf = up->leaf + t.lf;
		leaf->child = left, trie_bmp_set(&up->is_child, t.lf);
		(leaf + 1)->child = right,
			assert(trie_bmp_test(&up->is_child, t.lf + 1));
		/* Advance the cursor to the next tree. */
		leaves_split = left->branch[0].left + 1;
		if((with_promote_bit = full.a.bit + branch->skip) <= diff) {
			assert(with_promote_bit < diff);
			full.a.bit = with_promote_bit;
			full.a.tr = !(TRIE_QUERY(key, full.a.bit)) ? left : right;
			full.a.bit++;
		} else {
			assert(full.n == 1);
			full.a.tr = up;
		}
		/* Copy the right part of the left to the new right. */
		right->bsize = left->bsize - leaves_split;
		memcpy(right->branch, left->branch + leaves_split,
			sizeof *left->branch * right->bsize);
		memcpy(right->leaf, left->leaf + leaves_split,
			sizeof *left->leaf * (right->bsize + 1));
		memcpy(&right->is_child, &left->is_child, sizeof left->is_child);
		trie_bmp_remove(&right->is_child, 0, leaves_split);
		/* Move back the branches of the left to account for the promotion. */
		left->bsize = leaves_split - 1;
		memmove(left->branch, left->branch + 1,
			sizeof *left->branch * (left->bsize + 1));
#endif
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

/** A bi-predicate; returns true if the `replace` replaces the `original`; used
 in <fn:<T>trie_policy_put>. */
typedef int (*PT_(replace_fn))(PT_(entry) *original, PT_(entry) *replace);

/** Adds `x` to `trie` and, if `eject` is non-null, stores the collided
 element, if any, as long as `replace` is null or returns true.
 @param[eject] If not-null, the ejected datum. If `replace` returns false, then
 `*eject == datum`, but it will still return true.
 @return Success. @throws[realloc, ERANGE] */
static int PT_(put)(struct T_(trie) *const trie, PT_(entry) x,
	PT_(entry) **const eject, const PT_(replace_fn) replace) {
	const char *key;
	PT_(entry) *leaf;
	assert(trie && x);
	key = PT_(to_key)(x);
	/* Add if absent. */
	assert(0);
	if(!(leaf = PT_(get)(trie, key)))
		{ if(eject) *eject = 0; return PT_(add_unique)(trie, x); }
	/* Collision policy. */
	if(replace && !replace(leaf, &x)) {
		if(eject) *eject = &x;
	} else {
		if(eject) *eject = leaf;
		*leaf = x;
	}
	return 1;
}

/** Try to remove `key` from `trie`.
 @fixme Join when combined-half is less than half. */
static int PT_(remove)(struct T_(trie) *const trie,
	const char *const key) {
	struct trie_trunk *trunk;
	size_t h, diff;
	struct { unsigned br0, br1, lf; } t, u, v;
	unsigned parent_br = 0; /* Useless initialization. */
	struct { size_t cur, next; } byte; /* `key` null checks. */
	PT_(entry) *rm;
	assert(trie && key);

	/* Same as match, but keep track of the branch not taken in `u`. */
	printf("remove: <<%s>> from %s-trie.\n", key, orcify(trie));
	if(!(h = trie->node_height)) return printf("remove: empty\n"), 0;
	for(trunk = trie->root, assert(trunk), byte.cur = 0, diff = 0; ;
		trunk = trie_inner(trunk)->leaf[t.lf].link) {
		assert(trunk->skip < h), h -= 1 + trunk->skip;
		t.br0 = 0, t.br1 = trunk->bsize, t.lf = 0;
		while(t.br0 < t.br1) {
			const struct trie_branch *const branch
				= trunk->branch + (parent_br = t.br0);
			for(byte.next = (diff += branch->skip) / CHAR_BIT;
				byte.cur < byte.next; byte.cur++)
				if(key[byte.cur] == '\0') return printf("remove: unfound\n"), 0;
			if(!TRIE_QUERY(key, diff))
				u.lf = t.lf + branch->left + 1,
				u.br1 = t.br1,
				u.br0 = t.br1 = ++t.br0 + branch->left;
			else
				u.br0 = ++t.br0,
				u.br1 = (t.br0 += branch->left),
				u.lf = t.lf, t.lf += branch->left + 1;
			/*printf("me: [%u,%u;%u], twin: [%u,%u;%u]\n",
				t.br0, t.br1, t.lf, u.br0, u.br1, u.lf);*/
			diff++;
		}
		if(!h) break;
	}
	rm = PT_(outer)(trunk)->leaf + t.lf;
	if(strcmp(key, PT_(to_key)(*rm))) return printf("remove: doesn't match <<%s>>\n", PT_(to_key)(*rm)), 0;

	/* If a branch, branch not taken's skip merges with the parent. */
	if(u.br0 < u.br1) {
		struct trie_branch *const parent = trunk->branch + parent_br,
			*const diverge = trunk->branch + u.br0;
		printf("remove: skip, parent %u, diverge %u.\n",
			parent->skip, diverge->skip);
		/* Would cause overflow. */
		if(parent->skip == UCHAR_MAX
			|| diverge->skip > UCHAR_MAX - parent->skip - 1)
			return printf("remove: no!\n"), errno = EILSEQ, 0;
		diverge->skip = parent->skip + 1 + diverge->skip;
	}

	/* Update `left` values for the path to the deleted branch. */
	v.br0 = 0, v.br1 = trunk->bsize, v.lf = t.lf;
	if(!v.br1) goto erased_tree;
	for( ; ; ) {
		struct trie_branch *const branch = trunk->branch + v.br0;
		if(branch->left >= v.lf) {
			if(!branch->left) break;
			v.br1 = ++v.br0 + branch->left;
			branch->left--;
		} else {
			if((v.br0 += branch->left + 1) >= v.br1) break;
			v.lf -= branch->left + 1;
		}
	}

	/* Remove the actual memory. */
	memmove(trunk->branch + parent_br, trunk->branch
		+ parent_br + 1, sizeof *trunk->branch
		* (trunk->bsize - parent_br - 1));
	memmove(rm, rm + 1, sizeof *rm * (trunk->bsize - t.lf));
	trunk->bsize--;
	printf("remove: success.\n");
	return 1;

erased_tree:
	/* Maybe previous tree would be good? Set in match, unless this is
	 recursive? Can it be? */
	assert(0);
	return 0;
}

#undef QUOTE
#undef QUOTE_

/** Counts the new iterator `it`. @order \O(|`it`|) */
static size_t PT_(size_r)(const struct PT_(iterator) *const it) {
	size_t size;
	unsigned i;
	size_t height = it->trie->node_height; /* No. */
	struct trie_trunk *trunk = it->trie->root;
	assert(it && height);
	/*if(!it->root || !(next = it->next)) return 0;
	assert(next == it->end
		&& it->leaf <= it->leaf_end && it->leaf_end <= next->bsize + 1);
	size = it->leaf_end - it->leaf;
	for(i = it->leaf; i < it->leaf_end; i++)
		size += PT_(sub_size)(next->leaf[i].child) - 1;*/
	assert(0);
	assert(trunk && trunk->skip < height);
	if(height -= 1 + trunk->skip) {
		const struct trie_inner_tree *const inner = trie_inner_c(trunk);
		for(size = 0, i = 0; i <= trunk->bsize; i++)
			size += 1/*PT_(size_r)(inner->link[i], height)*/;
	} else {
		size = trunk->bsize + 1;
	}
	return size;
}

/* <!-- iterate interface */

/** Loads the first element of `trie` (can be null) into `it`.
 @implements begin */
static void PT_(begin)(struct PT_(iterator) *const it,
	const struct T_(trie) *const trie) {
	PT_(match_prefix)(trie, "", it);
	it->end = 0; /* More robust to concurrent modifications. */
}

/** Advances `it`. @return The previous value or null. @implements next */
const static PT_(entry) *PT_(next)(struct PT_(iterator) *const it) {
	assert(it);
	printf("_next_\n");
	if(!it->trie) return 0;
	assert(it->current && it->end);
	if(&it->current->trunk == it->end) {
		/* Only dealing with one tree. */
		if(it->leaf > it->leaf_end || it->leaf > it->current->trunk.bsize)
			{ it->trie = 0; return 0; }
	} else if(it->leaf > it->current->trunk.bsize) {
		/* Off the end of the tree; keep track of the next branch when doing a
		 look-up of the last entry when the end is past. */
		const char *key
			= PT_(to_key)(it->current->leaf[it->current->trunk.bsize]);
		const struct trie_trunk *trunk1 = &it->current->trunk;
		struct trie_trunk *trunk2, *next = 0;
		size_t h2 = it->trie->node_height, bit2;
		struct { unsigned br0, br1, lf; } t2;
		int is_past_end = !it->end; /* Or else go through the entire trie. */
		assert(key);
		printf("next: %s is the last one on the tree.\n", key);
		for(it->current = 0, trunk2 = it->trie->root, assert(trunk2), bit2 = 0;
			; trunk2 = trie_inner(trunk2)->leaf[t2.lf].link) {
			int is_considering = 0;
			if(trunk2 == trunk1) break; /* Reached the tree. */
			assert(trunk2->skip < h2), h2 -= 1 + trunk2->skip;
			if(!h2) { printf("next: bailing.\n"); break; } /* Concurrent modification? */
			t2.br0 = 0, t2.br1 = trunk2->bsize, t2.lf = 0;
			while(t2.br0 < t2.br1) {
				const struct trie_branch *const branch2
					= trunk2->branch + t2.br0;
				bit2 += branch2->skip;
				if(!TRIE_QUERY(key, bit2))
					t2.br1 = ++t2.br0 + branch2->left;
				else
					t2.br0 += branch2->left + 1,
					t2.lf += branch2->left + 1;
				bit2++;
			}
			/* Past the end? */
			if(is_past_end) {
				is_considering = 1;
			} else if(trunk2 == it->end) {
				is_past_end = 1;
				if(t2.lf < it->leaf_end) is_considering = 1;
			}
			/* Set it to the next value. */
			if(is_considering && t2.lf < trunk2->bsize)
				next = trunk2, it->leaf = t2.lf + 1,
				printf("next: continues in tree %s, leaf %u.\n",
				orcify(trunk2), it->leaf);
		}
		if(!next) { printf("next: fin\n"); it->trie = 0; return 0; } /* No more. */
		while(h2) trunk2 = trie_inner_c(trunk2)->leaf[it->leaf].link,
			it->leaf = 0, assert(trunk2->skip < h2), h2 -= 1 + trunk2->skip;
		it->current = PT_(outer)(trunk2);
	}
	return it->current->leaf + it->leaf++;
}

/* iterate --> */

/** Frees `tr` at `h` and it's children recursively. Stores any one outer tree
 in `one`. `height` is the node height, (height plus one.) */
static void PT_(clear_r)(struct trie_trunk *const tr, size_t height,
	struct PT_(outer_tree) **const one) {
	unsigned i;
	assert(tr && height > tr->skip && one);
	if(height -= 1 + tr->skip) {
		for(i = 0; i <= tr->bsize; i++)
			PT_(clear_r)(trie_inner(tr)->leaf[i].link, height, one);
		free(trie_inner(tr));
	} else if(!*one) {
		*one = PT_(outer)(tr);
	} else {
		free(PT_(outer)(tr));
	}
}

/** Stores an iteration range in a trie. Any changes in the topology of the
 trie invalidate it. */
struct T_(trie_iterator) { struct PT_(iterator) i; };

/** Initializes `trie` to idle. @order \Theta(1) @allow */
static void T_(trie)(struct T_(trie) *const trie)
	{ assert(trie); trie->root = 0; }

/** Returns an initialized `trie` to idle. @allow */
static void T_(trie_)(struct T_(trie) *const trie) {
	struct PT_(outer_tree) *clear_all = (struct PT_(outer_tree) *)1;
	assert(trie);
	PT_(clear_r)(trie->root, trie->node_height, &clear_all);
	T_(trie)(trie);
}

/** Clears every entry in a valid `trie`, but it continues to be active if it
 is not idle. */
static void T_(trie_clear)(struct T_(trie) *const trie) {
	struct PT_(outer_tree) *will_be_root = 0;
	assert(trie);
	PT_(clear_r)(trie->root, trie->node_height, &will_be_root);
	T_(trie)(trie);
	trie->root = &will_be_root->trunk; /* Hysteresis. */
}

#if 0
/** Initializes `trie` from an `array` of pointers-to-`<T>` of `array_size`.
 @return Success. @throws[realloc] @order \O(`array_size`) @allow
 @fixme Write this function, somehow. */
static int T_(trie_from_array)(struct T_(trie) *const trie,
	PT_(type) *const*const array, const size_t array_size) {
	return assert(trie && array && array_size),
		PT_(init)(trie, array, array_size);
}
#endif

/** @return Whether `key` is in `trie`; in case either one is null, returns
 false. @order \O(\log `trie.size`) or \O(|`key`|) */
static int T_(trie_is)(const struct T_(trie) *const trie,
	const char *const key) { return !(!trie || !key || !PT_(get)(trie, key)); }

/** @return Looks at only the index of `trie` for potential `key` matches,
 but will ignore the values of the bits that are not in the index.
 @order \O(|`key`|) @allow */
static PT_(entry) *T_(trie_match)(const struct T_(trie) *const trie,
	const char *const key) { return PT_(match)(trie, key); }

/** @return Exact match for `key` in `trie` or null no such item exists.
 @order \O(|`key`|), <Thareja 2011, Data>. @allow */
static PT_(entry) T_(trie_get)(const struct T_(trie) *const trie,
	const char *const key) { return PT_(get)(trie, key); }

/** Adds a pointer to `x` into `trie` if the key doesn't exist already.
 @return If the key did not exist and it was created, returns true. If the key
 of `x` is already in `trie`, or an error occurred, returns false.
 @throws[realloc, ERANGE] Set `errno = 0` before to tell if the operation
 failed due to error. @order \O(|`key`|) @allow */
static enum trie_result T_(trie_try)(struct T_(trie) *const trie,
	PT_(entry) entry) {
	if(!trie || !entry) return printf("add: null\n"), TRIE_ERROR;
	printf("add: trie %s; entry <<%s>>.\n", orcify(trie), PT_(to_key)(entry));
	return PT_(get)(trie, PT_(to_key)(entry)) ? TRIE_YIELD :
		(PT_(add_unique)(trie, entry), TRIE_UNIQUE); }

/** Updates or adds a pointer to `x` into `trie`.
 @param[eject] If not null, on success it will hold the overwritten value or
 a pointer-to-null if it did not overwrite any value.
 @return Success. @throws[realloc, ERANGE] @order \O(|`key`|) @allow */
static int T_(trie_put)(struct T_(trie) *const trie, const PT_(entry) x,
	PT_(entry) */*const fixme*/eject)
	{ return assert(trie && x), PT_(put)(trie, x, &eject, 0); }

/** Adds a pointer to `x` to `trie` only if the entry is absent or if calling
 `replace` returns true or is null.
 @param[eject] If not null, on success it will hold the overwritten value or
 a pointer-to-null if it did not overwrite any value. If a collision occurs and
 `replace` does not return true, this will be a pointer to `x`.
 @param[replace] Called on collision and only replaces it if the function
 returns true. If null, it is semantically equivalent to <fn:<T>trie_put>.
 @return Success. @throws[realloc, ERANGE] @order \O(|`key`|) @allow */
static int T_(trie_policy)(struct T_(trie) *const trie, const PT_(entry) x,
	PT_(entry) */*const*/ eject, const PT_(replace_fn) replace)
	{ return assert(trie && x), PT_(put)(trie, x, &eject, replace); }

/** Tries to remove `key` from `trie`. @return Success. */
static int T_(trie_remove)(struct T_(trie) *const trie,
	const char *const key) { return PT_(remove)(trie, key); }

/** Fills `it` with iteration parameters that find values of keys that start
 with `prefix` in `trie`.
 @param[prefix] To fill `it` with the entire `trie`, use the empty string.
 @param[it] A pointer to an iterator that gets filled. It is valid until a
 topological change to `trie`. Calling <fn:<T>trie_next> will iterate them in
 order. @order \O(\log `trie.size`) or \O(|`prefix`|) @allow */
static void T_(trie_prefix)(struct T_(trie) *const trie,
	const char *const prefix, struct T_(trie_iterator) *const it)
	{ assert(it); PT_(prefix)(trie, prefix, &it->i); }

/** Advances `it`. @return The previous value or null. @allow */
static const PT_(entry) *T_(trie_next)(struct T_(trie_iterator) *const it)
	{ return PT_(next)(&it->i); }

/** Counts the of the items in initialized `it`. @order \O(|`it`|) @allow */
static size_t T_(trie_size)(const struct T_(trie_iterator) *const it)
	{ return assert(it), PT_(size_r)(&it->i); }

/* <!-- box: Define these for traits. */
#define BOX_ PT_
#define BOX_CONTAINER struct T_(trie)
#define BOX_CONTENTS PT_(entry)

#ifdef TRIE_TO_STRING /* <!-- str */
/** Uses the natural `a` -> `z` that is defined by `TRIE_KEY_IN_VALUE`. */
static void PT_(to_string)(const PT_(entry) *a, char (*const z)[12])
	{ assert(a && *a && z); sprintf(*z, "%.11s", PT_(to_key)(*a)); }
#define SZ_(n) TRIE_CAT(T_(trie), n)
#define TO_STRING &PT_(to_string)
#define TO_STRING_LEFT '{'
#define TO_STRING_RIGHT '}'
#include "to_string.h" /** \include */
#undef SZ_
#undef TRIE_TO_STRING
#endif /* str --> */

#ifdef TRIE_TEST /* <!-- test */
#include "../test/test_trie.h" /** \include */
#endif /* test --> */

#undef BOX_
#undef BOX_CONTAINER
#undef BOX_CONTENTS
/* box --> */

static void PT_(unused_base_coda)(void);
static void PT_(unused_base)(void) {
	PT_(begin)(0, 0);
	T_(trie)(0); T_(trie_)(0); T_(trie_clear)(0);
	T_(trie_is)(0, 0); T_(trie_match)(0, 0); T_(trie_get)(0, 0);
	T_(trie_try)(0, 0); T_(trie_put)(0, 0, 0); T_(trie_policy)(0, 0, 0, 0);
	T_(trie_remove)(0, 0);
	T_(trie_prefix)(0, 0, 0); T_(trie_next)(0); T_(trie_size)(0);
	PT_(unused_base_coda)();
}
static void PT_(unused_base_coda)(void) { PT_(unused_base)(); }

#undef TRIE_NAME
#undef TRIE_VALUE
#undef TRIE_KEY_IN_VALUE
#ifdef TRIE_TEST
#undef TRIE_TEST
#endif
#ifdef TRIE_DEFAULT_TEST
#undef TRIE_DEFAULT_TEST
#endif
