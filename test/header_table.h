#ifdef HEADER_TABLE_DEFINE
#	undef HEADER_TABLE_DEFINE
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

struct header_table { struct static_table _; };

struct header_table header_table(void);
void header_table_(struct header_table *);
void header_table_test(void);
