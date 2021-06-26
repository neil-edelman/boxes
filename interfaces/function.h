/* @license 2021 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Function Interface

 A trait relying on the interface <iterate.h>.

 @param[Z_]
 A one-argument macro producing a name that is responsible for the name of the
 to string function. Does not undefine.

 @param[ZI_]
 A one-argument macro producing a name that is the same as has been previously
 been called on as `I_` on <iterate.h>. This is responsible for the order.

 @param[TO_STRING]
 Function implementing <typedef:<PZ>to_string_fn>.

 @param[TO_STRING_LEFT, TO_STRING_RIGHT]
 7-bit characters, defaults to '(' and ')'.

 @param[TO_STRING_EXTERN, TO_STRING_INTERN]
 Normally the space to put the temporary strings is static, one per file. With
 this, it's possible to have a global programme storage to save space: have one
 file have `TO_STRING_INTERN` as the first box, the other files
 `TO_STRING_EXTERN`. This is unsynchronized.

 @std C89 */

/* Check defines. */
#if !defined(CAT) || !defined(CAT_) || !defined(Z_) || !defined(ZI_) \
	|| !defined(TO_STRING) || defined(PZ_) || defined(PZI_)
#error Unexpected preprocessor symbols.
#endif
#ifndef TO_STRING_LEFT
#define TO_STRING_LEFT '('
#endif
#ifndef TO_STRING_RIGHT
#define TO_STRING_RIGHT ')'
#endif

#define PZ_(n) CAT(to_string, Z_(n))
#define PZI_(n) CAT(iterate, ZI_(n))

/** Responsible for turning the first argument into a 12-`char` null-terminated
 output string. */
typedef void (*PZ_(to_string_fn))(const PZI_(type) *, char (*)[12]);

/* Check that `TO_STRING` is a function implementing <typedef:<PZ>to_string>. */
static const PZ_(to_string_fn) PZ_(to_string) = (TO_STRING);

/** @return Print the contents of `box` in a static string buffer of 256
 bytes with limitations of only printing 4 things at a time.
 @order \Theta(1) @allow */
static const char *Z_(to_string)(const PZI_(box) *const box) {
	const char comma = ',', space = ' ', *const ellipsis = "…",
		left = TO_STRING_LEFT, right = TO_STRING_RIGHT;
	const size_t ellipsis_len = strlen(ellipsis);
	char *const buffer = to_string_buffers[to_string_buffer_i++], *b = buffer;
	size_t advance, size;
	const PZI_(type) *x;
	PZI_(iterator) it;
	int is_sep = 0;
	/* Minimum size: "(" "XXXXXXXXXXX" "," "…" ")" "\0". */
	assert(box && !(to_string_buffers_no & (to_string_buffers_no - 1))
		&& to_string_buffer_size >= 1 + 11 + 1 + ellipsis_len + 1 + 1);
	/* Advance the buffer for next time. */
	to_string_buffer_i &= to_string_buffers_no - 1;
	/* Begin iteration. */
	PZI_(begin)(&it, box);
	*b++ = left;
	while(x = PZI_(next)(&it)) {
		PZ_(to_string)(x, (char (*)[12])b);
		/* Paranoid about '\0'. */
		for(advance = 0; *b != '\0' && advance < 11; b++, advance++);
		is_sep = 1, *b++ = comma, *b++ = space;
		/* Greedy typesetting: enough for "XXXXXXXXXXX" "," "…" ")" "\0". */
		if((size = (size_t)(b - buffer))
			> to_string_buffer_size - 11 - 1 - ellipsis_len - 1 - 1)
			{ if(PZI_(next)(&it)) goto ellipsis; else break; }
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

static void PZ_(unused_to_string_coda)(void);
static void PZ_(unused_to_string)(void)
	{ Z_(to_string)(0); PZ_(unused_to_string_coda)(); }
static void PZ_(unused_to_string_coda)(void) { PZ_(unused_to_string)(); }

#undef PZI_
#undef PZ_
/* #undef Z_ We need this for test. */
#undef TO_STRING_ITERATE_
#undef TO_STRING
#undef TO_STRING_LEFT
#undef TO_STRING_RIGHT
