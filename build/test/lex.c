/* Generated by re2c 3.0 on Mon Sep  5 20:25:54 2022 */
#line 1 "test/lex.re.c"
#include <stddef.h>
#include <assert.h>
#include "../../test/lex.h"

int lex_dict(struct lex_state *lex) {
	unsigned char *YYCURSOR = (unsigned char *)lex->cursor, *s0, *s1;
	int success = 0;
	
#line 12 "build/test/lex.c"
unsigned char *yyt1;
#line 8 "test/lex.re.c"

start:
	
#line 18 "build/test/lex.c"
{
	unsigned char yych;
	yych = *YYCURSOR;
	switch (yych) {
		case 0x00: goto yy1;
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
		case 0x05:
		case 0x06:
		case 0x07:
		case 0x08:
		case '\t':
		case '\v':
		case '\f':
		case '\r':
		case 0x0E:
		case 0x0F:
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
		case 0x18:
		case 0x19:
		case 0x1A:
		case 0x1B:
		case 0x1C:
		case 0x1D:
		case 0x1E:
		case 0x1F:
		case ' ':
		case '!':
		case '"':
		case '#':
		case '$':
		case '%':
		case '&':
		case '\'':
		case '(':
		case ')':
		case '*':
		case '+':
		case ',':
		case '.':
		case '/':
		case ':':
		case ';':
		case '<':
		case '=':
		case '>':
		case '?':
		case '@':
		case '[':
		case '\\':
		case ']':
		case '^':
		case '_':
		case '`':
		case '{':
		case '|':
		case '}':
		case 0x7F: goto yy2;
		case '\n': goto yy3;
		default:
			yyt1 = YYCURSOR;
			goto yy4;
	}
yy1:
	++YYCURSOR;
#line 21 "test/lex.re.c"
	{ goto finally; }
#line 95 "build/test/lex.c"
yy2:
	++YYCURSOR;
#line 33 "test/lex.re.c"
	{ goto start; }
#line 100 "build/test/lex.c"
yy3:
	++YYCURSOR;
#line 22 "test/lex.re.c"
	{ lex->line++; goto start; }
#line 105 "build/test/lex.c"
yy4:
	yych = *++YYCURSOR;
	switch (yych) {
		case 0x00:
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
		case 0x05:
		case 0x06:
		case 0x07:
		case 0x08:
		case '\t':
		case '\n':
		case '\v':
		case '\f':
		case '\r':
		case 0x0E:
		case 0x0F:
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
		case 0x18:
		case 0x19:
		case 0x1A:
		case 0x1B:
		case 0x1C:
		case 0x1D:
		case 0x1E:
		case 0x1F:
		case ' ':
		case '!':
		case '"':
		case '#':
		case '$':
		case '%':
		case '&':
		case '\'':
		case '(':
		case ')':
		case '*':
		case '+':
		case ',':
		case '.':
		case '/':
		case ':':
		case ';':
		case '<':
		case '=':
		case '>':
		case '?':
		case '@':
		case '[':
		case '\\':
		case ']':
		case '^':
		case '_':
		case '`':
		case '{':
		case '|':
		case '}':
		case 0x7F: goto yy5;
		default: goto yy4;
	}
yy5:
	s0 = yyt1;
	s1 = YYCURSOR;
#line 23 "test/lex.re.c"
	{
		success = 1;
		lex->word = (char *)s0;
		switch(*YYCURSOR) {
		case '\n': lex->line++;
		default: *YYCURSOR++ = '\0';
		case 0: break;
		}
		goto finally;
	}
#line 189 "build/test/lex.c"
}
#line 34 "test/lex.re.c"

finally:
	lex->cursor = (char *)YYCURSOR;
	return success;
}
