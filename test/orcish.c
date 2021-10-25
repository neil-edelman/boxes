/** @license 2014 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT). Contains some syllables
 from [SMAUG](http://www.smaug.org/), which is a derivative of
 [Merc](http://dikumud.com/Children/merc2.asp), and
 [DikuMud](http://dikumud.com/); used under fair-use.

 @subtitle Random Name Generator

 Orcish names are gathered off the Internet, SMAUG1.8, made up myself,
 _etc_. They originate or are inspired by [JRR Tolkien's Orcish
 ](http://en.wikipedia.org/wiki/Languages_constructed_by_J._R._R._Tolkien).

 @std C89 */

#include <stdlib.h> /* rand */
#include <stdio.h>  /* strlen */
#include <ctype.h>  /* toupper */
#include <string.h> /* memcpy */
#include <assert.h> /* assert */
#include "orcish.h"

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
static const unsigned syllables_size = sizeof syllables / sizeof *syllables;
static const unsigned syllables_max_length = 4;

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
static const unsigned suffixes_size = sizeof suffixes / sizeof *suffixes;
static const unsigned suffixes_max_length = 7;

static const unsigned max_name_size = 256;

/** Fills `name` with a random Orcish name. Potentially up to `name_size` - 1,
 then puts a null terminator. Uses `r` plugged into `recur` to generate random
 values in the range of `RAND_MAX`.
 @param[name_size] If zero, does nothing. */
static void orcish_recur(char *const name, size_t name_size,
	unsigned long r, unsigned (*recur)(unsigned long *)) {
	char *n = name;
	const char *part;
	size_t part_len;
	assert(name);
	if(!name_size) { return; }
	else if(name_size == 1) { *n = '\0'; return; }
	else if(name_size > max_name_size) { name_size = max_name_size; }
	/* Now `name_size \in [2, max_name_size]`. */
	if(name_size <= syllables_max_length + suffixes_max_length) {
		part = syllables[recur(&r) / (RAND_MAX / syllables_size + 1)];
		part_len = strlen(part);
		if(part_len >= name_size) part_len = name_size - 1;
		memcpy(n, part, part_len), n += part_len, name_size -= part_len;
		if(name_size > suffixes_max_length) {
			part = suffixes[recur(&r) / (RAND_MAX / suffixes_size + 1)];
			part_len = strlen(part);
			memcpy(n, part, part_len), n += part_len, name_size -= part_len;
		}
	} else {
		unsigned no_syllables = ((unsigned)name_size - 1 - suffixes_max_length)
			/ syllables_max_length;
		while(no_syllables) {
			part = syllables[recur(&r) / (RAND_MAX / syllables_size +1)];
			part_len = strlen(part);
			memcpy(n, part, part_len), n += part_len, name_size -= part_len;
			no_syllables--;
		}
		part = suffixes[recur(&r) / (RAND_MAX / suffixes_size + 1)];
		part_len = strlen(part);
		memcpy(n, part, part_len), n += part_len, name_size -= part_len;
	}
	*n = '\0';
	*name = (char)toupper(*name);
}

/** Uses `rand`; ignores `r` and uses a global variable set by `srand`. */
static unsigned rand_recur(unsigned long *const r)
	{ (void)r; return (unsigned)rand(); }

/** <https://github.com/aappleby/smhasher>, used as the recurrence on `r`. */
static unsigned MurmurHash3Mixer(unsigned long *const r) {
	unsigned long key = *r;
	assert(r);
	key ^= (key >> 33);
	key *= 0xff51afd7ed558ccd;
	key ^= (key >> 33);
	key *= 0xc4ceb9fe1a85ec53;
	key ^= (key >> 33);
	*r = key;
	return key % (1lu + RAND_MAX);
}

/** Fills `name` with a random Orcish name. Potentially up to `name_size` - 1,
 then puts a null terminator. Uses `rand` from `stdlib.h`.
 @param[name_size] If zero, does nothing. */
void orcish(char *const name, const size_t name_size)
	{ orcish_recur(name, name_size, 0, &rand_recur); }

/** Fills `name` with a deterministic Orcish name based on `p`. Potentially up
 to `name_size` - 1, then puts a null terminator. Uses `MurmurHash3Mixer`.
 @param[name_size] If zero, does nothing. */
void orcish_ptr(char *const name, const size_t name_size,
	const void *const p) {
	assert(name);
	if(!name_size) return;
	if(!p) {
		switch(name_size) {
		default:
		case 5: name[3] = 'l';
		case 4: name[2] = 'l';
		case 3: name[1] = 'u';
		case 2: name[0] = 'n';
		case 1: break;
		}
		name[name_size < 5 ? name_size - 1 : 4] = '\0';
	} else {
		orcish_recur(name, name_size, (unsigned long)p, &MurmurHash3Mixer);
	}
}

/** Fills a static buffer of up to four names with a deterministic Orcish name
 based on `p` with <fn:orcish_ptr>. */
const char *orc(const void *const p) {
	static char str[4][12];
	static unsigned x;
	x %= sizeof str / sizeof *str;
	orcish_ptr(str[x], sizeof *str, p);
	return str[x++];
}
