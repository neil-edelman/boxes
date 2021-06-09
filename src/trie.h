/** @license 2020 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Prefix Tree

 ![Example of trie.](../web/trie.png)

 A <tag:<N>trie> is a prefix tree, digital tree, or trie, implemented as an
 array of pointers-to-`N` whose keys are always in lexicographically-sorted
 order and index on the keys. It can be seen as a <Morrison, 1968 PATRICiA>: a
 compact [binary radix trie](https://en.wikipedia.org/wiki/Radix_tree), only
 storing the where the keys are different. Strings can be any encoding with a
 byte null-terminator, (`C` strings,) including
 [modified UTF-8](https://en.wikipedia.org/wiki/UTF-8#Modified_UTF-8).

 `Array.h` must be present. `<N>Trie` is not synchronised. Errors are returned
 with `errno`. The parameters are `#define` preprocessor macros, and are all
 undefined at the end of the file for convenience. `assert.h` is used.

 @param[TRIE_NAME, TRIE_TYPE]
 <typedef:<PN>type> that satisfies `C` naming conventions when mangled and an
 optional returnable type that is declared, (it is used by reference only
 except if `TRIE_TEST`.) `<PN>` is private, whose names are prefixed in a
 manner to avoid collisions; any should be re-defined prior to use elsewhere.

 @param[TRIE_KEY]
 A function that satisfies <typedef:<PN>key_fn>. Must be defined if and only if
 `TRIE_TYPE` is defined.

 @param[TRIE_TO_STRING]
 Defining this includes `ToString.h` with the keys as the string.

 @param[TRIE_TEST]
 Unit testing framework <fn:<N>trie_test>, included in a separate header,
 <../test/TreeTest.h>. Must be defined equal to a (random) filler function,
 satisfying <typedef:<PN>action_fn>. Requires that `NDEBUG` not be defined.

 @fixme Have a replace; potentially much less wastful then remove and add.
 @fixme Compression _a la_ Judy; 64 bits to store mostly 0/1? Could it be done?
 @fixme Don't put two strings side-by-side or delete one that causes two
 strings to be side-by-side that have more than 512 matching characters in the
 same bit-positions, it will trip an `assert`. (Genomic data, perhaps?)
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
/*
#define TRIE_TRAITS (defined(TRIE_TO_STRING))
#if TRIE_TRAITS > 1
#error Only one interface per include is allowed; use TRIE_EXPECT_TRAIT.
#endif
#if TRIE_TRAITS != 0 && (!defined(N_) || !defined(CAT) || !defined(CAT_))
#error N_ or CAT_? not yet defined; use TRIE_EXPECT_TRAIT?
#endif
#if (TRIE_TRAITS == 0) && defined(TRIE_TEST)
#error TRIE_TEST must be defined in TRIE_TO_STRING interface.
#endif
*/
#if defined(TRIE_TEST) && !defined(TRIE_TO_STRING)
#error TRIE_TEST requires TRIE_TO_STRING.
#endif
#define TRIE_TRAITS 0


#if TRIE_TRAITS == 0 /* <!-- base code */


/* <Kernighan and Ritchie, 1988, p. 231>; before idempotent _st_ `CAT`. */
#if defined(N_) || defined(PN_) \
	|| (defined(TRIE_SUBTYPE) ^ (defined(CAT) || defined(CAT_)))
#error Unexpected P?N_ or CAT_?; possible stray TRIE_EXPECT_TRAIT?
#endif
#ifndef TRIE_SUBTYPE /* <!-- !sub-type */
#define CAT_(x, y) x ## _ ## y
#define CAT(x, y) CAT_(x, y)
#endif /* !sub-type --> */
#define N_(thing) CAT(TRIE_NAME, thing)
#define PN_(thing) CAT(trie, N_(thing))

#ifndef TRIE_H /* <!-- idempotent */
#define TRIE_H

/* Trie internal nodes encode the branches semi-implicitly. Each contains two
 items of information in a `size_t`: left children branches are <fn:trie_left>
 immediately following, right children are the rest; <fn:trie_skip>, the length
 of the string trie branch in bytes beyond the first byte, don't care values
 for the index. */
typedef size_t TrieBranch;

#define ARRAY_NAME trie_branch
#define ARRAY_TYPE TrieBranch
#define ARRAY_SUBTYPE
#include "array.h"

/* 12 makes the maximum skip length 512 bytes and the maximum size of a trie is
 `size_t` 64-bits: 4503599627370496, 32-bits: 1048576, 16-bits: 16, 8-bits: not
 supported at all, (unlikely since `C++` has additional constraints.) */
#define TRIE_SKIP 12
#define TRIE_SKIP_MAX ((1 << TRIE_SKIP) - 1)
#define TRIE_LEFT_MAX (((size_t)1 << ((sizeof(size_t) << 3) - TRIE_SKIP)) - 1)

/** @return Packs `skip` and `left` into a branch. */
static TrieBranch trie_branch(const size_t skip, const size_t left) {
	assert(skip <= TRIE_SKIP_MAX && left <= TRIE_LEFT_MAX);
	return skip + (left << TRIE_SKIP);
}

/** @return Unpacks skip from `branch`. */
static size_t trie_skip(const TrieBranch branch)
	{ return branch & TRIE_SKIP_MAX; }

/** @return Unpacks left descendent branches from `branch`. */
static size_t trie_left(const TrieBranch branch) { return branch >> TRIE_SKIP; }

/** Overwrites `skip` in `branch`. */
static void trie_skip_set(size_t *const branch, size_t skip) {
	assert(branch && skip <= TRIE_SKIP_MAX);
	*branch &= ~TRIE_SKIP_MAX;
	*branch += skip;
}

/** Increments the left descendants `branch` count. */
static void trie_left_inc(size_t *const branch) {
	assert(branch && *branch < ~(size_t)TRIE_SKIP_MAX);
	*branch += TRIE_SKIP_MAX + 1;
}

/** Decrements the left descendants `branch` count. */
static void trie_left_dec(size_t *const branch) {
	assert(branch && *branch > TRIE_SKIP_MAX);
	*branch -= TRIE_SKIP_MAX + 1;
}

/** Compares `bit` from the string `a` against `b`.
 @return In the `bit` position, positive if `a` is after `b`, negative if `a`
 is before `b`, or zero if `a` is equal to `b`. */
static int trie_strcmp_bit(const char *const a, const char *const b,
	const size_t bit) {
	const size_t byte = bit >> 3, mask = 128 >> (bit & 7);
	return !(b[byte] & mask) - !(a[byte] & mask);
}

/** From string `a`, extract `bit`. */
static int trie_is_bit(const char *const a, const size_t bit) {
	const size_t byte = bit >> 3, mask = 128 >> (bit & 7);
	return !!(a[byte] & mask);
}

/** @return Whether `a` and `b` are equal up to the minimum of their lengths'.
 Used in <fn:<N>trie_prefix>. */
static int trie_is_prefix(const char *a, const char *b) {
	for( ; ; a++, b++) {
		if(*a == '\0') return 1;
		if(*a != *b) return *b == '\0';
	}
}

#endif /* idempotent --> */

/* Defaults. */
#ifndef TRIE_TYPE /* <!-- !type */
typedef char PN_(vtype);
typedef const char PN_(type);
typedef const char PN_(ctype);
#ifndef TRIE_RAW /* <!-- !raw */
#define TRIE_RAW /* Idempotent function. */
/** @return The `key`, which is the string itself in the case where one doesn't
 specify `TRIE_TYPE`. */
static const char *trie_raw(const char *const key) { return key; }
#endif /* !raw --> */
#define TRIE_KEY &trie_raw
#else /* !type --><!-- type */
/* Variable type for <ToString.h> because duplicate const on `const char`. */
typedef TRIE_TYPE PN_(vtype);
/** A valid tag type set by `TRIE_TYPE`; defaults to `const char`. */
typedef TRIE_TYPE PN_(type);
/** Same as <typedef:<PN>type>, except read-only. */
typedef const TRIE_TYPE PN_(ctype);
#endif /* type --> */

/* Used internally to get rid of the confusing double-pointers. */
typedef PN_(type) *PN_(leaf);

/** Responsible for picking out the null-terminated string. One must not modify
 this string while in any trie. */
typedef const char *(*PN_(key_fn))(PN_(ctype) *);

/* Check that `TRIE_KEY` is a function implementing <typedef:<PN>key_fn>. */
static const PN_(key_fn) PN_(to_key) = (TRIE_KEY);

/** A bi-predicate; returns true if the `replace` replaces the `original`; used
 in <fn:<N>trie_policy_put>. */
typedef int (*PN_(replace_fn))(PN_(type) *original, PN_(type) *replace);

/** @return False. Ignores `a` and `b`. @implements <typedef:<PN>replace_fn> */
static int PN_(false_replace)(PN_(type) *const a, PN_(type) *const b)
	{ return (void)a, (void)b, 0; }

/** Compares keys of `a` and `b`. Used in array compare following.
 @implements bipredicate function */
static int PN_(compare)(const PN_(leaf) *const a, const PN_(leaf) *const b)
	{ return strcmp(PN_(to_key)(*a), PN_(to_key)(*b)); }

/* Trie leaf array is sorted by key. */
#define ARRAY_NAME PN_(leaf)
#define ARRAY_TYPE PN_(leaf)
#define ARRAY_SUBTYPE
#define ARRAY_EXPECT_TRAIT
#include "array.h"
#define ARRAY_COMPARE &PN_(compare)
#include "array.h"
#define T_(thing) CAT(PN_(leaf), thing)
#define PT_(thing) CAT(CAT(array, PN_(leaf)), thing)

/** To initialise it to an idle state, see <fn:<N>trie>, `TRIE_IDLE`, `{0}`
 (`C99`), or being `static`.

 A full binary tree stored semi-implicitly in two Arrays: as `branches` backed
 by one as pointers-to-<typedef:<PN>type> as `leaves` in
 lexicographically-sorted order.

 ![States.](../web/states.png) */
struct N_(trie)
	{ struct trie_branch_array branches; struct PN_(leaf_array) leaves; };
#ifndef TRIE_IDLE /* <!-- !zero */
#define TRIE_IDLE { ARRAY_IDLE, ARRAY_IDLE }
#endif /* !zero --> */

/** Initialises `trie` to idle. @order \Theta(1) @allow */
static void N_(trie)(struct N_(trie) *const trie) {
	assert(trie);
	trie_branch_array(&trie->branches), T_(array)(&trie->leaves);
}

/** Returns an initialised `trie` to idle. @allow */
static void N_(trie_)(struct N_(trie) *const trie) {
	assert(trie);
	trie_branch_array_(&trie->branches), T_(array_)(&trie->leaves);
}

/** Recursive function used for <fn:<PN>init>. Initialise branches of `trie` up
 to `bit` with `a` to `a_size` array of sorted leaves.
 @order Speed \O(`a_size` log E(`a.length`))?; memory \O(E(`a.length`)). */
static void PN_(init_branches_r)(struct N_(trie) *const trie, size_t bit,
	const size_t a, const size_t a_size) {
	size_t b = a, b_size = a_size, half;
	size_t skip = 0;
	TrieBranch *branch;
	assert(trie && a_size && a_size <= trie->leaves.size && trie->leaves.size
		&& trie->branches.capacity >= trie->leaves.size - 1);
	if(a_size <= 1) return;
	/* Endpoints of sorted range: skip [_1_111...] or [...000_0_] don't care.
	 fixme: UINT_MAX overflow. */
	while(trie_is_bit(PN_(to_key)(trie->leaves.data[a]), bit)
		|| !trie_is_bit(PN_(to_key)(trie->leaves.data[a + a_size - 1]), bit))
		bit++, skip++;
	/* Do a binary search for the first `leaves[a+half_s]#bit == 1`. */
	while(b_size) half = b_size >> 1,
		trie_is_bit(PN_(to_key)(trie->leaves.data[b + half]), bit)
		? b_size = half : (half++, b += half, b_size -= half);
	b_size = b - a;
	/* Should have space for all branches pre-allocated in <fn:<PN>init>. */
	branch = trie_branch_array_new(&trie->branches), assert(branch);
	*branch = trie_branch(skip, b_size - 1);
	bit++;
	PN_(init_branches_r)(trie, bit, a, b_size);
	PN_(init_branches_r)(trie, bit, b, a_size - b_size);
}

/** Initialises `trie` to `a` of size `a_size`, which cannot be zero.
 @return Success. @throws[ERANGE, malloc] */
static int PN_(init)(struct N_(trie) *const trie, PN_(type) *const*const a,
	const size_t a_size) {
	PN_(leaf) *leaves;
	assert(trie && a && a_size);
	N_(trie)(trie);
	/* This will store space for all of the duplicates, as well. */
	if(!T_(array_reserve)(&trie->leaves, a_size)
		|| !trie_branch_array_reserve(&trie->branches, a_size - 1)) return 0;
	leaves = trie->leaves.data;
	memcpy(leaves, a, sizeof *a * a_size);
	trie->leaves.size = a_size;
	/* Sort, get rid of duplicates, and initialise branches, from `Array.h`. */
	qsort(leaves, a_size, sizeof *a, &PT_(vcompar_anonymous));
	T_(array_unique)(&trie->leaves);
	PN_(init_branches_r)(trie, 0, 0, trie->leaves.size);
	assert(trie->branches.size + 1 == trie->leaves.size);
	return 1;
}

/** Initialises `trie` from an `array` of pointers-to-`<N>` of `array_size`.
 @return Success. @throws[realloc] @order \O(`array_size`) @allow */
static int N_(trie_from_array)(struct N_(trie) *const trie,
	PN_(type) *const*const array, const size_t array_size) {
	return assert(trie && array && array_size),
		PN_(init)(trie, array, array_size);
}

/** @return The number of elements in the `trie`. @order \Theta(1) @allow */
static size_t N_(trie_size)(const struct N_(trie) *const trie)
	{ return assert(trie), trie->leaves.size; }

/** It remains valid up to a structural modification of `trie` and is indexed
 up to <fn:<N>trie_size>.
 @return An array of pointers to the leaves of `trie`, ordered by key. @allow */
static PN_(type) *const*N_(trie_array)(const struct N_(trie) *const trie)
	{ return trie ? trie->leaves.data : 0; }

/** Sets `trie` to be empty. That is, the size of `trie` will be zero, but if
 it was previously in an active non-idle state, it continues to be.
 @order \Theta(1) @allow */
static void N_(trie_clear)(struct N_(trie) *const trie)
	{ assert(trie), trie->branches.size = trie->leaves.size = 0; }

/** Looks at only the index for potential matches.
 @param[result] A index pointer to leaves that matches `key` when true.
 @return True if `key` in `trie` has matched, otherwise `key` is definitely is
 not in `trie`. @order \O(`key.length`) */
static int PN_(param_index_get)(const struct N_(trie) *const trie,
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
static int PN_(param_get)(const struct N_(trie) *const trie,
	const char *const key, size_t *const result) {
	return PN_(param_index_get)(trie, key, result)
		&& !strcmp(PN_(to_key)(trie->leaves.data[*result]), key);
}

/** @return `trie` entry that matches bits of `key`, (ignoring the don't care
 bits,) or null if either `key` didn't have the length to fully differentiate
 more then one entry or the `trie` is empty. */
static PN_(type) *PN_(index_get)(const struct N_(trie) *const trie,
	const char *const key) {
	size_t i;
	return PN_(param_index_get)(trie, key, &i) ? trie->leaves.data[i] : 0;
}

/** @return Exact match for `key` in `trie` or null. */
static PN_(type) *PN_(get)(const struct N_(trie) *const trie,
	const char *const key) {
	size_t i;
	return PN_(param_get)(trie, key, &i) ? trie->leaves.data[i] : 0;
}

/** @return The <typedef:<PN>type> that matches `key` bits in `trie`, excluding
 don't-cares. @allow */
static PN_(type) *N_(trie_index_get)(const struct N_(trie) *const trie,
	const char *const key) {
	return assert(trie && key), PN_(index_get)(trie, key);
}

/** @return The <typedef:<PN>type> with `key` in `trie` or null no such item
 exists. @order \O(|`key`|), <Thareja 2011, Data>. @allow */
static PN_(type) *N_(trie_get)(const struct N_(trie) *const trie,
	const char *const key) {
	return assert(trie && key), PN_(get)(trie, key);
}

/** In `trie`, which must be non-empty, given a partial `prefix`, stores all
 leaf prefix matches between `low`, `high`, only given the index, ignoring
 don't care bits. @order \O(`prefix.length`) @allow */
static void N_(trie_index_prefix)(const struct N_(trie) *const trie,
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
static int N_(trie_prefix)(const struct N_(trie) *const trie,
	const char *const prefix, size_t *const low, size_t *const high) {
	assert(trie && prefix && low && high);
	return trie->leaves.size ? (N_(trie_index_prefix)(trie, prefix, low, high),
		trie_is_prefix(prefix, PN_(to_key)(trie->leaves.data[*low]))) : 0;
}

/** Add `datum` to `trie`. Must not be the same as any key of `trie`; _ie_ it
 does not check for the end of the string. @return Success. @order \O(|`trie`|)
 @throws[ERANGE] Trie reached it's conservative maximum, which on machines
 where the pointer is 64-bits, is 4.5T. On 32-bits, it's 1M.
 @throws[realloc, ERANGE] @fixme Throw EILSEQ if two strings have subsequences
 that are equal in more than 2^12 bits. */
static int PN_(add)(struct N_(trie) *const trie, PN_(type) *const datum) {
	const size_t leaf_size = trie->leaves.size, branch_size = leaf_size - 1;
	size_t n0 = 0, n1 = branch_size, i = 0, left, bit = 0, bit0 = 0, bit1;
	TrieBranch *branch = 0;
	const char *const data_key = PN_(to_key)(datum), *n0_key;
	PN_(leaf) *leaf;
	int cmp;
	assert(trie && datum);
	/* Empty special case. */
	if(!leaf_size) return assert(!trie->branches.size),
		(leaf = T_(array_new)(&trie->leaves)) ? *leaf = datum, 1 : 0;
	/* Redundant `size_t`, but maybe we will use it like Judy. */
	assert(leaf_size == branch_size + 1);
	/* Conservative maximally unbalanced trie. Reserve one more. */
	if(leaf_size >= TRIE_LEFT_MAX) return errno = ERANGE, 0;
	if(!T_(array_reserve)(&trie->leaves, leaf_size + 1)
		|| !trie_branch_array_reserve(&trie->branches, branch_size + 1))
		return 0;
	/* Branch from internal node. */
	while(branch = trie->branches.data + n0,
		n0_key = PN_(to_key)(trie->leaves.data[i]), n0 < n1) {
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

/** Adds `datum` to `trie` and, if `eject` is non-null, stores the collided
 element, if any, as long as `replace` is null or returns true.
 @param[eject] If not-null, the ejected datum. If `replace` returns false, then
 `*eject == datum`, but it will still return true.
 @return Success. @throws[realloc, ERANGE] */
static int PN_(put)(struct N_(trie) *const trie, PN_(type) *const datum,
	PN_(type) **const eject, const PN_(replace_fn) replace) {
	const char *data_key;
	PN_(leaf) *match;
	size_t i;
	assert(trie && datum);
	data_key = PN_(to_key)(datum);
	/* Add if absent. */
	if(!PN_(param_get)(trie, data_key, &i)) {
		if(eject) *eject = 0;
		return PN_(add)(trie, datum);
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
static int N_(trie_add)(struct N_(trie) *const trie, PN_(type) *const datum) {
	return assert(trie && datum), PN_(put)(trie, datum, 0, &PN_(false_replace));
}

/** Updates or adds `datum` to `trie`.
 @param[trie, datum] If null, returns null.
 @param[eject] If not null, on success it will hold the overwritten value or
 a pointer-to-null if it did not overwrite.
 @return Success.
 @throws[realloc] There was an error with a re-sizing.
 @throws[ERANGE] The key is greater then 510 characters or the trie has reached
 it's maximum size. @order \O(`size`) @allow */
static int N_(trie_put)(struct N_(trie) *const trie,
	PN_(type) *const datum, PN_(type) **const eject) {
	return assert(trie && datum), PN_(put)(trie, datum, eject, 0);
}

/** Adds `datum` to `trie` only if the entry is absent or if calling `replace`
 returns true.
 @param[eject] If not null, on success it will hold the overwritten value or
 a pointer-to-null if it did not overwrite a previous value. If a collision
 occurs and `replace` does not return true, this value will be `data`.
 @param[replace] Called on collision and only replaces it if the function
 returns true. If null, it is semantically equivalent to <fn:<N>trie_put>.
 @return Success. @throws[realloc] There was an error with a re-sizing.
 @throws[ERANGE] The key is greater then 510 characters or the trie has reached
 it's maximum size. @order \O(`size`) @allow */
static int N_(trie_policy_put)(struct N_(trie) *const trie,
	PN_(type) *const datum, PN_(type) **const eject,
	const PN_(replace_fn) replace) {
	return assert(trie && datum), PN_(put)(trie, datum, eject, replace);
}

/** @return Whether leaf index `i` has been removed from `trie`.
 @fixme There is nothing stopping an `assert` from being triggered. */
static int PN_(index_remove)(struct N_(trie) *const trie, size_t i) {
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
static int N_(trie_remove)(struct N_(trie) *const trie, const char *const key) {
	size_t i;
	assert(trie && key);
	return PN_(param_get)(trie, key, &i) && PN_(index_remove)(trie, i);
}

/** Shrinks the capacity of `trie` to size.
 @return Success. @throws[ERANGE, realloc] Unlikely `realloc` error. @allow */
static int N_(trie_shrink)(struct N_(trie) *const trie) {
	return assert(trie), trie_branch_array_shrink(&trie->branches)
		&& T_(array_shrink)(&trie->leaves);
}

/** Loads `trie` into `it`. @implements begin */
static void PN_(begin)(struct PT_(iterator) *const it,
	const struct N_(trie) *const trie)
	{ assert(it && trie), it->a = &trie->leaves, it->i = 0; }

/** Advances `it`. @implements next */
static PN_(ctype) *PN_(next)(struct PT_(iterator) *const it) {
	assert(it && it->a);
	return it->i < it->a->size ? it->a->data[it->i++] : 0;
}

#if defined(ITERATE) || defined(ITERATE_BOX) || defined(ITERATE_TYPE) \
	|| defined(ITERATE_BEGIN) || defined(ITERATE_NEXT)
#error Unexpected ITERATE*.
#endif
	
#define ITERATE struct PT_(iterator)
#define ITERATE_BOX struct N_(trie)
#define ITERATE_TYPE PN_(vtype)
#define ITERATE_BEGIN PN_(begin)
#define ITERATE_NEXT PN_(next)

static void PN_(unused_base_coda)(void);
static void PN_(unused_base)(void) {
	N_(trie)(0); N_(trie_)(0); N_(trie_from_array)(0, 0, 0); N_(trie_size)(0);
	N_(trie_array)(0); N_(trie_clear)(0); N_(trie_index_get)(0, 0);
	N_(trie_get)(0, 0); N_(trie_index_prefix)(0, 0, 0, 0);
	N_(trie_prefix)(0, 0, 0, 0); N_(trie_add)(0, 0); N_(trie_put)(0, 0, 0);
	N_(trie_policy_put)(0, 0, 0, 0); N_(trie_remove)(0, 0); N_(trie_shrink)(0);
	PN_(begin)(0, 0); PN_(unused_base_coda)();
}
static void PN_(unused_base_coda)(void) { PN_(unused_base)(); }


#ifdef TRIE_TO_STRING /* <!-- string */


/** Uses the natural `datum` -> `a` that is defined by `TRIE_KEY`. */
static void PN_(to_string)(PN_(ctype) *const datum, char (*const a)[12]) {
	assert(datum && a);
	sprintf(*a, "%.11s", PN_(to_key)(datum));
}

#define A_(thing) CAT(N_(trie), thing)
#define TO_STRING &PN_(to_string)
#include "to_string.h" /** \include */

#if !defined(TRIE_TEST_BASE) && defined(TRIE_TEST) /* <!-- test */
#define TRIE_TEST_BASE /* Only one instance of base tests. */
#include "../test/test_trie.h" /** \include */
#endif /* test --> */

#undef A_
#undef TRIE_TO_STRING

	
#endif /* string --> */


#endif /* base --> */


#ifdef TRIE_EXPECT_TRAIT /* <!-- trait */
#undef TRIE_EXPECT_TRAIT
#else /* trait --><!-- !trait */
#ifndef TRIE_SUBTYPE /* <!-- !sub-type */
#undef CAT
#undef CAT_
#else /* !sub-type --><!-- sub-type */
#undef TRIE_SUBTYPE
#endif /* sub-type --> */
#undef N_
#undef PN_
#undef T_
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
#undef ITERATE
#undef ITERATE_BOX
#undef ITERATE_TYPE
#undef ITERATE_BEGIN
#undef ITERATE_NEXT
#endif /* !trait --> */

#undef TRIE_TRAITS
