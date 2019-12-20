#include <stdlib.h> /* EXIT */

struct OrderListLink;

static void order_to_string(const struct OrderListLink *const l,
	char (*const a)[12]) {
	(void)(l);
	*a[0] = '0';
	*a[1] = '\0';
}

#define LIST_NAME Order
#define LIST_TO_STRING &order_to_string
#define LIST_TEST &order_fill
#include "../src/List.h"

int main(void) {
	OrderListTest();
	return EXIT_SUCCESS;
}
