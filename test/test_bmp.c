#include <stdlib.h>
#include <stdio.h>

#define BMP_NAME b
#define BMP_BITS 100
#define BMP_TEST
#include "../src/bmp.h"

/** Entry point. @return Either EXIT_SUCCESS or EXIT_FAILURE. */
int main(void) {
	struct b_bmp b;
	printf("size: %lu.\n", sizeof b.chunk);
	b_bmp_test();
	return EXIT_SUCCESS;
}
