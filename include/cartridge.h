#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct {

} Cartridge_t;

bool cartridge_load(Cartridge_t *cart, const char *path);

#endif /* CARTRIDGE_H */
