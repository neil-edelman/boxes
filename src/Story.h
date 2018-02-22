#include <stdio.h> /* for FILE */

struct Story;

struct Story *Story(void);
void Story_(struct Story **pstory);
struct Story *StoryFileCat(struct Story *const this, FILE *const fp);
const char *StoryGetError(struct Story *const this);
