#ifndef SOURCE_APP_CONFIG_H_
#define SOURCE_APP_CONFIG_H_

#include <stdbool.h>
#include <stdint.h>
#include "cybsp.h"
#include "cyhal.h"

#define APP_LOOP_DELAY_MS          (10UL)
#define APP_SENSOR_PERIOD_MS       (100UL)
#define APP_TELEMETRY_PERIOD_MS    (1000UL)

#define APP_ENABLE_I2C             (true)
#define APP_ENABLE_DISPLAY         1
#define APP_ENABLE_MIDI            (true)
#define APP_ENABLE_CAPSENSE_TUNER  (false)

/* SSD1306/SH1106 I2C fast-mode max is 400 kHz (not 1 MHz / 4 MHz). */
#define APP_I2C_FREQ_HZ            (400000UL)
#define APP_I2C_TIMEOUT_MS         (100UL)
/* Adafruit 1.3" 128x64 (#1101) is SH1106 — use winstar init (0xAD DC-DC, not SSD1306 0x8D). */
#define APP_DISPLAY_USE_SH1106     (0)
#define APP_DISPLAY_SH1106_WINSTAR (0)
/* Bus scan, read probe, raw bar tests, u8g2 I2C write stats on serial at boot. */
#define APP_ENABLE_DISPLAY_DIAG    (0)

#define APP_SSD1306_I2C_ADDR       (0x3Cu)
#define APP_SSD1306_I2C_ADDR_ALT   (0x3Du)
#define APP_DISPLAY_PERIOD_MS      (500UL)
/* SSD1306 contrast 0–255 (u8g2 default after init is ~207). */
#define APP_DISPLAY_CONTRAST       (255U)
#define APP_ENABLE_MCP23008        (1)
/* 7-bit addr: 0x20 | (A2<<2 | A1<<1 | A0); A2=0 A1=1 A0=1 -> 0x23 */
#define APP_MCP23008_I2C_ADDR      (0x20u)
#define APP_MCP23008_BTN_MASK      (0x0Fu)
#define APP_MCP23008_INT_PIN       (CYBSP_D2)
#define APP_MCP23008_INT_IRQ_PRIORITY (7u)
#define APP_MIDI_BAUDRATE          (31250UL)

/* D2 is MCP23008 INT; kit header inputs are D3–D9 only. */
#define APP_GPIO_INPUT_COUNT       (7U)

static const cyhal_gpio_t APP_GPIO_INPUT_PINS[APP_GPIO_INPUT_COUNT] =
{
    CYBSP_D3,
    CYBSP_D4,
    CYBSP_D5,
    CYBSP_D6,
    CYBSP_D7,
    CYBSP_D8,
    CYBSP_D9,
};

#endif /* SOURCE_APP_CONFIG_H_ */
