/** 2018 Neil Edelman, distributed under the terms of the MIT License;
 see readme.txt, or \url{ https://opensource.org/licenses/MIT }.

 A {Story} is composed of {Text} in a way that makes them easy and fast to
 edit large strings.

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
#include "Text.h"
#include "Story.h"

/* @fixme {Text} is a fixed length, and we could do it with one less level of
 indirection. {Text} is outside this class and I don't know how best to get
 it? Wait, it's C, privacy doesn't exist in the same compilation unit; relying
 on mangled names to access them directly is a hack, but possible. */

#define POOL_NAME Text
#define POOL_TYPE struct Text *
#include "Pool.h" /* Defines {TextPool}. */

#define LIST_NAME Line
#define LIST_TYPE struct TextPool *
#include "List.h" /* Defines {LineList}. */

enum Error {
	E_NO_ERROR,
	E_ERRNO,
	E_PARAMETER,
	E_LINE
};
static const char *const error_explination[] = {
	"no error",
	0,
	"parameter out-of-range",
	"error on line"
};
static enum Error global_error = E_NO_ERROR;
static int        global_errno_copy;



struct Story {
	struct LineList lines;
	struct TextPool texts;
	int errno_copy;
	enum Error error;
};

/** @implements Metric */
static int difference(int *const a, int *const b) {
	return *a - *b;
}

/** Prints debug information if {STORY_DEBUG} is turned on, otherwise it does
 nothing and should be optimised out. */
static void debug(struct Story *const this, const char *const fn,
	const char *const fmt, ...) {
#ifdef STORY_DEBUG
	const size_t length = this->text ? strlen(this->text) : 0;
	va_list parg;
	va_start(parg, fmt);
	fprintf(stderr, "Text.%s: ", fn);
	vfprintf(stderr, fmt, parg);
	va_end(parg);
#else
	UNUSED(this); UNUSED(fn); UNUSED(fmt);
#endif
}

/** Entry point.
 @param argc: The number of arguments, starting with the programme name.
 @param argv: The arguments.
 @return Either EXIT_SUCCESS or EXIT_FAILURE. */
int main(void) {
	struct Story *story = 0;
	enum { E_NO, E_STDERR, E_STORY } e = E_NO;
	do {
		FILE *fp;
		if(!(fp = fopen("../../src/Story.h", "r"))) { e = E_STDERR; break; }
		if(!(story = Story())) { e = E_STORY; break; }
		StoryFileCat(story, fp);
		Story_(&story);
	} while(0); switch(e) {
		case E_NO:
			break;
		case E_STDERR:
			perror("Story"); break;
		case E_STORY:
			fprintf(stderr, "Error: %s.\n", StoryGetError(story)); break;
	}
	return e ? EXIT_FAILURE : EXIT_SUCCESS;
}

/** Destructor.
 @param story_ptr A reference to the object that is to be deleted. */
void Story_(struct Story **const story_ptr) {
	struct Story *story;
	if(!story_ptr || !(story = *story_ptr)) return;
	fprintf(stderr, "~Story: erase, #%p.\n", (void *)story);
	free(story), story = *story_ptr = 0;
}

/** Constructor.
 @return An object or a null pointer, if the object couldn't be created. */
struct Story *Story(void) {
	struct Story *story;
	if(!(story = malloc(sizeof(struct Story)))) {
		perror("Story constructor");
		Story_(&story);
		return 0;
	}
	LineListClear(&story->lines);
	story->error = E_NO_ERROR;
	fprintf(stderr, "Story: new, #%p.\n", (void *)story);
	return story;
}

/** Concatenates the contents of the text file, {fp}, after the active line.
 One {Line} per line. On success, the read cursor will be at the end.
 @return {this}.
 @throws E_PARAMETER, E_OVERFLOW, E_ERRNO */
struct Story *StoryFileCat(struct Story *const this, FILE *const fp) {
	struct Text *line;
	if(!this || !fp) return 0;
	printf("StoryFileCat:\n");
	for( ; ; ) {
		if(!(line = Text())) { this->error = E_ERRNO; return 0; }
		if(!TextFileLineCat(line, fp)) break;
		printf("<%s>\n", TextGet(line));
		Line
		Text_(&line);
	}
	debug(this, "StoryFileCat", "appended a file.\n");
	return this;
}

/** Resets the error flag.
 @return A lower-case string, (or in the case of a E_ERRNO, the first letter
 has an extraneous upper case on most systems,) without any punctuation, that
 explains the last error associated with {this}; can be null. */
const char *StoryGetError(struct Story *const this) {
	const char *str;
	enum Error *perr;
	int *perrno;	
	perr   = this ? &this->error      : &global_error;
	perrno = this ? &this->errno_copy : &global_errno_copy;
	if(!(str = error_explination[*perr])) str = strerror(*perrno);
	*perr = 0, *perrno = 0;
	return str;
}
