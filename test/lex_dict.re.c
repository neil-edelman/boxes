#include <stddef.h>
#include <assert.h>
#include "../test/lex_dict.h"

int lex_dict(struct lex_state *lex) {
	unsigned char *YYCURSOR = (unsigned char *)lex->cursor, *s0;
	int success = 0;
	/*!stags:re2c format = 'unsigned char *@@;\n'; */
start:
	/*!re2c
	re2c:yyfill:enable = 0;
	re2c:define:YYCTYPE = 'unsigned char';

	utf8s = [^\x00-\x7f];
	numbers = [0-9];
	caps = [A-Z];
	lowers = [a-z];
	maybes = [~-];
	words = utf8s | numbers | caps | lowers | maybes;

	"\x00" { goto finally; }
	"\n" { lex->line++; goto start; }
	@s0 words+ {
		success = 1;
		lex->word = (char *)s0;
		switch(*YYCURSOR) {
		case '\n': lex->line++;
		default: *YYCURSOR++ = '\0';
		case 0: break;
		}
		goto finally;
	}
	* { goto start; }
	*/
finally:
	lex->cursor = (char *)YYCURSOR;
	return success;
}
