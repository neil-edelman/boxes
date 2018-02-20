/** 2018 Neil Edelman, distributed under the terms of the MIT License;
 see readme.txt, or \url{ https://opensource.org/licenses/MIT }.

 A {Story} is composed of {Text} in a way that makes them easy and fast to
 edit large strings.

 @title		Story
 @author	Neil
 @std		C89/90
 @version	2018-01
 @since		2018-01 */

#include <stdlib.h> /* malloc free */
#include <stdio.h>  /* fprintf */
#include "Text.h"
#include "Story.h"

#define LIST_NAME Line
#define LIST_TYPE struct Text *
#include "List.h" /* Defines {TextList}. */



enum Error {
	E_NO_ERROR,
	E_ERRNO,
	E_TOO_LARGE,
	E_LOCKED,
	E_OTHER_LOCKED,
	E_METRIC,
	E_WRONG_TYPE
};
static const char *const error_explination[] = {
	"no error",
	0,
	"tstory large for index",
	"list is locked",
	"other list is locked",
	"metric not set",
	"incompatible list types"
};

struct Story {
	char text[256];
	size_t size;
	size_t capacity[2];
};
static const int story_text_size = sizeof((struct Story *)0)->text / sizeof(char);

/** @implements Metric */
static int difference(int *const a, int *const b) {
	return *a - *b;
}

/** Entry point.
 @param argc: The number of arguments, starting with the programme name.
 @param argv: The arguments.
 @return Either EXIT_SUCCESS or EXIT_FAILURE. */
int main(int argc, char **argv) {
	return EXIT_SUCCESS;
}

/* public */

/** Constructor.
 @return An object or a null pointer, if the object couldn't be created. */
struct Story *Story(void) {
	struct Story *story;

	if(0) {
		fprintf(stderr, "Story: 0 was true.\n");
		return 0;
	}
	if(!(story = malloc(sizeof(struct Story)))) {
		perror("Story constructor");
		Story_(&story);
		return 0;
	}
	story->size        = 0;
	fprintf(stderr, "Story: new, #%p.\n", (void *)story);
	if(0) {
		fprintf(stderr, "Story: did something with #%p.\n", (void *)story);
		Story_(&story);
		return 0;
	}

	return story;
}

/** Destructor.
 @param story_ptr A reference to the object that is to be deleted. */
void Story_(struct Story **const story_ptr) {
	struct Story *story;

	if(!story_ptr || !(story = *story_ptr)) return;

	fprintf(stderr, "~Story: erase, #%p.\n", (void *)story);

	free(story);

	*story_ptr = 0;
}

/** Accessor: var.
 @param story: The object.
 @return The first var. */

/* private functions */

/** fn; only called in \see{main}.
 @param story: Story.
 @return 0
 @throws
 @implements
 @fixme
 @author
 @since
 @deprecated
 @include */
