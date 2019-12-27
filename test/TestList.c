#include <stdlib.h> /* EXIT rand */
#include <stdio.h>  /* printf */
#include <string.h> /* strcmp */
#include "Orcish.h"

/* An order with no parent. It could happen. */

struct OrderListNode;
static void order_to_string(const struct OrderListNode *const l,
	char (*const a)[12]) {
	(void)(l);
	(*a)[0] = '0';
	(*a)[1] = '\0';
}
static void order_fill(struct OrderListNode *const l) { (void)(l); }

#define LIST_NAME Order
#define LIST_TO_STRING &order_to_string
#define LIST_TEST &order_fill
#include "../src/List.h"

#define POOL_NAME OrderLink
#define POOL_TYPE struct OrderListNode
#include "Pool.h"
static struct OrderListNode *order_from_pool(void *const volls) {
	struct OrderLinkPool *const olls = volls;
	struct OrderListNode *oll = OrderLinkPoolNew(olls);
	assert(oll); if(!oll) return 0;
	return oll;
}

/* This is the minimum useful example. */

struct NoListNode;
static void no_to_string(const struct NoListNode *, char (*)[12]);
static void no_fill(struct NoListNode *);
static int no_compare(const struct NoListNode *, const struct NoListNode *);

#define LIST_NAME No
#define LIST_COMPARE &no_compare
#define LIST_TO_STRING &no_to_string
#define LIST_TEST &no_fill
#include "../src/List.h"

struct No { struct NoListNode link; int i; };

static int no_compare(const struct NoListNode *const all,
	const struct NoListNode *const bll) {
	/* Can do this because `link` is the first field in `struct No`. */
	const struct No *const a = (const struct No *)all,
		*const b = (const struct No *)bll;
	return (a->i > b->i) - (b->i > a->i);
}
static void no_to_string(const struct NoListNode *const noll,
	char (*const a)[12]) {
	const struct No *const no = (const struct No *)noll;
	/*assert(RAND_MAX <= 99999999999l);*/
	sprintf(*a, "%d", no->i);
}
static void no_fill(struct NoListNode *const noll) {
	struct No *const no = (struct No *)noll;
	no->i = rand() / (RAND_MAX / 100000 + 1);
}
#define POOL_NAME No
#define POOL_TYPE struct No
#include "Pool.h"
static struct NoListNode *no_from_pool(void *const vnos) {
	struct NoPool *const nos = vnos;
	struct No *no = NoPoolNew(nos);
	assert(no); if(!no) return 0;
	return &no->link;
}

/* For testing bin-ops just to be sure the examples were accurate. */

struct LetterListNode;
static void letter_to_string(const struct LetterListNode *, char (*)[12]);
static void letter_fill(struct LetterListNode *);
static int letter_compare(const struct LetterListNode *,
	const struct LetterListNode *);

#define LIST_NAME Letter
#define LIST_COMPARE &letter_compare
#define LIST_TO_STRING &letter_to_string
#define LIST_TEST &letter_fill
#include "../src/List.h"

struct Letter { struct LetterListNode link; char letter; };

static int letter_compare(const struct LetterListNode *const all,
	const struct LetterListNode *const bll) {
	/* Can do this because `link` is the first field in `struct No`. */
	const struct Letter *const a = (const struct Letter *)all,
		*const b = (const struct Letter *)bll;
	return (a->letter > b->letter) - (b->letter > a->letter);
}
static void letter_to_string(const struct LetterListNode *const lll,
	char (*const a)[12]) {
	const struct Letter *const l = (const struct Letter *)lll;
	(*a)[0] = l->letter;
	(*a)[1] = '\0';
}
static void letter_fill(struct LetterListNode *const lll) {
	struct Letter *const l = (struct Letter *)lll;
	l->letter = rand() / (RAND_MAX / 26 + 1) + 'A';
}
#define POOL_NAME Letter
#define POOL_TYPE struct Letter
#include "Pool.h"
static struct LetterListNode *letter_from_pool(void *const vls) {
	struct LetterPool *const ls = vls;
	struct Letter *l = LetterPoolNew(ls);
	assert(l); if(!l) return 0;
	return &l->link;
}

/* Multi-list. Three lists at once; because why not? */

struct NameListNode;
static void name_to_string(const struct NameListNode *, char (*)[12]);
static void fill_panda_name(struct NameListNode *);
static int name_compare(const struct NameListNode *,
	const struct NameListNode *);

#define LIST_NAME Name
#define LIST_COMPARE &name_compare
#define LIST_TO_STRING &name_to_string
#define LIST_TEST &fill_panda_name
#include "../src/List.h"

struct WhereListNode;
static void where_to_string(const struct WhereListNode *, char (*)[12]);
static void fill_panda_where(struct WhereListNode *);
static int where_compare(const struct WhereListNode *,
	const struct WhereListNode *);

#define LIST_NAME Where
#define LIST_COMPARE &where_compare
#define LIST_TO_STRING &where_to_string
#define LIST_TEST &fill_panda_where
#include "../src/List.h"

struct FeroListNode;
static void fero_to_string(const struct FeroListNode *, char (*)[12]);
static void fill_panda_fero(struct FeroListNode *);
static int fero_compare(const struct FeroListNode *,
	const struct FeroListNode *);

#define LIST_NAME Fero
#define LIST_COMPARE &fero_compare
#define LIST_TO_STRING &fero_to_string
#define LIST_TEST &fill_panda_fero
#include "../src/List.h"

struct Panda {
	char name[12];
	struct NameListNode name_node;
	int where;
	struct WhereListNode where_node;
	int ferociousness;
	struct FeroListNode fero_node;
};

static struct Panda *name_upcast(struct NameListNode *const n) {
	return (struct Panda *)(void *)((char *)n
		- offsetof(struct Panda, name_node));
}
static const struct Panda *name_upcast_c(const struct NameListNode *const n) {
	return (const struct Panda *)(const void *)((const char *)n
		- offsetof(struct Panda, name_node));
}
static struct Panda *where_upcast(struct WhereListNode *const w) {
	return (struct Panda *)(void *)((char *)w
		- offsetof(struct Panda, where_node));
}
static const struct Panda *where_upcast_c(const struct WhereListNode *const w) {
	return (const struct Panda *)(const void *)((const char *)w
		- offsetof(struct Panda, where_node));
}
static struct Panda *fero_upcast(struct FeroListNode *const f) {
	return (struct Panda *)(void *)((char *)f
		- offsetof(struct Panda, fero_node));
}
static const struct Panda *fero_upcast_c(const struct FeroListNode *const f) {
	return (const struct Panda *)(const void *)((const char *)f
		- offsetof(struct Panda, fero_node));
}
static void panda_to_string(const struct Panda *const p, char (*const a)[12]) {
	sprintf(*a, "%.11s", p->name);
}
static void name_to_string(const struct NameListNode *const n,
	char (*const a)[12]) {
	panda_to_string(name_upcast_c(n), a);
}
static void where_to_string(const struct WhereListNode *const w,
	char (*const a)[12]) {
	panda_to_string(where_upcast_c(w), a);
}
static void fero_to_string(const struct FeroListNode *const f,
	char (*const a)[12]) {
	panda_to_string(fero_upcast_c(f), a);
}
static int name_compare(const struct NameListNode *const a,
	const struct NameListNode *const b) {
	return strcmp(name_upcast_c(a)->name, name_upcast_c(b)->name);
}
static int int_compare(const int a, const int b) {
	return (a > b) - (b > a);
}
static int where_compare(const struct WhereListNode *const a,
	const struct WhereListNode *const b) {
	return int_compare(where_upcast_c(a)->where, where_upcast_c(b)->where);
}
static int fero_compare(const struct FeroListNode *const a,
	const struct FeroListNode *const b) {
	return int_compare(fero_upcast_c(a)->ferociousness,
		fero_upcast_c(b)->ferociousness);
}
static void fill_panda(struct Panda *const p) {
	assert(p);
	Orcish(p->name, sizeof p->name);
	p->where = rand() / (RAND_MAX / 198 + 1) - 99;
	p->ferociousness = rand() / (RAND_MAX / 11 + 1);
}
static void fill_panda_name(struct NameListNode *const name) {
	fill_panda(name_upcast(name));
}
static void fill_panda_where(struct WhereListNode *const where) {
	fill_panda(where_upcast(where));
}
static void fill_panda_fero(struct FeroListNode *const fero) {
	fill_panda(fero_upcast(fero));
}
#define POOL_NAME Panda
#define POOL_TYPE struct Panda
#include "Pool.h"
static struct NameListNode *panda_name_from_pool(void *const vpool) {
	struct PandaPool *const pool = vpool;
	struct Panda *p = PandaPoolNew(pool);
	assert(p); if(!p) return 0;
	return &p->name_node;
}
static struct WhereListNode *panda_where_from_pool(void *const vpool) {
	struct PandaPool *const pool = vpool;
	struct Panda *p = PandaPoolNew(pool);
	assert(p); if(!p) return 0;
	return &p->where_node;
}
static struct FeroListNode *panda_fero_from_pool(void *const vpool) {
	struct PandaPool *const pool = vpool;
	struct Panda *p = PandaPoolNew(pool);
	assert(p); if(!p) return 0;
	return &p->fero_node;
}
static struct Panda *panda_from_pool_extra(struct PandaPool *const pool,
	struct NameList *const n, struct WhereList *const w,
	struct FeroList *const f) {
	struct Panda *p = PandaPoolNew(pool);
	assert(p && pool && n && w && f);
	if(!p) return 0;
	fill_panda(p);
	NameListPush(n, &p->name_node);
	WhereListPush(w, &p->where_node);
	FeroListPush(f, &p->fero_node);
	return p;
}
static void panda_graph(const struct NameList *const n,
	const struct WhereList *const w, const struct FeroList *const f) {
	const char *fn = "graph/Pandas.gv";
	FILE *fp;
	assert(n && w && f);
	if(!(fp = fopen(fn, "w"))) { perror(fn); return; }
	fprintf(fp, "digraph {\n"
		/*"\trankdir=LR;\n"*/
		/*"\tnodesep=0;\n"*/);
	/* Overriding private! */
	list_Name_subgraph(n, fp, "royalblue",
		offsetof(struct Panda, name_node), 1);
	list_Where_subgraph(w, fp, /*"firebrick"*/"orchid",
		offsetof(struct Panda, where_node), 0);
	list_Fero_subgraph(f, fp, "darkseagreen",
		offsetof(struct Panda, fero_node), 0);
	fprintf(fp, "\tnode [colour=red, style=filled];\n");
	fprintf(fp, "}\n");
	fclose(fp);
	
}
static void pandas_everywhere(void) {
	struct NameList names;
	struct WhereList wheres;
	struct FeroList feros;
	struct PandaPool pandas = POOL_IDLE;
	size_t i;
	NameListClear(&names), WhereListClear(&wheres), FeroListClear(&feros);
	for(i = 0; i < 60; i++)
		panda_from_pool_extra(&pandas, &names, &wheres, &feros);
	NameListSort(&names);
	WhereListSort(&wheres);
	FeroListSort(&feros);
	panda_graph(&names, &wheres, &feros);
	PandaPool_(&pandas);
}



/* Animals! See <../web/animals.gv>. */

/* Id is the list that holds all the animals together. */
struct IdListNode;
static void id_to_string(const struct IdListNode *, char (*)[12]);
#define LIST_NAME Id
#define LIST_TO_STRING &id_to_string
#include "../src/List.h"

enum Colour { PINK, RED, BLUE, YELLOW, BEIGE, COLOUR_END };
static const char *const colours[] = { "pink", "red", "blue","yellow", "beige"};
enum { BOTTOM, TOP, RIDING_END };

/* Abstract `Animal`. */
struct AnimalVt;
struct Animal {
	const struct AnimalVt *vt;
	struct IdListNode id;
	char name[16];
	enum Colour colour;
};
static const size_t animal_name_size = sizeof ((struct Animal *)0)->name;
static struct Animal *id_upcast(struct IdListNode *const id) {
	return (struct Animal *)(void *)((char *)id
		- offsetof(struct Animal, id));
}
static const struct Animal *id_upcast_c(const struct IdListNode *const id) {
	return (const struct Animal *)(const void *)((const char *)id
		- offsetof(struct Animal, id));
}
static void animal_to_string(const struct Animal *const animal,
	char (*const a)[12]) {
	strncpy(*a, animal->name, sizeof *a - 1);
	*a[sizeof *a - 1] = '\0';
}
static void id_to_string(const struct IdListNode *const id,
	char (*const a)[12]) {
	animal_to_string(id_upcast_c(id), a);
}

/* `Mount` involves two animals. */
struct Mount;
struct MountInfo {
	struct Mount *steed_of, *riding;
	enum Allowed { STEED = 1, RIDER = 2 } is_allowed; /* Bit-field. */
};
struct Mount {
	struct Animal *steed, *rider;
};
#define POOL_NAME Mount
#define POOL_TYPE struct Mount
#include "Pool.h"

/* `Sloth` extends `Animal`. */
struct Sloth {
	struct Animal animal;
	unsigned hours_slept;
};
#define POOL_NAME Sloth
#define POOL_TYPE struct Sloth
#include "Pool.h"

/* `Emu` extends `Animal`. */
struct Emu {
	struct Animal animal;
	char favourite_letter;
};
#define POOL_NAME Emu
#define POOL_TYPE struct Emu
#include "Pool.h"

/* `BadEmu` extends `Emu`. */
struct BadEmu {
	struct Emu emu;
	struct MountInfo mount_info;
	char muhaha[12];
};
#define POOL_NAME BadEmu
#define POOL_TYPE struct BadEmu
#include "Pool.h"

/* `Llama` extends `Animal`. */
struct Llama {
	struct Animal animal;
	struct MountInfo mount_info;
	unsigned chomps;
};
#define POOL_NAME Llama
#define POOL_TYPE struct Llama
#include "Pool.h"

/* `Lemur` extends `Animal`. */
struct Lemur {
	struct Animal animal;
	struct MountInfo mount_info;
};
#define POOL_NAME Lemur
#define POOL_TYPE struct Lemur
#include "Pool.h"

/* `Bear` extends `Animal`. We have always two or less. */
struct Bear {
	struct Animal animal;
	int is_active;
	struct MountInfo mount_info;
};

/* Id list with backing. */
struct Animals {
	struct IdList list;
	struct MountPool mounts;
	struct SlothPool sloths;
	struct EmuPool emus;
	struct BadEmuPool bad_emus;
	struct LlamaPool llamas;
	struct LemurPool lemurs;
	struct Bear bears[2];
} animals;
static const unsigned no_bears = sizeof(((struct Animals *)0)->bears)
	/ sizeof(*((struct Animals *)0)->bears);

typedef void (*AnimalAction)(struct Animal *);
typedef struct MountInfo *(*AnimalMountInfo)(struct Animal *);

struct AnimalVt {
	const char type[16];
	AnimalAction delete;
	AnimalAction act/*transmogrify*/;
	AnimalMountInfo mount_info;
};

static void dismount(struct Mount *);

/** @implements AnimalAction */
static void animal_delete(struct Animal *const animal) {
	struct MountInfo *mount_info;
	if(!animal) return;
	if((mount_info = animal->vt->mount_info(animal))) {
		if(mount_info->steed_of) dismount(mount_info->steed_of);
		if(mount_info->riding)   dismount(mount_info->riding);
	}
	animal->vt->delete(animal);
}
/** @implements <Id>Action */
static void id_delete(struct IdListNode *const id) {
	animal_delete(id_upcast(id));
}
static void sloth_delete(struct Sloth *const sloth) {
	printf("Bye %s.\n", sloth->animal.name);
	IdListRemove(&sloth->animal.id);
	SlothPoolRemove(&animals.sloths, sloth);
}
static void emu_delete(struct Emu *const emu) {
	printf("Bye %s.\n", emu->animal.name);
	IdListRemove(&emu->animal.id);
	EmuPoolRemove(&animals.emus, emu);
}
static void bad_emu_delete(struct BadEmu *const bad_emu) {
	printf("%s dissapers in a puff of smoke.\n", bad_emu->emu.animal.name);
	IdListRemove(&bad_emu->emu.animal.id);
	BadEmuPoolRemove(&animals.bad_emus, bad_emu);
}
static void lemur_delete(struct Lemur *const lemur) {
	printf("Bye %s.\n", lemur->animal.name);
	IdListRemove(&lemur->animal.id);
	LemurPoolRemove(&animals.lemurs, lemur);
}
static void llama_delete(struct Llama *const llama) {
	printf("Bye %s.\n", llama->animal.name);
	IdListRemove(&llama->animal.id);
	LlamaPoolRemove(&animals.llamas, llama);
}
static void bear_delete(struct Bear *const bear) {
	if(!bear->is_active) return;
	printf("Bye %s.\n", bear->animal.name);
	IdListRemove(&bear->animal.id);
	bear->is_active = 0;
}

/** @implements AnimalAction */
static void animal_act(struct Animal *const animal) {
	assert(animal);
	animal->vt->act(animal);
}
/** @implements <Id>Action */
static void id_act(struct IdListNode *const id) {
	animal_act(id_upcast(id));
}
static void sloth_act(struct Sloth *const sloth) {
	printf("%s %s has favourite colour %s and has been sleeping %u hours.\n",
		sloth->animal.vt->type, sloth->animal.name,
		colours[sloth->animal.colour], sloth->hours_slept);
}
static void emu_act(struct Emu *const emu) {
	printf("%s %s has favourite colour %s and favourite letter %c.\n",
		emu->animal.vt->type, emu->animal.name,
		colours[emu->animal.colour], emu->favourite_letter);
}
static void bad_emu_act(struct BadEmu *const bad_emu) {
	char ride[128] = "";
	if(bad_emu->mount_info.riding) {
		const struct Animal *const steed = bad_emu->mount_info.riding->steed;
		assert(steed);
		sprintf(ride, " They are riding on %s the %s.",
			steed->name, steed->vt->type);
	}
	printf("%s %s has favourite colour %s and favourite letter %c; "
		   "he is mumbling \"%s.\"%s\n", bad_emu->emu.animal.vt->type,
		   bad_emu->emu.animal.name, colours[bad_emu->emu.animal.colour],
		   bad_emu->emu.favourite_letter, bad_emu->muhaha, ride);
}
static void lemur_act(struct Lemur *const lemur) {
	char ride[128] = "";
	if(lemur->mount_info.riding) {
		const struct Animal *const steed = lemur->mount_info.riding->steed;
		assert(steed);
		sprintf(ride, " They are riding on %s the %s.",
			steed->name, steed->vt->type);
	}
	printf("%s %s.%s\n", lemur->animal.vt->type, lemur->animal.name, ride);
}
static void llama_act(struct Llama *const llama) {
	char ride[128] = "";
	if(llama->mount_info.steed_of) {
		const struct Animal *const rider = llama->mount_info.steed_of->rider;
		assert(rider);
		sprintf(ride, " They are the noble steed of %s the %s.",
			rider->name, rider->vt->type);
	}
	printf("%s %s has chomped %u fingers today.%s\n",
		llama->animal.vt->type, llama->animal.name, llama->chomps, ride);
}
static void bear_act(struct Bear *const bear) {
	char ride[128] = " chilling";
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
	printf("%s %s is%s.\n", bear->animal.vt->type, bear->animal.name, ride);
}

/** @implements AnimalMountInfo */
static struct MountInfo *animal_mount_info(struct Animal *const animal) {
	assert(animal);
	return animal->vt->mount_info(animal);
}
static struct MountInfo *no_mount_info(struct Animal *const animal) {
	(void)animal;
	return 0;
}
static struct MountInfo *bad_emu_mount_info(struct BadEmu *const bad_emu) {
	return &bad_emu->mount_info;
}
static struct MountInfo *lemur_mount_info(struct Lemur *const lemur) {
	return &lemur->mount_info;
}
static struct MountInfo *llama_mount_info(struct Llama *const llama) {
	return &llama->mount_info;
}
static struct MountInfo *bear_mount_info(struct Bear *const bear) {
	return &bear->mount_info;
}

/* Static data containing the functions defined above. Because `struct Animal`
 is always the first item in every animal, we can cast them. This is not really
 a good design choice, in hindsight, (should have accepted all `Animal` and
 upcast.) */
static struct AnimalVt sloth_vt = {
	"Sloth",
	(AnimalAction)&sloth_delete,
	(AnimalAction)&sloth_act,
	&no_mount_info
};
static struct AnimalVt emu_vt = {
	"Emu",
	(AnimalAction)&emu_delete,
	(AnimalAction)&emu_act,
	&no_mount_info
};
static struct AnimalVt bad_emu_vt = {
	"Emu",
	(AnimalAction)&bad_emu_delete,
	(AnimalAction)&bad_emu_act,
	(AnimalMountInfo)&bad_emu_mount_info
};
static struct AnimalVt lemur_vt = {
	"Lemur",
	(AnimalAction)&lemur_delete,
	(AnimalAction)&lemur_act,
	(AnimalMountInfo)&lemur_mount_info
};
static struct AnimalVt llama_vt = {
	"Llama",
	(AnimalAction)&llama_delete,
	(AnimalAction)&llama_act,
	(AnimalMountInfo)&llama_mount_info
};
static struct AnimalVt bear_vt = {
	"Bear",
	(AnimalAction)&bear_delete,
	(AnimalAction)&bear_act,
	(AnimalMountInfo)&bear_mount_info
};

/** Helper for delete. */
static void dismount(struct Mount *const mount) {
	assert(mount && mount->steed && mount->rider);
	printf("%s the %s dismounts %s the %s.\n", mount->rider->name,
		mount->rider->vt->type, mount->steed->name, mount->steed->vt->type);
	animal_mount_info(mount->steed)->steed_of = 0;
	animal_mount_info(mount->rider)->riding   = 0;
	MountPoolRemove(&animals.mounts, mount);
}

/** Only called from constructors of children. */
static void animal_filler(struct Animal *const animal,
	const struct AnimalVt *const vt) {
	assert(animal && vt);
	animal->vt     = vt;
	animal->colour = (enum Colour)(rand() / (RAND_MAX / COLOUR_END + 1));
	Orcish(animal->name, sizeof animal->name);
}
static void mount_info_filler(struct MountInfo *const this,
	struct Animal *const animal, const enum Allowed is_allowed) {
	assert(this && animal && animal->vt->mount_info(animal) == this);
	this->steed_of = this->riding = 0;
	this->is_allowed = is_allowed;
}
/** Destructor for all. */
static void Animals_(void) {
	IdListClear(&animals.list);
	LemurPool_(&animals.lemurs);
	LlamaPool_(&animals.llamas);
	BadEmuPool_(&animals.bad_emus);
	EmuPool_(&animals.emus);
	SlothPool_(&animals.sloths);
	MountPool_(&animals.mounts);
}
/** Constructor for all. */
static void Animals(void) {
	struct Bear *bear, *end;
	IdListClear(&animals.list);
	/*assert() PoolIsIdle? */
	SlothPool(&animals.sloths);
	EmuPool(&animals.emus);
	BadEmuPool(&animals.bad_emus);
	LlamaPool(&animals.llamas);
	LemurPool(&animals.lemurs);
	for(bear = animals.bears, end = bear + no_bears; bear < end; bear++)
		bear->is_active = 0;
}
/** Random constructor for a `Sloth`. */
static struct Sloth *sloth(void) {
	struct Sloth *s;
	if(!(s = SlothPoolNew(&animals.sloths))) return 0;
	animal_filler(&s->animal, &sloth_vt);
	s->hours_slept = rand() / (RAND_MAX / 10 + 1) + 5;
	IdListPush(&animals.list, &s->animal.id);
	return s;
}
/** Random constructor for an `Emu`. */
static struct Emu *emu(void) {
	struct Emu *e;
	if(!(e = EmuPoolNew(&animals.emus))) return 0;
	animal_filler(&e->animal, &emu_vt);
	e->favourite_letter = 'a' + rand() / (RAND_MAX / 26 + 1);
	IdListPush(&animals.list, &e->animal.id);
	return e;
}
/** Random constructor for a `BadEmu`. */
static struct BadEmu *bad_emu(void) {
	struct BadEmu *e;
	if(!(e = BadEmuPoolNew(&animals.bad_emus))) return 0;
	animal_filler(&e->emu.animal, &bad_emu_vt);
	e->emu.favourite_letter = 'a' + rand() / (RAND_MAX / 26 + 1);
	mount_info_filler(&e->mount_info, &e->emu.animal, RIDER);
	Orcish(e->muhaha, sizeof e->muhaha);
	IdListPush(&animals.list, &e->emu.animal.id);
	return e;
}
/** Random constructor for a `Llama`. */
static struct Llama *llama(void) {
	struct Llama *l;
	if(!(l = LlamaPoolNew(&animals.llamas))) return 0;
	animal_filler(&l->animal, &llama_vt);
	mount_info_filler(&l->mount_info, &l->animal, STEED);
	l->chomps = 5 + rand() / (RAND_MAX / 10 + 1);
	IdListPush(&animals.list, &l->animal.id);
	return l;
}
/** Random constructor for a `Lemur`. */
static struct Lemur *lemur(void) {
	struct Lemur *l;
	if(!(l = LemurPoolNew(&animals.lemurs))) return 0;
	animal_filler(&l->animal, &lemur_vt);
	mount_info_filler(&l->mount_info, &l->animal, RIDER);
	IdListPush(&animals.list, &l->animal.id);
	return l;
}
/** Constructor for a `Bear`.
 @param[no] Has to be `[0, 1]`. You can not overwrite without deleting it first.
 @param[name] Bears have non-random names. */
static struct Bear *bear(const unsigned no, const char *const name) {
	struct Bear *b;
	assert(name && no < no_bears);
	if(no >= no_bears) return 0;
	b = animals.bears + no;
	if(b->is_active) { fprintf(stderr, "Bear is active.\n"); return 0; }
	animal_filler(&b->animal, &bear_vt);
	strncpy(b->animal.name, name, animal_name_size - 1),
		b->animal.name[animal_name_size - 1] = '\0';
	b->is_active = 1;
	mount_info_filler(&b->mount_info, &b->animal, STEED | RIDER);
	IdListPush(&animals.list, &b->animal.id);
	return b;
}

/** Cause `a` to try to ride `b`. If `a` or `b` is null, causes that connection
 to be broken.
 @return Success. */
static int ride(struct Animal *const a, struct Animal *const b) {
	struct Animal *erase, *steed = 0, *rider = 0;
	struct Mount *mount;
	if((!a && !b) || a == b) return 0;
	erase = a ? b ? 0 : a : b;
	if(erase) {
		const struct MountInfo *const mi = erase->vt->mount_info(erase);
		if(!mi) return fprintf(stderr, "Animal %s the %s does not have mount "
			"information.\n", erase->name, erase->vt->type), 0;
		if(mi->riding)   dismount(mi->riding);
		if(mi->steed_of) dismount(mi->steed_of);
		assert(!mi->steed_of && !mi->riding);
		return 1;
	} else {
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
	mount = MountPoolNew(&animals.mounts);
	mount->steed = steed, animal_mount_info(steed)->steed_of = mount;
	mount->rider = rider, animal_mount_info(rider)->riding   = mount;
	printf("%s the %s mounts %s the %s.\n", rider->name, rider->vt->type,
		steed->name, steed->vt->type);
	return 1;
}
/** Prints something out on all the `animals`. */
static void animals_act(void) {
	size_t count = 0;
	struct IdListNode *id;
	for(id = IdListFirst(&animals.list); id; id = IdListNext(id)) count++;
	printf("There are %lu animals.\n", (unsigned long)count);
	IdListForEach(&animals.list, &id_act);
}
/** Clears all the `animals`. */
static void animals_clear(void) {
	IdListForEach(&animals.list, &id_delete);
}


#include <time.h>	/* clock */

static int animals_everywhere(void) {
	unsigned seed = (unsigned)clock();
	int is_success = 0;
	clock_t t;
	srand(seed), rand(), printf("Seed %u.\n", seed);
	t = clock();
	Animals();
	do {
		struct IdListNode *id = 0, *prev_id = 0;
		struct Bear *w, *n;
		const unsigned animal_no = 10000/*00*/;
		unsigned i;
		n = bear(1, "Napoleon");
		for(i = 0; i < animal_no; i++) {
			float r = (float)(rand() / ((double)RAND_MAX + 1));
			if(r < 0.25f) {
				if(!sloth()) break;
			} else if(r < 0.45f) {
				if(!emu()) break;
			} else if(r < 0.55) {
				if(!bad_emu()) break;
			} else if(r < 0.8) {
				if(!llama()) break;
			} else {
				if(!lemur()) break;
			}
		}
		if(i != animal_no) break;
		w = bear(0, "Winnie");
		for(id = IdListFirst(&animals.list); id; id = IdListNext(id)) {
			if(prev_id && !ride(id_upcast(prev_id), id_upcast(id)))
				ride(id_upcast(id), id_upcast(prev_id));
			prev_id = id;
		}
		animals_act();
		animals_clear();
		is_success = 1;
	} while(0); if(!is_success) {
		perror("Animals");
	} {
		Animals_();
	}
	fprintf(stderr, "Time: %lu\n", (unsigned long)(clock() - t));

	return is_success;
}

int main(void) {
	struct OrderLinkPool olls = POOL_IDLE;
	struct NoPool nos = POOL_IDLE;
	struct LetterPool ls = POOL_IDLE;
	struct PandaPool pandas = POOL_IDLE;
	OrderListTest(&order_from_pool, &olls), OrderLinkPool_(&olls);
	NoListTest(&no_from_pool, &nos), NoPool_(&nos);
	LetterListTest(&letter_from_pool, &ls), LetterPool_(&ls);
	NameListTest(&panda_name_from_pool, &pandas), PandaPool_(&pandas);
	WhereListTest(&panda_where_from_pool, &pandas), PandaPool_(&pandas);
	FeroListTest(&panda_fero_from_pool, &pandas), PandaPool_(&pandas);
	pandas_everywhere();
	animals_everywhere();
	return EXIT_SUCCESS;
}
