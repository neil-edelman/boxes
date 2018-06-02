#include <stdio.h> /* for FILE */

struct Text;
struct Line;

/** {Line} in {Text} generic action. */
typedef void (*LineAction)(struct Line *const);

/** Prints information on the line to a supplied buffer. */
typedef void (*LinePrint)(const struct Line *const, char *const, const int);

/** Usually outputs text. */
typedef int (*LineOutput)(const struct Line *const, FILE *const);

struct Text *Text(void);
void Text_(struct Text **ptext);

struct Line *TextNew(struct Text *const text);

void TextReset(struct Text *const text);
const char *TextNext(struct Text *const text);

int TextFile(struct Text *const text, FILE *const fp, const char *const fn);
int TextOutput(struct Text *const text, const LineOutput out, FILE *const fp);
int TextPrint(struct Text *const text, FILE *const fp, const char *const fmt);
void TextForEach(struct Text *const this, const LineAction action);

const char *LineGet(const struct Line *const line);
void LineSource(const struct Line *const line, char *const a, size_t a_len);
