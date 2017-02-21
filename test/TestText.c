/** Copyright 2015 Neil Edelman, distributed under the terms of the MIT License;
 see readme.txt, or \url{ https://opensource.org/licenses/MIT }.

 This is a String malloc; 

 @author	Neil
 @version	3.0; 2016-08
 @since		3.0; 2016-08 */

#include <stdlib.h>	/* rand, EXIT_* */
#include <stdio.h>  /* fprintf */
#include <string.h>	/* strcmp */
#include "../src/Text.h"

int main(int argc, char *argv[]) {
	enum { E_NO_ERR, E_ERRNO, E_UNEXPECTED } error = E_NO_ERR;
	struct Text *text = 0;
	char *fn;

	if(argc != 2) {
		fprintf(stderr, "Needs <filename>.\n");
		return EXIT_FAILURE;
	} else {
		fn = argv[1];
	}

	do {

		if((text = Text("!@#$%%^&*()`\n"))) { error = E_UNEXPECTED; break; }
		printf("Text(\"!@#$%%^&*()`\\n\"): %s.\n", TextGetError(text));
		if(!(text = Text(fn))) { error = E_UNEXPECTED; break; }

	} while(0);
	switch(error) {
		case E_NO_ERR: break;
		case E_ERRNO: perror(fn); break;
		case E_UNEXPECTED:
			fprintf(stderr, "%s: %s.\n", fn, TextGetError(text)); break;
	}
	{
		printf("Destroying Text: \"%s,\"\n", fn);
		Text_(&text);
	}

	fprintf(stderr, "Done all tests; %s.\n", error ? "FAILED" : "SUCCEDED");

	return error ? EXIT_FAILURE : EXIT_SUCCESS;
}
