#ifdef DEFINE
#	undef DEFINE
#else
#	define TABLE_DECLARE_ONLY
#endif
#define TABLE_NAME static
#define TABLE_KEY enum zodiac
#define TABLE_UNHASH
#define TABLE_UINT unsigned char
#define TABLE_TO_STRING
#define TABLE_TEST
#include "../src/table.h"
