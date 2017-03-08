/** Copyright 2017 Neil Edelman, distributed under the terms of the MIT License;
 see readme.txt, or \url{ https://opensource.org/licenses/MIT }.

 For reading and parsing text files.

 @param TEXT_DEBUG
 Prints things to stderr.

 @file		Text
 @author	Neil
 @version	1.0; 2017-02
 @since		1.0; 2017-02 */

/*#define TEXT_DEBUG*/

#include <stdlib.h> /* malloc free */
#include <stdio.h>  /* fprintf */
#include <errno.h>
#include <string.h>	/* strerror */
#include <limits.h>	/* INT_MAX */
#include <ctype.h>	/* isspace */
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

/* for allocating value */
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
	struct Text *parent;
	int is_value_in_parent;
	size_t parent_begin, parent_end; /* only valid if is_value_in_parent */
	char *key;
	char *value;
	size_t value_capacity[2];
	struct Text **childs;
	size_t childs_size, childs_capacity[2];
	enum Error error;
	int errno_copy;
};

/* private */

static struct Text *Text_string_recursive(struct Text *const parent,
	const char *const key, char *const value, const int is_value_in_parent,
	const size_t parent_begin, const size_t parent_end);
static int value_capacity_up(struct Text *const this,
	const size_t *const size_ptr);
static int childs_capacity_up(struct Text *const this);
static char *anonymous_volatile_key(void);
static void debug(struct Text *const this, const char *const fn,
	const char *const fmt, ...);



/**********
 * Public */

/** Constructs a generic, empty Text. */
struct Text *Text(const char *const key) {
	struct Text *this;
	size_t key_size;

	key_size = strlen(key) + 1;
	if(!(this = malloc(sizeof *this + key_size))) return 0;
	this->parent             = 0;
	this->is_value_in_parent = 0;
	this->parent_begin       = 0;
	this->parent_end         = 0;
	this->key                = (char *)(this + 1);
	memcpy(this->key, key, key_size);
	this->value              = 0;
	this->value_capacity[0]  = fibonacci11;
	this->value_capacity[1]  = fibonacci12;
	this->childs             = 0;
	this->childs_size        = 0;
	this->childs_capacity[0] = fibonacci6;
	this->childs_capacity[1] = fibonacci7;
	this->error              = E_NO_ERROR;
	this->errno_copy         = 0;

	if(!(this->childs = malloc(this->childs_capacity[0] * sizeof *this->childs))
		|| !(this->value = malloc(this->value_capacity[0]
		* sizeof *this->value))) {
		global_error = E_ERRNO, global_errno_copy = errno;
		Text_(&this);
		return 0;
	}

	debug(this, "cons", "new, \"%s,\" capacity %d.\n", key,
		this->value_capacity[0]);
	return this;
}

/** Destructs a {Text}. */
void Text_(struct Text **const this_ptr) {
	struct Text *this;
	size_t i;

	if(!this_ptr || !(this = *this_ptr)) return;

	for(i = 0; i < this->childs_size; i++) Text_(&this->childs[i]);

	debug(this, "~", "erase.\n");
	free(this->childs);
	free(this->value);
	free(this);

	*this_ptr = 0;
}

/** Gets the key. */
const char *TextGetKey(struct Text *const this) {
	if(!this) return 0;
	return this->key;
}

/** Gets a string from the text; volatile; valid until the text size changes. */
char *TextGetValue(struct Text *const this) {
	if(!this) return 0;
	return this->value;
}

/** Does a short-circuit linear search of {this} for a child key that matches
 {key}.
 @return	The text or null if the key was not found. */
struct Text *TextGetChildKey(struct Text *const this, const char *const key) {
	struct Text *found = 0, *child;
	size_t d;

	if(!this || !key) return 0;
	for(d = 0; d < this->childs_size; d++) {
		child = this->childs[d];
		if(strcmp(child->key, key)) continue;
		found = child;
		break;
	}
	return found;
}

char *TextGetParentValue(struct Text *const this) {
	if(!this) return 0;
	if(!this->parent) { this->error = E_PARENT; return 0; }
	return this->parent->value;
}

int TextGetIsWithinParentValue(struct Text *const this) {
	if(!this) return 0;
	return this->is_value_in_parent;
}

size_t TextGetParentStart(struct Text *const this) {
	if(!this) return 0;
	return this->parent_begin;
}

size_t TextGetParentEnd(struct Text *const this) {
	if(!this) return 0;
	return this->parent_end;
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

/** Concatenates the value in {this} with the entre contents of the file, {fp}.
 @fixme		Assumes 8-bit encoding? :[ */
int TextFile(struct Text *const this, FILE *const fp) {
	size_t cursor;
	int err;

	if(!this) return 0;
	if(!fp) { this->error = E_PARAMETER; return 0; }
	cursor = strlen(this->value);

	/* read */
	while(fgets(this->value + cursor, (int)(this->value_capacity[0] - cursor),
		fp)) {

		/* cursor is always just ahead of the reading */
		cursor += strlen(this->value + cursor);

		/* allocate new space? */
		if(cursor >= this->value_capacity[0] - 1
			&& !value_capacity_up(this, 0)) {
			if(fclose(fp) == EOF) perror(this->key); /* unreported error */
			return 0;
		}
	}
	if((err = ferror(fp))) {
		this->error = E_ERRNO, this->errno_copy = err;
		return 0;
	}

	return -1;
}

/** Concatenates {*cat_len_ptr} characters (or all the string if it is null) of
 {cat} onto the value in {this}.
 @fixme		Hmm, maybe there should be a value_len for this?
 @return	Success.
 @throws	E_PARAMETER, E_OVERFLOW, E_ERRNO */
int TextCat(struct Text *const this, char *const cat,
	const size_t *const cat_len_ptr) {
	size_t old_len, cat_len, new_size;

	if(!this) return 0;
	if(!cat) return this->error = E_PARAMETER, 0;
	old_len = strlen(this->value);
	cat_len = cat_len_ptr ? *cat_len_ptr : strlen(cat);
	new_size = old_len + cat_len + 1; /* danger */
	if(!value_capacity_up(this, &new_size)) return 0;
	memcpy(this->value + old_len, cat, cat_len);
	this->value[new_size - 1] = '\0';
	return -1;
}

/** Replaces the value of {this} with {str} up to the pointed-to {str_len_ptr},
 or, if null, the entire string. */
int TextCopy(struct Text *const this, char *const str,
	const size_t *const str_len_ptr) {
	size_t str_len, new_size;

	if(!this) return 0;
	if(!str) return this->error = E_PARAMETER, 0;
	str_len = str_len_ptr ? *str_len_ptr : strlen(str);
	new_size = str_len + 1; /* danger */
	if(!value_capacity_up(this, &new_size)) return 0;
	memcpy(this->value, str, str_len);
	this->value[str_len] = '\0';
	return -1;
}

/** Transforms {this} according to all specified {patterns} array of
 {patterns_size}. This allocates new children as needed to go with the matches.
 @param patterns	An array of {TextPattern}; when the {begin} of a pattern
 					encompasses another pattern, it should be before in the
					array. All patterns must have {begin} and {transform}, but
					{end} is optional; where missing, it will just call
					{transform} with {begin}. */
int TextMatch(struct Text *const this, const struct TextPattern *const patterns,
	const size_t patterns_size) {
	struct TextPattern *pattern; size_t p;
	char *s0, *b;
	struct Match {
		struct TextPattern *pattern;
		struct Expression { char *s0, *s1; } exp0, exp1;
	} match;
	struct Replace { int is; char *pos; char stored; } replace = { 0, 0, 0 };

	if(!this) return 0;

	b = this->value;
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
		debug(this, "TextMatch", "match: \"%.*s\" \"%.30s...\" \"%.*s\"\n",
			(int)(match.exp0.s1 - match.exp0.s0), match.exp0.s0,
			match.exp0.s1,
			(int)(match.exp1.s1 - match.exp1.s0), match.exp1.s0);
		/* allocate the recursion; set the value back to how it was */
		if(this->childs_size >= this->childs_capacity[0]
			&& !childs_capacity_up(this)) return 0;
		if(!(this->childs[this->childs_size++] = Text_string_recursive(this,
			anonymous_volatile_key(), match.exp0.s1, -1,
			(size_t)(match.exp0.s0 - this->value),
			(size_t)(match.exp1.s1 - this->value))))
			{ *replace.pos = replace.stored; return 0; }
		*replace.pos = replace.stored;
		/* call pattern function on the duplicated substring */
		match.pattern->transform(this->childs[this->childs_size - 1]);
		/*printf("now value \"%.40s..\" and first \"%s\" at \"%.40s..\".\n",
			this->value, first_pat ? first_pat->begin : "(null)", first_pos);*/
		b = match.exp1.s1;
	} while(b);

	return -1;
}

/** White-space trims the value associated with {this}. */
void TextTrim(struct Text *const this) {
	char *str, *a, *z;

	if(!this) return;
	str = this->value;
	z = str + strlen(str) - 1, a = str;
	while(z > str && isspace(*z)) z--;
	z++, *z = '\0';
	while(isspace(*a)) a++;
	if(a - str) memmove(str, a, (size_t)(z - a + 1));
}

/** Manually make a new child out of {key_begin -> value_begin} having a length
 of {key_length} and {value_length}, respectfully. The elements are copied from
 this string, so they can be overlapping.
 @param key_begin	Can be null, in which case {key_length} is ignored and the
					key will have an anonymous key.
 @return	Success.
 @throws	E_PARAMETER */
struct Text *TextNewChild(struct Text *const this,
	char *const key, const size_t key_length,
	char *const value, const size_t value_length) {
	int is_key_dup = 0, is_within = 0;
	char *kcpy = 0, *vcpy = 0;
	struct Text *child = 0;
	struct Replace { int is; char *pos; char stored; } replace = { 0, 0, 0 };
	enum { NC_NO_ERR, NC_ERRNO, NC_EXTERIOR } error = NC_NO_ERR;

	if(!this) return 0;
	if(value >= this->value
		&& value + value_length <= this->value + strlen(this->value))
		is_within = -1;
	/*	{ this->error = E_PARAMETER; return 0; }*/

	do { /* try */
		/* lazy -- throw memory at it -- key */
		if(!key) {
			kcpy = anonymous_volatile_key();
		} else {
			replace.is = -1, replace.pos = key + key_length,
				replace.stored = *replace.pos, *replace.pos = '\0';
			if(!(kcpy = strdup(key))) { error = NC_ERRNO; break; }
			is_key_dup = -1;
			*replace.pos = replace.stored, replace.is = 0;
		}
		/* value */
		replace.is = -1, replace.pos = value + value_length,
			replace.stored = *replace.pos, *replace.pos = '\0';
		if(!(vcpy = strdup(value))) { error = NC_ERRNO; break; }
		*replace.pos = replace.stored, replace.is = 0;

		/* allocate the recursion; set the value back to how it was */
		if((this->childs_size >= this->childs_capacity[0]
			&& !childs_capacity_up(this))
			|| !(child = Text_string_recursive(this, kcpy, vcpy, is_within,
			(size_t)(is_within ? (value - this->value) : 0),
			(size_t)(is_within ? (value + value_length - this->value) : 0))))
			{ error = NC_EXTERIOR; break; }
		this->childs[this->childs_size++] = child;

	} while(0);

	{ /* finally */
		if(replace.is) *replace.pos = replace.stored;
		if(is_key_dup) free(kcpy);
		free(vcpy);
	}

	switch(error) { /* catch */
		case NC_NO_ERR: break;
		case NC_ERRNO: this->error = E_ERRNO, this->errno_copy = errno; break;
		case NC_EXTERIOR: break; /* already set error */
	}

	return child;
}

/** Ensures that we have a value of at least {capacity}. All pointers to the
 value will potentially change.
 @return	Success.
 @throws	E_OVERFLOW, E_ERRNO */
int TextEnsureCapacity(struct Text *const this, const size_t capacity) {
	if(!this) return 0;
	return value_capacity_up(this, &capacity);
}

/** E ch, e: p(e.key) -> a(e.value) */
void TextForEachTrue(struct Text *const this, TextPredicate p, TextAction a) {
	struct Text *child;
	unsigned i;
	for(i = 0; i < this->childs_size; i++) {
		child = this->childs[i];
		if(!p || p(child)) a(child);
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

static void to_string_recursive(struct Text *const this,
	struct SuperCat *cat_ptr) {
	unsigned i;
	char key[80];
	if(snprintf(key, sizeof key, "%s", this->value) > (int)sizeof key) {
		const unsigned key_size = sizeof key;
		key[key_size - 4] = '.',key[key_size - 3] = '.',key[key_size - 2] = '.';
	}
	super_cat(cat_ptr, start_str);
	super_cat(cat_ptr, this->key);
	super_cat(cat_ptr, ass_str);
	super_cat(cat_ptr, key);
	for(i = 0; i < this->childs_size; i++) {
		to_string_recursive(this->childs[i], cat_ptr);
	}
	super_cat(cat_ptr, end_str);
}

/** Prints the Text in a static value; one can print 4 things at once before
 it overwrites. */
char *TextToString(struct Text *const this) {
	static char value[4][2048];
	static int value_i;
	struct SuperCat cat;
	super_cat_init(&cat, value[value_i], sizeof *value / sizeof **value);
	value_i++, value_i &= 3;
	if(!this) {
		super_cat(&cat, null_str);
		return cat.print;
	}
	to_string_recursive(this, &cat);
	return cat.print;
}

/** Shortcut to add something to the text when editing it.
 @param fmt	Accepts %% as '%' and %s as the original string.
 @return	The new address of the string, which may have changed. */
char *TextAdd(struct Text *const this, char *const fmt) {
	char *str, *copy, *p;
	size_t str_len, copy_len = 0, copy_size;

	if(!this) return 0;
	str = this->value, str_len = strlen(str);
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
	if(!value_capacity_up(this, &copy_size)) { free(copy); return 0; };
	/* new string */
	str = this->value;
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
	return this->value;
}

void TextCut(struct TextCut *const r, char *const cut) {
	if(!r || !cut || r->is) return;
	r->is = -1;
	r->pos = cut;
	r->stored = *cut;
	*cut = '\0';
}

void TextUncut(struct TextCut *const r) {
	if(!r || !r->is) return;
	r->is = 0;
	*r->pos = r->stored;
}



/************
 * Private. */

/** Allocates a child under {parent} with key-value {key}->{value} going from
 {parent_begin} to {parent_end} in the origional string; the key-value does not
 have to be the same size as the length of the string or come from the value
 at all. */
static struct Text *Text_string_recursive(struct Text *const parent,
	const char *const key, char *const value, const int is_value_in_parent,
	const size_t parent_begin, const size_t parent_end) {
	struct Text *this;
	size_t str_size;

	if(!value) return 0;

	str_size = strlen(value) + 1; /* danger */

	if(parent && is_value_in_parent && str_size > parent_end - parent_begin+1) {
		parent->error = E_ASSERT;
		return 0;
	}
	/*printf("Text_string_rec: parent %s, parent_begin %lu, parent_end %lu, \"%s\".\n",
		TextToString(parent), parent_begin, parent_end, str);*/

	if(!(this = Text(key)))
		{ parent->error = E_ERRNO, parent->errno_copy = errno; return 0; }

	this->parent          = parent;
	this->is_value_in_parent = is_value_in_parent ? -1       : 0;
	this->parent_begin    = is_value_in_parent ? parent_begin : 0;
	this->parent_end      = is_value_in_parent ? parent_end   : 0;
	this->value_capacity[0] += str_size; /* danger */
	this->value_capacity[1] += str_size; /* danger */

	if(!(this->value=malloc(this->value_capacity[0] * sizeof *this->value))){
		parent->error = E_ERRNO, parent->errno_copy = errno;
		Text_(&this);
		return 0;
	}
	memcpy(this->value, value, str_size);
	/*printf("recursive: \"%s\"\n", this->value);*/

	return this;
}

/** Ensures value capacity.
 @param size_ptr	Can be null, in which case the capacity increases one level
					in size. If it is not, the capacity increases at or beyond
					this number.
 @return	Success.
 @throws	E_OVERFLOW, E_ERRNO */
static int value_capacity_up(struct Text *const this,
	const size_t *const size_ptr) {
	size_t c0, c1;
	char *value;

	/* fibonacci */
	c0 = this->value_capacity[0];
	c1 = this->value_capacity[1];
	while(!size_ptr || c0 < *size_ptr) {
		if(c0 == (size_t)-1) { this->error = E_OVERFLOW; return 0; }
		c0 ^= c1;
		c1 ^= c0;
		c0 ^= c1;
		c1 += c0;
		if(c1 <= c0) c1 = (size_t)-1;
		if(!size_ptr) break;
	}
	if(this->value_capacity[0] >= c0) return -1; /* it's already that size */
	debug(this, "value_capacity_up","%lu->%lu.\n",this->value_capacity[0],c0);
	if(!(value = realloc(this->value, c0 * sizeof *this->value))) {
		this->error = E_ERRNO, this->errno_copy = errno;
		return 0;
	}
	this->value = value;
	this->value_capacity[0] = c0;
	this->value_capacity[1] = c1;

	return -1;
}

/** Ensures childs capacity.
 @return	Success.
 @throws	E_OVERFLOW, E_ERRNO */
static int childs_capacity_up(struct Text *const this) {
	size_t c0, c1;
	struct Text **childs;

	/* fibonacci */
	c0 = this->childs_capacity[0];
	c1 = this->childs_capacity[1];
	if(c0 == (size_t)-1) { this->error = E_OVERFLOW; return 0; }
	c0 ^= c1;
	c1 ^= c0;
	c0 ^= c1;
	c1 += c0;
	if(c1 <= c0) c1 = (size_t)-1;
	debug(this, "childs_capacity_up", "%lu->%lu.\n", this->childs_capacity[0],c0);
	if(!(childs = realloc(this->childs, c0 * sizeof *this->childs))) {
		this->error = E_ERRNO, this->errno_copy = errno;
		return 0;
	}
	this->childs = childs;
	this->childs_capacity[0] = c0;
	this->childs_capacity[1] = c1;

	return -1;
}

static char *anonymous_volatile_key(void) {
	static char anon[64];
	static unsigned i = 0;
	sprintf(anon, "_nemo%u", ++i);
	return anon;
}

/** Private debug messages from list functions; turn on using {LIST_DEBUG}. */
static void debug(struct Text *const this, const char *const fn,
	const char *const fmt, ...) {
#ifdef TEXT_DEBUG
	va_list parg;

	va_start(parg, fmt);
	fprintf(stderr, "Text<%s>#%p.%s: ", this->key, (void *)this, fn);
	vfprintf(stderr, fmt, parg);
	va_end(parg);
#else
	_UNUSED(this); _UNUSED(fn); _UNUSED(fmt);
#endif
}
