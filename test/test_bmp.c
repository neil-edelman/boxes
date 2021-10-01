#include <stdlib.h>
#include <stdio.h>

#define BMP_NAME b
#define BMP_BITS 100
#define BMP_TEST
#include "../src/bmp.h"

/** Entry point. @return Either EXIT_SUCCESS or EXIT_FAILURE. */
int main(void) {
	b_bmp_test();
	return EXIT_SUCCESS;
}
