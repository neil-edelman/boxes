struct Bear;
struct Animals;

void Animals_(struct Animals **const animalsp);
struct Animals *Animals(void);
struct Sloth *Sloth(struct Animals *const animals);
struct Emu *Emu(struct Animals *const animals);
struct BadEmu *BadEmu(struct Animals *const animals);
struct Llama *Llama(struct Animals *const animals);
struct Lemur *Lemur(struct Animals *const animals);
void Bear(struct Animals *const animals, struct Bear *const bear);
void AnimalsAct(struct Animals *const animals);
void AnimalsClear(struct Animals *const animals);
