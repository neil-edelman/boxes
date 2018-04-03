/** Copyright 2015 Neil Edelman, distributed under the terms of the MIT License;
 see readme.txt, or \url{ https://opensource.org/licenses/MIT }.

 This is a test of String.

 @author	Neil
 @version	2018-03 Re-factored code to split up {String} and {Text}.
 @since		2017-03 */

#include <stdlib.h>	/* EXIT_* */
#include <stdio.h>  /* fprintf */
#include <string.h>	/* strcmp */
#include <assert.h>
#include "../src/String.h"

#if 0
static void url(struct String *const this) {
	StringTrim(this), StringTransform(this, "<a href = \"%s\">%s</a>");
}
static void cite(struct String *const this) {
	StringTrim(this), StringTransform(this,
		"<a href = \"https://scholar.google.ca/scholar?q=%s\">%s</a>");
}
/** @implements	StringAction */
static void em(struct String *const this) { StringTransform(this, "<em>%s</em>"); }

/*static const struct StringPattern tpattern[] = {
	{ "\\url{",  "}", &url },
	{ "\\cite{", "}", &cite },
	{ "{",       "}", &em }
};
static const size_t tpattern_size = sizeof tpattern / sizeof *tpattern;*/

/** @implements	StringPredicate */
static int is_delim(const char *const str, const char *p) {
	return *++p == '/' || 0 == str ? 0 : -1;
}
#endif

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

	/* @fixme This is a pitiful test. */
	StringCopy(&s, bit3x7);
	StringCopy(&s, bit1x30), bytes += 30, codep += 30;
	StringCat(&s, bit2x2), bytes += 2*2, codep += 2;
	StringCat(&s, bit3x7), bytes += 3*7, codep += 7;
	StringCat(&s, bit4x4), bytes += 4*4, codep += 4;
	verify(&s, bytes, codep);
	StringNCat(&s, bit1x30, (size_t)5), bytes += 5, codep += 5;
	verify(&s, bytes, codep);
	StringBetweenCat(&s, bit1x30 + 10, bit1x30 + 20), bytes += 11, codep += 11;
	verify(&s, bytes, codep);
	StringPrintCat(&s, "%s%s%.3s \t", bit2x2, bit4x4, bit1x30),
		bytes += 2*2 + 4*4 + 3 + 2, codep += 2 + 4 + 3 + 2;
	verify(&s, bytes, codep);
	StringTransform(&s, "    %s%%%s\n\f");
	bytes *= 2, bytes += 4 + 1 + 2, codep *= 2, codep += 4 + 1 + 2;
	verify(&s, bytes, codep);
	StringCopy(&t, StringGet(&s));
	verify(&t, bytes, codep);
	StringRightTrim(&t);
	verify(&t, bytes - 4, codep - 4);
	StringTrim(&s);
	bytes -= 4 + 4, codep -= 4 + 4;
	verify(&s, bytes, codep);

	StringClear(&s);
	verify(&s, 0, 0);
	String_(&t);
	verify(&t, 0, 0);
	String_(&s);
	verify(&s, 0, 0);

	/*
	void String_(struct String *const);
	void String(struct String *const);
	struct String *StringClear(struct String *const);
	const char *StringGet(const struct String *const);
	size_t StringLength(const struct String *const);
	size_t StringCodePoints(const struct String *const);
	int StringHasContent(const struct String *const);
	struct String *StringRightTrim(struct String *const);
	struct String *StringTrim(struct String *const);
	struct String *StringCopy(struct String *const, const char *const);
	struct String *StringCat(struct String *const, const char *const);
	struct String *StringNCat(struct String *const, const char *const,const size_t);
	struct String *StringBetweenCat(struct String *const, const char *const,
									const char *const);
	struct String *StringPrintCat(struct String *const, const char *const, ...);
	struct String *StringTransform(struct String *const, const char *);
	*/

	/*printf("StringNCat:\n");
	StringNCat(t, "TestString", (size_t)4);
	printf("String: %s\n", StringGet(t));
	assert((str = StringGet(t)) && !strcmp(sup = "Test", str));

	printf("StringTransform:\n");
	StringTransform(t, "\\url{%s%%%s} yo {YO}");
	printf("String: %s\n", StringGet(t));
	assert((str = StringGet(t)) && !strcmp(sup = "\\url{Test%Test} yo {YO}",str));

	printf("StringMatch:\n");
	StringMatch(t, tpattern, tpattern_size);
	printf("String: %s\n", StringGet(t));
	assert((str = StringGet(t)) && !strcmp(sup
		= "<a href = \"Test%Test\">Test%Test</a> yo <em>YO</em>", str));
	StringClear(t);

	printf("StringBetweenCat:\n");
	s0 = strchr(fn + 1, '/');
	s1 = strchr(s0 + 1, '/');
	StringBetweenCat(t, s0, s1);
	assert((str = StringGet(t)) && !strcmp(sup = "/neil/", str));
	s0 = strchr(s1 + 1, '/');
	s1 = strchr(s0 + 1, '/');
	StringBetweenCat(t, s0, s1);
	assert((str = StringGet(t)) && !strcmp(sup = "/neil//Common/", str));
	printf("String: %s\n", StringGet(t));
	StringClear(t);

	printf("StringSep:\n");
	StringCat(t, "/foo///bar/qux//xxx");
	printf("String: '%s'\n", StringGet(t));
	assert(t);
	s = 0;
	printf("entering\n");
	while((sep = StringSep(&t, "/", &is_delim))) {
		printf("here\n");
		printf("StringSep: '%s' '%s'\n", StringGet(sep), StringGet(t));
		switch(s++) {
			case 0:
				assert((str = StringGet(sep)) && !strcmp(sup = "", str));
				break;
			case 1:
				assert((str = StringGet(sep)) && !strcmp(sup = "foo//", str));
				break;
			case 2:
				assert((str = StringGet(sep)) && !strcmp(sup = "bar", str));
				break;
			case 3:
				assert((str = StringGet(sep)) && !strcmp(sup = "qux/", str));
				break;
			case 4:
				assert((str = StringGet(sep)) && !strcmp(sup = "xxx", str));
				break;
			default:
				assert((str = StringGet(sep), 0));
				break;
		}
		String_(&sep);
	}
	assert(!t);

	t = String();
	StringCat(t, "words separated by spaces -- and, punctuation!!!");
	printf("original: \"%s\"\n", StringGet(t));
	StringCat(t, "word!!!");
	printf("modified: \"%s\"\n", StringGet(t));
	s = 0;
	while((sep = StringSep(&t, " .,;:!-", 0))) {
		s++;
		printf("token => \"%s\"\n", StringGet(sep));
		String_(&sep);
	}
	assert(s == 16 && !t);

	t = String();
	for(i = 0; i < 300; i++) StringPrintCat(t, "%c", '0' + i % 10);
	printf("t: \"%s\"\n", StringGet(t));
	s = StringLength(t);
	assert(s == 300);*/

	return EXIT_SUCCESS;
}

