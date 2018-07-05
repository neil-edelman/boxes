#include <stdio.h> /* for FILE */

struct Text;
struct Line;

/** {Line} in {Text} generic action. */
typedef void (*LineAction)(struct Line *const);

/** {Line} in {Text} generic predicate. */
typedef int (*LinePredicate)(const struct Line *const);

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

int TextCursorSet(struct Text *const text, struct Line *const set);
const struct Line *TextCursor(const struct Text *const text);
const struct String *LineString(const struct Line *const line);
/*const*/ struct Line *TextNext(struct Text *const text);
/*const*/ struct Line *TextPrevious(struct Text *const text);
struct Line *LineCopyMeta(const struct Line *const src, struct Text *const dst);
int LineBetweenCat(struct Line *const line,
	const char *const a, const char *const b);
void TextRemove(struct Text *const text);
int TextHasContent(const struct Text *const text);

int TextFile(struct Text *const text, FILE *const fp, const char *const fn);
int TextPrint(struct Text *const text, FILE *const fp, const char *const fmt);
void TextForEach(struct Text *const this, const LineAction action);
struct Line *TextAll(const struct Text *const text,
	const LinePredicate predicate);
size_t TextLineCount(const struct Text *const text);

const char *LineGet(const struct Line *const line);
size_t LineLength(const struct Line *const line);
void LineSource(const struct Line *const line, char *const a, size_t a_len);
