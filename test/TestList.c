#include <stdlib.h> /* EXIT rand */
#include <stdio.h>  /* printf */

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
	assert(oll);
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
	assert(no);
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
	assert(l);
	return &l->link;
}

int main(void) {
	struct OrderLinkPool olls = POOL_IDLE;
	struct NoPool nos = POOL_IDLE;
	struct LetterPool ls = POOL_IDLE;
	OrderListTest(&order_from_pool, &olls), OrderLinkPool_(&olls);
	NoListTest(&no_from_pool, &nos), NoPool_(&nos);
	LetterListTest(&letter_from_pool, &ls), LetterPool_(&ls);
	return EXIT_SUCCESS;
}
