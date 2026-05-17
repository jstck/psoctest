#ifndef SOURCE_APP_CONFIG_H_
#define SOURCE_APP_CONFIG_H_

#include <stdbool.h>
#include <stdint.h>
#include "cybsp.h"
#include "cyhal.h"

#define APP_LOOP_DELAY_MS          (10UL)
#define APP_SENSOR_PERIOD_MS       (100UL)
#define APP_TELEMETRY_PERIOD_MS    (500UL)

#define APP_ENABLE_I2C             (true)
#define APP_ENABLE_MIDI            (true)
#define APP_ENABLE_CAPSENSE_TUNER  (false)

#define APP_I2C_FREQ_HZ            (400000UL)
#define APP_MIDI_BAUDRATE          (31250UL)

#define APP_GPIO_INPUT_COUNT       (8U)

static const cyhal_gpio_t APP_GPIO_INPUT_PINS[APP_GPIO_INPUT_COUNT] =
{
    CYBSP_D2,
    CYBSP_D3,
    CYBSP_D4,
    CYBSP_D5,
    CYBSP_D6,
    CYBSP_D7,
    CYBSP_D8,
    CYBSP_D9,
};

#endif /* SOURCE_APP_CONFIG_H_ */
