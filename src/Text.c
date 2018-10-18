/** 2018 Neil Edelman, distributed under the terms of the MIT License;
 see readme.txt, or \url{ https://opensource.org/licenses/MIT }.

 A {Text} is composed of {String} in a way that makes them easy and fast to
 edit. Requires {List}, {Pool}, and {String}. Each {Text} is made up of lines,
 but one can stick whatever you want in here, including new-lines or not.

 If one needs traditional, Python-like editing, call \see{TextNext} to get a
 read-only line, and maybe parse into a new {Text} using one's favourite
 regular expressions tool, (suggest \url{ http://re2c.org/ }.)

 @title		Text
 @author	Neil
 @std		C89/90
 @version	2018-10 Excess {split}s removed; {re2c} is better.
 @since		2018-01 */

#include <stddef.h>	/* offset_of */
#include <stdlib.h> /* malloc free */
#include <stdio.h>  /* fprintf fopen */
#include <string.h>	/* strerror strchr */
#include <limits.h>	/* INT_MAX */
#include <errno.h>	/* errno */
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



/** {Plain} extends {Line}. Plain has no extra data; _is_ a {LineLink}, but
 renamed to avoid confusion. */
struct Plain { struct LineLink line; };

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
static struct Line *plain_copy(const struct Line *const src,
	struct Text *const dst);

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

/** @implements LineTextOperator */
static struct Line *file_copy(const struct Line *const src,
	struct Text *const dst);

#define POOL_NAME File
#define POOL_TYPE struct File
#define POOL_MIGRATE_EACH &file_migrate
#define POOL_MIGRATE_UPDATE struct Line
#include "Pool.h"



/** Agglomeration {Text}. */
struct Text {
	/* List of lines. */
	struct LineList lines;
	/* Stores for list. */
	struct PlainPool plains;
	struct FilePool files;
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
	const LineTextOperator copy;
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
 @implements LineTextOperator */
static struct Line *plain_copy(const struct Line *const src,
	struct Text *const dst) {
	struct Plain *const plain = Plain(dst);
	(void)src;
	assert(dst);
	return plain ? &plain->line.data : 0;
}
/** @implements LineTextOperator */
static struct Line *file_copy(const struct Line *const src,
	struct Text *const dst) {
	const struct File *const old_file = file_const_downcast(src);
	struct File *const new_file = File(dst, old_file->fn, old_file->line_no);
	assert(dst && src);
	return new_file ? &new_file->line.data : 0;
}





/* Initialisation. */



/** Destructor.
 @param ptext: A reference to the object that is to be deleted. If null or
 points to null, doesn't do anything. */
void Text_(struct Text **const ptext) {
	struct Text *text;
	if(!ptext || !(text = *ptext)) return;
	/*fprintf(stderr, "~Text: erase, #%p.\n", (void *)text); Debug. */
	LineListClear(&text->lines);
	PlainPool_(&text->plains);
	FilePool_(&text->files);
	/*printf("~: %s\n", StringPoolToString(&text->filenames)); Debug. */
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
	text->cursor = 0;
	/*fprintf(stderr, "Text: new, #%p.\n", (void *)text); Debug. */
	return text;
}


/* Cursor movement. */


/**
 @param set: A line in the {text} or null to reset it. Undefined behaviour is
 evoked if the line is not in {text}.
 @fixme */
int TextCursorSet(struct Text *const text, struct Line *const set) {
	if(!text) return 0;
	text->cursor = set;
	return 1;
}

/** @param text: If null, returns null.
 @return The line at the cursor or null if the cursor is reset. */
const struct Line *TextCursor(const struct Text *const text) {
	if(!text || !text->cursor) return 0;
	return text->cursor;
}

/** @fixme */
const struct String *LineString(const struct Line *const line) {
	if(!line) return 0;
	return &line->string;
}

/** Advances the cursor. If the cursor is reset, sets the cursor to the first
 line.
 @param text: If null, returns false.
 @return The string contents or null if there is no next position (the cursor
 will be reset.)
 @fixme const? */
struct Line *TextNext(struct Text *const text) {
	if(!text) return 0;
	return text->cursor = text->cursor ?
		LineListNext(text->cursor) : LineListFirst(&text->lines);
}

/** @fixme const? */
struct Line *TextPrevious(struct Text *const text) {
	if(!text) return 0;
	return text->cursor = text->cursor ?
		LineListPrevious(text->cursor) : LineListLast(&text->lines);
}

/** Makes a copy of the meta-data, but not the actual text, in {src}. Places
 the copy before the {dst} cursor; if the cursor of {dst} is reset, places it
 at the end. {dst} can be the same list or different {Text}, but copies any
 depedencies on filenames as well.
 @param dst, src: If either is null, does nothing.
 @return The copy.
 @throws {realloc} errors. */
struct Line *LineCopyMeta(const struct Line *const src, struct Text *const dst){
	if(!dst || !src) return 0;
	return src->vt->copy(src, dst);
}

/** Fills {line} with bits {[a, b)}.
 @param line: If null, does nothing.
 @param a, b: If either null or {a >= b}, does nothing.
 @return Success.
 @throws ... */
int LineBetweenCat(struct Line *const line,
	const char *const a, const char *const b) {
	return !(!line || !StringBetweenCat(&line->string, a, b));
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

/** @fixme */
int TextHasContent(const struct Text *const text) {
	return text && !!LineListFirst(&text->lines);
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
 the line.

 On error, the contents may be an an intermediate state.

 @param fp: Must be valid for the duration of this object or any other objects
 copied off this object.
 @fixme Respect cursor.
 @return Success.
 @throws ERANGE: the file is too large to fit in a {size_t}.
 @throws {realloc} errors.
 @throws {fgets} errors. */
int TextFile(struct Text *const text, FILE *const fp, const char *const fn) {
	struct File *file = 0; /* One line in the file. */
	char input[256];
	size_t input_len;
	size_t line_no = 0;
	int is_eol = 0;
	if(!text || !fp) return 0;
	/* Append text file to {text}. */
	while(fgets(input, sizeof input, fp)) {
		/* Creates a new blank file line. {StringGet} would normally be Bad,
		 but it's essentially constant: don't change. */
		if(!file && !(file = File(text, fn, ++line_no))) break;
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

/** Short-circiut evaluates {text} with each line's {predicate}.
 @param list, predicate: If null, returns null.
 @return The first {Line} in the linked-list that causes the {predicate} with
 {Line} as argument to return false, or null if the {predicate} is true for
 every case.
 @order ~ O({text.lines}) \times O({predicate}) */
struct Line *TextAll(const struct Text *const text,
	const LinePredicate predicate) {
	struct Line *line;
	if(!text || !predicate) return 0;
	for(line = LineListFirst(&text->lines); line; line = LineListNext(line))
		if(!predicate(line)) return line;
	return 0;
}

/** @fixme
 @order O({lines}) */
size_t TextLineCount(const struct Text *const text) {
	struct Line *line;
	size_t count = 0;
	if(!text) return 0;
	for(line = LineListFirst(&text->lines); line; line = LineListNext(line))
		count++;
	return count;
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
