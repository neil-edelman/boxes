/* Must link `orcish.c` to do tests, another small project that forwards all
 the requests to here so one can change the location. `orcify` is particularly
 useful for debugging—it translates gobbledygook pointer addresses into more
 semi-meaningful deterministic orc names. (So instead of node "0x0000356754220"
 and "0x00003567564220" we have node "Trogdor" and "Gab-ukghash", or
 something.) It also is used extensively in graphs so one can see what's going
 on in real-time, as well as `orcish.c` is also used in testing. However, it is
 not needed; it could be useful. It's here on the repository, but one can
 change it if one needs to have graphs in one place. */
#include "../orcish/orcish.h"
