/** 2014 Neil Edelman: neil dot edelman each mail dot mcgill dot ca.

 Orcish words are gathered off the Internet, SMAUG1.8, made up myself,
 etc. They originate or are inspired by JRR Tolkien's Orcish; this
 file has had many incarnations and was ported from Java. Random
 words are super-useful in testing and character generation;
 \url{http://en.wikipedia.org/wiki/Languages_constructed_by_J._R._R._Tolkien}.
 Provides one function, \see{Orcish}.

 @title		Orcish
 @std		ANSI C89/ISO C90
 @author	Neil
 @version	1.2, 2016-09
 @since		2014 */

#include <stdlib.h> /* rand */
#include <stdio.h>	/* snprintf strlen */
#include <ctype.h>	/* toupper */
#include <string.h>	/* strcat, strncat */
#include "Orcish.h"

static const char *syllables[] = {
	"ub", "ul", "uk", "um", "uu", "oo", "ee", "uuk", "uru",
	"ick", "gn", "ch", "ar", "eth", "ith", "ath", "uth", "yth",
	"ur", "uk", "ug", "sna", "or", "ko", "uks", "ug", "lur", "sha", "grat",
	"mau", "eom", "lug", "uru", "mur", "ash", "goth", "sha", "cir", "un",
	"mor", "ann", "sna", "gor", "dru", "az", "azan", "nul", "biz", "balc",
	"balc", "tuo", "gon", "dol", "bol", "dor", "luth", "bolg", "beo",
	"vak", "bat", "buy", "kham", "kzam", "lg", "bo", "thi", "ia", "es", "en",
	"ion", "mok", "muk", "tuk", "gol", "fim", "ette", "moor", "goth", "gri",
	"shn", "nak", "ash", "bag", "ronk", "ask", "mal", "ome", "hi",
	"sek", "aah", "ove", "arg", "ohk", "to", "lag", "muzg", "ash", "mit",
	"rad", "sha", "saru", "ufth", "warg", "sin", "dar", "ann", "mor", "dab",
	"val", "dur", "dug", "bar", "ash", "krul", "gakh", "kraa", "rut", "udu",
	"ski", "kri", "gal", "nash", "naz", "hai", "mau", "sha", "akh", "dum",
	"olog", "lab", "lat"
};
static const unsigned syllables_size = sizeof syllables / sizeof *syllables;
static const unsigned syllables_max_length = 4;

static const char *suffixes[] = {
	"at", "ob", "agh", "uk", "uuk", "um", "uurz", "hai", "ishi", "ub",
	"ull", "ug", "an", "hai", "gae", "-hai", "luk", "tz", "hur", "dush",
	"ks", "mog", "grat", "gash", "th", "on", "gul", "gae", "gun",
	"dan", "og", "ar", "meg", "or", "lin", "dog", "ath", "ien", "rn", "bul",
	"bag", "ungol", "mog", "nakh", "gorg", "-dug", "duf", "ril", "bug",
	"snaga", "naz", "gul", "ak", "kil", "ku", "on", "ritz", "bad", "nya",
	"durbat", "durb", "kish", "olog", "-atul", "burz", "puga", "shar",
	"snar", "hai", "ishi", "uruk", "durb", "krimp", "krimpat", "zum",
	"gimb", "-gimb", "glob", "-glob", "sharku", "sha", "-izub", "-izish",
	"izg", "-izg", "ishi", "ghash", "thrakat", "thrak", "golug", "mokum",
	"ufum", "bubhosh", "gimbat", "shai", "khalok", "kurta", "ness", "funda"
};
static const unsigned suffixes_size = sizeof suffixes / sizeof *suffixes;
static const unsigned suffixes_max_length = 7;

static const unsigned max_name_size = 256;

/** Takes {name}, a string, and replaces it, to a maximum of {name_size}
 characters, with a {rand} Orcish name. You must have space for (at least)
 {name_size} (byte) characters.
 @param name: Filled with a random word in psudo-Orcish.
 @param name_size: sizeof(name); suggest 16, which would be enough for
 2 syllables and a suffix. */
void Orcish(char *const name, const size_t name_size) {
	char *str;
	int a;
	const unsigned name_chars = (name_size > (unsigned)max_name_size) ?
		max_name_size : (unsigned)name_size;

	if(name_size == 0) return;

	name[0] = '\0';

	if(name_size == 1) {
		return;
	} else if(name_size < syllables_max_length + 1) {
		a = (int)(rand() / (RAND_MAX + 1.0) * syllables_size);
		strncat(name, syllables[a], name_size - 1);
	} else if(name_size < syllables_max_length + suffixes_max_length + 1) {
		a = (int)(rand() / (RAND_MAX + 1.0) * syllables_size);
		str = strcat(name, syllables[a]);
		a = (int)(rand() / (RAND_MAX + 1.0) * syllables_size);
		strncat(str, syllables[a], name_size - strlen(name) - 1);
	} else {
		unsigned i, no_syllables;

		no_syllables = (name_chars-1-suffixes_max_length)/syllables_max_length;
		str = name;
		name[0] = '\0';
		for(i = 0; i < no_syllables; i++) {
			a = (int)(rand() / (RAND_MAX + 1.0) * syllables_size);
			str = strcat(str, syllables[a]);
		}
		a = (int)(rand() / (RAND_MAX + 1.0) * suffixes_size);
		strcat(str, suffixes[a]);
	}

	name[0] = toupper(name[0]);

}
