#include "sensors_adc.h"

#include <math.h>
#include <string.h>

#include "cy_pdl.h"
#include "cycfg.h"
#include "cycfg_peripherals.h"

#define REF_RESISTOR_CHANNEL        (0u)
#define THERMISTOR_SENSOR_CHANNEL   (1u)
#define ALS_SENSOR_CHANNEL          (2u)
#define ANALOG0_CHANNEL             (3u)
#define ANALOG1_CHANNEL             (4u)

#define R_REFERENCE                 (10000.0f)
#define B_CONSTANT                  (3380.0f)
#define R_INFINITY                  (0.1192855f)
#define ABSOLUTE_ZERO_C             (-273.15)
#define ALS_OFFSET_PERCENT          (20)

static double calculate_temperature_c(int32_t therm_count, int32_t ref_count);
static uint8_t calculate_light_percent(int32_t adc_count);

cy_rslt_t sensors_adc_init(void)
{
    cy_en_sar_status_t sar_status;
    cy_rslt_t result;

    result = Cy_SysAnalog_Init(&pass_0_aref_0_config);
    if (CY_RSLT_SUCCESS != result)
    {
        return result;
    }

    result = Cy_SysAnalog_DeepSleepInit(PASS, &cy_cfg_pass0_deep_sleep_config);
    if (CY_RSLT_SUCCESS != result)
    {
        return result;
    }

    Cy_SysAnalog_Enable();
    Cy_SysAnalog_LpOscEnable(PASS);

    sar_status = Cy_SAR_Init(SAR0, &pass_0_saradc_0_sar_0_config);
    if (CY_SAR_SUCCESS != sar_status)
    {
        return CY_RSLT_TYPE_ERROR;
    }

    sar_status = Cy_SAR_CommonInit(PASS, &pass_0_saradc_0_config);
    if (CY_SAR_SUCCESS != sar_status)
    {
        return CY_RSLT_TYPE_ERROR;
    }

    Cy_SAR_Enable(SAR0);

    return CY_RSLT_SUCCESS;
}

cy_rslt_t sensors_adc_read(sensor_readings_t *readings)
{
    if (NULL == readings)
    {
        return CY_RSLT_TYPE_ERROR;
    }

    memset(readings, 0, sizeof(*readings));

    Cy_SAR_ClearInterrupt(SAR0, CY_SAR_INTR_EOS);
    Cy_SAR_StartConvert(SAR0, CY_SAR_START_CONVERT_SINGLE_SHOT);

    if (CY_SAR_SUCCESS != Cy_SAR_IsEndConversion(SAR0, CY_SAR_WAIT_FOR_RESULT))
    {
        return CY_RSLT_TYPE_ERROR;
    }

    readings->reference_counts = Cy_SAR_GetResult32(SAR0, REF_RESISTOR_CHANNEL);
    readings->thermistor_counts = Cy_SAR_GetResult32(SAR0, THERMISTOR_SENSOR_CHANNEL);
    readings->als_counts = Cy_SAR_GetResult32(SAR0, ALS_SENSOR_CHANNEL);
    readings->analog0_counts = Cy_SAR_GetResult32(SAR0, ANALOG0_CHANNEL);
    readings->analog1_counts = Cy_SAR_GetResult32(SAR0, ANALOG1_CHANNEL);

    readings->temperature_c = calculate_temperature_c(readings->thermistor_counts,
                                                       readings->reference_counts);
    readings->light_percent = calculate_light_percent(readings->als_counts);

    return CY_RSLT_SUCCESS;
}

static double calculate_temperature_c(int32_t therm_count, int32_t ref_count)
{
    double r_thermistor;

    if ((therm_count <= 0) || (ref_count <= 0))
    {
        return 0.0;
    }

    r_thermistor = (double)therm_count * R_REFERENCE / (double)ref_count;
    return (double)(B_CONSTANT / logf((float)(r_thermistor / R_INFINITY))) +
           ABSOLUTE_ZERO_C;
}

static uint8_t calculate_light_percent(int32_t adc_count)
{
    int32_t als_level;

    if (adc_count < 0)
    {
        adc_count = 0;
    }

    als_level = ((adc_count * 100) >> 10) - ALS_OFFSET_PERCENT;

    if (als_level > 100)
    {
        als_level = 100;
    }
    else if (als_level < 0)
    {
        als_level = 0;
    }

    return (uint8_t)als_level;
}
