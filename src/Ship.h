struct Ship;
struct Ships;

void Ships_(struct Ships **const pship);
struct Ships *Ships(const unsigned x, const unsigned y);
struct Destroyer *Destroyer(struct Ships *const ships);
struct Cruiser *Cruiser(struct Ships *const ships);
void ShipsClear(struct Ships *const ships);
void ShipsOut(const struct Ships *const ships);
