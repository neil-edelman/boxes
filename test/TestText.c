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

/** Helper for {fclose} is a little more robust about null-values.
 @param pfp: A pointer to the file pointer.
 @return Success.
 @throws {fclose} errors. */
static int pfclose(FILE **const pfp) {
	FILE *fp;
	int is;
	if(!pfp || !(fp = *pfp)) return 1;
	/* Whatever {fclose} returns, the file pointer is now useless. "After the
	 call to fclose(), any use of stream results in undefined behavior." */
	is = (fclose(fp) != EOF);
	*pfp = fp = 0;
	return is;
}

/** Splits all the words on in {src} on different lines on before the cursor in
 {dst}.
 @param pwords: If not null, the number of words split is stored.
 @return Success.
 @throws {realloc} errors. */
static int split(const struct Line *const src, struct Text *const dst,
	size_t *pwords) {
	const char *cursor, *start, *end;
	size_t words = 0;
	assert(dst && src);
	for(cursor = LineGet(src); start = trim(cursor), end = next(start);
		cursor = end) {
		assert(start < end);
		if(!LineBetweenCat(LineCopyMeta(src, dst), start, end)) break;
		words++;
	}
	if(pwords) *pwords = words;
	return !end;
}

/** Splits the entire paragraph starting with cusor of {src} into strings
 before the cursor of {dst}. The cursor is updated in {src} to the line after
 or reset if there was no line after. A paragraph is delimited by lines
 composed of only classic white-space; this skips over all the delimiters,
 if the file has words, it outputs them to {dst}, and stops with a delimiter or
 end-of-text, and returns true, otherwise the cursor is reset and it will
 return false.
 @return Whether a paragraph was output.
 @throws realloc */
static int split_para(struct Text *const src, struct Text *const dst) {
	const struct Line *line;
	int is_para = 0;
	size_t words;
	assert(src && dst);
	while((line = TextNext(src))) {
		if(!split(line, dst, &words)) return 0;
		if(words) is_para = 1; else if(is_para) break;
	}
	return is_para;
}

/** Wraps the line at 80.
 @param words: Any words after the cursor is erased. */
static int greedy(struct Text *const words, struct Text *const wrap) {
	const struct Line *word;
	struct Line *line = 0;
	const char *const space = " ", *const space_end = space + 1;
	const char *str;
	size_t line_len = 0;
	assert(words && wrap);
	while((word = TextNext(words))) {
		/*printf("Inserting <%s>.\n", LineGet(word));*/
		if((!line || ((line_len = LineLength(line))
			&& line_len + 1 + LineLength(word) >= 50))
			&& !(line_len = 0, line = LineCopyMeta(word, wrap))) return 0;
		if(line_len) LineBetweenCat(line, space, space_end);
		str = LineGet(word);
		LineBetweenCat(line, str, str + LineLength(word));
		TextRemove(words);
	}
	return 1;
}

/** Expects {head} and {body} to be on the same directory as it is called from.
 Word wraps.
 @return Either EXIT_SUCCESS or EXIT_FAILURE. */
int main(void) {
	FILE *fp = 0;
	struct Text *text = 0, *words = 0, *greed = 0, *divide = 0;
	const struct Line *newline = 0;
	const char *e = 0;
	do { /* Try. */
		if(!(text = Text()) || !(words = Text()) || !(greed = Text())
			|| !(divide = Text())) { e = "Text"; break; }
		/* Load all. In reality, would read from stdin, just testing. */
		if(!(fp = fopen(head, "r"))
			|| !TextFile(text, fp, head)
			|| !pfclose(&fp)) { e = head; break; }
		if(!TextNew(text)) { e = "edit"; break; }
		if(!(fp = fopen(body, "r"))
			|| !TextFile(text, fp, body)
			|| !pfclose(&fp)) { e = body; break; }
		fprintf(stderr, "Loaded files <%s> and <%s>.\n", head, body);
		/* Split the text into words and then wraps them. */
		do {
			/* Insert a double-break between paragraphs. */
			if(newline) LineCopyMeta(newline, greed);
			/*  */
			if(!split_para(text, words)) break; /* Newlines at EOF. */
			if(!greedy(words, greed)) { e = "wrap"; break; };
		} while((newline = TextLine(text)));
		if(e) break;
		/* Output. */
		/*printf("***text:\n");
		if(!TextPrint(text, stdout, "%a: <%s>\n")) { e = "stdout"; break; }
		printf("\n\n***words:\n");
		if(!TextPrint(words, stdout, "%a: <%s>\n")) { e = "stdout"; break; }
		printf("\n\n***wrap:\n");*/
		if(!TextPrint(greed, stdout, "%a: <%s>\n")) { e = "stdout"; break; }
	} while(0); if(e) perror(e); /* Catch. */
	if(!pfclose(&fp)) perror("shutdown"); /* Finally. */
	Text_(&divide), Text_(&greed), Text_(&words), Text_(&text); /* Finally. */
	return e ? EXIT_FAILURE : EXIT_SUCCESS;
}
