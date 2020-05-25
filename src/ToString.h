/** @license 2020 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle To String Interface

 @param[IA_]
 Function-like define macro accepting one argument and producing a valid name.

 @param[TO_STRING_NEXT]
 A function satisfying <typedef:<IA>NextToString>.

 @param[TO_STRING_ITERATOR]
 Tag type for the first argument to <typedef:<IA>NextToString>.

 @std C89
 @cf [Heap](https://github.com/neil-edelman/Heap)
 @cf [List](https://github.com/neil-edelman/List)
 @cf [Orcish](https://github.com/neil-edelman/Orcish)
 @cf [Pool](https://github.com/neil-edelman/Pool)
 @cf [Set](https://github.com/neil-edelman/Set)
 @cf [Trie](https://github.com/neil-edelman/Trie) */

#include <string.h>

#ifndef TO_STRING_H /* <!-- idempotent: for all in compilation unit. */
#define TO_STRING_H
static char to_string_buffers[4][256];
static const unsigned to_string_buffers_no = sizeof to_string_buffers
	/ sizeof *to_string_buffers, to_string_buffer_size
	= sizeof *to_string_buffers / sizeof **to_string_buffers;
static unsigned to_string_buffer_i;
#endif /* idempotent --> */

/* Check defines. */
#if !defined(CAT) || !defined(CAT_) || !defined(PCAT) || !defined(PCAT_)
#error ToString is meant to be a mixin included from other headers.
#endif
#ifndef TO_STRING_NAME
#error Name TO_STRING_NAME undefined.
#endif
#ifndef TO_STRING_NEXT
#error Function TO_STRING_NEXT undefined.
#endif
#ifndef TO_STRING_ITERATOR
#error Tag type TO_STRING_ITERATOR undefined.
#endif
#ifdef PA_
#error PA_ cannot be defined.
#endif

#define PA_(thing) PCAT(TO_STRING_NAME, thing)

/** Tag type set by `TO_STRING_ITERATOR` should be a type that encodes all the
 values needed for iteration. */
typedef TO_STRING_ITERATOR PA_(Iterator);

/** Returns true if it wrote to the buffer and advances to the next. */
typedef int (*PA_(NextToString))(PA_(Iterator) *, char (*)[12]);

/* Check that `TO_STRING_NEXT` is a function implementing
 <typedef:<S>NextToString>. */
static const PA_(NextToString) PA_(next_to_str12) = (TO_STRING_NEXT);

/** Fills the to string function up with `it`, with `start` and `end`
 delimiters around the `<PA>NextToString` `TO_NEXT_STRING`. @allow */
static const char *PA_(iterator_to_string)(PA_(Iterator) *const it,
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
	while(PA_(next_to_str12)(it, (char (*)[12])b)) {
		/* Be paranoid about the '\0'. */
		for(advance = 0; *b != '\0' && advance < 11; b++, advance++);
		is_sep = 1, *b++ = comma, *b++ = space;
		/* Greedy typesetting: enough for "XXXXXXXXXXX" "," "…" ")" "\0". */
		if((size = b - buffer) > to_string_buffer_size
			- 11 - 1 - ellipsis_len - 1 - 1) {
			char throw_out[12];
			if(PA_(next_to_str12)(it, &throw_out)) goto ellipsis; else break;
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

#undef PA_
#undef TO_STRING_NAME
#undef TO_STRING_ITERATOR
#undef TO_STRING_NEXT
