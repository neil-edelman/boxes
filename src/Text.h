#ifndef HAVE_TEXT_H /* <-- guards */
#define HAVE_TEXT_H

struct Text;

typedef void (*TextAction)(struct Text *const);

struct TextPattern {
	char *start, *end;
	TextAction transform;
};

struct Text *Text(void);
void Text_(struct Text **const this_ptr);
/* fixme: this should be removed */
struct TextArray *TextArray(void);
void TextArray_(struct TextArray **const this_ptr);
const char *TextToString(struct Text *const this);
const char *TextGetError(struct Text *const this);
void TextClear(struct Text *const this);
void TextTrim(struct Text *const this);
int TextCopy(struct Text *const this, const char *const str);
int TextNCopy(struct Text *const this, const char *const str,
	const size_t str_len);
int TextCat(struct Text *const this, const char *const str);
int TextNCat(struct Text *const this, const char *const str,
	const size_t cat_len);
int TextFileCat(struct Text *const this, FILE *const fp);
int TextPrintfCat(struct Text *const this, const char *const fmt, ...);
int TextTransform(struct Text *const this, char *const fmt);
int TextMatch(struct Text *const this, const struct TextPattern *const patterns,
	const size_t patterns_size);

#endif /* --> */
