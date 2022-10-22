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

#if !defined(BOX_TYPE) || !defined(BOX_CONTENT) || !defined(BOX_) \
	|| !defined(BOX_MAJOR_NAME) || !defined(BOX_MINOR_NAME) \
	|| defined(STR_) || defined(STRLABEL_)
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
#error Unexpected preprocessor symbols.
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

#ifndef BOX_TRAIT_NAME /* <!-- !trait */
#define STR_(n) TO_STRING_CAT(TO_STRING_CAT(BOX_MINOR_NAME, BOX_MAJOR_NAME), n)
#define STRLABEL_(n) TO_STRING_CAT(BOX_MINOR_NAME, n)
#else /* !trait --><!-- trait */
#define STR_(n) TO_STRING_CAT(TO_STRING_CAT(BOX_MINOR_NAME, BOX_MAJOR_NAME), \
	TO_STRING_CAT(BOX_TRAIT_NAME, n))
#define STRLABEL_(n) TO_STRING_CAT(TO_STRING_CAT(BOX_MINOR_NAME, \
	BOX_TRAIT_NAME), n)
#endif /* trait --> */

typedef BOX_TYPE PSTR_(box);
typedef BOX_CONTENT PSTR_(element);

/** <src/to_string.h>: responsible for turning the read-only argument into a
 12-`char` null-terminated output string. The first argument should be a
 read-only reference to an element and the second a pointer to the bytes. */
typedef void (*PSTR_(to_string_fn))(const PSTR_(element), char (*)[12]);
/* _Nb_: this is for documentation only; there is no way to get a general
 read-only type which what we are supplied. Think of nested pointers. */

/** <src/to_string.h>: print the contents of `box` in a static string buffer of
 256 bytes, with limitations of only printing 4 things at a time. `<STR>` is
 `<NAME>_<BOX>[_<TRAIT>]`. Requires `<NAME>[_<TRAIT>]_to_string` be declared as
 a <typedef:<PSTR>to_string_fn>.
 @return Address of the static buffer. @order \Theta(1) @allow */
static const char *STR_(to_string)(const PSTR_(box) *const box) {
	const char comma = ',', space = ' ', ellipsis[] = "…",
		left = TO_STRING_LEFT, right = TO_STRING_RIGHT;
	const size_t ellipsis_len = sizeof ellipsis - 1;
	char *const buffer = to_string_buffers[to_string_buffer_i++], *b = buffer;
	size_t advance;
	PSTR_(element) x;
	struct BOX_(iterator) it;
	int is_sep = 0;
	/* Minimum size: "(" "XXXXXXXXXXX" "," "…" ")" "\0". */
	assert(box && !(to_string_buffers_no & (to_string_buffers_no - 1))
		&& to_string_buffer_size >= 1 + 11 + 1 + ellipsis_len + 1 + 1);
	/* Advance the buffer for next time. */
	to_string_buffer_i &= to_string_buffers_no - 1;
	{ /* We do not modify `box`, but the compiler doesn't know that. */
		PSTR_(box) *promise_box;
		memcpy(&promise_box, &box, sizeof box);
		it = BOX_(iterator)(promise_box);
	}
	*b++ = left;
	while(BOX_(is_element)(x = BOX_(next)(&it))) {
		/* One must have this function declared! */
		STRLABEL_(to_string)(x, (char (*)[12])b);
		/* Paranoid about '\0'; wastes 1 byte of 12, but otherwise confusing. */
		for(advance = 0; *b != '\0' && advance < 11; b++, advance++);
		is_sep = 1, *b++ = comma, *b++ = space;
		/* Greedy typesetting: enough for "XXXXXXXXXXX" "," "…" ")" "\0". */
		if((size_t)(b - buffer)
			> to_string_buffer_size - 11 - 1 - ellipsis_len - 1 - 1) {
			if(BOX_(is_element)(BOX_(next)(&it))) goto ellipsis;
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

static void PSTR_(unused_to_string_coda)(void);
static void PSTR_(unused_to_string)(void)
	{ STR_(to_string)(0); PSTR_(unused_to_string_coda)(); }
static void PSTR_(unused_to_string_coda)(void) { PSTR_(unused_to_string)(); }

#undef STR_
#undef STRLABEL_
#ifdef TO_STRING_EXTERN
#undef TO_STRING_EXTERN
#endif
#ifdef TO_STRING_INTERN
#undef TO_STRING_INTERN
#endif
#undef TO_STRING_LEFT
#undef TO_STRING_RIGHT
