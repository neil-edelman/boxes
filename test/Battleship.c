/** @title Battleship
 @author Neil
 @std C89 */

#include <stdlib.h> /* EXIT_ */
#include <stdio.h>  /* printf */
#include <time.h>	/* clock */
#include "../src/Ship.h"

/** (Incomplete) unit test of {Animals}. */
int main(void) {
	unsigned seed = (unsigned)clock();
	struct Ships *ships = 0;
	int is_success = 0;

	srand(seed), rand(), printf("Seed %u.\n", seed);

	do {
		const unsigned ship_no = 10000;
		unsigned i;
		if(!(ships = Ships(10, 10))) break;
		for(i = 0; i < ship_no; i++) {
			float r = (float)(rand() / ((double)RAND_MAX + 1));
			if(r < 0.5f) {
				if(!Destroyer(ships)) break;
			} else {
				if(!Cruiser(ships)) break;
			}
		}
		if(i != ship_no) break;
		ShipsOut(ships);
		ShipsClear(ships);
		/*printf("--\n"), ShipsOut(ships);*/
		Ships_(&ships);
		is_success = 1;
	} while(0); if(!is_success) {
		perror("Ships");
	} {
		Ships_(&ships);
	}

	return is_success ? EXIT_SUCCESS : EXIT_FAILURE;
}
