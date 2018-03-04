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
 @since		2018-01
 @fixme This is a good way of error reporting; very short. Propagate. */

#define STORY_DEBUG

#include <stdlib.h> /* malloc free */
#include <stdio.h>  /* fprintf fopen */
#include <string.h>	/* strerror */
#include <limits.h>	/* INT_MAX */
#include <errno.h>	/* errno */
#ifdef STORY_DEBUG /* <-- debug */
#include <stdarg.h>	/* va_* */
#endif /* debug --> */
#include "Story.h"

struct Line {
	struct Text *text;
	size_t no;
};
#define LIST_NAME Line
#define LIST_TYPE struct Line
#include "List.h" /* Defines {LineList} and {LineListNode}. */

#define POOL_NAME Line
#define POOL_TYPE struct LineListNode
#define POOL_PARENT struct LineList
#define POOL_UPDATE struct Line
#define POOL_NO_MIGRATE_POINTER
#include "Pool.h" /* Defines {TextPool}. */

/*enum Error {
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
static int        global_errno_copy;*/



struct Story {
	struct LineList lines;
	struct LinePool *pool;
};

/** Prints debug information if {STORY_DEBUG} is turned on, otherwise it does
 nothing and should be optimised out. */
static void debug(struct Story *const this, const char *const fn,
	const char *const fmt, ...) {
#ifdef STORY_DEBUG
	va_list parg;
	va_start(parg, fmt);
	fprintf(stderr, "Story.%s: ", fn);
	vfprintf(stderr, fmt, parg);
	va_end(parg);
#else
	UNUSED(fn); UNUSED(fmt);
#endif
	UNUSED(this);
}

/** Destructor.
 @param story_ptr A reference to the object that is to be deleted. */
void Story_(struct Story **const story_ptr) {
	struct Story *story;
	if(!story_ptr || !(story = *story_ptr)) return;
	fprintf(stderr, "~Story: erase, #%p.\n", (void *)story);
	LinePool_(&story->pool);
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
	story->pool = 0;
	fprintf(stderr, "Story: new, #%p.\n", (void *)story);
	if(!(story->pool = LinePool(&LineListMigrate, &story->lines))) {
		fprintf(stderr, "Story: %s.\n", LinePoolGetError(0));
		Story_(&story);
		return 0;
	}
	return story;
}

/** Concatenates the contents of the text file, {fp}, after the active line.
 One {Line} per line. On success, the read cursor will be at the end.
 @return Success.
 @throws E_OVERFLOW, E_ERRNO */
int StoryFileCat(struct Story *const this, FILE *const fp) {
	struct Text *text = 0;
	struct LineListNode *line;
	size_t no = 0;
	enum { E_NO, E_TEXT, E_LINES } e = E_NO;
	if(!this || !fp) return 0;
	for( ; ; ) {
		if(!(text = Text())) { e = E_TEXT; break; }
		if(!TextFileLineCat(text, fp)) break;
		no++;
		if(!(line = LinePoolNew(this->pool))) { e = E_LINES; break; }
		line->data.text = text;
		line->data.no = no;
		LineListPush(&this->lines, &line->data);
	} switch(e) {
		case E_NO:
			if(!TextIsError(text)) break;
			e = E_TEXT;
		case E_TEXT:
			fprintf(stderr, "Text: %s.\n", TextGetError(text));
			break;
		case E_LINES:
			fprintf(stderr,"Line: %s.\n",LinePoolGetError(this->pool));
			break;
	}
	/* The last line is empty and does not have any references; must delete
	 (deleting a null does nothing.) */
	Text_(&text);
	if(!e) debug(this, "FileCat", "appended a file.\n");
	return !e;
}

int StoryWrite(struct Story *const this, FILE *const fp) {
	struct Line *line;
	if(!this || !fp) return 0;
	for(line = LineListFirst(&this->lines); line; line = LineListNext(line)) {
		if(fputs(TextGet(line->text), fp) == EOF) return 0;
	}
	return 1;
}

/** Executes {pred(text line, index)} for all lines and deletes those that
 return false. */
void StoryKeepIf(struct Story *const this, const StoryLinePredicate pred) {
	struct Line *line, *nextline;
	/*size_t number = 1;*/
	if(!this || !pred) return;
	for(line = LineListFirst(&this->lines); line; line = nextline) {
		nextline = LineListNext(line);
		if(pred(line->text, line->no)) { /*number++;*/ continue; }
		LineListRemove(line);
		Text_(&line->text);
		LinePoolRemove(this->pool, (struct LineListNode *)line);
	}
}

void StorySplit(struct Story *const this, const char *delims,
	const TextPredicate pred) {
	struct Line *line;
	struct LineListNode *pool;
	struct Text *text;
	if(!this) return;
	for(line = LineListFirst(&this->lines); line; line = LineListNext(line)) {
		while((text = TextSep(line->text, delims, pred))) {
			if(!(pool = LinePoolUpdateNew(this->pool, &line))) {
				Text_(&text);
				fprintf(stderr, "StorySplit: %s.\n",
					LinePoolGetError(this->pool));
				return;
			}
			pool->data.text = text;
			pool->data.no = line->no;
			LineListAddBefore(line, &pool->data);
		}
		if(TextIsError(line->text)) {
			fprintf(stderr, "StorySplit: %s.\n", TextGetError(line->text));
			return;
		}
	}
}
