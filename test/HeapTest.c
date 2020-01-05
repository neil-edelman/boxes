/** @license 2020 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 Test Heap.

 @std C89/90 */

#include <stdlib.h> /* EXIT malloc free */
/*#include <stdio.h>*/  /* fprintf */
/*#include <assert.h>*/ /* assert */
#include <errno.h>  /* errno */

#define HEAP_NAME Int
/*#define HEAP_TO_STRING &int_to_string
#define HEAP_TEST &test_int*/
#include "../src/Heap.h"

#define POOL_NAME Int
#define POOL_TYPE int
#include "Pool.h"

int main(void) {
	return EXIT_SUCCESS;
}
