#include <stddef.h>

extern const char *const parole[];
extern const size_t parole_size;

const char *const parole[] = {
#include "parole_inglesi.h"
};
const size_t parole_size = sizeof parole / sizeof *parole;
