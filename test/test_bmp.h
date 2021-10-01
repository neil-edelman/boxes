#if defined(QUOTE) || defined(QUOTE_)
#error QUOTE_? cannot be defined.
#endif
#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)

#include <stdio.h>

/* The byte versions for testing. */

struct PB_(str)
	{ char bits[sizeof(((struct B_(bmp) *)0)->chunk) * CHAR_BIT + 1]; };

/** `bmp` of is written in `str` for use in testing. */
static void PB_(to_string)(const struct B_(bmp) *const bmp,
	struct PB_(str) *const str) {
	size_t i, j;
	char *s = str->bits;
	assert(bmp && str);
	for(i = 0; i < sizeof bmp->chunk / sizeof *bmp->chunk; i++)
		for(j = 0; j < sizeof *bmp->chunk * CHAR_BIT; j++) *s++
		= bmp->chunk[i] & (1 << sizeof *bmp->chunk * CHAR_BIT - 1 - j)
		? '1' : '0';
	*s = '\0';
}

/** Adds `label` and spaces into `s` and adds parentheses between `offset`
 to `range`, new line, and sends it to `stdout`. */
static void PB_(adorn)(const struct PB_(str) *str,
	const unsigned offset, const unsigned range, const char label) {
	size_t i;
	fputc(label, stdout);
	for(i = 0; ; i++) {
		const int end = str->bits[i] == '\0', sep = !(i & 3) && i,
			lef = i == offset, rht = i == offset + range, stp = i == BMP_BITS;
		if(!end && sep && !rht && !stp) fputc(i & 31 ? i & 7 ? ':' : ' ' : '/', stdout);
		if(lef) fputc('[', stdout);
		if(rht) fputc(']', stdout);
		if(stp) fputc('#', stdout);
		if(!end && sep && (rht || stp))
			fputc(i & 31 ? i & 7 ? ':' : ' ' : '/', stdout);
		if(!end) assert(str->bits[i] == '0' || str->bits[i] == '1'),
			fputc(str->bits[i], stdout);
		if(end) break;
	}
	fputc('\n', stdout);
}

static void PB_(str_clear)(struct PB_(str) *const a) {
	assert(a);
	memset(a, '0', sizeof a->bits - 1);
	a->bits[sizeof a->bits - 1] = '\0';
}

static void PB_(str_invert)(struct PB_(str) *const a) {
	size_t i;
	assert(a);
	for(i = 0; i < BMP_BITS; i++) a->bits[i] = a->bits[i] == '0' ? '1' : '0';
	for( ; i < sizeof a->bits - 1; i++) assert(a->bits[i] == '0');
	assert(a->bits[sizeof a->bits - 1] == '\0');
}

static void PB_(str_insert)(struct PB_(str) *const a,
	const unsigned offset, const unsigned n) {
	assert(a && offset + n <= BMP_BITS);
	memmove(a + offset + n, a + offset, BMP_BITS - offset - n);
	memset(a + offset, '0', n);
	assert(a->bits[sizeof a->bits - 1] == '\0');
}

#if 0
static void string_remove(char *const a,
	const unsigned delete, const unsigned a_size) {
	assert(a && delete < a_size);
	memmove(a + delete, a + delete + 1, a_size - delete - 1);
	a[a_size - 1] = '0';
	assert(a[BMP_CHUNK(a_size) * BMP_BITS] == '\0');
}

/** Move `byte_range` at `byte_offset` in `parent` with `bytes` to
 `child` with `bytes`, starting at zero and filling the rest with '0'; the
 moved part in `parent` is replaced with a single '1'. */
static void string_split(char *const parent, char *const child,
	const unsigned offset_byte, const unsigned byte_range,
	const unsigned bytes) {
	const unsigned full = BMP_CHUNK(bytes) * BMP_BITS;
	assert(parent && child && byte_range && offset_byte + byte_range <= bytes);
	/* Copy select `parent` to `child`. */
	memcpy(child, parent + offset_byte, byte_range);
	memset(child + byte_range, '0', full - byte_range);
	child[full] = '\0';
	/* Move the selected range in `parent` to '1'. */
	parent[offset_byte] = '1';
	/* Move back. */
	memmove(parent + offset_byte + 1, parent + offset_byte + byte_range,
		bytes - offset_byte - byte_range);
	memset(parent + bytes - byte_range + 1, '0', byte_range - 1);
	assert(parent[full] == '\0');
}
#endif


static void PB_(test)(void) {
	struct B_(bmp) bmp;
	struct PB_(str) str, bmp_str;
	size_t i;

	printf("clear:\n");
	B_(bmp_clear)(&bmp);
	PB_(to_string)(&bmp, &bmp_str);
	PB_(str_clear)(&str);
	PB_(adorn)(&str, 0, 0, 'S'), PB_(adorn)(&bmp_str, 0, 0, 'B');
	assert(!strcmp(str.bits, bmp_str.bits));
	for(i = 0; i < sizeof bmp.chunk / sizeof *bmp.chunk; i++) assert(!bmp.chunk[i]);

	printf("invert:\n");
	B_(bmp_invert_all)(&bmp);
	PB_(to_string)(&bmp, &bmp_str);
	PB_(str_invert)(&str);
	PB_(adorn)(&str, 0, 0, 'S'), PB_(adorn)(&bmp_str, 0, 0, 'B');
	assert(!strcmp(str.bits, bmp_str.bits));
}

/** Will be tested on stdout. Requires `BMP_TEST`, and not `NDEBUG` while
 defining `assert`. @allow */
static void B_(bmp_test)(void) {
	struct B_(bmp) b;
	printf("<" QUOTE(BMP_NAME) ">bmp is storing " QUOTE(BMP_BITS) " boolean"
		" values, backed by an underlying array of %lu <" QUOTE(BMP_TYPE) ">,"
		" %luB, %lub; testing:\n",
		(unsigned long)sizeof b.chunk / sizeof *b.chunk,
		(unsigned long)sizeof b.chunk,
		(unsigned long)sizeof b.chunk * CHAR_BIT);
	PB_(test)();
	fprintf(stderr, "Done tests of <" QUOTE(BMP_NAME) ">bmp.\n\n");
}

/* Un-define all macros. */
#undef QUOTE
#undef QUOTE_
