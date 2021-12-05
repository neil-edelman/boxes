/** 2018 Neil Edelman, distributed under the terms of the MIT License;
 see readme.txt, or \url{ https://opensource.org/licenses/MIT }.

 Unit-test.

 @title		TestStats
 @author	Neil
 @std		C89/90
 @version	2018-03
 @since		2018-03 */

#include <stdlib.h> /* rand */
#include <stdio.h>  /* fprintf */
#include <time.h>	/* clock */
#include <limits.h>	/* INT_MAX */

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

/** @implements <Foo>ToString */
static void Float_to_string(const float this, char (*const a)[12]) {
	sprintf(*a, "%2.2g", this);
}
/** @implements <Foo>Action */
static void Float_filler(float this) {
	this = rand() / ((float)RAND_MAX / 200.0f) - 100.0f;
}
#define MEASURE_NAME Float
#define MEASURE_TYPE float
#define MEASURE_TO_STRING &Float_to_string
#define MEASURE_TEST &Float_filler
#include "../src/Measure.h"

/** Entry point. */
int main(void) {
	unsigned seed = (unsigned)clock();
	srand(seed), rand(), printf("Seed %u.\n", seed);
	FooMapTest();
	return 0;
}
