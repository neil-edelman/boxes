/* @license 2020 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle To string trait

 Interface defined by box. Requires `<NAME>[_<TRAIT>]_to_string` be declared as
 a <typedef:<PSTR>to_string_fn>.

 @param[TO_STRING_LEFT, TO_STRING_RIGHT]
 7-bit characters, defaults to '(' and ')'.

 @param[TO_STRING_EXTERN, TO_STRING_INTERN]
 Normally the space to put the temporary strings is static, one per file. With
 this, it's possible to have a global storage to save space: have one file have
 `TO_STRING_INTERN` as the first box, the other files `TO_STRING_EXTERN`. This
 is unsynchronized.

 @fixme `extern` untested.

 @std C89 */

#if !defined(BOX_H) || !defined(BOX_CAT) || !defined(TU_) || !defined(PTU_) \
	|| !defined(BOX_MAJOR_NAME) || !defined(BOX_MAJOR) \
	|| !defined(BOX_NAME) || !defined(BOX_MINOR)
#	error Unexpected preprocessor symbols.
#endif
#if defined(TO_STRING_H) \
	&& (defined(TO_STRING_EXTERN) || defined(TO_STRING_INTERN))
#	error Should be the on the first to_string in the compilation unit.
#else
#	if defined(TO_STRING_EXTERN) && defined(TO_STRING_INTERN)
#		error These can not be defined together.
#	endif
#endif

#ifndef TO_STRING_H /* <!-- idempotent */
#	define TO_STRING_H
#	include <string.h>
#	if defined(TO_STRING_EXTERN) || defined(TO_STRING_INTERN) /* <!-- ntern */
extern char to_string_buffers[4][256];
extern const unsigned to_string_buffers_no;
extern unsigned to_string_i;
#		ifdef TO_STRING_INTERN
char to_string_buffers[4][256];
const unsigned to_string_buffers_no = sizeof to_string_buffers
	/ sizeof *to_string_buffers, to_string_buffer_size
	= sizeof *to_string_buffers / sizeof **to_string_buffers;
unsigned to_string_buffer_i;
#		endif
#	else /* ntern --><!-- static */
static char to_string_buffers[4][256];
static const unsigned to_string_buffers_no = sizeof to_string_buffers
	/ sizeof *to_string_buffers, to_string_buffer_size
	= sizeof *to_string_buffers / sizeof **to_string_buffers;
static unsigned to_string_buffer_i;
#	endif /* static --> */
#endif /* idempotent --> */

#ifndef TO_STRING_LEFT
#define TO_STRING_LEFT '('
#endif
#ifndef TO_STRING_RIGHT
#define TO_STRING_RIGHT ')'
#endif

#if 0 /* <!-- documentation. */
/** <src/to_string.h>: responsible for turning the read-only argument into a
 12-`char` null-terminated output string, passed as a pointer in the last
 argument. This function can have 2 or 3 arguments, where `<PSTR>element` might
 be a map with a key-value pair.  */
typedef void (*PTU_(to_string_fn))(const PT_(type) *, char (*)[12]);
#endif /* documentation --> */

/** <src/to_string.h>: print the contents of `box` in a static string buffer of
 256 bytes, with limitations of only printing 4 things in a single sequence
 point. @return Address of the static buffer. @order \Theta(1) @allow */
static const char *TU_(to_string)(const PT_(box) *const box) {
	const char comma = ',', space = ' ', ellipsis[] = "…",
		left = TO_STRING_LEFT, right = TO_STRING_RIGHT;
	const size_t ellipsis_len = sizeof ellipsis - 1;
	char *const buffer = to_string_buffers[to_string_buffer_i++], *b = buffer;
	size_t advance;
	struct T_(cursor) cur;
	int is_sep = 0;
	/* Minimum size: "(" "XXXXXXXXXXX" "," "…" ")" "\0". */
	assert(box && !(to_string_buffers_no & (to_string_buffers_no - 1))
		&& to_string_buffer_size >= 1 + 11 + 1 + ellipsis_len + 1 + 1);
	/* Advance the buffer for next time. */
	to_string_buffer_i &= to_string_buffers_no - 1;
	{ /* We do not modify `box`, but the compiler doesn't know that. */
		/* fixme: I think I know how to do it. */
		PT_(box) *promise_box;
		memcpy(&promise_box, &box, sizeof box);
		cur = T_(begin)(promise_box);
	}
	*b++ = left;
	for( ; T_(cursor_exists)(&cur); T_(cursor_next)(&cur)) {
		tu_(to_string)(T_(cursor_look)(&cur), (char (*)[12])b);
		/* Paranoid about '\0'; wastes 1 byte of 12, but otherwise confusing. */
		for(advance = 0; *b != '\0' && advance < 11; b++, advance++);
		is_sep = 1, *b++ = comma, *b++ = space;
		/* Greedy typesetting: enough for "XXXXXXXXXXX" "," "…" ")" "\0". */
		if((size_t)(b - buffer) > to_string_buffer_size - 11 - 1
			- ellipsis_len - 1 - 1) {
			if(T_(cursor_next)(&cur), T_(cursor_exists)) goto ellipsis;
			else break;
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
	assert(b - buffer <= to_string_buffer_size);
	return buffer;
}

static void PTU_(unused_to_string_coda)(void);
static void PTU_(unused_to_string)(void)
	{ TU_(to_string)(0); PTU_(unused_to_string_coda)(); }
static void PTU_(unused_to_string_coda)(void) { PTU_(unused_to_string)(); }

#ifdef TO_STRING_EXTERN
#	undef TO_STRING_EXTERN
#elifdef TO_STRING_INTERN
#	undef TO_STRING_INTERN
#endif
#undef TO_STRING_LEFT
#undef TO_STRING_RIGHT
