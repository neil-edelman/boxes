/** Animals is polymoric class that includes several animal types. There is
 also an interface-thing, where you can mount certain animals. For a more
 detailed expliation, see
 \url{http://neil.chaosnet.org/code/Ctools/tests/Animals/}.

 @title Animals
 @author Neil
 @std C89 */

#include <stdio.h>	/* *printf */
#include <assert.h>	/* assert */
#include <string.h>	/* strncpy */
#include "Orcish.h"
#include "Animals.h"

/*#define ARRAY*/
/*#define OLD*/

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
	strncpy(*a, animal->name, sizeof *a / sizeof **a - 1);
	*a[sizeof *a / sizeof **a - 1] = '\0';
}
#define LIST_NAME Animal
#define LIST_TYPE struct Animal
#define LIST_TO_STRING &Animal_to_string
#include "List.h"

/* Class Mount. */
struct Mount;
struct MountInfo {
	struct Mount *steed_of, *riding;
	enum Allowed { STEED = 1, RIDER = 2 } is_allowed; /* Bit-field. */
};
struct Mount {
	struct Animal *steed, *rider;
};
#if defined(OLD) || defined(ARRAY) /* <-- migrate */
/* This needs to have {AnimalVt} defined; define it later. */
static void mount_migrate(struct Mount *const mount,
	const struct Migrate *const migrate);
#endif /* migrate --> */
#ifdef ARRAY /* <-- array */
#define ARRAY_NAME Mount
#define ARRAY_TYPE struct Mount
#define ARRAY_FREE_LIST
#define ARRAY_MIGRATE_EACH &mount_migrate
#include "Array.h"
#else /* array --><-- !array */
#define POOL_NAME Mount
#define POOL_TYPE struct Mount
#ifdef OLD
#define POOL_MIGRATE_EACH &mount_migrate
#include "Pool.h"
#else
#include "StablePool.h"
#endif
#endif /* !array --> */

/* Class Sloth extends Animal. */
struct Sloth {
	struct AnimalLink animal;
	unsigned hours_slept;
};
#if defined(OLD) || defined(ARRAY) /* <-- migrate */
static void sloth_migrate(struct Sloth *const this,
	const struct Migrate *const migrate) {
	assert(this && migrate);
	AnimalLinkMigrate(&this->animal.data, migrate);
}
#endif /* migrate --> */
#ifdef ARRAY /* <-- array */
#define ARRAY_NAME Sloth
#define ARRAY_TYPE struct Sloth
#define ARRAY_FREE_LIST
#define ARRAY_MIGRATE_EACH &sloth_migrate
#include "Array.h"
#else /* array --><-- !array */
#define POOL_NAME Sloth
#define POOL_TYPE struct Sloth
#ifdef OLD
#define POOL_MIGRATE_EACH &sloth_migrate
#include "Pool.h"
#else
#include "StablePool.h"
#endif
#endif /* !array --> */

/* Class Emu extends Animal. */
struct Emu {
	struct AnimalLink animal;
	char favourite_letter;
};
#if defined(OLD) || defined(ARRAY) /* <-- migrate */
static void emu_migrate(struct Emu *const this,
	const struct Migrate *const migrate) {
	assert(this && migrate);
	AnimalLinkMigrate(&this->animal.data, migrate);
}
#endif /* migrate --> */
#ifdef ARRAY /* <-- array */
#define ARRAY_NAME Emu
#define ARRAY_TYPE struct Emu
#define ARRAY_FREE_LIST
#define ARRAY_MIGRATE_EACH &emu_migrate
#include "Array.h"
#else /* array --><-- !array */
#define POOL_NAME Emu
#define POOL_TYPE struct Emu
#ifdef OLD
#define POOL_MIGRATE_EACH &emu_migrate
#include "Pool.h"
#else
#include "StablePool.h"
#endif
#endif /* !array --> */

/* Class BadEmu extends Emu. */
struct BadEmu {
	struct Emu emu;
	struct MountInfo mount_info;
	char muhaha[12];
};
#if defined(OLD) || defined(ARRAY) /* <-- migrate */
static void bad_emu_migrate(struct BadEmu *const this,
	const struct Migrate *const migrate) {
	assert(this && migrate);
	AnimalLinkMigrate(&this->emu.animal.data, migrate);
#ifdef OLD
	MountPoolMigratePointer(&this->mount_info.steed_of, migrate);
	MountPoolMigratePointer(&this->mount_info.riding, migrate);
#else
	MountArrayMigratePointer(&this->mount_info.steed_of, migrate);
	MountArrayMigratePointer(&this->mount_info.riding, migrate);
#endif
}
#endif /* migrate --> */
#ifdef ARRAY /* <-- array */
#define ARRAY_NAME BadEmu
#define ARRAY_TYPE struct BadEmu
#define ARRAY_FREE_LIST
#define ARRAY_MIGRATE_EACH &bad_emu_migrate
#include "Array.h"
#else /* array --><-- !array */
#define POOL_NAME BadEmu
#define POOL_TYPE struct BadEmu
#ifdef OLD
#define POOL_MIGRATE_EACH &bad_emu_migrate
#include "Pool.h"
#else
#include "StablePool.h"
#endif
#endif /* !array --> */

/* Class Llama extends Animal. */
struct Llama {
	struct AnimalLink animal;
	struct MountInfo mount_info;
	unsigned chomps;
};
#if defined(OLD) || defined(ARRAY) /* <-- migrate */
static void llama_migrate(struct Llama *const this,
	const struct Migrate *const migrate) {
	assert(this && migrate);
	AnimalLinkMigrate(&this->animal.data, migrate);
#ifdef OLD
	MountPoolMigratePointer(&this->mount_info.steed_of, migrate);
	MountPoolMigratePointer(&this->mount_info.riding, migrate);
#else
	MountArrayMigratePointer(&this->mount_info.steed_of, migrate);
	MountArrayMigratePointer(&this->mount_info.riding, migrate);
#endif
}
#endif /* migrate --> */
#ifdef ARRAY /* <-- array */
#define ARRAY_NAME Llama
#define ARRAY_TYPE struct Llama
#define ARRAY_FREE_LIST
#define ARRAY_MIGRATE_EACH &llama_migrate
#include "Array.h"
#else /* array --><-- !array */
#define POOL_NAME Llama
#define POOL_TYPE struct Llama
#ifdef OLD
#define POOL_MIGRATE_EACH &llama_migrate
#include "Pool.h"
#else
#include "StablePool.h"
#endif
#endif /* !array --> */

/* Class Lemur extends Animal. */
struct Lemur {
	struct AnimalLink animal;
	struct MountInfo mount_info;
};
#if defined(OLD) || defined(ARRAY) /* <-- migrate */
static void lemur_migrate(struct Lemur *const this,
	const struct Migrate *const migrate) {
	assert(this && migrate);
	AnimalLinkMigrate(&this->animal.data, migrate);
#ifdef OLD
	MountPoolMigratePointer(&this->mount_info.steed_of, migrate);
	MountPoolMigratePointer(&this->mount_info.riding, migrate);
#else
	MountArrayMigratePointer(&this->mount_info.steed_of, migrate);
	MountArrayMigratePointer(&this->mount_info.riding, migrate);
#endif
}
#endif /* migrate --> */
#ifdef ARRAY /* <-- array */
#define ARRAY_NAME Lemur
#define ARRAY_TYPE struct Lemur
#define ARRAY_FREE_LIST
#define ARRAY_MIGRATE_EACH &lemur_migrate
#include "Array.h"
#else /* array --><-- !array */
#define POOL_NAME Lemur
#define POOL_TYPE struct Lemur
#ifdef OLD
#define POOL_MIGRATE_EACH &lemur_migrate
#include "Pool.h"
#else
#include "StablePool.h"
#endif
#endif /* !array --> */

/* Class Bear extends Animal. We have always two or less, so we don't need to
 define a {Pool}. */
struct Bear {
	struct AnimalLink animal;
	int is_active;
	struct MountInfo mount_info;
};

/* Animal list with backing. These are the storage structures. */
struct Animals {
	struct AnimalList list;
#ifdef ARRAY
	struct MountArray mounts;
	struct SlothArray sloths;
	struct EmuArray emus;
	struct BadEmuArray bad_emus;
	struct LlamaArray llamas;
	struct LemurArray lemurs;
#else
	struct MountPool mounts;
	struct SlothPool sloths;
	struct EmuPool emus;
	struct BadEmuPool bad_emus;
	struct LlamaPool llamas;
	struct LemurPool lemurs;
#endif
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

static void dismount(struct Animals *const animals, struct Mount *const mount);

/** @implements <Animal, [Animals]>BiAction */
static void Animal_delete(struct Animal *const animal,
	void *const void_animals) {
	struct Animals *const animals = void_animals;
	struct MountInfo *mount_info;
	if(!animals || !animal) return;
	if((mount_info = animal->vt->mount_info(animal))) {
		if(mount_info->steed_of) dismount(animals, mount_info->steed_of);
		if(mount_info->riding)   dismount(animals, mount_info->riding);
	}
	animal->vt->delete(animals, animal);
}
static void Sloth_delete(struct Animals *const animals,
	struct Sloth *const sloth) {
	printf("Bye %s.\n", sloth->animal.data.name);
	AnimalListRemove(&sloth->animal.data);
#ifdef ARRAY
	SlothArrayRemove(&animals->sloths, sloth);
#else
	SlothPoolRemove(&animals->sloths, sloth);
#endif
}
static void Emu_delete(struct Animals *const animals,
	struct Emu *const emu) {
	printf("Bye %s.\n", emu->animal.data.name);
	AnimalListRemove(&emu->animal.data);
#ifdef ARRAY
	EmuArrayRemove(&animals->emus, emu);
#else
	EmuPoolRemove(&animals->emus, emu);
#endif
}
static void BadEmu_delete(struct Animals *const animals,
	struct BadEmu *const bad_emu) {
	printf("%s dissapers in a puff of smoke.\n", bad_emu->emu.animal.data.name);
	AnimalListRemove(&bad_emu->emu.animal.data);
#ifdef ARRAY
	BadEmuArrayRemove(&animals->bad_emus, bad_emu);
#else
	BadEmuPoolRemove(&animals->bad_emus, bad_emu);
#endif
}
static void Lemur_delete(struct Animals *const animals,
	struct Lemur *const lemur) {
	printf("Bye %s.\n", lemur->animal.data.name);
	AnimalListRemove(&lemur->animal.data);
#ifdef ARRAY
	LemurArrayRemove(&animals->lemurs, lemur);
#else
	LemurPoolRemove(&animals->lemurs, lemur);
#endif
}
static void Llama_delete(struct Animals *const animals,
	struct Llama *const llama) {
	printf("Bye %s.\n", llama->animal.data.name);
	AnimalListRemove(&llama->animal.data);
#ifdef ARRAY
	LlamaArrayRemove(&animals->llamas, llama);
#else
	LlamaPoolRemove(&animals->llamas, llama);
#endif
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
		const struct Animal *const steed = bad_emu->mount_info.riding->steed;
		assert(steed);
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
		const struct Animal *const steed = lemur->mount_info.riding->steed;
		assert(steed);
		sprintf(ride, " They are riding on %s the %s.",
			steed->name, steed->vt->type);
	}
	printf("%s %s.%s\n", lemur->animal.data.vt->type, lemur->animal.data.name,
		ride);
}
static void Llama_act(struct Llama *const llama) {
	char ride[128] = "";
	if(llama->mount_info.steed_of) {
		const struct Animal *const rider = llama->mount_info.steed_of->rider;
		assert(rider);
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
		const struct Animal *const steed = bear->mount_info.riding->steed;
		assert(steed);
		sprintf(ride + strlen(ride), " riding on %s the %s",
			steed->name, steed->vt->type);
	}
	if(bear->mount_info.steed_of) {
		const struct Animal *const rider = bear->mount_info.steed_of->rider;
		assert(rider);
		sprintf(ride + strlen(ride), " being ridden by %s the %s",
			rider->name, rider->vt->type);
	}
	if(*ride == '\0') strcpy(ride, " chilling");
	printf("%s %s is%s.\n", bear->animal.data.vt->type, bear->animal.data.name,
		ride);
}

/** @implements AnimalMountField */
static struct MountInfo *Animal_mount_info(struct Animal *const animal) {
	assert(animal);
	return animal->vt->mount_info(animal);
}
static struct MountInfo *no_mount_info(struct Animal *const animal) {
	(void)animal;
	return 0;
}
static struct MountInfo *BadEmu_mount_info(struct BadEmu *const bad_emu) {
	return &bad_emu->mount_info;
}
static struct MountInfo *Lemur_mount_info(struct Lemur *const lemur) {
	return &lemur->mount_info;
}
static struct MountInfo *Llama_mount_info(struct Llama *const llama) {
	return &llama->mount_info;
}
static struct MountInfo *Bear_mount_info(struct Bear *const bear) {
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
	(AnimalMountInfo)&BadEmu_mount_info
};
static struct AnimalVt Lemur_vt = {
	"Lemur",
	(AnimalsAction)&Lemur_delete,
	(AnimalAction)&Lemur_act,
	(AnimalMountInfo)&Lemur_mount_info
};
static struct AnimalVt Llama_vt = {
	"Llama",
	(AnimalsAction)&Llama_delete,
	(AnimalAction)&Llama_act,
	(AnimalMountInfo)&Llama_mount_info
};
static struct AnimalVt Bear_vt = {
	"Bear",
	(AnimalsAction)&Bear_delete,
	(AnimalAction)&Bear_act,
	(AnimalMountInfo)&Bear_mount_info
};

#if defined(OLD) || defined(ARRAY) /* <-- migrate */
/** From before, waiting until {AnimalVt.mount_field} and {Animal_mount} was
 defined. This follows the links to {steed} and {rider} and makes them point
 back to the new memory location of {mount}.
 @implements <Mount>Migrate */
static void mount_migrate(struct Mount *const mount,
	const struct Migrate *const migrate) {
	assert(mount && migrate
		&& mount->steed && Animal_mount_info(mount->steed)
		&& mount->rider && Animal_mount_info(mount->rider));
#ifdef ARRAY
	MountArrayMigratePointer(&Animal_mount_info(mount->steed)->steed_of,migrate);
	MountArrayMigratePointer(&Animal_mount_info(mount->rider)->riding, migrate);
#else
	MountPoolMigratePointer(&Animal_mount_info(mount->steed)->steed_of,migrate);
	MountPoolMigratePointer(&Animal_mount_info(mount->rider)->riding, migrate);
#endif
}
#endif /* migrate --> */

/** Helper for delete. */
static void dismount(struct Animals *const animals, struct Mount *const mount) {
	assert(animals && mount && mount->steed && mount->rider);
	printf("%s the %s dismounts %s the %s.\n", mount->rider->name,
		mount->rider->vt->type, mount->steed->name, mount->steed->vt->type);
	Animal_mount_info(mount->steed)->steed_of = 0;
	Animal_mount_info(mount->rider)->riding = 0;
#ifdef ARRAY
	MountArrayRemove(&animals->mounts, mount);
#else
	MountPoolRemove(&animals->mounts, mount);
#endif
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
	this->steed_of = this->riding = 0;
	this->is_allowed = is_allowed;
}
/** Destructor. */
void Animals_(struct Animals **const panimals) {
	struct Animals *animals;
	if(!panimals || !(animals = *panimals)) return;
	AnimalListClear(&animals->list);
#ifdef ARRAY
	LemurArray_(&animals->lemurs);
	LlamaArray_(&animals->llamas);
	BadEmuArray_(&animals->bad_emus);
	EmuArray_(&animals->emus);
	SlothArray_(&animals->sloths);
	MountArray_(&animals->mounts);
#else
	LemurPool_(&animals->lemurs);
	LlamaPool_(&animals->llamas);
	BadEmuPool_(&animals->bad_emus);
	EmuPool_(&animals->emus);
	SlothPool_(&animals->sloths);
	MountPool_(&animals->mounts);
#endif
	free(animals), *panimals = animals = 0;
}
/** Constructor. */
struct Animals *Animals(void) {
	struct Animals *a;
	struct Bear *bear, *end;
	if(!(a = malloc(sizeof *a))) { perror("Animals"); Animals_(&a); return 0; }
	AnimalListClear(&a->list);
#ifdef ARRAY
	MountArray(&a->mounts);
	SlothArray(&a->sloths);
	EmuArray(&a->emus);
	BadEmuArray(&a->bad_emus);
	LlamaArray(&a->llamas);
	LemurArray(&a->lemurs);
#else
	MountPool(&a->mounts);
	SlothPool(&a->sloths);
	EmuPool(&a->emus);
	BadEmuPool(&a->bad_emus);
	LlamaPool(&a->llamas);
	LemurPool(&a->lemurs);
#endif	
	for(bear = a->bears, end = bear + no_bears; bear < end; bear++)
		bear->is_active = 0;
	return a;
}
/** Random constructor for a {Sloth} in {animals}. */
struct Sloth *Sloth(struct Animals *const animals) {
	struct Sloth *sloth;
	if(!animals) return 0;
#ifdef ARRAY
	if(!(sloth = SlothArrayNew(&animals->sloths))) return 0;
#else
	if(!(sloth = SlothPoolNew(&animals->sloths))) return 0;
#endif
	Animal_filler(&sloth->animal.data, &Sloth_vt);
	sloth->hours_slept = (int)(10.0 * rand() / RAND_MAX) + 4;
	AnimalListPush(&animals->list, &sloth->animal.data);
	return sloth;
}
/** Random constructor for an {Emu} in {animals}. */
struct Emu *Emu(struct Animals *const animals) {
	struct Emu *emu;
	if(!animals) return 0;
#ifdef ARRAY
	if(!(emu = EmuArrayNew(&animals->emus))) return 0;
#else
	if(!(emu = EmuPoolNew(&animals->emus))) return 0;
#endif
	Animal_filler(&emu->animal.data, &Emu_vt);
	emu->favourite_letter = 'a' + (char)(26.0 * rand() / RAND_MAX);
	AnimalListPush(&animals->list, &emu->animal.data);
	return emu;
}
/** Random constructor for a {BadEmu} in {animals}. */
struct BadEmu *BadEmu(struct Animals *const animals) {
	struct BadEmu *emu;
	if(!animals) return 0;
#ifdef ARRAY
	if(!(emu = BadEmuArrayNew(&animals->bad_emus))) return 0;
#else
	if(!(emu = BadEmuPoolNew(&animals->bad_emus))) return 0;
#endif
	Animal_filler(&emu->emu.animal.data, &BadEmu_vt);
	emu->emu.favourite_letter = 'a' + (char)(26.0 * rand() / RAND_MAX);
	MountInfo_filler(&emu->mount_info, &emu->emu.animal.data, RIDER);
	Orcish(emu->muhaha, sizeof emu->muhaha);
	AnimalListPush(&animals->list, &emu->emu.animal.data);
	return emu;
}
/** Random constructor for a {Llama} in {animals}. */
struct Llama *Llama(struct Animals *const animals) {
	struct Llama *llama;
	if(!animals) return 0;
#ifdef ARRAY
	if(!(llama = LlamaArrayNew(&animals->llamas))) return 0;
#else
	if(!(llama = LlamaPoolNew(&animals->llamas))) return 0;
#endif
	Animal_filler(&llama->animal.data, &Llama_vt);
	MountInfo_filler(&llama->mount_info, &llama->animal.data, STEED);
	llama->chomps = 5 + 10 * rand() / RAND_MAX;
	AnimalListPush(&animals->list, &llama->animal.data);
	return llama;
}
/** Random constructor for a {Lemur} in {animals}. */
struct Lemur *Lemur(struct Animals *const animals) {
	struct Lemur *lemur;
	if(!animals) return 0;
#ifdef ARRAY
	if(!(lemur = LemurArrayNew(&animals->lemurs))) return 0;
#else
	if(!(lemur = LemurPoolNew(&animals->lemurs))) return 0;
#endif
	Animal_filler(&lemur->animal.data, &Lemur_vt);
	MountInfo_filler(&lemur->mount_info, &lemur->animal.data, RIDER);
	AnimalListPush(&animals->list, &lemur->animal.data);
	return lemur;
}
/** Constructor for a {Bear} in {animals}.
 @param no: Has to be [0, 1]. You can not overwrite a {Bear} without deleting
 it first.
 @param name: Bears have non-random names. */
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

/** Cause {a} to try to ride {b}. If {a} or {b} is null, causes that connection
 to be broken.
 @return Success. */
int AnimalsRide(struct Animals *const animals, struct Animal *const a,
	struct Animal *const b) {
	struct Animal *erase, *steed = 0, *rider = 0;
	struct Mount *mount;
	if(!animals || (!a && !b) || a == b) return 0;
	erase = a ? b ? 0 : a : 0;
	if(erase) {
		/* One was null. */
		const struct MountInfo *const mi = erase->vt->mount_info(erase);
		if(!mi) return fprintf(stderr, "Animal %s the %s does not have mount "
			"information.\n", erase->name, erase->vt->type), 0;
		if(mi->riding)   dismount(animals, mi->riding);
		if(mi->steed_of) dismount(animals, mi->steed_of);
		assert(!mi->steed_of && !mi->riding);
		return 1;
	} else {
		/* Both are full. */
		struct MountInfo *const ami = a->vt->mount_info(a),
			*const bmi = b->vt->mount_info(b);
		if(!ami || !bmi) {
		} else if((ami->is_allowed & RIDER) && !ami->riding
			&& (bmi->is_allowed & STEED) && !bmi->steed_of) {
			steed = b, rider = a;
		} else if((ami->is_allowed & STEED) && !ami->steed_of
			&& (bmi->is_allowed & RIDER) && !bmi->riding) {
			steed = a, rider = b;
		}
	}
	if(!steed || !rider) return /*fprintf(stderr, "Animal %s the %s and "
		"%s the %s do not understand your mount in this configuration.\n",
		a->name, a->vt->type, b->name, b->vt->type),*/ 0;
	/* {steed} and {rider} are good. */
#ifdef ARRAY
	mount = MountArrayNew(&animals->mounts);
#else
	mount = MountPoolNew(&animals->mounts);
#endif
	mount->steed = steed, Animal_mount_info(steed)->steed_of = mount;
	mount->rider = rider, Animal_mount_info(rider)->riding   = mount;
	printf("%s the %s mounts %s the %s.\n", rider->name, rider->vt->type,
		steed->name, steed->vt->type);
	return 1;
	
}
/** @implements <Animal, [size_t *]>BiAction */
static void Animal_count(struct Animal *const animal, void *const pcount) {
	assert(animal && pcount);
	(*(size_t *)pcount)++;
}
/** Prints something out on all the {animals}. */
void AnimalsAct(struct Animals *const animals) {
	size_t count = 0;
	if(!animals) return;
	AnimalListBiForEach(&animals->list, &Animal_count, &count);
	printf("There are %lu animals.\n", (long unsigned)count);
	AnimalListForEach(&animals->list, &Animal_act);
}
/** Clears all the {animals}. */
void AnimalsClear(struct Animals *const animals) {
	if(!animals) return;
	AnimalListBiForEach(&animals->list, &Animal_delete, animals);
#ifdef OLD
	assert(!MountPoolPeek(&animals->mounts));
#endif
}
/** @return The first animal in {animals} or null if it doesn't have any. Don't
 add to {animals} while the iteration is still going. */
struct Animal *AnimalsFirst(struct Animals *const animals) {
	if(!animals) return 0;
	return AnimalListFirst(&animals->list);
}
/** @return The next animal from {animal} or null if it is the last. */
struct Animal *AnimalsNext(struct Animal *const animal) {
	if(!animal) return 0;
	return AnimalListNext(animal);
}
