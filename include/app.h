#include "frontend/display.h"
#include "core/gb.h"

typedef struct
{
    gb_t gameboy;
    gb_display_t display;

    bool running;
} gb_app_t;
