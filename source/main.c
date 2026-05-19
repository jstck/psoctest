#include <stdbool.h>
#include <stdio.h>

#include "app_config.h"
#include "capsense_app.h"
#include "cy_retarget_io.h"
#include "cybsp.h"
#include "cyhal.h"
#include "display_ssd1306.h"
#include "gpio_inputs.h"
#include "i2c_bus.h"
#include "led.h"
#include "mcp23008_buttons.h"
#include "midi_uart.h"
#include "sensors_adc.h"
#include "telemetry.h"

static void handle_error(cy_rslt_t result);
static void boot_log(const char *msg);

int main(void)
{
    cy_rslt_t result;
    sensor_readings_t sensors = {0};
    uint32_t elapsed_ms = 0;
    uint32_t sensor_ms = 0;
    uint32_t telemetry_ms = 0;
    uint32_t display_ms = 0;
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

    boot_log("init: LED...");
    handle_error(led_init());

    boot_log("init: GPIO inputs...");
    handle_error(gpio_inputs_init());

    boot_log("init: SAR ADC...");
    handle_error(sensors_adc_init());

    boot_log("init: CapSense...");
    handle_error(capsense_app_init(APP_ENABLE_CAPSENSE_TUNER));

    if (APP_ENABLE_I2C)
    {
        boot_log("init: I2C master (SDA D14/P6[5], SCL D15/P6[4])...");
        handle_error(i2c_bus_init());
        boot_log("init: I2C configured");

#if APP_ENABLE_MCP23008
        if (CY_RSLT_SUCCESS != mcp23008_buttons_init())
        {
            boot_log("init: MCP23008 not available (app continues)");
        }
#endif

#if APP_ENABLE_DISPLAY
        if (CY_RSLT_SUCCESS != display_init())
        {
            boot_log("init: SSD1306 not available (app continues)");
        }
#endif
    }

    if (APP_ENABLE_MIDI)
    {
        boot_log("init: MIDI UART...");
        handle_error(midi_uart_init());
    }

    boot_log("init: complete — entering main loop");

    for (;;)
    {
#if APP_ENABLE_MCP23008
        mcp23008_buttons_process_events();
#endif

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

#if APP_ENABLE_DISPLAY
        if ((elapsed_ms - display_ms) >= APP_DISPLAY_PERIOD_MS)
        {
            display_ms = elapsed_ms;
            display_update(&sensors, capsense_app_get_state(), gpio_inputs_read_mask());
        }
#endif

        if ((elapsed_ms - heartbeat_ms) >= 1000UL)
        {
            heartbeat_ms = elapsed_ms;
            led_toggle(APP_LED_1);
        }

        cyhal_system_delay_ms(APP_LOOP_DELAY_MS);
        elapsed_ms += APP_LOOP_DELAY_MS;
    }
}

static void boot_log(const char *msg)
{
    printf("%s\r\n", msg);
    fflush(stdout);
}

static void handle_error(cy_rslt_t result)
{
    if (CY_RSLT_SUCCESS == result)
    {
        return;
    }

    __disable_irq();
    printf("Fatal init error: 0x%08lx\r\n", (unsigned long)result);
    fflush(stdout);
    CY_ASSERT(0);
    for (;;)
    {
    }
}
