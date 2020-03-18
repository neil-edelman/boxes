/** @license 2020 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle String Trie

 ![Example of trie.](../web/trie.png)

 An <tag:<N>Trie> is a trie of byte-strings ended with `NUL`, compatible with
 any byte-encoding with a null-terminator; in particular, `C` strings,
 including [modified UTF-8](https://en.wikipedia.org/wiki/UTF-8#Modified_UTF-8).

 It has the same asymptotic run-time as keeping a sorted array of pointers, but
 lookup is faster because it keeps an index; likewise insertion is slower,
 (asymptotically, it still has to lookup to insert,) because it has to update
 that index. Experimentally, insertion performs linearly worse, and lookup
 performs logarithmically worse, then a hash `Set` starting at about 100 items.
 However, advantages of this data structure are,

 \* because it is deterministic, a stable pointer suffices instead of a node --
    one can insert the same data into multiple tries;
 \* for the same reason, it has no need of a hash function;
 \* the keys are packed and in order;
 \* moreover, it is easy to search for like keys.

 `Array.h` must be present. `<N>Trie` is not synchronised. Errors are returned
 with `errno`. The parameters are `#define` preprocessor macros, and are all
 undefined at the end of the file for convenience. `assert.h` is used; to stop
 assertions, use `#define NDEBUG` before inclusion.

 @param[TRIE_NAME, TRIE_TYPE]
 `<N>` that satisfies `C` naming conventions when mangled and an optional
 returnable type that is declared, (it is used by reference only.) `<PN>` is
 private, whose names are prefixed in a manner to avoid collisions; any should
 be re-defined prior to use elsewhere.

 @param[TRIE_KEY]
 A function that satisfies <typedef:<PN>Key>. Must be defined if and only if
 `TRIE_TYPE` is defined.

 @param[TRIE_TEST]
 Unit testing framework <fn:<N>TrieTest>, included in a separate header,
 <../test/TreeTest.h>. Must be defined equal to a (random) filler function,
 satisfying <typedef:<PN>Action>. Requires `TRIE_TO_STRING` and not `NDEBUG`.

 @fixme Have a replace; much faster then remove and add.
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

/* This encodes the branches semi-implicitly, it contains two items of
 information in a `size_t`: left children are <fn:trie_left> immediately
 following, right children are the rest, (thus, one has to traverse
 the trie from the root,) and <fn:trie_bit>, the first bit at which the all the
 branches on the left differ from that on the right. Used in <tag:<N>Trie>. */
#define ARRAY_NAME TrieBranch
#define ARRAY_TYPE size_t
#define ARRAY_CHILD
#include "Array.h"

/* The maximum string length is 510. This is designed so that it fits 509, the
 maxiumum string length in `C90` compilers, while packing densily as much range
 as possible into this array. If `size_t` is 64-bits, that leaves
 4503599627370495 items that can be stored, (4 bits less than an array.)
 However, if 32-bits, 1048575, and if 16-bits, 15. One might modify `TRIE_BITS`
 as appropiate, or make it a larger type for one's target architecture. */
#define TRIE_BITS 12
#define TRIE_BIT_MAX ((1 << TRIE_BITS) - 1)
#define TRIE_LEFT_MAX (((size_t)1 << ((sizeof(size_t) << 3) - TRIE_BITS)) - 1)

/** @return Packs `bit` and `left` into a branch `size_t`. */
static size_t trie_branch(const unsigned bit, const size_t left) {
	assert(bit <= TRIE_BIT_MAX && left <= TRIE_LEFT_MAX);
	return bit + (left << TRIE_BITS);
}

/** @return Unpacks bit from `branch`. */
static unsigned trie_bit(const size_t branch)
	{ return (unsigned)branch & TRIE_BIT_MAX; }

/** @return Unpacks left from `branch`. */
static size_t trie_left(const size_t branch) { return branch >> TRIE_BITS; }

/** Increments the left `branch` count. Does not check for overflow. */
static void trie_left_inc(size_t *const branch) { *branch += TRIE_BIT_MAX + 1; }

/** Compares `bit` from the string `a` against `b`. Does not check for the end
 of the string, so ensure that they are different. It may be better to compare
 muliple bits at a time, but 1) this is used in insertion only, 2) the advatage
 goes down or reverses at high string packing, and 3) it's difficult.
 @return In the `bit` position, positive if `a` is after `b`, negative if `a`
 is before `b`, or zero if `a` is equal to `b`. */
static int trie_strcmp_bit(const char *const a, const char *const b,
	const unsigned bit) {
	const unsigned byte = bit >> 3, mask = 128 >> (bit & 7);
	return (a[byte] & mask) - (b[byte] & mask);
}

/** From string `a`, extract `bit` and return zero or non-zero if one. Does not
 check for the end of the string. */
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
/** @return The `key`, which is the string itself in the case where one doesn't
 specify `TRIE_TYPE`. */
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

/** Responsible for picking out the null-terminated string. One must not modify
 this string while in any trie. */
typedef const char *(*PN_(Key))(PN_(Type) *);

/* Check that `TRIE_KEY` is a function implementing <typedef:<PN>Key>. */
static const PN_(Key) PN_(to_key) = (TRIE_KEY);

/** A bi-predicate; returns true if the `replace` replaces the `original`; used
 in <fn:<N>TriePolicyPut>. */
typedef int (*PN_(Replace))(PN_(Type) *original, PN_(Type) *replace);

/* Used internally to get rid of the confusing double-pointers. */
typedef PN_(Type) *PN_(Leaf);

/* Trie leaf array is sorted by key. Private code follows a convention that in
 `branches` (internal nodes) the subscripts begin by `n` and in `leaves`
 (external nodes) it begins with `i`. */
#define ARRAY_NAME PN_(Leaf)
#define ARRAY_TYPE PN_(Leaf)
#define ARRAY_CHILD
#include "Array.h"

#define PT_(thing) PCAT(array, PCAT(PN_(Leaf), thing))

/** To initialise it to an idle state, see <fn:<N>Trie>, `TRIE_IDLE`, `{0}`
 (`C99`), or being `static`.

 A full binary tree stored semi-implicitly in two arrays: one as the branches
 backed by one as pointers-to-<typedef:<PN>Type> as leaves. We take two arrays
 because it speeds up iteration as the leaves are also an array sorted by key,
 it is \O(1) instead of \O(log `items`) to get an example for comparison in
 insert, and experimetally it is slightly faster.

 It can be seen as a
 [binary radix trie](https://en.wikipedia.org/wiki/Radix_tree) or
 <Morrison, 1968 PATRICiA>. The branches do not store data on the strings, only
 the positions where the strings are different.

 ![States.](../web/states.png) */
struct N_(Trie);
struct N_(Trie) {
	struct TrieBranchArray branches;
	struct PN_(LeafArray) leaves;
};
#ifndef TRIE_IDLE /* <!-- !zero */
#define TRIE_IDLE { { 0, 0, 0, 0 }, { 0, 0, 0, 0 } }
#endif /* !zero --> */

/** Debug: prints `trie`. */
static void PN_(print)(const struct N_(Trie) *const trie) {
	size_t i, n;
	printf("__Trie: ");
	if(!trie) { printf("null.\n"); return; }
	printf("{");
	for(i = 0; i < trie->leaves.size; i++)
		printf("%s%s", i ? ", " : "", PN_(to_key)(trie->leaves.data[i]));
	printf("}; {");
	for(n = 0; n < trie->branches.size; n++)
		printf("%s%u:%lu", n ? ", " : "", trie_bit(trie->branches.data[n]),
		(unsigned long)trie_left(trie->branches.data[n]));
	printf("}.\n");
}

/** Initialises `trie` to idle. */
static void PN_(trie)(struct N_(Trie) *const trie) {
	assert(trie);
	array_TrieBranch_array(&trie->branches), PT_(array)(&trie->leaves);
}

/** Returns an initalised `trie` to idle. */
static void PN_(trie_)(struct N_(Trie) *const trie) {
	assert(trie);
	array_TrieBranch_array_(&trie->branches), PT_(array_)(&trie->leaves);
	PN_(trie)(trie);
}

/** Add `data` to `trie`. This assumes that the key of `data` is not the same
 as any in `trie`; it does not check for the end of the string.
 @order \Theta(`nodes`) */
static int PN_(add)(struct N_(Trie) *const trie, PN_(Type) *const data) {
	const size_t leaf_size = trie->leaves.size, branch_size = leaf_size - 1;
	size_t n0, n1, i;
	size_t *n0_branch, left;
	const char *const data_key = PN_(to_key)(data), *n0_key;
	unsigned bit, n0_bit;
	PN_(Leaf) *i_leaf;
	int cmp;

	/* Verify data. */
	assert(trie && data && n1 < (size_t)-2);
	if(strlen(data_key) > (TRIE_BIT_MAX >> 3) - 1/* null */)
		return errno = ERANGE, 0;

	/* Empty short circuit; add one entry to `leaves`. */
	if(!leaf_size) {
		assert(!trie->branches.size);
		return (i_leaf = PT_(new)(&trie->leaves)) ? *i_leaf = data, 1 : 0;
	}

	/* Non-empty; conservative maximally unbalanced trie. */
	assert(leaf_size == branch_size + 1); /* Waste `size_t`. */
	if(leaf_size >= TRIE_LEFT_MAX) return errno = ERANGE, 0;
	if(!PT_(reserve)(&trie->leaves, leaf_size + 1)
		|| !array_TrieBranch_reserve(&trie->branches, branch_size + 1))
		return 0;

	/* Internal nodes. */
	bit = 0;
	i = 0, n0 = 0, n1 = branch_size;
	while(n0_branch = trie->branches.data + n0,
		n0_key = trie->leaves.data[i],
		n0 < n1) {
		for(n0_bit = trie_bit(*n0_branch); bit < n0_bit; bit++)
			if((cmp = trie_strcmp_bit(data_key, n0_key, bit)) != 0) goto insert;
		/* Follow the left or right branch; update the left. */
		if(!trie_is_bit(data_key, bit))
			trie_left_inc(n0_branch), n1 = n0++ + trie_left(*n0_branch);
		else n0 += trie_left(*n0_branch) + 1, i += trie_left(*n0_branch) + 1;
	}

	/* Leaf. */
	while((cmp = trie_strcmp_bit(data_key, n0_key, bit)) == 0) bit++;

insert:
	assert(n0 <= n1 && n1 <= trie->branches.size && n0_key
		&& i <= trie->leaves.size);
	if(cmp < 0) left = 0;
	else i += n1 - n0 + 1, left = (unsigned)(n1 - n0);

	i_leaf = trie->leaves.data + i;
	memmove(i_leaf + 1, i_leaf, sizeof *i_leaf * (leaf_size - i));
	*i_leaf = data;
	trie->leaves.size++;

	n0_branch = trie->branches.data + n0;
	memmove(n0_branch + 1, n0_branch, sizeof *n0_branch * (branch_size - n0));
	*n0_branch = trie_branch(bit, left);
	trie->branches.size++;

	return 1;
}

/** Only looks in internal nodes, (branches.)
 @return `trie` leaf that potentially matches `key` or null if it definitely is
 not in `trie`. */
static PN_(Leaf) *PN_(match)(const struct N_(Trie) *const trie,
	const char *const key) {
	size_t n0 = 0, n1 = trie->leaves.size, i = 0;
	size_t n0_branch;
	unsigned n0_byte, str_byte = 0;
	assert(trie);
	/* Special case. */
	if(n1 <= 1) return n1 ? trie->leaves.data : 0;
	n1--, assert(n1 == trie->branches.size);
	while(n0 < n1) {
		n0_branch = trie->branches.data[n0];
		/* Don't go farther than the string. */
		for(n0_byte = trie_bit(n0_branch) >> 3; str_byte < n0_byte; str_byte++)
			if(key[str_byte] == '\0') return 0;
		/* Follow the branch left/right. */
		if(!trie_is_bit(key, trie_bit(n0_branch)))
			n1 = ++n0 + trie_left(n0_branch);
		else n0 += trie_left(n0_branch) + 1, i += trie_left(n0_branch) + 1;
	}
	assert(n0 == n1 && i < trie->leaves.size);
	return trie->leaves.data + i;
}

/** If `key` in `trie` is a branch-match to some element, follow a pointer to
 that element and see if it is an exact match.
 @return The exact match leaf, (a pointer-to-pointer-to-<typedef:<PN>Type>,) or
 null if the data is not in `trie`. */
static PN_(Leaf) *PN_(get)(const struct N_(Trie) *const trie,
	const char *const key) {
	PN_(Type) **pmatch;
	assert(trie && key);
	return (pmatch = PN_(match)(trie, key))
		&& !strcmp(PN_(to_key)(*pmatch), key) ? pmatch : 0;
}

/** Adds `data` to `trie` and, if `eject` is non-null, stores the collided
 element, if any, as long as `replace` is null or returns true.
 @param[eject] If not-null, the reference will be set to null if there is no
 ejection. If `replace`, and `replace` returns false, and `eject`, than
 `*eject == data`.
 @throws[realloc, ERANGE] */
static int PN_(put)(struct N_(Trie) *const trie, PN_(Type) *const data,
	PN_(Type) **const eject, const PN_(Replace) replace) {
	PN_(Leaf) *match;
	const char *data_key;
	assert(trie && data);
	data_key = PN_(to_key)(data);
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

/** Used in <fn:<N>TrieAdd>; `original` and `replace` are ignored and it
 returns false.
 @implements <typedef:<PN>Replace> */
static int PN_(false)(PN_(Type) *original, PN_(Type) *replace) {
	return (void)(original), (void)(replace), 0;
}


#ifndef TRIE_CHILD /* <!-- !sub-type */

/** Returns `trie` to the idle state where it takes no dynamic memory.
 @param[trie] If null, does nothing.
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

/** It remains valid up to a structural modification of `trie`.
 @param[trie] If null, returns null.
 @return The leaves of `trie`, ordered by key.
 @allow */
static PN_(Type) *const*N_(TrieArray)(const struct N_(Trie) *const trie) {
	return trie && trie->leaves.size ? trie->leaves.data : 0;
}

/** Sets `trie` to be empty. That is, the size of `trie` will be zero, but if
 it was previously in an active non-idle state, it continues to be.
 @param[trie] If null, does nothing.
 @order \Theta(1)
 @allow */
static void N_(TrieClear)(struct N_(Trie) *const trie) {
	if(trie) trie->branches.size = trie->leaves.size = 0;
}

/** @param[trie, key] If null, returns null.
 @return The <typedef:<PN>Type> with `key` in `trie` or null no such item
 exists. */
static PN_(Type) *N_(TrieGet)(const struct N_(Trie) *const trie,
	const char *const key) {
	PN_(Leaf) *l;
	return trie && key && (l = PN_(get)(trie, key)) ? *l : 0;
}

/** Adds `data` to `trie` if absent.
 @param[trie, data] If null, returns null.
 @return Success. If data with the same key is present, returns false, but does
 not set `errno`.
 @throws[realloc, ERANGE] There was an error with a re-sizing.
 @throws[ERANGE] The key is greater then 510 characters or the trie has reached
 it's maximum size.
 @order \O(`size`)
 @allow */
static int N_(TrieAdd)(struct N_(Trie) *const trie, PN_(Type) *const data) {
	return trie && data ? PN_(put)(trie, data, 0, &PN_(false)) : 0;
}

/** Updates or adds `data` to `trie`.
 @param[trie, data] If null, returns null.
 @param[eject] If not null, on success it will hold the overwritten value or
 a pointer-to-null if it did not overwrite.
 @return Success.
 @throws[realloc, ERANGE] There was an error with a re-sizing.
 @throws[ERANGE] The key is greater then 510 characters or the trie has reached
 it's maximum size.
 @order \Theta(`size`)
 @allow */
static int N_(TriePut)(struct N_(Trie) *const trie,
	PN_(Type) *const data, PN_(Type) **const eject) {
	return trie && data ? PN_(put)(trie, data, eject, 0) : 0;
}

/** Adds `data` to `trie` only if the entry is absent or if calling `replace`
 returns true.
 @param[trie, data] If null, returns null.
 @param[eject] If not null, on success it will hold the overwritten value or
 a pointer-to-null if it did not overwrite a previous value.
 @param[replace] Called on collision and only replaces it if the function
 returns true. If null, it is semantically equivalent to <fn:<N>TriePut>.
 @return Success.
 @throws[realloc, ERANGE] There was an error with a re-sizing.
 @throws[ERANGE] The key is greater then 510 characters or the trie has reached
 it's maximum size.
 @order \O(`size`)
 @allow */
static int N_(TriePolicyPut)(struct N_(Trie) *const trie,
	PN_(Type) *const data, PN_(Type) **const eject,
	const PN_(Replace) replace) {
	return trie && data ? PN_(put)(trie, data, eject, replace) : 0;
}

/** Can print four strings at once before it overwrites.
 @return Prints the keys of `trie` in a static buffer.
 @order \Theta(1); it has a 255 character limit; every element takes some of it.
 @allow */
static const char *N_(TrieToString)(const struct N_(Trie) *const trie) {
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
	size_t i;
	const char *str;
	int is_first = 1;
	assert(!(buffers_no & (buffers_no - 1)) && ellipsis_end_len >= 1
		&& buffer_size >= 1/*start*/ + 11/*max*/ + ellipsis_end_len + 1/*null*/
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
		for(i = 0; *str != '\0' && i < 12; str++, b++, i++) *b = *str;
		if((size_t)(b - buffer) >= buffer_size - 2/*comma,space*/ - 11/*max*/
			- ellipsis_end_len - 1/*null*/) goto ellipsis;
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
	N_(TrieArray)(0);
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
