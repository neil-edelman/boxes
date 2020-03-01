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

#include <stddef.h>
#include <string.h>


#ifndef TRIE_H /* <!-- idempotent */
#define TRIE_H

/* An internal node; assume takes up one register for speed,
 `sizeof(struct TrieInternal) <= sizeof(size_t)` */
struct TrieInternal {
	unsigned choice_bit;
	unsigned char left_branch, right_branch;
};

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

/** Strict binary tree nodes in an array; either internal node (`branch`) which
 is always followed by an additive self-reference (`right_offset`,) or an
 external reference (`leaf`.) */
union PN_(TrieNode) {
	struct TrieInternal branch;
	size_t right_offset;
	PN_(Type) *leaf;
};

#define ARRAY_NAME PN_(TrieNode)
#define ARRAY_TYPE union PN_(TrieNode)
#define ARRAY_CHILD
#include "Array.h"

#define PT_(thing) PCAT(array, PCAT(PN_(TrieNode), thing))

/** To initialise it to an idle state, see <fn:<N>Tree>, `TRIE_IDLE`, `{0}`
 (`C99`), or being `static`. Internally, it is an array of <tag:<PN>TrieNode>.
 If size is zero, it's empty; otherwise it has `3 (nodes - 1) + 1` elements. If
 size is one, it's a leaf node, otherwise it's always a branch.

 ![States.](../web/states.png) */
struct N_(Trie);
struct N_(Trie) { struct PN_(TrieNodeArray) a; };
#ifndef TRIE_IDLE /* <!-- !zero */
#define TRIE_IDLE { { 0, 0, 0, 0 } }
#endif /* !zero --> */

/** Debug. Only print one at a time. */
static const char *PN_(to_desc)(const struct N_(Trie) *const trie,
	const union PN_(TrieNode) *const n, const int is_branch) {
	static char a[256];
	assert(trie);
	assert(!n || n >= trie->a.data);
	if(!n) {
		sprintf(a, "node_null");
	} else if(n >= trie->a.data + trie->a.size) {
		sprintf(a, "node_%lu one-past end", n - trie->a.data);
	} else if(is_branch) {
		/*size_t ni = n - trie->a.data, offset = n[1].right_offset;
		union PN_(TrieNode) *const end = trie->a.data + trie->a.size;*/
		sprintf(a, "node_%lu: choice_bit %u, %s<+2>:%s<+%ld>",
			n - trie->a.data, n->branch.choice_bit,
			n->branch.left_branch ? "branch" : "leaf",
			n->branch.right_branch ? "branch" : "leaf", n[1].right_offset);
		/*assert(n->branch.left_branch <= 1 && n->branch.right_branch <= 1);
		assert(n < trie->a.data + trie->a.size - 1);
		assert(n + 1 + offset < end);*/
	} else {
		sprintf(a, "node_%lu: \"%s\"", n - trie->a.data, PN_(to_key)(n->leaf));
	}
	return a;
}

/** We would have made it `4 (nodes - 1) + 1` with \O(`1`) iteration, not sure
 if it's possible with 3.
 @order \O(`size`); avererge \O(log `size`). */
static PN_(Type) *PN_(iterate)(struct N_(Trie) *const trie, size_t *const it) {
	struct PN_(TrieNodeArray) *a;
	size_t target, n, on;
	int branch;
	assert(it);
	/* No trie or done iterations. */
	if(!trie || (a = &trie->a, target = *it, target >= a->size)) return 0;
	/* Special case of a trie with size one leaf. */
	if(a->size == 1) { *it = 1; return a->data[0].leaf; }
	/* Start at the top and traverse internal nodes until we hit a leaf. */
	n = 0;
	do {
		on = n + 1, on = on + a->data[on].right_offset;
		if(on > target) {
			branch = a->data[n].branch.left_branch;
			n += 2;
		} else {
			branch = a->data[n].branch.right_branch;
			n = on;
		}
	} while(branch);
	return a->data[*it = n].leaf;
}

/** Initialises `trie`. */
static void PN_(trie)(struct N_(Trie) *const trie)
	{ assert(trie); PT_(array)(&trie->a); }

/** Destructor of `trie`. */
static void PN_(trie_)(struct N_(Trie) *const trie)
	{ assert(trie); free(trie->a.data); PN_(trie)(trie); }

/** @return The key at the leaf taken from `node` all the way to the left.
 This is more cache efficient then the right on average.
 @order O(`nodes`) */
static const char *PN_(node_key)(const union PN_(TrieNode) *node,
	const int is_node_branch) {
	if(is_node_branch) { while(node->branch.left_branch) node += 2; node += 2; }
	return PN_(to_key)(node->leaf);
}

/** Add `data` to `trie`. This assumes that the key of `data` is not the same
 as any in `trie`, so make sure before calling this or else it may crash, (it
 doesn't do `NUL` checks.)
 @order O(`nodes`) */
static int PN_(add)(struct N_(Trie) *const trie, PN_(Type) *const data) {
	union PN_(TrieNode) *n1, *n2;
	unsigned char is_n1_branch;
	const char *const data_key = PN_(to_key)(data), *n1_key;
	unsigned bit = 0;
	int cmp;
	assert(trie && data);
	if(trie->a.size == 0) /* Empty special case. */
		{ if(!(n1 = PT_(new)(&trie->a, 0))) return 0; n1->leaf=data; return 1; }
	/* Partition the array into three consecutive disjoint sets,
	 `[0,n1)`, `[n1,n2)`, `[n2,-1]`. */
	assert((trie->a.size - 1) % 3 == 0 && trie->a.size < (size_t)-3);
	if(!PT_(reserve)(&trie->a, trie->a.size + 3, 0)) return 0;
	n1 = trie->a.data, n2 = 0;
	n1_key = PN_(node_key)(n1, is_n1_branch = trie->a.size > 1), assert(n1_key);
	for( ; ; ) {
		/* Compare bit-by-bit. If it's a leaf, there no upper bound. */
		while((!is_n1_branch || bit < n1->branch.choice_bit)
			&& (cmp = trie_strcmp_bit(data_key, n1_key, bit)) == 0) bit++;
		/* Leaf or bit difference; otherwise follow the branch. */
		if(!is_n1_branch || bit != n1->branch.choice_bit) break;
		if(!trie_is_bit(data_key, bit)) { /* Descend left. */
			n2 = n1 + 1 + n1[1].right_offset;
			is_n1_branch = n1->branch.left_branch;
			n1->branch.left_branch = 1;
			n1[1].right_offset += 3; /* Future trie with another leaf. */
			n1 += 2;
		} else { /* Descend right. */
			is_n1_branch = n1->branch.right_branch;
			n1->branch.right_branch = 1;
			n1 = n1 + 1 + n1[1].right_offset;
			n1_key = PN_(node_key)(n1, is_n1_branch);
			assert(n1_key);
		}
	}
	if(!n2) n2 = trie->a.data + trie->a.size;
	assert(trie->a.data <= n1 && n1 < n2 && n2 <= trie->a.data + trie->a.size);
	if(cmp < 0) { /* Insert left leaf; `[n1,n2], [n2,-1]` are moved together. */
		memmove(n1 + 3, n1, sizeof n1 * (trie->a.data + trie->a.size - n1));
		n1[0].branch.choice_bit = bit;
		n1[0].branch.left_branch  = 0;
		n1[0].branch.right_branch = is_n1_branch;
		n1[1].right_offset = 2;
		n1[2].leaf = data;
	} else { /* Insert a right leaf. */
		memmove(n2 + 3, n2, sizeof n1 * (trie->a.data + trie->a.size - n2));
		memmove(n1 + 2, n1, sizeof n1 * (n2 - n1));
		n1[0].branch.choice_bit = bit;
		n1[0].branch.left_branch  = is_n1_branch;
		n1[0].branch.right_branch = 0;
		n1[1].right_offset = n2 - n1 + 1;
		n2[2].leaf = data;
	}
	trie->a.size += 3;
	return 1;
}

/** Doesn't look at the underlying data, but goes through only the index trie
 deciding bits.
 @return `TrieNode` leaf that potentially matches `str` or null, definitely
 isn't in the trie. */
static union PN_(TrieNode) *PN_(match)(const struct N_(Trie) *const trie,
	const char *const str) {
	union PN_(TrieNode) *n = trie->a.data;
	unsigned bit, byte, str_byte = 0;
	unsigned char branch = trie->a.size > 1;
	assert(trie && str);
	if(!trie->a.size) return 0;
	while(branch) {
		/* Don't go farther than the string. */
		bit = n->branch.choice_bit;
		for(byte = bit >> 3; str_byte < byte; str_byte++)
			if(str[str_byte] == '\0') return 0;
		/* Otherwise, choose the branch. */
		if(trie_is_bit(str, bit)) {
			branch = n->branch.right_branch;
			n = n + 1 + n[1].right_offset;
		} else {
			branch = n->branch.left_branch;
			n += 2;
		}
		assert(n < trie->a.data + trie->a.size);
	}
	return n;
}

static union PN_(TrieNode) *PN_(get)(const struct N_(Trie) *const trie,
	const char *const str) {
	union PN_(TrieNode) *match;
	assert(trie && str);
	return (match = PN_(match)(trie, str))
		&& !strcmp(PN_(to_key)(match->leaf), str) ? match : 0;
}

/** Adds `data` to `trie` and, if `eject` is non-null, stores the collided
 element, if any, as long as `replace` is null or returns true.
 @param[eject] If not-null, the reference will be set to null if there is no
 ejection. If `replace`, and `replace` returns false, and `eject`, than
 `*eject == data`. */
static int PN_(put)(struct N_(Trie) *const trie, PN_(Type) *const data,
	PN_(Type) **const eject, const PN_(Replace) replace) {
	union PN_(TrieNode) *match;
	const char *const data_key = PN_(to_key)(data);
	assert(trie && data);
	if(!trie || !data) return 0;
	if(!(match = PN_(get)(trie, data_key))) {
		if(eject) *eject = 0;
		return PN_(add)(trie, data);
	}
	/* Collision policy. */
	if(replace && !replace(match->leaf, data)) {
		if(eject) *eject = data;
	} else {
		if(eject) *eject = match->leaf;
		match->leaf = data;
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
	return trie && trie->a.size ? 1 + (trie->a.size - 1) / 3 : 0;
}

/** Sets `trie` to be empty. That is, the size of `trie` will be zero, but if
 it was previously in an active non-idle state, it continues to be.
 @param[trie] If null, does nothing.
 @order \Theta(1)
 @allow */
static void N_(TrieClear)(struct N_(Trie) *const trie) {
	if(trie) trie->a.size = 0;
}

static PN_(Type) *N_(TrieGet)(const struct N_(Trie) *const trie,
	const char *const str) {
	union PN_(TrieNode) *n;
	return trie && str && (n = PN_(get)(trie, str)) ? n->leaf : 0;
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
	size_t i, it;
	PN_(Type) *element;
	const char *str;
	int is_first = 1;
	assert(!(buffers_no & (buffers_no - 1)) && ellipsis_end_len >= 1
		&& buffer_size >= 1 + 11 + ellipsis_end_len + 1
		&& buffer_size >= null_len + 1
		&& buffer_size >= idle_len + 1);
	/* Advance the buffer for next time. */
	buffer_i &= buffers_no - 1;
	if(!trie) { memcpy(b, null, null_len), b += null_len; goto terminate; }
	if(!trie->a.data) { memcpy(b, idle, idle_len), b += idle_len;
		goto terminate; }
	*b++ = start;
	for(it = 0; (element = PN_(iterate)(trie, &it)); it++) {
		if(!is_first) *b++ = comma, *b++ = space;
		else is_first = 0;
		str = PN_(to_key)(element);
		for(i = 0; *str != '\0' && i < 12; str++, b++, i++) *b = *str;
		/* fixme: if(++e >= e_end) break; */
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
