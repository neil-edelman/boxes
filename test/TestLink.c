/** Unit test of Link.c.

 @file		TestLink
 @author	Neil
 @std		C89/90
 @version	1.0; 2017-05
 @since		1.0; 2017-05 scraped from TestList.h */

#include <stdlib.h> /* EXIT_ malloc free */
#include <stdio.h>  /* fprintf */
#include <string.h>	/* strcmp */
#include "Orcish.h"

#ifdef __GNUC__ /* <-- GCC */
#pragma GCC diagnostic ignored "-Wunused-function" /* doesn't work */
#pragma GCC diagnostic ignored "-Wconversion"
#elif _MSC_VER /* GCC --><-- MSVC: not a C89/90 compiler; needs a little help;
Assignment within conditional expression." "<ANSI89/ISO90 name>: The POSIX
name for this item is deprecated. Instead use the ISO C and C++ conformant
name <ISO C++11 name>." */
#pragma warning(disable: 4706)
#pragma warning(disable: 4996)
#elif __BORLANDC__ /* MSVC --><-- BCC */
#elif __MINGW32__ /* BCC --><-- MinGW */
#elif __DJGPP__ /* MinGW --><-- DJGPP */
#endif /* --> */

struct Foo {
	int key;
	char value[32];
};
/** @implements <Foo>Comparator */
static int Foo_key_cmp(const struct Foo *a, const struct Foo *b) {
	return (b->key < a->key) - (a->key < b->key);
}
/** @implements <Foo>Comparator */
static int Foo_value_cmp(const struct Foo *a, const struct Foo *b) {
	return strcmp(a->value, b->value);
}
/** @implements <Foo>ToString */
static void Foo_to_string(const struct Foo *this, char (*const a)[9]) {
	snprintf(*a, sizeof *a, "%.3d%.3s", this->key, this->value);
}
/** @implements <Foo>Action */
static void Foo_filler(struct Foo *const this) {
	this->key = (float)(rand() / (RAND_MAX + 1.0) * 99);
	Orcish(this->value, sizeof this->value);
}
#define LINK_NAME Foo
#define LINK_TYPE struct Foo
#define LINK_A_NAME Key
#define LINK_A_COMPARATOR &Foo_key_cmp
#define LINK_B_NAME Value
#define LINK_B_COMPARATOR &Foo_value_cmp
#define LINK_TO_STRING &Foo_to_string
#define LINK_TEST &Foo_filler
#include "../src/Link.h"

/** Entry point.
 @return Either EXIT_SUCCESS or EXIT_FAILURE. */
int main(void) {
	FooLinkTest();
	printf("Test succeeded.\n");
	return EXIT_SUCCESS;
}
