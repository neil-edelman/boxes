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


int main(void) {
	struct OrderLinkPool olls = POOL_IDLE;
	struct NoPool nos = POOL_IDLE;
	OrderListTest(&order_from_pool, &olls), OrderLinkPool_(&olls);
	NoListTest(&no_from_pool, &nos), NoPool_(&nos);
	return EXIT_SUCCESS;
}
