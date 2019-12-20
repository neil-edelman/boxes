#include <stdlib.h> /* EXIT malloc free */
/*#include <stdio.h>*/  /* fprintf */
/*#include <assert.h>*/ /* assert */
#include <errno.h>  /* errno */

#define LIST_NAME Order
#define LIST_TO_STRING &order_to_string
#define LIST_TEST &order_fill
#include "../src/List.h"

int main(void) {
	return EXIT_SUCCESS;
}
