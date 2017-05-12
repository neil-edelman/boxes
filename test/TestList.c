/** Unit test of List.c.

 @file		TestList
 @author	Neil
 @std		C89/90
 @version	1.0; 2017-05
 @since		1.0; 2017-05 salvaged from TestList.h */

#include <stdlib.h> /* EXIT_ malloc free */
#include <stdio.h>  /* fprintf */
#include <string.h>	/* strcmp */
#include <time.h>	/* clock */
#include <limits.h>	/* INT_MAX */
#include "Orcish.h"

#ifdef __clang__ /* <-- clang must be placed ahead of __GNUC__ */
#pragma clang diagnostic ignored "-Wpadded"
#pragma clang diagnostic ignored "-Wdocumentation"
#pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
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

/** Define class {Foo} */
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
/** Assumes {key} is {[0, 99]}.
 @implements <Foo>ToString */
static void Foo_to_string(const struct Foo *this, char (*const a)[12]) {
	sprintf(*a, "%d%.9s", this->key, this->value);
}
/** @implements <Foo>Action */
static void Foo_filler(struct Foo *const this) {
	this->key = (int)(float)(rand() / (RAND_MAX + 1.0) * 99.0);
	Orcish(this->value, sizeof this->value);
}
#define LIST_NAME Foo
#define LIST_TYPE struct Foo
#define LIST_A_NAME Key
#define LIST_A_COMPARATOR &Foo_key_cmp
#define LIST_B_NAME Value
#define LIST_B_COMPARATOR &Foo_value_cmp
#define LIST_TO_STRING &Foo_to_string
#define LIST_TEST &Foo_filler
#include "../src/List.h"

/* Class {Int} is a single {int}. */
/** @implements <Int>Comparator */
static int Int_N_cmp(const int *a, const int *b) {
	return (*b < *a) - (*a < *b);
}
/** Assumes {[-9 999 999 999, 99 999 999 999]}.
 @implements <Int>ToString */
static void Int_to_string(const int *this, char (*const a)[12]) {
	sprintf(*a, "%d", *this);
}
#if INT_MAX > 9999999999
#define LIST_NUM_MAX 9999999999
#else
#define LIST_NUM_MAX INT_MAX
#endif
/** @implements <Int>Action */
static void Int_filler(int *const this) {
	*this = (int)(float)((2.0 * rand() / (RAND_MAX + 1.0) - 1.0) *LIST_NUM_MAX);
}
#undef LIST_NUM_MAX
#define LIST_NAME Int
#define LIST_TYPE int
#define LIST_A_NAME N
#define LIST_A_COMPARATOR &Int_N_cmp
#define LIST_TO_STRING &Int_to_string
#define LIST_TEST &Int_filler
#include "../src/List.h"

/* Class {Colour} is an {enum}. */
enum Colour { White, Silver, Gray, Black, Red, Maroon, Bisque, Wheat, Tan,
	Sienna, Brown, Yellow, Khaki, Gold, Olive, Lime, Green, Aqua, Cyan, Teal,
	Salmon, Orange, Powder, Sky, Steel, Royal, Blue, Navy, Fuchsia, Pink,
	Purple };
static const char *const colour_names[] = { "White", "Silver", "Gray", "Black",
	"Red", "Maroon", "Bisque", "Wheat", "Tan", "Sienna", "Brown", "Yellow",
	"Khaki", "Gold", "Olive", "Lime", "Green", "Aqua", "Cyan", "Teal",
	"Salmon", "Orange", "Powder", "Sky", "Steel", "Royal", "Blue", "Navy",
	"Fuchsia", "Pink", "Purple" }; /* max 11 letters */
static const size_t colour_size = sizeof colour_names / sizeof *colour_names;
/** @implements <Colour>ToString */
static void Colour_to_string(const enum Colour *this, char (*const a)[12]) {
	sprintf(*a, "%s", colour_names[*this]);
}
/** @implements <Colour>Action */
static void Colour_filler(enum Colour *const this) {
	*this = (enum Colour)(float)(rand() / (RAND_MAX + 1.0) * colour_size);
}
#define LIST_NAME Colour
#define LIST_TYPE enum Colour
#define LIST_A_NAME Declare
#define LIST_TO_STRING &Colour_to_string
#define LIST_TEST &Colour_filler
#include "../src/List.h"

/** {Animal} virtual functions. */
struct AnimalVt;
/** Define class {Animal}. */
struct Animal {
	const struct AnimalVt *vt;
	int x;
	char name[16];
};
/** @implements <Animal>Comparator */
static int Animal_name_cmp(const struct Animal *a, const struct Animal *b) {
	return strcmp(a->name, b->name);
}
/** @implements <Animal>Comparator */
static int Animal_x_cmp(const struct Animal *a, const struct Animal *b) {
	return (b->x < a->x) - (a->x < b->x);
}
/** Assumes {x \in [-99, 999]}.
 @implements <Animal>ToString */
static void Animal_to_string(const struct Animal *this, char (*const a)[12]) {
	sprintf(*a, "%d%.8s", this->x, this->name);
}
/* @implements <Animal>Action */
static void Animal_filler(struct Animal *const this) {
	Orcish(this->name, sizeof this->name);
	this->vt = 0;
	this->x  = (int)(198.0 * rand() / (RAND_MAX + 1.0) - 99.0);
}
#define LIST_NAME Animal
#define LIST_TYPE struct Animal
#define LIST_C_NAME Name
#define LIST_C_COMPARATOR &Animal_name_cmp
#define LIST_D_NAME X
#define LIST_D_COMPARATOR &Animal_x_cmp
#define LIST_TO_STRING &Animal_to_string
#define LIST_TEST &Animal_filler
#include "../src/List.h"
struct Sloth {
	struct AnimalListNode animal;
	unsigned lazy;
};
struct Llama {
	struct AnimalListNode animal;
	unsigned chomps;
};
struct Bear {
	struct AnimalListNode animal;
	struct Animal *riding;
};
/** @implements <Animal>Action */
static void sloth_act(struct Animal *const animal) {
	struct Sloth *const sloth = (struct Sloth *)animal;
	printf("Sloth %s at %d has been sleeping %u hours.\n",
		animal->name, animal->x, sloth->lazy);
}
/** @implements <Animal>Action */
static void llama_act(struct Animal *const animal) {
	struct Llama *const llama = (struct Llama *)animal;
	printf("Llama %s at %d has chomped %u fingers today.\n",
		animal->name, animal->x, llama->chomps);
}
/** @implements <Animal>Action */
static void bear_act(struct Animal *const animal) {
	struct Bear *const bear = (struct Bear *)animal;
	printf("Bear %s at %d is riding on llama %s.\n", animal->name, animal->x,
		bear->riding->name);
}
static const struct AnimalVt {
	void (*const act)(struct Animal *const);
} sloth_vt = { &sloth_act }, llama_vt = { &llama_act }, bear_vt = { &bear_act };
/* the linked-list */
static struct AnimalList animals;
static void Animal_init(struct AnimalListNode *const this) {
	Animal_filler((struct Animal *)this);
	AnimalListAdd(&animals, this);
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
static void Bear_init(struct Bear *const bear, struct Llama *const llamas,
	const size_t llamas_size) {
	Animal_init(&bear->animal);
	bear->animal.data.vt = &bear_vt;
	bear->riding = (struct Animal *)(llamas
		+ (unsigned)((double)llamas_size * rand() / (RAND_MAX + 1.0)));
	/* Overwrite to make the Bear and the Llama the same. */
	bear->animal.data.x = bear->riding->x;
}
/* @implements AnimalAction */
static void act(struct Animal *const this) {
	this->vt->act(this);
}
/** Test BlockMove. */
static void test_block_move(void) {
	struct Sloth sloths[3];
	const size_t sloths_size = sizeof sloths / sizeof *sloths;
	struct Llama llamas[6];
	const size_t llamas_size = sizeof llamas / sizeof *llamas;
	struct Bear bears[2];
	const size_t bears_size = sizeof bears / sizeof *bears;
	struct Sloth others[9];
	const size_t others_size = sizeof others / sizeof *others;
	size_t i;

	printf("Llama test:\n");
	assert(others_size >= sloths_size);
	for(i = 0; i < sloths_size; i++) Sloth_init(sloths + i);
	for(i = 0; i < llamas_size; i++) Llama_init(llamas + i);
	for(i = 0; i < bears_size; i++)  Bear_init(bears + i, llamas, llamas_size);
	printf("Unsorted: by name %s; by x %s.\n", AnimalListNameToString(&animals),
		AnimalListXToString(&animals));
	AnimalListSort(&animals);
	printf("Sorted: by name %s; by x %s.\n", AnimalListNameToString(&animals),
		AnimalListXToString(&animals));
	printf("By name:\n");
	AnimalListNameForEach(&animals, &act);
	printf("By x:\n");
	AnimalListXForEach(&animals, &act);
	list_Animal_in_order(&animals);
	memcpy(others, sloths, sizeof sloths);
	for(i = 0; i < sloths_size; i++) sloths[i].animal.data.name[0] = '!';
	printf("Moved sloths: %s.\n", AnimalListNameToString(&animals));
	AnimalListMigrateBlock(&animals, others, sizeof sloths, sloths);
	printf("Block move: %s.\n", AnimalListNameToString(&animals));
	for(i = sloths_size; i < others_size; i++) Sloth_init(others + i);
	printf("New sloths: %s.\n", AnimalListNameToString(&animals));
	AnimalListSort(&animals);
	printf("Sorted: by name %s; by x %s.\n", AnimalListNameToString(&animals),
		AnimalListXToString(&animals));
	printf("By name:\n");
	AnimalListNameForEach(&animals, &act);
	printf("By x:\n");
	AnimalListXForEach(&animals, &act);
	list_Animal_in_order(&animals);
}

/** Entry point.
 @return Either EXIT_SUCCESS or EXIT_FAILURE. */
int main(void) {
	unsigned seed = (unsigned)clock();

	srand(seed), rand(), printf("Seed %u.\n", seed);
	FooListTest();
	IntListTest();
	ColourListTest();
	AnimalListTest();
	test_block_move();
	printf("Test succeeded.\n\n");

	return EXIT_SUCCESS;
}
