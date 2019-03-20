/** @title Animal
 @author Neil
 @std C89 */

#include <stdlib.h> /* EXIT_ */
#include <stdio.h>  /* printf */
#include <time.h>	/* clock */
#include "../src/Animals.h"

/** (Incomplete) unit test of {Animals}. */
int main(void) {
	unsigned seed = (unsigned)clock();
	struct Animals *animals = 0;
	int is_success = 0;
	clock_t t;

	srand(seed), rand(), printf("Seed %u.\n", seed);

	t = clock();
	do {
		struct Animal *a = 0, *prev_a = 0;
		struct Bear *w, *n;
		const unsigned animal_no = 1000/*000*/;
		unsigned i;
		if(!(animals = Animals())) break;
		for(i = 0; i < animal_no; i++) {
			float r = (float)(rand() / ((double)RAND_MAX + 1));
			if(r < 0.25f) {
				if(!Sloth(animals)) break;
			} else if(r < 0.45f) {
				if(!Emu(animals)) break;
			} else if(r < 0.55) {
				if(!BadEmu(animals)) break;
			} else if(r < 0.8) {
				if(!Llama(animals)) break;
			} else {
				if(!Lemur(animals)) break;
			}
		}
		if(i != animal_no) break;
		w = Bear(animals, 0, "Winnie");
		n = Bear(animals, 1, "Napoloen");
		for(a = AnimalsFirst(animals); a; a = AnimalsNext(a)) {
			if(prev_a && !AnimalsRide(animals, prev_a, a))
				AnimalsRide(animals, a, prev_a);
			prev_a = a;
		}
		AnimalsAct(animals);
		AnimalsClear(animals);
		Animals_(&animals);
		is_success = 1;
	} while(0); if(!is_success) {
		perror("Animals");
	} {
		Animals_(&animals);
	}
	fprintf(stderr, "Time: %lu\n", (unsigned long)(clock() - t));

	return is_success ? EXIT_SUCCESS : EXIT_FAILURE;
}
