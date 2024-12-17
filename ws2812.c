/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "ws2812.pio.h"

#define NUM_PIXELS 200

#define WS2812_PIN 1
#define BRIGHTNESS 0.01

// Packed in rgb of 8 bits each
typedef uint32_t Color;
Color buffer[NUM_PIXELS];

static inline void put_pixel(uint index, uint8_t r, uint8_t g, uint8_t b) {
    buffer[index] = ((uint32_t) (r * BRIGHTNESS) << 16) |
                    ((uint32_t) (g * BRIGHTNESS) << 24) |
                    ((uint32_t) (b * BRIGHTNESS) << 8) ;
}

uint init_dma(PIO pio, uint sm) {
    uint dma_chan = dma_claim_unused_channel(true);
    dma_channel_config c = dma_channel_get_default_config(dma_chan);
    channel_config_set_dreq(&c, pio_get_dreq(pio, sm, true));
    dma_channel_configure(dma_chan, 
                          &c,
                          &pio->txf[sm],
                          buffer, // I think this is set by dma_channel_hw_addr(DMA_CB_CHANNEL)->al3_read_addr_trig = (uintptr_t) fragment_start; 
                          NUM_PIXELS, // Since we are expecting words
                          false);

    dma_channel_set_irq0_enabled(dma_chan, true);

    return dma_chan;
}

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

int main() {
    //set_sys_clock_48();
    stdio_init_all();
    printf("WS2812 Smoke Test, using pin %d\n", WS2812_PIN);

    PIO pio;
    uint sm;
    uint offset;

    uint DMA;

    // This will find a free pio and state machine for our program and load it for us
    // We use pio_claim_free_sm_and_add_program_for_gpio_range (for_gpio_range variant)
    // so we will get a PIO instance suitable for addressing gpios >= 32 if needed and supported by the hardware
    bool success = pio_claim_free_sm_and_add_program_for_gpio_range(&ws2812_program, &pio, &sm, &offset, WS2812_PIN, 1, true);
    hard_assert(success);

    ws2812_program_init(pio, sm, offset, WS2812_PIN, 833333);
    DMA = init_dma(pio, sm);

    while (true)
    {
        int t = 0;
        for (int i = 0; i < 1000; ++i) {
            puts("test");
            pattern_snakes(NUM_PIXELS, t);
            dma_channel_set_read_addr(DMA, buffer, true);
            t += 1;

            while (!(dma_hw->intr & 1u << DMA))
                tight_loop_contents();
            sleep_ms(10);

            
        }
    }

    while (true);

    // This will free resources and unload our program
    pio_remove_program_and_unclaim_sm(&ws2812_program, pio, sm, offset);
}
