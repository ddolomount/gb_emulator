#include <stdint.h>
#include <stdio.h>
#include "../include/cartridge.h"

int main()
{
    const char *env_value = getenv("ROM");
    
    Cartridge_t cart;

    bool res = cartridge_load(&cart, env_value);

    if (!res)
    {
        printf("cartridge_load failed\n");
    }

    return 0;
}
