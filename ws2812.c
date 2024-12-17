/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"

#define NUM_PIXELS 200

#define WS2812_PIN 1
#define BRIGHTNESS 0.01

typedef uint32_t Color;

static inline void put_pixel(PIO pio, uint sm, Color pixel_grb) {
    pio_sm_put_blocking(pio, sm, pixel_grb << 8u);
}

static inline Color urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return  ((uint32_t) (r * BRIGHTNESS) << 8) |
            ((uint32_t) (g * BRIGHTNESS) << 16) |
            (uint32_t) (b * BRIGHTNESS);
}

int main() {
    //set_sys_clock_48();
    stdio_init_all();
    printf("WS2812 Smoke Test, using pin %d\n", WS2812_PIN);

    PIO pio;
    uint sm;
    uint offset;

    // This will find a free pio and state machine for our program and load it for us
    // We use pio_claim_free_sm_and_add_program_for_gpio_range (for_gpio_range variant)
    // so we will get a PIO instance suitable for addressing gpios >= 32 if needed and supported by the hardware
    bool success = pio_claim_free_sm_and_add_program_for_gpio_range(&ws2812_program, &pio, &sm, &offset, WS2812_PIN, 1, true);
    hard_assert(success);

    ws2812_program_init(pio, sm, offset, WS2812_PIN, 833333);

    int t = 0;
    while (1) {
        // for (int i = 0; i < 1000; ++i) {
        //     puts("test");
        //     pattern_snakes(pio, sm, NUM_PIXELS, t);
        //     sleep_ms(10);
        //     t += 1;
        // }

        for (int i = 0; i < NUM_PIXELS; i++)
        {
            put_pixel(pio, sm, urgb_u32(0xff, 0, 0));
        }
        sleep_ms(10);
    }

    // This will free resources and unload our program
    pio_remove_program_and_unclaim_sm(&ws2812_program, pio, sm, offset);
}
