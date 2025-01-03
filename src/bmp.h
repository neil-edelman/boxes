/** @license 2021 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @abstract Header <../../src/bmp.h>; examples <../../test/test_bmp.c>.

 @subtitle Fixed bit-field

 `<t>bmp` is a bit-field of `BMP_BITS` bits. The representation in memory is
 most-signifiant bit first.

 @param[BMP_NAME, BMP_BITS]
 `<t>` that satisfies `C` naming conventions when mangled and a number of bits
 associated therewith, which must be positive; required.

 @depend [box](../../src/box.h)
 @std C89/90 */

#if !defined BMP_NAME || !defined BMP_BITS
#	error Name or unsigned number bits undefined.
#endif
#if BMP_BITS < 1
#	error BMP_BITS too small.
#endif

#define BOX_START
#include "box.h"

#ifdef BMP_NON_STATIC
#	define BOX_NON_STATIC
#endif
#ifdef BMP_DECLARE_ONLY
#	define BOX_DECLARE_ONLY
#endif
#ifndef BMP_H
#	define BMP_H
/** The underlying array type. */
typedef unsigned bmpchunk;
#endif

/* <http://c-faq.com/misc/bitsets.html>, except reversed for msb-first. */
#define BMP_MAX (~(bmpchunk)0)
#define BMP_CHUNK (sizeof(bmpchunk) * CHAR_BIT)
#define BMP_CHUNKS (((BMP_BITS) - 1) / BMP_CHUNK + 1)
#define BMP_CHUNK_HI (1u << BMP_CHUNK - 1)
#define BMP_MASK(x) (BMP_CHUNK_HI >> (x) % (unsigned)BMP_CHUNK)
#define BMP_SLOT(x) ((x) / (unsigned)BMP_CHUNK)
#define BMP_AT(a, x) ((a)[BMP_SLOT(x)] & BMP_MASK(x))
#define BMP_DIFF(a, b, x) (((a)[BMP_SLOT(x)] ^ (b)[BMP_SLOT(x)]) & BMP_MASK(x))
#define BMP_SET(a, x) ((a)[BMP_SLOT(x)] |= BMP_MASK(x))
#define BMP_CLEAR(a, x) ((a)[BMP_SLOT(x)] &= ~(BMP_MASK(x)))
#define BMP_TOGGLE(a, x) ((a)[BMP_SLOT(x)] ^= BMP_MASK(x))

#include <string.h>
#include <limits.h>
#include <assert.h>

#define BOX_MINOR BMP_NAME
#define BOX_MAJOR bmp

/** An array of `BMP_BITS` bits, (taking up the next multiple of
 `sizeof(bmpchunk)` \times `CHARBIT`.) */
struct t_(bmp) { bmpchunk chunk[BMP_CHUNKS]; };

#ifdef BMP_NON_STATIC /* Public functions. */
void T_(clear_all)(struct t_(bmp) *);
void T_(copy)(struct t_(bmp) *, const unsigned, const unsigned,
	struct t_(bmp) *);
void T_(invert_all)(struct t_(bmp) *);
static unsigned T_(test)(const struct t_(bmp) *, unsigned);
void T_(set)(struct t_(bmp) *, unsigned);
void T_(clear)(struct t_(bmp) *, unsigned);
void T_(toggle)(struct t_(bmp) *, unsigned);
void T_(insert)(struct t_(bmp) *, unsigned, unsigned);
void T_(remove)(struct t_(bmp) *, unsigned, unsigned);
#endif
#ifndef BMP_DECLARE_ONLY /* Produce code. */

#	define BOX_PUBLIC_OVERRIDE
#	include "box.h"

/** Sets `a` to all false. @allow */
static void T_(clear_all)(struct t_(bmp) *const a)
	{ assert(a); memset(a, 0, sizeof *a); }

/** Copies and overwrites `b` with `bit_offset` range `bit_range` from `a`.
 Left over is filled with zeros. `bit_range` cannot be zero. @fixme Untested. */
static void T_(copy)(struct t_(bmp) *const a, const unsigned bit_offset,
	const unsigned bit_range, struct t_(bmp) *const b) {
	struct { unsigned hi, lo; } i, j;
	unsigned rest;
	assert(a && b && bit_range && bit_offset + bit_range < BMP_BITS);
	i.hi = bit_offset / BMP_CHUNK, i.lo = bit_offset % BMP_CHUNK;
	j.hi = j.lo = 0;
	for(rest = bit_range; rest > BMP_CHUNK; j.hi++, rest -= BMP_CHUNK)
		b->chunk[j.hi] = (a->chunk[i.hi + j.hi] << i.lo)
		| (a->chunk[i.lo + j.lo + 1] >> (8 - i.lo));
	b->chunk[j.hi] = a->chunk[i.hi + j.hi] << i.lo;
	if(i.hi + j.hi < (bit_offset + bit_range) / BMP_CHUNK)
		b->chunk[j.hi] |= (a->chunk[i.hi + j.hi + 1] >> (8 - i.lo));
	b->chunk[j.hi++] &= ~(BMP_MAX >> rest);
	memset(b + j.hi, 0, BMP_CHUNK - j.hi);
}

/** Inverts all entries of `a`. @allow */
static void T_(invert_all)(struct t_(bmp) *const a) {
	size_t i;
	assert(a);
	for(i = 0; i < sizeof a->chunk / sizeof *a->chunk; i++)
		a->chunk[i] = ~a->chunk[i];
	/* Obsessively zero padded bits. */
	a->chunk[BMP_CHUNKS - 1]
		&= ~((1u << sizeof a->chunk * CHAR_BIT - BMP_BITS) - 1);
}

/** @return Projects the eigenvalue of bit `x` of `a`. Either zero of
 non-zero, but not necessarily one. @allow */
static unsigned T_(test)(const struct t_(bmp) *const a, const unsigned x)
	{ assert(a && x < BMP_BITS); return BMP_AT(a->chunk, x); }

/** Sets bit `x` in `a`. @allow */
static void T_(set)(struct t_(bmp) *const a, const unsigned x)
	{ assert(a && x < BMP_BITS); BMP_SET(a->chunk, x); }

/** Clears bit `x` in `a`. @allow */
static void T_(clear)(struct t_(bmp) *const a, const unsigned x)
	{ assert(a && x < BMP_BITS); BMP_CLEAR(a->chunk, x); }

/** Toggles bit `x` in `a`. @allow */
static void T_(toggle)(struct t_(bmp) *const a, const unsigned x)
	{ assert(a && x < BMP_BITS); BMP_TOGGLE(a->chunk, x); }

/** Inserts `n` zeros at `x` in `a`. The `n` right bits are discarded. @allow */
static void T_(insert)(struct t_(bmp) *const a,
	const unsigned x, const unsigned n) {
	struct { unsigned hi, lo; } move, first;
	unsigned i;
	bmpchunk store, temp;
	assert(a && x + n <= BMP_BITS);
	if(!n) return;
	move.hi = n / BMP_CHUNK, move.lo = n % BMP_CHUNK;
	first.hi = x / BMP_CHUNK, first.lo = x % BMP_CHUNK;
	i = BMP_CHUNKS - move.hi - (move.lo ? 1 : 0); store = a->chunk[first.hi];
	/* Zero the bits that are not involved on the last iteration. */
	a->chunk[first.hi] &= BMP_MAX >> first.lo;
	/* Copy a superset aligned with `<PM>chunk` bits, backwards. */
	for( ; ; ) {
		temp = a->chunk[i] >> move.lo;
		if(i == first.hi) { a->chunk[i + move.hi] = temp; break; }
		if(move.lo) temp |= a->chunk[i - 1] << BMP_CHUNK - move.lo;
		a->chunk[i-- + move.hi] = temp;
	}
	/* Zero intervening, restore the bits that are not involved, and clip. */
	for(i = 0; i < move.hi; i++) a->chunk[first.hi + i] = 0;
	a->chunk[first.hi] |= ~(BMP_MAX >> first.lo) & store;
	a->chunk[BMP_CHUNKS - 1]
		&= ~((1u << sizeof a->chunk * CHAR_BIT - BMP_BITS) - 1);
}

/** Removes `n` at `x` in `a`. The `n` bits coming from the right are zero.
 @allow */
static void T_(remove)(struct t_(bmp) *const a,
	const unsigned x, const unsigned n) {
	struct { unsigned hi, lo; } move, first;
	unsigned i;
	bmpchunk store, temp;
	assert(a && x + n <= BMP_BITS);
	if(!n) return;
	move.hi = n / BMP_CHUNK, move.lo = n % BMP_CHUNK;
	first.hi = x / BMP_CHUNK, first.lo = x % BMP_CHUNK;
	i = first.hi + move.hi; store = a->chunk[first.hi];
	/* Copy a superset aligned with `<PM>chunk` bits. */
	for( ; ; ) {
		temp = a->chunk[i] << move.lo;
		if(i >= BMP_CHUNKS - 1) { a->chunk[i - move.hi] = temp; break; }
		if(move.lo) temp |= a->chunk[i + 1] >> BMP_CHUNK - move.lo;
		a->chunk[i++ - move.hi] = temp;
	}
	/* Zero intervening, restore the bits that are not involved, and clip. */
	for(i = BMP_CHUNKS - move.hi; i < BMP_CHUNKS; i++) a->chunk[i] = 0;
	/* <https://graphics.stanford.edu/~seander/bithacks.html#MaskedMerge> */
	a->chunk[first.hi] ^= (a->chunk[first.hi] ^ store) & ~(BMP_MAX >> first.lo);
	a->chunk[BMP_CHUNKS - 1]
		&= ~((1u << sizeof a->chunk * CHAR_BIT - BMP_BITS) - 1);
}

#	define BOX_PRIVATE_AGAIN
#	include "box.h"

static void pT_(unused_base_coda)(void);
static void pT_(unused_base)(void) {
	T_(clear_all)(0); T_(test)(0, 0); T_(copy)(0, 0, 0, 0); T_(invert_all)(0);
	T_(set)(0, 0); T_(clear)(0, 0); T_(toggle)(0, 0); T_(insert)(0, 0, 0);
	T_(remove)(0, 0, 0);
	pT_(unused_base_coda)();
}
static void pT_(unused_base_coda)(void) { pT_(unused_base)(); }
#endif

#ifdef BMP_TEST
#	include "../test/test_bmp.h" /* (not needed) \include */
#endif

#undef BMP_MAX
#undef BMP_CHUNK
#undef BMP_CHUNKS
#undef BMP_CHUNK_HI
#undef BMP_MASK
#undef BMP_SLOT
#undef BMP_AT
#undef BMP_DIFF
#undef BMP_SET
#undef BMP_CLEAR
#undef BMP_TOGGLE

#undef BOX_MINOR
#undef BOX_MAJOR
#undef BMP_NAME
#undef BMP_BITS
#ifdef BMP_TEST
#	undef BMP_TEST
#endif
#ifdef BMP_DECLARE_ONLY
#	undef BMP_DECLARE_ONLY
#	undef BOX_DECLARE_ONLY
#endif
#ifdef BMP_NON_STATIC
#	undef BMP_NON_STATIC
#	undef BOX_DECLARE_ONLY
#endif

#define BOX_END
#include "box.h"
