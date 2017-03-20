#ifndef HAVE_TEXT_H /* <-- guards */
#define HAVE_TEXT_H

struct Text;

typedef void (*TextAction)(struct Text *const);
typedef int (*TextPredicate)(const char *const string, const char *sub);

struct TextPattern {
	const char *start, *end;
	TextAction transform;
};

struct Text *Text(void);
void Text_(struct Text **const this_ptr);
const char *TextGet(const struct Text *const this);
struct Text *TextClear(struct Text *const this);
struct Text *TextTrim(struct Text *const this);
struct Text *TextSep(struct Text *const this, const char *const delims,
	const TextPredicate pred);
struct Text *TextCopy(struct Text *const this, const char *const str);
struct Text *TextCat(struct Text *const this, const char *const str);
struct Text *TextNCat(struct Text *const this, const char *const str,
	const size_t cat_len);
struct Text *TextBetweenCat(struct Text *const this,
	const char *const a, const char *const b);
struct Text *TextFileCat(struct Text *const this, FILE *const fp);
struct Text *TextPrintCat(struct Text *const this, const char *const fmt, ...);
struct Text *TextTransform(struct Text *const this, const char *fmt);
struct Text *TextMatch(struct Text *const this,
	const struct TextPattern *const patterns, const size_t patterns_size);
int TextGetMatchInfo(struct Text **const parent_ptr,
	size_t *const start_ptr, size_t *const end_ptr);
int TextIsError(struct Text *const this);
const char *TextGetError(struct Text *const this);

#endif /* guards --> */
