/** Copyright 2015 Neil Edelman, distributed under the terms of the MIT License;
 see readme.txt, or \url{ https://opensource.org/licenses/MIT }.

 This is a test of Text. It is not complete by any means.

 @author	Neil
 @version	1.0; 2017-03
 @since		1.0; 2017-03 */

#include <stdlib.h>	/* EXIT_* */
#include <stdio.h>  /* fprintf */
#include <string.h>	/* strcmp */
#include <assert.h>
#include "../src/Text.h"

static void url(struct Text *const this) {
	TextTrim(this), TextTransform(this, "<a href = \"%s\">%s</a>");
}
static void cite(struct Text *const this) {
	TextTrim(this), TextTransform(this,
		"<a href = \"https://scholar.google.ca/scholar?q=%s\">%s</a>");
}
/** @implements	TextAction */
static void em(struct Text *const this) { TextTransform(this, "<em>%s</em>"); }

static const struct TextPattern tpattern[] = {
	{ "\\url{",  "}", &url },
	{ "\\cite{", "}", &cite },
	{ "{",       "}", &em }
};
static const size_t tpattern_size = sizeof tpattern / sizeof *tpattern;

/** @implements	TextPredicate */
static int is_delim(const char *const str, const char *p) {
	return *++p == '/' || 0 == str ? 0 : -1;
}

/** The is a test of Text.
 @param argc	Count
 @param argv	Vector. */
int main(void) {
#if 0
	FILE *fp = 0;
#endif
	const char *const fn = "/Users/neil/Movies/Common/Text/src/Text.c";
	struct Text *t = 0, *sep = 0;
	const char *str, *sup = 0;
	size_t s, i;
	const char *s0, *s1;

	printf("Testing:\n");

	t = Text();
	assert(t);

#if 0
	printf("TextFileCat:\n");
	errno = 0; {
		if(!(fp = fopen(fn, "r"))) break;
		if(!TextMatch(TextFileCat(t, fp), tpattern, tpattern_size));
		printf("Text: %s", TextGet(t));
		TextClear(t);
	} while(0); if(errno) {
		perror("TextFileCat");
	}
	fclose(fp);
#endif
	printf("TextNCat:\n");
	TextNCat(t, "TestText", (size_t)4);
	printf("Text: %s\n", TextGet(t));
	assert((str = TextGet(t)) && !strcmp(sup = "Test", str));

	printf("TextTransform:\n");
	TextTransform(t, "\\url{%s%%%s} yo {YO}");
	printf("Text: %s\n", TextGet(t));
	assert((str = TextGet(t)) && !strcmp(sup = "\\url{Test%Test} yo {YO}",str));

	printf("TextMatch:\n");
	TextMatch(t, tpattern, tpattern_size);
	printf("Text: %s\n", TextGet(t));
	assert((str = TextGet(t)) && !strcmp(sup
		= "<a href = \"Test%Test\">Test%Test</a> yo <em>YO</em>", str));
	TextClear(t);

	printf("TextBetweenCat:\n");
	s0 = strchr(fn + 1, '/');
	s1 = strchr(s0 + 1, '/');
	TextBetweenCat(t, s0, s1);
	assert((str = TextGet(t)) && !strcmp(sup = "/neil/", str));
	s0 = strchr(s1 + 1, '/');
	s1 = strchr(s0 + 1, '/');
	TextBetweenCat(t, s0, s1);
	assert((str = TextGet(t)) && !strcmp(sup = "/neil//Common/", str));
	printf("Text: %s\n", TextGet(t));
	TextClear(t);

	printf("TextSep:\n");
	TextCat(t, "/foo///bar/qux//xxx");
	printf("Text: '%s'\n", TextGet(t));
	assert(t);
	s = 0;
	printf("entering\n");
	while((sep = TextSep(&t, "/", &is_delim))) {
		printf("here\n");
		printf("TextSep: '%s' '%s'\n", TextGet(sep), TextGet(t));
		switch(s++) {
			case 0:
				assert((str = TextGet(sep)) && !strcmp(sup = "", str));
				break;
			case 1:
				assert((str = TextGet(sep)) && !strcmp(sup = "foo//", str));
				break;
			case 2:
				assert((str = TextGet(sep)) && !strcmp(sup = "bar", str));
				break;
			case 3:
				assert((str = TextGet(sep)) && !strcmp(sup = "qux/", str));
				break;
			case 4:
				assert((str = TextGet(sep)) && !strcmp(sup = "xxx", str));
				break;
			default:
				assert((str = TextGet(sep), 0));
				break;
		}
		Text_(&sep);
	}
	assert(!t);

	t = Text();
	TextCat(t, "words separated by spaces -- and, punctuation!!!");
	printf("original: \"%s\"\n", TextGet(t));
	TextCat(t, "word!!!");
	printf("modified: \"%s\"\n", TextGet(t));
	s = 0;
	while((sep = TextSep(&t, " .,;:!-", 0))) {
		s++;
		printf("token => \"%s\"\n", TextGet(sep));
		Text_(&sep);
	}
	assert(s == 16 && !t);

	t = Text();
	for(i = 0; i < 300; i++) TextPrintCat(t, "%c", '0' + i % 10);
	printf("t: \"%s\"\n", TextGet(t));
	s = TextLength(t);
	assert(s == 300);

	Text_(&t);
	assert(!t && !sep);

	return EXIT_SUCCESS;
}

