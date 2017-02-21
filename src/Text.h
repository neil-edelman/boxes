struct Text;

struct Text *Text(char *const fn);
void Text_(struct Text **const this_ptr);
const char *TextGetError(struct Text *const this);
