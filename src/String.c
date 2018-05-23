/** 2017 Neil Edelman, distributed under the terms of the MIT License;
 see readme.txt, or \url{ https://opensource.org/licenses/MIT }.

 A dynamic string, intended to be used with modified UTF-8 encoding,
 \url{ https://en.wikipedia.org/wiki/UTF-8#Modified_UTF-8 }. That is, this is a
 wrapper that automatically expands memory as needed around a standard {C}
 null-terminated string in a monolithic array and is compatible with {ASCII}.
 If you need to edit a potentially large string, just one of {String} will be
 generally linear-time and is unsuited alone for such a purpose.

 {struct String} is meant to used directly; it is in the header. It's initial
 state is zero where it will be inactive, for example
 {struct String s = { 0 };} or static data. Any calls to string functions
 generally make it active and one should destruct the string by \see{String_}.
 The only exception is \see{String}, which also initialises the string to be
 empty. All functions accept null pointers as a valid state, which means one
 can compose functions safely without
 \url{ https://en.wikipedia.org/wiki/Pyramid_of_doom_(programming) }.

 This is a very small file with not a lot of editing features, but one can use
 \see{StringGet} and to build up a new {String} with \see{StringCat} using
 one's favourite regular expressions tool.

 @param STRING_STRICT_ANSI
 Does not define \see{StringPrintCat} because it uses {vsnprintf} which was
 standardised in {C99}. For example, this may be an issue with older {MSVC}
 compilers.

 @title		String
 @author	Neil
 @std		C89/90 with C99 vsnprintf
 @version	2018-03 {Text -> String}; complete refactoring to work with {Text}.
 @since		2018-01
			2017-03
 @fixme {StringByteOffsetCodePoints()}?
 @fixme Test {STRING_STRICT_ANSI}. */

#include <stdlib.h> /* malloc realloc free */
#include <string.h> /* strlen memmove memcpy memchr */
#include <errno.h>  /* errno */
#include <assert.h> /* assert */
#include <ctype.h>  /* isspace */
#ifndef STRING_STRICT_ANSI /* <-- !STRING_STRICT_ANSI */
#include <stdarg.h> /* va_* */
#endif /* !STRING_STRICT_ANSI --> */
#include "String.h"

#ifndef STRING_STRICT_ANSI /* <-- !STRING_STRICT_ANSI */

/* This function was standardised in C99. */
int vsnprintf(char *s, size_t n, const char *format, va_list ap);

#endif /* !STRING_STRICT_ANSI --> */

/** {strdup} is a {POSIX.1-2017} extension. This is better. Used in
 \see{StringTransform}.
 @throws {malloc} errors: {IEEE Std 1003.1-2001}. */
static void *memdup(const void *src, const size_t n) {
	void *copy;
	assert(src);
	if(!(copy = malloc(n))) return 0;
	memcpy(copy, src, n);
	return copy;
}

/* Used in \see{text_length}. */
static const size_t fibonacci11 = 89;
static const size_t fibonacci12 = 144;

/** Initialises {string} to inactive. */
static void inactive(struct String *const string) {
	assert(string);
	string->text        = 0;
	string->length      = 0;
	string->capacity[0] = 0;
	string->capacity[1] = 0;
}

/** Clears the text of an active {string}. */
static void clear(struct String *const string) {
	assert(string && string->text);
	string->length  = 0;
	string->text[0] = '\0';
}

/** Ensures {text} capacity.
 @param len: The length to which one wants it to grow.
 @return Success; otherwise, {errno} may be set.
 @throws ERANGE: Tried allocating more then can fit in {size_t}.
 @throws {realloc} errors: {IEEE Std 1003.1-2001}. */
static int text_length(struct String *const string, const size_t len) {
	const size_t size = len + 1;
	size_t c0, c1;
	char *text;
	assert(string);
	if(size <= len) return errno = ERANGE, 0;
	/* It's already that size. */
	if(string->capacity[0] >= size) return 1;
	if(!string->text) c0 = fibonacci11, c1 = fibonacci12;
	else c0 = string->capacity[0], c1 = string->capacity[1];
	while(c0 < size) {
		assert(c0 < c1);
		c0 ^= c1, c1 ^= c0, c0 ^= c1, c1 += c0;
		if(c1 <= c0) c1 = (size_t)-1;
	}
	/* If this fails, we are good with whatever {realloc} sets {errno},
	 including not setting it at all, by {C89} standards. Probably {ENOMEM}. */
	if(!(text = realloc(string->text, c0 * sizeof *string->text))) return 0;
	string->text = text, string->capacity[0] = c0, string->capacity[1] = c1;
	return 1;
}

/** @return Success.
 @throws ERANGE: Tried allocating more then can fit in {size_t}.
 @throws {realloc} errors: {IEEE Std 1003.1-2001}. */
static int cat(struct String *const string, const char *const str,
	const size_t str_len) {
	const size_t old_len = string->length, new_len = old_len + str_len;
	assert(string && str);
	if(!str_len) return 1;
	if(new_len <= old_len) { errno = ERANGE; return 0; }
	if(!text_length(string, new_len)) return 0;
	memcpy(string->text + old_len, str, str_len);
	string->text[new_len] = '\0';
	string->length = new_len;
	return 1;
}

/** Use this if {string} is already in an initialised state. The {String} text
 will be set to null and any memory will be freed.
 @param string: If null, does nothing.
 @order O(1) */
void String_(struct String *const string) {
	if(!string) return;
	free(string->text);
	inactive(string);
}

/** Use this if {string} is uninitialised. Sets the {String} text to be null,
 thus in a well-defined state. Static {String} variables do not need
 initialisation, though it will not hurt. Calling this on an active {string}
 results in a memory leak.
 @param string: A string whose text will be set to null. If null, does nothing.
 @order O(1) */
void String(struct String *const string) {
	if(!string) return;
	inactive(string);
}

/** Erases the text of {string} so the text is empty. If the text of {string}
 is null, initialises an empty string.
 @param string: If null, returns null.
 @return {string}.
 @throws {realloc} errors: {IEEE Std 1003.1-2001}.
 @order O(1) */
struct String *StringClear(struct String *const string) {
	if(!string) return 0;
	if(!string->text) {
		if(!text_length(string, 0)) return 0;
		assert(string->text);
	}
	clear(string);
	return string;
}

/** Volatile, in the sense that it exposes the text; specifically, not
 guaranteed to last between {String} calls to the same object. If you want a
 copy, do {strdup(StringGet(string))}.
 @return The text associated to {string} or null if there is no text or if
 {string} is null.
 @order O(1) */
const char *StringGet(const struct String *const string) {
	if(!string) return 0;
	return string->text;
}

/** @param string: If null, returns zero.
 @return The length in bytes.
 @order O(1) */
size_t StringLength(const struct String *const string) {
	if(!string) return 0;
	return string->length;
}

/** @param string: If null, returns zero.
 @return How many code-points in
 \url{ https://en.wikipedia.org/wiki/UTF-8#Modified_UTF-8 }. If it is not a
 valid string in {UTF-8}, string will return an undefined value between
 {[0, size]}.
 @order O({string.size})
 @fixme Untested.
 @fixme Work with {int} instead of {char} to speed this up? */
size_t StringCodePoints(const struct String *const string) {
	char *text, ch;
	const char *end_null;
	size_t length;
	if(!string || !(text = string->text)) return 0;
	length = string->length;
	end_null = text + length;
	assert(*end_null == '\0');
	while(text < end_null) {
		/* Less and less likely; optimise for the top. */
		if((ch = *text) > 0)    { text++; continue; }
		if((ch & 0xE0) == 0xC0) { text += 2; length -= 1; continue; }
		if((ch & 0xF0) == 0xE0) { text += 3; length -= 2; continue; }
		if((ch & 0xF8) == 0xF0) { text += 4; length -= 3; continue; }
		/* RFC 3629: "treat any ill-formed code unit sequence as an error
		 condition." Skip? */
		text++, length--;
	}
	return length;
}

/** @return True if the text of {string} exists and is not empty. */
int StringHasContent(const struct String *const string) {
	return !(!string || !string->text || *string->text == '\0');
}

/** White-space trims the text associated with {string} using {isspace} only
 at the end.
 @param string: If null, returns null.
 @return {string}.
 @fixme Untested. */
struct String *StringRightTrim(struct String *const string) {
	char *str, *z;
	if(!string || !(str = string->text) || !string->length) return string;
	z = str + string->length - 1;
	while(z >= str && isspace(*z)) z--;
	z++, *z = '\0';
	string->length = (size_t)(z - str);
	return string;
}

/** White-space trims the text associated with {string} using {isspace}.
 @param string: If null, returns null.
 @return {string}. */
struct String *StringTrim(struct String *const string) {
	char *str, *a, *z;
	if(!string || !(str = string->text) || !string->length) return string;
	z = str + string->length - 1, a = str;
	while(z >= str && isspace(*z)) z--;
	z++, *z = '\0';
	while(isspace(*a)) a++;
	string->length = (size_t)(z - a);
	if(a - str) memmove(str, a, string->length + 1);
	return string;
}

/** Replaces the text in {string} with {str}.
 @param string: If null, returns null.
 @param str: If null, returns {string}.
 @return {string}.
 @throws ERANGE: Tried allocating more then can fit in {size_t}.
 @throws {realloc} errors: {IEEE Std 1003.1-2001}. */
struct String *StringCopy(struct String *const string, const char *const str) {
	size_t len;
	if(!string || !str) return string;
	len = strlen(str);
	if(!text_length(string, len)) return 0; /* Pre-allocate. */
	clear(string);
	if(!cat(string, str, len)) return 0; /* Unthinkable. */
	return string;
}

/** Concatenates {str} onto the text in {string}.
 @param string: If null, returns null.
 @param str: If null, returns {string}.
 @return {string}.
 @throws ERANGE: Tried allocating more then can fit in {size_t}.
 @throws {realloc} errors: {IEEE Std 1003.1-2001}. */
struct String *StringCat(struct String *const string, const char *const str) {
	if(!string || !str) return string;
	if(!cat(string, str, strlen(str))) return 0;
	return string;
}

/** Concatenates up to {str_len} bytes characters of {str} onto the text in
 {string}. The responsibility lies with the caller to check for chopped
 code-points.
 @param string: If null, returns null.
 @param str: If null, returns {string}.
 @param str_len: If the bytes one has access to is smaller then this value, the
 results are technically undefined, if using a compiler mode before {C11}.
 \url{ https://stackoverflow.com/questions/47315902/is-it-legal-to-call-memchr-with-a-too-long-length-if-you-know-the-character-wil }
 @return {string}.
 @throws ERANGE: Tried allocating more then can fit in {size_t}.
 @throws {realloc} errors: {IEEE Std 1003.1-2001}. */
struct String *StringNCat(struct String *const string, const char *const str,
	const size_t str_len) {
	const char *end;
	if(!string || !str) return string;
	end = memchr(str, 0, str_len);
	if(!cat(string, str, end ? (size_t)(end - str) : str_len)) return 0;
	return string;
}

/** Concatenates {string} with {[a, b]}.
 @param string: If null, returns null.
 @param a, b: If {a} or {b} are null or {a} > {b}, returns {string}.
 @return {string}.
 @throws ERANGE: Tried allocating more then can fit in {size_t}.
 @throws {realloc} errors: {IEEE Std 1003.1-2001}. */
struct String *StringBetweenCat(struct String *const string,
	const char *const a, const char *const b) {
	if(!string || !a || !b || a > b) return string;
	/* @fixme ?? end = memchr(a, 0, (size_t)(b - a + 1));
	 to make sure it doesn't contain nulls? do we want that? */
	if(!cat(string, a, (size_t)(b - a + 1))) return 0;
	return string;
}

#ifndef STRING_STRICT_ANSI /* <-- !STRING_STRICT_ANSI */

/** Concatenates the text with an {fprintf};
 \url{http://pubs.opengroup.org/onlinepubs/007908799/xsh/fprintf.html}.
 If {STRING_STRICT_ANSI} is defined, this function is not defined.
 @param string: If null, returns null.
 @param fmt: If null, returns {string}.
 @return {string}.
 @throws ERANGE: Tried allocating more then can fit in {size_t}.
 @throws {vsnprintf/realloc} errors: {IEEE Std 1003.1-2001}. */
struct String *StringPrintCat(struct String *const string,
	const char *const fmt, ...) {
	va_list argp;
	char garbage;
	int length;
	size_t total_length;
	if(!string || !fmt) return string;
	/* Check the length first by printing to garbage. */
	va_start(argp, fmt);
	length = vsnprintf(&garbage, 0ul, fmt, argp);
	va_end(argp);
	if(length < 0) return 0; /* {vsnprintf} error. */
	total_length = string->length + length;
	if(total_length < string->length) { errno = ERANGE; return 0; }
	if(!text_length(string, total_length)) return 0;
	/* Now that we have enough space, do the actual printing. */
	va_start(argp, fmt);
	length = vsnprintf(string->text + string->length, string->capacity[0], fmt,
		argp);
	va_end(argp);
	if(length < 0) return 0; /* {vsnprintf} error. */
	string->length += length;
	return string;
}

#endif /* !STRING_STRICT_ANSI --> */

/** Transforms the original text according to {fmt}.
 @param string: If null, returns null.
 @param fmt: Accepts %% as '%' and %s as the original string. If null, returns
 {string}.
 @return {string}.
 @throws ERANGE: Tried allocating more then can fit in {size_t}.
 @throws {strdup/malloc/realloc} errors: {IEEE Std 1003.1-2001}. */
struct String *StringTransform(struct String *const string, const char *fmt) {
	int is_old_ref = 0, is_old_dup = 0;
	const char *f;
	char *t, null_termiate = '\0', *old = &null_termiate;
	size_t copy_len = 0, old_len = 0;
	if(!string || !fmt) return string;
	/* Count. */
	old_len = string->length;
	for(f = fmt; *f; f++) {
		if(*f != '%') { copy_len++; continue; }
		switch(*++f) {
			case '%': copy_len++; break;
			case 's': copy_len += old_len; is_old_ref = 1; break;
		}
	}
	/* Copy the string into {old}. */
	if(is_old_ref && string->text && string->text != '\0') {
		assert(string->length > 0);
		if(!(old = memdup(string->text, old_len + 1))) return 0;
		is_old_dup = 1;
	} else {
		old_len = 0; /* Paranoid. */
	}
	/* Allocate. */
	if(!text_length(string, copy_len))
		{ if(is_old_dup) free(old); return 0; }
	/* New string is the transform. */
	for(t = string->text, f = fmt; *f; f++) {
		if(*f != '%') { *t++ = *f; continue; }
		switch(*++f) {
			case '%': *t++ = '%'; break;
			case 's': memcpy(t, old, old_len), t += old_len; break;
		}
	}
	*t = '\0';
	string->length = copy_len;
	/* Free {old}. */
	if(is_old_dup) free(old);
	return string;
}
