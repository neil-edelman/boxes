/** @license 2014 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT). Contains some syllables
 from [SMAUG](http://www.smaug.org/), which is a derivative of
 [Merc](http://dikumud.com/Children/merc2.asp), and
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
#include <math.h>   /* exp */

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

static const unsigned max_name_size = 128;

/** This is Poisson process in a similar manner to that proposed by Knuth. It
 uses floating point; the values were too small to reliably use fixed point.
 @param[r, recur] A pointer to the recurrence that will generate numbers in the
 range of `[0, RAND_MAX]`.
 @return A random number based on the expectation value `expect`.
 @order \O(`expect`) */
static unsigned poisson(double expect,
	unsigned long *const r, unsigned (*recur)(unsigned long *)) {
	const double limit = exp(-expect);
	double prod = 1.0 * recur(r) / RAND_MAX;
	unsigned n;
	/* These are orc-specific; ensures that we don't spend too much time. */
	assert(expect >= 0.0 && expect < 128.0 && r && recur);
	for(n = 0; prod >= limit; n++) prod *= 1.0 * recur(r) / RAND_MAX;
	return n;
}

/** Fills `name` with a random Orcish name. Potentially up to `name_size` - 1,
 (if zero, does nothing) then puts a null terminator. Uses `r` plugged into
 `recur` to generate random values in the range of `[0, RAND_MAX]`. */
static void orc_rand(char *const name, const size_t name_size,
	unsigned long r, unsigned (*recur)(unsigned long *)) {
	unsigned len, syl_len, suf_len, ten_len;
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
	ten_len = poisson((len + syl_len) / 2.0, &r, recur);
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
	{ return (*r = fmix(*r)) % (1lu + RAND_MAX); }

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
 up to `name_size` - 1, (with a maximum of 255,) then puts a null terminator.
 @param[name] A valid pointer to at least `name_size` characters.
 @param[name_size] If zero, does nothing. */
void orc_long(char *const name, const size_t name_size, const unsigned long l) {
	assert(name || !name_size);
	orc_rand(name, name_size, l, &murmur_callback);
}

/** Fills `name` with a deterministic Orcish name based on `p`, or if `p` is
 null, then "null". Potentially up to `name_size` - 1, (with a maximum of 255,)
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
