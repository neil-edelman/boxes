/** Unit test of Animal.c.

 @file		Animals
 @author	Neil
 @std		C89/90
 @version	1.0; 20xx-xx
 @since		1.0; 20xx-xx
 @param
 @fixme
 @deprecated */

#include <stdlib.h> /* EXIT_ */
#include <stdio.h>  /* printf */
#include <time.h>	/* clock */
#include "../src/Animal.h"

/** Entry point.
 @param argc: The number of arguments, starting with the programme name.
 @param argv: The arguments.
 @return Either EXIT_SUCCESS or EXIT_FAILURE. */
int main(void) {
	unsigned seed = (unsigned)clock();
	struct Animals *a = 0;
	int is_success = 0;

	srand(seed), rand(), printf("Seed %u.\n", seed);

	do {
		const unsigned animal_no = 100;
		unsigned i;
		if(!(a = Animals())) break;
		for(i = 0; i < animal_no; i++) {
			float r = (float)(rand() / ((double)RAND_MAX + 1));
			printf("%f\n", r);
			if(r < 0.25f) {
				printf("Sloth\n");
				if(!Sloth(a)) break;
			} else if(r < 0.45f) {
				printf("Emu\n");
				if(!Emu(a)) break;
			} else if(r < 0.55) {
				printf("BadEmu\n");
				if(!BadEmu(a)) break;
			} else if(r < 0.8) {
				printf("Llama\n");
				if(!Llama(a)) break;
				printf("go\n");
			} else {
				printf("Lemur\n");
				if(!Lemur(a)) break;
				printf("go\n");
			}
		}
		if(i != animal_no) break;
		/* fixme: bear. */
		AnimalsAct(a);
		AnimalsClear(a);
		Animals_(&a);
		is_success = 1;
	} while(0); if(!is_success) {
		perror("Animals");
	} {
		Animals_(&a);
	}

	return is_success ? EXIT_SUCCESS : EXIT_FAILURE;
}
