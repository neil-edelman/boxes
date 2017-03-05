struct Text;

typedef void (*TextAction)(struct Text *const match);
typedef int (*TextPredicate)(const char *const);

struct TextPattern {
	char *begin;
	char *end;
	TextAction transform;
};

struct Text *TextFile(char *const fn);
struct Text *TextString(char *const name, char *const str);
char *TextGetKey(struct Text *const this);
char *TextGetValue(struct Text *const this);
const char *TextGetParentBuffer(struct Text *const this);
size_t TextGetParentStart(struct Text *const this);
size_t TextGetParentEnd(struct Text *const this);
void Text_(struct Text **const this_ptr);
const char *TextGetError(struct Text *const this);
int TextMatch(struct Text *this, const struct TextPattern *const patterns,
	const size_t patterns_size);
struct Text *TextNewChild(struct Text *const this,
	char *const key_begin, const size_t key_length,
	char *const value_begin, const size_t value_length);	
int TextEnsureCapacity(struct Text *const this, const size_t capacity);
void TextForEachPassed(struct Text *const this, TextPredicate p, TextAction a);
char *TextToString(struct Text *const this);
char *TextAdd(struct Text *const this, char *const fmt);
