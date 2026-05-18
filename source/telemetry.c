#include "telemetry.h"

#include <stdio.h>

void telemetry_print_header(void)
{
    printf("\x1b[2J\x1b[;H");
    printf("PSoC 62S4 template telemetry\r\n");
    printf("Fields: temperature, ambient light, CapSense buttons/slider, GPIO mask, raw ADC counts\r\n\r\n");
}

void telemetry_print(const sensor_readings_t *sensors,
                     const capsense_app_state_t *capsense,
                     uint16_t gpio_mask)
{
    printf("ADC=%s T=%3.1fC ALS=%u%% BTN0=%u BTN1=%u SLIDER=%3u/%3u GPIO=0x%02X"
           " TH=%ld REF=%ld ALS_RAW=%ld A0=%ld A1=%ld\r\n",
           sensors->adc_ok ? "OK" : "ERR",
           sensors->temperature_c,
           (unsigned int)sensors->light_percent,
           capsense->button0_active ? 1u : 0u,
           capsense->button1_active ? 1u : 0u,
           (unsigned int)(capsense->slider_active ? capsense->slider_position : 0u),
           (unsigned int)capsense->slider_resolution,
           (unsigned int)gpio_mask,
           (long)sensors->thermistor_counts,
           (long)sensors->reference_counts,
           (long)sensors->als_counts,
           (long)sensors->analog0_counts,
           (long)sensors->analog1_counts);
}
