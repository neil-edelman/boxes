#ifndef PATTERN_H /* <-- guards (@fixme Are they really necessary?) */
#define PATTERN_H

struct Pattern;

void Pattern_(struct Pattern **const);
struct Pattern *Pattern(const char *const);
const char *PatternMatch(const struct Pattern *const, const char *const);
int PatternOut(const struct Pattern *const, FILE *const);

#endif /* guards --> */
