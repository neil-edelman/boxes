/** Unit test of Animals.c.

 @file		Animal
 @author	Neil
 @std		C89/90 */

#include <stdlib.h> /* EXIT_ */
#include <stdio.h>  /* printf */
#include <time.h>	/* clock */
#include "../src/Animals.h"

/*#define POOL_NAME AnimalRef
#define POOL_TYPE const struct Animal *
#include "../src/Pool.h" <- This is useless: memory move. Maybe we should make
 it useful? */

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
		struct Animal *animal = 0;
		struct Bear *w, *n;
		const unsigned animal_no = 100;
		unsigned i;
		if(!(a = Animals())) break;
		for(i = 0; i < animal_no; i++) {
			float r = (float)(rand() / ((double)RAND_MAX + 1));
			if(r < 0.25f) {
				animal = (struct Animal *)Sloth(a);
			} else if(r < 0.45f) {
				animal = (struct Animal *)Emu(a);
			} else if(r < 0.55) {
				animal = (struct Animal *)BadEmu(a);
			} else if(r < 0.8) {
				animal = (struct Animal *)Llama(a);
			} else {
				animal = (struct Animal *)Lemur(a);
			}
			if(!animal) break;
			/*AnimalRide(a, );*/
		}
		if(i != animal_no) break;
		w = Bear(a, 0, "Winnie");
		n = Bear(a, 1, "Napoloen");
		AnimalsAct(a);
		for(i = 100; i; i--) {
		}
		AnimalsRide(a, (struct Animal *)n, (struct Animal *)w);
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
