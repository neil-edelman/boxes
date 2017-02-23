/** Copyright 2015 Neil Edelman, distributed under the terms of the MIT License;
 see readme.txt, or \url{ https://opensource.org/licenses/MIT }.

 This is a test of Text; call it with a .c file.

 @author	Neil
 @version	3.0; 2016-08
 @since		3.0; 2016-08 */

#include <stdlib.h>	/* rand, EXIT_* */
#include <stdio.h>  /* fprintf */
#include <string.h>	/* strcmp */
#include "../src/Text.h"

#define LIST_NAME Text
#define LIST_TYPE struct Text *
#include "../../List/src/List.h"

/** @implments	Transform */
static void new_docs(char *const match) {
	printf("splits_docs: \"%s\".\n", match);
}
/** @implments	Transform */
static void url(char *const match) {
	printf("url: \"%s\".\n", match);
}
/** @implments	Transform */
static void each(char *const match) {
	printf("each: \"%s\".\n", match);
}
/** @implments	Transform */
static void em(char *const match) {
	printf("em: \"%s\".\n", match);
}
/** @implments	Transform */
static void amp(char *const match) {
	printf("amp: \"%s\".\n", match);
}
/** @implments	Transform */
static void lt(char *const match) {
	printf("lt: \"%s\".\n", match);
}
/** @implments	Transform */
static void gt(char *const match) {
	printf("gt: \"%s\".\n", match);
}

/** The is a test of Text.
 @param argc	Count
 @param argv	Vector. */
int main(int argc, char *argv[]) {
	enum { E_NO_ERR, E_ERRNO, E_UNEXPECTED } error = E_NO_ERR;
	struct Text *text = 0;
	struct TextList *list = 0;
	char *fn;

	if(argc != 2) {
		/*fn = "src/Test.c";*/
		fprintf(stderr, "Needs <filename>.\n");
		return EXIT_FAILURE;
	} else {
		fn = argv[1];
	}

	do {
		struct TextPattern tp_docs[] = {
			{ "/** ", "*/", &new_docs }
		}, tp_each[] = {
			{ "@", 0, &each }
		}, tp_inner[] = {
			{ "\\url{", "}", &url },
			{ "{", "}", &em },
			{ "&", 0, &amp },
			{ "<", 0, &lt },
			{ ">", 0, &gt }
		};

		if((text = TextFile("!@#$%%^&*()`\n"))) { error = E_UNEXPECTED; break; }
		printf("Text(\"!@#$%%^&*()`\\n\"): %s.\n", TextGetError(text));
		if(!(text = TextFile(fn))) { error = E_UNEXPECTED; break; }
		TextMatch(text, tp_docs, sizeof tp_docs / sizeof *tp_docs, 0);
		TextMatch(text, tp_each, sizeof tp_each / sizeof *tp_each, 0);
		TextMatch(text, tp_inner, sizeof tp_inner / sizeof *tp_inner, 0);

	} while(0);
	switch(error) {
		case E_NO_ERR: break;
		case E_ERRNO: perror(fn); break;
		case E_UNEXPECTED:
			fprintf(stderr, "%s: %s.\n", fn, TextGetError(text)); break;
	}
	{
		printf("Destroying Text: \"%s,\"\n", fn);
		Text_(&text);
	}

	fprintf(stderr, "Done all tests; %s.\n", error ? "FAILED" : "SUCCEDED");

	return error ? EXIT_FAILURE : EXIT_SUCCESS;
}
