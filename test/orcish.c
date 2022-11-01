/** @license 2014 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT). Orcish is from
 JRR Tolkien's work, and some syllables from [SMAUG](http://www.smaug.org/),
 which is a derivative of [Merc](http://dikumud.com/Children/merc2.asp), and
 [DikuMud](http://dikumud.com/); used under fair-use. Contains
 [MurmurHash](https://github.com/aappleby/smhasher)-derived code, placed in
 public domain by Austin Appleby.

 @subtitle Name generator

 Orcish names originate or are inspired by [JRR Tolkien's Orcish
 ](http://en.wikipedia.org/wiki/Languages_constructed_by_J._R._R._Tolkien).

 @std C89 */

#include "orcish.h"
#include <stdlib.h> /* rand */
#include <stdio.h>  /* strlen */
#include <ctype.h>  /* toupper */
#include <string.h> /* memcpy */
#include <assert.h> /* assert */
#include <limits.h> /* CHAR_BIT, ULONG_MAX */
/* Lookup-table: don't force the users to compile with math libraries. */
/*#include <math.h>*/   /* exp */

static const char *syllables[] = {
	"ub", "ul", "uk", "um", "uu", "oo", "ee", "uuk", "uru", "ick", "gn", "ch",
	"ar", "eth", "ith", "ath", "uth", "yth", "ur", "uk", "ug", "sna", "or",
	"ko", "uks", "ug", "lur", "sha", "grat", "mau", "eom", "lug", "uru", "mur",
	"ash", "goth", "sha", "cir", "un", "mor", "ann", "sna", "gor", "dru", "az",
	"azan", "nul", "biz", "balc", "balc", "tuo", "gon", "dol", "bol", "dor",
	"luth", "bolg", "beo", "vak", "bat", "buy", "kham", "kzam", "lg", "bo",
	"thi", "ia", "es", "en", "ion", "mok", "muk", "tuk", "gol", "fim", "ette",
	"moor", "goth", "gri", "shn", "nak", "ash", "bag", "ronk", "ask", "mal",
	"ome", "hi", "sek", "aah", "ove", "arg", "ohk", "to", "lag", "muzg", "ash",
	"mit", "rad", "sha", "saru", "ufth", "warg", "sin", "dar", "ann", "mor",
	"dab", "val", "dur", "dug", "bar", "ash", "krul", "gakh", "kraa", "rut",
	"udu", "ski", "kri", "gal", "nash", "naz", "hai", "mau", "sha", "akh",
	"dum", "olog", "lab", "lat"
};

static const char *suffixes[] = {
	"at", "ob", "agh", "uk", "uuk", "um", "uurz", "hai", "ishi", "ub", "ull",
	"ug", "an", "hai", "gae", "-hai", "luk", "tz", "hur", "dush", "ks", "mog",
	"grat", "gash", "th", "on", "gul", "gae", "gun", "dan", "og", "ar", "meg",
	"or", "lin", "dog", "ath", "ien", "rn", "bul", "bag", "ungol", "mog",
	"nakh", "gorg", "-dug", "duf", "ril", "bug", "snaga", "naz", "gul", "ak",
	"kil", "ku", "on", "ritz", "bad", "nya", "durbat", "durb", "kish", "olog",
	"-atul", "burz", "puga", "shar", "snar", "hai", "ishi", "uruk", "durb",
	"krimp", "krimpat", "zum", "gimb", "-gimb", "glob", "-glob", "sharku",
	"sha", "-izub", "-izish", "izg", "-izg", "ishi", "ghash", "thrakat",
	"thrak", "golug", "mokum", "ufum", "bubhosh", "gimbat", "shai", "khalok",
	"kurta", "ness", "funda"
};

/* There are entries near the end that are never used `+ 1 + min_suffix`, but
 we might as well fill all 128. */
static double expM1_2[] = {
	/*0.0*/	1,
	/*0.5*/	0.60653,
	/*1.0*/	0.36788,
	/*1.5*/	0.22313,
	/*2.0*/	0.13534,
	/*2.5*/	0.082085,
	/*3.0*/	0.049787,
	/*3.5*/	0.030197,
	/*4.0*/	0.018316,
	/*4.5*/	0.011109,
	/*5.0*/	0.0067379,
	/*5.5*/	0.0040868,
	/*6.0*/	0.0024788,
	/*6.5*/	0.0015034,
	/*7.0*/	0.00091188,
	/*7.5*/	0.00055308,
	/*8.0*/	0.00033546,
	/*8.5*/	0.00020347,
	/*9.0*/	0.00012341,
	/*9.5*/	7.4852e-05,
	/*10.0*/	4.54e-05,
	/*10.5*/	2.7536e-05,
	/*11.0*/	1.6702e-05,
	/*11.5*/	1.013e-05,
	/*12.0*/	6.1442e-06,
	/*12.5*/	3.7267e-06,
	/*13.0*/	2.2603e-06,
	/*13.5*/	1.371e-06,
	/*14.0*/	8.3153e-07,
	/*14.5*/	5.0435e-07,
	/*15.0*/	3.059e-07,
	/*15.5*/	1.8554e-07,
	/*16.0*/	1.1254e-07,
	/*16.5*/	6.8256e-08,
	/*17.0*/	4.1399e-08,
	/*17.5*/	2.511e-08,
	/*18.0*/	1.523e-08,
	/*18.5*/	9.2374e-09,
	/*19.0*/	5.6028e-09,
	/*19.5*/	3.3983e-09,
	/*20.0*/	2.0612e-09,
	/*20.5*/	1.2502e-09,
	/*21.0*/	7.5826e-10,
	/*21.5*/	4.5991e-10,
	/*22.0*/	2.7895e-10,
	/*22.5*/	1.6919e-10,
	/*23.0*/	1.0262e-10,
	/*23.5*/	6.2241e-11,
	/*24.0*/	3.7751e-11,
	/*24.5*/	2.2897e-11,
	/*25.0*/	1.3888e-11,
	/*25.5*/	8.4235e-12,
	/*26.0*/	5.1091e-12,
	/*26.5*/	3.0988e-12,
	/*27.0*/	1.8795e-12,
	/*27.5*/	1.14e-12,
	/*28.0*/	6.9144e-13,
	/*28.5*/	4.1938e-13,
	/*29.0*/	2.5437e-13,
	/*29.5*/	1.5428e-13,
	/*30.0*/	9.3576e-14,
	/*30.5*/	5.6757e-14,
	/*31.0*/	3.4425e-14,
	/*31.5*/	2.088e-14,
	/*32.0*/	1.2664e-14,
	/*32.5*/	7.6812e-15,
	/*33.0*/	4.6589e-15,
	/*33.5*/	2.8258e-15,
	/*34.0*/	1.7139e-15,
	/*34.5*/	1.0395e-15,
	/*35.0*/	6.3051e-16,
	/*35.5*/	3.8242e-16,
	/*36.0*/	2.3195e-16,
	/*36.5*/	1.4069e-16,
	/*37.0*/	8.533e-17,
	/*37.5*/	5.1756e-17,
	/*38.0*/	3.1391e-17,
	/*38.5*/	1.904e-17,
	/*39.0*/	1.1548e-17,
	/*39.5*/	7.0044e-18,
	/*40.0*/	4.2484e-18,
	/*40.5*/	2.5768e-18,
	/*41.0*/	1.5629e-18,
	/*41.5*/	9.4794e-19,
	/*42.0*/	5.7495e-19,
	/*42.5*/	3.4873e-19,
	/*43.0*/	2.1151e-19,
	/*43.5*/	1.2829e-19,
	/*44.0*/	7.7811e-20,
	/*44.5*/	4.7195e-20,
	/*45.0*/	2.8625e-20,
	/*45.5*/	1.7362e-20,
	/*46.0*/	1.0531e-20,
	/*46.5*/	6.3871e-21,
	/*47.0*/	3.874e-21,
	/*47.5*/	2.3497e-21,
	/*48.0*/	1.4252e-21,
	/*48.5*/	8.6441e-22,
	/*49.0*/	5.2429e-22,
	/*49.5*/	3.18e-22,
	/*50.0*/	1.9287e-22,
	/*50.5*/	1.1698e-22,
	/*51.0*/	7.0955e-23,
	/*51.5*/	4.3036e-23,
	/*52.0*/	2.6103e-23,
	/*52.5*/	1.5832e-23,
	/*53.0*/	9.6027e-24,
	/*53.5*/	5.8243e-24,
	/*54.0*/	3.5326e-24,
	/*54.5*/	2.1426e-24,
	/*55.0*/	1.2996e-24,
	/*55.5*/	7.8824e-25,
	/*56.0*/	4.7809e-25,
	/*56.5*/	2.8998e-25,
	/*57.0*/	1.7588e-25,
	/*57.5*/	1.0668e-25,
	/*58.0*/	6.4702e-26,
	/*58.5*/	3.9244e-26,
	/*59.0*/	2.3803e-26,
	/*59.5*/	1.4437e-26,
	/*60.0*/	8.7565e-27,
	/*60.5*/	5.3111e-27,
	/*61.0*/	3.2213e-27,
	/*61.5*/	1.9538e-27,
	/*62.0*/	1.1851e-27,
	/*62.5*/	7.1878e-28,
	/*63.0*/	4.3596e-28,
	/*63.5*/	2.6442e-28
};
static const unsigned max_name_size = sizeof expM1_2 / sizeof *expM1_2;

/** This is Poisson process in a similar manner to that proposed by Knuth. It
 uses floating point; the values were too small to reliably use fixed point.
 @param[limit] `exp -expectation`, for optimization, this is looked up in a
 table.
 @param[r, recur] A pointer to the recurrence that will generate numbers in the
 range of `[0, RAND_MAX]`.
 @return A random number based on the expectation value `expect`.
 @order \O(`expect`) */
static unsigned poisson_lim(/*double expect,*/const double limit,
	unsigned long *const r, unsigned (*recur)(unsigned long *)) {
	/*const double limit = exp(-expect);*/
	double prod = 1.0 * recur(r) / RAND_MAX;
	unsigned n;
	/* These are orc-specific; ensures that we don't spend too much time. */
	assert(/*expect >= 0.0 && expect < 1.0 * max_name_size &&*/ r && recur);
	assert(limit > 0.0);
	for(n = 0; prod >= limit; n++) prod *= 1.0 * recur(r) / RAND_MAX;
	return n;
}

/** Fills `name` with a random Orcish name. Potentially up to `name_size` - 1,
 (if zero, does nothing) then puts a null terminator. Uses `r` plugged into
 `recur` to generate random values in the range of `[0, RAND_MAX]`. */
static void orc_rand(char *const name, const size_t name_size,
	unsigned long r, unsigned (*recur)(unsigned long *)) {
	unsigned len, syl_len, suf_len, ten_len, expectation2;
	const char *syl, *suf;
	char *n = name;
	assert((name || !name_size) && recur);

	if(!name_size) { return; }
	if(name_size == 1) { goto terminate; }
	len = (name_size < max_name_size ? (unsigned)name_size : max_name_size) - 1;

#define ORC_SAMPLE(array, seed) (assert((seed) <= RAND_MAX), \
	(array)[(seed) / (RAND_MAX / (sizeof (array) / sizeof *(array)) + 1)])

	/* Place the first syllable. */
	syl_len = (unsigned)strlen(syl = ORC_SAMPLE(syllables, recur(&r)));
	if(syl_len > len) syl_len = len;
	memcpy(n, syl, (size_t)syl_len), n += syl_len, len -= syl_len;
	if(!len) goto capitalize;

	/* Choose the suffix, but don't insert it until the end. */
	suf_len = (unsigned)strlen(suf = ORC_SAMPLE(suffixes, recur(&r)));
	if(suf_len > len) suf_len = len;
	len -= suf_len;
	if(!len) goto suffix;

	/* Reduce the length to a number drawn from a Poisson random variable
	 having the expected value of half the syllable part. */
	expectation2 = len + syl_len;
	assert(expectation2 < sizeof expM1_2 / sizeof *expM1_2);
	ten_len = poisson_lim(expM1_2[expectation2], &r, recur);
	if(ten_len < len) { len = ten_len; if(!len) goto suffix; }

	/* While we can still fit syllables. */
	for( ; ; ) {
		syl_len = (unsigned)strlen(syl = ORC_SAMPLE(syllables, recur(&r)));
		if(syl_len > len) break;
		memcpy(n, syl, (size_t)syl_len), n += syl_len, len -= syl_len;
	}

#undef ORC_SAMPLE

suffix:
	memcpy(n, suf, (size_t)suf_len), n += suf_len;
capitalize:
	*name = (char)toupper((unsigned char)*name);
terminate:
	*n = '\0';
}

#if ULONG_MAX <= 0xffffffff || ULONG_MAX < 0xffffffffffffffff /* <!-- !long */
/** <https://github.com/aappleby/smhasher> `src/MurmurHash3.cpp fmix32`.
 @return Recurrence on `h`. */
static unsigned long fmix(unsigned long h) {
	h ^= h >> 16;
	h *= 0x85ebca6b;
	h ^= h >> 13;
	h *= 0xc2b2ae35;
	h ^= h >> 16;
	return h;
}
#else /* !long --><!-- long */
/** <https://github.com/aappleby/smhasher> `src/MurmurHash3.cpp fmix64`.
 @return Recurrence on `k`. */
static unsigned long fmix(unsigned long k) {
	k ^= k >> 33;
	k *= 0xff51afd7ed558ccd;
	k ^= k >> 33;
	k *= 0xc4ceb9fe1a85ec53;
	k ^= k >> 33;
	return k;
}
#endif /* long --> */

/** Advances `r` with `MurmurHash` finalizer.
 @return Number in `[0, RAND_MAX]`. @implements `orc_rand` */
static unsigned murmur_callback(unsigned long *const r)
	{ /* `fmix(0) = 0`, not sure if that's a problem. */
	return (*r = fmix(*r)) % (1lu + RAND_MAX); }

/** Uses `rand`; ignores `r` and uses a global variable set by `srand`.
 @return Number in `[0, RAND_MAX]`. @implements `orc_rand` */
static unsigned rand_callback(unsigned long *const r)
	{ (void)r; return (unsigned)rand(); }

/** Fills `name` with a random Orcish name. Potentially up to `name_size` - 1,
 (with a maximum of 128,) then puts a null terminator. Uses `rand` from
 `stdlib.h`.
 @param[name] A valid pointer to at least `name_size` characters.
 @param[name_size] If zero, does nothing. */
void orcish(char *const name, const size_t name_size) {
	assert(name || !name_size);
	orc_rand(name, name_size, 0, &rand_callback);
}

/** Fills `name` with a deterministic Orcish name based on `l`, potentially
 up to `name_size` - 1, (with a maximum,) then puts a null terminator.
 @param[name] A valid pointer to at least `name_size` characters.
 @param[name_size] If zero, does nothing. */
void orc_long(char *const name, const size_t name_size, const unsigned long l) {
	assert(name || !name_size);
	orc_rand(name, name_size, l, &murmur_callback);
}

/** Fills `name` with a deterministic Orcish name based on `p`, or if `p` is
 null, then "null". Potentially up to `name_size` - 1, (with a maximum,)
 then puts a null terminator.
 @param[name] A valid pointer to at least `name_size` characters.
 @param[name_size] If zero, does nothing. */
void orc_ptr(char *const name, const size_t name_size, const void *const p) {
	assert(name || !name_size);
	if(p) {
		/* There will be data lost in the upper bits if
		 `sizeof(unsigned long) < sizeof(void *)`, but it's probably okay? */
		orc_long(name, name_size, (unsigned long)p);
	} else {
		switch(name_size) {
		case 0: return;
		default: /* _Sic_; `name_size > 5` has enough. Fall-through. */
		case 5: name[3] = 'l';
		case 4: name[2] = 'l';
		case 3: name[1] = 'u';
		case 2: name[0] = 'n';
		case 1: break;
		}
		name[name_size < 5 ? name_size - 1 : 4] = '\0';
	}
}

/** Call <fn:orc_ptr> with `p` with default values and a small temporary buffer.
 @return A temporary string; can handle four names at a time. */
const char *orcify(const void *const p) {
	static char names[4][10];
	static unsigned n;
	n %= sizeof names / sizeof *names;
	orc_ptr(names[n], sizeof *names, p);
	return names[n++];
}
