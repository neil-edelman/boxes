#include <stdio.h> /* for FILE */

struct Text;
struct Line;

/** {Line} in {Text} generic action. */
typedef void (*LineAction)(struct Line *const);

/** Prints information on the line to a supplied buffer. */
typedef void (*LinePrint)(const struct Line *const, char *const, const int);

/** Supplied a {Text} and {Line}. */
typedef void (*TextLineAction)(struct Text *const, struct Line *const);

/** Supplied a {const Line} and {Text}, returns a {Line}. */
typedef struct Line *(*LineTextOperator)(const struct Line *const,
	struct Text *const);

struct Text *Text(void);
void Text_(struct Text **ptext);

struct Line *TextNew(struct Text *const text);

void TextReset(struct Text *const text);
const struct Line *TextLine(const struct Text *const text);
const struct Line *TextNext(struct Text *const text);
struct Line *TextCopyLine(const struct Line *const src, struct Text *const dst);
int LineCopyBetween(struct Line *const line,
	const char *const a, const char *const b);
void TextRemove(struct Text *const text);

int TextFile(struct Text *const text, FILE *const fp, const char *const fn);
int TextPrint(struct Text *const text, FILE *const fp, const char *const fmt);
void TextForEach(struct Text *const this, const LineAction action);

const char *LineGet(const struct Line *const line);
size_t LineLength(const struct Line *const line);
void LineSource(const struct Line *const line, char *const a, size_t a_len);
