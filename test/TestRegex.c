/** Unit test.

 @title		TestDigraph
 @author	Neil
 @std		C89/90
 @version	1.0; 2018-04
 @since		1.0; 2018-04 */

#include <stddef.h>	/* ptrdiff_t offset_of */
#include <stdlib.h> /* EXIT_ rand */
#include <stdio.h>  /* fprintf */
#include <string.h>	/* memcmp strcmp */
#include <time.h>	/* clock */
#include <limits.h>	/* INT_MAX */
#include <stdlib.h>	/* EXIT_* */
#include <stdio.h>  /* fprintf */
#include <string.h>	/* strcmp */
#include <assert.h>
#include "../src/Regex.h"
#include "Orcish.h"

static struct Result {
	const char *compile, *match, *expected;
} results[] = {
	{ 0, 0, 0 },
	{ "", "hi", "hi" },
	{ "hi", "", 0 },
	{ "hi", "this", "his" },
	{ "bar|baz", "foo baz", "baz" },
	{ "((())", 0, 0 },
	{ "(()))", 0, 0 },
	{ "(())", "hi", "hi" }
};
static const size_t results_size = sizeof results / sizeof *results;

static void re_assert(const struct Result *const r) {
	FILE *fp;
	struct Regex *re = Regex(r->compile);
	const char *a;
	assert(r);
	if(r->match) {
		printf("re_assert: checking that /%s/ ~= \"%s\" == \"%s\".\n",
			r->compile, r->match, r->expected);
		if(!re) { perror(r->compile); assert(0); return; }
	} else {
		printf("re_assert: checking that /%s/ does not compile.\n", r->compile);
		{ assert(!re); perror(r->compile); return; }
	}
	{ /* Output graph. */
		char fn[64] = "graphs/re", *f, *f_end;
		const size_t fn_init = strlen(fn);
		const char *ch;
		for(f = fn + fn_init, f_end = fn + sizeof fn - 4; f < f_end; f++) {
			ch = r->compile + (f - fn) - fn_init;
			if(*ch == '\0') break;
			if((*ch >= 'A' && *ch <= 'Z') || (*ch >= 'a' && *ch <= 'z')) {
				*f = *ch;
			} else {
				*f = '_';
			}
		}
		*f = '\0';
		strcpy(f, ".gv");
		if(!(fp = fopen(fn, "w")) || !RegexOut(re, fp)) perror(fn);
		fclose(fp);
	}
	a = RegexMatch(re, r->match);
	if(r->expected) {
		assert(a && !strcmp(r->expected, a));
	} else {
		assert(!a);
	}
	Regex_(&re);
}

	/*const char *bit1x30 = "all your base are belong to us",
		*bit2x2 = "¥æ",
		*bit3x7 = "煮爫禎ﬀﭖﳼﷺ",
		*bit4x4 = "𐑫𐑣𐑟𐐥";
	const char *str1 = "hellohithere", *str2 = "thsdoesnot", *m;
	if((m = RegexMatch(re, str1))) printf("matches <%s> <%s>\n", str1, m);
	else assert(0);
	printf("str2: <%s>\n", str2);
	if((m = RegexMatch(re, str2)))
		printf("matches <%s> <%s>\n", str2, m), assert(0);

	Regex_(&re_null);
	Regex_(&re);
	assert(!re_null && !re_empty && !re);*/

/** @return Either EXIT_SUCCESS or EXIT_FAILURE. */
int main(void) {
	unsigned seed = (unsigned)clock();
	struct Result *r, *r_end;
	srand(seed), rand(), printf("Seed %u.\n", seed);
	printf("Testing:\n");
	for(r = results, r_end = r + results_size; r < r_end; r++)
		printf("--\n"), re_assert(r);
	return EXIT_SUCCESS;
}
