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

#if 0

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
#endif

static int write_file(const struct Line *const line, FILE *const fp) {
	char a[32];
	assert(line && fp);
	TextLineSource(line, a, sizeof a);
	return fputs(a, fp) != EOF && fputs(": ", fp) != EOF
		&& fputs(TextLineGet(line), fp) != EOF && fputc('\n', fp) != EOF;
}

/** Entry point.
 @param argc: The number of arguments, starting with the programme name.
 @param argv: The arguments.
 @return Either EXIT_SUCCESS or EXIT_FAILURE. */
int main(void) {
	struct Text *text = 0;
	FILE *fp = 0;
	const char *e = 0;
	do {
		if(!(text = Text())) { e = "Text"; break; }
		if(!(fp = fopen("../../src/Text.h", "r"))
			|| !TextFile(text, fp, "Text.h")
			|| fclose(fp) == EOF
			|| !(fp = fopen("../../src/Text.c", "r"))
			|| !TextFile(text, fp, "Text.c")
			|| fclose(fp) == EOF) { e = "Text.h"; break; }
		fp = 0;
		if(!TextOutput(text, &write_file, stdout)) { e = "stdout"; break; }
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
	fclose(fp);
	Text_(&text);
	return e ? EXIT_FAILURE : EXIT_SUCCESS;
}

#if 0
printf("StringSep:\n");
StringCat(t, "/foo///bar/qux//xxx");
printf("String: '%s'\n", StringGet(t));
assert(t);
s = 0;
printf("entering\n");
while((sep = StringSep(&t, "/", &is_delim))) {
	printf("here\n");
	printf("StringSep: '%s' '%s'\n", StringGet(sep), StringGet(t));
	switch(s++) {
		case 0:
			assert((str = StringGet(sep)) && !strcmp(sup = "", str));
			break;
		case 1:
			assert((str = StringGet(sep)) && !strcmp(sup = "foo//", str));
			break;
		case 2:
			assert((str = StringGet(sep)) && !strcmp(sup = "bar", str));
			break;
		case 3:
			assert((str = StringGet(sep)) && !strcmp(sup = "qux/", str));
			break;
		case 4:
			assert((str = StringGet(sep)) && !strcmp(sup = "xxx", str));
			break;
		default:
			assert((str = StringGet(sep), 0));
			break;
	}
	String_(&sep);
}
assert(!t);

t = String();
StringCat(t, "words separated by spaces -- and, punctuation!!!");
printf("original: \"%s\"\n", StringGet(t));
StringCat(t, "word!!!");
printf("modified: \"%s\"\n", StringGet(t));
s = 0;
while((sep = StringSep(&t, " .,;:!-", 0))) {
	s++;
	printf("token => \"%s\"\n", StringGet(sep));
	String_(&sep);
}
assert(s == 16 && !t);

printf("StringMatch:\n");
StringMatch(t, tpattern, tpattern_size);
printf("String: %s\n", StringGet(t));
assert((str = StringGet(t)) && !strcmp(sup
									   = "<a href = \"Test%Test\">Test%Test</a> yo <em>YO</em>", str));
StringClear(t);

#endif
