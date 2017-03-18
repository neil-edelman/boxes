#include "Text.h"

struct Relates;
struct Relate;

typedef void (*RelateAction)(struct Relate *const);
typedef int (*RelatePredicate)(const struct Relate *const);

struct RelateParent {
	int is_within;
	size_t start, end;
};

struct Relates *Relates(void);
void Relates_(struct Relates **const this_ptr);
struct Relate *RelatesGetRoot(struct Relates *const this);
const char *RelatesGetError(struct Relates *const this);

const char *RelateKey(const struct Relate *const this);
const char *RelateValue(const struct Relate *const this);
struct Text *RelateGetKey(const struct Relate *const this);
struct Text *RelateGetValue(const struct Relate *const this);
const struct RelateParent *RelateGetKeyParent(const struct Relate *const this);
const struct RelateParent *RelateGetValueParent(const struct Relate *const this);
struct Relate *RelateNewChild(struct Relate *const this);
void RelateForEachTrueChild(struct Relate *const this,
	RelatePredicate p, RelateAction a);
struct Relate *RelateGetChild(const struct Relate *const this,
	const char *const key);
const char *RelateGetChildValue(const struct Relate *const this,
	const char *const key);
struct Text *RelateGetParentValue(const struct Relate *const this);
