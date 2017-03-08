struct Text;

typedef void (*TextAction)(struct Text *const);
typedef int (*TextPredicate)(struct Text *const);

struct TextPattern {
	char *begin;
	char *end;
	TextAction transform;
};

struct TextCut {
	int is;
	char *pos;
	char stored;
};

struct Text *TextFile(char *const fn);
struct Text *TextString(char *const name, char *const str);
char *TextGetKey(struct Text *const this);
char *TextGetValue(struct Text *const this);
struct Text *TextGetChildKey(struct Text *const this, const char *const key);
char *TextGetParentValue(struct Text *const this);
int TextGetIsWithinParentValue(struct Text *const this);
size_t TextGetParentStart(struct Text *const this);
size_t TextGetParentEnd(struct Text *const this);
void Text_(struct Text **const this_ptr);
const char *TextGetError(struct Text *const this);
int TextCat(struct Text *const this, char *const cat,
	const size_t *const cat_len_ptr);
int TextCopy(struct Text *const this, char *const str,
	const size_t *const str_len_ptr);
int TextMatch(struct Text *const this, const struct TextPattern *const patterns,
	const size_t patterns_size);
void TextTrim(struct Text *const this);
struct Text *TextNewChild(struct Text *const this,
	char *const key_begin, const size_t key_length,
	char *const value_begin, const size_t value_length);	
int TextEnsureCapacity(struct Text *const this, const size_t capacity);
void TextForEachTrue(struct Text *const this, TextPredicate p, TextAction a);
char *TextToString(struct Text *const this);
char *TextAdd(struct Text *const this, char *const fmt);

void TextCut(struct TextCut *const r, char *const cut);
void TextUncut(struct TextCut *const r);
