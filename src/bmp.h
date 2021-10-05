/** @license 2021 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 `<B>bmp` is a bit-field of `BMP_BITS` bits.

 @param[BMP_NAME, BMP_BITS]
 `<B>` that satisfies `C` naming conventions when mangled and a number of bits associated therewith; required. `<PB>` is private, whose names are prefixed in
 a manner to avoid collisions.

 @param[BMP_TYPE]
 The underlying unsigned type of array, `unsigned int` default. The size of the
 type determines the granularity; changing it may affect the speed.

 @param[BMP_TEST]
 Optional function implementing <typedef:<PZ>action_fn> that fills the
 <typedef:<PB>type> from uninitialized to random for unit testing framework
 using `assert`. Testing array contained in <../test/test_bmp.h>.

 @std C89/90 */

#include <string.h> /* mem */
#include <limits.h> /* CHAR_BIT */
#include <assert.h>


#if !defined(BMP_NAME) || !defined(BMP_BITS)
#error Name BMP_NAME or unsigned number BMP_BITS undefined.
#endif
#if BMP_BITS <= 1
#error BMP_BITS too small.
#endif

/* <Kernighan and Ritchie, 1988, p. 231>; before idempotent _st_ `CAT`. */
#if defined(B_) || defined(PB_) \
	|| (defined(BMP_SUBTYPE) ^ (defined(CAT) || defined(CAT_)))
#error Unexpected P?B_ or CAT_?
#endif
#ifndef BMP_SUBTYPE /* <!-- !sub-type */
#define CAT_(x, y) x ## _ ## y
#define CAT(x, y) CAT_(x, y)
#endif /* !sub-type --> */
#define B_(thing) CAT(BMP_NAME, thing)
#define PB_(thing) CAT(bmp, B_(thing))

#ifndef BMP_H /* <!-- idempotent */
#define BMP_H
/* <http://c-faq.com/misc/bitsets.html>, except reversed for msb-first. */
#define BMP_MAX (~(PB_(chunk))0)
#define BMP_CHUNK (sizeof(PB_(chunk)) * CHAR_BIT)
#define BMP_CHUNKS (((BMP_BITS) - 1) / BMP_CHUNK + 1)
#define BMP_CHUNK_HI (1u << BMP_CHUNK - 1)
#define BMP_MASK(x) (BMP_CHUNK_HI >> (x) % (unsigned)BMP_CHUNK)
#define BMP_SLOT(x) ((x) / (unsigned)BMP_CHUNK)
#define BMP_AT(a, x) ((a)[BMP_SLOT(x)] & BMP_MASK(x))
#define BMP_DIFF(a, b, x) (((a)[BMP_SLOT(x)] ^ (b)[BMP_SLOT(x)]) & BMP_MASK(x))
#define BMP_SET(a, x) ((a)[BMP_SLOT(x)] |= BMP_MASK(x))
#define BMP_CLEAR(a, x) ((a)[BMP_SLOT(x)] &= ~(BMP_MASK(x)))
#define BMP_TOGGLE(a, x) ((a)[BMP_SLOT(x)] ^= BMP_MASK(x))
#endif /* idempotent --> */


/* Defaults. */
#ifndef BMP_TYPE /* <!-- !type */
#define BMP_TYPE unsigned
#endif /* !type --> */

/** The underlying array, an unsigned type set by `BMP_TYPE`. */
typedef BMP_TYPE PB_(chunk);

/** An array of `BMP_BITS` bits, taking up the next multiple of `BMP_TYPE`
 size. */
struct B_(bmp) { PB_(chunk) chunk[BMP_CHUNKS]; };

/** Sets `a` to all false. */
static void B_(bmp_clear_all)(struct B_(bmp) *const a)
	{ assert(a); memset(a, 0, sizeof *a); }

/** Inverts all entries of `a`. */
static void B_(bmp_invert_all)(struct B_(bmp) *const a) {
	size_t i;
	assert(a);
	for(i = 0; i < sizeof a->chunk / sizeof *a->chunk; i++)
		a->chunk[i] = ~a->chunk[i];
	/* Obsessively zero padded bits. */
	a->chunk[sizeof a->chunk / sizeof *a->chunk - 1]
		&= ~((1u << sizeof a->chunk * CHAR_BIT - BMP_BITS) - 1);
}

/** Copies `a` to `b`. */
static void B_(bmp_copy)(const struct B_(bmp) *const a,
	struct B_(bmp) *const b) { assert(a && b); memcpy(b, a, sizeof *a); }

/** Projects the eigenvalue of bit `x` of `a`. */
static unsigned B_(bmp_at)(struct B_(bmp) *const a, const unsigned x)
	{ assert(a && x < BMP_BITS); return !!BMP_AT(a->chunk, x); }

/** Sets bit `x` in `a`. */
static void B_(bmp_set)(struct B_(bmp) *const a, const unsigned x)
	{ assert(a && x < BMP_BITS); BMP_SET(a->chunk, x); }

/** Clears bit `x` in `a`. */
static void B_(bmp_clear)(struct B_(bmp) *const a, const unsigned x)
	{ assert(a && x < BMP_BITS); BMP_CLEAR(a->chunk, x); }

/** Toggles bit `x` in `a`. */
static void B_(bmp_toggle)(struct B_(bmp) *const a, const unsigned x)
	{ assert(a && x < BMP_BITS); BMP_TOGGLE(a->chunk, x); }

static void B_(bmp_insert_range)(struct B_(bmp) *const a,
	const unsigned x, const unsigned n) {
	const unsigned stop = x / BMP_CHUNK;
	unsigned source_bit = BMP_BITS - n, dest_bit = BMP_BITS;
	struct { unsigned hi, lo; }
		stp = { x / BMP_CHUNK, x % BMP_CHUNK },
		src = { source / BMP_CHUNK, source % BMP_CHUNK },
		dst = { dest / BMP_CHUNK, dest % BMP_CHUNK },
		nxx = { n / BMP_CHUNK, n % BMP_CHUNK };
	const PB_(chunk) store = a->chunk[stop];
	PB_(chunk) construct;
	assert(a && n && x + n < BMP_BITS);
	/* Inverted copying is good. */
	printf("stp %u:%u, src %u:%u, dst %u:%u\n",
		stp.hi, stp.lo, src.hi, src.lo, dst.hi, dst.lo);

	for( ; ; ) {
		printf("stp.hi %u, dst.hi = %u\n", stp.hi, dst.hi);
		construct = a->chunk[src.hi] >> nxx.lo;
		if(dst.hi == stp.hi) { a->chunk[dst.hi] = construct; break; }
		dst.hi--;
		construct |= a->chunk[]
	}
	/*a->chunk[sizeof a->chunk / sizeof *a->chunk - 1]
		&= ~((1u << sizeof a->chunk * CHAR_BIT - BMP_BITS) - 1);*/
	printf("done\n");
}

/** Insert bit `n` into `a`, moving over all the bits past it right; the bit on
 the end is erased. */
static void B_(bmp_insert)(struct B_(bmp) *const a, const unsigned n) {
	struct { unsigned whole, remain; }
		insert = { n / BMP_CHUNK, n % BMP_CHUNK },
		size = { BMP_BITS / BMP_CHUNK, BMP_BITS % BMP_CHUNK };
	const int multi = insert.whole < size.whole;
	PB_(chunk) x = a->chunk[insert.whole], carry = x & 1,
		mask = 1 << BMP_CHUNK - 1 - insert.remain;
	assert(a && n < BMP_BITS);
	/* <https://graphics.stanford.edu/~seander/bithacks.html#MaskedMerge>. */
	a->chunk[insert.whole] = (x ^ ((x ^ (x >> 1)) & (mask - 1))) & ~mask;
	if(multi) while(++insert.whole < size.whole) {
		x = a->chunk[insert.whole];
		a->chunk[insert.whole] = (PB_(chunk))(carry << BMP_CHUNK-1) | (x >> 1);
		carry = x & 1;
	}
	if(size.remain) {
		mask = BMP_MAX >> size.remain;
		x = a->chunk[insert.whole];
		a->chunk[insert.whole]
			= (multi ? (carry << BMP_CHUNK-1) | (x >> 1) : x) & ~mask;
	}
}

#ifdef BMP_TEST /* <!-- test */
#include "../test/test_bmp.h" /** \include */
#endif /* test --> */

static void PB_(unused_base_coda)(void);
static void PB_(unused_base)(void) {
	B_(bmp_clear_all)(0); B_(bmp_invert_all)(0); B_(bmp_at)(0, 0);
	B_(bmp_set)(0, 0); B_(bmp_clear)(0, 0); B_(bmp_toggle)(0, 0);
	PB_(unused_base_coda)();
}
static void PB_(unused_base_coda)(void) { PB_(unused_base)(); }

#ifndef BMP_SUBTYPE /* <!-- !sub-type */
#undef CAT
#undef CAT_
#else /* !sub-type --><!-- sub-type */
#undef BMP_SUBTYPE
#endif /* sub-type --> */
#undef B_
#undef PB_
#undef BMP_NAME
#undef BMP_BITS
#undef BMP_TYPE
#ifdef BMP_TEST
#undef BMP_TEST
#endif
#undef BMP_MAX
