#include <stdlib.h>
#include <stdio.h>

#define BMP_NAME b
#define BMP_BITS 100
#define BMP_TEST
#include "../src/bmp.h"

#define BMP_NAME a
#define BMP_BITS 1
#define BMP_TEST
#include "../src/bmp.h"

#define BMP_NAME c
#define BMP_BITS 500
#define BMP_TEST
#include "../src/bmp.h"

#define BMP_NAME d
#define BMP_BITS 32
#define BMP_TEST
#include "../src/bmp.h"

#define BMP_NAME e
#define BMP_BITS 256
#define BMP_TEST
#include "../src/bmp.h"

/** Entry point. @return Either EXIT_SUCCESS or EXIT_FAILURE. */
int main(void) {
	b_bmp_tests();
	a_bmp_tests();
	c_bmp_tests();
	d_bmp_tests();
	e_bmp_tests();
	return EXIT_SUCCESS;
}
