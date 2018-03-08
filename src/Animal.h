struct Animals;

void Animals_(struct Animals **const animalsp);
struct Animals *Animals(void);
struct Sloth *Sloth(struct Animals *const animals);
struct Emu *Emu(struct Animals *const animals);
void AnimalsTransmogrify(struct Animals *const animals);
void AnimalsClear(struct Animals *const animals);
