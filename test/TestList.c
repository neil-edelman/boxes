#include <stdlib.h> /* EXIT rand */
#include <stdio.h>  /* printf */
#include <string.h> /* strcmp */
#include "Orcish.h"

/* An order with no parent. It could happen. */

struct OrderListLink;
static void order_to_string(const struct OrderListLink *const l,
	char (*const a)[12]) {
	(void)(l);
	(*a)[0] = '0';
	(*a)[1] = '\0';
}
static void order_fill(struct OrderListLink *const l) { (void)(l); }

#define LIST_NAME Order
#define LIST_TO_STRING &order_to_string
#define LIST_TEST &order_fill
#include "../src/List.h"

#define POOL_NAME OrderLink
#define POOL_TYPE struct OrderListLink
#include "Pool.h"
static struct OrderListLink *order_from_pool(void *const volls) {
	struct OrderLinkPool *const olls = volls;
	struct OrderListLink *oll = OrderLinkPoolNew(olls);
	assert(oll); if(!oll) return 0;
	return oll;
}

/* This is the minimum useful example. */

struct NoListLink;
static void no_to_string(const struct NoListLink *, char (*)[12]);
static void no_fill(struct NoListLink *);
static int no_compare(const struct NoListLink *, const struct NoListLink *);

#define LIST_NAME No
#define LIST_COMPARE &no_compare
#define LIST_TO_STRING &no_to_string
#define LIST_TEST &no_fill
#include "../src/List.h"

struct No { struct NoListLink link; int i; };

static int no_compare(const struct NoListLink *const all,
	const struct NoListLink *const bll) {
	/* Can do this because `link` is the first field in `struct No`. */
	const struct No *const a = (const struct No *)all,
		*const b = (const struct No *)bll;
	return (a->i > b->i) - (b->i > a->i);
}
static void no_to_string(const struct NoListLink *const noll,
	char (*const a)[12]) {
	const struct No *const no = (const struct No *)noll;
	/*assert(RAND_MAX <= 99999999999l);*/
	sprintf(*a, "%d", no->i);
}
static void no_fill(struct NoListLink *const noll) {
	struct No *const no = (struct No *)noll;
	no->i = rand() / (RAND_MAX / 100000 + 1);
}
#define POOL_NAME No
#define POOL_TYPE struct No
#include "Pool.h"
static struct NoListLink *no_from_pool(void *const vnos) {
	struct NoPool *const nos = vnos;
	struct No *no = NoPoolNew(nos);
	assert(no); if(!no) return 0;
	return &no->link;
}

/* For testing bin-ops just to be sure the examples were accurate. */

struct LetterListLink;
static void letter_to_string(const struct LetterListLink *, char (*)[12]);
static void letter_fill(struct LetterListLink *);
static int letter_compare(const struct LetterListLink *,
	const struct LetterListLink *);

#define LIST_NAME Letter
#define LIST_COMPARE &letter_compare
#define LIST_TO_STRING &letter_to_string
#define LIST_TEST &letter_fill
#include "../src/List.h"

struct Letter { struct LetterListLink link; char letter; };

static int letter_compare(const struct LetterListLink *const all,
	const struct LetterListLink *const bll) {
	/* Can do this because `link` is the first field in `struct No`. */
	const struct Letter *const a = (const struct Letter *)all,
		*const b = (const struct Letter *)bll;
	return (a->letter > b->letter) - (b->letter > a->letter);
}
static void letter_to_string(const struct LetterListLink *const lll,
	char (*const a)[12]) {
	const struct Letter *const l = (const struct Letter *)lll;
	(*a)[0] = l->letter;
	(*a)[1] = '\0';
}
static void letter_fill(struct LetterListLink *const lll) {
	struct Letter *const l = (struct Letter *)lll;
	l->letter = rand() / (RAND_MAX / 26 + 1) + 'A';
}
#define POOL_NAME Letter
#define POOL_TYPE struct Letter
#include "Pool.h"
static struct LetterListLink *letter_from_pool(void *const vls) {
	struct LetterPool *const ls = vls;
	struct Letter *l = LetterPoolNew(ls);
	assert(l); if(!l) return 0;
	return &l->link;
}

/* Multi-list. Three lists at once; because why not? */

struct NameListLink;
static void name_to_string(const struct NameListLink *, char (*)[12]);
static void fill_panda_name(struct NameListLink *);
static int name_compare(const struct NameListLink *,
	const struct NameListLink *);

#define LIST_NAME Name
#define LIST_COMPARE &name_compare
#define LIST_TO_STRING &name_to_string
#define LIST_TEST &fill_panda_name
#include "../src/List.h"

struct WhereListLink;
static void where_to_string(const struct WhereListLink *, char (*)[12]);
static void fill_panda_where(struct WhereListLink *);
static int where_compare(const struct WhereListLink *,
	const struct WhereListLink *);

#define LIST_NAME Where
#define LIST_COMPARE &where_compare
#define LIST_TO_STRING &where_to_string
#define LIST_TEST &fill_panda_where
#include "../src/List.h"

struct FeroListLink;
static void fero_to_string(const struct FeroListLink *, char (*)[12]);
static void fill_panda_fero(struct FeroListLink *);
static int fero_compare(const struct FeroListLink *,
	const struct FeroListLink *);

#define LIST_NAME Fero
#define LIST_COMPARE &fero_compare
#define LIST_TO_STRING &fero_to_string
#define LIST_TEST &fill_panda_fero
#include "../src/List.h"

struct Panda {
	char name[12];
	struct NameListLink name_link;
	int where;
	struct WhereListLink where_link;
	int ferociousness;
	struct FeroListLink fero_link;
};

static struct Panda *name_upcast(struct NameListLink *const n) {
	return (struct Panda *)(void *)((char *)n
		- offsetof(struct Panda, name_link));
}
static const struct Panda *name_upcast_c(const struct NameListLink *const n) {
	return (const struct Panda *)(const void *)((const char *)n
		- offsetof(struct Panda, name_link));
}
static struct Panda *where_upcast(struct WhereListLink *const w) {
	return (struct Panda *)(void *)((char *)w
		- offsetof(struct Panda, where_link));
}
static const struct Panda *where_upcast_c(const struct WhereListLink *const w) {
	return (const struct Panda *)(const void *)((const char *)w
		- offsetof(struct Panda, where_link));
}
static struct Panda *fero_upcast(struct FeroListLink *const f) {
	return (struct Panda *)(void *)((char *)f
		- offsetof(struct Panda, fero_link));
}
static const struct Panda *fero_upcast_c(const struct FeroListLink *const f) {
	return (const struct Panda *)(const void *)((const char *)f
		- offsetof(struct Panda, fero_link));
}
static void panda_to_string(const struct Panda *const p, char (*const a)[12]) {
	sprintf(*a, "%.11s", p->name);
}
static void name_to_string(const struct NameListLink *const n,
	char (*const a)[12]) {
	panda_to_string(name_upcast_c(n), a);
}
static void where_to_string(const struct WhereListLink *const w,
	char (*const a)[12]) {
	panda_to_string(where_upcast_c(w), a);
}
static void fero_to_string(const struct FeroListLink *const f,
	char (*const a)[12]) {
	panda_to_string(fero_upcast_c(f), a);
}
static int name_compare(const struct NameListLink *const a,
	const struct NameListLink *const b) {
	return strcmp(name_upcast_c(a)->name, name_upcast_c(b)->name);
}
static int int_compare(const int a, const int b) {
	return (a > b) - (b > a);
}
static int where_compare(const struct WhereListLink *const a,
	const struct WhereListLink *const b) {
	return int_compare(where_upcast_c(a)->where, where_upcast_c(b)->where);
}
static int fero_compare(const struct FeroListLink *const a,
	const struct FeroListLink *const b) {
	return int_compare(fero_upcast_c(a)->ferociousness,
		fero_upcast_c(b)->ferociousness);
}
static void fill_panda(struct Panda *const p) {
	assert(p);
	Orcish(p->name, sizeof p->name);
	p->where = rand() / (RAND_MAX / 198 + 1) - 99;
	p->ferociousness = rand() / (RAND_MAX / 11 + 1);
}
static void fill_panda_name(struct NameListLink *const name) {
	fill_panda(name_upcast(name));
}
static void fill_panda_where(struct WhereListLink *const where) {
	fill_panda(where_upcast(where));
}
static void fill_panda_fero(struct FeroListLink *const fero) {
	fill_panda(fero_upcast(fero));
}
#define POOL_NAME Panda
#define POOL_TYPE struct Panda
#include "Pool.h"
static struct NameListLink *panda_name_from_pool(void *const vpool) {
	struct PandaPool *const pool = vpool;
	struct Panda *p = PandaPoolNew(pool);
	assert(p); if(!p) return 0;
	return &p->name_link;
}
static struct WhereListLink *panda_where_from_pool(void *const vpool) {
	struct PandaPool *const pool = vpool;
	struct Panda *p = PandaPoolNew(pool);
	assert(p); if(!p) return 0;
	return &p->where_link;
}
static struct FeroListLink *panda_fero_from_pool(void *const vpool) {
	struct PandaPool *const pool = vpool;
	struct Panda *p = PandaPoolNew(pool);
	assert(p); if(!p) return 0;
	return &p->fero_link;
}
static struct Panda *panda_from_pool_extra(struct PandaPool *const pool,
	struct NameList *const n, struct WhereList *const w,
	struct FeroList *const f) {
	struct Panda *p = PandaPoolNew(pool);
	assert(p && pool && n && w && f);
	if(!p) return 0;
	fill_panda(p);
	NameListPush(n, &p->name_link);
	WhereListPush(w, &p->where_link);
	FeroListPush(f, &p->fero_link);
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
		"\tnodesep=0.05;\n");
	set_Name_subgraph(n, fp, "royalblue",
		offsetof(struct Panda, name_link), 1);
	set_Where_subgraph(w, fp, /*"firebrick"*/"orchid",
		offsetof(struct Panda, where_link), 0);
	set_Fero_subgraph(f, fp, "darkseagreen",
		offsetof(struct Panda, fero_link), 0);
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
	for(i = 0; i < 80; i++)
		panda_from_pool_extra(&pandas, &names, &wheres, &feros);
	NameListSort(&names);
	WhereListSort(&wheres);
	FeroListSort(&feros);
	panda_graph(&names, &wheres, &feros);
	PandaPool_(&pandas);
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
	return EXIT_SUCCESS;
}
