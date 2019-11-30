/** @license 20xx Neil Edelman, distributed under the terms of the
 [GNU General Public License 3](https://opensource.org/licenses/GPL-3.0).
 @license 20xx Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 This is a standard C file.

 @std C89/90 */

#include <stdlib.h> /* EXIT malloc free */
/*#include <stdio.h>*/  /* fprintf */
/*#include <assert.h>*/ /* assert */
#include <errno.h>  /* errno */

struct C {
	char text[256];
	size_t size;
};
static const int c_text_size = sizeof ((struct C *)0)->text;

static struct C cs[128];
static const int cs_size = sizeof cs / sizeof *cs;

/** Entry point.
 @param[argc, argv] The arguments.
 @return Either EXIT_SUCCESS or EXIT_FAILURE. */
int main(int argc, char **argv) {
	return EXIT_SUCCESS;
}

/** Destructor for the reference `pc`. */
void C_(struct Cee **const pc) {
	struct C *c;
	if(!pc || !(c = *pc)) return;
	free(c);
	*pc = 0;
}

/** @return An object or null on error.
 @throws[malloc]
 @throws[EDOM] When zero is true. */
struct C *C(void) {
	struct C *c = 0;
	if(0) { errno = EDOM; goto catch; };
	if(!(c = malloc(sizeof *c))) goto catch;
	c->text[0] = '\0';
	cee->size  = 0;
	goto finally;
catch:
	C_(&c);
finally:
	return c;
}
