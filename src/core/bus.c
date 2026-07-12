#include "bus.h"

void bus_init(bus_t *bus, Memory_t *memory, Cartridge_t *cartridge, gb_timer_t *timer)
{
    bus->memory = memory;
    bus->cartridge = cartridge;
    bus->timer = timer;
}

uint8_t bus_read8(bus_t *bus, uint16_t addr)
{
    // Read from ROM (cartridge)
    if (addr <= ROM_END_ADDR) 
    {
        // TODO: Check to see out of bounds behaviour
        return cartridge_read8(bus->cartridge, addr);
    }

    // Read from VRAM
    if (addr >= VRAM_START_ADDR && addr <= VRAM_END_ADDR)
    {
        return bus->memory->VRAM[addr - VRAM_START_ADDR];
    }

    // Read from WRAM
    if (addr >= WRAM_START_ADDR && addr <= WRAM_END_ADDR)
    {
        return bus->memory->WRAM[addr - WRAM_START_ADDR];
    }

    // Read from OAM
    if (addr >= OAM_START_ADDR && addr <= OAM_END_ADDR)
    {
        return bus->memory->OAM[addr - OAM_START_ADDR];
    }

    if (addr >= 0xFF04 && addr <= 0xFF07)
    {
        return timer_read8(bus->timer, addr);
    }

    // Read from IO registers
    if (addr >= IO_START_ADDR && addr <= IO_END_ADDR)
    {
        return bus->memory->IO[addr - IO_START_ADDR];
    }

    // Read from HRAM
    if (addr >= HRAM_START_ADDR && addr <= HRAM_END_ADDR)
    {
        return bus->memory->HRAM[addr - HRAM_START_ADDR];
    }
    if (addr == IE_ADDR)
    {
        return bus->memory->ie;
    }

    // External cartridge RAM / MBC RAM / RTC
    if (addr >= 0xA000 && addr <= 0xBFFF)
    {
        return cartridge_read8(bus->cartridge, addr);
    }

    return 0xFF;
}

uint16_t bus_read16(bus_t *bus, uint16_t addr)
{
    uint16_t value = 0;

    // little-endian
    value |= bus_read8(bus, addr);
    addr++;
    value |= (bus_read8(bus, addr) << 8);

    return value;
}

void bus_write8(bus_t *bus, uint16_t addr, uint8_t value)
{
    if (addr <= ROM_END_ADDR)
    {
        cartridge_write8(bus->cartridge, addr, value);
        return;
    }

    // Write to VRAM
    if (addr >= VRAM_START_ADDR && addr <= VRAM_END_ADDR)
    {
        bus->memory->VRAM[addr - VRAM_START_ADDR] = value;
        return;
    }

    // Write to WRAM
    if (addr >= WRAM_START_ADDR && addr <= WRAM_END_ADDR)
    {
        bus->memory->WRAM[addr - WRAM_START_ADDR] = value;
        return;
    }

    // Write to external cartridge RAM / MBC RAM / RTC 
    if (addr >= 0xA000 && addr <= 0xBFFF)
    {
        cartridge_write8(bus->cartridge, addr, value);
        return;
    }

    // Write to OAM
    if (addr >= OAM_START_ADDR && addr <= OAM_END_ADDR)
    {
        bus->memory->OAM[addr - OAM_START_ADDR] = value;
        return;
    }

    if (addr >= 0xFF04 && addr <= 0xFF07)
    {
        timer_write8(bus->timer, addr, value);
        return;
    }

    // Write to IO registers
    if (addr >= IO_START_ADDR && addr <= IO_END_ADDR)
    {
        bus->memory->IO[addr - IO_START_ADDR] = value;
        return;
    }

    // Write to HRAM
    if (addr >= HRAM_START_ADDR && addr <= HRAM_END_ADDR)
    {
        bus->memory->HRAM[addr - HRAM_START_ADDR] = value;
        return;
    }

    if (addr == IE_ADDR)
    {
        bus->memory->ie = value;
        return;
    }
}

void bus_write16(bus_t *bus, uint16_t addr, uint16_t value)
{
    uint8_t low_byte = (uint8_t)(value & 0xFF);
    uint8_t high_byte = (uint8_t)((value >> 8) & 0xFF);

    bus_write8(bus, addr, low_byte);
    addr++;
    bus_write8(bus, addr, high_byte);
}
