#include "display_diag.h"

#include <stdio.h>
#include <string.h>

#include "app_config.h"
#include "cy_scb_i2c.h"
#include "cyhal.h"
#include "cyhal_system.h"
#include "i2c_bus.h"

#define OLED_CTRL_CMD  (0x00u)
#define OLED_CTRL_DATA (0x40u)

static void log_line(const char *msg)
{
    printf("%s\r\n", msg);
    fflush(stdout);
}

static bool oled_write(uint8_t addr, const uint8_t *bytes, uint16_t len, const char *label)
{
    cy_en_scb_i2c_status_t st =
        (cy_en_scb_i2c_status_t)cyhal_i2c_master_write(i2c_bus_handle(), addr, bytes, len,
                                                      APP_I2C_TIMEOUT_MS, true);

    printf("  %-28s %u B -> %s (0x%08lx)\r\n", label, (unsigned int)len,
           i2c_bus_scb_status_str(st), (unsigned long)st);
    fflush(stdout);
    return CY_SCB_I2C_SUCCESS == st;
}

static bool oled_cmd1(uint8_t addr, uint8_t cmd, const char *label)
{
    const uint8_t buf[2] = { OLED_CTRL_CMD, cmd };

    return oled_write(addr, buf, 2u, label);
}

static bool oled_cmd2(uint8_t addr, uint8_t cmd, uint8_t arg, const char *label)
{
    const uint8_t buf[3] = { OLED_CTRL_CMD, cmd, arg };

    return oled_write(addr, buf, 3u, label);
}

static bool oled_fill_page_data(uint8_t addr, uint8_t fill_byte)
{
    uint8_t buf[1u + 128u];
    uint16_t i;

    buf[0] = OLED_CTRL_DATA;
    for (i = 0; i < 128u; i++)
    {
        buf[1u + i] = fill_byte;
    }
    return oled_write(addr, buf, sizeof(buf), "GDDRAM 128B data");
}

/** SH1106 (Adafruit 1.3"): DC-DC via 0xAD, not SSD1306 charge pump 0x8D. */
static bool oled_init_sh1106_winstar(uint8_t addr)
{
    bool ok = true;

    ok = oled_cmd1(addr, 0xAE, "display off") && ok;
    ok = oled_cmd1(addr, 0xA4, "entire display off") && ok;
    ok = oled_cmd2(addr, 0xD5, 0x50, "clock div") && ok;
    ok = oled_cmd2(addr, 0xA8, 0x3F, "multiplex 63") && ok;
    ok = oled_cmd2(addr, 0xD3, 0x00, "display offset") && ok;
    ok = oled_cmd1(addr, 0x40, "start line 0") && ok;
    ok = oled_cmd2(addr, 0xAD, 0x8B, "SH1106 DC-DC on") && ok;
    ok = oled_cmd2(addr, 0xD9, 0x22, "pre-charge") && ok;
    ok = oled_cmd2(addr, 0xDB, 0x35, "VCOM") && ok;
    ok = oled_cmd1(addr, 0x32, "pump voltage") && ok;
    ok = oled_cmd2(addr, 0x81, 0xFF, "contrast max") && ok;
    ok = oled_cmd1(addr, 0xA6, "normal display") && ok;
    ok = oled_cmd2(addr, 0xDA, 0x12, "com pins") && ok;
    ok = oled_cmd1(addr, 0xAF, "display on") && ok;
    return ok;
}

/** SSD1306-style init (0.96" modules). */
static bool oled_init_ssd1306(uint8_t addr)
{
    bool ok = true;

    ok = oled_cmd1(addr, 0xAE, "display off") && ok;
    ok = oled_cmd2(addr, 0xD5, 0x80, "clk div") && ok;
    ok = oled_cmd2(addr, 0xA8, 0x3F, "multiplex 63") && ok;
    ok = oled_cmd2(addr, 0xD3, 0x00, "display offset") && ok;
    ok = oled_cmd1(addr, 0x40, "start line 0") && ok;
    ok = oled_cmd2(addr, 0x8D, 0x14, "SSD1306 pump on") && ok;
    ok = oled_cmd2(addr, 0x20, 0x00, "horiz addr mode") && ok;
    ok = oled_cmd2(addr, 0x81, 0xCF, "contrast") && ok;
    ok = oled_cmd1(addr, 0xA1, "seg remap") && ok;
    ok = oled_cmd1(addr, 0xC8, "com scan dec") && ok;
    ok = oled_cmd2(addr, 0xDA, 0x12, "com pins") && ok;
    ok = oled_cmd1(addr, 0xAF, "display on") && ok;
    return ok;
}

static bool oled_page_fill_white(uint8_t addr, uint8_t col_low, uint8_t col_high)
{
    uint8_t page;
    bool ok = true;

    ok = oled_cmd2(addr, 0x20, 0x02, "page addr mode") && ok;

    for (page = 0u; page < 8u; page++)
    {
        char label[20];

        (void)snprintf(label, sizeof(label), "page %u setup", (unsigned int)page);
        ok = oled_cmd1(addr, (uint8_t)(0xB0u | page), label) && ok;
        ok = oled_cmd1(addr, col_low, "col low") && ok;
        ok = oled_cmd1(addr, col_high, "col high") && ok;
        ok = oled_fill_page_data(addr, 0xFFu) && ok;
    }
    return ok;
}

static void raw_fill_test(uint8_t addr, const char *tag, bool use_sh1106_init,
                          uint8_t col_low, uint8_t col_high)
{
    printf("display-diag: raw fill [%s] col cmd 0x%02X/0x%02X\r\n", tag,
           (unsigned int)col_low, (unsigned int)col_high);
    fflush(stdout);

    if ((use_sh1106_init ? oled_init_sh1106_winstar(addr) : oled_init_ssd1306(addr)) &&
        oled_page_fill_white(addr, col_low, col_high))
    {
        log_line("  -> expect full white screen");
    }
    else
    {
        log_line("  -> one or more I2C writes failed");
    }
}

void display_diag_run(uint8_t oled_addr_7bit)
{
    uint8_t read_byte = 0xA5u;
    cy_en_scb_i2c_status_t read_st;

    log_line("display-diag: no readable status registers (I2C is write-only)");
    log_line("display-diag: check RST->3.3V and module is in I2C mode (not SPI)");

    printf("display-diag: bus scan (%lu kHz)...\r\n",
           (unsigned long)(APP_I2C_FREQ_HZ / 1000UL));
    fflush(stdout);
    i2c_bus_scan();

    printf("display-diag: read 1 byte @0x%02X...\r\n", (unsigned int)oled_addr_7bit);
    fflush(stdout);
    read_st = i2c_bus_try_read_byte(oled_addr_7bit, &read_byte);
    printf("  read -> %s (0x%08lx) data=0x%02X\r\n", i2c_bus_scb_status_str(read_st),
           (unsigned long)read_st, (unsigned int)read_byte);
    if (CY_SCB_I2C_SUCCESS == read_st)
    {
        log_line("  WARNING: OLED normally NAKs reads; OK here may mean a different chip");
    }
    fflush(stdout);

    raw_fill_test(oled_addr_7bit, "SH1106 winstar init", true, 0x02, 0x10);
    cyhal_system_delay_ms(2000);
    raw_fill_test(oled_addr_7bit, "SSD1306 init", false, 0x00, 0x10);
}
