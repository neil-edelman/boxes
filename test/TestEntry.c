/** 2018 Neil Edelman, distributed under the terms of the MIT License;
 see readme.txt, or \url{ https://opensource.org/licenses/MIT }.

 Unit-test.

 @title		TestMap
 @author	Neil
 @std		C89/90 + C99 stdint.h:uint32_t
 @version	1.0; 2018-03
 @since		1.0; 2018-03 */

#include <stdlib.h> /* rand */
#include <stdio.h>  /* fprintf */
#include <stdint.h>	/* uint32_t, c99, guaranteed by IEEE 1003.1-2001, :[ */
#include <string.h>	/* strcmp */
#include <time.h>	/* clock */
#include <limits.h>	/* INT_MAX */
#include "Orcish.h"

#ifdef __clang__ /* <-- clang must be placed ahead of __GNUC__ */
#if __clang_major__ > 2 /* fixme: this is not how you do it; I don't know how */
#pragma clang diagnostic ignored "-Wpadded"
#pragma clang diagnostic ignored "-Wdocumentation"
#pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
#endif
#elif __GNUC__ /* clang --><-- GCC */
#pragma GCC diagnostic ignored "-Wconversion"
#elif __BORLANDC__ /* GCC --><-- BCC must be placed ahead of _MSC_VER */
#elif _MSC_VER /* BCC --><-- MSVC */
#pragma warning(disable: 4464)
#pragma warning(disable: 4706)
#pragma warning(disable: 4710)
#pragma warning(disable: 4711)
#pragma warning(disable: 4820)
#pragma warning(disable: 4996)
#elif __MINGW32__ /* MSVC --><-- MinGW */
#elif __DJGPP__ /* MinGW --><-- DJGPP */
#endif /* --> */

struct Test {
	char test[128];
};

/** @implements <Test>ToString */
/*static void Test_to_string(const struct Test *const test, char (*const a)[12]) {
	sprintf(*a, "%.11s", test->test);
}*/

struct TestEntry;
static void TestEntry_to_string(const struct TestEntry *const,
	char (*const)[12]);
static void TestEntry_filler(struct TestEntry *const);

#define ENTRY_NAME Test
#define ENTRY_KEY const char *
#define ENTRY_VALUE struct Test
#define ENTRY_CMP &strcmp
#define ENTRY_HASH &map_fnv_32a_str
#define ENTRY_TO_STRING &TestEntry_to_string
#define ENTRY_TEST &TestEntry_filler
#include "../src/Entry.h"

/** @implements <TestEntry>ToString */
static void TestEntry_to_string(const struct TestEntry *const test,
	char (*const a)[12]) {
	sprintf(*a, "%.5s:%.5s", test->key, test->value.test);
}

#include "words_3000.h"
static const size_t words_size = sizeof words / sizeof *words;

/** @implements <TestEntry>Action */
static void TestEntry_filler(struct TestEntry *const test) {
	test->key = words[(size_t)(rand() / (1.0 * RAND_MAX / words_size))];
	Orcish(test->value.test, sizeof test->value.test);
}

/** Entry point. */
int main(void) {
	unsigned seed = (unsigned)clock();
	srand(seed), rand(), printf("Seed %u.\n", seed);
	TestEntryTest();
	return 0;
}
