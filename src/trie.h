/** @license 2020 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Prefix Tree

 ![Example of trie.](../web/trie.png)

 A <tag:<T>trie> is a prefix tree, digital tree, or trie, implemented as an
 array of pointers-to-`T` and an index on the key string. It can be seen as a
 <Morrison, 1968 PATRICiA>: a compact
 [binary radix trie](https://en.wikipedia.org/wiki/Radix_tree), only
 storing the where the keys are different. Strings can be any encoding with a
 byte null-terminator, including
 [modified UTF-8](https://en.wikipedia.org/wiki/UTF-8#Modified_UTF-8).

 In memory, it is similar to <Bayer, McCreight, 1972 Large (B-Trees)>. Using
 <Knuth, 1998 Art 3> terminology, but instead of a B-tree of order-n nodes, it
 is a forest of non-empty complete binary trees. Thus the leaves in a tree are
 also the branching factor; the maximum is the order, fixed by compilation
 macros.

 @param[TRIE_NAME, TRIE_TYPE]
 <typedef:<PT>type> that satisfies `C` naming conventions when mangled and an
 optional returnable type that is declared, (it is used by reference only
 except if `TRIE_TEST`.) `<PT>` is private, whose names are prefixed in a
 manner to avoid collisions.

 @param[TRIE_KEY]
 A function that satisfies <typedef:<PT>key_fn>. Must be defined if and only if
 `TRIE_TYPE` is defined.

 @param[TRIE_TO_STRING]
 Defining this includes <to_string.h>, with the keys as the string.

 @param[TRIE_TEST]
 Unit testing framework <fn:<T>trie_test>, included in a separate header,
 <../test/test_trie.h>. Must be defined equal to a (random) filler function,
 satisfying <typedef:<PT>action_fn>. Requires `TRIE_TO_STRING` and that
 `NDEBUG` not be defined.

 @std C89 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
/* Suspect it will not work if `CHAR_BIT != 8`; need TI compiler? */
#include <limits.h>


#ifndef TRIE_NAME
#error Name TRIE_NAME undefined.
#endif
#if defined(TRIE_TYPE) ^ defined(TRIE_KEY)
#error TRIE_TYPE and TRIE_KEY have to be mutually defined.
#endif
#if defined(TRIE_TEST) && !defined(TRIE_TO_STRING)
#error TRIE_TEST requires TRIE_TO_STRING.
#endif


#ifndef TRIE_H /* <!-- idempotent */
#define TRIE_H
/* <http://c-faq.com/misc/bitsets.html>, except reversed for msb-first. */
#define TRIE_BITMASK(n) ((1 << CHAR_BIT - 1) >> (n) % CHAR_BIT)
#define TRIE_BITSLOT(n) ((n) / CHAR_BIT)
#define TRIE_BITTEST(a, n) ((a)[TRIE_BITSLOT(n)] & TRIE_BITMASK(n))
#define TRIE_BITDIFF(a, b, n) \
	(((a)[TRIE_BITSLOT(n)] ^ (b)[TRIE_BITSLOT(n)]) & TRIE_BITMASK(n))
/* Worst-case all-left, `(128,UCHAR_MAX]`. It's possible to go right down to 0,
 but need to edit the `TRIE_STORE*`. We could go one more, but alignment. */
#define TRIE_MAX_LEFT 6/*254*/
#define TRIE_MAX_BRANCH (TRIE_MAX_LEFT + 1)
#define TRIE_ORDER (TRIE_MAX_BRANCH + 1) /* Maximum branching factor. */
#define TRIE_BMP_SIZE(n) (((n) - 1) / CHAR_BIT + 1) /* Bitmap size in bytes. */
#define TRIE_BMP_ALIGN_SIZE(n) \
	(((TRIE_BMP_SIZE(n) - 1) / sizeof(size_t) + 1) * CHAR_BIT)
struct trie_info { unsigned char bsize, no; };
struct trie_branch { unsigned char left, skip; };
/* Stores tree numbers of different arbitrary sizes, `(n, m)`: `n` must be
 ascending from zero; `m > 0` branching factor are strictly increasing. */
#define TRIE_TREE_FIRST_X X(0, 1)
/*#define TRIE_TREE_MID_X   X(1, 4) X(2, 8) X(3, 16) X(4, 32) X(5, 64) X(6, 128)
#define TRIE_TREE_LAST_X  X(7, TRIE_ORDER)*/
#define TRIE_TREE_MID_X   X(1, 4) /* Debug: too straight. Must be monotonic. */
#define TRIE_TREE_LAST_X  X(2, TRIE_ORDER)
#define TRIE_TREE_TAIL_X TRIE_TREE_MID_X TRIE_TREE_LAST_X
#define TRIE_TREE_HEAD_X TRIE_TREE_FIRST_X TRIE_TREE_MID_X
#define TRIE_TREE_X TRIE_TREE_FIRST_X TRIE_TREE_TAIL_X
/* `C90` doesn't allow trialing commas in initializer lists. */
static const unsigned trie_tree_bsizes[] = {
#define X(n, m) m - 1,
	TRIE_TREE_HEAD_X
#undef X
#define X(n, m) m - 1
	TRIE_TREE_LAST_X
#undef X
};
static const unsigned trie_tree_count
	= sizeof trie_tree_bsizes / sizeof *trie_tree_bsizes;
/** Inserts 0 in the bit-addressed `insert` in the `bmp` with `bmp_size` bytes.
 All the other bits past the `insert` are shifted right, and one bit at the end
 is erased. */
static void trie_bmp_insert(unsigned char *const bmp, size_t bmp_size,
	const unsigned insert) {
	size_t insert_byte = insert / CHAR_BIT;
	unsigned char a = bmp[insert_byte], carry = a & 1, b = a >> 1;
	const unsigned char mask = 127 >> (insert & 7); /* Assumes `CHAR_BIT`. */
	assert(bmp && insert_byte < bmp_size);
	/* <https://graphics.stanford.edu/~seander/bithacks.html#MaskedMerge>. */
	bmp[insert_byte++] = (a ^ ((a ^ b) & mask)) & ~(mask + 1);
	while(insert_byte < bmp_size) {
		a = bmp[insert_byte];
		b = (unsigned char)(carry << 7) | (a >> 1);
		carry = a & 1;
		bmp[insert_byte++] = b;
	}
}
/** Moves and overwrites `child` with `bit_offset` to `bit_range` from
 `parent`. Both must have maximum tree, `TRIE_ORDER` leaves. `parent` has the
 moved part replaced with a single bit, '1'. */
static void trie_bmp_split(unsigned char *const parent,
	unsigned char *const child,
	const unsigned bit_offset, const unsigned bit_range) {
	assert(parent && child);
	assert(bit_range && bit_offset + bit_range <= TRIE_ORDER);
	{ /* Copy a contiguous subset of bits from `a` into the new array, `b`. */
		const unsigned a = bit_offset >> 3, a_bit = bit_offset & 7;
		unsigned b, rest;
		for(b = 0, rest = bit_range; rest > 8; b++, rest -= 8)
			child[b] = (unsigned char)(parent[a + b] << a_bit)
			| (parent[a + b + 1] >> (8 - a_bit));
		child[b] = (unsigned char)(parent[a + b] << a_bit);
		if(a + b < (bit_offset + bit_range) >> 3)
			child[b] |= (parent[a + b + 1] >> (8 - a_bit));
		child[b++] &= ~(255 >> rest);
		memset(child + b, 0, TRIE_BMP_ALIGN_SIZE(TRIE_ORDER) - b);
	}
	{ /* Replace copied bits from `a` with '1'. */
		const unsigned a = bit_offset >> 3, a_bit = bit_offset & 7;
		parent[a] |= 128 >> a_bit;
	}
	{ /* Move bits back in `a`. */
		unsigned a0 = (bit_offset + 1) >> 3, a1 = (bit_offset + bit_range) >> 3;
		const unsigned a0_bit = (bit_offset + 1) & 7,
			a1_bit = (bit_offset + bit_range) & 7;
		assert(a0 <= TRIE_BMP_SIZE(TRIE_ORDER)
			&& a1 <= TRIE_BMP_SIZE(TRIE_ORDER));
		if(a1 == TRIE_BMP_SIZE(TRIE_ORDER)) { /* On the trailing edge. */
			assert(!a1_bit); /* Extreme right. */
			if(a0 == TRIE_BMP_SIZE(TRIE_ORDER)) assert(!a0_bit);
			else parent[a0++] &= 255 << 8-a0_bit;
		} else if(a1_bit < a0_bit) { /* Inversion of shift. */
			const unsigned shift = a0_bit - a1_bit;
			assert(a0 < a1);
			{
				const unsigned char bmp_a_a0 = parent[a0],
					bmp_a_a1 = parent[a1] >> shift,
					mask = 255 >> a0_bit;
				parent[a0] = bmp_a_a0 ^ ((bmp_a_a0 ^ bmp_a_a1) & mask);
			}
			while(++a0, ++a1 < TRIE_BMP_SIZE(TRIE_ORDER)) parent[a0]
				= (unsigned char)(parent[a1 - 1] <<8-shift | parent[a1] >>shift);
			parent[a0++] = (unsigned char)(parent[a1 - 1] << 8-shift);
		} else { /* Shift right or zero. */
			const unsigned shift = a1_bit - a0_bit;
			assert(a0 <= a1);
			{
				const unsigned char bmp_a_a0 = parent[a0],
					bmp_a_a1 = (unsigned char)(parent[a1] << shift),
					mask = 255 >> a0_bit;
				parent[a0] = bmp_a_a0 ^ ((bmp_a_a0 ^ bmp_a_a1) & mask);
			}
			while(++a0, ++a1 < TRIE_BMP_SIZE(TRIE_ORDER))
				parent[a0 - 1] |= parent[a1] >> 8-shift,
				parent[a0] = (unsigned char)(parent[a1] << shift);
		}
		memset(parent + a0, 0, TRIE_BMP_SIZE(TRIE_ORDER) - a0);
	}
}
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

#ifndef TRIE_TYPE /* <!-- !type */
/** Default values for string: uses `a` as the key. */
static char *PT_(raw)(char **a) { return assert(a), *a; }
#define TRIE_TYPE char
#define TRIE_KEY &PT_(raw)
#endif /* !type --> */

/** Declared type of the trie; `char` default. */
typedef TRIE_TYPE PT_(type);

/** Pointers to generic trees stored in memory, and part of the B-forest.
 Points to a non-empty semi-implicit complete binary tree of a
 fixed-maximum-size; reading `key.tree` will tell which tree it is. */
union PT_(any_tree) {
	struct trie_info *info;
#define X(n, m) struct PT_(tree##n) *t##n;
	TRIE_TREE_X
#undef X
};

/** A leaf is either data or another child tree; the `children` of
 <tag:<PT>tree> is a bitmap that tells which. */
union PT_(leaf) { PT_(type) *data; union PT_(any_tree) child; };

/* Different stores of trees, designed to fit alignment boundaries. */
struct PT_(tree0) { struct trie_info info; unsigned char children[6];
	union PT_(leaf) leaves[1]; };
#define X(n, m) struct PT_(tree##n) { struct trie_info info; \
	struct trie_branch branches[m - 1]; \
	unsigned char children[TRIE_BMP_ALIGN_SIZE(m)]; \
	union PT_(leaf) leaves[m]; };
TRIE_TREE_TAIL_X
#undef X

static const unsigned PT_(tree_sizes)[] = {
#define X(n, m) sizeof(struct PT_(tree##n)),
	TRIE_TREE_HEAD_X
#undef X
#define X(n, m) sizeof(struct PT_(tree##n))
	TRIE_TREE_LAST_X
#undef X
};

/** A working tree of any size extracted from different-width storage by
 <fn:<PT>extract>. */
struct PT_(tree) { unsigned bsize, no; struct trie_branch *branches;
	unsigned char *children; union PT_(leaf) *leaves; };

/** To initialize it to an idle state, see <fn:<T>trie>, `TRIE_IDLE`, `{0}`
 (`C99`), or being `static`.

 ![States.](../web/states.png) */
struct T_(trie) { union PT_(any_tree) root; };
#ifndef TRIE_IDLE /* <!-- !zero */
#define TRIE_IDLE { { 0 } }
#endif /* !zero --> */

/** Responsible for picking out the null-terminated string. Modifying the
 string while in any trie causes the trie to go into an undefined state. */
typedef const char *(*PT_(key_fn))(const PT_(type) *);

/* Check that `TRIE_KEY` is a function satisfying <typedef:<PT>key_fn>. */
static PT_(key_fn) PT_(to_key) = (TRIE_KEY);

/** @return Fills `tree` for the kind of tree storage in `store`. */
static void PT_(extract)(const union PT_(any_tree) store,
	struct PT_(tree) *const tree) {
	assert(store.info && tree);
	tree->bsize = store.info->bsize;
	switch(tree->no = store.info->no) {
	case 0: /* Special case where there are no branches. */
		tree->branches = 0; tree->children = store.t0->children;
		tree->leaves = store.t0->leaves; break;
#define X(n, m) case n: tree->branches = store.t##n->branches; tree->children \
	= store.t##n->children; tree->leaves = store.t##n->leaves; break;
		TRIE_TREE_TAIL_X
#undef X
	default: assert(0);
	}
}

/** @return Looks at only the index of `trie` for potential `key` matches,
 otherwise `key` is definitely not in `trie`. @order \O(`key.length`) */
static PT_(type) *PT_(match)(const struct T_(trie) *const trie,
	const char *const key) {
	union PT_(any_tree) store = trie->root;
	struct PT_(tree) tree;
	const struct trie_branch *branch;
	size_t bit; /* `bit \in key`.  */
	struct { unsigned br0, br1, lf; } in_tree;
	struct { size_t cur, next; } byte; /* `key` null checks. */
	assert(trie && key);
	if(!store.info) return 0; /* Idle. */
	for(byte.cur = 0, bit = 0; ; ) { /* Forest. */
		PT_(extract)(store, &tree);
		in_tree.br0 = 0, in_tree.br1 = tree.bsize, in_tree.lf = 0;
		while(in_tree.br0 < in_tree.br1) { /* Tree. */
			branch = tree.branches + in_tree.br0;
			for(byte.next = (bit += branch->skip) / CHAR_BIT;
				byte.cur < byte.next; byte.cur++)
				if(key[byte.cur] == '\0') return 0; /* Too short. */
			if(!TRIE_BITTEST(key, bit))
				in_tree.br1 = ++in_tree.br0 + branch->left;
			else
				in_tree.br0 += branch->left + 1, in_tree.lf += branch->left + 1;
			bit++;
		}
		if(!TRIE_BITTEST(tree.children, in_tree.lf)) break;
		store = tree.leaves[in_tree.lf].child;
	};
	return tree.leaves[in_tree.lf].data;
}

/** @return Exact match for `key` in `trie` or null. (fixme: private?) */
static PT_(type) *PT_(get)(const struct T_(trie) *const trie,
	const char *const key) {
	PT_(type) *n;
	return (n = PT_(match)(trie, key)) && !strcmp(PT_(to_key)(n), key) ? n : 0;
}

/** Expand `any` to ensure that it has one more unused capacity when the size
 is not the maximum. @return Potentially a re-allocated tree.
 @throws[realloc] @fixme Cached any. */
static union PT_(any_tree) PT_(expand)(const union PT_(any_tree) any) {
	struct PT_(tree) tree0, tree1;
	union PT_(any_tree) larger;
	size_t links0, links1;
	assert(any.info);
	PT_(extract)(any, &tree0);
	/*printf("expand: bsize %u, width%u: %u\n",
		tree0.bsize, tree0.tree, trie_tree_bsizes[tree0.tree]);*/
	assert(tree0.bsize < TRIE_MAX_BRANCH);
	if(tree0.bsize < trie_tree_bsizes[tree0.no]) return any;
	assert(tree0.bsize == trie_tree_bsizes[tree0.no]
		&& tree0.no + 1 < trie_tree_count);
	/* Augment the allocation. */
	if(!(larger.info = realloc(any.info, PT_(tree_sizes)[tree0.no + 1])))
		{ if(!errno) errno = ERANGE; return larger; }
	PT_(extract)(larger, &tree0); /* The address may have changed. */
	/*printf("expand: #%p width%u %luB -> #%p store%u %luB\n", (void *)any.info,
		tree0.no, (unsigned long)PT_(tree_sizes)[tree0.no],
		(void *)larger.info, tree0.no + 1,
		(unsigned long)PT_(tree_sizes)[tree0.no + 1]);*/
	/* Augment the allocation size. */
	larger.info->no++;
	PT_(extract)(larger, &tree1);
	assert(tree0.bsize == tree1.bsize
		&& (!tree0.branches || tree0.branches == tree1.branches)
		&& tree0.children <= tree1.children
		&& tree0.leaves <= tree1.leaves);
	/* Careful to go backwards because we don't want to overwrite. */
	memmove(tree1.leaves, tree0.leaves,
		sizeof *tree0.leaves * (tree0.bsize + 1));
	links0 = TRIE_BMP_SIZE(tree0.bsize + 1);
	links1 = TRIE_BMP_SIZE(trie_tree_bsizes[tree0.no + 1] + 1);
	/*printf("expand: moving from %luB to %luB\n", links0, links1);*/
	memmove(tree1.children, tree0.children, links0);
	memset(tree0.children + links0, 0, links1 - links0);
	return larger;
}

/** @return Success splitting the tree `any`. Must be full. */
static int PT_(split)(union PT_(any_tree) any) {
	struct PT_(tree) tree;
	struct { unsigned br0, br1, lf; } in_tree;
	struct { unsigned br0, br1; } in_write;
	struct { int opt, left, right; } balance; /* Minimize this. */
#define X(n, m) struct PT_(tree##n) *split;
	TRIE_TREE_LAST_X
#undef X
	assert(any.info);
	PT_(extract)(any, &tree);
	/*printf("split: bsize %u, tree%u: %u\n",
		tree.bsize, tree.no, trie_tree_bsizes[tree.no]);*/
	assert(tree.bsize == TRIE_MAX_BRANCH
		&& tree.no == trie_tree_count - 1);
	/*{
		unsigned b;
		printf("branches: {"); for(b = 0; b < TRIE_MAX_BRANCH; b++) printf("%s%u", b ? ", " : "", tree.branches[b].left); printf("}\n");
	}*/
	in_tree.br0 = 0, in_tree.br1 = tree.bsize, in_tree.lf = 0;
	for(balance.opt = TRIE_MAX_BRANCH; ; ) { /* Descend branches of tree. */
		const struct trie_branch *const branch = tree.branches + in_tree.br0;
		assert(in_tree.br0 < in_tree.br1);
		/* See how these splits do compared to the optimum. */
		balance.left  = (int)(TRIE_MAX_BRANCH - 2 * branch->left);
		balance.right = (int)(TRIE_MAX_BRANCH - 2
			* (in_tree.br1 - in_tree.br0 - 1 - branch->left));
		/*printf("[%u, %u]:%u, balance %d{%d,%d}\n",
			in_tree.br0, in_tree.br1, in_tree.lf,
			balance.opt, balance.left, balance.right);*/
		if(abs(balance.opt) < abs(balance.left)) {
			if(abs(balance.opt) < abs(balance.right)) break;
			else goto right;
		} else {
			if(abs(balance.left) < abs(balance.right)) goto left;
			else goto right;
		}
left:
		balance.opt = balance.left;
		in_tree.br1 = ++in_tree.br0 + branch->left;
		continue;
right:
		balance.opt = balance.right;
		in_tree.br0 += branch->left + 1, in_tree.lf += branch->left + 1;
	}
	assert(in_tree.br0 != in_tree.br1 && in_tree.lf <= tree.bsize);
	/*printf("split on [%u,%u]:%u.\n", in_tree.br0, in_tree.br1, in_tree.lf);*/
	/* Split off a new tree. */
	if(!(split = malloc(sizeof *split)))
		{ if(!errno) errno = ERANGE; return 0; }
	split->info.bsize = 0;
#define X(n, m) split->info.no = n;
	TRIE_TREE_LAST_X
#undef X
	/*printf("split: new split %p.\n", (void *)split);*/
	/* Re-descend and decrement branches in preparation to split. */
	in_write.br0 = 0, in_write.br1 = tree.bsize;
	while(in_write.br0 < in_tree.br0) {
		struct trie_branch *const branch = tree.branches + in_write.br0;
		if(in_tree.br0 <= in_write.br0 + branch->left) {
			in_write.br1 = ++in_write.br0 + branch->left;
			branch->left -= in_tree.br1 - in_tree.br0;
		} else {
			in_write.br0 += branch->left + 1;
		}
	}
	/*{
		unsigned b;
		printf("branches: {"); for(b = 0; b < TRIE_MAX_BRANCH; b++) printf("%s%u", b ? ", " : "", tree.branches[b].left); printf("}\n");
	}*/
	/* Move leaves. */
	memcpy(split->leaves, tree.leaves + in_tree.lf,
		sizeof *tree.leaves * (in_tree.br1 - in_tree.br0 + 1));
	memmove(tree.leaves + in_tree.lf + 1,
		tree.leaves + in_tree.lf + (in_tree.br1 - in_tree.br0 + 1),
		sizeof *tree.leaves * (TRIE_MAX_BRANCH - in_tree.lf
		- in_tree.br1 + in_tree.br0));
#define X(n, m) tree.leaves[in_tree.lf].child.t##n = split;
	TRIE_TREE_LAST_X
#undef X
	/* Move children bitmap. */
	trie_bmp_split(tree.children, split->children,
		in_tree.lf, in_tree.br1 - in_tree.br0 + 1);
	/*{
		size_t i;
		printf("Old:");
		for(i = 0; i < TRIE_ORDER; i++)
			printf("%u", !!TRIE_BITTEST(tree.children, i));
		printf("\n");
		printf("New:");
		for(i = 0; i < TRIE_ORDER; i++)
		printf("%u", !!TRIE_BITTEST(split->children, i));
		printf("\n");
	}*/
	/* Move branches. */
	memcpy(split->branches, tree.branches + in_tree.br0,
		sizeof *tree.branches * (in_tree.br1 - in_tree.br0));
	memmove(tree.branches + in_tree.br0, tree.branches
		+ in_tree.br1, sizeof *tree.branches * (TRIE_MAX_BRANCH - in_tree.br1));
	/* Move branch size. *//* tree.bsize -= in_tree.br1 - in_tree.br0; */
	any.info->bsize -= in_tree.br1 - in_tree.br0;
	split->info.bsize += in_tree.br1 - in_tree.br0;
	return 1;
}

/** @return The leftmost key `lf` of key `any`. */
static const char *PT_(sample)(union PT_(any_tree) any, unsigned lf) {
	struct PT_(tree) tree;
	assert(any.info);
	while(PT_(extract)(any, &tree), TRIE_BITTEST(tree.children, lf))
		any = tree.leaves[lf].child, lf = 0;
	return PT_(to_key)(tree.leaves[lf].data);
}

/** Adds `x` to `trie`, which must not be in `trie`.
 @return Success. @throw[malloc, ERANGE]
 @throw[ERANGE] There is too many bytes similar for the data-type. */
static int PT_(add_unique)(struct T_(trie) *const trie, PT_(type) *const x) {
	const char *const x_key = PT_(to_key)(x);
	struct { size_t x, x0, x1; } in_bit;
	struct { union PT_(any_tree) *ref, any; size_t start_bit; } in_forest;
	struct { unsigned br0, br1, lf; } in_tree;
	struct PT_(tree) tree;
	struct trie_branch *branch;
	union PT_(leaf) *leaf;
	const char *sample;
	int is_write = 0, is_right = 0, is_split = 0;

	assert(trie && x);
	/*printf("add: %s -> %s.\n", x_key, T_(trie_to_string)(trie));*/
	if(!trie->root.info) { /* Empty special case. */
		struct PT_(tree0) *const t0 = malloc(sizeof *t0);
		if(!t0) { if(!errno) errno = ERANGE; return 0; }
		t0->info.bsize = 0, t0->info.no = 0, t0->children[0] = 0,
			t0->leaves[0].data = x;
		trie->root.t0 = t0;
		return 1;
	}
	/* B-forest. */
	for(in_bit.x = 0, in_forest.any = *(in_forest.ref = &trie->root); ; ) {
tree:
		in_forest.start_bit = in_bit.x;
		sample = PT_(sample)(in_forest.any, 0);
		PT_(extract)(in_forest.any, &tree);
		in_bit.x0 = in_bit.x;
		in_tree.br0 = 0, in_tree.br1 = tree.bsize, in_tree.lf = 0;
		while(in_tree.br0 < in_tree.br1) { /* Descend branches of tree. */
			branch = tree.branches + in_tree.br0;
			for(in_bit.x1 = in_bit.x + branch->skip; in_bit.x < in_bit.x1;
				in_bit.x++) if(TRIE_BITDIFF(x_key, sample, in_bit.x)) goto leaf;
			if(!TRIE_BITTEST(x_key, in_bit.x)) {
				in_tree.br1 = ++in_tree.br0 + branch->left;
				if(is_write) branch->left++;
			} else {
				in_tree.br0 += branch->left + 1;
				in_tree.lf  += branch->left + 1;
				sample = PT_(sample)(in_forest.any, in_tree.lf);
			}
			in_bit.x0 = in_bit.x1, in_bit.x++;
		}
		assert(in_tree.br0 == in_tree.br1 && in_tree.lf <= tree.bsize);
		if(!TRIE_BITTEST(tree.children, in_tree.lf)) break;
		in_forest.any = *(in_forest.ref = &tree.leaves[in_tree.lf].child);
	}
	/* Got to the leaves. */
	in_bit.x1 = in_bit.x + UCHAR_MAX;
	while(!TRIE_BITDIFF(x_key, sample, in_bit.x))
		if(++in_bit.x > in_bit.x1) return errno = ERANGE, 0;

leaf:
	if(TRIE_BITTEST(x_key, in_bit.x))
		is_right = 1, in_tree.lf += in_tree.br1 - in_tree.br0 + 1;
	/*printf("add: %s, at leaf %u bit %lu.\n", is_right ? "right" : "left",
		in_tree.lf, in_bit.x);*/
	assert(in_tree.lf <= tree.bsize + 1u);

	if(is_write) goto insert;
	assert(tree.bsize <= TRIE_MAX_BRANCH);
	if(tree.bsize == TRIE_MAX_BRANCH) {
		/* If the tree is full, split it, and go again. */
		if(!PT_(split)(in_forest.any)) return 0;
		/*printf("add: split %s.\n", T_(trie_to_string)(trie));*/
		assert(!is_split && (is_split = 1));
	} else {
		/* Go back and modify the tree for one extra branch/leaf pair;
		 if unneeded, this will return immediately. */
		union PT_(any_tree) store = PT_(expand)(in_forest.any);
		if(!store.info) return 0;
		/*printf("add: expand %s.\n", T_(trie_to_string)(trie));*/
		*in_forest.ref = in_forest.any = store;
		is_write = 1;
	}
	in_bit.x = in_forest.start_bit;
	goto tree;

insert:
	leaf = tree.leaves + in_tree.lf;
	memmove(leaf + 1, leaf, sizeof *leaf * (tree.bsize + 1 - in_tree.lf));
	leaf->data = x;
	branch = tree.branches + in_tree.br0;
	if(in_tree.br0 != in_tree.br1) { /* Split `skip` with the existing branch. */
		assert(in_bit.x0 <= in_bit.x
			&& in_bit.x + !in_tree.br0 <= in_bit.x0 + branch->skip);
		branch->skip -= in_bit.x - in_bit.x0 + !in_tree.br0;
	}
	trie_bmp_insert(tree.children, tree.bsize + 2, in_tree.lf); /* Test! */
	memmove(branch + 1, branch, sizeof *branch * (tree.bsize - in_tree.br0));
	assert(in_tree.br1 - in_tree.br0 < 256
		&& in_bit.x >= in_bit.x0 + !!in_tree.br0
		&& in_bit.x - in_bit.x0 - !!in_tree.br0 < 256);
	branch->left = is_right ? (unsigned char)(in_tree.br1 - in_tree.br0) : 0;
	branch->skip = (unsigned char)(in_bit.x - in_bit.x0 - !!in_tree.br0);
	in_forest.any.info->bsize++;

	return 1;
}

/** Frees all `any` and it's children. */
static void PT_(clear_tree)(const union PT_(any_tree) any) {
	struct PT_(tree) tree;
	unsigned i;
	assert(any.info);
	PT_(extract)(any, &tree);
	for(i = 0; i <= tree.bsize; i++) if(TRIE_BITTEST(tree.children, i))
		PT_(clear_tree)(tree.leaves[i].child);
	free(any.info);
}

/* <!-- iterate interface */

/** Contains all iteration parameters. */
struct PT_(iterator);
struct PT_(iterator) { const struct T_(trie) *trie;
	union PT_(any_tree) cur; unsigned i, unused; };

/** Loads `trie` into `it`. @implements begin */
static void PT_(begin)(struct PT_(iterator) *const it,
	const struct T_(trie) *const trie)
	{ assert(it && trie); it->trie = trie; it->cur.info = 0; }

/** Advances `it`. @implements next */
static PT_(type) *PT_(next)(struct PT_(iterator) *const it) {
	struct PT_(tree) tree;
	assert(it && it->trie);
	if(!it->cur.info) { /* Starting. Descend to first leaf. */
		if(!(it->cur = it->trie->root).info) return 0; /* Empty. */
		while(PT_(extract)(it->cur, &tree), TRIE_BITTEST(tree.children, 0))
			it->cur = tree.leaves[0].child;
		it->i = 0;
	} else { /* Iterating. */
		PT_(extract)(it->cur, &tree);
		if(++it->i > tree.bsize) { /* Finished tree; restart down next. */
			/* Any one from the tree will do; we just need to match the path.
			 The last one is definitely a data leaf. */
			const char *key = PT_(to_key)(tree.leaves[it->i - 1].data);
			const union PT_(any_tree) store1 = it->cur;
			union PT_(any_tree) store2 = it->trie->root;
			struct PT_(tree) tree2;
			size_t bit2 = 0;
			const struct trie_branch *branch2;
			struct { unsigned br0, br1, lf; } in_tree2;
			assert(key && store2.info);
			/*printf("next: got stuck on %s.\n",
				PT_(to_key)(tree.leaves[it->i - 1].data));*/
			for(it->cur.info = 0, it->i = 0; ; ) {
				if(store1.info == store2.info) break; /* Reached the tree. */
				PT_(extract)(store2, &tree2);
				in_tree2.br0 = 0, in_tree2.br1 = tree2.bsize, in_tree2.lf = 0;
				while(in_tree2.br0 < in_tree2.br1) {
					branch2 = tree2.branches + in_tree2.br0;
					bit2 += branch2->skip;
					if(!TRIE_BITTEST(key, bit2))
						in_tree2.br1 = ++in_tree2.br0 + branch2->left;
					else
						in_tree2.br0 += branch2->left + 1,
						in_tree2.lf += branch2->left + 1;
					bit2++;
				}
				/* We found a continuation. */
				if(in_tree2.lf < tree2.bsize)
					it->cur.info = store2.info, it->i = in_tree2.lf + 1/*,
					printf("next: continues in tree %p, leaf %u.\n",
						(void *)store2.key, it->i)*/;
				assert(TRIE_BITTEST(tree2.children, in_tree2.lf));
				store2 = tree2.leaves[in_tree2.lf].child;
			}
			if(!it->cur.info) { assert(!it->i); return 0; } /* No more. */
			PT_(extract)(it->cur, &tree); /* Update tree. */
		} /*else {
			printf("next: tree %p, leaf %u, bsize %u.\n",
				(void *)it->cur.key, it->i, it->cur.key->bsize);
		}*/
		while(TRIE_BITTEST(tree.children, it->i))
			PT_(extract)(it->cur = tree.leaves[it->i].child, &tree), it->i = 0;
	}
	return tree.leaves[it->i].data;
}

/* iterate --> */



/** Initialises `trie` to idle. @order \Theta(1) @allow */
static void T_(trie)(struct T_(trie) *const trie)
	{ assert(trie); trie->root.info = 0; }

/** Returns an initialised `trie` to idle. @allow */
static void T_(trie_)(struct T_(trie) *const trie) {
	assert(trie);
	if(trie->root.info) PT_(clear_tree)(trie->root), T_(trie)(trie);
}

/** @return The <typedef:<PT>type> with `key` in `trie` or null no such item
 exists. @order \O(|`key`|), <Thareja 2011, Data>. @allow */
static PT_(type) *T_(trie_get)(const struct T_(trie) *const trie,
	const char *const key) { return assert(trie && key), PT_(get)(trie, key); }

/** @return If `x` is already in `trie`, returns false, otherwise success.
 @throws[realloc, ERANGE] @allow */
static int T_(trie_add)(struct T_(trie) *const trie, PT_(type) *const x) {
	return assert(trie && x),
		PT_(get)(trie, PT_(to_key)(x)) ? /*printf("add: %s already in trie.\n",
		PT_(to_key)(x)),*/ 0 : PT_(add_unique)(trie, x);
	/*return assert(trie && datum), PT_(put)(trie, datum, 0, &PT_(false_replace));*/
}

/* Define these for traits. */
#define BOX_ PT_
#define BOX_CONTAINER struct T_(trie)
#define BOX_CONTENTS PT_(type)

#ifdef TRIE_TO_STRING /* <!-- str */
/** Uses the natural `a` -> `z` that is defined by `TRIE_KEY`. */
static void PT_(to_string)(const PT_(type) *const a, char (*const z)[12])
	{ assert(a && z); sprintf(*z, "%.11s", PT_(to_key)(a)); }
#define Z_(n) CAT(T_(trie), n)
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
	T_(trie)(0); T_(trie_)(0); T_(trie_get)(0, 0); T_(trie_add)(0, 0);
	PT_(begin)(0, 0); PT_(next)(0); PT_(unused_base_coda)();
}
static void PT_(unused_base_coda)(void) { PT_(unused_base)(); }

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
