/* @license 2020 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle To String Trait

 The inclusion must define an iterator, ITERATE, ITERATE_BOX, ITERATE_TYPE,
 ITERATE_BEGIN, and ITERATE_NEXT, and,

 @param[A_]
 Function-like define macro accepting one argument and producing a valid name.
 Defines `PA_` to be private.

 @param[TO_STRING]
 Function implementing <typedef:<PA>to_string_fn>.

 @param[TO_STRING_LEFT, TO_STRING_RIGHT]
 7-bit characters, defaults to '(' and ')'.

 @std C89
 @fixme Traits separate documentation. */

#ifndef TO_STRING_H /* <!-- idempotent: for all in compilation unit. */
#define TO_STRING_H
#include <string.h>
static char to_string_buffers[4][256];
static const unsigned to_string_buffers_no = sizeof to_string_buffers
	/ sizeof *to_string_buffers, to_string_buffer_size
	= sizeof *to_string_buffers / sizeof **to_string_buffers;
static unsigned to_string_buffer_i;
#endif /* idempotent --> */

/* Check defines. */
#if !defined(CAT) || !defined(CAT_) || !defined(ITERATE) \
	|| !defined(ITERATE_BOX) || !defined(ITERATE_TYPE) \
	|| !defined(ITERATE_BEGIN) || !defined(ITERATE_NEXT)
#error To string: CAT_? or ITERATE* are undefined.
#endif
#ifndef A_
#error To string: macro A_ undefined.
#endif
#ifdef PA_
#error To string: PA_ can not be defined.
#endif
#ifndef TO_STRING_LEFT
#define TO_STRING_LEFT '('
#endif
#ifndef TO_STRING_RIGHT
#define TO_STRING_RIGHT ')'
#endif

#define PA_(thing) CAT(to_string, A_(thing))

typedef ITERATE PA_(iterator);
typedef ITERATE_BOX PA_(box);
typedef ITERATE_TYPE PA_(type);
typedef void (*PA_(begin_fn))(PA_(iterator) *, const PA_(box) *);
typedef PA_(type) *(*PA_(next_fn))(PA_(iterator) *);
/** Responsible for turning the first argument into a 12-`char` null-terminated
 output string. */
typedef void (*PA_(to_string_fn))(const PA_(type) *, char (*)[12]);

static const PA_(begin_fn) PA_(begin) = (ITERATE_BEGIN);
static const PA_(next_fn) PA_(next) = (ITERATE_NEXT);
/* Check that `TO_STRING` is a function implementing <typedef:<PA>to_string>. */
static const PA_(to_string_fn) PA_(to_string) = (TO_STRING);

/** @return Print the contents of `box` in a static string buffer of 256
 bytes with limitations of only printing 4 things at a time.
 @order \Theta(1) @allow */
static const char *A_(to_string)(const PA_(box) *const box) {
	const char comma = ',', space = ' ', *const ellipsis = "…",
		left = TO_STRING_LEFT, right = TO_STRING_RIGHT;
	const size_t ellipsis_len = strlen(ellipsis);
	char *const buffer = to_string_buffers[to_string_buffer_i++], *b = buffer;
	size_t advance, size;
	PA_(type) *x;
	PA_(iterator) it;
	int is_sep = 0;
	/* Minimum size: "(" "XXXXXXXXXXX" "," "…" ")" "\0". */
	assert(box && !(to_string_buffers_no & (to_string_buffers_no - 1))
		&& to_string_buffer_size >= 1 + 11 + 1 + ellipsis_len + 1 + 1);
	/* Advance the buffer for next time. */
	to_string_buffer_i &= to_string_buffers_no - 1;
	/* Begin iteration. */
	PA_(begin)(&it, box);
	*b++ = left;
	while((x = PA_(next)(&it))) {
		PA_(to_string)(x, (char (*)[12])b);
		/* Paranoid about '\0'. */
		for(advance = 0; *b != '\0' && advance < 11; b++, advance++);
		is_sep = 1, *b++ = comma, *b++ = space;
		/* Greedy typesetting: enough for "XXXXXXXXXXX" "," "…" ")" "\0". */
		if((size = b - buffer) > to_string_buffer_size
			- 11 - 1 - ellipsis_len - 1 - 1) {
			if(PA_(next)(&it)) goto ellipsis; else break;
		}
	}
	if(is_sep) b -= 2;
	*b++ = right;
	goto terminate;
ellipsis:
	b--;
	memcpy(b, ellipsis, ellipsis_len), b += ellipsis_len;
	*b++ = right;
terminate:
	*b++ = '\0';
	assert((size = b - buffer) <= to_string_buffer_size);
	return buffer;
}

static void PA_(unused_to_string_coda)(void);
static void PA_(unused_to_string)(void)
	{ A_(to_string)(0); PA_(unused_to_string_coda)(); }
static void PA_(unused_to_string_coda)(void) { PA_(unused_to_string)(); }

#undef PA_
#undef TO_STRING_LEFT
#undef TO_STRING_RIGHT
