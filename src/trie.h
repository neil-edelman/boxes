/** @license 2020 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Prefix Tree

 ![Example of trie.](../web/trie.png)

 A <tag:<T>trie> is a prefix tree, digital tree, or trie, implemented as an
 array of pointers-to-`T` and an index on the key string. It can be seen as a
 <Morrison, 1968 PATRICiA>: a compact
 [binary radix trie](https://en.wikipedia.org/wiki/Radix_tree), only
 storing the where the key bits are different. Strings can be any encoding with
 a byte null-terminator, including
 [modified UTF-8](https://en.wikipedia.org/wiki/UTF-8#Modified_UTF-8).
 Practically, this is a set or map of strings, in order, with performance
 comparable to that of a B-tree, allowing fast prefix matches.

 @param[TRIE_NAME, TRIE_TYPE]
 <typedef:<PT>type> that satisfies `C` naming conventions when mangled and an
 optional returnable type that is declared, (it is used by reference only
 except if `TRIE_TEST`.) `<PT>` is private, whose names are prefixed in a
 manner to avoid collisions.

 @param[TRIE_KEY]
 A function that satisfies <typedef:<PT>key_fn>. Must be defined if and only if
 `TRIE_TYPE` is defined. (This imbues it with the properties of a string
 associative array.)

 @param[TRIE_TO_STRING]
 Defining this includes <to_string.h>, with the keys as the string.

 @param[TRIE_TEST]
 Unit testing framework <fn:<T>trie_test>, included in a separate header,
 <../test/test_trie.h>. Must be defined equal to a (random) filler function,
 satisfying <typedef:<PT>action_fn>. Requires `TRIE_TO_STRING` and that
 `NDEBUG` not be defined.

 @depend [bmp](https://github.com/neil-edelman/bmp)
 @std C89 */

#ifndef TRIE_NAME
#error Name TRIE_NAME undefined.
#endif
#if defined(TRIE_TYPE) ^ defined(TRIE_KEY)
#error TRIE_TYPE and TRIE_KEY have to be mutually defined.
#endif
#if defined(TRIE_TEST) && (!defined(TRIE_TO_STRING) || !defined(TRIE_TYPE))
#error TRIE_TEST requires TRIE_TO_STRING and TRIE_TYPE.
#endif

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <limits.h> /* CHAR_BIT (will compile on C89) */

#ifndef TRIE_H /* <!-- idempotent */
#define TRIE_H
/* <Kernighan and Ritchie, 1988, p. 231>. */
#if defined(TRIE_CAT_) || defined(TRIE_CAT) || defined(T_) || defined(PT_)
#error Unexpected defines.
#endif
#define TRIE_CAT_(n, m) n ## _ ## m
#define TRIE_CAT(n, m) TRIE_CAT_(n, m)
#define T_(n) TRIE_CAT(TRIE_NAME, n)
#define PT_(n) TRIE_CAT(trie, T_(n))
/* <http://c-faq.com/misc/bitsets.html>, except reversed for msb-first. */
#define TRIE_MASK(n) ((1 << CHAR_BIT - 1) >> (n) % CHAR_BIT)
#define TRIE_SLOT(n) ((n) / CHAR_BIT)
#define TRIE_QUERY(a, n) ((a)[TRIE_SLOT(n)] & TRIE_MASK(n))
#define TRIE_DIFF(a, b, n) \
	(((a)[TRIE_SLOT(n)] ^ (b)[TRIE_SLOT(n)]) & TRIE_MASK(n))
/* Worst-case all-branches-left root. Parameter sets the maximum tree size.
 Prefer alignment `4n - 2`; cache `32n - 2`. (Easily, `{a, b, ..., A}`). */
#define TRIE_MAX_LEFT 1/*6*//*254*/
#if TRIE_MAX_LEFT </*=?*/ 0 || TRIE_MAX_LEFT > UCHAR_MAX
#error TRIE_MAX_LEFT parameter range `[0, UCHAR_MAX]` without modifications.
#endif
#define TRIE_BRANCHES (TRIE_MAX_LEFT + 1) /* Maximum branches. */
#define TRIE_ORDER (TRIE_BRANCHES + 1) /* Maximum branching factor/leaves. */
struct trie_branch { unsigned char left, skip; };
/* Dependency on `bmp.h`. */
#define BMP_NAME trie
#define BMP_BITS TRIE_ORDER
#include "bmp.h"
/** @return Whether `a` and `b` are equal up to the minimum of their lengths'.
 Used in <fn:<T>trie_prefix>. */
static int trie_is_prefix(const char *a, const char *b) {
	for( ; ; a++, b++) {
		if(*a == '\0') return 1;
		if(*a != *b) return *b == '\0';
	}
}
#endif /* idempotent --> */

#ifndef TRIE_TYPE /* <!-- !type */
/** Default `char` uses `a` as the key, which makes it a set of strings. */
static const char *PT_(raw)(const char *a) { return assert(a), a; }
#define TRIE_TYPE char
#define TRIE_KEY &PT_(raw)
#endif /* !type --> */
/** Declared type of the trie; `char` default. */
typedef TRIE_TYPE PT_(type);

/** A leaf is either data or another tree; the `children` of <tag:<PT>tree> is
 a bitmap that tells which. */
union PT_(leaf) { PT_(type) *data; struct PT_(tree) *child; };

/** A trie is a forest of non-empty complete binary trees. In
 <Knuth, 1998 Art 3> terminology, this structure is similar to a node of
 `TRIE_ORDER` in a B-tree, described in <Bayer, McCreight, 1972 Large>, but
 node already has conflicting meaning. */
struct PT_(tree) {
	unsigned char bsize, skip;
	struct trie_branch branch[TRIE_BRANCHES];
	struct trie_bmp is_child;
	union PT_(leaf) leaf[TRIE_ORDER];
};

/** To initialize it to an idle state, see <fn:<T>trie>, `TRIE_IDLE`, `{0}`
 (`C99`), or being `static`.

 ![States.](../web/states.png) */
struct T_(trie) { struct PT_(tree) *root; };
#ifndef TRIE_IDLE /* <!-- !zero */
#define TRIE_IDLE { 0 }
#endif /* !zero --> */

/* Contains all iteration parameters; satisfies box interface iteration. This
 is a private version of the <tag:<T>trie_iterator> that does all the work, but
 it can only iterate through the entire trie. */
struct PT_(iterator)
	{ struct PT_(tree) *root, *next; unsigned leaf, unused; };

/** Stores a range in the trie. Any changes in the topology of the trie
 invalidate it. @fixme Replacing `root` with `bit` would make it faster and
 allow size remaining; just have to fiddle with `end` to `above`. That makes it
 incompatible with private, but could merge. */
struct T_(trie_iterator);
struct T_(trie_iterator)
	{ struct PT_(tree) *root, *next, *end; unsigned leaf, leaf_end; };

/** Responsible for picking out the null-terminated string. Modifying the
 string key in the original <typedef:<PT>type> while in any trie causes the
 entire trie to go into an undefined state. */
typedef const char *(*PT_(key_fn))(const PT_(type) *);

/* Check that `TRIE_KEY` is a function satisfying <typedef:<PT>key_fn>. */
static PT_(key_fn) PT_(to_key) = (TRIE_KEY);

/** @return The address of a index candidate match for `key` in `trie`, or
 null, if `key` is definitely not in `trie`. @order \O(|`key`|) */
static PT_(type) **PT_(leaf_match)(const struct T_(trie) *const trie,
	const char *const key) {
	struct PT_(tree) *tree;
	size_t bit; /* `bit \in key`.  */
	struct { unsigned br0, br1, lf; } t;
	struct { size_t cur, next; } byte; /* `key` null checks. */
	assert(trie && key);
	if(!(tree = trie->root)) return 0; /* Empty. */
	for(byte.cur = 0, bit = 0; ; ) { /* Forest. */
		t.br0 = 0, t.br1 = tree->bsize, t.lf = 0;
		while(t.br0 < t.br1) { /* Tree. */
			const struct trie_branch *const branch = tree->branch + t.br0;
			for(byte.next = (bit += branch->skip) / CHAR_BIT;
				byte.cur < byte.next; byte.cur++)
				if(key[byte.cur] == '\0') return 0; /* Too short. */
			if(!TRIE_QUERY(key, bit))
				t.br1 = ++t.br0 + branch->left;
			else
				t.br0 += branch->left + 1, t.lf += branch->left + 1;
			bit++;
		}
		if(!trie_bmp_test(&tree->is_child, t.lf)) break;
		tree = tree->leaf[t.lf].child;
	}
	return &tree->leaf[t.lf].data;
}

/** @return An index candidate match for `key` in `trie`. */
static PT_(type) *PT_(match)(const struct T_(trie) *const trie,
	const char *const key)
	{ PT_(type) **const x = PT_(leaf_match)(trie, key); return x ? *x : 0; }

/** @return The address of the exact match for `key` in `trie` or null. */
static PT_(type) **PT_(leaf_get)(const struct T_(trie) *const trie,
	const char *const key) {
	PT_(type) **const x = PT_(leaf_match)(trie, key);
	return x && !strcmp(PT_(to_key)(*x), key) ? x : 0;
}

/** @return Exact match for `key` in `trie` or null. */
static PT_(type) *PT_(get)(const struct T_(trie) *const trie,
	const char *const key) {
	PT_(type) *const x = PT_(match)(trie, key);
	return x && !strcmp(PT_(to_key)(x), key) ? x : 0;
}

/** Looks at only the index of `trie` for potential `prefix` matches,
 and stores them in `it`. @order \O(|`prefix`|) */
static void PT_(match_prefix)(const struct T_(trie) *const trie,
	const char *const prefix, struct T_(trie_iterator) *it) {
	struct PT_(tree) *tree;
	size_t bit; /* `bit \in key`.  */
	struct { unsigned br0, br1, lf; } t;
	struct { size_t cur, next; } byte; /* `key` null checks. */
	assert(prefix && it);
	it->root = it->next = it->end = 0;
	it->leaf = it->leaf_end = 0;
	if(!trie) return;
	for(tree = trie->root, byte.cur = 0, bit = 0; ; ) { /* Forest. */
		t.br0 = 0, t.br1 = tree->bsize, t.lf = 0;
		while(t.br0 < t.br1) { /* Tree. */
			const struct trie_branch *const branch = tree->branch + t.br0;
			/* _Sic_; '\0' is _not_ included for partial match. */
			for(byte.next = (bit += branch->skip) / CHAR_BIT;
				byte.cur <= byte.next; byte.cur++)
				if(prefix[byte.cur] == '\0') goto finally;
			if(!TRIE_QUERY(prefix, bit))
				t.br1 = ++t.br0 + branch->left;
			else
				t.br0 += branch->left + 1, t.lf += branch->left + 1;
			bit++;
		}
		if(!trie_bmp_test(&tree->is_child, t.lf)) break;
		tree = tree->leaf[t.lf].child;
	};
finally:
	assert(t.br0 <= t.br1
		&& t.lf - t.br0 + t.br1 <= tree->bsize);
	it->root = trie->root;
	it->next = it->end = tree;
	it->leaf = t.lf;
	it->leaf_end = t.lf + t.br1 - t.br0 + 1;
}

/** @return The leftmost key `lf` of `any`. */
static const char *PT_(sample)(const struct PT_(tree) *tree,
	unsigned lf) {
	assert(tree);
	while(trie_bmp_test(&tree->is_child, lf))
		tree = tree->leaf[lf].child, lf = 0;
	return PT_(to_key)(tree->leaf[lf].data);
}

/** Stores all `prefix` matches in `trie` and stores them in `it`.
 @param[it] Output remains valid until the topology of the trie changes.
 @order \O(|`prefix`|) */
static void PT_(prefix)(const struct T_(trie) *const trie,
	const char *const prefix, struct T_(trie_iterator) *it) {
	assert(trie && prefix && it);
	PT_(match_prefix)(trie, prefix, it);
	if(it->leaf_end <= it->leaf) return;
	assert(it->root && it->next && it->next == it->end
		&& it->leaf_end <= it->end->bsize + 1); /* fixme: what? */
	/* Makes sure the trie matches the string. */
	if(!trie_is_prefix(prefix, PT_(sample)(it->end, it->leaf_end - 1)))
		it->leaf_end = it->leaf;
}

/** @return Allocate a new tree with one undefined leaf. @throws[malloc] */
static struct PT_(tree) *PT_(tree)(void) {
	struct PT_(tree) *tree;
	if(!(tree = malloc(sizeof *tree))) { if(!errno) errno = ERANGE; return 0; }
	tree->bsize = 0, tree->skip = 0, trie_bmp_clear_all(&tree->is_child);
	/* fixme: doesn't need clear? */
	return tree;
}

#ifdef TRIE_TO_STRING
static const char *T_(trie_to_string)(const struct T_(trie) *);
#endif

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

static void PT_(grph)(const struct T_(trie) *const trie, const char *const fn) {
	assert(trie && fn);
#ifdef TRIE_TEST
	PT_(graph)(trie, fn);
#endif
}

static void PT_(prnt)(const struct PT_(tree) *const tree) {
	assert(tree);
#ifdef TRIE_TEST
	PT_(print)(tree);
#endif
}


#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)

/** All parameters to insert a node in a tree, minus the node. This is just
 for the insert function, which could end up inserting multiple times at
 different places when it causes a cascaded split. */
struct PT_(insert) {
	struct PT_(tree) *tr;
	unsigned br0, br1, lf, unused;
	struct { size_t b0, b1; } end;
	union { size_t top, end; } bit; /* Unused for insertion. */
};

/* Expand an un-full tree. Helper method for <fn:<PT>add_unique>.
 @return Inserted spot, an uninitialized data leaf. (_Ie_ `i.tr->leaf[i.lf]`;
 it doesn't really need to be a return value, but for clarity.) */
static union PT_(leaf) *PT_(expand)(/*const*/ struct PT_(insert) i) {
	struct { unsigned br0, br1, lf; } t;
	union PT_(leaf) *leaf;
	struct trie_branch *branch;
	assert(i.tr && i.tr->bsize < TRIE_BRANCHES);
	assert(i.br0 <= i.br1);
	assert(i.br1 <= i.tr->bsize);
	assert(i.br1 - i.br0 <= TRIE_MAX_LEFT);
	assert(i.lf <= i.tr->bsize + 1);
	assert(i.br0 == i.br1
		|| (i.end.b0 <= i.end.b1 && i.end.b1 - i.end.b0 <= UCHAR_MAX));

	/* Path defined by parameters: augment left counts along the left. */
	t.br0 = 0, t.br1 = i.tr->bsize, t.lf = 0;
	printf("insert: %s[%u,%u;%u]\n",
		orcify(i.tr), i.br0, i.br1, i.lf);
	while(t.br0 < i.br0) {
		branch = i.tr->branch + t.br0;
		if(t.br0 + branch->left + 1 < i.br0)
			t.br1 = ++t.br0 + branch->left++, printf("l");
		else
			t.br0 += branch->left + 1, t.lf += branch->left + 1, printf("r");
	}
	printf("\n");
	/*assert((t.lf += (t.br1 - t.br0 + 1) * i.is_right,
		printf("insert.augment: mir [%u,%u;%u]\n",
		t.br0, t.br1, t.lf),
		t.br0 == i.br0 && t.br1 == i.br1 && t.lf == i.lf));*/

	/* Expand the tree to include one more leaf. */
	leaf = i.tr->leaf + i.lf;
	memmove(leaf + 1, leaf, sizeof *leaf * ((i.tr->bsize + 1) - i.lf));
	/* Split with existing branch. */
	branch = i.tr->branch + i.br0;
	if(i.br0 != i.br1) assert(i.end.b0 <= i.end.b1
		&& i.end.b1 + 1 <= i.end.b0 + branch->skip),
		branch->skip -= i.end.b1 - i.end.b0 + 1;
	trie_bmp_insert(&i.tr->is_child, i.lf, 1);
	memmove(branch + 1, branch, sizeof *branch * (i.tr->bsize - i.br0));
	branch->left = (unsigned char)(i.br1 - i.br0);
	branch->skip = (unsigned char)(i.end.b1 - i.end.b0);
	i.tr->bsize++;
	return leaf;
}

/** Adds `x` to `trie`, which must not be present. @return Success.
 @throw[malloc, ERANGE]
 @throw[ERANGE] There is too many bytes similar for the data-type. */
static int PT_(add_unique)(struct T_(trie) *const trie,
	PT_(type) *const x) {
	const char *const key = PT_(to_key)(x);
	size_t bit;
	struct PT_(insert) find;
	struct { struct PT_(insert) prnt; size_t n; } full;
	const char *sample; /* Only used in Find. */
	assert(trie && x && key);

	printf("\n_add_: %s -> %s.\n", key, PT_(str)(trie));

start:
	/* <!-- Solitary. */
	if(!(find.tr = trie->root)) return (find.tr = PT_(tree)())
		&& (find.tr->leaf[0].data = x, trie->root = find.tr, 1);
	/* Solitary. --> */

	/* <!-- Find the first bit not in the tree. */
	assert(find.tr);
	bit = 0;
	full.prnt.tr = 0, full.prnt.lf = 0, full.n = 0;
	for( ; ; ) { /* Forest. */
		const int is_full = find.tr->bsize >= TRIE_BRANCHES;
		struct PT_(tree) *child;
		full.n = is_full ? full.n + 1 : 0;
		find.bit.top = find.end.b0 = bit;
		sample = PT_(sample)(find.tr, 0);
		printf("add.find: %s(b%lu)\n", orcify(find.tr), find.bit.top);
		find.br0 = 0, find.br1 = find.tr->bsize, find.lf = 0;
		while(find.br0 < find.br1) { /* Tree. */
			const struct trie_branch *const branch
				= find.tr->branch + find.br0;
			const size_t next = bit + branch->skip;
			/* is_full || !full.n -- don't have to worry. */
			for( ; bit < next; bit++)
			if(TRIE_DIFF(key, sample, bit)) { printf("(branch)\n"); goto found;}
			if(!TRIE_QUERY(key, bit)) {
				find.br1 = ++find.br0 + branch->left;
				printf("l");
			} else {
				find.br0 += branch->left + 1;
				find.lf += branch->left + 1;
				sample = PT_(sample)(find.tr, find.lf);
				printf("r");
			}
			find.end.b0 = ++bit;
		} /* Tree. */
		printf("\n");
		assert(find.br0 == find.br1 && find.lf <= find.tr->bsize);
		if(!trie_bmp_test(&find.tr->is_child, find.lf)) break;
		child = find.tr->leaf[find.lf].child;
		find.end.b1 = find.end.b0 + (child->bsize ? child->branch[0].skip : 0);
		if(!is_full) /* `bit.top` in `find` is `bit.end` in `full`. */
			memcpy(&full.prnt, &find, sizeof find), full.prnt.bit.end = bit;
		find.tr = child;
	} /* Forest. */
	{ /* Got to a leaf. Fixme: maybe a recursion would fix this limit. */
		const size_t limit = bit + UCHAR_MAX;
		while(!TRIE_DIFF(key, sample, bit))
			if(++bit > limit) return errno = EILSEQ, 0;
	}
found:
	find.end.b1 = bit;
	if(!TRIE_QUERY(key, bit)) find.br1 = find.br0; /* Left leaf. */
	else find.lf += find.br1 - find.br0 + 1; /* Right leaf. */
	printf("add.find: found %s[%u,%u;%u](b%lu,b%lu), top b%lu\n",
		orcify(find.tr), find.br0, find.br1, find.lf, find.end.b0, find.end.b1, find.bit.top);
	/* Find. --> */

	/* <!-- Backtrack and split down the path of all the full. */
	if(!full.n) goto insert;
	for( ; ; ) { /* Split a tree. */
		struct PT_(tree) *up, *left = 0, *right = 0;
		unsigned char split;
		printf("add.split: full.prnt %s(end b%lu)[%u,%u;%u]; "
			"full trees %lu\n",
			orcify(full.prnt.tr), full.prnt.bit.end, full.prnt.br0,
			full.prnt.br1, full.prnt.lf, full.n);
		/* Allocate one or two if the root-tree is being split. This is a
		 sequence point in splitting where the trie is valid. */
		if(!(up = full.prnt.tr) && !(up = PT_(tree)())
			|| !(right = PT_(tree)())) { if(!full.prnt.tr) free(up);
			free(right); return 0; }
		if(!full.prnt.tr) { /* Promoting the root raises depth of forest. */
			assert(up != full.prnt.tr && !full.prnt.lf);
			left = trie->root;
			assert(left && left->bsize);
			up->bsize = 1;
			up->branch[0].left = 0;
			up->branch[0].skip = left->branch[0].skip;
			trie_bmp_set(&up->is_child, 0), up->leaf[0].child = left;
			trie_bmp_set(&up->is_child, 1), up->leaf[1].child = right;
			trie->root = up;
		} else { /* Promote the root; (`up` is `prnt.tr`.) */
			assert(up->bsize < TRIE_BRANCHES
				&& full.prnt.lf <= full.prnt.tr->bsize
				&& trie_bmp_test(&up->is_child, full.prnt.lf));
			/* Switch places. */
			left = up->leaf[full.prnt.lf].child;
			assert(left && left->bsize); /* Or else wouldn't be full. */
			PT_(expand)(full.prnt)->child = left;
			trie_bmp_set(&full.prnt.tr->is_child, full.prnt.lf);
			full.prnt.tr->leaf[full.prnt.lf + 1].child = right;
		}
		assert(left->bsize);
		split = left->branch[0].left + 1; /* Leaves split. */
		printf("add.split: splitting on %u.\n", split);
		/* Copy the right part of the left to the new right. */
		right->bsize = left->bsize - split;
		memcpy(right->branch, left->branch + split,
			sizeof *left->branch * right->bsize);
		memcpy(right->leaf, left->leaf + split,
			sizeof *left->leaf * (right->bsize + 1));
		memcpy(&right->is_child, &left->is_child, sizeof left->is_child);
		trie_bmp_remove(&right->is_child, 0, split);
		/* Move back the branches of the left to account for the promotion. */
		left->bsize = split - 1;
		memmove(left->branch, left->branch + 1,
			sizeof *left->branch * (left->bsize + 1));

		{
			char a[128];
			sprintf(a, "graph/" QUOTE(TRIE_NAME) "-split-%lu.gv", full.n);
			PT_(grph)(trie, a);
		}

		if(--full.n) { /* Continue to the next tree. */
			/* fixme: Multi-intervening tree support is not implemented. */
			assert(0);
		} else { /* Last tree split -- adjust invalidated `find`. */
			printf("add.correct: find before %s[%u,%u;%u]\n",
				orcify(find.tr), find.br0, find.br1, find.lf);
			if(!find.br0) {
				printf("add.correct:"
					" position top %s, bit %lu, starting again\n", orcify(up),
					(unsigned long)find.end.b1);
				/* This is an unfortunate position: we should be storing the
				 positions on a stack, but it's easier and less power-hungry to
				 start again. This is also unlikely. */
				goto start;
			} else if(find.br1 < split) {
				find.tr = left;
				find.br0--, find.br1--;
			} else {
				find.tr = right;
				find.br0 -= split, find.br1 -= split, find.lf -= split;
			}
			printf("add.correct: find after %s[%u,%u;%u]\n",
				orcify(find.tr), find.br0, find.br1, find.lf);
			break;
		}
	} /* Split. --> */

insert:
	PT_(expand)(find)->data = x;
	PT_(grph)(trie, "graph/" QUOTE(TRIE_NAME) "-add.gv");
	printf("add_unique(%s) completed, tree bsize %d\n", key, find.tr->bsize);
	return 1;
}
#undef QUOTE
#undef QUOTE_

/** A bi-predicate; returns true if the `replace` replaces the `original`; used
 in <fn:<T>trie_policy_put>. */
typedef int (*PT_(replace_fn))(PT_(type) *original, PT_(type) *replace);

/** Adds `x` to `trie` and, if `eject` is non-null, stores the collided
 element, if any, as long as `replace` is null or returns true.
 @param[eject] If not-null, the ejected datum. If `replace` returns false, then
 `*eject == datum`, but it will still return true.
 @return Success. @throws[realloc, ERANGE] */
static int PT_(put)(struct T_(trie) *const trie, PT_(type) *const x,
	PT_(type) **const eject, const PT_(replace_fn) replace) {
	const char *key;
	PT_(type) **leaf;
	assert(trie && x);
	key = PT_(to_key)(x);
	/* Add if absent. */
	if(!(leaf = PT_(leaf_get)(trie, key)))
		{ if(eject) *eject = 0; return PT_(add_unique)(trie, x); }
	/* Collision policy. */
	if(replace && !replace(*leaf, x)) {
		if(eject) *eject = x;
	} else {
		if(eject) *eject = *leaf;
		*leaf = x;
	}
	return 1;
}

/* join() */

static PT_(type) *PT_(remove)(struct T_(trie) *const trie,
	const char *const key) {
#if 0
	struct PT_(tree) *parent_store;
	PT_(type) *data;
	struct PT_(tree) tree;
	struct trie_branch *branch;
	struct { unsigned br0, br1, lf; } in_tree;
	struct { unsigned parent, twin; } branches;
	unsigned leaf;
	size_t bit;
	struct { size_t cur, next; } byte;
	PT_(type) *rm;
#endif
	assert(trie && key);
	assert(0);
#if 0
	if(!store.info) return 0; /* Empty. */
	/* Preliminary exploration. Need parent tree and twin. */
	for(bit = 0; ; ) {
		PT_(extract)(store, &tree);
		if(!tree.bsize) {
			/* Not ensured . . . but should be? */
			assert(!TRIE_BITTEST(tree.children, 0));
			/* Delete this tree.
			 This is messy, could have `parent` in another tree. */
			assert(0);
		}
		in_tree.br0 = 0, in_tree.br1 = tree.bsize, in_tree.lf = 0;
		do {
			branch = tree.branches + (branches.parent = in_tree.br0);
			for(byte.next = (bit += branch->skip) / CHAR_BIT;
				byte.cur < byte.next; byte.cur++)
				if(key[byte.cur] == '\0') return 0;
			if(!TRIE_BITTEST(key, bit))
				branches.twin = in_tree.br0 + branch->left + 1,
				in_tree.br1 = ++in_tree.br0 + branch->left;
				/*if(is_write) branch->left--;*/
			else
				branches.twin = in_tree.br0 + 1,
				in_tree.br0 += branch->left + 1, in_tree.lf += branch->left + 1;
			bit++;
		} while(in_tree.br0 < in_tree.br1);
		assert(in_tree.br0 == in_tree.br1 && in_tree.lf <= tree.bsize);
		if(!TRIE_BITTEST(tree.children, in_tree.lf)) break;
		parent_store = store;
		store = tree.leaves[in_tree.lf].child;
	}
	/* We have the candidate leaf. */
	leaf = in_tree.lf;
	data = tree.leaves[leaf].data;
	if(strcmp(key, PT_(to_key)(rm = tree.leaves[leaf].data))) return 0;
	printf("Yes, \"%s\" exists as leaf %u. Parent tree %p.\n",
		key, leaf, (void *)parent_store.info);
	if(tree.branches[branches.parent].skip + 1
		+ tree.branches[branches.twin].skip > 255)
		{ printf("Would make too long.\n"); return 0; }
	/* Go down a second time and modify the tree. */
	in_tree.br0 = 0, in_tree.br1 = tree.bsize; /* Now `lf` goes down. */
	for( ; ; ) {
		branch = tree.branches + in_tree.br0;
		if(branch->left >= in_tree.lf) { /* Pre-order binary search. */
			if(!branch->left) break;
			in_tree.br1 = ++in_tree.br0 + branch->left;
			branch->left--;
		} else {
			if((in_tree.br0 += branch->left + 1) >= in_tree.br1) break;
			in_tree.lf -= branch->left + 1;
		}
	}
	tree.branches[branches.twin].skip
		+= 1 + tree.branches[branches.parent].skip;
	memmove(tree.branches + branches.parent,
		tree.branches + branches.parent + 1,
		sizeof tree.branches * (tree.bsize - branches.parent - 1));
	assert(store.info->bsize), store.info->bsize--;
	/* fixme: Delete from the children bitmap. */
	return data;
#endif
	return 0;
}

/** Frees `tree` and it's children recursively. */
static void PT_(clear)(struct PT_(tree) *const tree) {
	unsigned i;
	assert(tree);
	for(i = 0; i <= tree->bsize; i++) if(trie_bmp_test(&tree->is_child, i))
		PT_(clear)(tree->leaf[i].child);
	free(tree);
}

/** Counts the sub-tree `any`. @order \O(|`any`|) */
static size_t PT_(sub_size)(const struct PT_(tree) *const tree) {
	unsigned i;
	size_t size;
	assert(tree);
	size = tree->bsize + 1;
	/* This is extremely inefficient, but processor agnostic. */
	for(i = 0; i <= tree->bsize; i++) if(trie_bmp_test(&tree->is_child, i))
		size += PT_(sub_size)(tree->leaf[i].child) - 1;
	return size;
}

/** Counts the new iterator `it`. @order \O(|`it`|) */
static size_t PT_(size)(const struct T_(trie_iterator) *const it) {
	struct PT_(tree) *next = it->root; /* ? */
	size_t size;
	unsigned i;
	assert(it);
	if(!it->root || !it->next) return 0;
	/* We actually could find the size, but it's non-trivial, and just storing
	 the size is way more efficient. */
	assert(it->next == it->end
		&& it->leaf <= it->leaf_end && it->leaf_end <= next->bsize + 1);
	size = it->leaf_end - it->leaf;
	/* fixme: I have no idea what's going on. */
	for(i = it->leaf; i < it->leaf_end; i++)
		if(trie_bmp_test(&next->is_child, i))
		size += PT_(sub_size)(next->leaf[i].child) - 1;
	return size;
}

/* <!-- iterate interface */

/** Loads the first element of `trie` into `it`. @implements begin */
static void PT_(begin)(struct PT_(iterator) *const it,
	const struct T_(trie) *const trie)
	{ assert(it && trie); it->root = it->next = trie->root; it->leaf = 0; }

/** Advances `it`. @return The previous value or null. @implements next */
static PT_(type) *PT_(next)(struct PT_(iterator) *const it) {
	struct PT_(tree) *tree;
	assert(it);
	/*printf("_next_\n");*/
	if(!it->root || !(tree = it->next)) return 0;
	/* Off the end of the tree. */
	if(it->leaf > tree->bsize) {
		/* Definitely a data leaf or else we would have fallen thought.
		 Unless it had a concurrent modification. That would be bad; don't. */
		const char *key = PT_(to_key)(tree->leaf[tree->bsize].data);
		const struct PT_(tree) *tree1 = it->next;
		struct PT_(tree) *tree2 = it->root;
		size_t bit2 = 0;
		const struct trie_branch *branch2;
		struct { unsigned br0, br1, lf; } in_tree2;
		assert(key && tree2 && !trie_bmp_test(&tree->is_child, tree->bsize));
		/*printf("next: over the end of the tree on %s.\n",
			PT_(to_key)(tree.leaves[it->leaf - 1].data));*/
		for(it->next = 0; ; ) { /* Forest. */
			if(tree2 == tree1) break; /* Reached the tree. */
			in_tree2.br0 = 0, in_tree2.br1 = tree2->bsize, in_tree2.lf = 0;
			while(in_tree2.br0 < in_tree2.br1) { /* Tree. */
				branch2 = tree2->branch + in_tree2.br0;
				bit2 += branch2->skip;
				if(!TRIE_QUERY(key, bit2))
					in_tree2.br1 = ++in_tree2.br0 + branch2->left;
				else
					in_tree2.br0 += branch2->left + 1,
					in_tree2.lf += branch2->left + 1;
				bit2++;
			}
			/* Set it to the next value. */
			if(in_tree2.lf < tree2->bsize)
				it->next = tree2, it->leaf = in_tree2.lf + 1/*,
				printf("next: continues in tree %p, leaf %u.\n",
					(void *)store2.key, it->i)*/;
			/* We never reach the bottom, since it breaks up above. */
			assert(trie_bmp_test(&tree2->is_child, in_tree2.lf));
			tree2 = tree2->leaf[in_tree2.lf].child;
		}
		if(!it->next) { /*printf("next: fin\n");*/ it->leaf = 0; return 0; } /* No more. */
		tree = it->next; /* Update tree. */
	}
	/* Fall through the trees. */
	while(trie_bmp_test(&tree->is_child, it->leaf))
		tree = it->next = tree->leaf[it->leaf].child, it->leaf = 0
		/*, printf("next: fall though.\n")*/; /* !!! */
	/* Until we hit data. */
	/*printf("next: more data\n");*/
	return tree->leaf[it->leaf++].data;
}

/* iterate --> */


/** Initializes `trie` to idle. @order \Theta(1) @allow */
static void T_(trie)(struct T_(trie) *const trie)
	{ assert(trie); trie->root = 0; }

/** Returns an initialized `trie` to idle. @allow */
static void T_(trie_)(struct T_(trie) *const trie) {
	assert(trie);
	if(trie->root) PT_(clear)(trie->root), T_(trie)(trie);
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

/** @return Looks at only the index of `trie` for potential `key` matches,
 but will ignore the values of the bits that are not in the index.
 @order \O(|`key`|) @allow */
static PT_(type) *T_(trie_match)(const struct T_(trie) *const trie,
	const char *const key) { return PT_(match)(trie, key); }

/** @return Exact match for `key` in `trie` or null no such item exists.
 @order \O(|`key`|), <Thareja 2011, Data>. @allow */
static PT_(type) *T_(trie_get)(const struct T_(trie) *const trie,
	const char *const key) { return PT_(get)(trie, key); }

static PT_(type) *T_(trie_remove)(struct T_(trie) *const trie,
	const char *const key) { return PT_(remove)(trie, key); }

/** Adds a pointer to `x` into `trie` if the key doesn't exist already.
 @return If the key did not exist and it was created, returns true. If the key
 of `x` is already in `trie`, or an error occurred, returns false.
 @throws[realloc, ERANGE] Set `errno = 0` before to tell if the operation
 failed due to error. @order \O(|`key`|) @allow */
static int T_(trie_add)(struct T_(trie) *const trie, PT_(type) *const x)
	{ return assert(trie && x),
	PT_(get)(trie, PT_(to_key)(x)) ? 0 : PT_(add_unique)(trie, x); }

/** Updates or adds a pointer to `x` into `trie`.
 @param[eject] If not null, on success it will hold the overwritten value or
 a pointer-to-null if it did not overwrite any value.
 @return Success. @throws[realloc, ERANGE] @order \O(|`key`|) @allow */
static int T_(trie_put)(struct T_(trie) *const trie, PT_(type) *const x,
	PT_(type) **const eject)
	{ return assert(trie && x), PT_(put)(trie, x, eject, 0); }

/** Adds a pointer to `x` to `trie` only if the entry is absent or if calling
 `replace` returns true or is null.
 @param[eject] If not null, on success it will hold the overwritten value or
 a pointer-to-null if it did not overwrite any value. If a collision occurs and
 `replace` does not return true, this will be a pointer to `x`.
 @param[replace] Called on collision and only replaces it if the function
 returns true. If null, it is semantically equivalent to <fn:<T>trie_put>.
 @return Success. @throws[realloc, ERANGE] @order \O(|`key`|) @allow */
static int T_(trie_policy_put)(struct T_(trie) *const trie, PT_(type) *const x,
	PT_(type) **const eject, const PT_(replace_fn) replace)
	{ return assert(trie && x), PT_(put)(trie, x, eject, replace); }

/** Fills `it` with iteration parameters that find values of keys that start
 with `prefix` in `trie`.
 @param[prefix] To fill `it` with the entire `trie`, use the empty string.
 @param[it] A pointer to an iterator that gets filled. It is valid until a
 topological change to `trie`. Calling <fn:<T>trie_next> will iterate them in
 order. @order \O(|`prefix`|) */
static void T_(trie_prefix)(const struct T_(trie) *const trie,
	const char *const prefix, struct T_(trie_iterator) *const it)
	{ PT_(prefix)(trie, prefix, it); }

/** Counts the of the items in the new `it`; iterator must be new,
 (calling <fn:<T>trie_next> causes it to become undefined.)
 @order \O(|`it`|) @allow */
static size_t T_(trie_size)(const struct T_(trie_iterator) *const it)
	{ return PT_(size)(it); }

/** Advances `it`. @return The previous value or null. */
static PT_(type) *T_(trie_next)(struct T_(trie_iterator) *const it) {
	struct PT_(iterator) shunt;
	PT_(type) *x;
	assert(it && (it->next && it->root || !it->next));
	/* This adds another constraint: instead of ending when the trie has no
	 more entries like <fn:<PT>next>, we check if it has passed the point. */
	if(it->next == it->end && it->leaf >= it->leaf_end) return 0;
	shunt.root = it->root, shunt.next = it->next,
		shunt.leaf = it->leaf, x = PT_(next)(&shunt);
	it->next = shunt.next, it->leaf = shunt.leaf;
	return x;
}

/* typename std::unordered_map<Key,T,Hash,KeyEqual,Alloc>::size_type
	erase_if(std::unordered_map<Key,T,Hash,KeyEqual,Alloc>& c, Pred pred);
 std::pair */

/* Define these for traits. */
#define BOX_ PT_
#define BOX_CONTAINER struct T_(trie)
#define BOX_CONTENTS PT_(type)

#ifdef TRIE_TO_STRING /* <!-- str */
/** Uses the natural `a` -> `z` that is defined by `TRIE_KEY`. */
static void PT_(to_string)(const PT_(type) *const a, char (*const z)[12])
	{ assert(a && z); sprintf(*z, "%.11s", PT_(to_key)(a)); }
#define Z_(n) TRIE_CAT(T_(trie), n)
#define TO_STRING &PT_(to_string)
#define TO_STRING_LEFT '{'
#define TO_STRING_RIGHT '}'
#include "to_string.h" /** \include */
#endif /* str --> */

#ifdef TRIE_TEST /* <!-- test */
#include "../test/test_trie.h" /** \include */
#endif /* test --> */

static void PT_(unused_base_coda)(void);
static void PT_(unused_base)(void) {
	PT_(begin)(0, 0);
	T_(trie)(0); T_(trie_)(0);
	T_(trie_match)(0, 0); T_(trie_get)(0, 0);
	T_(trie_remove)(0, 0);
	T_(trie_add)(0, 0); T_(trie_put)(0, 0, 0); T_(trie_policy_put)(0, 0, 0, 0);
	T_(trie_prefix)(0, 0, 0); T_(trie_size)(0); T_(trie_next)(0);
	PT_(unused_base_coda)();
}
static void PT_(unused_base_coda)(void) { PT_(unused_base)(); }

#undef TRIE_NAME
#undef TRIE_TYPE
#undef TRIE_KEY
#ifdef TRIE_TEST
#undef TRIE_TEST
#endif
