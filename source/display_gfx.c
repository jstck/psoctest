#include "display_gfx.h"

#include <stdio.h>
#include <string.h>

#define GFX_COLOR_WHITE 1

static const uint8_t *font_for_size(uint8_t size)
{
    switch (size)
    {
        case 0:
        case 1:
        default:
            return u8g2_font_5x8_tf;
        case 2:
            return u8g2_font_9x15_tf;
        case 6:
            return u8g2_font_logisoso32_tf;
    }
}

static void apply_font(display_gfx_t *gfx)
{
    u8g2_SetFont(gfx->u8g2, font_for_size(gfx->text_size));
    u8g2_SetFontPosTop(gfx->u8g2);
    u8g2_SetDrawColor(gfx->u8g2, GFX_COLOR_WHITE);
}

void display_gfx_bind(display_gfx_t *gfx, u8g2_t *u8g2)
{
    gfx->u8g2 = u8g2;
    gfx->cursor_x = 0;
    gfx->cursor_y = 0;
    gfx->text_origin_x = 0;
    gfx->text_origin_y = 0;
    gfx->text_size = 1;
    apply_font(gfx);
}

void display_gfx_clear(display_gfx_t *gfx)
{
    u8g2_ClearBuffer(gfx->u8g2);
}

void display_gfx_set_text_size(display_gfx_t *gfx, uint8_t size)
{
    gfx->text_size = size;
    apply_font(gfx);
}

void display_gfx_set_cursor(display_gfx_t *gfx, int16_t x, int16_t y)
{
    gfx->text_origin_x = x;
    gfx->text_origin_y = y;
    gfx->cursor_x = x;
    gfx->cursor_y = y;
}

void display_gfx_print(display_gfx_t *gfx, const char *text)
{
    if ((NULL == gfx) || (NULL == gfx->u8g2) || (NULL == text))
    {
        return;
    }

    gfx->cursor_x = (int16_t)u8g2_DrawStr(gfx->u8g2, gfx->cursor_x, gfx->cursor_y, text);
}

void display_gfx_println(display_gfx_t *gfx, const char *text)
{
    if ((NULL == gfx) || (NULL == gfx->u8g2))
    {
        return;
    }

    if (NULL != text)
    {
        display_gfx_print(gfx, text);
    }

    gfx->cursor_x = gfx->text_origin_x;
    gfx->cursor_y = (int16_t)(gfx->cursor_y + u8g2_GetMaxCharHeight(gfx->u8g2) + 1);
}

void display_gfx_draw_line(display_gfx_t *gfx, int16_t x0, int16_t y0, int16_t x1, int16_t y1)
{
    u8g2_SetDrawColor(gfx->u8g2, GFX_COLOR_WHITE);
    u8g2_DrawLine(gfx->u8g2, x0, y0, x1, y1);
}

void display_gfx_draw_rect(display_gfx_t *gfx, int16_t x, int16_t y, int16_t w, int16_t h)
{
    u8g2_SetDrawColor(gfx->u8g2, GFX_COLOR_WHITE);
    u8g2_DrawFrame(gfx->u8g2, x, y, (uint16_t)w, (uint16_t)h);
}

void display_gfx_fill_rect(display_gfx_t *gfx, int16_t x, int16_t y, int16_t w, int16_t h)
{
    u8g2_SetDrawColor(gfx->u8g2, GFX_COLOR_WHITE);
    u8g2_DrawBox(gfx->u8g2, x, y, (uint16_t)w, (uint16_t)h);
}

void display_gfx_draw_triangle(display_gfx_t *gfx, int16_t x0, int16_t y0, int16_t x1,
                               int16_t y1, int16_t x2, int16_t y2)
{
    u8g2_SetDrawColor(gfx->u8g2, GFX_COLOR_WHITE);
    u8g2_DrawTriangle(gfx->u8g2, x0, y0, x1, y1, x2, y2);
}

void display_gfx_draw_bitmap(display_gfx_t *gfx, int16_t x, int16_t y, int16_t w, int16_t h,
                             const uint8_t *xbm_bits)
{
    u8g2_SetDrawColor(gfx->u8g2, GFX_COLOR_WHITE);
    u8g2_DrawXBMP(gfx->u8g2, x, y, (uint16_t)w, (uint16_t)h, xbm_bits);
}

void display_gfx_flush(display_gfx_t *gfx)
{
    u8g2_SendBuffer(gfx->u8g2);
}
