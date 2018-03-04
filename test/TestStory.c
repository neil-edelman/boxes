/** 2018 Neil Edelman, distributed under the terms of the MIT License;
 see readme.txt, or \url{ https://opensource.org/licenses/MIT }.

 A {Story} is composed of {Text} in a way that makes them easy and fast to
 edit large strings. Requires {List}, {Pool}, and {Text}.

 ${
 Text a = [ "foo bar\n\n", "baz\n", "qux" ]
 a.split(0) = [ "foo", "bar", "baz", "qux" ]
 a.split("a", " ") = [ "foo", "b", "r", "b", "z", "qux" ]
 a.strip(0) = [ "foo bar", "baz", "qux" ]
 a.strip("quxf ") =  [ "oo bar", "baz", "" ]
 a.empty.strip("quxf ") = [ "oo bar", "baz" ]
 a.join(0) = [ "foo bar\n\n baz\n qux" ]
 a.join("") = [ "foo bar\n\nbaz\nqux" ]
 a.sort() = [ "baz\n", "foo bar\n\n", "qux" ]
 a.replace("a", "oo") = [ "foo boor\n\n", "booz\n", "qux" ]
 /a.cat("quxx") = [ "foo bar\n\n", "baz\n", "qux", "quxx" ]/not needed
 a.cat("%d", 42) = [ "foo bar\n\n42", "baz\n42", "qux42" ]
 a.format("%d foo", 42) = [ "foo bar\n\n", "baz\n", "qux", "42 foo" ]
 TextMap b = [ "o"->"a", "a"->"o" ]
 a.substitute(b) = [ "faa bor\n\n", "boz\n", "qux" ]
 }

 @param STORY_DEBUG
 Prints debug information to {stderr}.

 @title		Story
 @author	Neil
 @std		C89/90
 @version	2018-01
 @since		2018-01 */

#include <stdlib.h> /* malloc free */
#include <stdio.h>  /* fprintf fopen */
#include <string.h>	/* strerror */
#include <limits.h>	/* INT_MAX */
#include <assert.h>	/* assert */
#include <errno.h>	/* errno */
#include "../src/Story.h"

/** @implements StoryLinePredicate */
static int show(const struct Text *const text, const size_t position) {
	assert(text);
	printf("Line %lu: %s", (long unsigned)position, TextGet(text));
	return 1;
}

/** @implements StoryLinePredicate */
static int delete(const struct Text *const text, const size_t position) {
	assert(text);
	if(rand() < RAND_MAX >> 1) {
		printf("Deleting line %lu: %s", (long unsigned)position, TextGet(text));
		return 0;
	} else {
		printf("Keeping line %lu: %s", (long unsigned)position, TextGet(text));
		return 1;
	}
}

/** Entry point.
 @param argc: The number of arguments, starting with the programme name.
 @param argv: The arguments.
 @return Either EXIT_SUCCESS or EXIT_FAILURE. */
int main(void) {
	struct Story *story = 0;
	FILE *read = 0, *write = 0;
	enum { E_NO, E_STDERR, E_STORY } e = E_NO;
	do {
		if(!(read = fopen("../../src/Story.h", "r"))) { e = E_STDERR; break; }
		if(!(story = Story())) { e = E_STORY; break; }
		StoryFileCat(story, read);
		fclose(read), read = 0;
		StoryKeepIf(story, &show);
		StoryKeepIf(story, &delete);
		StoryKeepIf(story, &show);
		if(!(write = fopen("../../result.txt", "w"))) { e = E_STDERR; break; }
		StorySplit(story, 0, 0);
		if(!StoryWrite(story, write)) { e = E_STORY; break; };
		StoryKeepIf(story, &show);
	} while(0); switch(e) {
		case E_NO:
			break;
		case E_STDERR:
			perror("SdtErr"); break;
		case E_STORY:
			perror("Story"); break;
			/*fprintf(stderr, "Error: %s.\n", StoryGetError(story)); break;*/
	} {
		fclose(read);
		fclose(write);
		Story_(&story);
	}
	return e ? EXIT_FAILURE : EXIT_SUCCESS;
}
