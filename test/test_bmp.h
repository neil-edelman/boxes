#if defined(QUOTE) || defined(QUOTE_)
#error QUOTE_? cannot be defined.
#endif
#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)

#include <stdio.h>

/* The byte versions for testing. */

struct PB_(gadget)
	{ char bits[sizeof(((struct B_(bmp) *)0)->chunk) * CHAR_BIT + 1]; };

/** Homomorphism from `a` to `g` for use in testing. */
static void PB_(to_gadget)(const struct B_(bmp) *const a,
	struct PB_(gadget) *const g) {
	size_t i, j;
	char *s = g->bits;
	assert(a && g);
	for(i = 0; i < sizeof a->chunk / sizeof *a->chunk; i++)
		for(j = 0; j < sizeof *a->chunk * CHAR_BIT; j++) *s++
		= a->chunk[i] & (1 << sizeof *a->chunk * CHAR_BIT - 1 - j) ? '1' : '0';
	*s = '\0';
}

/** @return Temporary string representation of `g` with spaces, parentheses
 at `x` to `n`. */
static char *PB_(adorn)(const struct PB_(gadget) *g,
	const unsigned x, const unsigned n) {
	static unsigned choice;
	static char temp[4][(sizeof g->bits - 1) * 10 / 8 + 3 + 1];
	char *const zs = temp[choice++], *z = zs;
	size_t i;
	choice %= sizeof temp / sizeof *temp;
	for(i = 0; ; i++) {
		const int end = g->bits[i] == '\0', sep = !(i & 3) && i,
			lef = i == x, rht = i == x + n, stp = i == BMP_BITS;
		if(!end && sep && !rht && !stp)
			*z++ = i & 31 ? i & 7 ? ':' : '/' : ' ';
		if(lef) *z++ = '[';
		if(rht) *z++ = ']';
		if(stp) *z++ = '#';
		if(!end && sep && (rht || stp))
			*z++ = i & 31 ? i & 7 ? ':' : '/' : ' ';
		if(!end) assert(g->bits[i] == '0' || g->bits[i] == '1'),
			*z++ = g->bits[i];
		if(end) break;
	}
	*z = '\0';
	return zs;
}

static void PB_(gadget_clear_all)(struct PB_(gadget) *const g) {
	assert(g);
	memset(g, '0', sizeof g->bits - 1);
	g->bits[sizeof g->bits - 1] = '\0';
}

static void PB_(gadget_invert_all)(struct PB_(gadget) *const g) {
	size_t i;
	assert(g);
	for(i = 0; i < BMP_BITS; i++) g->bits[i] = g->bits[i] == '0' ? '1' : '0';
	for( ; i < sizeof g->bits - 1; i++) assert(g->bits[i] == '0');
	assert(g->bits[sizeof g->bits - 1] == '\0');
}

static void PB_(gadget_set)(struct PB_(gadget) *const g, const unsigned n)
	{ assert(g && n < BMP_BITS); g->bits[n] = '1'; }

static void PB_(gadget_clear)(struct PB_(gadget) *const g, const unsigned n)
	{ assert(g && n < BMP_BITS); g->bits[n] = '0'; }

static void PB_(gadget_toggle)(struct PB_(gadget) *const g, const unsigned n)
	{ assert(g && n < BMP_BITS); g->bits[n] = g->bits[n] == '1' ? '0' : '1'; }

static void PB_(gadget_insert)(struct PB_(gadget) *const g,
	const unsigned x, const unsigned n) {
	assert(g && x + n < BMP_BITS);
	memmove(g->bits + x + n, g->bits + x, BMP_BITS - x - n);
	memset(g->bits + x, '0', n);
	assert(g->bits[sizeof g->bits - 1] == '\0');
}

static void PB_(gadget_remove)(struct PB_(gadget) *const g,
	const unsigned x, const unsigned n) {
	assert(g && x + n < BMP_BITS);
	memmove(g->bits + x, g->bits + x + n, BMP_BITS - x - n);
	memset(g->bits + BMP_BITS - n, '0', n);
	assert(g->bits[sizeof g->bits - 1] == '\0');
}

#if 0
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
	struct B_(bmp) bmp, bmp_bkp;
	struct PB_(gadget) gdt, bmp_gdt, gdt_bkp;
	unsigned i, j;
	const unsigned r[] = { 0, 1, 2, 0, 2, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1 };

	PB_(gadget_clear_all)(&gdt);
	B_(bmp_clear_all)(&bmp);
	PB_(to_gadget)(&bmp, &bmp_gdt);
	printf("clear_all:\n"
		" str %s;\n"
		" bmp %s.\n", PB_(adorn)(&gdt, 0, 0), PB_(adorn)(&bmp_gdt, 0, 0));
	assert(!strcmp(gdt.bits, bmp_gdt.bits));
	for(i = 0; i < sizeof bmp.chunk / sizeof *bmp.chunk; i++)
		assert(!bmp.chunk[i]);

	PB_(gadget_invert_all)(&gdt);
	B_(bmp_invert_all)(&bmp);
	PB_(to_gadget)(&bmp, &bmp_gdt);
	printf("invert:\n"
		" str %s;\n"
		" bmp %s.\n", PB_(adorn)(&gdt, 0, 0), PB_(adorn)(&bmp_gdt, 0, 0));
	assert(!strcmp(gdt.bits, bmp_gdt.bits));

	B_(bmp_clear_all)(&bmp);
	PB_(gadget_clear_all)(&gdt);

	/* Has to be clear. */
	for(i = 0; i < BMP_BITS; i += 1 + r[i % sizeof r / sizeof *r])
		PB_(gadget_set)(&gdt, i);
	for(i = 0; i < BMP_BITS; i += 1 + r[i % sizeof r / sizeof *r])
		B_(bmp_set)(&bmp, i);
	PB_(to_gadget)(&bmp, &bmp_gdt);
	printf("set:\n"
		" str %s;\n"
		" bmp %s.\n", PB_(adorn)(&gdt, 0, 0), PB_(adorn)(&bmp_gdt, 0, 0));
	assert(!strcmp(gdt.bits, bmp_gdt.bits));

	printf("test:\n"); /* Has to come just after set. */
	for(i = 0, j = 0; i < BMP_BITS; i++) {
		assert(B_(bmp_at)(&bmp, i) == (i == j));
		if(i < j) continue;
		j += 1 + r[j % sizeof r / sizeof *r];
	}

	PB_(gadget_clear)(&gdt, 0);
	B_(bmp_clear)(&bmp, 0);
	PB_(to_gadget)(&bmp, &bmp_gdt);
	printf("clear:\n"
		" str %s;\n"
		" bmp %s.\n", PB_(adorn)(&gdt, 0, 0), PB_(adorn)(&bmp_gdt, 0, 0));
	assert(!strcmp(gdt.bits, bmp_gdt.bits));

	PB_(gadget_toggle)(&gdt, 0);
	B_(bmp_toggle)(&bmp, 0);
	PB_(to_gadget)(&bmp, &bmp_gdt);
	printf("toggle:\n"
		" str %s;\n"
		" bmp %s.\n", PB_(adorn)(&gdt, 0, 0), PB_(adorn)(&bmp_gdt, 0, 0));
	assert(!strcmp(gdt.bits, bmp_gdt.bits));

	memcpy(&bmp_bkp, &bmp, sizeof bmp);
	memcpy(&gdt_bkp, &gdt, sizeof gdt);
	for(i = 0; i < BMP_BITS; i++) {
		for(j = 0; j < BMP_BITS - i; j++) {
			memcpy(&bmp, &bmp_bkp, sizeof bmp);
			memcpy(&gdt, &gdt_bkp, sizeof gdt);
			PB_(to_gadget)(&bmp, &bmp_gdt);
			assert(!strcmp(gdt.bits, bmp_gdt.bits));
			PB_(gadget_insert)(&gdt, i, j);
			B_(bmp_insert)(&bmp, i, j);
			PB_(to_gadget)(&bmp, &bmp_gdt);
			printf("insert(%u, %u):\n"
				" bfr %s;\n"
				" str %s;\n"
				" bmp %s.\n",
				i, j, PB_(adorn)(&gdt_bkp, i, 0),
				PB_(adorn)(&gdt, i, j), PB_(adorn)(&bmp_gdt, i, j));
			assert(!strcmp(gdt.bits, bmp_gdt.bits));
		}
	}
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
