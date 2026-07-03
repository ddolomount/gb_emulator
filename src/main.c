#include <stdint.h>
#include <stdio.h>
#include "../include/cartridge.h"

int main(int argc, char *argv[])
{
    const char *env_value = getenv("ROM");
    
    Cartridge_t cart;

    bool res = cartridge_load(&cart, env_value);

    printf("Cart Type: %u\n", cart.mbc_type);
    printf("Ram size: %u\n", cart.ram_size);

    return 0;
}
