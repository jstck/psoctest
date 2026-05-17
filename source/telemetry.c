#include "telemetry.h"

#include <inttypes.h>
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
    printf("T=%2.1fC ALS=%" PRIu8 "%% BTN0=%u BTN1=%u SLIDER=%u/%u GPIO=0x%02" PRIX16
           " TH=%" PRId32 " REF=%" PRId32 " ALS_RAW=%" PRId32
           " A0=%" PRId32 " A1=%" PRId32 "\r\n",
           sensors->temperature_c,
           sensors->light_percent,
           capsense->button0_active ? 1u : 0u,
           capsense->button1_active ? 1u : 0u,
           capsense->slider_active ? capsense->slider_position : 0u,
           capsense->slider_resolution,
           gpio_mask,
           sensors->thermistor_counts,
           sensors->reference_counts,
           sensors->als_counts,
           sensors->analog0_counts,
           sensors->analog1_counts);
}
