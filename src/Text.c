/** Copyright 2017 Neil Edelman, distributed under the terms of the MIT License;
 see readme.txt, or \url{ https://opensource.org/licenses/MIT }.

 For reading and parsing text files.

 @param TEXT_DEBUG
 Prints things to stderr.

 @file		Text
 @author	Neil
 @version	1.0; 2017-02
 @since		1.0; 2017-02 */

#define TEXT_DEBUG

#include <stdlib.h> /* malloc free */
#include <stdio.h>  /* fprintf */
#include <errno.h>
#include <string.h>	/* strerror */
#include <limits.h>	/* INT_MAX */
#ifdef TEXT_DEBUG /* <-- */
#include <stdarg.h>
#endif /* --> */
#include "Text.h"

/* <-- ugly compiler-specific code, ie, MSVC/not-MSVC */
#ifdef _UNUSED
#undef _UNUSED
#endif
#ifndef _MSC_VER /* <-- not msvc */
/* don't produce warnings, yet optimise away the obviously useless while? hmm,
 I have not seen a clean solution for this */
#define _UNUSED(a) while(0 && (a));
#else /* not msvc --><-- msvc: not a C89/90 compiler; needs a little help */
#pragma warning(push)
/* "Assignment within conditional expression." That is a C language feature
 that I use all the time and is perfectly valid. */
#pragma warning(disable: 4706)
/* "<ANSI/ISO name>: The POSIX name for this item is deprecated. Instead use
 the ISO C and C++ conformant name <ISO C11 name>." No. */
#pragma warning(disable: 4996)
/* the pre-compiler is a little too smart for it's own good,
 http://stackoverflow.com/questions/4851075/universally-compiler-independent-
 way-of-implementing-an-unused-macro-in-c-c . . . what does that even mean?
 that can't be valid C? but works with MSVC */
#define _UNUSED(a) (void)(sizeof((a), 0))
#endif /* msvc --> */
/* ugly --> */



/* for allocating recursively */
static const unsigned fibonacci6  = 8;
static const unsigned fibonacci7  = 13;

/* for allocating buffer */
static const unsigned fibonacci11 = 89;
static const unsigned fibonacci12 = 144;

enum Error {
	E_NO_ERROR,
	E_ERRNO,
	E_PARAMETER,
	E_OVERFLOW,
	E_SYNTAX,
	E_ASSERT,
	E_PARENT
};

static const char *const error_explination[] = {
	"no error",
	0,
	"parameter out-of-range",
	"overflow",
	"syntax error",
	"assertion failed",
	"null parent"
};

/* globals */

static enum Error global_error = E_NO_ERROR;
static int        global_errno_copy;

struct Text {
	struct Text *up;
	size_t up_begin, up_end;
	char *name;
	char *buffer;
	size_t buffer_size, buffer_capacity[2];
	struct Text **downs;
	size_t downs_size, downs_capacity[2];
	enum Error error;
	int errno_copy;
};

/* private */

static struct Text *Text(char *const name);
static struct Text *Text_string_recursive(struct Text *const up,
	const size_t up_begin, const size_t up_end, char *const str);
static int buffer_capacity_up(struct Text *const this,
	const size_t *const size_ptr);
static int downs_capacity_up(struct Text *const this);
static void debug(struct Text *const this, const char *const fn,
	const char *const fmt, ...);



/**********
 * Public */

/** Constructs an empty {Text} out of the filename, {fn}, having the name {fn}.
 @fixme		Assumes 8-bit encoding.
 @fixme		Hmmmm, Text() and TextFile()? can't decide wether to make it big or
			small. */
struct Text *TextFile(char *const fn) {
	struct Text *this;
	FILE *fp;
	size_t cursor = 0;
	int size, err;

	/* param check */
	if(!fn || !fn[0])
		{ global_error = E_PARAMETER; return 0; }

	/* generic constructor */
	if(!(this = Text(fn)))
		{ global_error = E_ERRNO, global_errno_copy = errno; return 0; }

	/* open */
	if(!(fp = fopen(fn, "r"))) {
		global_error = E_ERRNO, global_errno_copy = errno;
		Text_(&this);
		return 0;
	}

	/* allocate buffer; there is no real way to portably and correctly get the
	 number of characters in a text file, so allocate incrementally */
	if(!(this->buffer=malloc(this->buffer_capacity[0] *sizeof *this->buffer))) {
		global_error = E_ERRNO, global_errno_copy = errno;
		if(fclose(fp) == EOF) perror(fn); /* unreported error */
		Text_(&this);
		return 0;
	}

	/* read */
	while(size = (this->buffer_capacity[0] - cursor > INT_MAX) ?
		INT_MAX : (int)(this->buffer_capacity[0] - cursor),
		fgets(this->buffer + cursor, size, fp)) {
		cursor += strlen(this->buffer + cursor);
		if(cursor >= this->buffer_capacity[0] - 1
			&& !buffer_capacity_up(this, 0)) {
			global_error = this->error;
			if(fclose(fp) == EOF) perror(fn); /* unreported error */
			Text_(&this);
			return 0;
		}
	}
	if((err = ferror(fp))) {
		global_error = E_ERRNO;
		global_errno_copy = err;
		Text_(&this);
		return 0;
	}

	/* close */
	if(fclose(fp) == EOF) perror(fn); /* unreported error */

	return this;
}

/** Constructs an empty {Text} out of the string, {str}, having the name,
 {name}. */
struct Text *TextString(char *const name, char *const str) {
	struct Text *this;
	size_t str_size;

	if(!str || !name) return 0;

	str_size = strlen(str) + 1; /* danger */

	if(!(this = Text(name)))
		{ global_error = E_ERRNO, global_errno_copy = errno; return 0; }

	this->buffer_capacity[0] += str_size; /* danger */
	this->buffer_capacity[1] += str_size; /* danger */

	if(!(this->buffer=malloc(this->buffer_capacity[0] * sizeof *this->buffer))){
		global_error = E_ERRNO, global_errno_copy = errno;
		Text_(&this);
		return 0;
	}
	memcpy(this->buffer, str, str_size);

	return this;
}

/** Destructs a {Text}. */
void Text_(struct Text **const this_ptr) {
	struct Text *this;
	size_t i;

	if(!this_ptr || !(this = *this_ptr)) return;

	for(i = 0; i < this->downs_size; i++) Text_(&this->downs[i]);

	debug(this, "~", "erase.\n");
	free(this->downs);
	free(this->buffer);
	free(this);

	*this_ptr = 0;
}

/** Gets a string from the text; valid until the text size changes. */
char *TextGetBuffer(struct Text *const this) {
	if(!this) return 0;
	return this->buffer;
}

const char *TextGetParentBuffer(struct Text *const this) {
	if(!this) return 0;
	if(!this->up) { this->error = E_PARENT; return 0; }
	return this->up->buffer;
}

size_t TextGetParentStart(struct Text *const this) {
	if(!this) return 0;
	return this->up_begin;
}

size_t TextGetParentEnd(struct Text *const this) {
	if(!this) return 0;
	return this->up_end;
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



/** Transforms {this} according to all specified {patterns} array of
 {patterns_size}.
 @param patterns	An array of {TextPattern}; when the {begin} of a pattern
 					encompasses another pattern, it should be before in the
					array. All patterns must have {begin} and {transform}, but
					{end} is optional; where missing, it will just call
					{transform} with {begin}. */
int TextMatch(struct Text *this, const struct TextPattern *const patterns,
	const size_t patterns_size) {
	struct TextPattern *pattern; size_t p;
	char *s0, *b;
	struct Match {
		struct TextPattern *pattern;
		struct Expression { char *s0, *s1; } exp0, exp1;
	} match;
	struct Replace { int is; char *pos; char stored; } replace = { 0,0,0 };

	if(!this) return 0;

	b = this->buffer;
	do {
		/* reset {match} */
		match.pattern = 0, match.exp0.s0 = 0, match.exp0.s1 = 0,
			match.exp1.s0 = 0, match.exp1.s1 = 0;
		for(p = 0; p < patterns_size; p++) {
			pattern = (struct TextPattern *)patterns + p;
			/*printf("matching(\"%s\"..\"%s\") in \"%.30s..\".\n",
				pattern->begin, pattern->end, b);*/
			if(!(s0 = strstr(b, pattern->begin))) continue;
			/* this happens when first_pos is [abcdefg] and [cdef] is matched */
			if(match.pattern && s0 >= match.exp0.s0) continue;
			/* remove the temporary null */
			if(replace.is) { *replace.pos = replace.stored, replace.is = 0; }
			/* replace match with this match, since it's the first */
			match.pattern = pattern;
			match.exp0.s0 = s0;
			match.exp0.s1 = s0 + strlen(pattern->begin);
			/* put in temporary null */
			replace.is = -1, replace.pos = match.exp0.s1,
				replace.stored = *replace.pos, *replace.pos = '\0';
		}
		/* didn't find any patterns */
		if(!match.pattern) break;
		/* if the {first_pat} has an ending, move the cursor to ending */
		if(match.pattern->end) {
			/* temp replacement of stored char */
			*replace.pos = replace.stored, replace.is = 0;
			/* search for the ending */
			if(!(match.exp1.s0 = strstr(match.exp0.s1, match.pattern->end)))
				{ this->error = E_SYNTAX; return 0; }
			match.exp1.s1 = match.exp1.s0 + strlen(match.pattern->end);
			/* put in temporary null */
			replace.is = -1, replace.pos = match.exp1.s0,
				replace.stored = *replace.pos, *replace.pos = '\0';
		}
		printf("match: \"%.*s\" \"%.30s...\" \"%.*s\"\n",
			(int)(match.exp0.s1 - match.exp0.s0), match.exp0.s0,
			match.exp0.s1,
			(int)(match.exp1.s1 - match.exp1.s0), match.exp1.s0);
		/* allocate the recursion; set the buffer back to how it was */
		if(this->downs_size >= this->downs_capacity[0]
			&& !downs_capacity_up(this)) return 0;
		if(!(this->downs[this->downs_size++] = Text_string_recursive(this,
			(size_t)(match.exp0.s0 - this->buffer),
			(size_t)(match.exp1.s1 - this->buffer), match.exp0.s1)))
			{ *replace.pos = replace.stored; return 0; }
		*replace.pos = replace.stored;
		/* call pattern function on the duplicated substring */
		match.pattern->transform(this->downs[this->downs_size - 1]);
		/*printf("now buffer \"%.40s..\" and first \"%s\" at \"%.40s..\".\n",
			this->buffer, first_pat ? first_pat->begin : "(null)", first_pos);*/
		b = match.exp1.s1;
	} while(b);

	return -1;
}

/** Ensures that we have a buffer of at least {capacity}. All pointers to the
 buffer will potentially change.
 @return	Success.
 @throws	E_OVERFLOW, E_ERRNO */
int TextEnsureCapacity(struct Text *const this, const size_t capacity) {
	if(!this) return 0;
	return buffer_capacity_up(this, &capacity);
}

/** E ch, e: p(e.key) -> a(e.value) */
void TextForEachPassed(struct Text *const this, TextPredicate p, TextAction a) {
	struct Text *down;
	unsigned i;
	for(i = 0; i < this->downs_size; i++) {
		down = this->downs[i];
		if(!p || p(down->name)) a(down);
	}
}

static const char *const start_str     = "[ ";
static const char *const end_str       = " ]";
static const char *const alter_end_str = "...]";
static const char *const ass_str       = "->";
static const char *const null_str      = "Null";

struct SuperCat {
	char *print, *cursor;
	size_t left;
	int is_truncated;
};
static void super_cat_init(struct SuperCat *const cat,
	char *const print, const size_t print_size) {
	cat->print = cat->cursor = print;
	cat->left = print_size;
	cat->is_truncated = 0;
	print[0] = '\0';
}
static void super_cat(struct SuperCat *const cat,
	const char *const append) {
	size_t lu_took; int took;
	if(cat->is_truncated) return;
	took = snprintf(cat->cursor, cat->left, "%s", append);
	if(took < 0)  { cat->is_truncated = -1; return; } /*implementation defined*/
	if(took == 0) { return; }
	if((lu_took=took)>=cat->left) {cat->is_truncated=-1,lu_took=cat->left-1;}
	cat->cursor += lu_took, cat->left -= lu_took;
}

static void to_string_recursive(struct Text *const this, struct SuperCat *cat_ptr) {
	unsigned i;
	char key[80];
	if(snprintf(key, sizeof key, "%s", this->buffer) > (int)sizeof key) {
		const unsigned key_size = sizeof key;
		key[key_size - 4] = '.',key[key_size - 3] = '.',key[key_size - 2] = '.';
	}
	super_cat(cat_ptr, start_str);
	super_cat(cat_ptr, this->name);
	super_cat(cat_ptr, ass_str);
	super_cat(cat_ptr, key);
	for(i = 0; i < this->downs_size; i++) {
		to_string_recursive(this->downs[i], cat_ptr);
	}
	super_cat(cat_ptr, end_str);
}

/** Prints the Text in a static buffer; one can print 4 things at once before
 it overwrites. */
char *TextToString(struct Text *const this) {
	static char buffer[4][2048];
	static int buffer_i;
	struct SuperCat cat;
	super_cat_init(&cat, buffer[buffer_i],
		sizeof *buffer / sizeof **buffer);
	buffer_i++, buffer_i &= 3;
	if(!this) {
		super_cat(&cat, null_str);
		return cat.print;
	}
	to_string_recursive(this, &cat);
	return cat.print;
}

/** Shortcut to add something to the text.
 @param fmt	Accepts %% as '%' and %s as the original string.
 @return	The new address of the string, which may have changed. */
char *TextAdd(struct Text *const this, char *const fmt) {
	char *str, *copy, *p;
	size_t str_len, copy_len = 0, copy_size;

	if(!this) return 0;
	str = this->buffer, str_len = strlen(str);
	if(!(copy = strdup(str))) return 0;
	
	/* count */
	for(p = fmt; *p; p++) {
		if(*p != '%') { copy_len++; continue; }
		switch(*++p) {
			case '%': copy_len++; break;
			case 's': copy_len += str_len; break;
		}
	}
	/* allocate */
	copy_size = copy_len + 1;
	if(!buffer_capacity_up(this, &copy_size)) { free(copy); return 0; };
	/* new string */
	str = this->buffer;
	for(p = fmt; *p; p++) {
		if(*p != '%') { *str++ = *p; continue; }
		switch(*++p) {
			case '%': *str++ = *p; break;
			case 's': memcpy(str, copy, str_len), str += str_len; break;
		}
	}
	*str = '\0';
	/* free */
	free(copy);
	return this->buffer;
}

void TextOutput(struct Text *const this, FILE *fp) {
	struct Text *down;
	size_t i;
	char *cursor;
	if(!this) return;
	cursor = this->buffer;
	for(i = 0; i < this->downs_size; i++) {
		down = this->downs[i];
		fprintf(fp, "[%.*s]\\", (int)(this->buffer+down->up_begin-cursor), cursor);
		TextOutput(down, fp);
		fprintf(fp, "/");
		cursor = this->buffer + down->up_end;
	}
	fprintf(fp, "[%s].", cursor);
}



/************
 * Private. */

/** Constructs a generic buffer, but buffer is null to give the calling fn room
 to expand; set it to something. If returning 0, there was a allocation error,
 and {errno} is set. */
static struct Text *Text(char *const name) {
	struct Text *this;
	size_t name_size;

	name_size = strlen(name) + 1;
	if(!(this = malloc(sizeof *this + name_size))) return 0;
	this->up                 = 0;
	this->up_begin           = 0;
	this->up_end             = 0;
	this->name               = (char *)(this + 1);
	memcpy(this->name, name, name_size);
	this->buffer             = 0;
	this->buffer_size        = 0;
	this->buffer_capacity[0] = fibonacci11;
	this->buffer_capacity[1] = fibonacci12;
	this->downs              = 0;
	this->downs_size         = 0;
	this->downs_capacity[0]  = fibonacci6;
	this->downs_capacity[1]  = fibonacci7;
	this->error              = E_NO_ERROR;
	this->errno_copy         = 0;

	if(!(this->downs = malloc(this->downs_capacity[0] * sizeof *this->downs)))
		{ Text_(&this); return 0; }

	debug(this, "cons", "new, \"%s,\" capacity %d.\n", name,
		this->buffer_capacity[0]);

	return this;
}

static struct Text *Text_string_recursive(struct Text *const up,
	const size_t up_begin, const size_t up_end, char *const str) {
	struct Text *this;
	size_t str_size;

	if(!str) return 0;

	str_size = strlen(str) + 1; /* danger */

	if(up && str_size > up_end - up_begin + 1) {
		up->error = E_ASSERT;
		return 0;
	}
	/*printf("Text_string_rec: up %s, up_begin %lu, up_end %lu, \"%s\".\n",
		TextToString(up), up_begin, up_end, str);*/

	if(!(this = Text("_nemo")))
		{ up->error = E_ERRNO, up->errno_copy = errno; return 0; }

	this->up       = up;
	this->up_begin = up_begin;
	this->up_end   = up_end;
	this->buffer_capacity[0] += str_size; /* danger */
	this->buffer_capacity[1] += str_size; /* danger */

	if(!(this->buffer=malloc(this->buffer_capacity[0] * sizeof *this->buffer))){
		up->error = E_ERRNO, up->errno_copy = errno;
		Text_(&this);
		return 0;
	}
	memcpy(this->buffer, str, str_size);
	/*printf("recursive: \"%s\"\n", this->buffer);*/

	return this;
}

/** Ensures buffer capacity.
 @param size_ptr	Can be null, in which case the capacity increases one level
					in size. If it is not, the capacity increases at or beyond
					this number.
 @return	Success.
 @throws	E_OVERFLOW, E_ERRNO */
static int buffer_capacity_up(struct Text *const this,
	const size_t *const size_ptr) {
	size_t c0, c1;
	int is_up = size_ptr ? 0 : -1;
	char *buffer;

	/* fibonacci */
	c0 = this->buffer_capacity[0];
	c1 = this->buffer_capacity[1];
	while(is_up || (size_ptr && c0 < *size_ptr)) {
		if(c0 == (size_t)-1) { this->error = E_OVERFLOW; return 0; }
		c0 ^= c1;
		c1 ^= c0;
		c0 ^= c1;
		c1 += c0;
		if(c1 <= c0) c1 = (size_t)-1;
		is_up = 0;
	}
	if(this->buffer_capacity[0] >= c0) return -1; /* it's already that size */
	debug(this, "buffer_capacity_up","%lu->%lu.\n",this->buffer_capacity[0],c0);
	if(!(buffer = realloc(this->buffer, c0 * sizeof *this->buffer))) {
		this->error = E_ERRNO, this->errno_copy = errno;
		return 0;
	}
	this->buffer = buffer;
	this->buffer_capacity[0] = c0;
	this->buffer_capacity[1] = c1;

	return -1;
}

/** Ensures downs capacity.
 @return	Success.
 @throws	E_OVERFLOW, E_ERRNO */
static int downs_capacity_up(struct Text *const this) {
	size_t c0, c1;
	struct Text **downs;

	/* fibonacci */
	c0 = this->downs_capacity[0];
	c1 = this->downs_capacity[1];
	if(c0 == (size_t)-1) { this->error = E_OVERFLOW; return 0; }
	c0 ^= c1;
	c1 ^= c0;
	c0 ^= c1;
	c1 += c0;
	if(c1 <= c0) c1 = (size_t)-1;
	debug(this, "downs_capacity_up", "%lu->%lu.\n", this->downs_capacity[0],c0);
	if(!(downs = realloc(this->downs, c0 * sizeof *this->downs))) {
		this->error = E_ERRNO, this->errno_copy = errno;
		return 0;
	}
	this->downs = downs;
	this->downs_capacity[0] = c0;
	this->downs_capacity[1] = c1;

	return -1;
}

/** Private debug messages from list functions; turn on using {LIST_DEBUG}. */
static void debug(struct Text *const this, const char *const fn,
	const char *const fmt, ...) {
#ifdef TEXT_DEBUG
	va_list parg;

	va_start(parg, fmt);
	fprintf(stderr, "Text<%s>#%p.%s: ", this->name, (void *)this, fn);
	vfprintf(stderr, fmt, parg);
	va_end(parg);
#else
	_UNUSED(this); _UNUSED(fn); _UNUSED(fmt);
#endif
}
