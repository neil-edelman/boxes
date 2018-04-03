#ifndef STRING_H /* <-- guards */
#define STRING_H

/** Struct to hold information about a string. */
struct String {
	char *text;
	size_t length, capacity[2];
};

void String_(struct String *const);
void String(struct String *const);
struct String *StringClear(struct String *const);
const char *StringGet(const struct String *const);
size_t StringLength(const struct String *const);
size_t StringCodePoints(const struct String *const);
int StringHasContent(const struct String *const);
struct String *StringRightTrim(struct String *const);
struct String *StringTrim(struct String *const);
struct String *StringCopy(struct String *const, const char *const);
struct String *StringCat(struct String *const, const char *const);
struct String *StringNCat(struct String *const, const char *const,const size_t);
struct String *StringBetweenCat(struct String *const, const char *const,
	const char *const);
struct String *StringPrintCat(struct String *const, const char *const, ...);
struct String *StringTransform(struct String *const, const char *);

#endif /* guards --> */
