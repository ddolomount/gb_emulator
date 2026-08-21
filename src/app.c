#include "app.h"
#include "core/gb.h"

void init_app(gb_app_t *app)
{
    app->running = true;
    display_init(&app->display);
    gb_init(&app->gameboy);
}

void app_main()
{
    gb_app_t gb_app;
    init_app(&gb_app);

    /*
     * Main loop
     */
    while (gb_app.running)
    {
        gb_step(&gb_app.gameboy);
    }
}
