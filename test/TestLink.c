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
/*#pragma GCC diagnostic ignored "-Wunused-function"*/ /* doesn't work */
/*#pragma GCC diagnostic ignored "-Wconversion"*/ /* I want this :[ assert */
#elif _MSC_VER /* GCC --><-- MSVC: not a C89/90 compiler; needs a little help;
"Assignment within conditional expression." "<ANSI89/ISO90 name>: The POSIX
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

struct AnimalVt;
struct Animal {
	const struct AnimalVt *vt;
	char name[16];
};
/** @implements <Animal>Comparator */
static int Animal_name_cmp(const struct Animal *a, const struct Animal *b) {
	return strcmp(a->name, b->name);
}
/** @implements <Foo>ToString */
static void Animal_to_string(const struct Animal *this, char (*const a)[9]) {
	snprintf(*a, sizeof *a, "%s", this->name);
}
#define LINK_NAME Animal
#define LINK_TYPE struct Animal
#define LINK_A_NAME Name
#define LINK_A_COMPARATOR &Animal_name_cmp
#define LINK_TO_STRING &Animal_to_string
#include "../src/Link.h"
struct Sloth {
	struct AnimalLinkNode animal;
	unsigned lazy;
};
struct Llama {
	struct AnimalLinkNode animal;
	unsigned chomps;
};
static void sloth_act(struct Animal *const animal) {
	struct Sloth *const sloth = (struct Sloth *)animal;
	printf("Sloth %s has been sleeping %u hours.\n", animal->name, sloth->lazy);
}
static void llama_act(struct Animal *const animal) {
	struct Llama *const llama = (struct Llama *)animal;
	printf("Llama %s has chomped %u fingers today.\n", animal->name, llama->chomps);
}
static const struct AnimalVt {
	void (*const act)(struct Animal *const);
} sloth_vt = { &sloth_act }, llama_vt = { &llama_act };
static struct AnimalLink animals;
static void Animal_init(struct AnimalLinkNode *const this) {
	struct Animal *const animal = (struct Animal *)this;
	Orcish(animal->name, sizeof animal->name);
	animal->vt = 0;
	AnimalLinkAdd(&animals, this);
}
static void Sloth_init(struct Sloth *const sloth) {
	Animal_init(&sloth->animal);
	sloth->animal.data.vt = &sloth_vt;
	sloth->lazy = (unsigned)(100.0 * rand() / (RAND_MAX + 1.0) + 20.0);
}
static void Llama_init(struct Llama *const llama) {
	Animal_init(&llama->animal);
	llama->animal.data.vt = &llama_vt;
	llama->chomps = (unsigned)(10.0 * rand() / (RAND_MAX + 1.0) + 1.0);
}
/* @implements AnimalAction */
static void do_stuff(struct Animal *const this) {
	this->vt->act(this);
}

/** Entry point.
 @return Either EXIT_SUCCESS or EXIT_FAILURE. */
int main(void) {
	struct Sloth sloths[3];
	const size_t sloths_size = sizeof sloths / sizeof *sloths;
	struct Llama llamas[6];
	const size_t llamas_size = sizeof llamas / sizeof *llamas;
	struct Sloth others[9];
	const size_t others_size = sizeof others / sizeof *others;
	size_t i;

	FooLinkTest();
	printf("Test succeeded.\n\n");

	assert(others_size >= sloths_size);
	for(i = 0; i < sloths_size; i++) Sloth_init(sloths + i);
	for(i = 0; i < llamas_size; i++) Llama_init(llamas + i);
	printf("Unsorted: %s.\n", AnimalLinkNameToString(&animals));
	AnimalLinkSort(&animals);
	printf("Sorted: %s.\n", AnimalLinkNameToString(&animals));
	AnimalLinkNameForEach(&animals, &do_stuff);
	memcpy(others, sloths, sizeof sloths);
	for(i = 0; i < sloths_size; i++) sloths[i].animal.data.name[0] = '_';
	printf("Moved sloths: %s.\n", AnimalLinkNameToString(&animals));
	AnimalLinkBlockMove(&animals, sloths, sizeof sloths, others);
	printf("Block move: %s.\n", AnimalLinkNameToString(&animals));
	for(i = sloths_size; i < others_size; i++) Sloth_init(others + i);
	printf("New sloths: %s.\n", AnimalLinkNameToString(&animals));
	AnimalLinkSort(&animals);
	printf("Sorted: %s.\n", AnimalLinkNameToString(&animals));
	AnimalLinkNameForEach(&animals, &do_stuff);
	return EXIT_SUCCESS;
}
