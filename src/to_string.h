/* @license 2020 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle To string trait

 Interface (defined by `BOX_`, `BOX`, and `BOX_FORWARD`):

 \* `int <BOX>is_element(<PSZ>element_c)`;
 \* `struct <BOX>forward`;
 \* `struct <BOX>forward <BOX>forward_begin(const <PSZ>box *)`;
 \* `<PSZ>element_c <BOX>forward_next(struct <BOX>iterator *)`.

 @param[STR_(n)]
 A one-argument macro producing a name that is responsible for the name of the
 to string function. Should be something like
 `STR_(to_string) -> widget_foo_to_string`. The caller is responsible for
 undefining `STR_`.

 @param[TO_STRING]
 Function implementing <typedef:<PZ>to_string_fn>.

 @param[TO_STRING_LEFT, TO_STRING_RIGHT]
 7-bit characters, defaults to '(' and ')'.

 @param[TO_STRING_EXTERN, TO_STRING_INTERN]
 Normally the space to put the temporary strings is static, one per file. With
 this, it's possible to have a global storage to save space: have one file have
 `TO_STRING_INTERN` as the first box, the other files `TO_STRING_EXTERN`. This
 is unsynchronized.

 @fixme `extern` untested.

 @std C89 */

#if !defined(BOX_) || !defined(BOX) || !defined(BOX_FORWARD) \
	|| !defined(STR_) || !defined(TO_STRING)
#error Unexpected preprocessor symbols.
#endif

#if defined(TO_STRING_H) \
	&& (defined(TO_STRING_EXTERN) || defined(TO_STRING_INTERN)) /* <!-- not */
#error Should be the on the first to_string in the compilation unit.
#else /* not --><!-- !not */
#if defined(TO_STRING_EXTERN) && defined(TO_STRING_INTERN) /* <!-- two */
#error These can not be defined together.
#endif /* two --> */
#endif /* !not --> */

#ifndef TO_STRING_H /* <!-- idempotent */
#define TO_STRING_H
#include <string.h>
#if defined(TO_STRING_CAT_) || defined(TO_STRING_CAT) || defined(PSTR_)
#error Unexpected defines.
#endif
/* <Kernighan and Ritchie, 1988, p. 231>. */
#define TO_STRING_CAT_(n, m) n ## _ ## m
#define TO_STRING_CAT(n, m) TO_STRING_CAT_(n, m)
#define PSTR_(n) TO_STRING_CAT(to_string, STR_(n))
#if defined(TO_STRING_EXTERN) || defined(TO_STRING_INTERN) /* <!-- ntern */
extern char to_string_buffers[4][256];
extern const unsigned to_string_buffers_no;
extern unsigned to_string_i;
#ifdef TO_STRING_INTERN /* <!-- intern */
char to_string_buffers[4][256];
const unsigned to_string_buffers_no = sizeof to_string_buffers
	/ sizeof *to_string_buffers, to_string_buffer_size
	= sizeof *to_string_buffers / sizeof **to_string_buffers;
unsigned to_string_buffer_i;
#endif /* intern --> */
#else /* ntern --><!-- static */
static char to_string_buffers[4][256];
static const unsigned to_string_buffers_no = sizeof to_string_buffers
	/ sizeof *to_string_buffers, to_string_buffer_size
	= sizeof *to_string_buffers / sizeof **to_string_buffers;
static unsigned to_string_buffer_i;
#endif /* static --> */
#endif /* idempotent --> */

#ifndef TO_STRING_LEFT
#define TO_STRING_LEFT '('
#endif
#ifndef TO_STRING_RIGHT
#define TO_STRING_RIGHT ')'
#endif

typedef BOX PSTR_(box);
typedef const BOX_FORWARD PSTR_(element_c);

/** <src/to_string.h>: responsible for turning the argument into a 12-`char`
 null-terminated output string. */
typedef void (*PSTR_(to_string_fn))(PSTR_(element_c), char (*)[12]);
/* Check that `TO_STRING` is a function implementing
 <typedef:<PSZ>to_string>. */
static const PSTR_(to_string_fn) PSTR_(to_string) = (TO_STRING);

/** <src/to_string.h>: print the contents of `box` in a static string buffer of
 256 bytes, with limitations of only printing 4 things at a time. `<SZ>` is
 loosely contracted to be a name `<X>box[<X_TO_STRING_NAME>]`.
 @return Address of the static buffer. @order \Theta(1) @allow */
static const char *STR_(to_string)(const PSTR_(box) *const box) {
	const char comma = ',', space = ' ', *const ellipsis = "…",
		left = TO_STRING_LEFT, right = TO_STRING_RIGHT;
	const size_t ellipsis_len = sizeof ellipsis - 1;
	char *const buffer = to_string_buffers[to_string_buffer_i++], *b = buffer;
	size_t advance, size;
	PSTR_(element_c) x;
	struct BOX_(forward) it;
	int is_sep = 0;
	/* Minimum size: "(" "XXXXXXXXXXX" "," "…" ")" "\0". */
	assert(box && !(to_string_buffers_no & (to_string_buffers_no - 1))
		&& to_string_buffer_size >= 1 + 11 + 1 + ellipsis_len + 1 + 1);
	/* Advance the buffer for next time. */
	to_string_buffer_i &= to_string_buffers_no - 1;
	it = BOX_(forward_begin)(box);
	*b++ = left;
	while(BOX_(is_element)(x = BOX_(forward_next)(&it))) {
		PSTR_(to_string)(x, (char (*)[12])b);
		/* Paranoid about '\0'. */
		for(advance = 0; *b != '\0' && advance < 11; b++, advance++);
		is_sep = 1, *b++ = comma, *b++ = space;
		/* Greedy typesetting: enough for "XXXXXXXXXXX" "," "…" ")" "\0". */
		if((size = (size_t)(b - buffer))
			> to_string_buffer_size - 11 - 1 - ellipsis_len - 1 - 1)
			if(BOX_(is_element)(BOX_(forward_next)(&it))) goto ellipsis;
			else break;
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

static void PSTR_(unused_to_string_coda)(void);
static void PSTR_(unused_to_string)(void)
	{ STR_(to_string)(0); PSTR_(unused_to_string_coda)(); }
static void PSTR_(unused_to_string_coda)(void) { PSTR_(unused_to_string)(); }

#undef TO_STRING
#ifdef TO_STRING_NAME
#undef TO_STRING_NAME
#endif
#ifdef TO_STRING_EXTERN
#undef TO_STRING_EXTERN
#endif
#ifdef TO_STRING_INTERN
#undef TO_STRING_INTERN
#endif
#undef TO_STRING_LEFT
#undef TO_STRING_RIGHT
