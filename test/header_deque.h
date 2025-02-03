/* This header is for potentially multiple files, but only one can contain the
 definition. All others just have declarations by `DEQUE_DECLARE_ONLY`. It's
 more convenient to have implicit declarations unless otherwise stated. We make
 up a preprocessor variable that flips them. */
#ifdef DEFINE
#	undef DEFINE
#else
#	define DEQUE_DECLARE_ONLY
#endif
#define DEQUE_NAME header
#define DEQUE_TYPE int
#define DEQUE_TEST
#define DEQUE_TO_STRING
#define DEQUE_NON_STATIC
#include "../src/deque.h"
