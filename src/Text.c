/** Copyright 2017 Neil Edelman, distributed under the terms of the MIT License;
 see readme.txt, or \url{ https://opensource.org/licenses/MIT }.

 Gets the whole text file and has basic functions for parsing.

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



/* for allocating buffer */
static const unsigned fibonacci11 = 89;
static const unsigned fibonacci12 = 144;

enum Error {
	E_NO_ERROR,
	E_ERRNO,
	E_PARAMETER,
	E_OVERFLOW
};

static const char *const error_explination[] = {
	"no error",
	0,
	"parameter out-of-range",
	"overflow"
};

/* globals */

static enum Error global_error = E_NO_ERROR;
static int        global_errno_copy;

struct Text {
	char *filename;
	char *buffer;
	size_t size, capacity[2];
	enum Error error;
	int errno_copy;
};

/* private */

static int up_capacity(struct Text *const this);
static void debug(struct Text *const this, const char *const fn,
	const char *const fmt, ...);



/**********
 * Public */

/** Constructs an empty {Text} out of the filename, {fn}.
 @fixme		Assumes 8-bit encoding. */
struct Text *Text(char *const fn) {
	struct Text *this;
	FILE *fp;
	size_t fn_size, cursor = 0;
	int size, err;

	/* param check */
	if(!fn || !fn[0]) {
		global_error = E_PARAMETER;
		return 0;
	}
	fn_size = strlen(fn) + 1;

	/* allocate */
	if(!(this = malloc(sizeof *this + fn_size))) {
		global_error = E_ERRNO;
		global_errno_copy = errno;
		return 0;
	}

	/* fill */
	this->filename    = (char *)(this + 1);
	memcpy(this->filename, fn, fn_size);
	this->buffer      = 0;
	this->size        = 0;
	this->capacity[0] = fibonacci11;
	this->capacity[1] = fibonacci12;
	this->error       = E_NO_ERROR;
	this->errno_copy  = 0;

	/* open */
	if(!(fp = fopen(fn, "r"))) {
		global_error = E_ERRNO;
		global_errno_copy = errno;
		Text_(&this);
		return 0;
	}

	debug(this, "cons", "new, capacity %d, opened \"%s.\"\n",
		this->capacity[0], fn);

	/* allocate buffer; there is no real way to portably and correctly get the
	 number of characters in a text file, so allocate incrementally */
	if(!(this->buffer = malloc(this->capacity[0] * sizeof *this->buffer))) {
		Text_(&this);
		global_error = E_ERRNO;
		global_errno_copy = errno;
		if(fclose(fp) == EOF) perror(fn); /* unreported error */
		Text_(&this);
		return 0;
	}

	/* read */
	while(size = (this->capacity[0] - cursor > INT_MAX) ?
		INT_MAX : (int)(this->capacity[0] - cursor),
		fgets(this->buffer + cursor, size, fp)) {
		cursor += strlen(this->buffer + cursor);
		if(cursor >= this->capacity[0] - 1 && !up_capacity(this)) {
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

/** Destructs a {Text}. */
void Text_(struct Text **const this_ptr) {
	struct Text *this;

	if(!this_ptr || !(this = *this_ptr)) return;

	debug(this, "~", "erase \"%s.\"\n", this->filename);

	free(this->buffer);
	free(this);

	*this_ptr = 0;
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



/************
 * Private. */

/** Ensures buffer capacity.
 @return	Success.
 @throws	E_OVERFLOW, E_ERRNO */
static int up_capacity(struct Text *const this) {
	size_t c0, c1;
	void *buffer;

	/* fibonacci */
	c0 = this->capacity[0];
	c1 = this->capacity[1];
	if(c0 == (size_t)-1) { this->error = E_OVERFLOW; return 0; }
	c0 ^= c1;
	c1 ^= c0;
	c0 ^= c1;
	c1 += c0;
	if(c1 <= c0) c1 = (size_t)-1;
	debug(this, "up_capacity", "capacity %lu->%lu.\n", this->capacity[0], c0);
	if(!(buffer = realloc(this->buffer, c0 * sizeof *this->buffer))) {
		this->error = E_ERRNO, this->errno_copy = errno;
		return 0;
	}
	this->buffer = buffer;
	this->capacity[0] = c0;
	this->capacity[1] = c1;

	return -1;
}

/** Private debug messages from list functions; turn on using {LIST_DEBUG}. */
static void debug(struct Text *const this, const char *const fn,
	const char *const fmt, ...) {
#ifdef LIST_DEBUG
	va_list parg;
	char scratch[9];

	va_start(parg, fmt);
	fprintf(stderr, "Text<%s>#%p.%s: ", this->fn, (void *)this, fn);
	vfprintf(stderr, fmt, parg);
	va_end(parg);
#else
	_UNUSED(this); _UNUSED(fn); _UNUSED(fmt);
#endif
}
