#include "display_ssd1306.h"

#include <stdio.h>

#include "app_config.h"
#include "cyhal_system.h"
#include "display_diag.h"
#include "display_gfx.h"
#include "i2c_bus.h"
#include "mcp23008_buttons.h"
#include "mtb_ssd1306.h"
#include "u8g2.h"
#include "u8g2_hw_i2c_stats.h"
#include "u8g2_support.h"

static u8g2_t s_u8g2;
static display_gfx_t s_gfx;
static bool s_display_ready = false;

u8g2_t *display_u8g2(void)
{
    return &s_u8g2;
}

display_gfx_t *display_gfx(void)
{
    return &s_gfx;
}

cy_rslt_t display_init(void)
{
    cy_rslt_t result;
    bool found_3c;
    bool found_3d;
    uint8_t oled_addr = 0u;

    printf("display: probing SSD1306 at 0x%02X...\r\n", (unsigned int)APP_SSD1306_I2C_ADDR);
    fflush(stdout);
    found_3c = i2c_bus_probe_ssd1306(APP_SSD1306_I2C_ADDR);
    printf("display: 0x%02X %s\r\n",
           (unsigned int)APP_SSD1306_I2C_ADDR, found_3c ? "ACK" : "no ACK");
    fflush(stdout);

    printf("display: probing SSD1306 at 0x%02X...\r\n", (unsigned int)APP_SSD1306_I2C_ADDR_ALT);
    fflush(stdout);
    found_3d = i2c_bus_probe_ssd1306(APP_SSD1306_I2C_ADDR_ALT);
    printf("display: 0x%02X %s\r\n",
           (unsigned int)APP_SSD1306_I2C_ADDR_ALT, found_3d ? "ACK" : "no ACK");
    fflush(stdout);

    if (found_3c)
    {
        oled_addr = APP_SSD1306_I2C_ADDR;
    }
    else if (found_3d)
    {
        oled_addr = APP_SSD1306_I2C_ADDR_ALT;
    }
    else
    {
        printf("display: not found (SDA=D14/P6[5], SCL=D15/P6[4])\r\n");
        fflush(stdout);
        return CY_RSLT_TYPE_ERROR;
    }

    i2c_bus_set_ssd1306_addr(oled_addr);
    printf("display: using I2C address 0x%02X\r\n", (unsigned int)oled_addr);
    fflush(stdout);

#if APP_ENABLE_DISPLAY_DIAG
    display_diag_run(oled_addr);
#endif

    printf("display: mtb_ssd1306_init_i2c...\r\n");
    fflush(stdout);
    result = mtb_ssd1306_init_i2c(i2c_bus_handle());
    if (CY_RSLT_SUCCESS != result)
    {
        printf("display: mtb_ssd1306_init_i2c failed 0x%08lx\r\n", (unsigned long)result);
        fflush(stdout);
        return result;
    }

    printf("display: u8g2 setup + InitDisplay (%s)...\r\n",
#if APP_DISPLAY_USE_SH1106
#if APP_DISPLAY_SH1106_WINSTAR
           "SH1106 128x64 winstar"
#else
           "SH1106 128x64 noname"
#endif
#else
           "SSD1306 128x64"
#endif
           );
    fflush(stdout);
#if APP_DISPLAY_USE_SH1106
#if APP_DISPLAY_SH1106_WINSTAR
    u8g2_Setup_sh1106_i2c_128x64_winstar_f(&s_u8g2, U8G2_R0, u8x8_byte_hw_i2c,
                                          u8x8_gpio_and_delay_cb);
#else
    u8g2_Setup_sh1106_i2c_128x64_noname_f(&s_u8g2, U8G2_R0, u8x8_byte_hw_i2c,
                                          u8x8_gpio_and_delay_cb);
#endif
#else
    u8g2_Setup_ssd1306_i2c_128x64_noname_f(&s_u8g2, U8G2_R0, u8x8_byte_hw_i2c,
                                          u8x8_gpio_and_delay_cb);
#endif
    /* u8g2 stores 8-bit I2C address (7-bit addr << 1); cyhal uses 7-bit via i2c_bus. */
    u8x8_SetI2CAddress(&s_u8g2.u8x8, (uint8_t)(oled_addr << 1));
    u8g2_hw_i2c_reset_stats();
    u8g2_InitDisplay(&s_u8g2);
    u8g2_SetPowerSave(&s_u8g2, 0);
    u8g2_SetContrast(&s_u8g2, APP_DISPLAY_CONTRAST);

#if APP_ENABLE_DISPLAY_DIAG
    {
        uint32_t ok = 0u;
        uint32_t fail = 0u;
        uint32_t ovf = 0u;

        u8g2_hw_i2c_get_stats(&ok, &fail, &ovf);
        printf("display: u8g2 I2C after InitDisplay: ok=%lu fail=%lu overflow=%lu\r\n",
               (unsigned long)ok, (unsigned long)fail, (unsigned long)ovf);
        fflush(stdout);
    }
#endif

#if APP_ENABLE_DISPLAY_DIAG
    printf("display: solid white fill test...\r\n");
    fflush(stdout);
    u8g2_ClearBuffer(&s_u8g2);
    u8g2_SetDrawColor(&s_u8g2, 1);
    u8g2_DrawBox(&s_u8g2, 0, 0, 128, 64);
    u8g2_SendBuffer(&s_u8g2);
    cyhal_system_delay_ms(500);
#endif

    printf("display: drawing splash...\r\n");
    fflush(stdout);
    display_gfx_bind(&s_gfx, &s_u8g2);
    display_gfx_clear(&s_gfx);
    display_gfx_set_text_size(&s_gfx, 1);
    display_gfx_set_cursor(&s_gfx, 0, 0);
    display_gfx_println(&s_gfx, "PSoC6 template");
    display_gfx_println(&s_gfx, "u8g2 ready");
    display_gfx_flush(&s_gfx);

#if APP_ENABLE_DISPLAY_DIAG
    {
        uint32_t ok = 0u;
        uint32_t fail = 0u;
        uint32_t ovf = 0u;

        u8g2_hw_i2c_get_stats(&ok, &fail, &ovf);
        printf("display: u8g2 I2C after splash: ok=%lu fail=%lu overflow=%lu\r\n",
               (unsigned long)ok, (unsigned long)fail, (unsigned long)ovf);
        fflush(stdout);
    }
#endif

    s_display_ready = true;
    printf("display: ready\r\n");
    fflush(stdout);
    return CY_RSLT_SUCCESS;
}

void display_update(const capsense_app_state_t *capsense)
{
    char line[24];

    if (!s_display_ready || (NULL == capsense))
    {
        return;
    }

    display_gfx_clear(&s_gfx);
    display_gfx_set_text_size(&s_gfx, 1);
    display_gfx_set_cursor(&s_gfx, 0, 0);

    (void)snprintf(line, sizeof(line), "P0:%4u P1:%4u P2:%4u",
                   (unsigned int)capsense->pad_diff[0],
                   (unsigned int)capsense->pad_diff[1],
                   (unsigned int)capsense->pad_diff[2]);
    display_gfx_println(&s_gfx, line);

    (void)snprintf(line, sizeof(line), "P3:%4u P4:%4u P5:%4u",
                   (unsigned int)capsense->pad_diff[3],
                   (unsigned int)capsense->pad_diff[4],
                   (unsigned int)capsense->pad_diff[5]);
    display_gfx_println(&s_gfx, line);

    (void)snprintf(line, sizeof(line), "P6:%4u",
                   (unsigned int)capsense->pad_diff[6]);
    display_gfx_println(&s_gfx, line);

#if APP_ENABLE_MCP23008
    {
        char btn[5];

        (void)mcp23008_buttons_poll(btn);
        (void)snprintf(line, sizeof(line), "MCP:%s", btn);
        display_gfx_println(&s_gfx, line);
    }
#endif

    display_gfx_flush(&s_gfx);
}
