#ifndef SOURCE_DISPLAY_GFX_H_
#define SOURCE_DISPLAY_GFX_H_

/**
 * Adafruit_GFX-like helpers on top of u8g2 for porting Teensy menu code.
 *
 * Text sizes map to u8g2 fonts (Adafruit-like where noted):
 *   0, 1 -> 5x8 (5x7 glyphs in 8px row; closest stock match to Adafruit 6x8)
 *   2 -> 9x15
 *   6 -> logisoso32 (large digits; tune as needed)
 */

#include <stdbool.h>
#include <stdint.h>

#include "u8g2.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    u8g2_t *u8g2;
    int16_t cursor_x;
    int16_t cursor_y;
    int16_t text_origin_x;
    int16_t text_origin_y;
    uint8_t text_size;
} display_gfx_t;

void display_gfx_bind(display_gfx_t *gfx, u8g2_t *u8g2);
void display_gfx_clear(display_gfx_t *gfx);
void display_gfx_set_text_size(display_gfx_t *gfx, uint8_t size);
void display_gfx_set_cursor(display_gfx_t *gfx, int16_t x, int16_t y);
void display_gfx_print(display_gfx_t *gfx, const char *text);
void display_gfx_println(display_gfx_t *gfx, const char *text);
void display_gfx_draw_line(display_gfx_t *gfx, int16_t x0, int16_t y0, int16_t x1, int16_t y1);
void display_gfx_draw_rect(display_gfx_t *gfx, int16_t x, int16_t y, int16_t w, int16_t h);
void display_gfx_fill_rect(display_gfx_t *gfx, int16_t x, int16_t y, int16_t w, int16_t h);
void display_gfx_draw_triangle(display_gfx_t *gfx, int16_t x0, int16_t y0, int16_t x1,
                               int16_t y1, int16_t x2, int16_t y2);
void display_gfx_draw_bitmap(display_gfx_t *gfx, int16_t x, int16_t y, int16_t w, int16_t h,
                            const uint8_t *xbm_bits);
void display_gfx_flush(display_gfx_t *gfx);

#ifdef __cplusplus
}
#endif

#endif /* SOURCE_DISPLAY_GFX_H_ */
