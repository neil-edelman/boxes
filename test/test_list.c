#include <stdlib.h> /* EXIT rand */
#include <stdio.h>  /* printf */
#include <string.h> /* strcmp */
#include "orcish.h"

/* Minimal example. An order with no parent represents a permutation of
 nothing. */

struct order_list_node;
static void order_to_string(const struct order_list_node *const l,
	char (*const a)[12]) { (void)(l); (*a)[0] = '0'; (*a)[1] = '\0'; }
static void order_fill(struct order_list_node *const l) { (void)(l); }

#define LIST_NAME order
#define LIST_EXPECT_TRAIT
#include "../src/list.h"
#define LIST_TO_STRING &order_to_string
#define LIST_TEST &order_fill
#include "../src/list.h"

#define POOL_NAME order_node
#define POOL_TYPE struct order_list_node
#include "pool.h"

static struct order_list_node *order_from_pool(void *const volls) {
	struct order_node_pool *const olls = volls;
	struct order_list_node *oll = order_node_pool_new(olls);
	assert(oll); if(!oll) return 0;
	return oll;
}


/* This is the minimum useful example, (plus testing framework.) */

struct no_list_node;
static void no_to_string(const struct no_list_node *, char (*)[12]);
static void no_fill(struct no_list_node *);
static int no_compare(const struct no_list_node *, const struct no_list_node *);

#define LIST_NAME no
#define LIST_COMPARE &no_compare
#define LIST_EXPECT_TRAIT
#include "../src/list.h"
#define LIST_TO_STRING &no_to_string
#define LIST_TEST &no_fill
#include "../src/list.h"

struct no { struct no_list_node node; int i; };

static int no_compare(const struct no_list_node *const all,
	const struct no_list_node *const bll) {
	/* Can do this because `node` is the first field. */
	const struct no *const a = (const struct no *)all,
		*const b = (const struct no *)bll;
	return (a->i > b->i) - (b->i > a->i);
}
static void no_to_string(const struct no_list_node *const noll,
	char (*const a)[12]) {
	const struct no *const no = (const struct no *)noll;
	/*assert(RAND_MAX <= 99999999999l);*/
	sprintf(*a, "%d", no->i);
}
static void no_fill(struct no_list_node *const noll) {
	struct no *const no = (struct no *)noll;
	no->i = rand() / (RAND_MAX / 100000 + 1);
}
#define POOL_NAME no
#define POOL_TYPE struct no
#include "pool.h"
static struct no_list_node *no_from_pool(void *const vnos) {
	struct no_pool *const nos = vnos;
	struct no *no = no_pool_new(nos);
	assert(no); if(!no) return 0;
	return &no->node;
}


/* For testing bin-ops just to be sure the examples were accurate. */

struct letter_list_node;
static void letter_to_string(const struct letter_list_node *, char (*)[12]);
static void letter_fill(struct letter_list_node *);
static int letter_compare(const struct letter_list_node *,
	const struct letter_list_node *);

#define LIST_NAME letter
#define LIST_COMPARE &letter_compare
#define LIST_EXPECT_TRAIT
#include "../src/list.h"
#define LIST_TO_STRING &letter_to_string
#define LIST_TEST &letter_fill
#include "../src/list.h"

struct letter { struct letter_list_node node; char letter; };

static int letter_compare(const struct letter_list_node *const all,
	const struct letter_list_node *const bll) {
	/* Can do this because `node` is the first field in `struct No`. */
	const struct letter *const a = (const struct letter *)all,
		*const b = (const struct letter *)bll;
	return (a->letter > b->letter) - (b->letter > a->letter);
}
static void letter_to_string(const struct letter_list_node *const lll,
	char (*const a)[12]) {
	const struct letter *const l = (const struct letter *)lll;
	(*a)[0] = l->letter;
	(*a)[1] = '\0';
}
static void letter_fill(struct letter_list_node *const lll) {
	struct letter *const l = (struct letter *)lll;
	l->letter = rand() / (RAND_MAX / 26 + 1) + 'A';
}
#define POOL_NAME letter
#define POOL_TYPE struct letter
#include "pool.h"
static struct letter_list_node *letter_from_pool(void *const vls) {
	struct letter_pool *const ls = vls;
	struct letter *l = letter_pool_new(ls);
	assert(l); if(!l) return 0;
	return &l->node;
}


/* Multi-list. Three lists at once. */

struct name_list_node;
static void name_to_string(const struct name_list_node *, char (*)[12]);
static void fill_panda_name(struct name_list_node *);
static int name_compare(const struct name_list_node *,
	const struct name_list_node *);

#define LIST_NAME name
#define LIST_COMPARE &name_compare
#define LIST_EXPECT_TRAIT
#include "../src/list.h"
#define LIST_TO_STRING &name_to_string
#define LIST_TEST &fill_panda_name
#include "../src/list.h"

struct where_list_node;
static void where_to_string(const struct where_list_node *, char (*)[12]);
static void fill_panda_where(struct where_list_node *);
static int where_compare(const struct where_list_node *,
	const struct where_list_node *);

#define LIST_NAME where
#define LIST_COMPARE &where_compare
#define LIST_EXPECT_TRAIT
#include "../src/list.h"
#define LIST_TO_STRING &where_to_string
#define LIST_TEST &fill_panda_where
#include "../src/list.h"

struct fero_list_node;
static void fero_to_string(const struct fero_list_node *, char (*)[12]);
static void fill_panda_fero(struct fero_list_node *);
static int fero_compare(const struct fero_list_node *,
	const struct fero_list_node *);

#define LIST_NAME fero
#define LIST_COMPARE &fero_compare
#define LIST_EXPECT_TRAIT
#include "../src/list.h"
#define LIST_TO_STRING &fero_to_string
#define LIST_TEST &fill_panda_fero
#include "../src/list.h"

struct Panda {
	char name[12];
	struct name_list_node name_node;
	int where;
	struct where_list_node where_node;
	int ferociousness;
	struct fero_list_node fero_node;
};

static struct Panda *name_upcast(struct name_list_node *const n) {
	return (struct Panda *)(void *)((char *)n
		- offsetof(struct Panda, name_node));
}
static const struct Panda *name_upcast_c(const struct name_list_node *const n) {
	return (const struct Panda *)(const void *)((const char *)n
		- offsetof(struct Panda, name_node));
}
static struct Panda *where_upcast(struct where_list_node *const w) {
	return (struct Panda *)(void *)((char *)w
		- offsetof(struct Panda, where_node));
}
static const struct Panda *where_upcast_c(const struct where_list_node *const w)
	{ return (const struct Panda *)(const void *)((const char *)w
		- offsetof(struct Panda, where_node)); }
static struct Panda *fero_upcast(struct fero_list_node *const f) {
	return (struct Panda *)(void *)((char *)f
		- offsetof(struct Panda, fero_node));
}
static const struct Panda *fero_upcast_c(const struct fero_list_node *const f) {
	return (const struct Panda *)(const void *)((const char *)f
		- offsetof(struct Panda, fero_node));
}
static void panda_to_string(const struct Panda *const p, char (*const a)[12]) {
	sprintf(*a, "%.11s", p->name);
}
static void name_to_string(const struct name_list_node *const n,
	char (*const a)[12]) {
	panda_to_string(name_upcast_c(n), a);
}
static void where_to_string(const struct where_list_node *const w,
	char (*const a)[12]) {
	panda_to_string(where_upcast_c(w), a);
}
static void fero_to_string(const struct fero_list_node *const f,
	char (*const a)[12]) {
	panda_to_string(fero_upcast_c(f), a);
}
static int name_compare(const struct name_list_node *const a,
	const struct name_list_node *const b) {
	return strcmp(name_upcast_c(a)->name, name_upcast_c(b)->name);
}
static int int_compare(const int a, const int b) {
	return (a > b) - (b > a);
}
static int where_compare(const struct where_list_node *const a,
	const struct where_list_node *const b) {
	return int_compare(where_upcast_c(a)->where, where_upcast_c(b)->where);
}
static int fero_compare(const struct fero_list_node *const a,
	const struct fero_list_node *const b) {
	return int_compare(fero_upcast_c(a)->ferociousness,
		fero_upcast_c(b)->ferociousness);
}
static void fill_panda(struct Panda *const p) {
	assert(p);
	Orcish(p->name, sizeof p->name);
	p->where = rand() / (RAND_MAX / 198 + 1) - 99;
	p->ferociousness = rand() / (RAND_MAX / 11 + 1);
}
static void fill_panda_name(struct name_list_node *const name) {
	fill_panda(name_upcast(name));
}
static void fill_panda_where(struct where_list_node *const where) {
	fill_panda(where_upcast(where));
}
static void fill_panda_fero(struct fero_list_node *const fero) {
	fill_panda(fero_upcast(fero));
}
#define POOL_NAME panda
#define POOL_TYPE struct Panda
#include "pool.h"
static struct name_list_node *panda_name_from_pool(void *const vpool) {
	struct panda_pool *const pool = vpool;
	struct Panda *p = panda_pool_new(pool);
	assert(p); if(!p) return 0;
	return &p->name_node;
}
static struct where_list_node *panda_where_from_pool(void *const vpool) {
	struct panda_pool *const pool = vpool;
	struct Panda *p = panda_pool_new(pool);
	assert(p); if(!p) return 0;
	return &p->where_node;
}
static struct fero_list_node *panda_fero_from_pool(void *const vpool) {
	struct panda_pool *const pool = vpool;
	struct Panda *p = panda_pool_new(pool);
	assert(p); if(!p) return 0;
	return &p->fero_node;
}
static struct Panda *panda_from_pool_extra(struct panda_pool *const pool,
	struct name_list *const n, struct where_list *const w,
	struct fero_list *const f) {
	struct Panda *p = panda_pool_new(pool);
	assert(p && pool && n && w && f);
	if(!p) return 0;
	fill_panda(p);
	name_list_push(n, &p->name_node);
	where_list_push(w, &p->where_node);
	fero_list_push(f, &p->fero_node);
	return p;
}
static void panda_graph(const struct name_list *const n,
	const struct where_list *const w, const struct fero_list *const f) {
	const char *fn = "graph/Pandas.gv";
	FILE *fp;
	assert(n && w && f);
	if(!(fp = fopen(fn, "w"))) { perror(fn); return; }
	fprintf(fp, "digraph {\n"
		/*"\trankdir=LR;\n"*/
		/*"\tnodesep=0;\n"*/);
	/* Overriding private! I probably should not do this? But it's `C`. */
	list_name_subgraph(n, fp, "royalblue",
		offsetof(struct Panda, name_node), 1);
	list_where_subgraph(w, fp, /*"firebrick"*/"orchid",
		offsetof(struct Panda, where_node), 0);
	list_fero_subgraph(f, fp, "darkseagreen",
		offsetof(struct Panda, fero_node), 0);
	fprintf(fp, "\tnode [colour=red, style=filled];\n");
	fprintf(fp, "}\n");
	fclose(fp);
	
}
static void pandas_everywhere(void) {
	struct name_list names;
	struct where_list wheres;
	struct fero_list feros;
	struct panda_pool pandas = POOL_IDLE;
	size_t i;
	name_list_clear(&names), where_list_clear(&wheres), fero_list_clear(&feros);
	for(i = 0; i < 60; i++)
		panda_from_pool_extra(&pandas, &names, &wheres, &feros);
	name_list_sort(&names);
	where_list_sort(&wheres);
	fero_list_sort(&feros);
	panda_graph(&names, &wheres, &feros);
	panda_pool_(&pandas);
}


/* (Fixed width) skip list. (fixme: don't need all that) */

struct layer0_list_node;
static int l0_compare(const struct layer0_list_node *,
	const struct layer0_list_node *);
static void l0_to_string(const struct layer0_list_node *, char (*)[12]);
static void fill_l0(struct layer0_list_node *);
#define LIST_NAME layer0
#define LIST_COMPARE &l0_compare
#define LIST_EXPECT_TRAIT
#include "../src/list.h"
#define LIST_TO_STRING &l0_to_string
#define LIST_TEST &fill_l0
#include "../src/list.h"

struct layer1_list_node;
static int l1_compare(const struct layer1_list_node *,
	const struct layer1_list_node *);
static void l1_to_string(const struct layer1_list_node *, char (*)[12]);
static void fill_l1(struct layer1_list_node *);
#define LIST_NAME layer1
#define LIST_COMPARE &l1_compare
#define LIST_EXPECT_TRAIT
#include "../src/list.h"
#define LIST_TO_STRING &l1_to_string
#define LIST_TEST &fill_l1
#include "../src/list.h"

struct layer2_list_node;
static int l2_compare(const struct layer2_list_node *,
	const struct layer2_list_node *);
static void l2_to_string(const struct layer2_list_node *, char (*)[12]);
static void fill_l2(struct layer2_list_node *);
#define LIST_NAME layer2
#define LIST_COMPARE &l2_compare
#define LIST_EXPECT_TRAIT
#include "../src/list.h"
#define LIST_TO_STRING &l2_to_string
#define LIST_TEST &fill_l2
#include "../src/list.h"

struct Skip {
	struct layer0_list_node l0;
	struct layer1_list_node l1;
	struct layer2_list_node l2;
	unsigned value;
};

static const struct Skip *l0_upcast_c(const struct layer0_list_node *const n) {
	return (const struct Skip *)(const void *)((const char *)n
		- offsetof(struct Skip, l0));
}
static const struct Skip *l1_upcast_c(const struct layer1_list_node *const n) {
	return (const struct Skip *)(const void *)((const char *)n
		- offsetof(struct Skip, l1));
}
static const struct Skip *l2_upcast_c(const struct layer2_list_node *const n) {
	return (const struct Skip *)(const void *)((const char *)n
		- offsetof(struct Skip, l2));
}

static struct Skip *l0_upcast(struct layer0_list_node *const n) {
	return (struct Skip *)(void *)((char *)n - offsetof(struct Skip, l0));
}
static struct Skip *l1_upcast(struct layer1_list_node *const n) {
	return (struct Skip *)(void *)((char *)n - offsetof(struct Skip, l1));
}
static struct Skip *l2_upcast(struct layer2_list_node *const n) {
	return (struct Skip *)(void *)((char *)n - offsetof(struct Skip, l2));
}

static int skip_compare(const struct Skip *const a,
	const struct Skip *const b) {
	return (a->value > b->value) - (b->value > a->value);
}
static int l0_compare(const struct layer0_list_node *a,
	const struct layer0_list_node *b) {
	return skip_compare(l0_upcast_c(a), l0_upcast_c(b));
}
static int l1_compare(const struct layer1_list_node *a,
	const struct layer1_list_node *b) {
	return skip_compare(l1_upcast_c(a), l1_upcast_c(b));
}
static int l2_compare(const struct layer2_list_node *a,
	const struct layer2_list_node *b) {
	return skip_compare(l2_upcast_c(a), l2_upcast_c(b));
}

static void skip_to_string(const struct Skip *const skip,
	char (*const a)[12]) {
	/*assert(RAND_MAX <= 99999999999l);*/
	sprintf(*a, "%d", skip->value);
}
static void l0_to_string(const struct layer0_list_node *const l0,
	char (*const a)[12]) {
	skip_to_string(l0_upcast_c(l0), a);
}
static void l1_to_string(const struct layer1_list_node *const l1,
	char (*const a)[12]) {
	skip_to_string(l1_upcast_c(l1), a);
}
static void l2_to_string(const struct layer2_list_node *const l2,
	char (*const a)[12]) {
	skip_to_string(l2_upcast_c(l2), a);
}

static void fill_skip(struct Skip *const skip) {
	assert(skip);
	/* This is accessing data that we shouldn't; maybe have a fn? (another one
	 that will be used so infrequently that it will only serve to confuse?) */
	skip->l0.prev = 0;
	skip->l0.next = 0;
	skip->l1.prev = 0;
	skip->l1.next = 0;
	skip->l2.prev = 0;
	skip->l2.next = 0;
	skip->value = rand();
}
static void fill_l0(struct layer0_list_node *const l0) {
	fill_skip(l0_upcast(l0));
}
static void fill_l1(struct layer1_list_node *const l1) {
	fill_skip(l1_upcast(l1));
}
static void fill_l2(struct layer2_list_node *const l2) {
	fill_skip(l2_upcast(l2));
}

#define POOL_NAME skip
#define POOL_TYPE struct Skip
#include "pool.h"

static struct layer0_list_node *l0_from_pool(void *const vpool) {
	struct skip_pool *const pool = vpool;
	struct Skip *s = skip_pool_new(pool);
	assert(s); if(!s) return 0;
	return &s->l0;
}
static struct layer1_list_node *l1_from_pool(void *const vpool) {
	struct skip_pool *const pool = vpool;
	struct Skip *s = skip_pool_new(pool);
	assert(s); if(!s) return 0;
	return &s->l1;
}
static struct layer2_list_node *l2_from_pool(void *const vpool) {
	struct skip_pool *const pool = vpool;
	struct Skip *s = skip_pool_new(pool);
	assert(s); if(!s) return 0;
	return &s->l2;
}

struct SkipList {
	struct layer0_list l0list;
	struct layer1_list l1list;
	struct layer2_list l2list;
};

static void skip_clear(struct SkipList *const skip) {
	assert(skip);
	layer0_list_clear(&skip->l0list);
	layer1_list_clear(&skip->l1list);
	layer2_list_clear(&skip->l2list);
}

static void skip_graph(const struct SkipList *const skip) {
	const char *fn = "graph/Skip.gv";
	FILE *fp;
	assert(skip);
	if(!(fp = fopen(fn, "w"))) { perror(fn); return; }
	fprintf(fp, "digraph {\n");
	list_layer0_subgraph(&skip->l0list, fp, "royalblue",
		offsetof(struct Skip, l0), 1);
	list_layer1_subgraph(&skip->l1list, fp, "firebrick",
		offsetof(struct Skip, l1), 0);
	list_layer2_subgraph(&skip->l2list, fp, "darkseagreen",
		offsetof(struct Skip, l2), 0);
	fprintf(fp, "\tnode [colour=red, style=filled];\n");
	fprintf(fp, "}\n");
	fclose(fp);
	
}

static void skips_everywhere(void) {
	const size_t i_lim = 1000;
	size_t i;
	unsigned r;
	struct skip_pool skips = POOL_IDLE;
	struct SkipList s;
	skip_clear(&s);
	assert(RAND_MAX / 16 > i_lim);
	for(i = 0; i < i_lim; i++) {
		/* Add random data. */
		struct Skip *const skip = skip_pool_new(&skips);
		struct layer2_list_node *l2, *l2_pr;
		struct layer1_list_node *l1, *l1_pr, *l1_lim;
		struct layer0_list_node *l0, *l0_pr, *l0_lim;
		if(!skip) { assert(0); goto finally; }
		fill_skip(skip);
		/* Find the order. */
		for(l2 = 0, l2_pr = layer2_list_first(&s.l2list);
			l2_pr && l2_compare(&skip->l2, l2_pr) > 0;
			l2 = l2_pr, l2_pr = layer2_list_next(l2_pr));
		l1 = l2 ? &l2_upcast(l2)->l1 : 0;
		l1_lim = l2_pr ? &l2_upcast(l2_pr)->l1 : 0;
		for(l1_pr = l1 ? layer1_list_next(l1) : layer1_list_first(&s.l1list);
			l1_pr && (l1_lim ? l1_pr != l1_lim : 1)
			&& l1_compare(&skip->l1, l1_pr) > 0;
			l1 = l1_pr, l1_pr = layer1_list_next(l1_pr));
		l0 = l1 ? &l1_upcast(l1)->l0 : 0;
		l0_lim = l1_pr ? &l1_upcast(l1_pr)->l0 : 0;
		for(l0_pr = l0 ? layer0_list_next(l0) : layer0_list_first(&s.l0list);
			l0_pr && (l0_lim ? l0_pr != l0_lim : 1)
			&& l0_compare(&skip->l0, l0_pr) > 0;
			l0 = l0_pr, l0_pr = layer0_list_next(l0_pr));
		/* Since we have a fixed number of lists,
		 \> n = \log_{1/p} size
		 \> n = \frac{\log size}{\log 1/p}
		 \> n \log 1/p = \log size
		 \> p = 2^n / size
		 Apparently, some modificalion is required for `n` fixed. */
		l0 ? layer0_list_add_after(l0, &skip->l0)
			: layer0_list_unshift(&s.l0list, &skip->l0);
		r = rand();
		if(r > RAND_MAX / 4 + i) continue;
		l1 ? layer1_list_add_after(l1, &skip->l1) :
			layer1_list_unshift(&s.l1list, &skip->l1);
		if(r > RAND_MAX / 16 + i) continue;
		l2 ? layer2_list_add_after(l2, &skip->l2) :
			layer2_list_unshift(&s.l2list, &skip->l2);
	}
	skip_graph(&s);
finally:
	skip_pool(&skips);
}


/* Animals! See <../web/animals.gv>. */

/* Id is the list that holds all the animals together. */
struct id_list_node;
static void id_to_string(const struct id_list_node *, char (*)[12]);
#define LIST_NAME id
#define LIST_EXPECT_TRAIT
#include "../src/list.h"
#define LIST_TO_STRING &id_to_string
#include "../src/list.h"

enum colour { PINK, RED, BLUE, YELLOW, BEIGE, COLOUR_END };
static const char *const colours[] = { "pink", "red", "blue","yellow", "beige"};
enum { BOTTOM, TOP, RIDING_END };

/* Abstract `Animal`. */
struct animal_vt;
struct animal {
	const struct animal_vt *vt;
	struct id_list_node id;
	char name[16];
	enum colour colour;
};
static const size_t animal_name_size = sizeof ((struct animal *)0)->name;
static struct animal *id_upcast(struct id_list_node *const id) {
	return (struct animal *)(void *)((char *)id
		- offsetof(struct animal, id));
}
static const struct animal *id_upcast_c(const struct id_list_node *const id) {
	return (const struct animal *)(const void *)((const char *)id
		- offsetof(struct animal, id));
}
static void animal_to_string(const struct animal *const animal,
	char (*const a)[12]) {
	strncpy(*a, animal->name, sizeof *a - 1);
	*a[sizeof *a - 1] = '\0';
}
static void id_to_string(const struct id_list_node *const id,
	char (*const a)[12]) {
	animal_to_string(id_upcast_c(id), a);
}

/* `Mount` involves two animals. */
struct mount;
struct mount_info {
	struct mount *steed_of, *riding;
	enum allowed { STEED = 1, RIDER, RIDER_STEED } is_allowed; /* Bit-field. */
};
struct mount {
	struct animal *steed, *rider;
};
#define POOL_NAME mount
#define POOL_TYPE struct mount
#include "pool.h"

/* `Sloth` extends `Animal`. */
struct sloth {
	struct animal animal;
	unsigned hours_slept;
};
#define POOL_NAME sloth
#define POOL_TYPE struct sloth
#include "pool.h"

/* `Emu` extends `Animal`. */
struct emu {
	struct animal animal;
	char favourite_letter;
};
#define POOL_NAME emu
#define POOL_TYPE struct emu
#include "pool.h"

/* `BadEmu` extends `Emu`. */
struct bad_emu {
	struct emu emu;
	struct mount_info mount_info;
	char muhaha[12];
};
#define POOL_NAME bademu
#define POOL_TYPE struct bad_emu
#include "pool.h"

/* `Llama` extends `Animal`. */
struct Llama {
	struct animal animal;
	struct mount_info mount_info;
	unsigned chomps;
};
#define POOL_NAME llama
#define POOL_TYPE struct Llama
#include "pool.h"

/* `Lemur` extends `Animal`. */
struct Lemur {
	struct animal animal;
	struct mount_info mount_info;
};
#define POOL_NAME lemur
#define POOL_TYPE struct Lemur
#include "pool.h"

/* `Bear` extends `Animal`. We have always two or less. */
struct bear {
	struct animal animal;
	int is_active;
	struct mount_info mount_info;
};

/* Id list with backing. */
static struct animals {
	struct id_list list;
	struct mount_pool mounts;
	struct sloth_pool sloths;
	struct emu_pool emus;
	struct bademu_pool bad_emus;
	struct llama_pool llamas;
	struct lemur_pool lemurs;
	struct bear bears[2];
} animals;
static const unsigned no_bears = sizeof(((struct animals *)0)->bears)
	/ sizeof(*((struct animals *)0)->bears);

typedef void (*animal_action_fn)(struct animal *);
typedef struct mount_info *(*animal_mount_info_fn)(struct animal *);

struct animal_vt {
	const char type[16];
	animal_action_fn delete;
	animal_action_fn act/*transmogrify*/;
	animal_mount_info_fn mount_info;
};

static void dismount(struct mount *);

/** @implements animal_action */
static void animal_delete(struct animal *const animal) {
	struct mount_info *mount_info;
	if(!animal) return;
	if((mount_info = animal->vt->mount_info(animal))) {
		if(mount_info->steed_of) dismount(mount_info->steed_of);
		if(mount_info->riding)   dismount(mount_info->riding);
	}
	animal->vt->delete(animal);
}
/** @implements <Id>Action */
static void id_delete(struct id_list_node *const id) {
	animal_delete(id_upcast(id));
}
static void sloth_delete(struct sloth *const sloth) {
	printf("Bye %s.\n", sloth->animal.name);
	id_list_remove(&sloth->animal.id);
	sloth_pool_remove(&animals.sloths, sloth);
}
static void emu_delete(struct emu *const emu) {
	printf("Bye %s.\n", emu->animal.name);
	id_list_remove(&emu->animal.id);
	emu_pool_remove(&animals.emus, emu);
}
static void bad_emu_delete(struct bad_emu *const bad_emu) {
	printf("%s dissapers in a puff of smoke.\n", bad_emu->emu.animal.name);
	id_list_remove(&bad_emu->emu.animal.id);
	bademu_pool_remove(&animals.bad_emus, bad_emu);
}
static void lemur_delete(struct Lemur *const lemur) {
	printf("Bye %s.\n", lemur->animal.name);
	id_list_remove(&lemur->animal.id);
	lemur_pool_remove(&animals.lemurs, lemur);
}
static void llama_delete(struct Llama *const llama) {
	printf("Bye %s.\n", llama->animal.name);
	id_list_remove(&llama->animal.id);
	llama_pool_remove(&animals.llamas, llama);
}
static void bear_delete(struct bear *const bear) {
	if(!bear->is_active) return;
	printf("Bye %s.\n", bear->animal.name);
	id_list_remove(&bear->animal.id);
	bear->is_active = 0;
}

/** @implements animal_action */
static void animal_act(struct animal *const animal) {
	assert(animal);
	animal->vt->act(animal);
}
/** @implements <Id>Action */
static void id_act(struct id_list_node *const id) {
	animal_act(id_upcast(id));
}
static void sloth_act(struct sloth *const sloth) {
	printf("%s %s has favourite colour %s and has been sleeping %u hours.\n",
		sloth->animal.vt->type, sloth->animal.name,
		colours[sloth->animal.colour], sloth->hours_slept);
}
static void emu_act(struct emu *const emu) {
	printf("%s %s has favourite colour %s and favourite letter %c.\n",
		emu->animal.vt->type, emu->animal.name,
		colours[emu->animal.colour], emu->favourite_letter);
}
static void bad_emu_act(struct bad_emu *const bad_emu) {
	char ride[128] = "";
	if(bad_emu->mount_info.riding) {
		const struct animal *const steed = bad_emu->mount_info.riding->steed;
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
		const struct animal *const steed = lemur->mount_info.riding->steed;
		assert(steed);
		sprintf(ride, " They are riding on %s the %s.",
			steed->name, steed->vt->type);
	}
	printf("%s %s.%s\n", lemur->animal.vt->type, lemur->animal.name, ride);
}
static void llama_act(struct Llama *const llama) {
	char ride[128] = "";
	if(llama->mount_info.steed_of) {
		const struct animal *const rider = llama->mount_info.steed_of->rider;
		assert(rider);
		sprintf(ride, " They are the noble steed of %s the %s.",
			rider->name, rider->vt->type);
	}
	printf("%s %s has chomped %u fingers today.%s\n",
		llama->animal.vt->type, llama->animal.name, llama->chomps, ride);
}
static void bear_act(struct bear *const bear) {
	char ride[128] = " chilling";
	if(bear->mount_info.riding) {
		const struct animal *const steed = bear->mount_info.riding->steed;
		assert(steed);
		sprintf(ride + strlen(ride), " riding on %s the %s",
			steed->name, steed->vt->type);
	}
	if(bear->mount_info.steed_of) {
		const struct animal *const rider = bear->mount_info.steed_of->rider;
		assert(rider);
		sprintf(ride + strlen(ride), " being ridden by %s the %s",
			rider->name, rider->vt->type);
	}
	printf("%s %s is%s.\n", bear->animal.vt->type, bear->animal.name, ride);
}

/** @implements animal_mount_info */
static struct mount_info *animal_mount_info(struct animal *const animal) {
	assert(animal);
	return animal->vt->mount_info(animal);
}
static struct mount_info *no_mount_info(struct animal *const animal) {
	(void)animal;
	return 0;
}
static struct mount_info *bad_emu_mount_info(struct bad_emu *const bad_emu) {
	return &bad_emu->mount_info;
}
static struct mount_info *lemur_mount_info(struct Lemur *const lemur) {
	return &lemur->mount_info;
}
static struct mount_info *llama_mount_info(struct Llama *const llama) {
	return &llama->mount_info;
}
static struct mount_info *bear_mount_info(struct bear *const bear) {
	return &bear->mount_info;
}

/* Static data containing the functions defined above. Because `struct animal`
 is always the first item in every animal, we can cast them. This is not really
 a good design choice, in hindsight, (should have accepted all `Animal` and
 upcast.)
 fixme: this is technically UB, and doesn't have to be. Fix. */
static struct animal_vt sloth_vt = {
	"Sloth",
	(animal_action_fn)&sloth_delete,
	(animal_action_fn)&sloth_act,
	&no_mount_info
};
static struct animal_vt emu_vt = {
	"Emu",
	(animal_action_fn)&emu_delete,
	(animal_action_fn)&emu_act,
	&no_mount_info
};
static struct animal_vt bad_emu_vt = {
	"Emu",
	(animal_action_fn)&bad_emu_delete,
	(animal_action_fn)&bad_emu_act,
	(animal_mount_info_fn)&bad_emu_mount_info
};
static struct animal_vt lemur_vt = {
	"Lemur",
	(animal_action_fn)&lemur_delete,
	(animal_action_fn)&lemur_act,
	(animal_mount_info_fn)&lemur_mount_info
};
static struct animal_vt llama_vt = {
	"Llama",
	(animal_action_fn)&llama_delete,
	(animal_action_fn)&llama_act,
	(animal_mount_info_fn)&llama_mount_info
};
static struct animal_vt bear_vt = {
	"Bear",
	(animal_action_fn)&bear_delete,
	(animal_action_fn)&bear_act,
	(animal_mount_info_fn)&bear_mount_info
};

/** Helper for delete. */
static void dismount(struct mount *const mount) {
	assert(mount && mount->steed && mount->rider);
	printf("%s the %s dismounts %s the %s.\n", mount->rider->name,
		mount->rider->vt->type, mount->steed->name, mount->steed->vt->type);
	animal_mount_info(mount->steed)->steed_of = 0;
	animal_mount_info(mount->rider)->riding   = 0;
	mount_pool_remove(&animals.mounts, mount);
}

/** Only called from constructors of children. */
static void animal_filler(struct animal *const animal,
	const struct animal_vt *const vt) {
	assert(animal && vt);
	animal->vt     = vt;
	animal->colour = (enum colour)(rand() / (RAND_MAX / COLOUR_END + 1));
	Orcish(animal->name, sizeof animal->name);
}
static void mount_info_filler(struct mount_info *const this,
	struct animal *const animal, const enum allowed is_allowed) {
	assert(this && animal && animal->vt->mount_info(animal) == this);
	this->steed_of = this->riding = 0;
	this->is_allowed = is_allowed;
}
/** Destructor for all. */
static void Animals_(void) {
	id_list_clear(&animals.list);
	lemur_pool_(&animals.lemurs);
	llama_pool_(&animals.llamas);
	bademu_pool_(&animals.bad_emus);
	emu_pool_(&animals.emus);
	sloth_pool_(&animals.sloths);
	mount_pool_(&animals.mounts);
}
/** Constructor for all. */
static void Animals(void) {
	struct bear *bear, *end;
	id_list_clear(&animals.list);
	/*assert() PoolIsIdle? */
	sloth_pool(&animals.sloths);
	emu_pool(&animals.emus);
	bademu_pool(&animals.bad_emus);
	llama_pool(&animals.llamas);
	lemur_pool(&animals.lemurs);
	for(bear = animals.bears, end = bear + no_bears; bear < end; bear++)
		bear->is_active = 0;
}
/** Random constructor for a `Sloth`. */
static struct sloth *sloth(void) {
	struct sloth *s;
	if(!(s = sloth_pool_new(&animals.sloths))) return 0;
	animal_filler(&s->animal, &sloth_vt);
	s->hours_slept = rand() / (RAND_MAX / 10 + 1) + 5;
	id_list_push(&animals.list, &s->animal.id);
	return s;
}
/** Random constructor for an `Emu`. */
static struct emu *emu(void) {
	struct emu *e;
	if(!(e = emu_pool_new(&animals.emus))) return 0;
	animal_filler(&e->animal, &emu_vt);
	e->favourite_letter = 'a' + rand() / (RAND_MAX / 26 + 1);
	id_list_push(&animals.list, &e->animal.id);
	return e;
}
/** Random constructor for a `BadEmu`. */
static struct bad_emu *bad_emu(void) {
	struct bad_emu *e;
	if(!(e = bademu_pool_new(&animals.bad_emus))) return 0;
	animal_filler(&e->emu.animal, &bad_emu_vt);
	e->emu.favourite_letter = 'a' + rand() / (RAND_MAX / 26 + 1);
	mount_info_filler(&e->mount_info, &e->emu.animal, RIDER);
	Orcish(e->muhaha, sizeof e->muhaha);
	id_list_push(&animals.list, &e->emu.animal.id);
	return e;
}
/** Random constructor for a `Llama`. */
static struct Llama *llama(void) {
	struct Llama *l;
	if(!(l = llama_pool_new(&animals.llamas))) return 0;
	animal_filler(&l->animal, &llama_vt);
	mount_info_filler(&l->mount_info, &l->animal, STEED);
	l->chomps = 5 + rand() / (RAND_MAX / 10 + 1);
	id_list_push(&animals.list, &l->animal.id);
	return l;
}
/** Random constructor for a `Lemur`. */
static struct Lemur *lemur(void) {
	struct Lemur *l;
	if(!(l = lemur_pool_new(&animals.lemurs))) return 0;
	animal_filler(&l->animal, &lemur_vt);
	mount_info_filler(&l->mount_info, &l->animal, RIDER);
	id_list_push(&animals.list, &l->animal.id);
	return l;
}
/** Constructor for a `Bear`.
 @param[no] Has to be `[0, 1]`. You can not overwrite without deleting it first.
 @param[name] Bears have non-random names. */
static struct bear *bear(const unsigned no, const char *const name) {
	struct bear *b;
	assert(name && no < no_bears);
	if(no >= no_bears) return 0;
	b = animals.bears + no;
	if(b->is_active) { fprintf(stderr, "Bear is active.\n"); return 0; }
	animal_filler(&b->animal, &bear_vt);
	strncpy(b->animal.name, name, animal_name_size - 1),
		b->animal.name[animal_name_size - 1] = '\0';
	b->is_active = 1;
	mount_info_filler(&b->mount_info, &b->animal, STEED | RIDER);
	id_list_push(&animals.list, &b->animal.id);
	return b;
}

/** Cause `a` to try to ride `b`. If `a` or `b` is null, causes that connection
 to be broken.
 @return Success. */
static int ride(struct animal *const a, struct animal *const b) {
	struct animal *erase, *steed = 0, *rider = 0;
	struct mount *mount;
	if((!a && !b) || a == b) return 0;
	erase = a ? b ? 0 : a : b;
	if(erase) {
		const struct mount_info *const mi = erase->vt->mount_info(erase);
		if(!mi) return fprintf(stderr, "Animal %s the %s does not have mount "
			"information.\n", erase->name, erase->vt->type), 0;
		if(mi->riding)   dismount(mi->riding);
		if(mi->steed_of) dismount(mi->steed_of);
		assert(!mi->steed_of && !mi->riding);
		return 1;
	} else {
		struct mount_info *const ami = a->vt->mount_info(a),
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
	mount = mount_pool_new(&animals.mounts);
	mount->steed = steed, animal_mount_info(steed)->steed_of = mount;
	mount->rider = rider, animal_mount_info(rider)->riding   = mount;
	printf("%s the %s mounts %s the %s.\n", rider->name, rider->vt->type,
		steed->name, steed->vt->type);
	return 1;
}
/** Prints something out on all the `animals`. */
static void animals_act(void) {
	size_t count = 0;
	struct id_list_node *id;
	for(id = id_list_first(&animals.list); id; id = id_list_next(id)) count++;
	printf("There are %lu animals.\n", (unsigned long)count);
	id_list_for_each(&animals.list, &id_act);
}
/** Clears all the `animals`. */
static void animals_clear(void) {
	id_list_for_each(&animals.list, &id_delete);
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
		struct id_list_node *id = 0, *prev_id = 0;
		struct bear *w, *n;
		const unsigned animal_no = 10000/*00*/;
		unsigned i;
		n = bear(1, "Napoleon");
		for(i = 0; i < animal_no; i++) {
			float r = (float)(rand() / ((double)RAND_MAX + 1));
			if(r < 0.25f) {
				if(!sloth()) break;
			} else if(r < 0.45f) {
				if(!emu()) break;
			} else if(r < 0.55f) {
				if(!bad_emu()) break;
			} else if(r < 0.8f) {
				if(!llama()) break;
			} else {
				if(!lemur()) break;
			}
		}
		if(i != animal_no) break;
		w = bear(0, "Winnie");
		for(id = id_list_first(&animals.list); id; id = id_list_next(id)) {
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
	struct order_node_pool olls = POOL_IDLE;
	struct no_pool nos = POOL_IDLE;
	struct letter_pool ls = POOL_IDLE;
	struct panda_pool pandas = POOL_IDLE;
	struct skip_pool skips = POOL_IDLE;
	order_list_test(&order_from_pool, &olls), order_node_pool_(&olls);
	no_list_test(&no_from_pool, &nos), no_pool_(&nos);
	letter_list_test(&letter_from_pool, &ls), letter_pool_(&ls);
	name_list_test(&panda_name_from_pool, &pandas), panda_pool_(&pandas);
	where_list_test(&panda_where_from_pool, &pandas), panda_pool_(&pandas);
	fero_list_test(&panda_fero_from_pool, &pandas), panda_pool_(&pandas);
	layer0_list_test(&l0_from_pool, &skips), skip_pool_clear(&skips);
	layer1_list_test(&l1_from_pool, &skips), skip_pool_clear(&skips);
	layer2_list_test(&l2_from_pool, &skips), skip_pool_clear(&skips);
	pandas_everywhere();
	skips_everywhere();
	animals_everywhere();
	return EXIT_SUCCESS;
}
