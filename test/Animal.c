#include <stdio.h>	/* *printf */
#include <assert.h>	/* assert */
#include "Orcish.h"
#include "Animal.h"

enum Colour { PINK, RED, BLUE, YELLOW, BEIGE, COLOUR_END };
static const char *const colours[] = { "pink", "red", "blue","yellow","beige" };

/* Abstract class Animal. */
struct AnimalVt;
struct Animal {
 	const struct AnimalVt *vt;
 	char name[16];
	enum Colour colour;
};
#define LIST_NAME Animal
#define LIST_TYPE struct Animal
#include "List.h"

/* Class Sloth extends Animal. */
struct Sloth {
 	struct AnimalListNode animal;
 	unsigned hours_slept;
};
#define POOL_NAME Sloth
#define POOL_TYPE struct Sloth
#define POOL_PARENT struct AnimalList
#include "../src/Pool.h"

/* Class Emu extends Animal. */
struct Emu {
 	struct AnimalListNode animal;
 	char favourite_letter;
};
#define POOL_NAME Emu
#define POOL_TYPE struct Emu
#define POOL_PARENT struct AnimalList
#include "../src/Pool.h"

/* Animal list with backing. */
struct Animals {
	struct AnimalList list;
	struct SlothPool *sloths;
	struct EmuPool *emus;
};
typedef void (*AnimalsAction)(struct Animals *const, struct Animal *const);

/*********/

struct AnimalVt {
	AnimalsAction delete;
 	AnimalAction transmogrify;
};
/** @implements <Animal, [Animals]>BiAction */
static void Animal_delete(struct Animal *const animal,
	void *const void_animals) {
	struct Animals *const animals = void_animals;
	if(!animals || !animal) return;
	animal->vt->delete(animals, animal);
}
static void Sloth_delete(struct Animals *const animals,
	struct Sloth *const sloth) {
	printf("Bye %s.\n", sloth->animal.data.name);
	AnimalListRemove(&sloth->animal.data);
	SlothPoolRemove(animals->sloths, sloth);
}
static void Emu_delete(struct Animals *const animals,
	struct Emu *const emu) {
	printf("Bye %s.\n", emu->animal.data.name);
	AnimalListRemove(&emu->animal.data);
	EmuPoolRemove(animals->emus, emu);
}
/** @implements <Animal>Action */
static void Animal_transmogrify(struct Animal *const animal) {
	if(!animal) return;
	animal->vt->transmogrify(animal);
}
static void Sloth_transmogrify(struct Sloth *const sloth) {
 	printf("Sloth %s has favourite colour %s and has been sleeping %u hours.\n",
		sloth->animal.data.name, colours[sloth->animal.data.colour],
		sloth->hours_slept);
}
static void Emu_transmogrify(struct Emu *const emu) {
 	printf("Emu %s has favourite colour %s and favourite letter %c.\n",
		emu->animal.data.name, colours[emu->animal.data.colour],
		emu->favourite_letter);
}
/* Static data containing the functions defined above. */
static struct AnimalVt Sloth_vt = {
	(AnimalsAction)&Sloth_delete,
	(AnimalAction)&Sloth_transmogrify
};
static struct AnimalVt Emu_vt = {
	(AnimalsAction)&Emu_delete,
	(AnimalAction)&Emu_transmogrify
};

/************/

/** Only called from constructors of children. */
static void Animal_filler(struct Animal *const animal,
	const struct AnimalVt *const vt) {
	assert(animal && vt);
 	animal->vt     = vt;
	animal->colour = (enum Colour)(1.0 * COLOUR_END * rand() / (RAND_MAX +1.0));
	Orcish(animal->name, sizeof animal->name);
}
/** Destructor. */
void Animals_(struct Animals **const animalsp) {
	struct Animals *animals;
	if(!animalsp || !(animals = *animalsp)) return;
	AnimalListClear(&animals->list);
	EmuPool_(&animals->emus);
	SlothPool_(&animals->sloths);
}
/** Constructor. */
struct Animals *Animals(void) {
	struct Animals *a;
	enum { NO, SLOTH, EMU } e = NO;
	if(!(a = malloc(sizeof *a))) { perror("Animals"); Animals_(&a); return 0; }
	AnimalListClear(&a->list);
	a->sloths = 0;
	a->emus   = 0;
	do {
		if(!(a->sloths = SlothPool(&AnimalListMigrate, &a->list)))
			{ e = SLOTH; break; }
		if(!(a->emus = EmuPool(&AnimalListMigrate, &a->list)))
			{ e = EMU; break; }
	} while(0); switch(e) {
		case NO: break;
		case SLOTH:
			fprintf(stderr, "SlothPool: %s.\n", SlothPoolGetError(a->sloths));
			break;
		case EMU:
			fprintf(stderr, "EmuPool: %s.\n", EmuPoolGetError(a->emus));
			break;
	} if(e) Animals_(&a);
	return a;
}
struct Sloth *Sloth(struct Animals *const animals) {
	struct Sloth *sloth;
	if(!animals) return 0;
	if(!(sloth = SlothPoolNew(animals->sloths))) {
		fprintf(stderr, "Sloth: %s.\n", SlothPoolGetError(animals->sloths));
		return 0;
	}
	Animal_filler(&sloth->animal.data, &Sloth_vt);
	sloth->hours_slept = (int)(10.0 * rand() / RAND_MAX) + 4;
	AnimalListPush(&animals->list, &sloth->animal.data);
	return sloth;
}
struct Emu *Emu(struct Animals *const animals) {
	struct Emu *emu;
	if(!animals) return 0;
	if(!(emu = EmuPoolNew(animals->emus))) {
		fprintf(stderr, "Emu: %s.\n", EmuPoolGetError(animals->emus));
		return 0;
	}
	Animal_filler(&emu->animal.data, &Emu_vt);
	emu->favourite_letter = 'a' + (char)(26.0 * rand() / RAND_MAX);
	AnimalListPush(&animals->list, &emu->animal.data);
	return emu;
}
/** @implements <Animal, [size_t *]>BiAction */
static void Animal_count(struct Animal *const animal, void *const pcount) {
	assert(animal && pcount);
	(*(size_t *)pcount)++;
}
void AnimalsTransmogrify(struct Animals *const animals) {
	size_t count = 0;
	if(!animals) return;
	AnimalListBiForEach(&animals->list, &Animal_count, &count);
	printf("There are %lu animals.\n", (long unsigned)count);
	AnimalListForEach(&animals->list, &Animal_transmogrify);
}
void AnimalsClear(struct Animals *const animals) {
	if(!animals) return;
	AnimalListBiForEach(&animals->list, &Animal_delete, animals);
}
