#ifndef SOURCE_APP_CONFIG_H_
#define SOURCE_APP_CONFIG_H_

#include <stdbool.h>
#include <stdint.h>
#include "cybsp.h"
#include "cyhal.h"

#define APP_LOOP_DELAY_MS          (10UL)
#define APP_SENSOR_PERIOD_MS       (100UL)
#define APP_TELEMETRY_PERIOD_MS    (1000UL)

#define APP_ENABLE_SAR_ADC         (0)
#define APP_ENABLE_I2C             (0)
#define APP_ENABLE_DISPLAY         (0)
#define APP_ENABLE_MIDI            (0)
#define APP_ENABLE_CAPSENSE_TUNER  (0)

/* SSD1306/SH1106 I2C fast-mode max is 400 kHz (not 1 MHz / 4 MHz). */
#define APP_I2C_FREQ_HZ            (400000UL)
#define APP_I2C_TIMEOUT_MS         (100UL)
#define APP_DISPLAY_USE_SH1106     (0)
#define APP_DISPLAY_SH1106_WINSTAR (0)
#define APP_ENABLE_DISPLAY_DIAG    (0)

#define APP_SSD1306_I2C_ADDR       (0x3Cu)
#define APP_SSD1306_I2C_ADDR_ALT   (0x3Du)
#define APP_DISPLAY_PERIOD_MS      (500UL)
#define APP_DISPLAY_CONTRAST       (255U)
#define APP_ENABLE_MCP23008        (0)
#define APP_MCP23008_I2C_ADDR      (0x20u)
#define APP_MCP23008_BTN_MASK      (0x0Fu)
#define APP_MCP23008_INT_PIN       (CYBSP_D2)
#define APP_MCP23008_INT_IRQ_PRIORITY (7u)
#define APP_MIDI_BAUDRATE          (31250UL)

#endif /* SOURCE_APP_CONFIG_H_ */
