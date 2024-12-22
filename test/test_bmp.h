#ifdef BMP_NON_STATIC
#	define static
void T_(tests)(void);
#endif
#ifndef BMP_DECLARE_ONLY

#	if defined(QUOTE) || defined(QUOTE_)
#		error QUOTE_? cannot be defined.
#	endif
#	define QUOTE_(name) #name
#	define QUOTE(name) QUOTE_(name)
#	ifdef static /* Private functions. */
#		undef static
#	endif

#include <stdio.h>

/* The byte versions for testing. */

struct pT_(gadget)
	{ char bits[sizeof(((struct t_(bmp) *)0)->chunk) * CHAR_BIT + 1]; };

/** Homomorphism from `a` to `g` for use in testing. */
static void pT_(to_gadget)(const struct t_(bmp) *const a,
	struct pT_(gadget) *const g) {
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
static char *pT_(adorn)(const struct pT_(gadget) *g,
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

/** Clears all `g` to zero. */
static void pT_(gadget_clear_all)(struct pT_(gadget) *const g) {
	assert(g);
	memset(g, '0', sizeof g->bits - 1);
	g->bits[sizeof g->bits - 1] = '\0';
}

/** Inverts all `g`. */
static void pT_(gadget_invert_all)(struct pT_(gadget) *const g) {
	size_t i;
	assert(g);
	for(i = 0; i < BMP_BITS; i++) g->bits[i] = g->bits[i] == '0' ? '1' : '0';
	for( ; i < sizeof g->bits - 1; i++) assert(g->bits[i] == '0');
	assert(g->bits[sizeof g->bits - 1] == '\0');
}

/** Sets `n` in `g`. */
static void pT_(gadget_set)(struct pT_(gadget) *const g, const unsigned n)
	{ assert(g && n < BMP_BITS); g->bits[n] = '1'; }

/** Clears `n` in `g`. */
static void pT_(gadget_clear)(struct pT_(gadget) *const g, const unsigned n)
	{ assert(g && n < BMP_BITS); g->bits[n] = '0'; }

/** Toggles `n` in `g`. */
static void pT_(gadget_toggle)(struct pT_(gadget) *const g, const unsigned n)
	{ assert(g && n < BMP_BITS); g->bits[n] = g->bits[n] == '1' ? '0' : '1'; }

/** Inserts `n` zeros at `x` in `g`. */
static void pT_(gadget_insert)(struct pT_(gadget) *const g,
	const unsigned x, const unsigned n) {
	assert(g && x + n <= BMP_BITS);
	memmove(g->bits + x + n, g->bits + x, BMP_BITS - x - n);
	memset(g->bits + x, '0', n);
	assert(g->bits[sizeof g->bits - 1] == '\0');
}

/** Removes `n` at `x` in `g`. Zeros right side. */
static void pT_(gadget_remove)(struct pT_(gadget) *const g,
	const unsigned x, const unsigned n) {
	assert(g && x + n <= BMP_BITS);
	memmove(g->bits + x, g->bits + x + n, BMP_BITS - x - n);
	memset(g->bits + BMP_BITS - n, '0', n);
	assert(g->bits[sizeof g->bits - 1] == '\0');
}

static void pT_(test)(void) {
	struct t_(bmp) bmp, bmp_bkp;
	struct pT_(gadget) gdt, bmp_gdt, gdt_bkp;
	unsigned i, j, show;
	const unsigned r[] = { 0, 1, 2, 0, 2, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1 };

	pT_(gadget_clear_all)(&gdt);
	T_(clear_all)(&bmp);
	pT_(to_gadget)(&bmp, &bmp_gdt);
	printf("clear_all:\n"
		" str %s;\n"
		" bmp %s.\n", pT_(adorn)(&gdt, 0, 0), pT_(adorn)(&bmp_gdt, 0, 0));
	assert(!strcmp(gdt.bits, bmp_gdt.bits));
	for(i = 0; i < sizeof bmp.chunk / sizeof *bmp.chunk; i++)
		assert(!bmp.chunk[i]);

	pT_(gadget_invert_all)(&gdt);
	T_(invert_all)(&bmp);
	pT_(to_gadget)(&bmp, &bmp_gdt);
	printf("invert:\n"
		" str %s;\n"
		" bmp %s.\n", pT_(adorn)(&gdt, 0, 0), pT_(adorn)(&bmp_gdt, 0, 0));
	assert(!strcmp(gdt.bits, bmp_gdt.bits));

	T_(clear_all)(&bmp);
	pT_(gadget_clear_all)(&gdt);

	/* Has to be clear. */
	for(i = 0; i < BMP_BITS; i += 1 + r[i % sizeof r / sizeof *r])
		pT_(gadget_set)(&gdt, i);
	for(i = 0; i < BMP_BITS; i += 1 + r[i % sizeof r / sizeof *r])
		T_(set)(&bmp, i);
	pT_(to_gadget)(&bmp, &bmp_gdt);
	printf("set:\n"
		" str %s;\n"
		" bmp %s.\n", pT_(adorn)(&gdt, 0, 0), pT_(adorn)(&bmp_gdt, 0, 0));
	assert(!strcmp(gdt.bits, bmp_gdt.bits));

	printf("test:\n"); /* Has to come just after set. */
	for(i = 0, j = 0; i < BMP_BITS; i++) {
		assert(!!T_(test)(&bmp, i) == (i == j));
		if(i < j) continue;
		j += 1 + r[j % sizeof r / sizeof *r];
	}

	pT_(gadget_clear)(&gdt, 0);
	T_(clear)(&bmp, 0);
	pT_(to_gadget)(&bmp, &bmp_gdt);
	printf("clear:\n"
		" str %s;\n"
		" bmp %s.\n", pT_(adorn)(&gdt, 0, 0), pT_(adorn)(&bmp_gdt, 0, 0));
	assert(!strcmp(gdt.bits, bmp_gdt.bits));

	pT_(gadget_toggle)(&gdt, 0);
	T_(toggle)(&bmp, 0);
	pT_(to_gadget)(&bmp, &bmp_gdt);
	printf("toggle:\n"
		" str %s;\n"
		" bmp %s.\n", pT_(adorn)(&gdt, 0, 0), pT_(adorn)(&bmp_gdt, 0, 0));
	assert(!strcmp(gdt.bits, bmp_gdt.bits));

	/* These are used in testing all insert and remove. */
	memcpy(&bmp_bkp, &bmp, sizeof bmp);
	memcpy(&gdt_bkp, &gdt, sizeof gdt);

	show = 0;
	for(i = 0; i < BMP_BITS; i++) {
		for(j = 0; j <= BMP_BITS - i; j++) {
			memcpy(&bmp, &bmp_bkp, sizeof bmp);
			memcpy(&gdt, &gdt_bkp, sizeof gdt);
			pT_(to_gadget)(&bmp, &bmp_gdt);
			assert(!strcmp(gdt.bits, bmp_gdt.bits));
			pT_(gadget_insert)(&gdt, i, j);
			T_(insert)(&bmp, i, j);
			pT_(to_gadget)(&bmp, &bmp_gdt);
			/* Once one gets over 100, it becomes spam. */
			if(show++, !(show & (show - 1))) printf("insert(%u, %u):\n"
				" bfr %s;\n"
				" str %s;\n"
				" bmp %s.\n",
				i, j, pT_(adorn)(&gdt_bkp, i, 0),
				pT_(adorn)(&gdt, i, j), pT_(adorn)(&bmp_gdt, i, j));
			assert(!strcmp(gdt.bits, bmp_gdt.bits));
		}
	}

	show = 0;
	for(i = 0; i < BMP_BITS; i++) {
		for(j = 0; j <= BMP_BITS - i; j++) {
			memcpy(&bmp, &bmp_bkp, sizeof bmp);
			memcpy(&gdt, &gdt_bkp, sizeof gdt);
			pT_(to_gadget)(&bmp, &bmp_gdt);
			assert(!strcmp(gdt.bits, bmp_gdt.bits));
			pT_(gadget_remove)(&gdt, i, j);
			T_(remove)(&bmp, i, j);
			pT_(to_gadget)(&bmp, &bmp_gdt);
			if(show++, !(show & (show - 1))) printf("remove(%u, %u):\n"
				" bfr %s;\n"
				" str %s;\n"
				" bmp %s.\n",
				i, j, pT_(adorn)(&gdt_bkp, i, j),
				pT_(adorn)(&gdt, i, 0), pT_(adorn)(&bmp_gdt, i, 0));
			assert(!strcmp(gdt.bits, bmp_gdt.bits));
		}
	}
}

#	ifdef BMP_NON_STATIC /* Public function. */
#		define static
#	endif
/** Will be tested on stdout. Requires `BMP_TEST`, and not `NDEBUG` while
 defining `assert`. @allow */
static void T_(tests)(void) {
	struct t_(bmp) b;
	printf("<" QUOTE(BMP_NAME) ">bmp is storing " QUOTE(BMP_BITS) " boolean"
		" values, backed by an underlying array of %lu <" QUOTE(BMP_TYPE) ">,"
		" %luB, %lub; testing:\n",
		(unsigned long)sizeof b.chunk / sizeof *b.chunk,
		(unsigned long)sizeof b.chunk,
		(unsigned long)sizeof b.chunk * CHAR_BIT);
	pT_(test)();
	fprintf(stderr, "Done tests of <" QUOTE(BMP_NAME) ">bmp.\n\n");
}

#	undef QUOTE
#	undef QUOTE_
#endif
#ifdef static /* Private functions. */
#	undef static
#endif
