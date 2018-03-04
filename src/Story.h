#include <stdio.h> /* for FILE */
#include "Text.h"

struct Story;

typedef int (*StoryLinePredicate)(const struct Text *const line,
	const size_t number);

struct Story *Story(void);
void Story_(struct Story **pstory);
int StoryFileCat(struct Story *const this, FILE *const fp);
int StoryWrite(struct Story *const this, FILE *const fp);
void StoryKeepIf(struct Story *const this, const StoryLinePredicate pred);
void StorySplit(struct Story *const this, const char *delims,
	const TextPredicate pred);
