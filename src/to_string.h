/* @license 2020 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle To string trait

 Interface (defined by `BOX_`):

 \* `int <BOX>is_cursor(<PSZ>cursor_c)` (defined by `BOX_CURSOR`);
 \* `struct <BOX>iterator`;
 \* `struct <BOX>iterator <BOX>begin(const <PSZ>box *)` (defined by `BOX`);
 \* `<PSZ>cursor_c <BOX>next(struct <BOX>iterator *)` (defined by `BOX_CURSOR`.)

 @param[SZ_(n)]
 A one-argument macro producing a name that is responsible for the name of the
 to string function. Should be something like
 `SZ_(to_string) -> widget_foo_to_string`. The caller is responsible for
 undefining `SZ_`.

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

#if !defined(BOX_) || !defined(BOX) || !defined(BOX_CURSOR) \
	|| !defined(SZ_) || !defined(TO_STRING)
#error Unexpected preprocessor symbols. Check that it is included it as a trait.
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
#if defined(TO_STRING_CAT_) || defined(TO_STRING_CAT) || defined(PSZ_)
#error Unexpected defines.
#endif
/* <Kernighan and Ritchie, 1988, p. 231>. */
#define TO_STRING_CAT_(n, m) n ## _ ## m
#define TO_STRING_CAT(n, m) TO_STRING_CAT_(n, m)
#define PSZ_(n) TO_STRING_CAT(to_string, SZ_(n))
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

typedef BOX PSZ_(box);
typedef const BOX_CURSOR PSZ_(cursor_c);

/** <src/to_string.h>: responsible for turning the argument into a 12-`char`
 null-terminated output string. */
typedef void (*PSZ_(to_string_fn))(PSZ_(cursor_c), char (*)[12]);
/* Check that `TO_STRING` is a function implementing
 <typedef:<PSZ>to_string>. */
static const PSZ_(to_string_fn) PSZ_(to_string) = (TO_STRING);

/** <src/to_string.h>: print the contents of `box` in a static string buffer of
 256 bytes, with limitations of only printing 4 things at a time. `<SZ>` is
 loosely contracted to be a name `<X>box[<X_TO_STRING_NAME>]`.
 @return Address of the static buffer. @order \Theta(1) @allow */
static const char *SZ_(to_string)(const PSZ_(box) *const box) {
	const char comma = ',', space = ' ', *const ellipsis = "…",
		left = TO_STRING_LEFT, right = TO_STRING_RIGHT;
	const size_t ellipsis_len = sizeof ellipsis - 1;
	char *const buffer = to_string_buffers[to_string_buffer_i++], *b = buffer;
	size_t advance, size;
	PSZ_(cursor_c) x;
	PSZ_(box) *promise_box;
	struct BOX_(iterator) it;
	int is_sep = 0;
	/* Minimum size: "(" "XXXXXXXXXXX" "," "…" ")" "\0". */
	assert(box && !(to_string_buffers_no & (to_string_buffers_no - 1))
		&& to_string_buffer_size >= 1 + 11 + 1 + ellipsis_len + 1 + 1);
	/* Advance the buffer for next time. */
	to_string_buffer_i &= to_string_buffers_no - 1;
	/* We want iteration to modify the values sometimes, so it is theoretically
	 possible to modify box, but we just don't. Promise. */
	memcpy(&promise_box, &box, sizeof box), assert(box == promise_box);
	it = BOX_(begin)(promise_box);
	*b++ = left;
	while(BOX_(is_cursor)(x = BOX_(next)(&it))) {
		PSZ_(to_string)(x, (char (*)[12])b);
		/* Paranoid about '\0'. */
		for(advance = 0; *b != '\0' && advance < 11; b++, advance++);
		is_sep = 1, *b++ = comma, *b++ = space;
		/* Greedy typesetting: enough for "XXXXXXXXXXX" "," "…" ")" "\0". */
		if((size = (size_t)(b - buffer))
			> to_string_buffer_size - 11 - 1 - ellipsis_len - 1 - 1)
			{ if(BOX_(is_cursor)(BOX_(next)(&it))) goto ellipsis;
			else break; }
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

static void PSZ_(unused_to_string_coda)(void);
static void PSZ_(unused_to_string)(void)
	{ SZ_(to_string)(0); PSZ_(unused_to_string_coda)(); }
static void PSZ_(unused_to_string_coda)(void) { PSZ_(unused_to_string)(); }

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
