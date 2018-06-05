/** 2018 Neil Edelman, distributed under the terms of the MIT License;
 see readme.txt, or \url{ https://opensource.org/licenses/MIT }.

 A {Story} is composed of {Text} in a way that makes them easy, (and faster,)
 to edit large strings. Requires {List}, {Pool}, and {Text}.

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
#include <stdio.h>  /* fprintf fopen FILE */
#include <string.h>	/* strerror */
#include <limits.h>	/* INT_MAX */
#include <assert.h>	/* assert */
#include <errno.h>	/* errno */
#include "../src/Text.h"
#include "split.h"

#if 0
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
#endif

static const char *const head = "abstract.txt",
	*const body = "lorem.txt";

/** Helper for {fclose} is a little more robust.
 @param pfp: A pointer to the file pointer.
 @return Success.
 @throws {fclose} errors. */
static int pfclose(FILE **const pfp) {
	FILE *fp;
	int is;
	if(!pfp || !(fp = *pfp)) return 1;
	is = (fclose(fp) != EOF);
	*pfp = fp = 0;
	return is;
}

/** Splits all the words on different lines. */
static int split(struct Text *const text) {
	const struct Line *line;
	struct Line *word;
	const char *cursor, *start, *end;
	enum { EMPTY = 1, BLANK = 2 } flags = 0;
	assert(text);
	TextReset(text);
	while((line = TextNext(text))) {
		for(cursor = LineGet(line), flags |= EMPTY;
			start = trim(cursor), end = next(start); cursor = end) {
			assert(start < end);
			if(!(word = TextCopyBetween(text, start, end))) return 0;
			flags &= ~EMPTY;
		}
		/* Any lines made of entirely white-space are collapsed into one
		 blank line. */
		if(flags & EMPTY) {
			if(!(flags & BLANK)) TextCopyBetween(text, 0, 0), flags |=BLANK;
		} else {
			flags &= ~BLANK;
		}
		/* Remove the line once all the words are separated. */
		TextRemove(text);
	}
	return 1;
}

/** Expects {head} and {body} to be on the same directory as it is called from.
 Word wraps.
 @return Either EXIT_SUCCESS or EXIT_FAILURE. */
int main(void) {
	FILE *fp = 0;
	struct Text *text = 0, *words = 0;
	const char *e = 0;
	do {
		if(!(text = Text()) || !(words = Text())) { e = "Text"; break; }
		/* Load all. In reality, would read from stdin, just testing. */
		if(!(fp = fopen(head, "r"))
			|| !TextFile(text, fp, head)
			|| !pfclose(&fp)) { e = head; break; }
		if(!TextNew(text)) { e = "edit"; break; }
		if(!(fp = fopen(body, "r"))
			|| !TextFile(text, fp, body)
			|| !pfclose(&fp)) { e = body; break; }
		fprintf(stderr, "Loaded files <%s> and <%s>.\n", head, body);
		/* Split the text into words. */
		if(!split(text)) { e = "split"; break; }
		/* Output. */
		if(!TextPrint(text, stdout, "%a: <%s>\n")) { e = "stdout"; break; }
	} while(0); if(e) perror(e);
	if(!pfclose(&fp)) perror("shutdown");
	Text_(&words), Text_(&text);
	return e ? EXIT_FAILURE : EXIT_SUCCESS;
}
