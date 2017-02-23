struct Text;

typedef void (*StringTransform)(char *const match);
typedef char *(*StringOperator)(char *const string);

struct TextPattern {
	char *begin;
	char *end;
	StringTransform transform;
};

struct Text *TextFile(char *const fn);
struct Text *TextString(char *const name, char *const str);
char *TextGetString(struct Text *const this);
void Text_(struct Text **const this_ptr);
const char *TextGetError(struct Text *const this);
int TextMatch(struct Text *this, struct TextPattern *const patterns,
	const size_t patterns_size, const StringOperator find_name);
