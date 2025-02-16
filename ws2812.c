#include <stdio.h>
#include "pico/stdlib.h"

#include "graphics.h"
#include "game.h"

void pattern_snakes(uint len, uint t) {
    for (uint i = 0; i < len; ++i) {
        uint x = (i + (t >> 1)) % 64;
        if (x < 10)
            put_pixel(i, 0xff, 0, 0);
        else if (x >= 15 && x < 25)
            put_pixel(i, 0, 0xff, 0);
        else if (x >= 30 && x < 40)
            put_pixel(i, 0, 0, 0xff);
        else
            put_pixel(i, 0, 0, 0);
    }
}

void solid(uint8_t r, uint8_t g, uint8_t b) {
    for (uint i = 0; i < NUM_PIXELS; ++i) {
        put_pixel(i, r, g, b);
    }

    write_buffer();
}

int main() {
    //set_sys_clock_48();
    stdio_init_all();
    stdio_getchar();

    init_graphics();
    init_game();

    game_loop();

    while (true)
    {
        // put_pixel_xy(0, 0, (uint8_t[3]){0xff, 0xFF, 0xFF});
        // write_buffer();
        // solid(0xff, 0, 0);
    }
    release_graphics();

    while (true);
}
