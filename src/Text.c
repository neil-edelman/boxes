/** 2018 Neil Edelman, distributed under the terms of the MIT License;
 see readme.txt, or \url{ https://opensource.org/licenses/MIT }.

 A {Text} is composed of {String} in a way that makes them easy to edit
 strings. Requires {List}, {Pool}, and {String}.
 Each {Text} is made up of lines. You can stick whatever you want in here,
 including new-lines or not.

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

 @param TEXT_TEST
 

 @title		Text
 @author	Neil
 @std		C89/90
 @version	2018-01
 @since		2018-01 */

#include <stdlib.h> /* malloc free */
#include <stdio.h>  /* fprintf fopen */
#include <string.h>	/* strerror */
#include <limits.h>	/* INT_MAX */
#include <errno.h>	/* errno */
#ifdef STORY_DEBUG /* <-- debug */
#include <stdarg.h>	/* va_* */
#endif /* debug --> */

#include "String.h"
#include "Text.h"

struct LineFile {
	struct String line;
	char *source;
	size_t source_no;
};

struct LineNew {
	struct String line;
};

#define LIST_NAME Line
#define LIST_TYPE struct String
#include "List.h" /* Defines {LineList} and {LineListNode}. */

#define POOL_NAME Line
#define POOL_TYPE struct LineListNode
#define POOL_MIGRATE_EACH &LineListNodeMigrate
#define POOL_UPDATE struct Line
#include "Pool.h" /* Defines {LinePool}. */

#define POOL_NAME Strdup
#define POOL_TYPE char *
#define POOL_MIGRATE_ALL &strdup_migrate
#include "Pool.h" /* Defines {StrdupPool}. */

struct Text {
	struct LineList lines;
	struct LinePool *lines_pool;
	struct StrdupPool *sources;
};

/** Prints debug information if {STORY_DEBUG} is turned on, otherwise it does
 nothing and should be optimised out. */
static void debug(struct Text *const this, const char *const fn,
	const char *const fmt, ...) {
#ifdef STORY_DEBUG
	va_list parg;
	va_start(parg, fmt);
	fprintf(stderr, "Text.%s: ", fn);
	vfprintf(stderr, fmt, parg);
	va_end(parg);
#else
	UNUSED(fn); UNUSED(fmt);
#endif
	UNUSED(this);
}

/** Destructor.
 @param story_ptr A reference to the object that is to be deleted. */
void Text_(struct Text **const story_ptr) {
	struct Text *story;
	if(!story_ptr || !(story = *story_ptr)) return;
	fprintf(stderr, "~Text: erase, #%p.\n", (void *)story);
	LinePool_(&story->pool);
	free(story), story = *story_ptr = 0;
}

/** Constructor.
 @return An object or a null pointer, if the object couldn't be created. */
struct Text *Text(void) {
	struct Text *story;
	if(!(story = malloc(sizeof(struct Text)))) {
		perror("Text constructor");
		Text_(&story);
		return 0;
	}
	LineListClear(&story->lines);
	story->pool = 0;
	fprintf(stderr, "Text: new, #%p.\n", (void *)story);
	if(!(story->pool = LinePool(&LineListMigrate, &story->lines))) {
		fprintf(stderr, "Text: %s.\n", LinePoolGetError(0));
		Text_(&story);
		return 0;
	}
	return story;
}

/* @fixme Replace with {split}. */
#if 0

/** Action function. */
typedef void (*StringAction)(struct String *const);
/** Predicate function.
 @param string: The string.
 @param sub: The position in the string which you must make a true/false
 decision. Necessarily, {sub \in string}. */
typedef int (*StringPredicate)(const char *const string, const char *sub);

/** Used in \see{StringMatch} as an array of patterns. Recognises brackets.
 @param start: Must be at least one character.
 @param end: can be null, in which case, {start} is the whole text.
 @param transform: if {end}, copies a buffer ({start}, {end}) as argument;
 can be null, it will just ignore. */
struct StringPattern {
	const char *start, *end;
	StringAction transform;
};

/** Separates a new token at the first of the {delims} that satisfy {pred}.
 @return If {string_ptr} or {string} is null, returns null. Otherwise, you must
 call \see{String_} on string pointer. If no tokens are found in {string}, {string} is
 destroyed and the entire {String} is returned.
 @param delims: If null, uses {POSIX} white-space to separate, ({Python}
 {split(' ')}.)
 @param pred: Can be null, in which case, it behaves like {true}.
 @throws ENOMEM?: {IEEE Std 1003.1-2001}; C standard does not say. */
struct String *StringSep(struct String **const string_ptr, const char *delims,
						 const StringPredicate pred) {
	struct String *string, *token;
	char *bork;
	if(!string_ptr || !(string = *string_ptr)) return 0;
	if(!delims || !*delims) delims = " \f\n\r\t\v";
	/* Find. */
	for(bork = string->text; (bork = strpbrk(bork, delims)); bork++) {
		if(!pred || pred(string->text, bork)) break;
	}
	/* Not found. Just return the original. */
	if(!bork) {
		*string_ptr = 0;
		return string;
	}
	/* Or else new text for the split at {bork}. This is a lot of wasted space
	 for large strings.
	 @fixme The error handling is lame; use {errno}? */
	if(!(token = String())) {
		string->error = global_error, global_error = E_NO_ERROR;
		string->errno_copy = global_errno_copy; global_errno_copy = 0;
		return 0;
	}
	if(!cat(token, bork + 1, (size_t)(string->text + string->length - (bork + 1)))){
		string->error = token->error, string->errno_copy = token->errno_copy;
		String_(&token);
		return 0;
	}
	/* Truncate at {bork}. */
	*bork        = '\0';
	string->length = bork - string->text;
	/* {string} is the truncated first token, and {token} is the rest: swap. */
	swap_texts(string, token);
	debug(string, "StringSep", "split.");
	return token;
}

/** {alternate_root("[[[foo]]]bar", '[', ']')}, would return {"]bar"} */
static char *alternate_root(char *const string, const int left,const int right){
	unsigned stack = 1;
	char *s = string + 1;
	
	if(*string != left) return s;
	while(*s) {
		if(*s == left) stack++;
		else if(*s == right && !--stack) return s;
		s++;
	}
	return 0;
}
/* Used in \see{StringMatch}. */
static const char right_side[256] = { '\0', '\0', '\0', '\0', '\0', '\0', '\0',
	'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
	'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
	'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', ')', '\0', '\0',
	'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
	'\0', '\0', '\0', '\0', '\0', '>', '\0', '\0', '\0', '\0', '\0', '\0',
	'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
	'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
	']', '\0', '\0', '\0', '\0', '\'', '\0', '\0', '\0', '\0', '\0', '\0',
	'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
	'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '}', '\0', '\0', '\0',
	'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
	'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
	'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
	'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
	'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
	'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
	'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
	'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
	'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
	'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
	'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0' };




/* private structs */

struct StringBraket {
	int is;
	const struct StringPattern *pattern;
	struct { char *s0, *s1; } bra, ket;
};

struct StringMatch {
	struct String *text;
	char *start, *end;
};

struct StringMatches {
	struct String *parent;
	struct StringMatch *matches;
	size_t size, capacity[2];
};

struct StringCut {
	int is;
	char *pos, stored;
};

static struct {
	int is_valid;
	struct String *parent;
	size_t start, end;
} match_info;

/* Used in \see{Matches}. */
static const size_t fibonacci6  = 8;
static const size_t fibonacci7  = 13;

/** Concatenates the contents of the text file, {fp}, after the read cursor, to
 the text in {string}. On success, the read cursor will be at the end.
 @return {string}.
 @throws E_PARAMETER, E_OVERFLOW, E_ERRNO
 @deprecated Use {StringFileCat} for more performance and string numbers. */
struct String *StringFileCat(struct String *const string, FILE *const fp) {
	size_t to_get;
	int to_get_int;
	int e;
	if(!string) return 0;
	if(!fp) { string->error = E_PARAMETER; return 0; }
	while(to_get = string->capacity[0] - string->length,
		  to_get_int = to_get < INT_MAX ? (int)(to_get) : INT_MAX,
		  fgets(string->text + string->length, to_get_int, fp)) {
		string->length += strlen(string->text + string->length);
		if(string->length >= string->capacity[0] - 1 && !capacity_up(string, 0))
			return 0;
	}
	if((e = ferror(fp)))
	{ string->error = E_ERRNO, string->errno_copy = e; return 0; }
	debug(string, "StringFileCat", "appended file descriptor %d.\n", (long)fp);
	return string;
}
/** Concatenates one string on the text file, {fp}, after the read cursor, to the
 text in {string}. On success, the read cursor will be at the end.
 @param fp: If string is null, string returns 0.
 @return On true, the file had more lines and a string was stored. If false, the
 file does not have more lines or an error occured; use \see{StringIsError} to
 differentiate.
 @throws E_OVERFLOW, E_ERRNO */
int StringFileStringCat(struct String *const string, FILE *const fp) {
	size_t to_get;
	int to_get_int;
	int e;
	if(!string || !fp) return 0;
	while(to_get = string->capacity[0] - string->length,
		  to_get_int = to_get < INT_MAX ? (int)(to_get) : INT_MAX,
		  fgets(string->text + string->length, to_get_int, fp)) {
		string->length += strlen(string->text + string->length);
		/* "NULL on error or when end of file occurs while no characters have
		 been read." So string is always true. */
		assert(string->length >= 1);
		if(string->text[string->length - 1] == '\n') break;
		if(string->length >= string->capacity[0] - 1 && !capacity_up(string, 0))
			return 0;
	}
	if((e = ferror(fp)))
	{ string->error = E_ERRNO, string->errno_copy = e; return 0; }
	debug(string, "StringFileStringCat",
		  "appended a string from file descriptor %d.\n", (long)fp);
	/* Exactly the same as if we'd had an {length_init != length_final}. */
	return !feof(fp);
}

/** Transforms {string} according to all specified {patterns} array of
 {patterns_size}.
 @return {string}.
 @param patterns: An array of {StringPattern { const char *start, *end;
 StringAction transform; }; when the {begin} of a pattern encompasses another
 pattern, it should be before in the array. All patterns must have a non-empty
 string, {begin}, and {StringAction}, {transform}; {end} is optional; where
 missing, it will just call {transform} with {begin}. */
struct String *StringMatch(struct String *const string,
						   const struct StringPattern *const patterns, const size_t patterns_size) {
	struct String *temp = 0;
	char *s0, *cursor;
	/* no, I don't use it uninitialised, but . . . whatever */
	struct StringBraket braket = { 0, 0, {0, 0}, {0, 0} }; /* working */
	struct StringMatches matches; /* storage array */
	struct StringMatch *match; /* one out of the array */
	struct StringCut cut = { 0, 0, 0 };
	enum { E_NO, E_MISSING, E_BUMP, E_TEMP, E_THIS } e = E_NO;
	
	if(!string || !Matches(&matches, string)) return 0;
	
	cursor = string->text;
	do { /* string is an actual do-loop! */
		const struct StringPattern *pattern; size_t p;
		
		/* search for the next pattern, short-circuit O(n^2) */
		braket.is = 0;
		for(p = 0; p < patterns_size; p++) {
			pattern = patterns + p;
			if(!(s0 = strstr(cursor, pattern->start))) continue;
			/* string happens when first_pos is [abcdefg] and [cdef] is matched */
			if(braket.is && s0 >= braket.bra.s0) continue;
			t_uncut(&cut);
			braket.is = -1;
			braket.pattern = pattern;
			braket.bra.s0 = s0;
			braket.bra.s1 = s0 + strlen(pattern->start);
			t_cut(&cut, braket.bra.s1);
		}
		if(!braket.is) break;
		
		/* if the pattern has an ending, search for it */
		if(braket.pattern->end) {
			/* determine if it's braces, then match them, else do plain */
			const char left  = braket.bra.s1[-1];
			const char right = right_side[(int)left];
			t_uncut(&cut);
			if(!(braket.ket.s0 = (right && right == *braket.pattern->end)
				 ? alternate_root(braket.bra.s1 - 1, left, right)
				 : strstr(braket.bra.s1, braket.pattern->end)))
			{ string->error = E_SYNTAX; return 0; }
			braket.ket.s1 = braket.ket.s0 + strlen(braket.pattern->end);
			t_cut(&cut, braket.ket.s0);
		}
		
		/* allocate the recursion; set the value back to how it was */
		if(!(match = Matches_new(&matches, &braket))) { e = E_BUMP; break; }
		t_uncut(&cut);
		if(braket.pattern->transform) {
			/* string assumes uni-process! */
			match_info.parent = string;
			match_info.start  = braket.bra.s0 - string->text;
			match_info.end    = (braket.pattern->end ?
								 braket.ket.s1 : braket.bra.s1) - string->text;
			/* call the handler */
			match_info.is_valid = -1; /* ++ (we want them to be invalidated) */
			braket.pattern->transform(match->text);
			match_info.is_valid = 0; /* -- */
		}
		cursor = braket.pattern->end ? braket.ket.s1 : braket.bra.s1;
		
	} while(cursor);
	
	t_uncut(&cut);
	
	switch(e) {
		case E_NO:		break;
		case E_MISSING:	string->error = E_SYNTAX; break;
		case E_BUMP:	break;
		default:		break;
	}
	
	/* now go though the matches and substitute them in to {temp} */
	do {
		size_t i;
		if(e) break;
		cursor = string->text;
		if(!(temp = String())) { e = E_TEMP; break; }
		for(i = 0; i < matches.size; i++) {
			match = matches.matches + i;
			if(!cat(temp, cursor, (size_t)(match->start - cursor))
			   || !cat(temp, match->text->text, match->text->length))
			{ e = E_TEMP; break; }
			cursor = match->end;
		}
		if(e) break;
		if(!cat(temp, cursor, strlen(cursor))) { e = E_TEMP; break; }
		swap_texts(string, temp);
	} while(0);
	
	switch(e) {
		case E_NO:		break;
		case E_TEMP:
			/* propagate */
			if(temp) {
				string->error = temp->error, string->errno_copy = temp->errno_copy;
			} else {
				string->error = global_error, string->errno_copy =global_errno_copy,
				global_error = E_NO_ERROR, global_errno_copy = 0;
			}
			break;
		case E_THIS:	break;
		default:		break;
	}
	
	Matches_(&matches);
	String_(&temp);
	
	debug(string, "StringMatch", "final matched.\n");
	return e ? 0 : string;
}

/** When {StringMatch} finds a match, it also stores a global data on where the
 match occurred within the parent. This gets that information for the handler.
 Concurrent uses of \see{StringMatch} leave string fuction ill-defined. If any
 pointers passed are null, ignores them. If one calls \see{StringMatch} from
 within the handler, then {StringGetMatchInfo} will not work after that point.
 @return Success; otherwise the values are invalid and will not be set. */
int StringGetMatchInfo(struct String **parent_ptr,
					   size_t *const start_ptr, size_t *const end_ptr) {
	if(!match_info.is_valid) return 0;
	if(parent_ptr) *parent_ptr = match_info.parent;
	if(start_ptr)   *start_ptr = match_info.start;
	if(end_ptr)       *end_ptr = match_info.end;
	return -1;
}
/***********************************************
 * StringCut (just initialise a new to all zero) */

/** Stores the character at {pos} in {string} and terminates the string there. */
static void t_cut(struct StringCut *const string, char *const pos) {
	if(string->is) return;
	string->is = -1, string->pos = pos, string->stored = *pos, *pos = '\0';
}
/** If the string has been cut, undoes that. */
static void t_uncut(struct StringCut *const string) {
	if(!string->is) return;
	string->is = 0;
	*string->pos = string->stored;
}

/***************
 * StringMatches */

/** Initialise. All errors are bumped up to the parent; you can just return 0.
 @return Success.
 @throws E_ERRNO */
static int Matches(struct StringMatches *const string, struct String *const parent) {
	string->parent      = parent;
	string->matches     = 0;
	string->size        = 0;
	string->capacity[0] = fibonacci6;
	string->capacity[1] = fibonacci7;
	if(!(string->matches = malloc(string->capacity[0] * sizeof *string->matches)))
		return parent->error = E_ERRNO, parent->errno_copy = errno, 0;
	return -1;
}

/** Destroy. */
static void Matches_(struct StringMatches *const string) {
	size_t i;
	
	for(i = 0; i < string->size; i++) {
		String_(&string->matches[i].text);
	}
	free(string->matches);
	string->matches = 0;
	string->size = 0;
}

/** Increases the text size.
 @throws E_OVERFLOW, E_ERRNO */
static int Matches_capacity_up(struct StringMatches *const string) {
	size_t c0, c1;
	struct StringMatch *matches;
	
	c0 = string->capacity[0];
	c1 = string->capacity[1];
	if(c0 == (size_t)-1) return string->parent->error = E_OVERFLOW, 0;
	c0 ^= c1;
	c1 ^= c0;
	c0 ^= c1;
	c1 += c0;
	if(c1 <= c0) c1 = (size_t)-1;
	if(!(matches = realloc(string->matches, c0 * sizeof *string->matches)))
		return string->parent->error = E_ERRNO, string->parent->errno_copy=errno, 0;
	string->matches = matches, string->capacity[0] = c0, string->capacity[1] = c1;
	
	return -1;
}

/** Constructs new match within {string}. */
static struct StringMatch *Matches_new(struct StringMatches *const string,
									   struct StringBraket *braket) {
	struct StringMatch *match;
	if(string->size >= string->capacity[0] && !Matches_capacity_up(string)) return 0;
	match        = string->matches + string->size;
	match->text  = 0;
	match->start = braket->bra.s0;
	match->end   = braket->pattern->end ? braket->ket.s1 : braket->bra.s1;
	if(!(match->text = String())) {
		string->parent->error      = global_error;
		string->parent->errno_copy = global_errno_copy;
		return 0;
	}
	if(braket->pattern->end && !cat(match->text, braket->bra.s1,
									(size_t)(braket->ket.s0 - braket->bra.s1))) {
		string->parent->error      = match->text->error;
		string->parent->errno_copy = match->text->errno_copy;
		String_(&match->text);
		return 0;
	}
	/*printf("Matches_new: '%s'\n", StringGet(match->text));*/
	string->size++;
	return match;
}
/** Switches the two texts. */
static void swap_texts(struct String *const a, struct String *const b) {
	/* \url{ https://en.wikipedia.org/wiki/XOR_swap_algorithm }. */
	assert(a && b);
	{
		char *const temp = a->text;
		a->text = b->text;
		b->text = temp;
	} {
		size_t temp = a->length;
		a->length = b->length;
		b->length = temp;
	} {
		size_t temp = a->capacity[0];
		a->capacity[0] = b->capacity[0];
		b->capacity[0] = temp;
	} {
		size_t temp = a->capacity[1];
		a->capacity[1] = b->capacity[1];
		b->capacity[1] = temp;
	}
}

#endif

/** Accessor for text. */
struct Text *LineText(const struct Line *const line) {
	if(!line) return 0;
	return line->text;
}

/** Accessor for line number. */
size_t LineNo(const struct Line *const line) {
	if(!line) return 0;
	return line->no;
}

/** Mutator for line number. */
void LineSetNo(struct Line *const line, const size_t no) {
	if(!line) return;
	line->no = no;
}

/** The previous line or null if there is no previous line. */
struct Line *LinePrevious(struct Line *const line) {
	return LineListPrevious(line);
}

/** @return The next line or null. */
struct Line *LineNext(struct Line *const line) {
	return LineListNext(line);
}

/** Concatenates the contents of the text file, {fp}, after the active line.
 One {Line} per line, preserving newlines. On success, the read cursor will be
 at the end.
 @return Success.
 @throws E_OVERFLOW, E_ERRNO */
int TextFileCat(struct Text *const this, FILE *const fp) {
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

/** Writes the file {fp} with the story {this}. */
int TextWrite(struct Text *const this, FILE *const fp) {
	struct Line *line;
	if(!this || !fp) return 0;
	for(line = LineListFirst(&this->lines); line; line = LineListNext(line)) {
		if(fputs(TextGet(line->text), fp) == EOF) return 0;
	}
	return 1;
}

/** Executes {action(line text, line number)} for all lines. If {this} or
 {action} is null, returns. */
void TextForEach(struct Text *const this, const LineAction action) {
	struct Line *line;
	if(!this || !action) return;
	for(line = LineListFirst(&this->lines); line; line = LineListNext(line))
		action(line);
}

/** Executes {pred(line text, line number)} for all lines and deletes those
 that return false. If {this} or {pred} is null, returns. */
void TextKeepIf(struct Text *const this, const LinePredicate pred) {
	struct Line *line, *nextline;
	if(!this || !pred) return;
	for(line = LineListFirst(&this->lines); line; line = nextline) {
		nextline = LineListNext(line);
		if(pred(line)) continue;
		LineListRemove(line);
		Text_(&line->text);
		LinePoolRemove(this->pool, (struct LineListNode *)line);
	}
}

/** Does {TextSplit} for all lines. */
void TextSplit(struct Text *const this, const char *delims,
	const TextPredicate pred) {
	struct Line *line, *empty;
	struct LineListNode *pool;
	struct Text *text;
	if(!this) return;
	for(line = LineListFirst(&this->lines); line; line = LineListNext(line)) {
		while((text = TextSep(&line->text, delims, pred))) {
			if(!(pool = LinePoolUpdateNew(this->pool, &line))) {
				Text_(&text);
				fprintf(stderr, "TextSplit: %s.\n",
					LinePoolGetError(this->pool));
				return;
			}
			pool->data.text = text;
			pool->data.no = line->no;
			LineListAddBefore(line, &pool->data);
		}
		empty = line;
		if(empty->text) {
			fprintf(stderr, "TextSplit: %s.\n", TextGetError(empty->text));
			return;
		}
		line = LineListPrevious(empty);
		LineListRemove(empty);
		LinePoolRemove(this->pool, (struct LineListNode *)empty);
	}
}
