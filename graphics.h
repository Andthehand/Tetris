#ifndef _GRAPHICS_H
#define _GRAPHICS_H
#include "pico/stdlib.h"

#define WS2812_PIN 1
#define NUM_PIXELS 200
#define BRIGHTNESS 0.05

void init_graphics();
void release_graphics();

void put_pixel(uint16_t index, uint8_t r, uint8_t g, uint8_t b);
void put_pixel_xy(uint8_t x, uint8_t y, const uint8_t rgb[3]);
void write_buffer();

#endif