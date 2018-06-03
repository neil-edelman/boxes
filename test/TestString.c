/** Copyright 2015 Neil Edelman, distributed under the terms of the MIT License;
 see readme.txt, or \url{ https://opensource.org/licenses/MIT }.

 This is an incomplete test of String.

 @author	Neil
 @version	2018-03 Re-factored code to split up {String} and {Text}.
 @since		2017-03 */

#include <stdlib.h>	/* EXIT_* */
#include <stdio.h>  /* fprintf */
#include <string.h>	/* strcmp */
#include <assert.h>
#include "../src/String.h"

static void verify(const struct String *const string, const size_t bytes,
	const size_t codep) {
	size_t b, c;
	assert(string);
	b = StringLength(string);
	c = StringCodePoints(string);
	printf("The string <%s> has %lu bytes and %lu code-points.\n",
		StringGet(string), (unsigned long)b, (unsigned long)c);
	assert(b == bytes);
	assert(c == codep);
	assert(!StringHasContent(string) == !bytes);
}

/** The is a test of String. */
int main(void) {
	const char *bit1x30 = "all your base are belong to us",
		*bit2x2 = "¥æ",
		*bit3x7 = "煮爫禎ﬀﭖﳼﷺ",
		*bit4x4 = "𐑫𐑣𐑟𐐥";
	struct String s = { 0, 0, { 0, 0 } }, t;
	const char *a, *b;
	size_t bytes = 0, codep = 0;

	printf("Testing:\n");

	String(&t);
	a = StringGet(&s);
	b = StringGet(&t);
	assert(!a && !b && StringLength(&s) == 0
		&& !StringHasContent(0) && !StringHasContent(&s));

	String_(&s);
	a = StringGet(&s);
	assert(!a);

	StringClear(&s);
	a = StringGet(&s);
	assert(a && !strcmp(a, "") && StringLength(&s) == 0
		&& !StringHasContent(&s));

	StringCopy(&s, bit3x7), bytes = 3 * 7, codep = 7;
	verify(&s, bytes, codep);
	StringBetweenCat(&s, bit1x30, bit1x30 + 3), bytes += 3, codep += 3;
	verify(&s, bytes, codep);
	/* @fixme This is a pitiful test. */
	StringCopy(&s, bit1x30), bytes = 30, codep = 30;
	StringCat(&s, bit2x2), bytes += 2*2, codep += 2;
	StringCat(&s, bit3x7), bytes += 3*7, codep += 7;
	StringCat(&s, bit4x4), bytes += 4*4, codep += 4;
	verify(&s, bytes, codep);
	StringNCat(&s, bit1x30, (size_t)5), bytes += 5, codep += 5;
	verify(&s, bytes, codep);
	StringBetweenCat(&s, bit1x30 + 10, bit1x30 + 20), bytes += 10, codep += 10;
	verify(&s, bytes, codep);
#ifndef STRING_STRICT_ANSI /* <-- !STRING_STRICT_ANSI */
	StringPrintCat(&s, "%s%s%.3s", bit2x2, bit4x4, bit1x30),
		bytes += 2*2 + 4*4 + 3, codep += 2 + 4 + 3;
	verify(&s, bytes, codep);
#endif /* !STRING_STRICT_ANSI --> */
	StringTransform(&s, "    %s%%%s \t\n\f");
	bytes *= 2, bytes += 4 + 1 + 4, codep *= 2, codep += 4 + 1 + 4;
	verify(&s, bytes, codep);
	StringCopy(&t, StringGet(&s));
	verify(&t, bytes, codep);
	StringRightTrim(&t);
	bytes -= 4, codep -= 4;
	verify(&t, bytes, codep);
	StringTrim(&s);
	bytes -= 4, codep -= 4;
	verify(&s, bytes, codep);

	printf("Double, double, double.\n");
	StringTransform(&s, "%s%s");
	bytes <<= 1, codep <<= 1;
	verify(&s, bytes, codep);
	StringTransform(&s, "%s%s");
	bytes <<= 1, codep <<= 1;
	verify(&s, bytes, codep);
	StringTransform(&s, "%s%s");
	bytes <<= 1, codep <<= 1;
	verify(&s, bytes, codep);

	StringClear(&s);
	verify(&s, 0, 0);
	String_(&t);
	verify(&t, 0, 0);
	String_(&s);
	verify(&s, 0, 0);

	return EXIT_SUCCESS;
}
