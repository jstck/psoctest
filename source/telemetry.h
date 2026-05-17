#ifndef SOURCE_TELEMETRY_H_
#define SOURCE_TELEMETRY_H_

#include <stdint.h>
#include "capsense_app.h"
#include "sensors_adc.h"

void telemetry_print_header(void);
void telemetry_print(const sensor_readings_t *sensors,
                     const capsense_app_state_t *capsense,
                     uint16_t gpio_mask);

#endif /* SOURCE_TELEMETRY_H_ */
