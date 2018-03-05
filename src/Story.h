#include <stdio.h> /* for FILE */
#include "Text.h"

struct Story;
struct Line;




typedef void (*LineAction)(struct Line *const);
typedef int (*LinePredicate)(struct Line *const);

struct Story *Story(void);
void Story_(struct Story **pstory);
struct Text *LineText(const struct Line *const line);
size_t LineNo(const struct Line *const line);
void LineSetNo(struct Line *const line, const size_t no);
struct Line *LinePrevious(struct Line *const line);
int StoryFileCat(struct Story *const this, FILE *const fp);
int StoryWrite(struct Story *const this, FILE *const fp);
void StoryForEach(struct Story *const this, const LineAction action);
void StoryKeepIf(struct Story *const this, const LinePredicate pred);
void StorySplit(struct Story *const this, const char *delims,
	const TextPredicate pred);
