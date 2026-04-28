#include "../include/cartridge.h"

#include <stdio.h>
#include <stdlib.h>

bool cartridge_load(Cartridge_t *cart, const char *path)
{
    FILE *fp = fopen(path, "rb");
    size_t rom_size;
    uint8_t *rom;

    if (!fp)
    {
        return false;
    }

    if (fseek(fp, 0, SEEK_END) != 0)
    {
        fclose(fp);
        return false;
    }

    long end_pos = ftell(fp);
    if (end_pos < 0)
    {
        fclose(fp);
        return false;
    }

    rom_size = (size_t)end_pos;

    if (fseek(fp, 0, SEEK_SET) != 0)
    {
        fclose(fp);
        return false;
    }

    rom = malloc(rom_size);
    if (!rom)
    {
        fclose(fp);
        return false;
    }

    if (fread(rom, 1, rom_size, fp) != rom_size)
    {
        free(rom);
        fclose(fp);
        return false;
    }

    fclose(fp);

    cart->rom = rom;
    cart->rom_size = rom_size;
    return true;
}
