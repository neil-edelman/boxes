/** 2017 Neil Edelman, distributed under the terms of the MIT License;
 see readme.txt, or \url{ https://opensource.org/licenses/MIT }.

 A dynamic string.

 @file		Text.c
 @author	Neil
 @version	1.0; 2017-03
 @since		1.0; 2017-03 */

/*#define TEXT_DEBUG*/

#include <stdlib.h> /* malloc free */
#include <stdio.h>  /* FILE fgets fprintf */
#include <errno.h>	/* errno */
#include <string.h>	/* strerror memcpy */
#include <limits.h>	/* INT_MAX */
#include <ctype.h>	/* isspace */
#include <stdarg.h>	/* va_* */
#include "Text.h"

/* <-- ugly */
#ifndef _MSC_VER /* <-- not msvc */
#define _TEXT_UNUSED(a) while(0 && (a));
#else /* not msvc --><-- msvc: not a C89/90 compiler; needs a little help */
#pragma warning(push)
/* "Assignment within conditional expression." No. */
#pragma warning(disable: 4706)
/* "<ANSI/ISO name>: The POSIX name for this item is deprecated." No. */
#pragma warning(disable: 4996)
#define _TEXT_UNUSED(a) (void)(sizeof((a), 0))
#endif /* msvc --> */
/* ugly --> */

static const size_t fibonacci6  = 8;
static const size_t fibonacci7  = 13;

static const size_t fibonacci11 = 89;
static const size_t fibonacci12 = 144;

enum Error {
	E_NO_ERROR,
	E_ERRNO,
	E_PARAMETER,
	E_OVERFLOW,
	E_SYNTAX
};
static const char *const error_explination[] = {
	"no error",
	0,
	"parameter out-of-range",
	"overflow",
	"syntax error"
};

static enum Error global_error = E_NO_ERROR;
static int        global_errno_copy;

struct Text {
	char *text;
	size_t length, capacity[2];
	enum Error error;
	int errno_copy;
};

/* private structs */

struct TextBraket {
	int is;
	const struct TextPattern *pattern;
	struct { char *s0, *s1; } bra, ket;
};

struct TextMatch {
	struct Text *text;
	char *start, *end;
};

struct TextMatches {
	struct Text *parent;
	struct TextMatch *matches;
	size_t size, capacity[2];
};

struct TextCut {
	int is;
	char *pos, stored;
};

static struct {
	int is_valid;
	struct Text *parent;
	size_t start, end;
} match_info;

/* private prototypes */

static void clear(struct Text *const this);
static int cat(struct Text *const this, const char *const str,
	const size_t str_len);
static int capacity_up(struct Text *const this, const size_t *const len_ptr);

void t_cut(struct TextCut *const this, char *const pos);
void t_uncut(struct TextCut *const this);
	
static int Matches(struct TextMatches *const this, struct Text *const parent);
static void Matches_(struct TextMatches *const this);
static int Matches_capacity_up(struct TextMatches *const this);
static struct TextMatch *Matches_new(struct TextMatches *const this,
	struct TextBraket *braket);
static void Matches_print(struct TextMatches *const this);
static void debug(struct Text *const this, const char *const fn,
	const char *const fmt, ...);



/**********
 * Public */

/** @return	A new, blank Text, will be constructed.
 @throws	E_ERRNO */
struct Text *Text(void) {
	struct Text *this;
	if(!(this = malloc(sizeof *this))) {
		global_error      = E_ERRNO;
		global_errno_copy = errno;
		Text_(&this);
		return 0;
	}
	this->text        = 0;
	this->length      = 0;
	this->capacity[0] = fibonacci11;
	this->capacity[1] = fibonacci12;
	this->error       = E_NO_ERROR;
	this->errno_copy  = 0;
	if(!(this->text = malloc(sizeof *this->text * this->capacity[0]))) {
		global_error      = E_ERRNO;
		global_errno_copy = errno;
		Text_(&this);
		return 0;
	}
	clear(this);
	debug(this, "Text", "constructed.\n");
	return this;
}

/** @param this_ptr	A pointer to Text that will be destructed. */
void Text_(struct Text **const this_ptr) {
	struct Text *this;

	if(!this_ptr || !(this = *this_ptr)) return;

	debug(this, "~Text", "destructing.\n");
	free(this->text);
	free(this);

	*this_ptr = 0;
}

/** Gets the string associated to {this}. Volatile, in the sense that it
 exposes the buffer; specifically, not guaranteed to last between {Text} calls
 to the same object. If you want a copy, do {strdup(TextToString(text))}. */
const char *TextToString(const struct Text *const this) {
	if(!this) return 0;
	return this->text;
}

/** @return		The last error associated with {this} (can be null.) */
const char *TextGetError(struct Text *const this) {
	const char *str;
	enum Error *perr;
	int *perrno;

	perr   = this ? &this->error      : &global_error;
	perrno = this ? &this->errno_copy : &global_errno_copy;
	if(!(str = error_explination[*perr])) str = strerror(*perrno);
	*perr = 0;

	return str;
}

/** Clears the Text. */
void TextClear(struct Text *const this) {
	if(!this) return;
	clear(this);
}

/** White-space trims the buffer associated with {this}. */
void TextTrim(struct Text *const this) {
	char *str, *a, *z;

	if(!this) return;
	str = this->text;
	z = str + strlen(str) - 1, a = str;
	while(z > str && isspace(*z)) z--;
	z++, *z = '\0';
	while(isspace(*a)) a++;
	/*if(a - str) memmove(str, a, (size_t)(z - a + 1));*/
	this->length = (size_t)(z - a);
	if(a - str) memmove(str, a, this->length + 1);
}

/** Separates a new token at the first {delims} that satisfy {pred}. Behaves
 like {strsep}.
 @param token_ptr	A pointer that receives a new {Text} token or null if the
					tokenisation is finished. You will have to call \see{Text_}.
 @param pred		Can be null, in which case, it behaves like true.
 @return			Success.
 @throws			E_PARAMETER, E_OVERFLOW, E_ERRNO */
int TextSplit(struct Text *const this, const char *const delims,
	struct Text **const token_ptr, const TextPredicate pred) {
	struct Text *token;
	char *bork;

	if(!this) return 0;
	if(!token_ptr || !delims) return this->error = E_PARAMETER, 0;
	*token_ptr = 0;

	/* find */
	bork = this->text;
	while((bork = strpbrk(bork, delims))) {
		if(pred && !pred(this->text, bork)) { bork++; continue; }
		break;
	}
	if(!bork) return -1;
	/* split at bork */
	if(!(token = Text())) {
		this->error = global_error, global_error = E_NO_ERROR;
		this->errno_copy = global_errno_copy; global_errno_copy = 0;
		return 0;
	}
	if(!cat(token, bork + 1, (size_t)(this->text + this->length - bork - 1))) {
		this->error = token->error, token->error = E_NO_ERROR;
		this->errno_copy = token->errno_copy, token->errno_copy = 0;
		Text_(&token);
		return 0;
	}
	/* truncate at bork */
	*bork        = '\0';
	this->length = bork - this->text;
	/* now switch the pointers; the token is supposed to be the one in front */
	{
		char *const temp   = this->text;
		size_t temp_length = this->length;
		this->text = token->text, this->length = token->length;
		token->text = temp,       token->length = temp_length;
	}
	/* assign the token_ptr to point at token */
	*token_ptr = token;

	debug(this, "TextSplit", "split.");
	return -1;
}

/** Replaces the value of {this} with {str}.
 @return	Success.
 @throws	E_PARAMETER, E_OVERFLOW, E_ERRNO */
int TextCopy(struct Text *const this, const char *const str) {
	if(!this) return 0;
	if(!str) return this->error = E_PARAMETER, 0;
	clear(this);
	return cat(this, str, strlen(str));
}

/** Replaces the value of {this} with {str} up to the {str_len}. Does not do a
 check to ensure that there are at least {str_len} bytes in the string. This
 differs from strncpy in that it copies exactly {str_len} characters instead of
 repeating null when it gets a zero. The string is always null-terminated.
 @return	Success.
 @throws	E_PARAMETER, E_OVERFLOW, E_ERRNO */
int TextNCopy(struct Text *const this, const char *const str,
	const size_t str_len) {
	if(!this) return 0;
	if(!str) return this->error = E_PARAMETER, 0;
	clear(this);
	return cat(this, str, str_len);
}

/** Replaces the value of {this} with {[a, b]}. If {a} > {b}, then empty.
 @return	Success.
 @throws	E_PARAMETER, E_OVERFLOW, E_ERRNO */
int TextBetweenCopy(struct Text *const this,
	const char *const a, const char *const b) {
	if(!this) return 0;
	if(!a || !b) return this->error = E_PARAMETER, 0;
	clear(this);
	return (a <= b) ? cat(this, a, (size_t)(b - a + 1)) : -1;
}

/** Concatenates {cat} onto the buffer in {this}.
 @return	Success.
 @throws	E_PARAMETER, E_OVERFLOW, E_ERRNO */
int TextCat(struct Text *const this, const char *const str) {
	if(!this) return 0;
	if(!str) return this->error = E_PARAMETER, 0;
	return cat(this, str, strlen(str));
}

/** Concatenates {cat_len} characters of {cat} onto the buffer in {this}. Does
 not do a check to ensure that there are at least {str_len} bytes in the string.
 @return	Success.
 @throws	E_PARAMETER, E_OVERFLOW, E_ERRNO */
int TextNCat(struct Text *const this, const char *const str,
	const size_t str_len) {
	if(!this) return 0;
	if(!str) return this->error = E_PARAMETER, 0;
	return cat(this, str, str_len);
}

/** Concatenates {this} with {[a, b]}. If {a} > {b}, then empty.
 @return	Success.
 @throws	E_PARAMETER, E_OVERFLOW, E_ERRNO */
int TextBetweenCat(struct Text *const this,
	const char *const a, const char *const b) {
	if(!this) return 0;
	if(!a || !b) return this->error = E_PARAMETER, 0;
	return (a <= b) ? cat(this, a, (size_t)(b - a + 1)) : -1;
}

/** Concatenates the contents of the text file, {fp}, after the read cursor, to
 the buffer in {this}. On success, the read cursor will be at the end.
 @return	Success.
 @throws	E_PARAMETER, E_OVERFLOW, E_ERRNO */
int TextFileCat(struct Text *const this, FILE *const fp) {
	size_t to_get;
	int to_get_int;
	int e;

	if(!this) return 0;
	if(!fp) { this->error = E_PARAMETER; return 0; }

	while(to_get = this->capacity[0] - this->length,
		to_get_int = to_get < INT_MAX ? (int)(to_get) : INT_MAX,
		fgets(this->text + this->length, to_get_int, fp)) {

		this->length += strlen(this->text + this->length);

		if(this->length >= this->capacity[0] - 1 && !capacity_up(this, 0))
			return 0;
	}
	if((e = ferror(fp))) return this->error = E_ERRNO, this->errno_copy = e, 0;

	debug(this, "TextFileCat", "appended a file.\n");
	return -1;
}

/** Concatenates the buffer with a printf, \see{printf}.
 @fixme		Have a function that allows replacing in the middle,
 TextPrintfCat(this, fmt, ...) -> TextPrintf(this, this.length, fmt, ...)
 and TextInsert() . . . */
int TextPrintfCat(struct Text *const this, const char *const fmt, ...) {
	va_list argp;
	char garbage;
	int length;
	size_t total_length;

	if(!this) return 0;
	if(!fmt) return this->error = E_PARAMETER, 0;

	va_start(argp, fmt);
	length = vsnprintf(&garbage, 0ul, fmt, argp);
	va_end(argp);

	if(length < 0) return this->error = E_ERRNO, this->errno_copy = errno, 0;
	total_length = this->length + length;
	if(total_length < (size_t)length) return this->error = E_OVERFLOW, 0;
	if(!capacity_up(this, &total_length)) return 0;

	va_start(argp, fmt);
	length = vsnprintf(this->text + this->length, this->capacity[0], fmt, argp);
	va_end(argp);

	if(length < 0) return this->error = E_ERRNO, this->errno_copy = errno, 0;
	this->length += length;

	debug(this, "TextPrintfCat", "printed.\n");
	return -1;
}

/** Transforms the original text according to {fmt}.
 @param fmt	Accepts %% as '%' and %s as the original string.
 @return	Success.
 @throws	E_OVERFLOW, E_ERRNO */
int TextTransform(struct Text *const this, const char *fmt) {
	char *copy, *t;
	const char *f;
	size_t copy_len = 0;

	if(!this) return 0;
	if(!(copy = strdup(this->text)))
		return this->error = E_ERRNO, this->errno_copy = errno, 0;

	/* count */
	for(f = fmt; *f; f++) {
		if(*f != '%') { copy_len++; continue; }
		switch(*++f) {
			case '%': copy_len++; break;
			case 's': copy_len += this->length; break;
		}
	}
	/* allocate */
	if(!capacity_up(this, &copy_len)) return free(copy), 0;
	/* new string */
	for(t = this->text, f = fmt; *f; f++) {
		if(*f != '%') { *t++ = *f; continue; }
		switch(*++f) {
			case '%': *t++ = '%'; break;
			case 's': memcpy(t, copy, this->length), t += this->length; break;
		}
	}
	*t = '\0';
	this->length = copy_len;
	/* free */
	free(copy);

	debug(this, "TextTransform", "transformed.\n");
	return -1;
}

/** Transforms {this} according to all specified {patterns} array of
 {patterns_size}.
 @param patterns	An array of {TextPattern}; when the {begin} of a pattern
					encompasses another pattern, it should be before in the
					array. All patterns must have {begin} and {transform}, but
					{end} is optional; where missing, it will just call
					{transform} with {begin}. */
int TextMatch(struct Text *const this, const struct TextPattern *const patterns,
	const size_t patterns_size) {
	struct Text *temp = 0;
	char *s0, *cursor;
	/* no, I don't use it uninitialised, but . . . whatever */
	struct TextBraket braket = { 0, 0, {0, 0}, {0, 0} }; /* working */
	struct TextMatches matches; /* storage array */
	struct TextMatch *match; /* one out of the array */
	struct TextCut cut = { 0, 0, 0 };
	enum { E_NO, E_MISSING, E_BUMP, E_TEMP, E_THIS } e = E_NO;

	if(!this || !Matches(&matches, this)) return 0;

	cursor = this->text;
	do {
		const struct TextPattern *pattern; size_t p;

		/* search for the next pattern */
		braket.is = 0;
		for(p = 0; p < patterns_size; p++) {
			pattern = patterns + p;
			if(!(s0 = strstr(cursor, pattern->start))) continue;
			/* this happens when first_pos is [abcdefg] and [cdef] is matched */
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
			t_uncut(&cut);
			if(!(braket.ket.s0 = strstr(braket.bra.s1, braket.pattern->end)))
				{ this->error = E_SYNTAX; return 0; }
			braket.ket.s1 = braket.ket.s0 + strlen(braket.pattern->end);
			t_cut(&cut, braket.ket.s0);
		}

		/* allocate the recursion; set the value back to how it was */
		if(!(match = Matches_new(&matches, &braket))) { e = E_BUMP; break; }
		t_uncut(&cut);
		/* this assumes uni-process! */
		match_info.parent = this;
		match_info.start = braket.bra.s0 - this->text;
		match_info.end = (braket.pattern->end ? braket.ket.s1 : braket.bra.s1)
			- this->text;
		/* call the handler */
		match_info.is_valid++;
		braket.pattern->transform(match->text);
		match_info.is_valid--;
		/*printf("now value \"%.40s..\" and first \"%s\" at \"%.40s..\".\n",
		 this->value, first_pat ? first_pat->begin : "(null)", first_pos);*/
		cursor = braket.pattern->end ? braket.ket.s1 : braket.bra.s1;
	} while(cursor);

	t_uncut(&cut);

	switch(e) {
		case E_NO:		break;
		case E_MISSING:	this->error = E_SYNTAX; break;
		case E_BUMP:	break;
		default:		break;
	}

	Matches_print(&matches);

	/* now go though the matches and substitute them in */
	do {
		size_t i;
		if(e) break;
		cursor = this->text;
		if(!(temp = Text())) { e = E_TEMP; break; }
		for(i = 0; i < matches.size; i++) {
			match = matches.matches + i;
			if(!cat(temp, cursor, (size_t)(match->start - cursor))
				|| !cat(temp, match->text->text, match->text->length))
				{ e = E_TEMP; break; }
			cursor = match->end;
		}
		if(e) break;
		if(!cat(temp, cursor, strlen(cursor))) { e = E_TEMP; break; }
		clear(this);
		if(!cat(this, temp->text, temp->length)) { e = E_THIS; break; };
	} while(0);

	switch(e) {
		case E_NO:		break;
		case E_TEMP:	this->error = temp ? temp->error : global_error,
			this->errno_copy = temp ? temp->errno_copy : global_errno_copy;
			break;
		case E_THIS:	break;
		default:		break;
	}

	Matches_(&matches);
	Text_(&temp);

	debug(this, "TextMatch", "matched.\n");
	return e ? 0 : -1;
}

/** When {TextMatch} finds a match, it also stores a global data on where the
 match occurred within the parent. This gets that information. Don't call on
 multi-threaded executions. If any pointers are null, ignores them. Useful for
 match-handlers.
 @return	Success; otherwise the values are invalid and will not be set. */
int TextGetMatchInfo(struct Text **parent_ptr,
	size_t *const start_ptr, size_t *const end_ptr) {
	if(!match_info.is_valid) return 0;
	if(parent_ptr) *parent_ptr = match_info.parent;
	if(start_ptr)   *start_ptr = match_info.start;
	if(end_ptr)       *end_ptr = match_info.end;
	return -1;
}





/************
 * Private. */

/** Clears the text. */
static void clear(struct Text *const this) {
	this->text[0] = '\0';
	this->length  = 0;
}

/** @throws	E_OVERFLOW, E_ERRNO */
static int cat(struct Text *const this, const char *const str,
	const size_t str_len) {
	const size_t old_len = this->length, new_len = old_len + str_len;

	if(new_len == old_len) return -1;
	if(new_len < old_len) return this->error = E_OVERFLOW, 0;
	if(!capacity_up(this, &new_len)) return 0;
	memcpy(this->text + old_len, str, str_len);
	this->text[new_len] = '\0';
	this->length = new_len;

#ifdef TEXT_DEBUG
	if(strlen(this->text) != this->length) {
		fprintf(stderr, "Error '%s' strlen %lu length %lu\n", this->text,
			strlen(this->text), this->length);
		exit(EXIT_FAILURE);
	}
	fprintf(stderr, "cat: '%s' <- '%.*s'; %lu <- %lu\n", this->text,
		(int)str_len, str, this->length, str_len);
#endif

	return -1;

}

/** Ensures value capacity.
 @param size_ptr	Can be null, in which case the capacity increases one level
					in size. If it is not, the capacity increases at or beyond
					this number.
 @return	Success.
 @throws	E_OVERFLOW, E_ERRNO */
static int capacity_up(struct Text *const this, const size_t *const len_ptr) {
	size_t c0, c1;
	char *text;

	/* fibonacci */
	c0 = this->capacity[0];
	c1 = this->capacity[1];
	while(!len_ptr || c0 <= *len_ptr) {
		if(c0 == (size_t)-1) { this->error = E_OVERFLOW; return 0; }
		c0 ^= c1;
		c1 ^= c0;
		c0 ^= c1;
		c1 += c0;
		if(c1 <= c0) c1 = (size_t)-1;
		if(!len_ptr) break;
	}
	if(this->capacity[0] >= c0) return -1; /* it's already that size */
	/*debug(this, "value_capacity_up","%lu->%lu.\n",this->value_capacity[0],c0);*/
	if(!(text = realloc(this->text, c0 * sizeof *this->text)))
		return this->error = E_ERRNO, this->errno_copy = errno, 0;
	this->text = text, this->capacity[0] = c0, this->capacity[1] = c1;

	return -1;
}

/*****************************************
 * TextCut (just initialise to all zero) */

/** Stores the character at {pos} in {this} and terminates the string there. */
void t_cut(struct TextCut *const this, char *const pos) {
	if(this->is) return;
	this->is = -1, this->pos = pos, this->stored = *pos, *pos = '\0';
}
/** If the string has been cut, undoes that. */
void t_uncut(struct TextCut *const this) {
	if(!this->is) return;
	this->is = 0;
	*this->pos = this->stored;
}

/***************
 * TextMatches */

/** Initialise. All errors are bumped up to the parent; you can just return 0.
 @return	Success.
 @throws	E_ERRNO */
static int Matches(struct TextMatches *const this, struct Text *const parent) {
	this->parent      = parent;
	this->matches     = 0;
	this->size        = 0;
	this->capacity[0] = fibonacci6;
	this->capacity[1] = fibonacci7;
	if(!(this->matches = malloc(this->capacity[0] * sizeof *this->matches)))
		return parent->error = E_ERRNO, parent->errno_copy = errno, 0;
	return -1;
}

/** Destroy. */
static void Matches_(struct TextMatches *const this) {
	size_t i;

	for(i = 0; i < this->size; i++) {
		Text_(&this->matches[i].text);
	}
	free(this->matches);
	this->matches = 0;
	this->size = 0;
}

/** Increases the buffer size.
 @throws	E_OVERFLOW, E_ERRNO */
static int Matches_capacity_up(struct TextMatches *const this) {
	size_t c0, c1;
	struct TextMatch *matches;

	c0 = this->capacity[0];
	c1 = this->capacity[1];
	if(c0 == (size_t)-1) return this->parent->error = E_OVERFLOW, 0;
	c0 ^= c1;
	c1 ^= c0;
	c0 ^= c1;
	c1 += c0;
	if(c1 <= c0) c1 = (size_t)-1;
	if(!(matches = realloc(this->matches, c0 * sizeof *this->matches)))
		return this->parent->error = E_ERRNO, this->parent->errno_copy=errno, 0;
	this->matches = matches, this->capacity[0] = c0, this->capacity[1] = c1;

	return -1;
}

/** Constructs new match within {this}. */
static struct TextMatch *Matches_new(struct TextMatches *const this,
	struct TextBraket *braket) {
	struct TextMatch *match;
	if(this->size >= this->capacity[0] && !Matches_capacity_up(this)) return 0;
	match        = this->matches + this->size;
	match->text  = 0;
	match->start = braket->bra.s0;
	match->end   = braket->pattern->end ? braket->ket.s1 : braket->bra.s1;
	if(!(match->text = Text())) {
		this->parent->error      = global_error;
		this->parent->errno_copy = global_errno_copy;
		return 0;
	}
	if(!cat(match->text, braket->bra.s1,
		(size_t)(braket->pattern->end ? braket->ket.s0 - braket->bra.s1 : 0))) {
		this->parent->error      = match->text->error;
		this->parent->errno_copy = match->text->errno_copy;
		Text_(&match->text);
		return 0;
	}
	this->size++;
	return match;
}

static void Matches_print(struct TextMatches *const this) {
#ifdef TEXT_DEBUG
	size_t i;
	if(!this) printf("null\n"); return;
	printf("Matches {");
	for(i = 0; i < this->size; i++) {
		printf("%s\n\t%.4s\"%s\"%.4s", i ? "," : "", this->matches->start,
			this->matches->text->text, this->matches->end);
	}
	printf("\n}\n");
#else
	_TEXT_UNUSED(this);
#endif
}

static void debug(struct Text *const this, const char *const fn,
	const char *const fmt, ...) {
#ifdef TEXT_DEBUG
	const size_t length = this->text ? strlen(this->text) : 0;
	va_list parg;

	va_start(parg, fmt);
	fprintf(stderr, "Text.%s[%.160s]: ", fn, this->text);
	vfprintf(stderr, fmt, parg);
	va_end(parg);
	if(length != this->length) fprintf(stderr, "Text.length %lu but strlen %lu."
		"\n", this->length, length), exit(EXIT_FAILURE);
#else
	_TEXT_UNUSED(this); _TEXT_UNUSED(fn); _TEXT_UNUSED(fmt);
#endif
}
