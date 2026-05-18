#ifndef SOURCE_SENSORS_ADC_H_
#define SOURCE_SENSORS_ADC_H_

#include <stdbool.h>
#include <stdint.h>
#include "cy_result.h"

typedef struct
{
    bool adc_ok;
    int32_t thermistor_counts;
    int32_t reference_counts;
    int32_t als_counts;
    int32_t analog0_counts;
    int32_t analog1_counts;
    double temperature_c;
    uint8_t light_percent;
} sensor_readings_t;

cy_rslt_t sensors_adc_init(void);
cy_rslt_t sensors_adc_read(sensor_readings_t *readings);

#endif /* SOURCE_SENSORS_ADC_H_ */
