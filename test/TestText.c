/** Copyright 2015 Neil Edelman, distributed under the terms of the MIT License;
 see readme.txt, or \url{ https://opensource.org/licenses/MIT }.

 This is a test of Text; call it with a .c file.

 @author	Neil
 @version	3.0; 2016-08
 @since		3.0; 2016-08 */

#include <stdlib.h>	/* rand, EXIT_* */
#include <stdio.h>  /* fprintf */
#include <string.h>	/* strcmp strdup */
#include <ctype.h>	/* isspace */
#include "../src/Text.h"

/*#define LIST_NAME Text
#define LIST_TYPE struct Text *
#include "../../List/src/List.h"

struct TextList *list;*/

static void trim(char *const str) {
	char *e = str + strlen(str) - 1, *s = str;
	while(e > str && isspace(*e)) e--;
	e++, *e = '\0';
	while(isspace(*s)) s++;
	if(s - str) memmove(str, s, (size_t)(e - s + 1));
}

/** @implments	TextAction */
static void each(struct Text *const match) {
	printf("each: \"%s\".\n", TextGetBuffer(match));
}
/** Must be in rfc3986 format; \url{https://www.ietf.org/rfc/rfc3986.txt }.
 @implments	TextAction */
static void url(struct Text *const this) {
	trim(TextGetBuffer(this));
	TextAdd(this, "<a href = \"%s\">%s</a>");
}
/** Must be in query format; \url{ https://www.ietf.org/rfc/rfc3986.txt }.
 @implments	TextAction */
static void cite(struct Text *const this) {
	trim(TextGetBuffer(this));
	TextAdd(this, "<a href = \"https://scholar.google.ca/scholar?q=%s\">%s</a>");
}
/** @implments	TextAction */
static void em(struct Text *const this) { TextAdd(this, "<em>%s</em>"); }
/** @implments	TextAction */
static void amp(struct Text *const this) { TextAdd(this, "&amp;"); }
/** @implments	TextAction */
static void lt(struct Text *const this) { TextAdd(this, "&lt;"); }
/** @implments	TextAction */
static void gt(struct Text *const this) { TextAdd(this, "&gt"); }
static void new_docs(struct Text *const); /* needs TextPattern */

static const struct TextPattern tp_docs[] = {
	{ "/** ", "*/", &new_docs }
}, tp_each[] = {
	{ "@", 0, &each }
}, tp_inner[] = {
	{ "\\url{", "}", &url },
	{ "\\cite{", "}", &cite },
	{ "{", "}", &em },
	{ "&", 0, &amp },
	{ "<", 0, &lt },
	{ ">", 0, &gt }
};

/** @implments	TextAction */
static void new_docs(struct Text *const sub) {
	/*printf("new_docs: \"%s\".\n", TextGetBuffer(sub));*/
	/* fixme: each */
	TextMatch(sub, tp_inner, sizeof tp_inner / sizeof *tp_inner);
}

/** The is a test of Text.
 @param argc	Count
 @param argv	Vector. */
int main(int argc, char *argv[]) {
	enum { E_NO_ERR, E_ERRNO, E_TEXT/*, E_LIST*/ } error = E_NO_ERR;
	struct Text *text = 0;
	char *fn;

	if(argc != 2) {
		/*fn = "src/Test.c";*/
		fprintf(stderr, "Needs <filename>.\n");
		return EXIT_FAILURE;
	} else {
		fn = argv[1];
	}

	do {

		/*if(!(list = TextList())) { error = E_LIST; break; }*/
		/*if((text = TextFile("!@#$%%^&*()`\n"))) { error = E_UNEXPECTED;
		 break; }
		printf("Text(\"!@#$%%^&*()`\\n\"): %s.\n", TextGetError(text));*/
		if(!(text = TextFile(fn))) { error = E_TEXT; break; }
		if(!TextMatch(text, tp_docs, sizeof tp_docs / sizeof *tp_docs))
			{ error = E_TEXT; break; }
		printf("%s\n", TextToString(text));

	} while(0);
	switch(error) {
		case E_NO_ERR: break;
		case E_ERRNO: perror(fn); break;
		case E_TEXT:
			fprintf(stderr, "%s: %s.\n", fn, TextGetError(text)); break;
		/*case E_LIST:
			fprintf(stderr, "%s: %s.\n", fn, TextListGetError(list)); break;*/
	}
	{
		Text_(&text);
		/*TextList_(&list);*/
	}

	fprintf(stderr, "Done all tests; %s.\n", error ? "FAILED" : "SUCCEDED");

	return error ? EXIT_FAILURE : EXIT_SUCCESS;
}
