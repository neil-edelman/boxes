#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* Mock. */
struct cursor { const char root, *start; };
static const char *ref_to_string(const char *const*const a) { return *a; }

/** Thunk(`cur`, `a`), transforming a cursor to the key string. */
static void tree_to_string(const struct cursor *const cur,
	char (*const a)[12]) {
	const char *from = ref_to_string(&cur->start);
	unsigned i;
	char *to = *a;
	assert(cur && cur->root && a);
	for(i = 0; i < 11 - 3; from++, i++) {
		*to++ = *from;
		if(*from == '\0') return;
	}
	/* Utf-8 assumed. Split at code-point. Still could be grapheme clusters,
	 but… we take what we can get without a full-functioning text engine. */
	for( ; i < 11; from++, i++) {
		const unsigned left = 11 - i;
		const unsigned char f = (const unsigned char)*from;
		if(f < 0x80) goto encode;
		if((f & 0xe0) == 0xc0) if(left < 2) break; else goto encode;
		if((f & 0xf0) == 0xe0) if(left < 3) break; else goto encode;
		if((f & 0xf8) == 0xf0) break;
encode:
		/* Very permissive otherwise; we don't actually know anything about the
		 encoding. */
		*to++ = *from;
		if(*from == '\0') return;
	}
	*to = '\0';
}

#define TEST(string, result) \
s = string, r = result, c.start = s; tree_to_string(&c, &a); \
printf("data: \"%s\", test: \"%s\", supposed: \"%s\".\n", s, a, r); \
cmp = strcmp(r, a), assert(!cmp);

int main(void) {
	const char *s, *r;
	struct cursor c = { 'a', "a" };
	char a[12];
	int cmp;
	TEST("", "")
	TEST("a", "a")
	TEST("aabbccddeeff", "aabbccddeef")
	TEST("aabbccddeæ", "aabbccddeæ")
	TEST("aabbccddeeæ", "aabbccddee")
	TEST("aabbccdd…", "aabbccdd…")
	TEST("aabbccdde…", "aabbccdde")
	TEST("aabbccd🂽", "aabbccd🂽")
	TEST("aabbccdd🂽", "aabbccdd")
	return EXIT_SUCCESS;
}
