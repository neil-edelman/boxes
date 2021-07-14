/** @license 2020 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Prefix Tree

 ![Example of trie.](../web/trie.png)

 A <tag:<T>trie> is a prefix tree, digital tree, or trie, implemented as an
 array of pointers-to-`T` whose keys are always in lexicographically-sorted
 order. It can be seen as a <Morrison, 1968 PATRICiA>: a compact
 [binary radix trie](https://en.wikipedia.org/wiki/Radix_tree), only
 storing the where the keys are different. Strings can be any encoding with a
 byte null-terminator, including
 [modified UTF-8](https://en.wikipedia.org/wiki/UTF-8#Modified_UTF-8).

 @param[TRIE_NAME, TRIE_TYPE]
 <typedef:<PT>type> that satisfies `C` naming conventions when mangled and an
 optional returnable type that is declared, (it is used by reference only
 except if `TRIE_TEST`.) `<PT>` is private, whose names are prefixed in a
 manner to avoid collisions.

 @param[TRIE_KEY]
 A function that satisfies <typedef:<PT>key_fn>. Must be defined if and only if
 `TRIE_TYPE` is defined.

 @param[TRIE_TO_STRING]
 Defining this includes <to_string.h> with the keys as the string.

 @param[TRIE_TEST]
 Unit testing framework <fn:<T>trie_test>, included in a separate header,
 <../test/test_trie.h>. Must be defined equal to a (random) filler function,
 satisfying <typedef:<PT>action_fn>. Requires that `NDEBUG` not be defined
 and `TRIE_ITERATE_TO_STRING`.

 @depend [array](https://github.com/neil-edelman/array)
 @std C89 */

#include <string.h> /* size_t memmove strcmp memcpy */
#include <limits.h> /* UINT_MAX */
#include <errno.h> /* errno ERANGE */
#include <assert.h>

#ifdef TRIE_STRICT_C90 /* <!-- c90: Just guess and hope. */
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
#else /* c90 --><!-- !c90 */
#include <stdint.h>
#endif /* !c90 --> */



#ifndef TRIE_NAME
#error Name TRIE_NAME undefined.
#endif
#if defined(TRIE_TYPE) ^ defined(TRIE_KEY)
#error TRIE_TYPE and TRIE_KEY have to be mutually defined.
#endif


#ifndef TRIE_H /* <!-- idempotent */
#define TRIE_H
/* http://c-faq.com/misc/bitsets.html */
#include <limits.h> /* CHAR_BIT */
#define TRIE_BITMASK(n) (1 << ((n) % CHAR_BIT))
#define TRIE_BITSLOT(n) ((n) / CHAR_BIT)
#define TRIE_BITTEST(a, n) ((a)[TRIE_BITSLOT(n)] & TRIE_BITMASK(n))
#define TRIE_BITDIFF(a, b, n) (((a)[TRIE_BITSLOT(n)] ^ (b)[TRIE_BITSLOT(n)]) \
	& ((1 << (CHAR_BIT - 1)) >> ((n) % CHAR_BIT)))
/* Worst-case all-left, `[0,max(tree.left)]` */
#define TRIE_MAX_LEFT ((1 << CHAR_BIT) - 1)
#define TRIE_MAX_BRANCH (TRIE_MAX_LEFT + 1)
#define TRIE_ORDER (TRIE_MAX_BRANCH + 1) /* Maximum branching factor. */
/* Fills `TRIE_BRANCH`: it's kind of arbitrary, see <fn:<PT>assign_bsize>. */
static const unsigned char trie_bsize_lookup[] = {
	0, 1, 2, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, /* 0..15 */
	5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, /* 16..31 */
	6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, /* 32..47 */
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, /* 48..63 */
	7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, /* 64..79 */
	8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, /* 80..95 */
	8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, /* 96..111 */
	8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, /* 112..127 */
	8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, /* 128..143 */
	9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, /* 144..159 */
	9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, /* 160..175 */
	9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, /* 176..191 */
	9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, /* 192..207 */
	9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, /* 208..223 */
	9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, /* 224..239 */
	9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, /* 240..255 */
	9 /* `x == 0 ? 0 : 9 - __builtin_clz(x)` on a bit.  */
};
/** @return Whether `a` and `b` are equal up to the minimum of their lengths'.
 Used in <fn:<T>trie_prefix>. */
static int trie_is_prefix(const char *a, const char *b) {
	for( ; ; a++, b++) {
		if(*a == '\0') return 1;
		if(*a != *b) return *b == '\0';
	}
}
struct trie_branch { unsigned char left, skip; };
struct trie_tree_start { unsigned short bsize; };
#endif /* idempotent --> */


/* <Kernighan and Ritchie, 1988, p. 231>. */
#if defined(T_) || defined(PT_) \
	|| (defined(TRIE_SUBTYPE) ^ (defined(CAT) || defined(CAT_)))
#error Unexpected P?T_ or CAT_?; possible stray TRIE_EXPECT_TRAIT?
#endif
#ifndef TRIE_SUBTYPE /* <!-- !sub-type */
#define CAT_(x, y) x ## _ ## y
#define CAT(x, y) CAT_(x, y)
#endif /* !sub-type --> */
#define T_(thing) CAT(TRIE_NAME, thing)
#define PT_(thing) CAT(trie, T_(thing))


/* Default values for string. */
#ifndef TRIE_TYPE /* <!-- !type */
static char *PT_(raw)(char **a) { return assert(a), *a; }
#define TRIE_TYPE char
#define TRIE_KEY &PT_(raw)
#endif /* !type --> */

/** Declared type of the trie; defaults to `char`. */
typedef TRIE_TYPE PT_(type);

/** B-trie nodes: non-empty semi-implicit complete binary tree of a
 fixed-maximum-size. `bsize + 1` is the rank. To save space, there could be
 multiple sizes; start fields are the same. */
union PT_(any_ptree) {
	struct trie_tree_start *t; struct PT_(tree0) *t0; struct PT_(tree1) *t1;
	struct PT_(tree2) *t2; struct PT_(tree4) *t4; struct PT_(tree8) *t8;
	struct PT_(tree16) *t16; struct PT_(tree32) *t32; struct PT_(tree64) *t64;
	struct PT_(tree128) *t128; struct PT_(tree256) *t256;
};

/** A leaf is either data at the base of the b-trie or another tree-link. */
union PT_(leaf) { PT_(type) *data; union PT_(any_ptree) link; };

struct PT_(tree0)
	{ unsigned short bsize; union PT_(leaf) leaves[1]; };
struct PT_(tree1) { unsigned short bsize;
	struct trie_branch branches[1]; union PT_(leaf) leaves[2]; };
struct PT_(tree2) { unsigned short bsize;
	struct trie_branch branches[2]; union PT_(leaf) leaves[3]; };
struct PT_(tree4) { unsigned short bsize;
	struct trie_branch branches[4]; union PT_(leaf) leaves[5]; };
struct PT_(tree8) { unsigned short bsize;
	struct trie_branch branches[8]; union PT_(leaf) leaves[9]; };
struct PT_(tree16) { unsigned short bsize;
	struct trie_branch branches[16]; union PT_(leaf) leaves[17]; };
struct PT_(tree32) { unsigned short bsize;
	struct trie_branch branches[32]; union PT_(leaf) leaves[33]; };
struct PT_(tree64) { unsigned short bsize;
	struct trie_branch branches[64]; union PT_(leaf) leaves[65]; };
struct PT_(tree128) { unsigned short bsize;
	struct trie_branch branches[128]; union PT_(leaf) leaves[129]; };
struct PT_(tree256) { unsigned short bsize;
	struct trie_branch branches[256]; union PT_(leaf) leaves[257]; };
union PT_(maybe_tree) { PT_(type) *data; union PT_(any_ptree) tree; };

/** A bi-predicate; returns true if the `replace` replaces the `original`; used
 in <fn:<T>trie_policy_put>. */
typedef int (*PT_(replace_fn))(PT_(type) *original, PT_(type) *replace);


/** Responsible for picking out the null-terminated string. One must not modify
 this string while in any trie. */
static const char *(*PT_(to_key))(const PT_(type) *a) = (TRIE_KEY);

/** @return False. Ignores `a` and `b`. @implements <typedef:<PT>replace_fn> */
static int PT_(false_replace)(PT_(type) *const a, PT_(type) *const b)
	{ return (void)a, (void)b, 0; }

/** For `tree`, outputs `branch_ptr` and `leaf_ptr` for the kind of tree.
 @return The branch size. */
static unsigned short PT_(extract)(union PT_(any_ptree) tree,
	struct trie_branch **branches_ptr, union PT_(leaf) **leaves_ptr) {
	assert(tree.t && tree.t->bsize < TRIE_MAX_BRANCH
		&& branches_ptr && leaves_ptr);
	switch(trie_bsize_lookup[tree.t->bsize]) {
	case 0: *branches_ptr = 0; *leaves_ptr = tree.t0->leaves; return 0;
#define TRIE_SWITCH(n, m) case n: *branches_ptr = tree.t##m->branches; \
	*leaves_ptr = tree.t##m->leaves; return tree.t##m->bsize;
		TRIE_SWITCH(1, 1)
		TRIE_SWITCH(2, 2)
		TRIE_SWITCH(3, 4)
		TRIE_SWITCH(4, 8)
		TRIE_SWITCH(5, 16)
		TRIE_SWITCH(6, 32)
		TRIE_SWITCH(7, 64)
		TRIE_SWITCH(8, 128)
		TRIE_SWITCH(9, 256)
#undef TRIE_SWITCH
	default: assert(0); return 0;
	}
}

/** Compares keys of `a` and `b`. Used in <fn:<T>trie_from_array>.
 @implements <typedef:<PT>bipredicate_fn> */
static int PT_(compare)(const PT_(type) *const a, const PT_(type) *const b)
	{ return strcmp(PT_(to_key)(a), PT_(to_key)(b)); }

#ifdef TRIE_TO_STRING /* <!-- str */
/** Uses the natural `datum` -> `a` that is defined by `TRIE_KEY`. */
static void PT_(to_string)(PT_(type) *const a, char (*const z)[12])
	{ assert(a && z); sprintf(*z, "%.11s", PT_(to_key)(a)); }
#endif /* str --> */

/** To initialize it to an idle state, see <fn:<T>trie>, `TRIE_IDLE`, `{0}`
 (`C99`), or being `static`.

 ![States.](../web/states.png) */
struct T_(trie) {
	unsigned short depth; /* depth ? b-tree root : data root */
	union PT_(maybe_tree) root;
};
#ifndef TRIE_IDLE /* <!-- !zero */
#define TRIE_IDLE { 0, { 0 } }
#endif /* !zero --> */

/** @return Looks at only the index of `trie` for potential `key` matches,
 otherwise `key` is definitely not in `trie`. @order \O(`key.length`) */
static PT_(type) *PT_(match)(const struct T_(trie) *const trie,
	const char *const key) {
	unsigned short tree_depth = trie->depth;
	union PT_(any_ptree) tree;
	struct trie_branch *branches;
	union PT_(leaf) *leaves;
	size_t bit; /* `bit \in key`.  */
	struct { unsigned br0, br1, lf; } in_tree;
	struct { size_t i, next; } byte; /* `key` null checks. */
	assert(trie && key);
	if(!tree_depth) return trie->root.data; /* [0, 1] items. */
	for(byte.i = 0, bit = 0, tree = trie->root.tree; ; ) { /* B-forest. */
		assert(tree.t);
		in_tree.br0 = 0;
		in_tree.br1 = PT_(extract)(tree, &branches, &leaves);
		in_tree.lf = 0;
		while(in_tree.br0 < in_tree.br1) { /* Binary tree. */
			const struct trie_branch *const branch = branches + in_tree.br0;
			for(byte.next = (bit += branch->skip) >> 3; byte.i < byte.next;
				byte.i++) if(key[byte.i] == '\0') return 0; /* Too short. */
			if(!TRIE_BITTEST(key, bit))
				in_tree.br1 = ++in_tree.br0 + branch->left;
			else
				in_tree.br0 += branch->left + 1, in_tree.lf += branch->left + 1;
			bit++;
		}
		if(!--tree_depth) break;
		tree = leaves[in_tree.lf].link;
	};
	return leaves[in_tree.lf].data;
}

/** @return Exact match for `key` in `trie` or null. (fixme: private?) */
static PT_(type) *PT_(get)(const struct T_(trie) *const trie,
	const char *const key) {
	PT_(type) *n;
	return (n = PT_(match)(trie, key)) && !strcmp(PT_(to_key)(n), key) ? n : 0;
}

/** @return The leftmost key of the `b` branch of tree `tree`. */
static const char *PT_(key_sample)(const union PT_(any_ptree) tree,
	const unsigned short branch) {
	/*struct tree *tree = ta->data + tr;
	assert(ta && tr < ta->size && br <= tree->bsize);
	if(!TRIESTR_TEST(tree->link, br)) return tree->leaves[br].data;
	tr = tree->leaves[br].link;
	for( ; ; ) {
		tree = ta->data + tr;
		if(!TRIESTR_TEST(tree->link, 0)) return tree->leaves[0].data;
		tr = tree->leaves[0].link;
	}*/
	return 0;
}

/** Initialises `trie` to idle. @order \Theta(1) @allow */
static void T_(trie)(struct T_(trie) *const trie)
	{ assert(trie); trie->depth = 0; trie->root.data = 0; }

/** Returns an initialised `trie` to idle. @allow */
static void T_(trie_)(struct T_(trie) *const trie) {
	assert(trie);
	/* fixme */
	T_(trie)(trie);
}

/** Sets `trie` to be empty. That is, the size of `trie` will be zero, but if
 it was previously in an active non-idle state, it continues to be.
 @order \Theta(1) @allow */
static void T_(trie_clear)(struct T_(trie) *const trie)
	{ assert(trie); /* ... */}

/** @return The <typedef:<PT>type> with `key` in `trie` or null no such item
 exists. @order \O(|`key`|), <Thareja 2011, Data>. @allow */
static PT_(type) *T_(trie_get)(const struct T_(trie) *const trie,
	const char *const key) { return assert(trie && key), PT_(get)(trie, key); }

static int PT_(add_unique)(struct T_(trie) *const trie, PT_(type) *const x) {
	const char *const x_key = PT_(to_key)(x);
	struct { size_t b, b0, b1; } in_bit;
	struct { size_t idx, tree_start_bit; } in_forest;
	struct { unsigned br0, br1, lf; } in_tree;
	union PT_(any_ptree) tree;
	struct branch *branch;
	union leaf *leaf;
	const char *sample;
	int is_write, is_right, is_split = 0;

	assert(trie && x);
	if(!trie->depth) { /* [0,1] items: root is an item. */
		struct PT_(tree1) *t1;
		const char *existing_key;
		size_t dif;
		if(!trie->root.data) return trie->root.data = x, 1;
		/* Change over to tree root. */
		existing_key = PT_(to_key)(trie->root.data);
		/* fixme: Find a way to eliminate 8-consecutive-byte limit. */
		for(dif = 0; !TRIE_BITDIFF(existing_key, x_key, dif); dif++)
			if(dif > 255) return errno = ERANGE, 0;
		if(!(t1 = malloc(sizeof *t1)))
			{ if(!errno) errno = ERANGE; return 0; }
		t1->bsize = 1;
		t1->branches[0].left = 0;
		t1->branches[0].skip = (unsigned char)dif;
		dif = !TRIE_BITTEST(x_key, dif);
		t1->leaves[dif].data = trie->root.data;
		t1->leaves[!dif].data = x;
		trie->root.tree.t1 = t1;
		trie->depth = 1;
		return 1;
	} /* [2,] items: root is a tree. */
	tree = trie->root.tree;
	in_bit.b = 0;
	{
		struct trie_branch *branches;
		union PT_(leaf) *leaves;
		unsigned short bsize = PT_(extract)(tree, &branches, &leaves);
		printf("bsize %u\n", bsize);
		in_forest.tree_start_bit = in_bit.b; /* Save for backtracking. */
	} while(0);
	assert(0);
#if 0
	in_bit.b = 0, in_forest.idx = 0, is_write = 0;
	do {
		in_forest.tree_start_bit = in_bit.b;
		is_write =
tree:
		assert(in_forest.idx < forest->size);
		tree = forest->data + in_forest.idx;
		sample = key_sample(forest, in_forest.idx, 0);
		/* Pre-select `is_write` if the tree is not full and has no links. */
		if(!is_write && tree->bsize < TRIE_BRANCH
			&& !memcmp(&tree->link, zero, TRIE_BITMAP)) is_write = 1;
		in_bit.b0 = in_bit.b;
		in_tree.br0 = 0, in_tree.br1 = tree->bsize, in_tree.lf = 0;
		while(in_tree.br0 < in_tree.br1) {
			branch = tree->branches + in_tree.br0;
			/* Test all the skip bits. */
			for(in_bit.b1 = in_bit.b + branch->skip; in_bit.b < in_bit.b1;
				in_bit.b++) if(TRIESTR_DIFF(key, sample, in_bit.b)) goto leaf;
			/* Decision bit. */
			if(!TRIESTR_TEST(key, in_bit.b)) {
				in_tree.br1 = ++in_tree.br0 + branch->left;
				if(is_write) branch->left++;
			} else {
				in_tree.br0 += branch->left + 1;
				in_tree.lf  += branch->left + 1;
				sample = key_sample(forest, in_forest.idx, in_tree.lf);
			}
			in_bit.b0 = in_bit.b1, in_bit.b++;
		}
		assert(in_tree.br0 == in_tree.br1 && in_tree.lf <= tree->bsize);
	} while(TRIESTR_TEST(tree->link, in_tree.lf)
		&& (in_forest.idx = tree->leaves[in_tree.lf].link, 1));
	/* Got to the leaves; uniqueness guarantees that this is safe. */
	while(!TRIESTR_DIFF(x_key, sample, in_bit.b)) in_bit.b++;

leaf:
	if(TRIESTR_TEST(x_key, in_bit.b))
		is_right = 1, in_tree.lf += in_tree.br1 - in_tree.br0 + 1;
	else
		is_right = 0;
	/*printf("insert %s, at index %u bit %lu.\n", key, in_tree.lf, in_bit.b);*/
	assert(in_tree.lf <= tree->bsize + 1u);

	if(is_write) goto insert;
	/* If the tree is full, split it. */
	assert(tree->bsize <= TRIE_BRANCH);
	if(tree->bsize == TRIE_BRANCH) {
		/*printf("Splitting tree %lu.\n", in_forest.idx);*/
		assert(!is_split);
		if(!trie_split(trie, in_forest.idx)) return 0;
		assert(is_split = 1);
		/*printf("Returning to \"%s\" in tree %lu.\n", key, in_forest.idx);*/
	} else {
		/* Now we are sure that this tree is the one getting modified. */
		is_write = 1;
	}
	in_bit.b = in_forest.tree_start_bit;
	goto tree;

insert:
	leaf = tree->leaves + in_tree.lf;
	memmove(leaf + 1, leaf, sizeof *leaf * (tree->bsize + 1 - in_tree.lf));
	leaf->data = key;
	bmp_insert(tree->link, in_tree.lf);
	branch = tree->branches + in_tree.br0;
	if(in_tree.br0 != in_tree.br1) { /* Split `skip` with the existing branch. */
		assert(in_bit.b0 <= in_bit.b
			&& in_bit.b + !in_tree.br0 <= in_bit.b0 + branch->skip);
		branch->skip -= in_bit.b - in_bit.b0 + !in_tree.br0;
	}
	memmove(branch + 1, branch, sizeof *branch * (tree->bsize - in_tree.br0));
	assert(in_tree.br1 - in_tree.br0 < 256
		&& in_bit.b >= in_bit.b0 + !!in_tree.br0
		&& in_bit.b - in_bit.b0 - !!in_tree.br0 < 256);
	branch->left = is_right ? (unsigned char)(in_tree.br1 - in_tree.br0) : 0;
	branch->skip = (unsigned char)(in_bit.b - in_bit.b0 - !!in_tree.br0);
	tree->bsize++;
#endif

	return 1;
}

/** @return If `x` is already in `trie`, returns false, otherwise success.
 @throws[realloc, ERANGE] */
static int T_(trie_add)(struct T_(trie) *const trie, PT_(type) *const x) {
	return assert(trie && x),
		PT_(get)(trie, PT_(to_key)(x)) ? 0 : PT_(add_unique)(trie, x);
}

#if 0
/** Add `datum` to `trie`. Must not be the same as any key of `trie`; _ie_ it
 does not check for the end of the string. @return Success. @order \O(|`trie`|)
 @throws[ERANGE] Trie reached it's conservative maximum, which on machines
 where the pointer is 64-bits, is 4.5T. On 32-bits, it's 1M.
 @throws[realloc, ERANGE] @fixme Throw EILSEQ if two strings have subsequences
 that are equal in more than 2^12 bits. */
static int PT_(add)(struct T_(trie) *const trie, PT_(type) *const datum) {
	const size_t leaf_size = trie->leaves.size, branch_size = leaf_size - 1;
	size_t n0 = 0, n1 = branch_size, i = 0, left, bit = 0, bit0 = 0, bit1;
	TrieBranch *branch = 0;
	const char *const data_key = PT_(to_key)(datum), *n0_key;
	PT_(leaf) *leaf;
	int cmp;
	assert(trie && datum);
	/* Empty special case. */
	if(!leaf_size) return assert(!trie->branches.size),
		(leaf = A_(array_new)(&trie->leaves)) ? *leaf = datum, 1 : 0;
	/* Redundant `size_t`, but maybe we will use it like Judy. */
	assert(leaf_size == branch_size + 1);
	/* Conservative maximally unbalanced trie. Reserve one more. */
	if(leaf_size >= TRIE_LEFT_MAX) return errno = ERANGE, 0;
	if(!A_(array_reserve)(&trie->leaves, leaf_size + 1)
		|| !trie_branch_array_reserve(&trie->branches, branch_size + 1))
		return 0;
	/* Branch from internal node. */
	while(branch = trie->branches.data + n0,
		n0_key = PT_(to_key)(trie->leaves.data[i]), n0 < n1) {
		/* fixme: Detect overflow 12 bits between. */
		for(bit1 = bit + trie_skip(*branch); bit < bit1; bit++)
			if((cmp = trie_strcmp_bit(data_key, n0_key, bit)) != 0) goto insert;
		bit0 = bit1;
		left = trie_left(*branch) + 1; /* Leaves. */
		if(!trie_is_bit(data_key, bit++))
			trie_left_inc(branch), n1 = n0++ + left;
		else n0 += left, i += left;
	}
	/* Branch from leaf. */
	while((cmp = trie_strcmp_bit(data_key, n0_key, bit)) == 0) bit++;
insert:
	assert(n0 <= n1 && n1 <= trie->branches.size && n0_key
		&& i <= trie->leaves.size && !n0 == !bit0);
	/* How many left entries are there to move. */
	if(cmp < 0) left = 0;
	else left = n1 - n0, i += left + 1;
	/* Insert leaf. */
	leaf = trie->leaves.data + i;
	memmove(leaf + 1, leaf, sizeof *leaf * (leaf_size - i));
	*leaf = datum, trie->leaves.size++;
	/* Insert branch. */
	branch = trie->branches.data + n0;
	if(n0 != n1) { /* Split the skip value with the existing branch. */
		const size_t branch_skip = trie_skip(*branch);
		assert(branch_skip + bit0 >= bit + !n0);
		trie_skip_set(branch, branch_skip + bit0 - bit - !n0);
	}
	memmove(branch + 1, branch, sizeof *branch * (branch_size - n0));
	*branch = trie_branch(bit - bit0 - !!n0, left), trie->branches.size++;
	return 1;
}

/** Looks at only the index for potential matches.
 @param[result] A index pointer to leaves that matches `key` when true.
 @return True if `key` in `trie` has matched, otherwise `key` is definitely is
 not in `trie`. @order \O(`key.length`) */
static int PT_(param_index_get)(const struct T_(trie) *const trie,
	const char *const key, size_t *const result) {
	size_t n0 = 0, n1 = trie->leaves.size, i = 0, left;
	TrieBranch branch;
	size_t n0_byte, str_byte = 0, bit = 0;
	assert(trie && key && result);
	if(!n1) return 0; /* Special case: there is nothing to match. */
	n1--, assert(n1 == trie->branches.size);
	while(n0 < n1) {
		branch = trie->branches.data[n0];
		bit += trie_skip(branch);
		/* Skip the don't care bits, ending up at the decision bit. */
		for(n0_byte = bit >> 3; str_byte < n0_byte; str_byte++)
			if(key[str_byte] == '\0') return 0;
		left = trie_left(branch);
		if(!trie_is_bit(key, bit)) n1 = ++n0 + left;
		else n0 += left + 1, i += left + 1;
		bit++;
	}
	assert(n0 == n1 && i < trie->leaves.size);
	*result = i;
	return 1;
}

/** @return True if found the exact `key` in `trie` and stored it's index in
 `result`. */
static int PT_(param_get)(const struct T_(trie) *const trie,
	const char *const key, size_t *const result) {
	return PT_(param_index_get)(trie, key, result)
		&& !strcmp(PT_(to_key)(trie->leaves.data[*result]), key);
}

/** @return `trie` entry that matches bits of `key`, (ignoring the don't care
 bits,) or null if either `key` didn't have the length to fully differentiate
 more then one entry or the `trie` is empty. */
static PT_(type) *PT_(index_get)(const struct T_(trie) *const trie,
	const char *const key) {
	size_t i;
	return PT_(param_index_get)(trie, key, &i) ? trie->leaves.data[i] : 0;
}

/** In `trie`, which must be non-empty, given a partial `prefix`, stores all
 leaf prefix matches between `low`, `high`, only given the index, ignoring
 don't care bits. @order \O(`prefix.length`) @allow */
static void T_(trie_index_prefix)(const struct T_(trie) *const trie,
	const char *const prefix, size_t *const low, size_t *const high) {
	size_t n0 = 0, n1 = trie->leaves.size, i = 0, left;
	TrieBranch branch;
	size_t n0_byte, str_byte = 0, bit = 0;
	assert(trie && prefix && low && high && n1);
	n1--, assert(n1 == trie->branches.size);
	while(n0 < n1) {
		branch = trie->branches.data[n0];
		bit += trie_skip(branch);
		/* _Sic_; '\0' is _not_ included for partial match. */
		for(n0_byte = bit >> 3; str_byte <= n0_byte; str_byte++)
			if(prefix[str_byte] == '\0') goto finally;
		left = trie_left(branch);
		if(!trie_is_bit(prefix, bit)) n1 = ++n0 + left;
		else n0 += left + 1, i += left + 1;
		bit++;
	}
	assert(n0 == n1);
finally:
	assert(n0 <= n1 && i - n0 + n1 < trie->leaves.size);
	*low = i, *high = i - n0 + n1;
}

/** @return Whether, in `trie`, given a partial `prefix`, it has found `low`,
 `high` prefix matches. */
static int T_(trie_prefix)(const struct T_(trie) *const trie,
	const char *const prefix, size_t *const low, size_t *const high) {
	assert(trie && prefix && low && high);
	return trie->leaves.size ? (T_(trie_index_prefix)(trie, prefix, low, high),
		trie_is_prefix(prefix, PT_(to_key)(trie->leaves.data[*low]))) : 0;
}

/** Adds `datum` to `trie` and, if `eject` is non-null, stores the collided
 element, if any, as long as `replace` is null or returns true.
 @param[eject] If not-null, the ejected datum. If `replace` returns false, then
 `*eject == datum`, but it will still return true.
 @return Success. @throws[realloc, ERANGE] */
static int PT_(put)(struct T_(trie) *const trie, PT_(type) *const datum,
	PT_(type) **const eject, const PT_(replace_fn) replace) {
	const char *data_key;
	PT_(leaf) *match;
	size_t i;
	assert(trie && datum);
	data_key = PT_(to_key)(datum);
	/* Add if absent. */
	if(!PT_(param_get)(trie, data_key, &i)) {
		if(eject) *eject = 0;
		return PT_(add)(trie, datum);
	}
	assert(i < trie->leaves.size), match = trie->leaves.data + i;
	/* Collision policy. */
	if(replace && !replace(*match, datum)) {
		if(eject) *eject = datum;
	} else {
		if(eject) *eject = *match;
		*match = datum;
	}
	return 1;
}

/** Adds `datum` to `trie` if absent.
 @param[trie, datum] If null, returns null.
 @return Success. If data with the same key is present, returns true but
 doesn't add `datum`.
 @throws[realloc] There was an error with a re-sizing.
 @throws[ERANGE] The key is greater then 510 characters or the trie has reached
 it's maximum size. @order \O(`size`) @allow */
static int T_(trie_add)(struct T_(trie) *const trie, PT_(type) *const datum) {
	return assert(trie && datum), PT_(put)(trie, datum, 0, &PT_(false_replace));
}

/** Updates or adds `datum` to `trie`.
 @param[trie, datum] If null, returns null.
 @param[eject] If not null, on success it will hold the overwritten value or
 a pointer-to-null if it did not overwrite.
 @return Success.
 @throws[realloc] There was an error with a re-sizing.
 @throws[ERANGE] The key is greater then 510 characters or the trie has reached
 it's maximum size. @order \O(`size`) @allow */
static int T_(trie_put)(struct T_(trie) *const trie,
	PT_(type) *const datum, PT_(type) **const eject) {
	return assert(trie && datum), PT_(put)(trie, datum, eject, 0);
}

/** Adds `datum` to `trie` only if the entry is absent or if calling `replace`
 returns true.
 @param[eject] If not null, on success it will hold the overwritten value or
 a pointer-to-null if it did not overwrite a previous value. If a collision
 occurs and `replace` does not return true, this value will be `data`.
 @param[replace] Called on collision and only replaces it if the function
 returns true. If null, it is semantically equivalent to <fn:<T>trie_put>.
 @return Success. @throws[realloc] There was an error with a re-sizing.
 @throws[ERANGE] The key is greater then 510 characters or the trie has reached
 it's maximum size. @order \O(`size`) @allow */
static int T_(trie_policy_put)(struct T_(trie) *const trie,
	PT_(type) *const datum, PT_(type) **const eject,
	const PT_(replace_fn) replace) {
	return assert(trie && datum), PT_(put)(trie, datum, eject, replace);
}

/** @return Whether leaf index `i` has been removed from `trie`.
 @fixme There is nothing stopping an `assert` from being triggered. */
static int PT_(index_remove)(struct T_(trie) *const trie, size_t i) {
	size_t n0 = 0, n1 = trie->branches.size, parent_n0, left;
	size_t *parent, *twin; /* Branches. */
	assert(trie && i < trie->leaves.size
		&& trie->branches.size + 1 == trie->leaves.size);
	/* Remove leaf. */
	if(!--trie->leaves.size) return 1; /* Special case of one leaf. */
	memmove(trie->leaves.data + i, trie->leaves.data + i + 1,
		sizeof trie->leaves.data * (n1 - i));
	/* fixme: Do another descent _not_ modifying to see if the values can be
	 combined without overflow. */
	/* Remove branch. */
	for( ; ; ) {
		left = trie_left(*(parent = trie->branches.data + (parent_n0 = n0)));
		if(i <= left) { /* Pre-order binary search. */
			if(!left) { twin = n0 + 1 < n1 ? trie->branches.data + n0 + 1 : 0;
				break; }
			n1 = ++n0 + left;
			trie_left_dec(parent);
		} else {
			if((n0 += left + 1) >= n1)
				{ twin = left ? trie->branches.data + n0 - left : 0; break; }
			i -= left + 1;
		}
	}
	/* Merge `parent` with `sibling` before deleting `parent`. */
	if(twin)
		/* fixme: There is nothing to guarantee this. */
		assert(trie_skip(*twin) < TRIE_SKIP_MAX - trie_skip(*parent)),
		trie_skip_set(twin, trie_skip(*twin) + 1 + trie_skip(*parent));
	memmove(parent, parent + 1, sizeof n0 * (--trie->branches.size -parent_n0));
	return 1;
}

/** Remove `key` from `trie`. @return Success or else `key` was not in `trie`.
 @order \O(`size`) @allow */
static int T_(trie_remove)(struct T_(trie) *const trie, const char *const key) {
	size_t i;
	assert(trie && key);
	return PT_(param_get)(trie, key, &i) && PT_(index_remove)(trie, i);
}
#endif

static void PT_(unused_base_coda)(void);
static void PT_(unused_base)(void) {
	T_(trie)(0); T_(trie_)(0);
	T_(trie_get)(0, 0);
	PT_(unused_base_coda)();
}
static void PT_(unused_base_coda)(void) { PT_(unused_base)(); }


#if 0

/** Recursive function used for <fn:<PT>init>. Initialise branches of `trie` up
 to `bit` with `a` to `a_size` array of sorted leaves.
 @order Speed \O(`a_size` log E(`a.length`))?; memory \O(E(`a.length`)). */
static void PT_(init_branches_r)(struct T_(trie) *const trie, size_t bit,
	const size_t a, const size_t a_size) {
	size_t b = a, b_size = a_size, half;
	size_t skip = 0;
	TrieBranch *branch;
	assert(trie && a_size && a_size <= trie->leaves.size && trie->leaves.size
		&& trie->branches.capacity >= trie->leaves.size - 1);
	if(a_size <= 1) return;
	/* Endpoints of sorted range: skip [_1_111...] or [...000_0_] don't care.
	 fixme: UINT_MAX overflow. */
	while(trie_is_bit(PT_(to_key)(trie->leaves.data[a]), bit)
		|| !trie_is_bit(PT_(to_key)(trie->leaves.data[a + a_size - 1]), bit))
		bit++, skip++;
	/* Do a binary search for the first `leaves[a+half_s]#bit == 1`. */
	while(b_size) half = b_size >> 1,
		trie_is_bit(PT_(to_key)(trie->leaves.data[b + half]), bit)
		? b_size = half : (half++, b += half, b_size -= half);
	b_size = b - a;
	/* Should have space for all branches pre-allocated in <fn:<PT>init>. */
	branch = trie_branch_array_new(&trie->branches), assert(branch);
	*branch = trie_branch(skip, b_size - 1);
	bit++;
	PT_(init_branches_r)(trie, bit, a, b_size);
	PT_(init_branches_r)(trie, bit, b, a_size - b_size);
}

/** Initializes `trie` to `a` of size `a_size`, which cannot be zero.
 @return Success. @throws[ERANGE, malloc] */
static int PT_(init)(struct T_(trie) *const trie, PT_(type) *const*const a,
	const size_t a_size) {
	PT_(leaf) *leaves;
	assert(trie && a && a_size);
	T_(trie)(trie);
	/* This will store space for all of the duplicates, as well. */
	if(!A_(array_reserve)(&trie->leaves, a_size)
		|| !trie_branch_array_reserve(&trie->branches, a_size - 1)) return 0;
	leaves = trie->leaves.data;
	memcpy(leaves, a, sizeof *a * a_size);
	trie->leaves.size = a_size;
	/* Sort, get rid of duplicates, and initialize branches, from `compare.h`. */
	qsort(leaves, a_size, sizeof *a, &PA_(vcompar));
	A_(array_unique)(&trie->leaves);
	PT_(init_branches_r)(trie, 0, 0, trie->leaves.size);
	assert(trie->branches.size + 1 == trie->leaves.size);
	return 1;
}

/** Initializes `trie` from an `array` of pointers-to-`<T>` of `array_size`.
 @return Success. @throws[realloc] @order \O(`array_size`) @allow */
static int T_(trie_from_array)(struct T_(trie) *const trie,
	PT_(type) *const*const array, const size_t array_size) {
	return assert(trie && array && array_size),
		PT_(init)(trie, array, array_size);
}

#endif


#ifdef TRIE_TO_STRING /* <!-- string */

static const char *T_(trie_to_string)(const struct T_(trie) *const trie) {
	(void)trie;
	return "foo";
}

#if defined(TRIE_TEST) /* <!-- test */
#include "../test/test_trie.h" /** \include */
#endif /* test --> */

#undef TRIE_TO_STRING
	
#endif /* string --> */

#ifdef TRIE_EXPECT_TRAIT /* <!-- trait */
#undef TRIE_EXPECT_TRAIT
#else /* trait --><!-- !trait */
#ifndef TRIE_SUBTYPE /* <!-- !sub-type */
#undef CAT
#undef CAT_
#else /* !sub-type --><!-- sub-type */
#undef TRIE_SUBTYPE
#endif /* sub-type --> */
#undef T_
#undef PT_
#undef TRIE_NAME
#undef TRIE_TYPE
#undef TRIE_KEY
#ifdef TRIE_TEST
#undef TRIE_TEST
#endif
#endif /* !trait --> */

#undef TRIE_TRAITS
