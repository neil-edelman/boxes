/* This header is for potentially multiple files, but only one can contain the
 definition. All others just have declarations by `ARRAY_DECLARE_ONLY`. It's
 more convenient to have implicit declarations unless otherwise stated. We make
 up a preprocessor variable that flips them. */
#ifdef DEFINE
#	undef DEFINE
#else
#	define ARRAY_DECLARE_ONLY
#endif
#define ARRAY_NAME header
#define ARRAY_TYPE int
#define ARRAY_TEST
#define ARRAY_COMPARE
#define ARRAY_TO_STRING
#define ARRAY_NON_STATIC
#include "../src/array.h"
