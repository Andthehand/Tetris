#include <stdio.h>
#include "pico/stdlib.h"

#include "graphics.h"
#include "game.h"

int main() {
    stdio_init_all();

    init_graphics();
    init_game();    

    game_loop();

    release_graphics();

    while (true);
}
