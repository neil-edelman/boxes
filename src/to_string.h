/* @license 2020 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle To String Trait

 The inclusion must define an iterator, ITERATE, ITERATE_BOX, ITERATE_TYPE,
 ITERATE_BEGIN, and ITERATE_NEXT.

 @param[S_]
 Function-like define macro accepting one argument and producing a valid name.
 Defines `PS_` to be private.

 @param[TO_STRING]
 Function implementing <typedef:<PS>to_string_fn>.

 @param[TO_STRING_LEFT, TO_STRING_RIGHT]
 7-bit characters, defaults to '(' and ')'.

 @std C89 */

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
#ifndef S_
#error To string: macro S_ undefined.
#endif
#ifdef PS_
#error To string: PS_ can not be defined.
#endif
#ifndef TO_STRING_LEFT
#define TO_STRING_LEFT '('
#endif
#ifndef TO_STRING_RIGHT
#define TO_STRING_RIGHT ')'
#endif

#define PS_(thing) CAT(to_string, S_(thing))

typedef ITERATE PS_(iterator);
typedef ITERATE_BOX PS_(box);
typedef ITERATE_TYPE PS_(type);
typedef void (*PS_(begin_fn))(PS_(iterator) *, const PS_(box) *);
typedef const PS_(type) *(*PS_(next_fn))(PS_(iterator) *);
/** Responsible for turning the first argument into a 12-`char` null-terminated
 output string. */
typedef void (*PS_(to_string_fn))(const PS_(type) *, char (*)[12]);

static const PS_(begin_fn) PS_(begin) = (ITERATE_BEGIN);
static const PS_(next_fn) PS_(next) = (ITERATE_NEXT);
/* Check that `TO_STRING` is a function implementing <typedef:<PS>to_string>. */
static const PS_(to_string_fn) PS_(to_string) = (TO_STRING);

/** @return Print the contents of `box` in a static string buffer of 256
 bytes with limitations of only printing 4 things at a time.
 @order \Theta(1) @allow */
static const char *S_(to_string)(const PS_(box) *const box) {
	const char comma = ',', space = ' ', *const ellipsis = "…",
		left = TO_STRING_LEFT, right = TO_STRING_RIGHT;
	const size_t ellipsis_len = strlen(ellipsis);
	char *const buffer = to_string_buffers[to_string_buffer_i++], *b = buffer;
	size_t advance, size;
	const PS_(type) *x;
	PS_(iterator) it;
	int is_sep = 0;
	/* Minimum size: "(" "XXXXXXXXXXX" "," "…" ")" "\0". */
	assert(box && !(to_string_buffers_no & (to_string_buffers_no - 1))
		&& to_string_buffer_size >= 1 + 11 + 1 + ellipsis_len + 1 + 1);
	/* Advance the buffer for next time. */
	to_string_buffer_i &= to_string_buffers_no - 1;
	/* Begin iteration. */
	PS_(begin)(&it, box);
	*b++ = left;
	while(x = PS_(next)(&it)) {
		PS_(to_string)(x, (char (*)[12])b);
		/* Paranoid about '\0'. */
		for(advance = 0; *b != '\0' && advance < 11; b++, advance++);
		is_sep = 1, *b++ = comma, *b++ = space;
		/* Greedy typesetting: enough for "XXXXXXXXXXX" "," "…" ")" "\0". */
		if((size = (size_t)(b - buffer))
			> to_string_buffer_size - 11 - 1 - ellipsis_len - 1 - 1)
			{ if(PS_(next)(&it)) goto ellipsis; else break; }
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
	assert(b - buffer <= to_string_buffer_size);
	return buffer;
}

static void PS_(unused_to_string_coda)(void);
static void PS_(unused_to_string)(void)
	{ S_(to_string)(0); PS_(unused_to_string_coda)(); }
static void PS_(unused_to_string_coda)(void) { PS_(unused_to_string)(); }

#undef PS_
#undef TO_STRING
#undef TO_STRING_LEFT
#undef TO_STRING_RIGHT
