#include "cartridge.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RAM_ENABLE   (addr <= 0x1FFF)
#define ROM_BANK     (addr >= 0x2000 && addr <= 0x3FFF)
#define RAM_BANK     (addr >= 0x4000 && addr <= 0x5FFF)
#define BANKING_MODE (addr >= 0x6000 && addr <= 0x7FFF)
#define EXT_RAM      (addr >= 0xA000 && addr <= 0xBFFF)

#define ROM_BANK_SIZE 0x4000 // 16 KiB
#define RAM_BANK_SIZE 0x2000 // 8 KiB

static uint8_t cart_rom_read(Cartridge_t *cart, uint16_t bank, uint16_t offset)
{
    if (cart->rom == NULL || cart->rom_size == 0)
    {
        return 0xFF;
    }

    uint32_t rom_bank_count = cart->rom_size / ROM_BANK_SIZE;

    if (rom_bank_count == 0)
    {
        return 0xFF;
    }

    bank %= rom_bank_count;

    uint32_t rom_addr = ((uint32_t)bank * ROM_BANK_SIZE) + offset;


    if (rom_addr >= cart->rom_size)
    {
        return 0xFF;
    }

    return cart->rom[rom_addr];
}

static uint8_t cart_ram_read(Cartridge_t *cart, uint16_t bank, uint16_t offset)
{
    if (!cart->ram_enabled || cart->ram == NULL || cart->ram_size == 0)
    {
        return 0xFF;
    }

    uint32_t ram_bank_count = cart->ram_size / RAM_BANK_SIZE;

    if (ram_bank_count == 0)
    {
        uint32_t ram_addr = offset % cart->ram_size;
        return cart->ram[ram_addr];
    }

    bank %= ram_bank_count;

    uint32_t ram_addr = ((uint32_t)bank * RAM_BANK_SIZE) + offset;

    if (ram_addr >= cart->ram_size)
    {
        ram_addr %= cart->ram_size;
    }

    return cart->ram[ram_addr];
}

static void cart_ram_write(Cartridge_t *cart, uint16_t bank, uint16_t offset, uint8_t value)
{
    if (!cart->ram_enabled || cart->ram == NULL || cart->ram_size == 0)
    {
        return;
    }

    uint32_t ram_bank_count = cart->ram_size / RAM_BANK_SIZE;

    if (ram_bank_count == 0)
    {
        uint32_t ram_addr = offset % cart->ram_size;
        cart->ram[ram_addr] = value;
        return;
    }

    bank %= ram_bank_count;

    uint32_t ram_addr = ((uint32_t)bank * RAM_BANK_SIZE) + offset;
    

    if (ram_addr >= cart->ram_size)
    {
        ram_addr %= cart->ram_size;
    }

    cart->ram[ram_addr] = value;
}

static uint16_t mbc1_get_rom_bank0(Cartridge_t *cart)
{
    if (cart->banking_mode == 0)
    {
        return 0;
    }

    return cart->mbc1_bank_high2 << 5;
}

static uint16_t mbc1_get_rom_bankx(Cartridge_t *cart)
{
    uint16_t bank = ((uint16_t)cart->mbc1_bank_high2 << 5) | cart->mbc1_bank_low5;

    if ((bank & 0x1F) == 0)
    {
        bank++;
    }

    return bank;
}

static uint16_t mbc1_get_ram_bank(Cartridge_t *cart)
{
    if (cart->banking_mode == 0)
    {
        return 0;
    }

    return cart->mbc1_bank_high2;
}

static uint8_t rom_only_read8(Cartridge_t *cart, const uint16_t addr)
{
    if (addr <= ROM_END_ADDR)
    {
        return cart_rom_read(cart, 0, addr);
    }
    
    if (addr >= 0xA000 && addr <= 0xBFFF)
    {
        return cart_ram_read(cart, 0, addr - 0xA000);
    }

    return 0xFF;
}

static uint8_t mbc1_read8(Cartridge_t *cart, const uint16_t addr)
{
    if (addr <= BANK0_END_ADDR)
    {
        uint16_t bank = mbc1_get_rom_bank0(cart);
        return cart_rom_read(cart, bank, addr);
    }

    if (addr >= 0x4000 && addr <= 0x7FFF)
    {
        uint16_t bank = mbc1_get_rom_bankx(cart);
        return cart_rom_read(cart, bank, addr - 0x4000);
    }

    if (addr >= 0xA000 && addr <= 0xBFFF)
    {
        uint8_t bank = mbc1_get_ram_bank(cart);
        return cart_ram_read(cart, bank, addr - 0xA000);
    }

    return 0xFF;
}

static void rom_only_write8(Cartridge_t *cart, const uint16_t addr, const uint8_t value)
{
    if (addr <= ROM_END_ADDR)
    {
        return;
    }

    /* Handle writes to RAM */
    if (addr >= 0xA000 && addr <= 0xBFFF)
    {
        if (cart->has_ram == false || cart->ram == NULL || cart->ram_size == 0)
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

static void mbc1_write8(Cartridge_t *cart, const uint16_t addr, const uint8_t value)
{
    // Enable external RAM
    if (RAM_ENABLE)
    {
        cart->ram_enabled = ((value & 0x0F) == 0x0A);
        return;
    }

    // ROM bank number (5 bits)
    if (ROM_BANK)
    {
        uint8_t bank = value & 0x1F;
        
        if (bank == 0)
        {
            bank = 1;
        }

        cart->mbc1_bank_low5 = bank;
        return;
    }

    // RAM bank number (2 bits)
    if (RAM_BANK)
    {
        cart->mbc1_bank_high2 = value & 0x03;
        return;
    }

    // Banking mode select
    if (BANKING_MODE)
    {
        cart->banking_mode = value & 0x01;
        return;
    }

    // External RAM write
    if (EXT_RAM)
    {
        uint8_t selected_ram_bank = mbc1_get_ram_bank(cart);
        cart_ram_write(cart, selected_ram_bank, addr - 0xA000, value);
        return;
    }
}

static Cart_type_t cartridge_type_from_header(const uint8_t type)
{
    // printf("Type: %u\n", type);
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

static size_t cartridge_rom_size_from_header(uint8_t code)
{
    if (code <= 0x08)
    {
        return 32 * 1024 * (1 << code);
    }
    return 0;
}

static size_t cartridge_ram_size_from_header(uint8_t code)
{
    switch (code)
    {   
        case 0x00: return 0;
        case 0x01: return 2 * 1024;
        case 0x02: return 8 * 1024;
        case 0x03: return 32 * 1024;
        case 0x04: return 128 * 1024;
        case 0x05: return 64 * 1024;
        default: return 0;
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

    /* Set pointer to end of file to get size */
    if (fseek(fp, 0, SEEK_END) != 0)
    {
        fclose(fp);
        return false;
    }

    /* Check if file is empty */
    long end_pos = ftell(fp);
    if (end_pos < 0)
    {
        fclose(fp);
        return false;
    }

    rom_size = (size_t)end_pos;

    /* Set pointer back to beginning of file */
    if (fseek(fp, 0, SEEK_SET) != 0)
    {
        fclose(fp);
        return false;
    }

    /* Allocate memory for ROM */
    rom = malloc(rom_size);
    if (!rom)
    {
        fclose(fp);
        return false;
    }

    /* Read file into ROM */
    if (fread(rom, 1, rom_size, fp) != rom_size)
    {
        free(rom);
        fclose(fp);
        return false;
    }

    memset(cart, 0, sizeof(*cart));

    uint8_t cartridge_type = rom[0x0147];
    uint8_t rom_size_code = rom[0x0148];
    uint8_t ram_size_code = rom[0x0149];

    // printf("ROM Code: %u, RAM Code: %u\n", rom_size_code, ram_size_code);

    // printf("ROM Size: %lu bytes\n", rom_size);

    cart->mbc_type = cartridge_type_from_header(cartridge_type);
    cart->ram_size = cartridge_ram_size_from_header(ram_size_code);
    cart->rom_size = cartridge_rom_size_from_header(rom_size_code);
    cart->has_ram = cart->ram_size > 0;

    if (cart->rom_size != rom_size)
    {
        free(rom);
        fclose(fp);
        return false;
    }

    if (cart->has_ram)
    {
        cart->ram = calloc(cart->ram_size, 1);

        if (!cart->ram)
        {
            free(rom);
            fclose(fp);
            return false;
        }
    }

    cart->ram_enabled = false;
    cart->banking_mode = 0;
    cart->rom = rom;
    cart->rom_size = rom_size;
  
    if (!cart->ram && cart->ram_size != 0)
    {
        free(rom);
        fclose(fp);
        return false;
    }

    // printf("RAM Size: %lu bytes\n", cart->ram_size);

    fclose(fp);
    return true;
}

uint8_t cartridge_read8(Cartridge_t *cart, const uint16_t addr)
{
    uint8_t value;

    switch (cart->mbc_type)
    {
        case CART_ROM_ONLY:
        {
            value = rom_only_read8(cart, addr);
            break;
        }
        case CART_MBC1:
        {
            value = mbc1_read8(cart, addr);
            break;
        }
        default:
        {
            // TODO: Handle invalid MBC type
            value = 0xFF;
            break;
        }
    }

    return value;
}

void cartridge_write8(Cartridge_t *cart, const uint16_t addr, const uint8_t value)
{
    switch (cart->mbc_type)
    {
        case CART_ROM_ONLY:
        {
            rom_only_write8(cart, addr, value);
            break;
        }
        case CART_MBC1:
        {
            mbc1_write8(cart, addr, value);
            break;
        }
        default:
        {
            break;
        }
    }
}
