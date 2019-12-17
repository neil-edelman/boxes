/** @license 2014 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT). Contains some syllables
 from [SMAUG](http://www.smaug.org/), which is a derivative of
 [Merc](http://dikumud.com/Children/merc2.asp), and
 [DikuMud](http://dikumud.com/); used under fair-use.

 @subtitle Random Name Generator

 Orcish names are gathered off the Internet, SMAUG1.8, made up myself,
 _etc_. They originate or are inspired by [JRR Tolkien's Orcish
 ](http://en.wikipedia.org/wiki/Languages_constructed_by_J._R._R._Tolkien).
 Random names are super-useful in testing.

 @std C89
 @cf [Array](https://github.com/neil-edelman/Array)
 @cf [Digraph](https://github.com/neil-edelman/Digraph)
 @cf [List](https://github.com/neil-edelman/List)
 @cf [Pool](https://github.com/neil-edelman/Pool)
 @cf [Set](https://github.com/neil-edelman/Set) */

#include <stdlib.h> /* rand */
#include <stdio.h>  /* strlen */
#include <ctype.h>  /* toupper */
#include <string.h> /* memcpy */
#include "Orcish.h"

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
 then puts a null terminator. Uses `rand` from `stdlib.h`.
 @param[name] If null, does nothing.
 @param[name_size] If zero, does nothing. */
void Orcish(char *const name, size_t name_size) {
	char *n = name;
	const char *part;
	size_t part_len;
	if(!name_size || !name) return;
	if(name_size == 1) { *n = '\0'; return; }
	if(name_size > max_name_size) name_size = max_name_size;
	/* Now `name_size \in [2, max_name_size]`. */
	if(name_size <= syllables_max_length + suffixes_max_length) {
		part = syllables[rand() / (RAND_MAX / syllables_size + 1)];
		part_len = strlen(part);
		if(part_len >= name_size) part_len = name_size - 1;
		memcpy(n, part, part_len), n += part_len, name_size -= part_len;
		if(name_size > suffixes_max_length) {
			part = suffixes[rand() / (RAND_MAX / suffixes_size + 1)];
			part_len = strlen(part);
			memcpy(n, part, part_len), n += part_len, name_size -= part_len;
		}
	} else {
		unsigned no_syllables = ((unsigned)name_size - 1 - suffixes_max_length)
			/ syllables_max_length;
		while(no_syllables) {
			part = syllables[rand() / (RAND_MAX / syllables_size + 1)];
			part_len = strlen(part);
			memcpy(n, part, part_len), n += part_len, name_size -= part_len;
			no_syllables--;
		}
		part = suffixes[rand() / (RAND_MAX / suffixes_size + 1)];
		part_len = strlen(part);
		memcpy(n, part, part_len), n += part_len, name_size -= part_len;
	}
	*n = '\0';
	*name = (char)toupper(*name);
}
