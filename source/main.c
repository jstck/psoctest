#include <stdbool.h>
#include <stdio.h>

#include "app_config.h"
#include "capsense_app.h"
#include "cy_retarget_io.h"
#include "cybsp.h"
#include "cyhal.h"
#include "gpio_inputs.h"
#include "i2c_bus.h"
#include "led.h"
#include "midi_uart.h"
#include "sensors_adc.h"
#include "telemetry.h"

static void handle_error(cy_rslt_t result);

int main(void)
{
    cy_rslt_t result;
    sensor_readings_t sensors = {0};
    uint32_t elapsed_ms = 0;
    uint32_t sensor_ms = 0;
    uint32_t telemetry_ms = 0;
    uint32_t heartbeat_ms = 0;

#if defined(CY_DEVICE_SECURE)
    cyhal_wdt_t wdt_obj;

    result = cyhal_wdt_init(&wdt_obj, cyhal_wdt_get_max_timeout_ms());
    CY_ASSERT(CY_RSLT_SUCCESS == result);
    cyhal_wdt_free(&wdt_obj);
#endif

    result = cybsp_init();
    handle_error(result);

    __enable_irq();

    result = cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX,
                                 CY_RETARGET_IO_BAUDRATE);
    handle_error(result);

    telemetry_print_header();

    handle_error(led_init());
    handle_error(gpio_inputs_init());
    handle_error(sensors_adc_init());
    handle_error(capsense_app_init(APP_ENABLE_CAPSENSE_TUNER));

    if (APP_ENABLE_I2C)
    {
        handle_error(i2c_bus_init());
    }

    if (APP_ENABLE_MIDI)
    {
        handle_error(midi_uart_init());
    }

    for (;;)
    {
        capsense_app_process();

        if ((elapsed_ms - sensor_ms) >= APP_SENSOR_PERIOD_MS)
        {
            sensor_ms = elapsed_ms;
            (void)sensors_adc_read(&sensors);

            /* Simple visible behavior: LED2 follows ambient light threshold. */
            led_set(APP_LED_2, sensors.light_percent < 45u);
        }

        if ((elapsed_ms - telemetry_ms) >= APP_TELEMETRY_PERIOD_MS)
        {
            telemetry_ms = elapsed_ms;
            telemetry_print(&sensors, capsense_app_get_state(), gpio_inputs_read_mask());
        }

        if ((elapsed_ms - heartbeat_ms) >= 1000UL)
        {
            heartbeat_ms = elapsed_ms;
            led_toggle(APP_LED_1);
        }

        cyhal_system_delay_ms(APP_LOOP_DELAY_MS);
        elapsed_ms += APP_LOOP_DELAY_MS;
    }
}

static void handle_error(cy_rslt_t result)
{
    if (CY_RSLT_SUCCESS == result)
    {
        return;
    }

    __disable_irq();
    printf("Fatal init error: 0x%08lx\r\n", (unsigned long)result);
    CY_ASSERT(0);
    for (;;)
    {
    }
}
