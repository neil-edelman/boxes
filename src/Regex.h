#ifndef REGEX_H /* <-- guards */
#define REGEX_H

struct Regex;

void Regex_(struct Regex **const pre);
struct Regex *Regex(const char *const compile);
const char *RegexMatch(const struct Regex *const re, const char *const match);
int RegexOut(const struct Regex *const re, FILE *const fp);

#endif /* guards --> */
