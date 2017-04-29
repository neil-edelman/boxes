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
	struct FooLink foos[100];
	const size_t foos_capacity = sizeof foos / sizeof *foos;
	size_t foos_size = 0;
	struct FooLinked a = { 0 };
	enum { E_NO, E_F, E_ASRT } e = E_NO;

	do {
		unsigned i;
		for(i = 0; i < foos_capacity; i++) {
			struct Foo *foo = FooLinkGet(foos + i);
			foo->key = (int)(rand() / (1.0 + RAND_MAX) * 1000.0);
			Orcish(foo->value, sizeof foo->value);
		}
		while(foos_size < 10) FooLinkedAdd(&a, foos + foos_size), foos_size++;
		printf("Unsorted:\nKey:   %s.\nValue: %s.\n", FooLinkedKeyToString(&a),
			FooLinkedValueToString(&a));
		FooLinkedKeySort(&a);
		FooLinkedValueSort(&a);
		printf("Sorted:\nKey:   %s.\nValue: %s.\n", FooLinkedKeyToString(&a),
			FooLinkedValueToString(&a));
		/*
		printf("\nTextMatch:\n");
		TextMatch(t, tpattern, tpattern_size);
		if(strcmp(sup = "<a href = \"Test%Test\">Test%Test</a> yo <em>YO</em>",
			str = TextGet(t))) { e = E_ASRT; break; }
		printf("Text: %s\n", TextGet(t));
		*/
	} while(0); switch(e) {
		case E_NO: break;
		case E_F: fprintf(stderr, "Text exception.\n");
			break;
		case E_ASRT: fprintf(stderr,"Text: assert failed, '%s' but was '%s'.\n",
			"", ""); break;
	}
	if(!FooLinkTest()) return printf("FAILED.\n"), EXIT_FAILURE;

	printf("Tests %s.\n", e ? "FAILED" : "SUCCEEDED");
	return e ? EXIT_FAILURE : EXIT_SUCCESS;
}
