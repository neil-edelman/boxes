#include <stddef.h>
struct lex_state { char *cursor, *word; size_t line; };
int lex_dict(struct lex_state *);
