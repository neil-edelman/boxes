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

#define STACK_NAME Size
#define STACK_TYPE size_t
#include "Stack.h"

/** @implements LineAction */
static void show(struct Line *const line) {
	assert(line);
	printf("Line %lu: <%s>.\n", (long unsigned)LineNo(line),TextGet(LineText(line)));
}

/** @implements LineAction */
static void trim(struct Line *const line) {
	assert(line);
	TextTrim(LineText(line));
}

/** @implements LineAction */
static void rtrim(struct Line *const line) {
	assert(line);
	TextRightTrim(LineText(line));
}

/** @implements LinePredicate */
static int collapse_para(struct Line *const line) {
	struct Line *const prev = LinePrevious(line);
	assert(line);
	/*printf("\"%s\".%lu -> \"%s\".%lu\n",
		TextGet(LineText(prev)), (long unsigned)LineNo(prev),
		TextGet(LineText(line)), (long unsigned)LineNo(line));*/
	if(TextGet(LineText(line))[0] != '\0') return 1;
	if(!prev || TextGet(LineText(prev))[0] == '\0') return 0;
	return 1;
}

/** @implements LineAction */
static void reinsert_newlines(struct Line *const line) {
	assert(line);
	TextCat(LineText(line), "\n");
}

/** @implements TextPredicate */
static int no_empty(const char *const string, const char *sub) {
	UNUSED(string);
	return isspace(sub[1]) ? 0 : 1;
}

/** \cite{Wilber1998} \url{http://xxyxyz.org/line-breaking/}. */
static void word_wrap(struct Line *const line) {
	struct SizeStack *offsets = SizeStack();
	struct Line *l;
	size_t count, *o;
	assert(line);
	errno = 0; do { /* try */
		if(!(o = SizeStackNew(offsets))) break;
		*o = 0;
		/* Count how many words in the paragraph. */
		for(count = 0, l = line; l && TextLength(LineText(l));
			l = LineNext(l), count++);
	} while(0); if(errno) { /* catch */
		perror("word_wrap");
	} { /* finally */
		SizeStack_(&offsets);
	}
}

/** Entry point.
 @param argc: The number of arguments, starting with the programme name.
 @param argv: The arguments.
 @return Either EXIT_SUCCESS or EXIT_FAILURE. */
int main(void) {
	struct Text *foo, *text;
	struct Story *story = 0;
	FILE *fp = 0;
	enum { E_NO, E_STDERR, E_STORY } e = E_NO;
	do {

		if(!(story = Story())) { e = E_STORY; break; }

		if(!(fp = fopen("../../src/Story.h", "r"))) { e = E_STDERR; break; }
		StoryFileCat(story, fp);
		fclose(fp), fp = 0;

		/* Delete newlines. */
		StoryForEach(story, &trim);
		/* Collapse multi-line paragraph indents. */
		StoryKeepIf(story, &collapse_para);
		/* Move each word to a line. */
		StorySplit(story, 0, &no_empty);
		StoryForEach(story, &rtrim);
		/* Prepare for output. */
		StoryForEach(story, &show);
		StoryForEach(story, &reinsert_newlines);

		if(!(fp = fopen("../../result.txt", "w"))) { e = E_STDERR; break; }
		if(!StoryWrite(story, fp)) { e = E_STORY; break; };
		fclose(fp), fp = 0;

		foo = Text();
		TextCat(foo, "blue,red,,,green,");
		printf("Foo: %s\n", TextGet(foo));
		while((text = TextSep(&foo, ",", 0))) {
			printf("Text: %s\n", TextGet(text));
			Text_(&text);
		}
		assert(!foo);
		printf("Foo: %s\n", TextGet(foo));

	} while(0); switch(e) {
		case E_NO:
			break;
		case E_STDERR:
			perror("SdtErr"); break;
		case E_STORY:
			perror("Story"); break;
	} {
		fclose(fp);
		Story_(&story);
	}
	return e ? EXIT_FAILURE : EXIT_SUCCESS;
}
