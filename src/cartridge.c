#include "../include/cartridge.h"

#include <stdio.h>
#include <stdlib.h>

static uint8_t rom_only_read8(Cartridge_t *cart, uint16_t addr)
{
    if (addr <= ROM_END_ADDR)
    {
        return cart->rom[addr];
    }
    else
    {
        return 0x00;
    }
}

static void rom_only_write8(Cartridge_t *cart, uint16_t addr, uint8_t value)
{
    if (addr <= ROM_END_ADDR)
    {
        return;
    }

    /* Handle writes to RAM */
    if (addr >= 0xA000 && addr <= 0xBFFF)
    {
        if (cart->has_ram == false || cart->ram_size == RAM_SIZE_NONE)
        {
            return;
        }

        uint32_t offset = addr - 0xA000;

        if (offset < cart->ram_size)
        {
            cart->ram[offset] = value;
        }
    }
}

static Cart_type_t cartridge_type_from_header(uint8_t type)
{
    printf("Type: %u\n", type);
    switch (type)
    {
        case 0x00: // ROM
        case 0x08: // ROM + RAM
        case 0x09: // ROM + RAM + BATTERY
        {
            return CART_ROM_ONLY;
        }
        case 0x01: // MBC1
        case 0x02: // MBC1 + RAM
        case 0x03: // MBC1 + RAM + BATTERY
        {
            return CART_MBC1;
        }
        case 0x05: // MBC2
        case 0x06: // MBC2 + BATTERY
        {
            return CART_MBC2;
        }
        case 0x0B: // MMM01
        case 0x0C: // MMM01 + RAM
        case 0x0D: // MMM01 + RAM + BATTERY
        {
            return CART_MM01;
        }
        case 0x0F: // MBC3 + TIMER + BATTERY
        case 0x10: // MBC3 + TIMER + RAM + BATTERY
        case 0x11: // MBC3
        case 0x12: // MBC3 + RAM
        case 0x13: // MBC3 + RAM + BATTERY
        {
            return CART_MBC3;
        }
        case 0x19: // MBC5
        case 0x1A: // MBC5 + RAM
        case 0x1B: // MBC5 + RAM + BATTERY
        case 0x1C: // MBC5 + RUMBLE
        case 0x1D: // MBC5 + RUMBLE + RAM
        case 0x1E: // MBC5 + RUMBLE + RAM + BATTERY
        {
            return CART_MBC5;
        }
        case 0x20: // MBC6
        {
            return CART_MBC6;
        }
        case 0x22: // MBC7 + SENSOR + RUMBLE + RAM + BATTERY
        {
            return CART_MBC7;
        }
        default:
        {
            return CART_UNSUPPORTED;
        }
    }
}

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

    if (fseek(fp, 0x0147, SEEK_SET) != 0)
    {
        free(rom);
        fclose(fp);
        return false;
    }

    uint8_t cartridge_type;

    uint8_t bytes_read = fread(&cartridge_type, 1, sizeof(uint8_t), fp);

    if (bytes_read == 0)
    {
        free(rom);
        fclose(fp);
        return false;
    }

    cart->mbc_type = cartridge_type_from_header(cartridge_type);

    Ram_size_t ram_size = 0;

    fclose(fp);

    cart->rom = rom;
    cart->rom_size = rom_size;
    return true;
}

uint8_t cartridge_read8(Cartridge_t *cart, uint16_t addr)
{
    uint8_t value;

    switch (cart->mbc_type)
    {
        case CART_ROM_ONLY:
        {
            value = rom_only_read8(cart, addr);
            break;
        }
        default:
        {
            // TODO: Handle invalid MBC type
            value = 0x00;
            break;
        }
    }

    return value;
}

void cartridge_write8(Cartridge_t *cart, uint16_t addr, uint8_t value)
{
    switch (cart->mbc_type)
    {
        case CART_ROM_ONLY:
        {
            rom_only_write8(cart, addr, value);
            break;
        }
        default:
        {
            break;
        }
    }
}
