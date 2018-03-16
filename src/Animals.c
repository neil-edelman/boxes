#include <stdio.h>	/* *printf */
#include <assert.h>	/* assert */
#include <string.h>	/* strncpy */
#include "Orcish.h"
#include "Animals.h"

enum Colour { PINK, RED, BLUE, YELLOW, BEIGE, COLOUR_END };
static const char *const colours[] = { "pink", "red", "blue","yellow", "beige"};
enum { BOTTOM, TOP, RIDING_END };

/* Abstract class Animal. */
struct AnimalVt;
struct Animal {
	const struct AnimalVt *vt;
	char name[16];
	enum Colour colour;
};
static const size_t animal_name_size = sizeof ((struct Animal *)0)->name
	/ sizeof *((struct Animal *)0)->name;
static void Animal_to_string(const struct Animal *const animal,
	char (*const a)[12]) {
	strncpy(*a, animal->name, sizeof *a / sizeof **a);
}
#define LIST_NAME Animal
#define LIST_TYPE struct Animal
#define LIST_TO_STRING &Animal_to_string
#include "List.h"

/* Class Mount. */
struct Mount;
struct MountInfo {
	struct Animal *animal;
	struct Mount *steed_of, *riding;
	enum Allowed { STEED = 1, RIDER = 2 } is_allowed; /* Bitfield. */
};
struct Mount {
	struct MountInfo *steed, *rider;
};
/* This needs to have {AnimalVt} defined; define it later. */
static void mount_migrate(struct Mount *const mount,
	const struct Migrate *const migrate);
#define POOL_NAME Mount
#define POOL_TYPE struct Mount
#define POOL_MIGRATE_EACH &mount_migrate
#include "Pool.h"

/* Class Sloth extends Animal. */
struct Sloth {
	struct AnimalListNode animal;
	unsigned hours_slept;
};
static void sloth_migrate(struct Sloth *const this,
	const struct Migrate *const migrate) {
	assert(this && migrate);
	AnimalListNodeMigrate(&this->animal, migrate);
}
#define POOL_NAME Sloth
#define POOL_TYPE struct Sloth
#define POOL_MIGRATE_EACH &sloth_migrate
#include "Pool.h"

/* Class Emu extends Animal. */
struct Emu {
	struct AnimalListNode animal;
	char favourite_letter;
};
static void emu_migrate(struct Emu *const this,
	const struct Migrate *const migrate) {
	assert(this && migrate);
	AnimalListNodeMigrate(&this->animal, migrate);
}
#define POOL_NAME Emu
#define POOL_TYPE struct Emu
#define POOL_MIGRATE_EACH &emu_migrate
#include "Pool.h"

/* Class BadEmu extends Emu. */
struct BadEmu {
	struct Emu emu;
	struct MountInfo mount_info;
	char muhaha[12];
};
static void bad_emu_migrate(struct BadEmu *const this,
	const struct Migrate *const migrate) {
	assert(this && migrate);
	AnimalListNodeMigrate(&this->emu.animal, migrate);
	this->mount_info.animal = &this->emu.animal.data; /* Update pointer. */
	MountPoolMigratePointer(&this->mount_info.steed_of, migrate);
	MountPoolMigratePointer(&this->mount_info.riding, migrate);
}
#define POOL_NAME BadEmu
#define POOL_TYPE struct BadEmu
#define POOL_MIGRATE_EACH &bad_emu_migrate
#include "Pool.h"

/* Class Llama extends Animal. */
struct Llama {
	struct AnimalListNode animal;
	struct MountInfo mount_info;
	unsigned chomps;
};
static void llama_migrate(struct Llama *const this,
	const struct Migrate *const migrate) {
	assert(this && migrate);
	AnimalListNodeMigrate(&this->animal, migrate);
	this->mount_info.animal = &this->animal.data; /* Update pointer. */
	MountPoolMigratePointer(&this->mount_info.steed_of, migrate);
	MountPoolMigratePointer(&this->mount_info.riding, migrate);
}
#define POOL_NAME Llama
#define POOL_TYPE struct Llama
#define POOL_MIGRATE_EACH &llama_migrate
#include "Pool.h"

/* Class Lemur extends Animal. */
struct Lemur {
	struct AnimalListNode animal;
	struct MountInfo mount_info;
};
static void lemur_migrate(struct Lemur *const this,
	const struct Migrate *const migrate) {
	assert(this && migrate);
	AnimalListNodeMigrate(&this->animal, migrate);
	this->mount_info.animal = &this->animal.data; /* Update pointer. */
	MountPoolMigratePointer(&this->mount_info.steed_of, migrate);
	MountPoolMigratePointer(&this->mount_info.riding, migrate);
}
#define POOL_NAME Lemur
#define POOL_TYPE struct Lemur
#define POOL_MIGRATE_EACH &lemur_migrate
#include "Pool.h"

/* Class Bear extends Animal. We have always two or less, so we don't need to
 define a {Pool}. */
struct Bear {
	struct AnimalListNode animal;
	int is_active;
	struct MountInfo mount_info;
};

/* Animal list with backing. These are the storage structures. */
struct Animals {
	struct AnimalList list;
	struct MountPool *mounts;
	struct SlothPool *sloths;
	struct EmuPool *emus;
	struct BadEmuPool *bad_emus;
	struct LlamaPool *llamas;
	struct LemurPool *lemurs;
	struct Bear bears[2];
};
static const unsigned no_bears = sizeof(((struct Animals *)0)->bears)
	/ sizeof(*((struct Animals *)0)->bears);

typedef void (*AnimalAction)(struct Animal *const);
typedef void (*AnimalsAction)(struct Animals *const, struct Animal *const);
typedef struct MountInfo *(*AnimalMountInfo)(struct Animal *const);

/*********/

struct AnimalVt {
	const char type[16];
	AnimalsAction delete;
	AnimalAction act/*transmogrify*/;
	AnimalMountInfo mount_info;
};

static void dismount(struct Mount *const mount);

/** @implements <Animal, [Animals]>BiAction */
static void Animal_delete(struct Animal *const animal,
	void *const void_animals) {
	struct Animals *const animals = void_animals;
	struct MountInfo *mount_info;
	if(!animals || !animal) return;
	if((mount_info = animal->vt->mount_info(animal))) {
		if(mount_info->steed_of) dismount(mount_info->steed_of);
		if(mount_info->riding)   dismount(mount_info->riding);
	}
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
	printf("%s %s has favourite colour %s and has been sleeping %u hours.\n",
		sloth->animal.data.vt->type, sloth->animal.data.name,
		colours[sloth->animal.data.colour], sloth->hours_slept);
}
static void Emu_act(struct Emu *const emu) {
	printf("%s %s has favourite colour %s and favourite letter %c.\n",
		emu->animal.data.vt->type, emu->animal.data.name,
		colours[emu->animal.data.colour], emu->favourite_letter);
}
static void BadEmu_act(struct BadEmu *const bad_emu) {
	char ride[128] = "";
	if(bad_emu->mount_info.riding) {
		const struct Animal *const steed
			= bad_emu->mount_info.riding->steed->animal;
		sprintf(ride, " They are riding on %s the %s.",
			steed->name, steed->vt->type);
	}
	printf("%s %s has favourite colour %s and favourite letter %c; "
		"he is mumbling \"%s.\"%s\n", bad_emu->emu.animal.data.vt->type,
		bad_emu->emu.animal.data.name, colours[bad_emu->emu.animal.data.colour],
		bad_emu->emu.favourite_letter, bad_emu->muhaha, ride);
}
static void Lemur_act(struct Lemur *const lemur) {
	char ride[128] = "";
	if(lemur->mount_info.riding) {
		const struct Animal *const steed
			= lemur->mount_info.riding->steed->animal;
		sprintf(ride, " They are riding on %s the %s.",
			steed->name, steed->vt->type);
	}
	printf("%s %s.%s\n", lemur->animal.data.vt->type, lemur->animal.data.name,
		ride);
}
static void Llama_act(struct Llama *const llama) {
	char ride[128] = "";
	if(llama->mount_info.steed_of) {
		const struct Animal *const rider
			= llama->mount_info.steed_of->rider->animal;
		sprintf(ride, " They are the noble steed of %s the %s.",
			rider->name, rider->vt->type);
	}
	printf("%s %s has chomped %u fingers today.%s\n",
		llama->animal.data.vt->type, llama->animal.data.name, llama->chomps,
		ride);
}
static void Bear_act(struct Bear *const bear) {
	char ride[128] = "";
	if(bear->mount_info.riding) {
		const struct Animal *const steed
			= bear->mount_info.riding->steed->animal;
		sprintf(ride + strlen(ride), " riding on %s the %s",
			steed->name, steed->vt->type);
	}
	if(bear->mount_info.steed_of) {
		const struct Animal *const rider
			= bear->mount_info.steed_of->rider->animal;
		sprintf(ride + strlen(ride), " being ridden by %s the %s",
			rider->name, rider->vt->type);
	}
	if(*ride == '\0') strcpy(ride, " chilling");
	printf("%s %s is%s.\n", bear->animal.data.vt->type, bear->animal.data.name,
		ride);
}

/** @implements AnimalMountField */
static struct MountInfo *Animal_mount(struct Animal *const animal) {
	assert(animal);
	return animal->vt->mount_info(animal);
}
static struct MountInfo *no_mount_info(struct Animal *const animal) {
	(void)animal;
	return 0;
}
static struct MountInfo *BadEmu_mount(struct BadEmu *const bad_emu) {
	return &bad_emu->mount_info;
}
static struct MountInfo *Lemur_mount(struct Lemur *const lemur) {
	return &lemur->mount_info;
}
static struct MountInfo *Llama_mount(struct Llama *const llama) {
	return &llama->mount_info;
}
static struct MountInfo *Bear_mount(struct Bear *const bear) {
	return &bear->mount_info;
}

/* Static data containing the functions defined above. */
static struct AnimalVt Sloth_vt = {
	"Sloth",
	(AnimalsAction)&Sloth_delete,
	(AnimalAction)&Sloth_act,
	&no_mount_info
};
static struct AnimalVt Emu_vt = {
	"Emu",
	(AnimalsAction)&Emu_delete,
	(AnimalAction)&Emu_act,
	&no_mount_info
};
static struct AnimalVt BadEmu_vt = {
	"Emu",
	(AnimalsAction)&BadEmu_delete,
	(AnimalAction)&BadEmu_act,
	(AnimalMountInfo)&BadEmu_mount
};
static struct AnimalVt Lemur_vt = {
	"Lemur",
	(AnimalsAction)&Lemur_delete,
	(AnimalAction)&Lemur_act,
	(AnimalMountInfo)&Lemur_mount
};
static struct AnimalVt Llama_vt = {
	"Llama",
	(AnimalsAction)&Llama_delete,
	(AnimalAction)&Llama_act,
	(AnimalMountInfo)&Llama_mount
};
static struct AnimalVt Bear_vt = {
	"Bear",
	(AnimalsAction)&Bear_delete,
	(AnimalAction)&Bear_act,
	(AnimalMountInfo)&Bear_mount
};

/** From before, waiting until {AnimalVt.mount_field} and {Animal_mount} was
 defined. This follows the links to {steed} and {rider} and makes them point
 back to the new memory location of {mount}.
 @implements <Mount>Migrate */
static void mount_migrate(struct Mount *const mount,
	const struct Migrate *const migrate) {
	assert(mount && migrate && mount->steed && mount->rider);
	MountPoolMigratePointer(&Animal_mount(mount->steed->animal)->steed_of,
		migrate);
	MountPoolMigratePointer(&Animal_mount(mount->rider->animal)->riding,
		migrate);
}

/** Helper for delete. */
static void dismount(struct Mount *const mount) {
	assert(mount && mount->steed && mount->rider);
	printf("%s the %s dismounts %s the %s.\n", mount->rider->animal->name,
		mount->rider->animal->vt->type, mount->steed->animal->name,
		mount->steed->animal->vt->type);
	mount->steed->steed_of = 0;
	mount->rider->riding = 0;
	/*MountPoolRemove(mount); @fixme!! */
}

/************/

/** Only called from constructors of children. */
static void Animal_filler(struct Animal *const animal,
	const struct AnimalVt *const vt) {
	assert(animal && vt);
	animal->vt     = vt;
	animal->colour = (enum Colour)(1.0 * COLOUR_END * rand() / (RAND_MAX +1.0));
	Orcish(animal->name, sizeof animal->name);
}
/** Mixin? Kind of. */
static void MountInfo_filler(struct MountInfo *const this,
	struct Animal *const animal, const enum Allowed is_allowed) {
	assert(this && animal && animal->vt->mount_info(animal) == this);
	this->animal = animal;
	this->steed_of = this->riding = 0;
	this->is_allowed = is_allowed;
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
	struct Bear *bear, *end;
	int is_success = 0;
	const char *e = "null";
	if(!(a = malloc(sizeof *a))) { perror("Animals"); Animals_(&a); return 0; }
	AnimalListClear(&a->list);
	a->mounts   = 0;
	a->sloths   = 0;
	a->emus     = 0;
	a->bad_emus = 0;
	a->llamas   = 0;
	a->lemurs   = 0;
	for(bear = a->bears, end = bear + no_bears; bear < end; bear++)
		bear->is_active = 0;
	errno = 0; do {
		if(!(e = "mount", a->mounts = MountPool())
			|| !(e = "sloths", a->sloths = SlothPool())
			|| !(e = "emus", a->emus = EmuPool())
			|| !(e = "bad_emus", a->bad_emus = BadEmuPool())
			|| !(e = "llamas", a->llamas = LlamaPool())
			|| !(e = "lemurs", a->lemurs = LemurPool())
		) break;
		is_success = 1;
	} while(0); if(!is_success) {
		fprintf(stderr, "Animals, constructing %s: %s.\n",
			e, strerror(errno));
		Animals_(&a);
	}
	return a;
}
struct Sloth *Sloth(struct Animals *const animals) {
	struct Sloth *sloth;
	if(!animals) return 0;
	if(!(sloth = SlothPoolNew(animals->sloths))) return 0;
	Animal_filler(&sloth->animal.data, &Sloth_vt);
	sloth->hours_slept = (int)(10.0 * rand() / RAND_MAX) + 4;
	AnimalListPush(&animals->list, &sloth->animal.data);
	return sloth;
}
struct Emu *Emu(struct Animals *const animals) {
	struct Emu *emu;
	if(!animals) return 0;
	if(!(emu = EmuPoolNew(animals->emus))) return 0;
	Animal_filler(&emu->animal.data, &Emu_vt);
	emu->favourite_letter = 'a' + (char)(26.0 * rand() / RAND_MAX);
	AnimalListPush(&animals->list, &emu->animal.data);
	return emu;
}
struct BadEmu *BadEmu(struct Animals *const animals) {
	struct BadEmu *emu;
	if(!animals) return 0;
	if(!(emu = BadEmuPoolNew(animals->bad_emus))) return 0;
	Animal_filler(&emu->emu.animal.data, &BadEmu_vt);
	emu->emu.favourite_letter = 'a' + (char)(26.0 * rand() / RAND_MAX);
	MountInfo_filler(&emu->mount_info, &emu->emu.animal.data, RIDER);
	Orcish(emu->muhaha, sizeof emu->muhaha);
	AnimalListPush(&animals->list, &emu->emu.animal.data);
	return emu;
}
struct Llama *Llama(struct Animals *const animals) {
	struct Llama *llama;
	if(!animals) return 0;
	if(!(llama = LlamaPoolNew(animals->llamas))) return 0;
	Animal_filler(&llama->animal.data, &Llama_vt);
	MountInfo_filler(&llama->mount_info, &llama->animal.data, STEED);
	llama->chomps = 5 + 10 * rand() / RAND_MAX;
	AnimalListPush(&animals->list, &llama->animal.data);
	return llama;
}
struct Lemur *Lemur(struct Animals *const animals) {
	struct Lemur *lemur;
	if(!animals) return 0;
	if(!(lemur = LemurPoolNew(animals->lemurs))) return 0;
	Animal_filler(&lemur->animal.data, &Lemur_vt);
	MountInfo_filler(&lemur->mount_info, &lemur->animal.data, RIDER);
	AnimalListPush(&animals->list, &lemur->animal.data);
	return lemur;
}
struct Bear *Bear(struct Animals *const animals, const unsigned no,
	const char *const name) {
	struct Bear *bear;
	if(!animals || no >= no_bears) return 0;
	bear = animals->bears + no;
	if(bear->is_active) { fprintf(stderr, "Bear is active.\n"); return 0; }
	Animal_filler(&bear->animal.data, &Bear_vt);
	if(name) strncpy(bear->animal.data.name, name, animal_name_size - 1),
		bear->animal.data.name[animal_name_size - 1] = '\0';
	bear->is_active = 1;
	MountInfo_filler(&bear->mount_info, &bear->animal.data, STEED | RIDER);
	AnimalListPush(&animals->list, &bear->animal.data);
	return bear;
}

/** Cause {a} to ride {b}. If {a} or {b} is null, causes that connection to be
 broken.
 @return Success. */
int AnimalsRide(struct Animals *const animals, struct Animal *const a,
	struct Animal *const b) {
	struct Animal *erase;
	struct MountInfo *steed = 0, *rider = 0;
	struct Mount *mount;
	if(!animals || (!a && !b)) return 0;
	erase = a ? b ? 0 : a : 0;
	if(erase) {
		/* One was null. */
		const struct MountInfo *const mi = erase->vt->mount_info(erase);
		if(!mi) return fprintf(stderr, "Animal %s the %s does not have mount "
			"information.\n", erase->name, erase->vt->type), 0;
		if(mi->riding)   dismount(mi->riding);
		if(mi->steed_of) dismount(mi->steed_of);
		assert(!mi->steed_of && !mi->riding);
		return 1;
	} else {
		/* Both are full. */
		struct MountInfo *const ami = a->vt->mount_info(a),
			*const bmi = b->vt->mount_info(b);
		if(!ami || !bmi) {
		} else if((ami->is_allowed & RIDER) && !ami->riding
			&& (bmi->is_allowed & STEED) && !bmi->steed_of) {
			steed = bmi, rider = ami;
		} else if((ami->is_allowed & STEED) && !ami->steed_of
			&& (bmi->is_allowed & RIDER) && !bmi->riding) {
			steed = ami, rider = bmi;
		}
	}
	if(!steed || !rider) return /*fprintf(stderr, "Animal %s the %s and "
		"%s the %s do not understand your mount in this configuration.\n",
		a->name, a->vt->type, b->name, b->vt->type),*/ 0;
	/* {steed} and {rider} are good. */
	mount = MountPoolNew(animals->mounts);
	mount->steed = steed, steed->steed_of = mount;
	mount->rider = rider, rider->riding   = mount;
	printf("%s the %s mounts %s the %s.\n", rider->animal->name,
		rider->animal->vt->type, steed->animal->name, steed->animal->vt->type);
	return 1;
	
}
/** @implements <Animal, [size_t *]>BiAction */
static void Animal_count(struct Animal *const animal, void *const pcount) {
	assert(animal && pcount);
	(*(size_t *)pcount)++;
}
void AnimalsAct(struct Animals *const animals) {
	size_t count = 0;
	if(!animals) return;
	AnimalListBiForEach(&animals->list, &Animal_count, &count);
	printf("There are %lu animals.\n", (long unsigned)count);
	AnimalListForEach(&animals->list, &Animal_act);
}
void AnimalsClear(struct Animals *const animals) {
	if(!animals) return;
	AnimalListBiForEach(&animals->list, &Animal_delete, animals);
}
struct Animal *AnimalsFirst(struct Animals *const animals) {
	if(!animals) return 0;
	return AnimalListFirst(&animals->list);
}
struct Animal *AnimalsNext(struct Animal *const animal) {
	if(!animal) return 0;
	return AnimalListNext(animal);
}
