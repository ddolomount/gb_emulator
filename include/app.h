#include "core/gb.h"
#include "frontend/display.h"

typedef struct
{
    gb_t gameboy;
    gb_display_t display;

    bool running;
} gb_app_t;
