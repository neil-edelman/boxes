/** Copyright 2015 Neil Edelman, distributed under the terms of the MIT License;
 see readme.txt, or \url{ https://opensource.org/licenses/MIT }.

 This is a test of Text.

 @author	Neil
 @version	1.0; 2017-03
 @since		1.0; 2017-03 */

#include <stdlib.h>	/* EXIT_* */
#include <stdio.h>  /* fprintf */
#include <string.h>	/* strcmp */
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
	FILE *fp = 0;
	const char *const fn = "/Users/neil/Movies/Common/Text/src/Text.c";
	struct Text *t = 0, *sep = 0;
	const char *str, *sup = 0;
	enum { E_NO, E_T, E_ASRT, E_FP } e = E_NO;

	printf("Testing\n");
	do {
		unsigned s, i;
		const char *s0, *s1;
		printf("Text:\n");
		if(!(t = Text()))
			{ e = E_T; break; }

#if 0
		printf("\nTextFileCat:\n");
		if(!(fp = fopen(fn, "r")))
			{ e = E_FP; break; }
		if(!TextFileCat(t, fp) || !TextMatch(t, tpattern, tpattern_size))
			{ e = E_T; break; }
		printf("Text: %s", TextGet(t));
		TextClear(t);
#endif

		printf("\nTextNCat:\n");
		if(!(TextNCat(t, "TestText", (size_t)4)))
			{ e = E_T; break; }
		if(strcmp(sup = "Test", str = TextGet(t)))
			{ e = E_ASRT; break; }
		printf("Text: %s\n", TextGet(t));

		printf("\nTextTransform:\n");
		TextTransform(t, "\\url{%s%%%s} yo {YO}");
		if(strcmp(sup = "\\url{Test%Test} yo {YO}", str = TextGet(t)))
			{ e = E_ASRT; break; }
		printf("Text: %s\n", TextGet(t));

		printf("\nTextMatch:\n");
		TextMatch(t, tpattern, tpattern_size);
		if(strcmp(sup = "<a href = \"Test%Test\">Test%Test</a> yo <em>YO</em>",
			str = TextGet(t)))
			{ e = E_ASRT; break; }
		printf("Text: %s\n", TextGet(t));
		TextClear(t);

		printf("\nTextBetweenCat:\n");
		s0 = strchr(fn + 1, '/');
		s1 = strchr(s0 + 1, '/');
		TextBetweenCat(t, s0, s1);
		if(strcmp(sup = "/neil/", str = TextGet(t)))
			{ e = E_ASRT; break; }
		s0 = strchr(s1 + 1, '/');
		s1 = strchr(s0 + 1, '/');
		TextBetweenCat(t, s0, s1);
		if(strcmp(sup = "/neil//Common/", str = TextGet(t)))
			{ e = E_ASRT; break; }
		printf("Text: %s\n", TextGet(t));
		TextClear(t);

		printf("\nTextSplit:\n");
		TextCat(t, "/foo///bar/qux//xxx");
		printf("Text: '%s'\n", TextGet(t));
		s = 0;
		while((sep = TextSep(t, "/", &is_delim))) {
			printf("TextSplit: '%s'\n", TextGet(sep));
			switch(s++) {
				case 0: if(strcmp(sup = "", str = TextGet(sep)))
					e = E_ASRT; break;
				case 1: if(strcmp(sup = "foo//", str = TextGet(sep)))
					e = E_ASRT; break;
				case 2: if(strcmp(sup = "bar", str = TextGet(sep)))
					e = E_ASRT; break;
				case 3: if(strcmp(sup = "qux/", str = TextGet(sep)))
					e = E_ASRT; break;
				case 4: if(strcmp(sup = "xxx", str = TextGet(sep)))
					e = E_ASRT; break;
				default: sup = "(null)", str = TextGet(sep),
					e = E_ASRT; break;
			}
			Text_(&sep);
			if(e) break;
		}
		if(e) break;
		if(strcmp(sup = "", str = TextGet(t)))
			{ e = E_ASRT; break; }

		TextCat(t, "words separated by spaces -- and, punctuation!");
		while((sep = TextSep(t, " .,;:!-", 0))) {
			printf("token => \"%s\"\n", TextGet(sep));
			Text_(&sep);
		}
		printf("original: '%s'\n", TextGet(t));
		TextCat(t, "word");
		while((sep = TextSep(t, " .,;:!-", 0))) {
			printf("token => \"%s\"\n", TextGet(sep));
			Text_(&sep);
		}
		printf("original: '%s'\n", TextGet(t)); /* hmm */
		TextClear(t);

		for(i = 0; i < 300; i++) {
			TextPrintCat(t, "%c", '0' + i % 10);
		}

	} while(0);

	switch(e) {
		case E_NO: break;
		case E_T: fprintf(stderr, "Text exception: %s.\n", TextGetError(t));
			break;
		case E_ASRT: fprintf(stderr,"Text: assert failed, '%s' but was '%s'.\n",
			sup, str); break;
		case E_FP: ferror(fp); break;
	}

	Text_(&t);
	fclose(fp);
	Text_(&sep);

	printf("Text tests %s.\n", e ? "FAILED" : "SUCCEEDED");
	return e ? EXIT_FAILURE : EXIT_SUCCESS;
}

