#include <stdio.h> /* for FILE */

struct Text;
struct Line;




typedef void (*LineAction)(struct Line *const);
typedef int (*LinePredicate)(struct Line *const);

struct Text *Text(void);
void Text_(struct Text **ptext);
struct Text *LineText(const struct Line *const line);
size_t LineNo(const struct Line *const line);
void LineSetNo(struct Line *const line, const size_t no);
struct Line *LinePrevious(struct Line *const line);
struct Line *LineNext(struct Line *const line);
int TextFileCat(struct Text *const this, FILE *const fp);
int TextWrite(struct Text *const this, FILE *const fp);
void TextForEach(struct Text *const this, const LineAction action);
void TextKeepIf(struct Text *const this, const LinePredicate pred);
/*void TextSplit(struct Text *const this, const char *delims,
	const TextPredicate pred);*/
