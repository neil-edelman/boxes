struct Bear;
struct Animal;
struct Animals;

void Animals_(struct Animals **const animalsp);
struct Animals *Animals(void);
struct Sloth *Sloth(struct Animals *const animals);
struct Emu *Emu(struct Animals *const animals);
struct BadEmu *BadEmu(struct Animals *const animals);
struct Llama *Llama(struct Animals *const animals);
struct Lemur *Lemur(struct Animals *const animals);
struct Bear *Bear(struct Animals *const animals, const unsigned no,
	const char *const name);
int AnimalsRide(struct Animals *const animals, struct Animal *const a,
	struct Animal *const b);
void AnimalsAct(struct Animals *const animals);
void AnimalsClear(struct Animals *const animals);
