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
#include "Pool.h"

/* Class Emu extends Animal. */
struct Emu {
	struct AnimalListNode animal;
	char favourite_letter;
};
#define POOL_NAME Emu
#define POOL_TYPE struct Emu
#define POOL_PARENT struct AnimalList
#include "Pool.h"

/* Class Riding. */
struct RidingVt;
struct Riding {
	const struct RidingVt *vt;
	struct Animal *riding[2];
};
#define POOL_NAME Riding
#define POOL_TYPE struct Riding
#define POOL_PARENT struct AnimalList
#include "Pool.h"

/* Class BadEmu extends Emu. */
struct BadEmu {
	struct Emu emu;
	struct Riding *riding;
	char muhaha[12];
};
#define POOL_NAME BadEmu
#define POOL_TYPE struct BadEmu
#define POOL_PARENT struct AnimalList
#include "Pool.h"

/* Class Llama extends Animal. */
struct Llama {
	struct AnimalListNode animal;
	struct Riding *riding;
	unsigned chomps;
};
#define POOL_NAME Emu
#define POOL_TYPE struct Emu
#define POOL_PARENT struct AnimalList
#include "Pool.h"

/* Class Lemur extends Animal. */
struct Lemur {
	struct AnimalListNode animal;
	struct Riding *riding;
};
#define POOL_NAME Lemur
#define POOL_TYPE struct Lemur
#define POOL_PARENT struct AnimalList
#include "Pool.h"

/* Class Bear extends Animal. We have always two or less, so we don't need to
 define a {Pool}. */
struct Bear {
	struct AnimalListNode animal;
	int is_active;
	struct Riding *riding;
};

/* Animal list with backing. These are the storage structures. */
struct Animals {
	struct AnimalList list;
	struct SlothPool *sloths;
	struct EmuPool *emus;
	struct RidingPool *riding;
	struct BadEmuPool *bad_emus;
	struct LlamaPool *llamas;
	struct LemurPool *lemurs;
	struct Bear bears[2];
};

typedef void (*AnimalAction)(struct Animal *const);
typedef void (*AnimalsAction)(struct Animals *const, struct Animal *const);

/*********/

struct AnimalVt {
	AnimalsAction delete;
	AnimalAction act/*transmogrify*/;
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
static void BadEmu_delete(struct Animals *const animals,
	struct BadEmu *const bad_emu) {
	printf("%s dissapers in a puff of smoke.\n", bad_emu->emu.animal.data.name);
	AnimalListRemove(&bad_emu->emu.animal.data);
	BadEmuPoolRemove(animals->bad_emus, bad_emu);
}
static void Lemur_delete(struct Animals *const animals,
	struct Lemur *const lemur) {
	printf("Bye %s.\n", lemur->animal.data.name);
	AnimalListRemove(&lemur->animal.data);
	LemurPoolRemove(animals->lemurs, lemur);
}
static void Llama_delete(struct Animals *const animals,
	struct Llama *const llama) {
	printf("Bye %s.\n", llama->animal.data.name);
	AnimalListRemove(&llama->animal.data);
	LlamaPoolRemove(animals->llamas, llama);
}
static void Bear_delete(struct Animals *const animals,
	struct Bear *const bear) {
	if(!bear->is_active) return;
	printf("Bye %s.\n", bear->animal.data.name);
	AnimalListRemove(&bear->animal.data);
	bear->is_active = 0;
	(void)animals;
}

/** @implements <Animal>Action */
static void Animal_act(struct Animal *const animal) {
	assert(animal);
	animal->vt->act(animal);
}
static void Sloth_act(struct Sloth *const sloth) {
	printf("Sloth %s has favourite colour %s and has been sleeping %u hours.\n",
		sloth->animal.data.name, colours[sloth->animal.data.colour],
		sloth->hours_slept);
}
static void Emu_act(struct Emu *const emu) {
	printf("Emu %s has favourite colour %s and favourite letter %c.\n",
		emu->animal.data.name, colours[emu->animal.data.colour],
		emu->favourite_letter);
}
static void BadEmu_act(struct BadEmu *const bad_emu) {
	char riding[256] = ;
	printf("Emu %s has favourite colour %s and favourite letter %c; "
		"he is mumbling \"%s.\"%s\n", bad_emu->emu.animal.data.name,
		colours[bad_emu->emu.animal.data.colour], bad_emu->emu.favourite_letter
		bad_emu->muhaha, riding);
}
static void Lemur_act(struct Lemur *const lemur) {
	printf("Lemur %s.\n", lemur->animal.data.name);
}
static void llama_act(struct Animal *const animal) {
	struct Llama *const llama = (struct Llama *)animal;
	printf("Llama %s at %d has chomped %u fingers today.\n",
		animal->name, animal->x, llama->chomps);
}
static void bear_act(struct Animal *const animal) {
	struct Bear *const bear = (struct Bear *)animal;
	printf("Bear %s at %d is riding on llama %s.\n", animal->name, animal->x,
		   bear->riding ? bear->riding->name : "no llama");
}

/* Static data containing the functions defined above. */
static struct AnimalVt Sloth_vt = {
	(AnimalsAction)&Sloth_delete,
	(AnimalAction)&Sloth_act
};
static struct AnimalVt Emu_vt = {
	(AnimalsAction)&Emu_delete,
	(AnimalAction)&Emu_act
};
static struct AnimalVt BadEmu_vt = {
	(AnimalsAction)&BadEmu_delete,
	(AnimalAction)&BadEmu_act
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
	int e;
	if(!(a = malloc(sizeof *a))) { perror("Animals"); Animals_(&a); return 0; }
	AnimalListClear(&a->list);
	a->sloths = 0;
	a->emus   = 0;
	e = errno = 0; do {
		if(!(a->sloths = SlothPool(&AnimalListMigrate, &a->list))) break;
		if(!(a->emus = EmuPool(&AnimalListMigrate, &a->list))) break;
	} while(0); if((e = errno)) {
		perror("Animals");
	} if(e) Animals_(&a);
	return a;
}
struct Sloth *Sloth(struct Animals *const animals) {
	struct Sloth *sloth;
	if(!animals) return 0;
	if(!(sloth = SlothPoolNew(animals->sloths))) { perror("Sloth"); return 0; }
	Animal_filler(&sloth->animal.data, &Sloth_vt);
	sloth->hours_slept = (int)(10.0 * rand() / RAND_MAX) + 4;
	AnimalListPush(&animals->list, &sloth->animal.data);
	return sloth;
}
struct Emu *Emu(struct Animals *const animals) {
	struct Emu *emu;
	if(!animals) return 0;
	if(!(emu = EmuPoolNew(animals->emus))) { perror("Sloth"); return 0; }
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
