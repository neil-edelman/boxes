#include <stdio.h> /* for FILE */

struct Text;
struct Line;




typedef void (*LineAction)(struct Line *const);
typedef int (*LinePredicate)(struct Line *const);

struct Text *Text(void);
void Text_(struct Text **ptext);
struct Line *TextFirst(struct Text *const text);
struct Line *TextNext(struct Line *const line);
int TextFile(struct Text *const text, FILE *const fp, const char *const fn);
int TextWrite(struct Text *const text, FILE *const fp);
