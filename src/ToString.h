/** @license 2020 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle To String Interface

 This is a sub-include.

 @std C89
 @cf [Heap](https://github.com/neil-edelman/Heap)
 @cf [List](https://github.com/neil-edelman/List)
 @cf [Orcish](https://github.com/neil-edelman/Orcish)
 @cf [Pool](https://github.com/neil-edelman/Pool)
 @cf [Set](https://github.com/neil-edelman/Set)
 @cf [Trie](https://github.com/neil-edelman/Trie) */

#include <string.h>

/* Check defines. */
/* @fixme S_ is terrible. */
#ifndef S_
#error Function-of-1 like macro S_ undefined.
#endif
#ifndef TO_STRING_ITERATOR
#error Tag type TO_STRING_ITERATOR undefined.
#endif
#ifndef TO_STRING_NEXT
#error Function TO_STRING_NEXT undefined.
#endif

#ifndef TO_STRING_H /* <!-- idempotent: For all ToString in compilation unit. */
#define TO_STRING_H
static char to_string_buffers[4][256];
static const unsigned to_string_buffers_no = sizeof to_string_buffers
	/ sizeof *to_string_buffers, to_string_buffer_size
	= sizeof *to_string_buffers / sizeof **to_string_buffers;
static unsigned to_string_buffer_i;
#endif /* idempotent --> */

typedef TO_STRING_ITERATOR S_(Iterator);

/** Returns true if it wrote to the buffer and advances to the next. */
typedef int (*S_(NextToString))(S_(Iterator) *, char (*)[12]);

/* Check that `TO_STRING_NEXT` is a function implementing
 <typedef:<S>NextToString>. */
static const S_(NextToString) S_(next_to_string) = (TO_STRING_NEXT);

static const char *S_(to_stringz)(S_(Iterator) *const it,
	const char start, const char end) {
	const char comma = ',', space = ' ',
		*const ellipsis = "…", *const null = "null";
	const size_t ellipsis_len = strlen(ellipsis), null_len = strlen(null);
	char *const buffer = to_string_buffers[to_string_buffer_i++], *b = buffer;
	size_t advance, size;
	int is_sep = 0;
	/* Minimum size: "(" "XXXXXXXXXXX" "," "…" ")" "\0" or "null" "\0". */
	assert(it && !(to_string_buffers_no & (to_string_buffers_no - 1))
		&& to_string_buffer_size >= 1 + 11 + 1 + ellipsis_len + 1 + 1
		&& to_string_buffer_size >= null_len + 1);
	/* Advance the buffer for next time. */
	to_string_buffer_i &= to_string_buffers_no - 1;
	if(!it->a) { memcpy(b, null, null_len), b += null_len; goto terminate; }
	*b++ = start;
	while(S_(next_to_string)(it, (char (*)[12])b)) {
		/* Be paranoid about the '\0'. */
		for(advance = 0; *b != '\0' && advance < 11; b++, advance++);
		is_sep = 1, *b++ = comma, *b++ = space;
		/* Greedy typesetting: enough for "XXXXXXXXXXX" "," "…" ")" "\0". */
		if((size = b - buffer) > to_string_buffer_size
			- 11 - 1 - ellipsis_len - 1 - 1) {
			char throw_out[12];
			if(S_(next_to_string)(it, &throw_out)) goto ellipsis; else break;
		}
	}
	if(is_sep) b -= 2;
	*b++ = end;
	goto terminate;
ellipsis:
	b--;
	memcpy(b, ellipsis, ellipsis_len), b += ellipsis_len;
	*b++ = end;
terminate:
	*b++ = '\0';
	assert((size = b - buffer) <= to_string_buffer_size);
	return buffer;
}

#undef S_
#undef TO_STRING_ITERATOR
#undef TO_STRING_NEXT



#if 0
/** Responsible for turning the first argument into a 12-`char` null-terminated
 output string. Used for `ARRAY_TO_STRING`. */
typedef void (*PT_(ToString))(const T *, char (*)[12]);
/* Check that `ARRAY_TO_STRING` is a function implementing
 <typedef:<PT>ToString>. */
static const PT_(ToString) PT_(to_string) = (ARRAY_TO_STRING);

/** Can print 4 things at once before it overwrites. One must a
 `ARRAY_TO_STRING` to a function implementing <typedef:<PT>ToString> to get
 this functionality.
 @return Prints `a` in a static buffer. @order \Theta(1) @allow */

static const char *T_(ArrayToString)(const struct T_(Array) *const a) {
	const char start = '(', comma = ',', space = ' ', end = ')',
	*const ellipsis_end = ",…)", *const null = "null",
	*const idle = "idle";
	const size_t ellipsis_end_len = strlen(ellipsis_end),
	null_len = strlen(null), idle_len = strlen(idle);
	size_t i;
	PT_(Type) *e, *e_end;
	int is_first = 1;
	assert(!(buffers_no & (buffers_no - 1)) && ellipsis_end_len >= 1
		   && buffer_size >= 1 + 11 + ellipsis_end_len + 1
		   && buffer_size >= null_len + 1
		   && buffer_size >= idle_len + 1);
	/* Advance the buffer for next time. */
	buffer_i &= buffers_no - 1;
	if(!a) { memcpy(b, null, null_len), b += null_len; goto terminate; }
	if(!a->first) { memcpy(b, idle, idle_len), b += idle_len; goto terminate; }
	*b++ = start;
	for(e = a->first, e_end = a->first + a->size; ; ) {
		if(!is_first) *b++ = comma, *b++ = space;
			else is_first = 0;
				PT_(to_string)(e, (char (*)[12])b);
		for(i = 0; *b != '\0' && i < 12; b++, i++);
		if(++e >= e_end) break;
		if((size_t)(b - buffer) > buffer_size - 2 - 11 - ellipsis_end_len - 1)
			goto ellipsis;
	}
	*b++ = end;
	goto terminate;
ellipsis:
	memcpy(b, ellipsis_end, ellipsis_end_len), b += ellipsis_end_len;
terminate:
	*b++ = '\0';
	assert(b <= buffer + buffer_size);
	return buffer;
}
#endif
