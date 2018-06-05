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
 a.empty().strip("quxf ") = [ "oo bar", "baz" ]
 a.join(0) = [ "foo bar\n\n baz\n qux" ]
 a.join("") = [ "foo bar\n\nbaz\nqux" ]
 a.sort() = [ "baz\n", "foo bar\n\n", "qux" ]
 a.replace("a", "oo") = [ "foo boor\n\n", "booz\n", "qux" ]
 /a.cat("quxx") = [ "foo bar\n\n", "baz\n", "qux", "quxx" ]/not needed
 a.cat("%d", 42) = [ "foo bar\n\n42", "baz\n42", "qux42" ]
 a.format("%d foo", 42) = [ "foo bar\n\n", "baz\n", "qux", "42 foo" ]
 TextMap b = [ "o"->"a", "a"->"o" ]
 a.substitute(b) = [ "faa bor\n\n", "boz\n", "qux" ]
 a.reverse() = [ "qux", "baz\n", "foo bar\n\n" ]
 }

 chop VARIABLE
 chop( LIST )
 chop
 Chops off the last character of a string and returns the character chopped. It is much more efficient than s/.$//s because it neither scans nor copies the string. Note that chop returns the last character.
 
 chomp VARIABLE
 chomp( LIST )
 chomp
 This safer version of chop removes any trailing string that corresponds to the current value of $/ (also known as $INPUT_RECORD_SEPARATOR in the English module). It returns the total number of characters removed from all its arguments. It's often used to remove the newline from the end of an input record when you're worried that the final record may be missing its newline.
 
 crypt PLAINTEXT,SALT
 Creates a digest string exactly like the crypt(3) function in the C library (assuming that you actually have a version there that has not been extirpated as a potential munition).
 crypt is a one-way hash function. The PLAINTEXT and SALT are turned into a short string, called a digest, which is returned. The same PLAINTEXT and SALT will always return the same string, but there is no (known) way to get the original PLAINTEXT from the hash. Small changes in the PLAINTEXT or SALT will result in large changes in the digest.
 
 length EXPR
 length
 Returns the length in characters of the value of EXPR. If EXPR is omitted, returns the length of $_ . If EXPR is undefined, returns undef.
 
 index STR,SUBSTR,POSITION
 index STR,SUBSTR
 The index function searches for one string within another, but without the wildcard-like behavior of a full regular-expression pattern match. It returns the position of the first occurrence of SUBSTR in STR at or after POSITION. If POSITION is omitted, starts searching from the beginning of the string. POSITION before the beginning of the string or after its end is treated as if it were the beginning or the end, respectively. POSITION and the return value are based at zero. If the substring is not found, index returns -1. 

 rindex STR,SUBSTR,POSITION
 rindex STR,SUBSTR
 Works just like index except that it returns the position of the last occurrence of SUBSTR in STR. If POSITION is specified, returns the last occurrence beginning at or before that position.
 
 reverse LIST
 In list context, returns a list value consisting of the elements of LIST in the opposite order. In scalar context, concatenates the elements of LIST and returns a string value with all characters in the opposite order.
 
 sprintf FORMAT, LIST
 Returns a string formatted by the usual printf conventions of the C library function sprintf.
 
 substr EXPR,OFFSET,LENGTH,REPLACEMENT
 substr EXPR,OFFSET,LENGTH
 substr EXPR,OFFSET
 Extracts a substring out of EXPR and returns it. First character is at offset zero. If OFFSET is negative, starts that far back from the end of the string. If LENGTH is omitted, returns everything through the end of the string. If LENGTH is negative, leaves that many characters off the end of the string.
 my $s = "The black cat climbed the green tree";
 my $color  = substr $s, 4, 5;      # black
 my $middle = substr $s, 4, -11;    # black cat climbed the
 my $end    = substr $s, 14;        # climbed the green tree
 my $tail   = substr $s, -4;        # tree
 my $z      = substr $s, -4, 2;     # tr
 (splice?)
 
 uc EXPR
 uc
 Returns an uppercased version of EXPR. This is the internal function implementing the \U escape in double-quoted strings. It does not attempt to do titlecase mapping on initial letters. See ucfirst for that.
 
 ucfirst EXPR
 ucfirst
 Returns the value of EXPR with the first character in uppercase (titlecase in Unicode). This is the internal function implementing the \u escape in double-quoted strings.
 
 
 split /PATTERN/,EXPR,LIMIT
 split /PATTERN/,EXPR
 split /PATTERN/
 split
 Splits the string EXPR into a list of strings and returns the list in list context, or the size of the list in scalar context.
 split(/-|,/, "1-10,20", 3)
 # ('1', '10', '20')
 split(/(-|,)/, "1-10,20", 3)
 # ('1', '-', '10', ',', '20')
 split(/-|(,)/, "1-10,20", 3)
 # ('1', undef, '10', ',', '20')
 split(/(-)|,/, "1-10,20", 3)
 # ('1', '-', '10', undef, '20')
 split(/(-)|(,)/, "1-10,20", 3)
 # ('1', '-', undef, '10', undef, ',', '20')
 
 void split(Text a, Regex r, void (*s)(Text const a, Text const b, size_t index))
 Splits {a} with {r} into a new, temporary {b}, sets the {a} cursor to the spot where the split was. {b} is always the unmatched text that comes before the match (possibly empty,) and the posibly the match itself or it has gotten to the end. {s} recieves {a} and {b} and the {index} of the match. To do a conventional split, {void s(Text a, Text b) { a.add(b.remove()); }}, or to do a capture of the text as well as the token, {void s(Text a, Text b) { a.add(b.remove()), a.add(b.remove()); }}.
 
 void split(Text a, Regex r)
 Splits {a} with {r}. Any captures go in there, too.

 int match(Text a, Regex r);
 Sets the next match or returns false.
 
 s/PATTERN/REPLACEMENT/msixpodualngcer
 Searches a string for a pattern, and if found, replaces that pattern with the replacement text and returns the number of substitutions made. Otherwise it returns false (specifically, the empty string).
 
 @param TEXT_TEST
 

 @title		Text
 @author	Neil
 @std		C89/90
 @version	2018-01
 @since		2018-01 */

#include <stddef.h>	/* offset_of */
#include <stdlib.h> /* malloc free */
#include <stdio.h>  /* fprintf fopen */
#include <string.h>	/* strerror strchr */
#include <limits.h>	/* INT_MAX */
#include <errno.h>	/* errno */
#ifdef STORY_DEBUG /* <-- debug */
#include <stdarg.h>	/* va_* */
#endif /* debug --> */

#include "String.h"
#include "Text.h"

struct LineVt;

/** Abstract {Line}. */
struct Line {
	const struct LineVt *vt;
	struct String string;
};

/** @implements <Line>ToString */
static void line_to_string(const struct Line *const line, char (*const a)[12]) {
	sprintf(*a, "%.11s", StringGet(&line->string));
}

#define LIST_NAME Line
#define LIST_TYPE struct Line
#define LIST_TO_STRING &line_to_string
#include "List.h"

/** Abstract constructor. */
static void Line(struct Line *const line, const struct LineVt *const vt) {
	assert(line && vt);
	line->vt = vt;
	String(&line->string);
}

/** Abstract destructor. */
static void Line_(struct Line *const line) {
	assert(line);
	String_(&line->string);
}



/** {Plain} extends {Line}. Plain has no extra data; is _is_ a {LineLink}, but
 renamed to avoid confusion. */
struct Plain { struct LineLink line; };

/* {container_of}; unused because it contains no data. */
/*static const struct Plain *plain_const_downcast(const struct Line *const line) {
	return (const struct Plain *)(const void *)((const char *)line
		- offsetof(struct Plain, line)
		- offsetof(struct LineLink, data));
}*/
/** {container_of}. */
static struct Plain *plain_downcast(struct Line *const line) {
	return (struct Plain *)(void *)((char *)line
		- offsetof(struct Plain, line)
		- offsetof(struct LineLink, data));
}

/** @implements <Plain>Migrate */
static void plain_migrate(struct Plain *const plain,
	const struct Migrate *const migrate) {
	LineLinkMigrate(&plain->line.data, migrate);
}

/** @implements <Plain>LinePrint */
static void plain_source(const struct Line *const line,
	char *const a, const int a_len) {
	assert(line && a && a_len > 0);
	sprintf(a, "%.*s", a_len, "new");
}

/* We have to have vt defined to define this. */
static struct Line *plain_copy(struct Text *const text,
	const struct Line *const line);

#define POOL_NAME Plain
#define POOL_TYPE struct Plain
#define POOL_MIGRATE_EACH &plain_migrate
#define POOL_MIGRATE_UPDATE struct Line
#include "Pool.h"



/** {File} extends {Line}. */
struct File {
	struct LineLink line;
	const char *fn;
	size_t line_no;
};

/** {container_of}. */
static const struct File *file_const_downcast(const struct Line *const line) {
	return (const struct File *)(const void *)((const char *)line
		- offsetof(struct File, line)
		- offsetof(struct LineLink, data));
}

/** {container_of}. */
static struct File *file_downcast(struct Line *const line) {
	return (struct File *)(void *)((char *)line
		- offsetof(struct File, line)
		- offsetof(struct LineLink, data));
}

/** @implements <File>Migrate */
static void file_migrate(struct File *const file,
	const struct Migrate *const migrate) {
	LineLinkMigrate(&file->line.data, migrate);
}

/** @param a_len: Must be > 2.
 @implements <File>LinePrint
 @fixme Test. */
static void file_source(const struct Line *const line,
	char *const a, const int a_len) {
	const struct File *const file = file_const_downcast(line);
	/* Don't think there is a way to limit the size that confoms to
	 internationalisation on C89, but this should be safe. Wish {snprintf} was
	 part of the standard. */
	char num[256];
	const int half_a_len = a_len >> 1;
	assert(line && a && a_len > 2);
	sprintf(num, "%lu", (unsigned long)file->line_no);
	sprintf(a, "%.*s:%.*s",
		half_a_len, file->fn, a_len - half_a_len - 2, num);
	/*fprintf(stderr, "file_source: %d half %d other %d.\n", a_len, half_a_len, a_len - half_a_len - 1);*/
}

/* We have to have vt defined to define this. */
static struct Line *file_copy(struct Text *const text,
	const struct Line *const line);

#define POOL_NAME File
#define POOL_TYPE struct File
#define POOL_MIGRATE_EACH &file_migrate
#define POOL_MIGRATE_UPDATE struct Line
#include "Pool.h"



/** @implements <String>ToString */
static void string_to_string(const struct String *const s, char (*const a)[12]){
	assert(s);
	sprintf(*a, "%.11s", StringGet(s));
}
/* Note: we don't need a migrate function if it's members are const {String},
 otherwise we would. Used for filenames. */
#define POOL_NAME String
#define POOL_TYPE struct String
#define POOL_TO_STRING &string_to_string
#include "Pool.h"
static void string_(struct StringPool *const pool, struct String *const s) {
	assert(pool);
	if(!s) return;
	String_(s);
	StringPoolRemove(pool, s);
}
static struct String *string(struct StringPool *const pool,
	const char *const input) {
	struct String *s = 0;
	assert(pool && input);
	if(!(s = StringPoolNew(pool)) || !StringCat(s, input))
		{ string_(pool, s); return 0; }
	return s;
}



/** Agglomeration {Text}. */
struct Text {
	/* List of lines. */
	struct LineList lines;
	/* Stores for list. */
	struct PlainPool plains;
	struct FilePool files;
	struct StringPool filenames;
	/* Editing functions. */
	struct Line *cursor;
};



/** Private; pushes before the cursor, or if the cursor is unset, pushes to the
 end. {line} must not be in {text.lines}. Called from constructors. */
static void push_above_cursor(struct Text *const text, struct Line *const line){
	assert(text && line);
	if(text->cursor) {
		LineListAddBefore(text->cursor, line);
	} else {
		LineListPush(&text->lines, line);
	}
}

/** Private destructor.
 @implements TextLineAction */
static void Plain_(struct Text *const text, struct Line *const line) {
	struct Plain *const plain = plain_downcast(line);
	assert(text);
	if(!line) return;
	LineListRemove(line);
	if(!PlainPoolRemove(&text->plains, plain)) assert(0);
	Line_(line);
}
/** Private destructor.
 @implements TextLineAction */
static void File_(struct Text *const text, struct Line *const line) {
	struct File *const file = file_downcast(line);
	assert(text);
	if(!line) return;
	LineListRemove(line);
	if(!FilePoolRemove(&text->files, file)) assert(0);
	Line_(line);
}



/* Virtual table definition. */
static const struct LineVt {
	const LinePrint source;
	const TextLineOperator copy;
	const TextLineAction delete;
} plain_vt = { &plain_source, &plain_copy, &Plain_ },
	file_vt = { &file_source, &file_copy, &File_ };



/** Private empty plain constructor. */
static struct Plain *Plain(struct Text *const text) {
	struct Plain *const plain = PlainPoolUpdateNew(&text->plains,&text->cursor);
	assert(text);
	if(!plain) return 0;
	Line(&plain->line.data, &plain_vt);
	push_above_cursor(text, &plain->line.data);
	return plain;
}

/** Private empty file constructor. */
static struct File *File(struct Text *const text,
	const char *const fn, const size_t line_no) {
	struct File *const file = FilePoolUpdateNew(&text->files, &text->cursor);
	assert(text && fn);
	if(!file) return 0;
	Line(&file->line.data, &file_vt);
	file->fn = fn;
	file->line_no = line_no;
	push_above_cursor(text, &file->line.data);
	return file;
}

/* Define these from above. */

/** @param line: Unused.
 @implements TextLineOperator */
static struct Line *plain_copy(struct Text *const text,
	const struct Line *const line) {
	struct Plain *const plain = Plain(text);
	(void)line;
	assert(text);
	return plain ? &plain->line.data : 0;
}
/** @implements TextLineOperator */
static struct Line *file_copy(struct Text *const text,
	const struct Line *const line) {
	const struct File *const old_file = file_const_downcast(line);
	struct File *const new_file = File(text, old_file->fn, old_file->line_no);
	assert(text && line);
	return new_file ? &new_file->line.data : 0;
}





/* Initialisation. */



/** Destructor.
 @param ptext: A reference to the object that is to be deleted. If null or
 points to null, doesn't do anything. */
void Text_(struct Text **const ptext) {
	struct Text *text;
	struct String *str;
	if(!ptext || !(text = *ptext)) return;
	/*fprintf(stderr, "~Text: erase, #%p.\n", (void *)text); Debug. */
	LineListClear(&text->lines);
	PlainPool_(&text->plains);
	FilePool_(&text->files);
	/*printf("~: %s\n", StringPoolToString(&text->filenames)); Debug. */
	while((str = StringPoolPop(&text->filenames))) String_(str);
	StringPool_(&text->filenames);
	free(text), text = *ptext = 0;
}

/** Constructor.
 @return An object or a null pointer, if the object couldn't be created.
 @throws {malloc} */
struct Text *Text(void) {
	struct Text *text;
	if(!(text = malloc(sizeof *text))) return 0;
	LineListClear(&text->lines);
	PlainPool(&text->plains);
	FilePool(&text->files);
	StringPool(&text->filenames);
	text->cursor = 0;
	/*fprintf(stderr, "Text: new, #%p.\n", (void *)text); Debug. */
	return text;
}


/* Cursor movement. */


/** Resets the cursor.
 @param text: If null, does nothing. */
void TextReset(struct Text *const text) {
	if(text) text->cursor = 0;
}

/** Advances the cursor. If the cursor is reset, sets the cursor to the first
 line.
 @param text: If null, returns false.
 @return The string contents or null if there is no next position (the cursor
 will be reset.) */
const struct Line *TextNext(struct Text *const text) {
	if(!text) return 0;
	if(!text->cursor) {
		text->cursor = LineListFirst(&text->lines);
	} else {
		text->cursor = LineListNext(text->cursor);
	}
	return text->cursor;
}

/** Makes a copy of the line in {text} at the cursor. Places the copy before
 the cursor. Fills the copy with bits {[a, b)}. If the cursor is reset, a plain
 copy is pushed at the end.
 @param text: If null, does nothing.
 @param a, b: If null or {a >= b}, the line will be blank.
 @return Success.
 @throws ... */
struct Line *TextCopyBetween(struct Text *const text,
	const char *const a, const char *const b) {
	struct Line *line;
	if(!text) return 0;
	/* {plain_copy} doesn't use 2nd arg. */
	if(!(line = (text->cursor ? text->cursor->vt->copy : plain_copy)(text,
		text->cursor)) || !StringBetweenCat(&line->string, a, b)) return /*@fixme: memory leak!!!*/0;
	return line;
}

/** Removes the line at the cursor. The cursor goes to the previous line; if
 the cursor is on the first line, the cursor is reset. If the cursor is reset,
 does nothing.
 @param text: If null, does nothing. */
void TextRemove(struct Text *const text) {
	struct Line *del;
	if(!text || !(del = text->cursor)) return;
	text->cursor = LineListPrevious(del);
	del->vt->delete(text, del);
}






/** Concatenates a blank before the {text} cursor, or if the cursor is reset,
 at the end.
 @param text: If null, returns null.
 @return The line that is created ot null.
 @throws {malloc} errors. */
struct Line *TextNew(struct Text *const text) {
	struct Plain *plain = 0;
	if(!text || !(plain = Plain(text))) return 0;
	/* Initialise to empty. (Not needed -- \see{LineGet} has extra things.) */
	/*if(!StringClear(&plain->data.string)) { Plain_(text, plain); return 0; }*/
	return &plain->line.data;
}

/** Concatenates the contents of the stream {fp}, after the {text} line cursor,
 labelled with {fn}. On success, the read cursor will be at the end of the
 file. The newlines are not preserved, rather it is seen as a command to end
 the line. @fixme

 On error, the contents may be an an intermediate state.

 @fixme Respect cursor.
 @return Success.
 @throws ERANGE: the file is too large to fit in a {size_t}.
 @throws {realloc} errors.
 @throws {fgets} errors. */
int TextFile(struct Text *const text, FILE *const fp, const char *const fn) {
	struct File *file = 0; /* One line in the file. */
	struct String *str_fn;
	char input[256];
	size_t input_len;
	size_t line_no = 0;
	int is_eol = 0;
	if(!text || !fp) return 0;
	/* Store a copy of this filename. */
	if(!(str_fn = string(&text->filenames, fn))) return 0;
	/* Append text file to {text}. */
	while(fgets(input, sizeof input, fp)) {
		/* Creates a new blank file line. {StringGet} would normally be Bad,
		 but it's essentially constant: don't change. */
		if(!file && !(file = File(text, StringGet(str_fn), ++line_no))) break;
		/* This is weird but not impossible; eg, zero in file. */
		if(!(input_len = strlen(input))) continue;
		assert(input_len < sizeof input);
		if(input[input_len - 1] == '\n') input[--input_len] = '\0', is_eol = 1;
		if(!StringBetweenCat(&file->line.data.string, input, input + input_len))
			break;
		if(is_eol) file = 0, is_eol = 0;
	}
	/* We can't delete {str_fn} because intermediate values may use it. */
	return feof(fp);
}

/** Outputs {text} to {fp}, each line according to {fmt}.
 @param text, fp, fmt: If null, returns false.
 @param fmt: Should be less then {INT_MAX} bytes. Accepts: \${
 %% as '%',
 %s as line,
 %a as source (max 255 bytes.) }
 @return Success.
 @throws {fwrite}, {fputs}, {fputc} errors. */
int TextPrint(struct Text *const text, FILE *const fp, const char *const fmt) {
	const struct Line *line;
	const char *f0, *f1;
	size_t size;
	char a[256];
	if(!text || !fp || !fmt) return 0;
	for(line = LineListFirst(&text->lines); line; line = LineListNext(line)) {
		for(f0 = f1 = fmt; ; f1++) {
			if(*f1 && *f1 != '%') continue;
			if(f0 < f1 && (size = (size_t)(f1 - f0),
				fwrite(f0, 1, size, fp)) != size) return 0;
			if(!*f1) break;
			switch(*++f1) {
			case 's':
				if(size = StringLength(&line->string),
					fwrite(StringGet(&line->string), 1, size, fp) != size)
					return 0; break;
			case 'a':
				if(line->vt->source(line, a, sizeof a), fputs(a, fp) == EOF)
					return 0; break;
			case '%':
				if(fputc('%', fp) == EOF) return 0; break;
			}
			f0 = f1 = f1 + 1;
		}
	}
	return 1;
}

/** Executes {action(line)} for all lines if {text} and {action} are
 non-null. */
void TextForEach(struct Text *const text, const LineAction action) {
	struct Line *line;
	if(!text || !action) return;
	for(line = LineListFirst(&text->lines); line; line = LineListNext(line))
		action(line);
}

/** @param line: If null, returns null.
 @return The text stored in line. */
const char *LineGet(const struct Line *const line) {
	const char *s;
	if(!line) return 0;
	return (s = StringGet(&line->string)) ? s : ""; /* Not null ever. */
}

/** @param line: If null, returns 0.
 @return The length of the line.
 @order O(1) */
size_t LineLength(const struct Line *const line) {
	if(!line) return 0;
	return StringLength(&line->string);
}

/** Gets the source of the {line} in a null-terminated string, {a}, not
 exceeding {a_size}. If {a_size} is zero, does nothing. */
void LineSource(const struct Line *const line,
	char *const a, size_t a_size) {
	int a_len;
	if(!a_size) return;
	a_len = (a_size > INT_MAX) ? INT_MAX : (int)a_size - 1;
	if(!line) { sprintf(a, "%.*s", a_len, "null"); return; }
	line->vt->source(line, a, a_len);
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
