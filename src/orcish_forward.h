/* Must link the file produced by compiling `orcish.c` to do tests or get
 graphs. (It is particularly useful for debugging—it translates gobbledygook
 pointer addresses into more semi-meaningful deterministic orc names. We have
 node "Trogdor" and "Gab-ukghash", instead of some numbers.) If one changes the
 directory structure but still could use graphs or testing, all the references
 in `src/` and `test/` are forwarded though this address. */
#include "../orcish/orcish.h"
