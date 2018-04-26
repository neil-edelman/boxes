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

static void test_null(void) {
	struct Regex *re_null = Regex(0), *re_empty = Regex("");
	const char *const fn = "graphs/empty.gv";
	FILE *fp = 0;

	assert(!re_null && re_empty);
	printf("hithere <%s>; null <%s>; hihi <%s>;  <%s>.\n",
		RegexMatch(re_empty, "hithere"),
		RegexMatch(re_empty, "bye"),
		RegexMatch(re_empty, "therehihi"),
		RegexMatch(re_empty, ""));
	/* @fixme on fail? */
	if(!(fp = fopen(fn, "w")) || !RegexOut(re_empty, fp)) perror(fn);
	fclose(fp);
	Regex_(&re_null);
	Regex_(&re_empty);
}

static void test_literals(void) {
	struct Regex *re = Regex("hi");
	const char *const fn = "graphs/literals.gv";
	FILE *fp = 0;

	assert(re);
	printf("hithere <%s>; null <%s>; hihi <%s>; null <%s>.\n",
		RegexMatch(re, "hithere"),
		RegexMatch(re, "bye"),
		RegexMatch(re, "therehihi"),
		RegexMatch(re, ""));
	/* @fixme on fail? */
	if(!(fp = fopen(fn, "w")) || !RegexOut(re, fp)) perror(fn);
	fclose(fp);
	Regex_(&re);
}

static void test_regex(void) {
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
}

/** @return Either EXIT_SUCCESS or EXIT_FAILURE. */
int main(void) {
	unsigned seed = (unsigned)clock();
	srand(seed), rand(), printf("Seed %u.\n", seed);
	printf("Testing:\n");
	test_null();
	test_literals();
	test_regex();
	return EXIT_SUCCESS;
}
