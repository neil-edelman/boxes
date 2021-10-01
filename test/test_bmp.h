#if defined(QUOTE) || defined(QUOTE_)
#error QUOTE_? cannot be defined.
#endif
#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)

#include <stdio.h>

static void PB_(test)(void) {
	struct B_(bmp) b;
	B_(bmp_clear)(b);
	
}

/** Will be tested on stdout. Requires `BMP_TEST`, and not `NDEBUG` while
 defining `assert`. @allow */
static void B_(bmp_test)(void) {
	struct B_(bmp) b;
	printf("<" QUOTE(BMP_NAME) ">bmp"
		" is an underlying array of %lu <" QUOTE(BMP_TYPE) ">, %luB, %lub,"
		" to store " QUOTE(BMP_BITS) "b testing:\n",
		(unsigned long)sizeof b.chunk / sizeof *b.chunk,
		(unsigned long)sizeof b.chunk,
		(unsigned long)sizeof b.chunk * CHAR_BIT);
	PB_(test)();
	fprintf(stderr, "Done tests of <" QUOTE(BMP_NAME) ">bmp.\n\n");
}

/* Un-define all macros. */
#undef QUOTE
#undef QUOTE_
