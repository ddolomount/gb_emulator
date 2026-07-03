#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define ROM_START_ADDR 0x0000
#define ROM_END_ADDR   0x7FFF

#define BANK0_START_ADDR 0x0000
#define BANK0_END_ADDR   0x3FFF

typedef enum {
    CART_ROM_ONLY,
    CART_MBC1,
    CART_MBC2,
    CART_MBC3,
    CART_MBC5,
    CART_MBC6,
    CART_MBC7,
    CART_MM01,
    CART_UNSUPPORTED
} Cart_type_t;

typedef enum {
    RAM_SIZE_NONE,
    RAM_SIZE_UNUSED,
    RAM_SIZE_8KB,
    RAM_SIZE_32KB,
    RAM_SIZE_128KB,
    RAM_SIZE_64KB,
} Ram_size_t;

typedef struct Cartridge_t {
    /* ROM and RAM */
    uint8_t *rom;
    uint8_t *ram;
    size_t rom_size;
    Ram_size_t ram_size;
    
    /* Information about ROM */
    Cart_type_t mbc_type;
    bool has_ram;
    bool has_battery; // TODO: Figure out if this matters in ROM

} Cartridge_t;

bool cartridge_load(Cartridge_t *cart, const char *path);

void cartridge_write8(Cartridge_t *cartridge, uint16_t addr, uint8_t value);
void cartridge_write16(Cartridge_t *cartridge, uint16_t addr, uint16_t value);

uint8_t cartridge_read8(Cartridge_t *cartridge, uint16_t addr);
uint16_t cartridge_read16(Cartridge_t *cartridge, uint16_t addr);

#endif /* CARTRIDGE_H */
