#include "pico/stdlib.h"
#include "pico/sem.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "ws2812.pio.h"

#include "graphics.h"

//Hardware PIO instances
uint DMA;
static struct semaphore reset_delay_complete_sem;
alarm_id_t reset_delay_alarm_id;

// PIO instance
PIO pio;
uint sm;
uint offset;

// Packed in rgb of 8 bits each
typedef uint32_t Color;
Color buffer[NUM_PIXELS];

int64_t reset_delay_complete(__unused alarm_id_t id, __unused void *user_data) {
    reset_delay_alarm_id = 0;
    sem_release(&reset_delay_complete_sem);
    // no repeat
    return 0;
}

void __isr dma_complete_handler() {
    if (dma_hw->ints0 & (1u << DMA)) {
        // clear IRQ
        dma_hw->ints0 = (1u << DMA);
        // when the dma is complete we start the reset delay timer
        if (reset_delay_alarm_id) cancel_alarm(reset_delay_alarm_id);
        reset_delay_alarm_id = add_alarm_in_ms(10, reset_delay_complete, NULL, true);
    }
}

void init_dma(PIO pio, uint sm) {
    DMA = dma_claim_unused_channel(true);
    dma_channel_config c = dma_channel_get_default_config(DMA);
    channel_config_set_dreq(&c, pio_get_dreq(pio, sm, true));
    dma_channel_configure(DMA, 
                          &c,
                          &pio->txf[sm],
                          buffer, // I think this is set by dma_channel_hw_addr(DMA_CB_CHANNEL)->al3_read_addr_trig = (uintptr_t) fragment_start; 
                          NUM_PIXELS, // Since we are expecting words
                          false);

    irq_set_exclusive_handler(DMA_IRQ_0, dma_complete_handler);
    dma_channel_set_irq0_enabled(DMA, true);
    irq_set_enabled(DMA_IRQ_0, true);
}

void init_graphics() {
    // This will find a free pio and state machine for our program and load it for us
    // We use pio_claim_free_sm_and_add_program_for_gpio_range (for_gpio_range variant)
    // so we will get a PIO instance suitable for addressing gpios >= 32 if needed and supported by the hardware
    bool success = pio_claim_free_sm_and_add_program_for_gpio_range(&ws2812_program, &pio, &sm, &offset, WS2812_PIN, 1, true);
    hard_assert(success);

    ws2812_program_init(pio, sm, offset, WS2812_PIN, 833333);
    sem_init(&reset_delay_complete_sem, 1, 1);
    init_dma(pio, sm);
}

void release_graphics() {
    dma_channel_unclaim(DMA);

    // This will free resources and unload our program
    pio_remove_program_and_unclaim_sm(&ws2812_program, pio, sm, offset);
}

void put_pixel(uint16_t index, uint8_t r, uint8_t g, uint8_t b) {
    buffer[index] = ((uint32_t) (r * BRIGHTNESS) << 16) |
                    ((uint32_t) (g * BRIGHTNESS) << 24) |
                    ((uint32_t) (b * BRIGHTNESS) << 8) ;
}

void put_pixel_xy(uint8_t x, uint8_t y, const uint8_t rgb[3]) {
    uint16_t index = y * 10 + x; // 10 is the width of the grid
    put_pixel(index, rgb[0], rgb[1], rgb[2]);
}

void write_buffer() {
    dma_channel_set_read_addr(DMA, buffer, true); // Reset the address because the DMA keeps counting up from the last address
    sem_acquire_blocking(&reset_delay_complete_sem); // I could just check to see if the dma is busy but I think this is more power efficientz
}

