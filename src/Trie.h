/** @license 2020 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Parameterised String-Key Trie

 ![Example of trie.](../web/trie.png)

 A <tag:<N>Trie> is an array of pointers-to-`N` and index on a unique
 identifier string that is associated to `N`. Strings can be any encoding with
 a byte null-terminator; in particular, `C` native strings, including
 [modified UTF-8](https://en.wikipedia.org/wiki/UTF-8#Modified_UTF-8). It can
 be seen as a <Morrison, 1968 PATRICiA>, in that it only stores data in the
 index on the positions where the strings are different. It is also a
 [binary radix trie](https://en.wikipedia.org/wiki/Radix_tree) which lives in
 compact \O(`size`) extra memory. Insertion and deletion are slower because
 the need to update the array index.

 `Array.h` must be present. `<N>Trie` is not synchronised. Errors are returned
 with `errno`. The parameters are `#define` preprocessor macros, and are all
 undefined at the end of the file for convenience. `assert.h` is used; to stop
 assertions, use `#define NDEBUG` before inclusion.

 @param[TRIE_NAME, TRIE_TYPE]
 <typedef:<PN>Type> that satisfies `C` naming conventions when mangled and an
 optional returnable type that is declared, (it is used by reference only
 except if `TRIE_TEST`.) `<PN>` is private, whose names are prefixed in a
 manner to avoid collisions; any should be re-defined prior to use elsewhere.

 @param[TRIE_KEY]
 A function that satisfies <typedef:<PN>Key>. Must be defined if and only if
 `TRIE_TYPE` is defined.

 @param[TRIE_TEST]
 Unit testing framework <fn:<N>TrieTest>, included in a separate header,
 <../test/TreeTest.h>. Must be defined equal to a (random) filler function,
 satisfying <typedef:<PN>Action>. Requires that `NDEBUG` not be defined.

 @fixme Have a replace; potentially much less wastful then remove and add.
 @fixme Compression _a la_ Judy; 64 bits to store mostly 0/1? Could it be done?
 @depend [Array.h](../../Array/)
 @std C89
 @cf [Array](https://github.com/neil-edelman/Array)
 @cf [Heap](https://github.com/neil-edelman/Heap)
 @cf [List](https://github.com/neil-edelman/List)
 @cf [Orcish](https://github.com/neil-edelman/Orcish)
 @cf [Pool](https://github.com/neil-edelman/Pool)
 @cf [Set](https://github.com/neil-edelman/Set) */

#include <string.h> /* size_t memmove strcmp memcpy */
#include <limits.h> /* UINT_MAX */


#ifndef TRIE_NAME
#error Name TRIE_NAME undefined.
#endif
#if (defined(TRIE_TYPE) && !defined(TRIE_KEY)) \
	|| (!defined(TRIE_TYPE) && defined(TRIE_KEY))
#error TRIE_TYPE and TRIE_KEY have to be mutually defined or not.
#endif
#define TRIE_INTERFACES (defined(TRIE_TO_STRING_NAME) \
	|| defined(TRIE_TO_STRING))
#if TRIE_INTERFACES > 1
#error Only one interface per include is allowed; use TRIE_UNFINISHED.
#endif
#if (TRIE_INTERFACES == 0) && defined(TRIE_TEST)
#error TRIE_TEST must be defined in TRIE_TO_STRING interface.
#endif
#if defined(TRIE_TO_STRING_NAME) && !defined(TRIE_TO_STRING)
#error TRIE_TO_STRING_NAME requires TRIE_TO_STRING.
#endif


#if TRIE_INTERFACES == 0 /* <!-- base code */


/* <Kernighan and Ritchie, 1988, p. 231>; before idempotent _st_ `CAT`. */
#if defined(N_) || defined(PN_)
#error P?N_? cannot be defined; possible stray TRIE_UNFINISHED?
#endif
#ifndef TRIE_CHILD /* <!-- !sub-type */
#define CAT_(x, y) x ## y
#define CAT(x, y) CAT_(x, y)
#define PCAT_(x, y) x ## _ ## y
#define PCAT(x, y) PCAT_(x, y)
#elif !defined(CAT) || !defined(PCAT) /* !sub-type --><!-- !cat */
#error TRIE_CHILD defined but CAT is not.
#endif /* !cat --> */
#define N_(thing) CAT(TRIE_NAME, thing)
#define PN_(thing) PCAT(trie, PCAT(TRIE_NAME, thing))

#ifndef TRIE_H /* <!-- idempotent */
#define TRIE_H

/* Trie internal nodes encode the branches semi-implicitly. Each contains two
 items of information in a `size_t`: left children branches are <fn:trie_left>
 immediately following, right children are the rest, and <fn:trie_bit>, the
 bit at which the all the branches on the left differ from that on the right. */
typedef size_t TrieBranch;

#define ARRAY_NAME TrieBranch
#define ARRAY_TYPE TrieBranch
#define ARRAY_CHILD
#include "Array.h"

/* 12 makes the maximum string length 510 and the maximum size of a trie is
 `size_t` 64-bits: 4503599627370495, 32-bits: 1048575, and 16-bits: 15. */
#define TRIE_BITS 12
#define TRIE_BIT_MAX ((1 << TRIE_BITS) - 1)
#define TRIE_LEFT_MAX (((size_t)1 << ((sizeof(size_t) << 3) - TRIE_BITS)) - 1)

/** @return Packs `bit` and `left` into a branch. */
static TrieBranch trie_branch(const unsigned bit, const size_t left) {
	assert(bit <= TRIE_BIT_MAX && left <= TRIE_LEFT_MAX);
	return bit + (left << TRIE_BITS);
}

/** @return Unpacks bit from `branch`. */
static unsigned trie_bit(const TrieBranch branch)
	{ return (unsigned)branch & TRIE_BIT_MAX; }

/** @return Unpacks left sub-branches from `branch`. */
static size_t trie_left(const TrieBranch branch) { return branch >> TRIE_BITS; }

/** Increments the left `branch` count. */
static void trie_left_inc(size_t *const branch)
	{ assert(*branch < ~(size_t)TRIE_BIT_MAX), *branch += TRIE_BIT_MAX + 1; }

/** Decrements the left `branch` count. */
static void trie_left_dec(size_t *const branch)
	{ assert(*branch > TRIE_BIT_MAX), *branch -= TRIE_BIT_MAX + 1; }

/** Compares `bit` from the string `a` against `b`.
 @return In the `bit` position, positive if `a` is after `b`, negative if `a`
 is before `b`, or zero if `a` is equal to `b`. */
static int trie_strcmp_bit(const char *const a, const char *const b,
	const unsigned bit) {
	const unsigned byte = bit >> 3, mask = 128 >> (bit & 7);
	return (a[byte] & mask) - (b[byte] & mask);
}

/** From string `a`, extract `bit` and return zero or non-zero if one. */
static unsigned trie_is_bit(const char *const a, const unsigned bit) {
	const unsigned byte = bit >> 3, mask = 128 >> (bit & 7);
	return a[byte] & mask;
}

#endif /* idempotent --> */

/* Defaults. */
#ifndef TRIE_TYPE /* <!-- !type */
#define TRIE_CONST /* Duplicate const. */
#define TRIE_TYPE const char
#define TRIE_KEY &trie_raw
#ifndef TRIE_RAW /* <!-- !raw */
#define TRIE_RAW /* Idempotent function. */
/** @return The `key`, which is the string itself in the case where one doesn't
 specify `TRIE_TYPE`. */
static const char *trie_raw(const char *const key) { return key; }
#endif /* !raw --> */
#else /* !type --><!-- type */
#define TRIE_CONST const
#endif /* type --> */

/** A valid tag type set by `TRIE_TYPE`; defaults to `const char`. */
typedef TRIE_TYPE PN_(Type);

/** Same as <typedef:<PN>Type>, except read-only. */
typedef TRIE_CONST PN_(Type) PN_(CType);

#undef TRIE_CONST /* Just for <typedef:<PN>CType>. */

/* Used internally to get rid of the confusing double-pointers. */
typedef PN_(Type) *PN_(Leaf);

/** Responsible for picking out the null-terminated string. One must not modify
 this string while in any trie. */
typedef const char *(*PN_(Key))(PN_(CType) *);

/* Check that `TRIE_KEY` is a function implementing <typedef:<PN>Key>. */
static const PN_(Key) PN_(to_key) = (TRIE_KEY);

/** A bi-predicate; returns true if the `replace` replaces the `original`; used
 in <fn:<N>TriePolicyPut>. */
typedef int (*PN_(Replace))(PN_(Type) *original, PN_(Type) *replace);

/** @return False. Ignores `a` and `b`. */
static int PN_(false_replace)(PN_(Type) *const a, PN_(Type) *const b)
	{ return (void)a, (void)b, 0; }

/** Responsible for turning the first argument into a 12-`char` null-terminated
 output string. Used for `ARRAY_TO_STRING`. */
typedef void (*PN_(ToString))(PN_(CType) *, char (*)[12]);

/** Compares keys of `a` and `b`. Used in array compare following.
 @implements <<PN>Leaf>Bipredicate */
static int PN_(cmp)(const PN_(Leaf) *const a, const PN_(Leaf) *const b)
	{ return strcmp(PN_(to_key)(*a), PN_(to_key)(*b)); }

/* Trie leaf array is sorted by key. Private code follows a convention that in
 `branches` (internal nodes) have `n` subscripts and `leaves` (external nodes)
 have `i` subscripts. */
#define ARRAY_NAME PN_(Leaf)
#define ARRAY_TYPE PN_(Leaf)
#define ARRAY_CHILD
#define ARRAY_UNFINISHED
#include "Array.h"
#define ARRAY_COMPARE &PN_(cmp)
#include "Array.h"

/*#define PT_(thing) PCAT(PCAT(array, PN_(Leaf)), PCAT(thing, anonymous))*/
/* array_trie_Str_Leaf_compactify_anonymous */
#define PT_(thing) PCAT(PCAT(array, PN_(Leaf)), thing)

/** To initialise it to an idle state, see <fn:<N>Trie>, `TRIE_IDLE`, `{0}`
 (`C99`), or being `static`.

 A full binary tree stored semi-implicitly in two arrays: a private one as
 branches backed by one as pointers-to-<typedef:<PN>Type> as leaves in
 numerically-sorted order.

 ![States.](../web/states.png) */
struct N_(Trie);
struct N_(Trie) {
	struct TrieBranchArray branches;
	struct PN_(LeafArray) leaves;
};
#ifndef TRIE_IDLE /* <!-- !zero */
#define TRIE_IDLE { ARRAY_IDLE, ARRAY_IDLE }
#endif /* !zero --> */

/** Initialises `trie` to idle. */
static void PN_(trie)(struct N_(Trie) *const trie) {
	assert(trie);
	array_TrieBranch_array(&trie->branches), PT_(array)(&trie->leaves);
}

/** Returns an initialised `trie` to idle. */
static void PN_(trie_)(struct N_(Trie) *const trie) {
	assert(trie);
	array_TrieBranch_array_(&trie->branches), PT_(array_)(&trie->leaves);
	PN_(trie)(trie);
}

/** Recursive function used for <fn:<PN>init>. Initialise branches of `trie` up
 to `bit` with `a` to `a_size` array of sorted leaves.
 @order Speed \O(`leaves`), memory \O(`longest string`). */
static void PN_(init_branches_r)(struct N_(Trie) *const trie, unsigned bit,
	const size_t a, const size_t a_size) {
	size_t a1 = a, s = a_size, half_s;
	TrieBranch *branch;
	assert(trie && a_size && a_size <= trie->leaves.size && trie->leaves.size
		&& trie->branches.capacity >= trie->leaves.size - 1);
	if(a_size <= 1) return;
	/* Endpoints. */
	while(trie_is_bit(PN_(to_key)(trie->leaves.data[a]), bit)
		|| !trie_is_bit(PN_(to_key)(trie->leaves.data[a + a_size - 1]), bit))
		bit++;
	/* Do a binary search for the first `leaves[a+half_s]#bit == 1`. */
	while(s) half_s = s >> 1,
		trie_is_bit(PN_(to_key)(trie->leaves.data[a1 + half_s]), bit)
		? s = half_s : (half_s++, a1 += half_s, s -= half_s);
	s = a1 - a;
	/* Should have space for all branches pre-allocated, (right?) */
	branch = array_TrieBranch_new(&trie->branches), assert(branch);
	*branch = trie_branch(bit, s - 1);
	bit++;
	PN_(init_branches_r)(trie, bit, a, s);
	PN_(init_branches_r)(trie, bit, a1, a_size - s);
}

/** @param[merge] Called with any duplicate entries and replaces if true; if
 null, doesn't replace.
 @return Success initialising `trie` with `a` of size `a_size`, (non-zero.) */
static int PN_(init)(struct N_(Trie) *const trie, PN_(Type) *const*const a,
	const size_t a_size, const PT_(Biproject) merge) {
	PN_(Leaf) *leaves;
	assert(trie && !trie->leaves.size && !trie->branches.size
		&& a && a_size /* `merge` can be null. */);
	if(!PT_(reserve)(&trie->leaves, a_size)
		|| !array_TrieBranch_reserve(&trie->branches, a_size - 1)) return 0;
	leaves = trie->leaves.data;
	memcpy(leaves, a, sizeof *a * a_size);
	trie->leaves.size = a_size;
	/* Private functions from `Array.h`. */
	qsort(leaves, a_size, sizeof *a, &PT_(compar_anonymous));
	PT_(compactify_anonymous)(&trie->leaves, merge);
	PN_(init_branches_r)(trie, 0, 0, trie->leaves.size);
	assert(trie->branches.size + 1 == trie->leaves.size);
	return 1;
}

/** Add `data` to `trie`. Must not be the same as any key of trie; _ie_ it does
 not check for the end of the string.
 @order \Theta(`nodes`)
 @throws[ERANGE] At capacity or string too long.
 @throws[realloc] */
static int PN_(add)(struct N_(Trie) *const trie, PN_(Type) *const data) {
	const size_t leaf_size = trie->leaves.size, branch_size = leaf_size - 1;
	size_t n0 = 0, n1 = branch_size, i = 0, left;
	TrieBranch *branch;
	const char *const data_key = PN_(to_key)(data), *n0_key;
	unsigned bit = 0, n0_bit;
	PN_(Leaf) *leaf;
	int cmp;

	/* Verify string length and empty short circuit. */
	assert(trie && data);
	if(strlen(data_key) > (TRIE_BIT_MAX >> 3) - 1/* null */)
		return errno = ERANGE, 0;
	if(!leaf_size) return assert(!trie->branches.size),
		(leaf = PT_(new)(&trie->leaves)) ? *leaf = data, 1 : 0;

	/* Non-empty; verify conservative maximally unbalanced trie. */
	assert(leaf_size == branch_size + 1); /* Waste `size_t`. */
	if(leaf_size >= TRIE_LEFT_MAX) return errno = ERANGE, 0;
	if(!PT_(reserve)(&trie->leaves, leaf_size + 1)
		|| !array_TrieBranch_reserve(&trie->branches, branch_size + 1))
		return 0;

	/* Internal nodes. */
	while(branch = trie->branches.data + n0,
		n0_key = PN_(to_key)(trie->leaves.data[i]), n0 < n1) {
		for(n0_bit = trie_bit(*branch); bit < n0_bit; bit++)
			if((cmp = trie_strcmp_bit(data_key, n0_key, bit)) != 0) goto insert;
		left = trie_left(*branch) + 1;
		if(!trie_is_bit(data_key, bit)) trie_left_inc(branch), n1 = n0++ + left;
		else n0 += left, i += left;
	}

	/* Leaf. */
	while((cmp = trie_strcmp_bit(data_key, n0_key, bit)) == 0) bit++;

insert:
	assert(n0 <= n1 && n1 <= trie->branches.size && n0_key
		&& i <= trie->leaves.size);
	if(cmp < 0) left = 0;
	else left = n1 - n0, i += left + 1;

	leaf = trie->leaves.data + i;
	memmove(leaf + 1, leaf, sizeof *leaf * (leaf_size - i));
	*leaf = data;
	trie->leaves.size++;

	branch = trie->branches.data + n0;
	memmove(branch + 1, branch, sizeof *branch * (branch_size - n0));
	*branch = trie_branch(bit, left);
	trie->branches.size++;

	return 1;
}

/** @return `trie` leaf that potentially matches `key` or null if it definitely
 is not in `trie`. */
static PN_(Leaf) *PN_(match)(const struct N_(Trie) *const trie,
	const char *const key) {
	size_t n0 = 0, n1 = trie->leaves.size, i = 0, left;
	TrieBranch branch;
	unsigned n0_byte, str_byte = 0, bit;
	assert(trie && key);
	if(n1 <= 1) return n1 ? trie->leaves.data : 0; /* Special case. */
	n1--, assert(n1 == trie->branches.size);
	while(n0 < n1) {
		branch = trie->branches.data[n0];
		bit = trie_bit(branch);
		for(n0_byte = bit >> 3; str_byte < n0_byte; str_byte++)
			if(key[str_byte] == '\0') return 0;
		left = trie_left(branch);
		if(!trie_is_bit(key, bit)) n1 = ++n0 + left;
		else n0 += left + 1, i += left + 1;
	}
	assert(n0 == n1 && i < trie->leaves.size);
	return trie->leaves.data + i;
}

/** @return `key` is an element of `trie` that is an exact match or null. */
static PN_(Leaf) *PN_(get)(const struct N_(Trie) *const trie,
	const char *const key) {
	PN_(Type) **pmatch;
	assert(trie && key);
	return (pmatch = PN_(match)(trie, key))
		&& !strcmp(PN_(to_key)(*pmatch), key) ? pmatch : 0;
}

/** Adds `data` to `trie` and, if `eject` is non-null, stores the collided
 element, if any, as long as `merge` is null or returns true.
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

/** Remove leaf index `i` from `trie`. */
static void PN_(remove)(struct N_(Trie) *const trie, size_t i) {
	size_t n0 = 0, n1 = trie->branches.size, last_n0, left;
	size_t *branch;
	assert(trie && i < trie->leaves.size
		&& trie->branches.size + 1 == trie->leaves.size);

	/* Remove leaf. */
	if(!--trie->leaves.size) return; /* Special case of one leaf. */
	memmove(trie->leaves.data + i, trie->leaves.data + i + 1,
		sizeof(PN_(Leaf)) * (n1 - i));

	/* Remove branch. */
	for( ; ; ) {
		left = trie_left(*(branch = trie->branches.data + (last_n0 = n0)));
		if(i <= left) {
			if(!left) break;
			n1 = ++n0 + left;
			trie_left_dec(branch);
		} else {
			if((n0 += ++left) >= n1) break;
			i -= left;
		}
	}
	memmove(branch, branch + 1, sizeof n0 * (--trie->branches.size - last_n0));
}

/** Shrinks `trie` to size. The arrays probably still a distance away. */
static int PN_(shrink)(struct N_(Trie) *const trie) {
	assert(trie);
	return array_TrieBranch_shrink(&trie->branches)
		&& PT_(shrink)(&trie->leaves);
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

/** Initialises `trie` from an `array` of pointers-to-`<N>` of `array_size`.
 @param[trie] If null, does nothing.
 @param[array] If null, initialises `trie` to empty.
 @return Success.
 @throws[realloc]
 @order \O(`array_size`)
 @allow */
static int N_(TrieFromArray)(struct N_(Trie) *const trie,
	PN_(Type) *const*const array, const size_t array_size,
	const PT_(Biproject) merge) {
	return trie ? (PN_(trie)(trie), !array || !array_size) ? 1
		: PN_(init)(trie, array, array_size, merge /* Can be null. */) : 0;
}

/** @param[trie] If null, returns zero;
 @return The number of elements in the `trie`.
 @order \Theta(1)
 @allow */
static size_t N_(TrieSize)(const struct N_(Trie) *const trie) {
	return trie ? trie->leaves.size : 0;
}

/** It remains valid up to a structural modification of `trie` and is indexed
 up to <fn:<N>TrieSize>.
 @param[trie] If null, returns null.
 @return An array of pointers to the leaves of `trie`, ordered by key.
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
 exists.
 @order \O(|`key`|). Specifically, faster then a tree, and deterministic,
 however, logarithmically slower then a good hash table for sizes not fitting
 in cache, <Thareja 2011, Data>.
 @allow */
static PN_(Type) *N_(TrieGet)(const struct N_(Trie) *const trie,
	const char *const key) {
	PN_(Leaf) *leaf;
	return trie && key && (leaf = PN_(get)(trie, key)) ? *leaf : 0;
}

/** @param[trie, key] If null, returns null.
 @return The <typedef:<PN>Type> reasonably with the Levenson distance closest
 to `key` in `trie`.
 @allow */
static PN_(Type) *N_(TrieClose)(const struct N_(Trie) *const trie,
	const char *const key) {
	PN_(Leaf) *leaf;
	return trie && key && (leaf = PN_(match)(trie, key)) ? *leaf : 0;
}

/** Adds `data` to `trie` if absent.
 @param[trie, data] If null, returns null.
 @return Success. If data with the same key is present, returns true but
 doesn't add `data`.
 @throws[realloc] There was an error with a re-sizing.
 @throws[ERANGE] The key is greater then 510 characters or the trie has reached
 it's maximum size.
 @order \O(`size`)
 @allow */
static int N_(TrieAdd)(struct N_(Trie) *const trie, PN_(Type) *const data) {
	return trie && data ? PN_(put)(trie, data, 0, &PN_(false_replace)) : 0;
}

/** Updates or adds `data` to `trie`.
 @param[trie, data] If null, returns null.
 @param[eject] If not null, on success it will hold the overwritten value or
 a pointer-to-null if it did not overwrite.
 @return Success.
 @throws[realloc] There was an error with a re-sizing.
 @throws[ERANGE] The key is greater then 510 characters or the trie has reached
 it's maximum size.
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
 a pointer-to-null if it did not overwrite a previous value. If a collision
 occurs and `replace` does not return true, this value will be `data`.
 @param[replace] Called on collision and only replaces it if the function
 returns true. If null, it is semantically equivalent to <fn:<N>TriePut>.
 @return Success.
 @throws[realloc] There was an error with a re-sizing.
 @throws[ERANGE] The key is greater then 510 characters or the trie has reached
 it's maximum size.
 @order \O(`size`)
 @allow */
static int N_(TriePolicyPut)(struct N_(Trie) *const trie,
	PN_(Type) *const data, PN_(Type) **const eject,
	const PN_(Replace) replace) {
	return trie && data && PN_(put)(trie, data, eject, replace);
}

/** Remove `key` from `trie`.
 @param[trie, key] If null, returns false.
 @return Success or else `key` was not in `trie`.
 @order \O(`size`)
 @allow */
static int N_(TrieRemove)(struct N_(Trie) *const trie, const char *const key) {
	PN_(Leaf) *leaf;
	return trie && key && (leaf = PN_(get)(trie, key))
		? (PN_(remove)(trie, leaf - trie->leaves.data), 1) : 0;
}

/** Shrinks the capacity of `trie` to size.
 @return Success. @throws[ERANGE, realloc] Unlikely `realloc` error. */
static int N_(TrieShrink)(struct N_(Trie) *const trie)
	{ return trie ? PN_(shrink)(trie) : 0; }

#endif /* !sub-type --> */

static void PN_(unused_base_coda)(void);
static void PN_(unused_base)(void) {
	PN_(trie)(0); PN_(trie_)(0); PN_(init)(0, 0, 0, 0); PN_(add)(0, 0);
	PN_(match)(0, 0); PN_(get)(0, 0); PN_(put)(0, 0, 0, 0); PN_(remove)(0, 0);
	PN_(shrink)(0);
#ifndef TRIE_CHILD /* <!-- !sub-type */
	N_(Trie_)(0); N_(Trie)(0); N_(TrieFromArray)(0, 0, 0, 0); N_(TrieSize)(0);
	N_(TrieArray)(0); N_(TrieClear)(0); N_(TrieGet)(0, 0); N_(TrieClose)(0, 0);
	N_(TrieAdd)(0, 0); N_(TriePut)(0, 0, 0); N_(TriePolicyPut)(0, 0, 0, 0);
	N_(TrieRemove)(0, 0); N_(TrieShrink)(0);
#endif /* !sub-type --> */
	PN_(unused_base_coda)();
}
static void PN_(unused_base_coda)(void) { PN_(unused_base)(); }


#elif defined(TRIE_TO_STRING) /* base code --><!-- to string interface */


#if !defined(N_) || !defined(PN_) || !defined(CAT) \
	|| !defined(CAT_) || !defined(PCAT) || !defined(PCAT_)
#error P?N_? or P?CAT_? not yet defined in to string interface; include trie?
#endif

#ifdef TRIE_TO_STRING_NAME /* <!-- name fixme: define anonymous */
#define PNA_(thing) PCAT(PT_(thing), TRIE_TO_STRING_NAME)
#define N_A_(thing1, thing2) CAT(N_(thing1), CAT(TRIE_TO_STRING_NAME, thing2))
#else /* name --><!-- !name */
#define PNA_(thing) PCAT(PT_(thing), anonymous)
#define N_A_(thing1, thing2) CAT(N_(thing1), thing2)
#endif /* !name --> */

/* Check that `TRIE_TO_STRING` is a function implementing
 <typedef:<PN>ToString>. */
static const PN_(ToString) PNA_(to_str12) = (TRIE_TO_STRING);

/** Writes `it` to `str` and advances or returns false.
 @implements <AI>NextToString */
static int PNA_(next_to_str12)(struct PT_(Iterator) *const it,
	char (*const str)[12]) {
	assert(it && it->a && str);
	if(it->i >= it->a->size) return 0;
	PNA_(to_str12)(it->a->data[it->i++], str);
	return 1;
}

/** @return If `it` contains a not-null pool. */
static int PNA_(is_valid)(const struct PT_(Iterator) *const it)
	{ assert(it); return !!it->a; }

#define AI_ PNA_
#define TO_STRING_ITERATOR struct PT_(Iterator)
#define TO_STRING_NEXT &PNA_(next_to_str12)
#define TO_STRING_IS_VALID &PNA_(is_valid)
#include "ToString.h"

/** @return Prints `trie`. */
static const char *PNA_(to_string)(const struct N_(Trie) *const trie) {
	struct PT_(Iterator) it = { 0, 0 };
	it.a = trie ? &trie->leaves : 0; /* Can be null. */
	return PNA_(iterator_to_string)(&it, '(', ')'); /* In ToString. */
}

#ifndef TRIE_CHILD /* <!-- !sub-type */

/** @return Print the contents of `trie` in a static string buffer with the
 limitations of `ToString.h`. @order \Theta(1) @allow */
static const char *N_A_(Trie, ToString)(const struct N_(Trie) *const trie)
	{ return PNA_(to_string)(trie); /* Can be null. */ }

#endif /* !sub-type --> */

static void PNA_(unused_to_string_coda)(void);
static void PNA_(unused_to_string)(void) {
	PNA_(to_string)(0);
#ifndef ARRAY_CHILD /* <!-- !sub-type */
	N_A_(Trie, ToString)(0);
#endif /* !sub-type --> */
	PNA_(unused_to_string_coda)();
}
static void PNA_(unused_to_string_coda)(void) { PNA_(unused_to_string)(); }

#if !defined(TRIE_TEST_BASE) && defined(TRIE_TEST) /* <!-- test */
#define TRIE_TEST_BASE /* Only one instance of base tests. */
#include "../test/TestTrie.h" /** \include */
#endif /* test --> */

#undef PNA_
#undef N_A_
#undef TRIE_TO_STRING
#ifdef TRIE_TO_STRING_NAME
#undef TRIE_TO_STRING_NAME
#endif


#endif /* interfaces --> */


#ifdef TRIE_UNFINISHED /* <!-- unfinish */
#undef TRIE_UNFINISHED
#else /* unfinish --><!-- finish */
#ifndef TRIE_CHILD /* <!-- !sub-type */
#undef CAT
#undef CAT_
#undef PCAT
#undef PCAT_
#else /* !sub-type --><!-- sub-type */
#undef TRIE_CHILD
#endif /* sub-type --> */
#undef N_
#undef PN_
#undef PT_
#undef TRIE_NAME
#undef TRIE_TYPE
#undef TRIE_KEY
#ifdef TRIE_TEST
#undef TRIE_TEST
#endif
#ifdef TRIE_TEST_BASE
#undef TRIE_TEST_BASE
#endif
#endif /* finish --> */

#undef TRIE_INTERFACES
