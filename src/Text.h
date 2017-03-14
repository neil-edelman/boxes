#ifndef HAVE_TEXT_H /* <-- guards */
#define HAVE_TEXT_H

struct Text;

typedef void (*TextAction)(struct Text *const);

struct TextPattern {
	const char *start, *end;
	TextAction transform;
};

struct TextCut {
	int is;
	char *pos, stored;
};

struct Text *Text(void);
void Text_(struct Text **const this_ptr);
const char *TextToString(const struct Text *const this);
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
int TextTransform(struct Text *const this, const char *fmt);
int TextMatch(struct Text *const this, const struct TextPattern *const patterns,
	const size_t patterns_size);
int TextGetMatchParentInfo(struct Text **const parent_ptr,
	size_t *const start_ptr, size_t *const end_ptr);
void TextCut(struct TextCut *const this, char *const pos);
void TextUncut(struct TextCut *const this);

#endif /* --> */
