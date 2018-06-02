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

/** Writes "<source>: <line>\n" in {line} to {fp}. */
static int write_file(const struct Line *const line, FILE *const fp) {
	char a[32];
	assert(line && fp);
	LineSource(line, a, sizeof a);
	return fputs(a, fp) != EOF && fputs(": ", fp) != EOF
		&& fputs(LineGet(line), fp) != EOF && fputc('\n', fp) != EOF;
}

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

/** Entry point.
 @param argc: The number of arguments, starting with the programme name.
 @param argv: The arguments.
 @return Either EXIT_SUCCESS or EXIT_FAILURE. */
int main(void) {
	struct Text *text = 0;
	FILE *fp = 0;
	const char *s;
	const char *e = 0;
	do {
		if(!(text = Text())) { e = "Text"; break; }
		/* Load all. */
		if(!(fp = fopen(head, "r"))
			|| !TextFile(text, fp, head)
			|| !pfclose(&fp)) { e = head; break; }
		if(!TextNew(text)) { e = "edit"; break; }
		if(!(fp = fopen(body, "r"))
			|| !TextFile(text, fp, body)
			|| !pfclose(&fp)) { e = body; break; }
		/* Split the text into words. */
		TextReset(text);
		while((s = TextNext(text))) {
			printf(">>%s\n", s);
		}
		/* Output. */
		if(!TextOutput(text, &write_file, stdout)) { e = "stdout"; break; }
		if(!TextPrint(text, stdout, "Laa:%a:%s\n")) { e = "stdout"; break; }
#if 0
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
#endif
	} while(0); if(e) perror(e);
	if(!pfclose(&fp)) perror("shutdown");
	Text_(&text);
	return e ? EXIT_FAILURE : EXIT_SUCCESS;
}
