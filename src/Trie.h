/** @license 2020 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle String Trie

 ![Example of trie.](../web/trie.png)

 An <tag:<N>Trie> is a trie of byte-strings ended with `NUL`, compatible with
 any byte-encoding with a null-terminator; in particular, `C` strings,
 including [modified UTF-8](https://en.wikipedia.org/wiki/UTF-8#Modified_UTF-8).

 Internally, it is an `<<PN>TrieNode>Array`, thus `Array.h` must be present. It
 resembles a binary [radix trie](https://en.wikipedia.org/wiki/Radix_tree) or
 [Morrison 1968, PATRICiA], except with two notable differences: the index does
 not store data on the string, only the positions where the strings are
 different. Also, the trie is stored in a semi-implicit array. It is optimised
 for lookup and not insertion or deletion, which have \O(`n`) time
 complexities; although `memmove` has a low constant factor.

 `<N>Trie` is not synchronised. Errors are returned with `errno`. The
 parameters are `#define` preprocessor macros, and are all undefined at the end
 of the file for convenience. `assert.h` is used; to stop assertions, use
 `#define NDEBUG` before inclusion.

 @param[TRIE_NAME, TRIE_TYPE]
 `<N>` that satisfies `C` naming conventions when mangled and an optional
 returnable type that is declared, (it is used by reference only.) `<PN>` is
 private, whose names are prefixed in a manner to avoid collisions; any should
 be re-defined prior to use elsewhere.

 @param[TRIE_KEY]
 A function that satisfies <typedef:<PN>Key>. Must be defined if and only if
 `TRIE_TYPE` is defined.

 @param[TRIE_TEST]
 Unit testing framework <fn:<N>TreeTest>, included in a separate header,
 <../test/TreeTest.h>. Must be defined equal to a (random) filler function,
 satisfying <typedef:<PN>Action>. Requires `TRIE_TO_STRING` and not `NDEBUG`.

 @fixme Have a `<N>TrieModifyKey` that's faster then re-inserting it.
 @depend [Array.h](../../Array/)
 @std C89
 @cf [Array](https://github.com/neil-edelman/Array)
 @cf [List](https://github.com/neil-edelman/List)
 @cf [Orcish](https://github.com/neil-edelman/Orcish)
 @cf [Pool](https://github.com/neil-edelman/Pool)
 @cf [Set](https://github.com/neil-edelman/Set) */

#include <string.h> /* size_t memmove strcmp */
#include <limits.h> /* UINT_MAX */


#ifndef TRIE_H /* <!-- idempotent */
#define TRIE_H

/* The deciding bit on the string, and how many branches (internal nodes) are
 in the left subtree. As an array, it encodes a pre-order full binary tree
 semi-implicitly: `left` children are immediately following, right children are
 the rest. The number of leaves is one more, (with a special case for empty,)
 and stored in a separate array. */
struct TrieBranch { unsigned bit, left; };

/* Define the struct used in all <tag:<N>Trie>. */
#define ARRAY_NAME TrieBranch
#define ARRAY_TYPE struct TrieBranch
#define ARRAY_CHILD
#include "Array.h"

/** Woefully unoptimised. Does it matter? */
static int trie_strcmp_bit(const char *const a, const char *const b,
	const unsigned bit) {
	const unsigned byte = bit >> 3, mask = 128 >> (bit & 7);
	return (a[byte] & mask) - (b[byte] & mask);
}

/** Not going to be bothered to make it 0/1. */
static unsigned trie_is_bit(const char *const a, const unsigned bit) {
	const unsigned byte = bit >> 3, mask = 128 >> (bit & 7);
	return a[byte] & mask;
}

#endif /* idempotent --> */


/* Check defines. */
#ifndef TRIE_NAME
#error Generic name TRIE_NAME undefined.
#endif
#if (defined(TRIE_TYPE) && !defined(TRIE_KEY)) \
	|| (!defined(TRIE_TYPE) && defined(TRIE_KEY))
#error TRIE_TYPE and TRIE_KEY have to be defined of not.
#endif
#if defined(N_) || defined(PN_)
#error N_ and PN_ cannot be defined.
#endif
#ifndef TRIE_TYPE /* <!-- !type */
#define TRIE_TYPE const char
#define TRIE_KEY &trie_raw
#ifndef TRIE_RAW /* <!-- !raw */
#define TRIE_RAW /* Idempotent. */
static const char *trie_raw(const char *const key) { return key; }
#endif /* !raw --> */
#endif /* !type --> */

/* <Kernighan and Ritchie, 1988, p. 231>. */
#ifdef CAT
#undef CAT
#endif
#ifdef CAT_
#undef CAT_
#endif
#ifdef PCAT
#undef PCAT
#endif
#ifdef PCAT_
#undef PCAT_
#endif
#define CAT_(x, y) x ## y
#define CAT(x, y) CAT_(x, y)
#define PCAT_(x, y) x ## _ ## y
#define PCAT(x, y) PCAT_(x, y)
#define N_(thing) CAT(TRIE_NAME, thing)
#define PN_(thing) PCAT(trie, PCAT(TRIE_NAME, thing))


/** A valid tag type set by `TRIE_TYPE`; defaults to `const char`. */
typedef TRIE_TYPE PN_(Type);

/** Responsible for picking out the null-terminated string. */
typedef const char *(*PN_(Key))(PN_(Type) *);

/* Check that `TRIE_KEY` is a function implementing <typedef:<PN>Key>. */
static const PN_(Key) PN_(to_key) = (TRIE_KEY);

/** A bi-predicate; returns true if the `replace` replaces the `original`; used
 in <fn:<N>TriePolicyPut>. */
typedef int (*PN_(Replace))(PN_(Type) *original, PN_(Type) *replace);

/* Gets rid of the internal confusing double-pointers. */
typedef PN_(Type) *PN_(Leaf);

/* Trie leaf array is just an array sorted by key. */
#define ARRAY_NAME PN_(Leaf)
#define ARRAY_TYPE PN_(Leaf)
#define ARRAY_CHILD
#include "Array.h"

#define PT_(thing) PCAT(array, PCAT(PN_(Leaf), thing))

/* Private code follows a convention that in `branches` (internal nodes) the
 subscripts begin by `n` and in `leaves` (external nodes) it begins with `i`. */

/** To initialise it to an idle state, see <fn:<N>Tree>, `TRIE_IDLE`, `{0}`
 (`C99`), or being `static`. Internally, it is an array of <tag:<PN>TrieNode>.
 If size is zero, it's empty; otherwise it has `2 (nodes - 1) + 1` elements. If
 size is one, it's a leaf node, otherwise it has an internal node as it's first
 element.

 ![States.](../web/states.png) */
struct N_(Trie);
struct N_(Trie) {
	struct TrieBranchArray branches;
	struct PN_(LeafArray) leaves;
};
#ifndef TRIE_IDLE /* <!-- !zero */
#define TRIE_IDLE { { 0, 0, 0, 0 }, { 0, 0, 0, 0 } }
#endif /* !zero --> */

static void PN_(print)(const struct N_(Trie) *const trie) {
	size_t i, n;
	printf("__Trie__\n");
	if(!trie) { printf("null"); return; }
	printf(" > leaves: ");
	for(i = 0; i < trie->leaves.size; i++)
		printf("%s%s", i ? ", " : "", PN_(to_key)(trie->leaves.data[i]));
	printf(";\n"
		" > branches: ");
	for(n = 0; n < trie->branches.size; n++)
		printf("%s%u:%u", n ? ", " : "", trie->branches.data[n].bit,
		trie->branches.data[n].left);
	printf(".\n");
}

/** Initialises `trie`. */
static void PN_(trie)(struct N_(Trie) *const trie) {
	assert(trie);
	array_TrieBranch_array(&trie->branches), PT_(array)(&trie->leaves);
}

/** Destructor of `trie`. */
static void PN_(trie_)(struct N_(Trie) *const trie) {
	assert(trie);
	free(trie->branches.data), free(trie->leaves.data), PN_(trie)(trie);
}

/** Add `data` to `trie`. This assumes that the key of `data` is not the same
 as any in `trie`, so make sure before calling this or else it may crash,
 (_viz_, it doesn't do `NUL` checks.)
 @order O(`nodes`) */
static int PN_(add)(struct N_(Trie) *const trie, PN_(Type) *const data) {
	struct TrieBranch *branch;
	PN_(Leaf) *leaf;
	const size_t leaf_size = trie->leaves.size, branch_size = leaf_size - 1;
	size_t n, n1, i;
	const char *const data_key = PN_(to_key)(data), *n_key;
	unsigned bit, n_bit, n_left;
	int cmp;

	assert(trie && data && n1 < (size_t)-2);

	printf("Adding %s to ", data_key), PN_(print)(trie);

	/* Empty short circuit; add one entry to `leaves`. */
	if(!leaf_size) {
		assert(!trie->branches.size);
		return (leaf = PT_(new)(&trie->leaves, 0)) ? *leaf = data, 1 : 0;
	}

	/* Non-empty. */
	assert(leaf_size == branch_size + 1); /* Waste `size_t`. */
	if(!PT_(reserve)(&trie->leaves, 1, 0)
		|| !array_TrieBranch_reserve(&trie->branches, 1, 0)) return 0;

	cmp = 0;
	bit = 0;
	i = 0, n = 0, n1 = branch_size;
	/* Want `branch` and `br_key` before testing. */
	while(branch = trie->branches.data + n, n_key = trie->leaves.data[i],
		n < n1) {
		for(n_bit = branch->bit; bit < n_bit; bit++)
			if((cmp = trie_strcmp_bit(data_key, n_key, bit)) != 0) goto insert;
		/* Follow the left or right branch; update the left. */
		if(!trie_is_bit(n_key, bit)) n1 = n++ + ++branch->left;
		else n += branch->left + 1, i += branch->left + 1;
	}
	while((cmp = trie_strcmp_bit(data_key, n_key, bit)) == 0) bit++;
	printf("leaf cmp(%s, %s) = %u.\n", data_key, n_key, bit);

insert:
	if(cmp > 0) i++;
	n_left = branch->left;
	printf(" -> %s to leaf %lu.\n", data_key, i);
	assert(n <= n1 && n1 <= trie->branches.size && n_key);

	/* Insert a leaf. */
	leaf = trie->leaves.data + i;
	memmove(leaf + 1, leaf, sizeof *leaf * (leaf_size - i));
	*leaf = data;
	trie->leaves.size++;

	printf(" -> %d:%d to branch %lu.\n", bit, n_left, n);
	/* Insert a branch. */
	branch = trie->branches.data + n;
	memmove(branch + 1, branch, sizeof *branch * (branch_size - n));
	branch->bit = bit;
	branch->left = n_left;
	trie->branches.size++;

	return 1;
}

/** Doesn't look at the underlying data, but goes through only the index trie.
 @return `TrieNode` leaf that potentially matches `str` or null if it
 definitely is not in the trie. */
static PN_(Leaf) *PN_(match)(const struct N_(Trie) *const trie,
	const char *const str) {
	size_t n0 = 0, n1 = trie->leaves.size, i = 0;
	struct TrieBranch *n0_branch;
	unsigned n0_byte, str_byte = 0;
	assert(trie);
	if(n1-- <= 1) return n1 ? trie->leaves.data : 0;
	assert(n1 == trie->branches.size);
	while(n0 < n1) {
		n0_branch = trie->branches.data + n0;

		/* Don't go farther than the string. */
		for(n0_byte = n0_branch->bit >> 3; str_byte < n0_byte; str_byte++)
			if(str[str_byte] == '\0') return 0;

		/* Follow the branch left/right. */
		if(!trie_is_bit(str, n0_branch->bit)) n1 = ++n0 + n0_branch->left;
		else n0 += n0_branch->left + 1, i += n0_branch->left + 1;
	}
	assert(n0 == n1 && i < trie->leaves.size);
	return trie->leaves.data + i;
}

static PN_(Type) **PN_(get)(const struct N_(Trie) *const trie,
	const char *const str) {
	PN_(Type) **pmatch;
	assert(trie && str);
	return (pmatch = PN_(match)(trie, str))
		&& !strcmp(PN_(to_key)(*pmatch), str) ? pmatch : 0;
}

/** Adds `data` to `trie` and, if `eject` is non-null, stores the collided
 element, if any, as long as `replace` is null or returns true.
 @param[eject] If not-null, the reference will be set to null if there is no
 ejection. If `replace`, and `replace` returns false, and `eject`, than
 `*eject == data`. */
static int PN_(put)(struct N_(Trie) *const trie, PN_(Type) *const data,
	PN_(Type) **const eject, const PN_(Replace) replace) {
	PN_(Leaf) *match;
	const char *const data_key = PN_(to_key)(data);
	assert(data);
	if(!trie || !data) return 0;
	/* Add. */
	if(!(match = PN_(get)(trie, data_key))) {
		if(eject) *eject = 0;
		return PN_(add)(trie, data);
	}
	/* Collision policy. */
	if(replace && !replace(*match, data)) {
		if(eject) *eject = data;
	} else {
		if(eject) *eject = *match;
		*match = data;
	}
	return 1;
}

/** Used in <fn:<N>TriePolicyPut> when `replace` is null; `original` and
 `replace` are ignored.
 @implements <typedef:<PN>Replace> */
static int PN_(false)(PN_(Type) *original, PN_(Type) *replace) {
	(void)(original); (void)(replace); return 0;
}


#ifndef TRIE_CHILD /* <!-- !sub-type */

/** Returns `trie` to the idle state where it takes no dynamic memory.
 @param[trie] If null, does nothing.
 @order \Theta(1)
 @allow */
static void N_(Trie_)(struct N_(Trie) *const trie)
	{ if(trie) PN_(trie_)(trie); }

/** Initialises `trie` to be idle.
 @param[trie] If null, does nothing.
 @order \Theta(1)
 @allow */
static void N_(Trie)(struct N_(Trie) *const trie)
	{ if(trie) PN_(trie)(trie); }

/** @param[trie] If null, returns zero;
 @return The number of elements in the `trie`.
 @order \Theta(1)
 @allow */
static size_t N_(TrieSize)(const struct N_(Trie) *const trie) {
	return trie ? trie->leaves.size : 0;
}

/** Sets `trie` to be empty. That is, the size of `trie` will be zero, but if
 it was previously in an active non-idle state, it continues to be.
 @param[trie] If null, does nothing.
 @order \Theta(1)
 @allow */
static void N_(TrieClear)(struct N_(Trie) *const trie) {
	if(trie) trie->branches.size = trie->leaves.size = 0;
}

static PN_(Type) *N_(TrieGet)(const struct N_(Trie) *const trie,
	const char *const str) {
	PN_(Leaf) *l;
	return trie && str && (l = PN_(get)(trie, str)) ? *l : 0;
}

/** Adds `data` to `trie`. If data with the same key is present, it fails but
 does not set `errno`.
 @param[trie, data] If null, returns null.
 @return Success.
 @throws[realloc, ERANGE] There was an error with a re-sizing.
 @order \O(`size`)
 @allow */
static int N_(TrieAdd)(struct N_(Trie) *const trie, PN_(Type) *const data) {
	return trie && data ? PN_(put)(trie, data, 0, &PN_(false)) : 0;
}

/** Adds `data` to `trie`. If data with the same key is present, it will
 overwrite it.
 @param[trie, data] If null, returns null.
 @param[eject] If not null, on success it will hold the overwritten value or
 null if it did not overwrite.
 @return Success.
 @throws[realloc, ERANGE] There was an error with a re-sizing.
 @order \O(`size`)
 @allow */
static int N_(TriePut)(struct N_(Trie) *const trie,
	PN_(Type) *const data, PN_(Type) **const eject) {
	return trie && data ? PN_(put)(trie, data, eject, 0) : 0;
}

/** Adds `data` to `trie` only if the entry is absent or if calling `replace`
 returns true.
 @param[trie, data] If null, returns null.
 @param[eject] If not null, on success it will hold the overwritten value or
 null if it did not overwrite.
 @param[replace] Called on collision and only replaces it if the function
 returns true. If null, it is sematically equivalent to <fn:<N>TreePut>.
 @return Success.
 @throws[realloc, ERANGE] There was an error with a re-sizing.
 @order \O(`size`)
 @allow */
static int N_(TriePolicyPut)(struct N_(Trie) *const trie,
	PN_(Type) *const data, PN_(Type) **const eject,
	const PN_(Replace) replace) {
	return trie && data ? PN_(put)(trie, data, eject, replace) : 0;
}

/** Can print 4 things at once before it overwrites. One must a
 `TRIE_TO_STRING` to a function implementing <typedef:<PH>ToString> to get
 this functionality.
 @return Prints `heap` in a static buffer.
 @order \Theta(1); it has a 255 character limit; every element takes some of it.
 @allow */
static const char *N_(TrieToString)(struct N_(Trie) *const trie) {
	static char buffers[4][256];
	static size_t buffer_i;
	char *const buffer = buffers[buffer_i++], *b = buffer;
	const size_t buffers_no = sizeof buffers / sizeof *buffers,
	buffer_size = sizeof *buffers / sizeof **buffers;
	const char start = '{', comma = ',', space = ' ', end = '}',
	*const ellipsis_end = ",…}", *const null = "null",
	*const idle = "idle";
	const size_t ellipsis_end_len = strlen(ellipsis_end),
		null_len = strlen(null), idle_len = strlen(idle);
	PN_(Type) *const*l, *const*l_end;
	size_t j;
	const char *str;
	int is_first = 1;
	assert(!(buffers_no & (buffers_no - 1)) && ellipsis_end_len >= 1
		&& buffer_size >= 1 + 11 + ellipsis_end_len + 1
		&& buffer_size >= null_len + 1
		&& buffer_size >= idle_len + 1);
	/* Advance the buffer for next time. */
	buffer_i &= buffers_no - 1;
	if(!trie) { memcpy(b, null, null_len), b += null_len; goto terminate; }
	if(!trie->leaves.data) { memcpy(b, idle, idle_len), b += idle_len;
		goto terminate; }
	*b++ = start;
	for(l = trie->leaves.data, l_end = l + trie->leaves.size; l < l_end; l++) {
		if(!is_first) *b++ = comma, *b++ = space;
		else is_first = 0;
		str = PN_(to_key)(*l);
		for(j = 0; *str != '\0' && j < 12; str++, b++, j++) *b = *str;
		if((size_t)(b - buffer) > buffer_size - 2 - 11 - ellipsis_end_len - 1)
			goto ellipsis;
	}
	*b++ = end;
	goto terminate;
ellipsis:
	memcpy(b, ellipsis_end, ellipsis_end_len), b += ellipsis_end_len;
terminate:
	*b++ = '\0';
	assert(b <= buffer + buffer_size);
	return buffer;
}

#ifdef TRIE_TEST /* <!-- test: need this file. */
#include "../test/TestTrie.h" /** \include */
#endif /* test --> */

static void PN_(unused_coda)(void);
/** This silences unused function warnings. */
static void PN_(unused_set)(void) {
	N_(Trie_)(0);
	N_(Trie)(0);
	N_(TrieSize)(0);
	N_(TrieClear)(0);
	N_(TrieGet)(0, 0);
	N_(TrieAdd)(0, 0);
	N_(TriePut)(0, 0, 0);
	N_(TriePolicyPut)(0, 0, 0, 0);
	N_(TrieToString)(0);
	PN_(unused_coda)();
}
static void PN_(unused_coda)(void) { PN_(unused_set)(); }

/* Un-define all macros. */
#undef CAT
#undef CAT_
#undef PCAT
#undef PCAT_
#else /* !sub-type --><!-- sub-type */
#undef TRIE_CHILD
static void PT_(unused_coda)(void);
/** This is a subtype of another, more specialised type. `CAT`, _etc_, have to
 have the same meanings; they will be replaced with these, and `T` and `N`
 cannot be used. */
static void PN_(unused_set)(void) {
	PN_(iterate)(0, 0);
	PN_(trie_)(0);
	PN_(trie)(0);
	PN_(unused_coda)();
	PN_(node_key)(0, 0);
	PN_(add)(0, 0);
	PN_(match)(0, 0);
	PN_(put)(0, 0, 0);
	PN_(false)(0);
}
static void PN_(unused_coda)(void) { PN_(unused_set)(); }
#endif /* sub-type --> */
#undef TRIE_NAME
#undef TRIE_TYPE
#undef TRIE_KEY
#undef N_
#undef PN_
#undef PT_
#ifdef TRIE_TEST
#undef TRIE_TEST
#endif
