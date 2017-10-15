struct Animals;

void Animals_(struct Animals **const animalsp);
struct Animals *Animals(void);
struct Sloth *AnimalsSloth(struct Animals *const animals);
struct Emu *AnimalsEmu(struct Animals *const animals);
void AnimalsTransmogrify(struct Animals *const animals);
void AnimalsClear(struct Animals *const animals);
