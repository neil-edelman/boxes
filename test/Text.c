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
/** @implements	TextAction */
static void amp(struct Text *const this) { TextCopy(this, "&amp;"); }
/** @implements	TextAction */
static void lt(struct Text *const this) { TextCopy(this, "&lt;"); }
/** @implements	TextAction */
static void gt(struct Text *const this) { TextCopy(this, "&gt;"); }

static const struct TextPattern tpattern[] = {
	{ "\\url{",  "}", &url },
	{ "\\cite{", "}", &cite },
	{ "{",       "}", &em },
	{ "&",       0,   &amp },
	{ "<",       0,   &lt },
	{ ">",       0,   &gt }
};
static const size_t tpattern_size = sizeof tpattern / sizeof *tpattern;

/** The is a test of Table.
 @param argc	Count
 @param argv	Vector. */
int main(int argc, char *argv[]) {
	FILE *fp = 0;
	const char *const fn = "/Users/neil/Movies/Common/Text/src/Text.c";
	struct Text *t = 0;
	const char *str, *sup = 0;
	enum { E_NO, E_T, E_ASRT, E_FP } e = E_NO;

	printf("Testing\n");
	do {
		if(!(t = Text())
			|| !TextNCopy(t, "TestText", 4))
			{ e = E_T; break; }
		if(strcmp(sup = "Test", str = TextToString(t)))
			{ e = E_ASRT; break; }
		printf("Text: %s\n", TextToString(t));
		TextTransform(t, "\\url{%s%%%s} yo {YO}");
		if(strcmp(sup = "\\url{Test%Test} yo {YO}", str = TextToString(t)))
			{ e = E_ASRT; break; }
		printf("Text: %s\n", TextToString(t));
		TextMatch(t, tpattern, tpattern_size);
		if(strcmp(sup = "<a href = \"Test%Test\">Test%Test</a> yo <em>YO</em>",
			str = TextToString(t)))
			{ e = E_ASRT; break; }
		printf("Text: %s\n", TextToString(t));
		if(!(fp = fopen(fn, "r")))
			{ e = E_FP; break; }
		TextClear(t);
		if(!TextFileCat(t, fp) || !TextMatch(t, tpattern, tpattern_size))
			{ e = E_T; break; }
		printf("Text: %s", TextToString(t));
	} while(0);

	switch(e) {
		case E_NO: break;
		case E_T: fprintf(stderr, "Text: %s.\n", TextGetError(t)); break;
		case E_ASRT: fprintf(stderr,"Text: assert failed, <%s> but was <%s>.\n",
			sup, str); break;
		case E_FP: ferror(fp); break;
	}

	Text_(&t);
	fclose(fp);

	return e ? EXIT_FAILURE : EXIT_SUCCESS;
}

