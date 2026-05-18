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
#define CHANNEL_COUNT               (5u)
#define ALL_CHANNELS_SEEN_MASK      ((1u << CHANNEL_COUNT) - 1u)

#define R_REFERENCE                 (10000.0f)
#define B_CONSTANT                  (3380.0f)
#define R_INFINITY                  (0.1192855f)
#define ABSOLUTE_ZERO_C             (-273.15)
#define ALS_DARK_COUNTS             (100.0)
#define ALS_FULL_SCALE_COUNTS       (32767.0)

static double calculate_temperature_c(int32_t therm_count, int32_t ref_count);
static uint8_t calculate_light_percent(int32_t adc_count);

static int32_t latest_counts[CHANNEL_COUNT];
static uint32_t seen_channels;

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

    Cy_SysAnalog_TimerEnable(PASS);

    return CY_RSLT_SUCCESS;
}

cy_rslt_t sensors_adc_read(sensor_readings_t *readings)
{
    if (NULL == readings)
    {
        return CY_RSLT_TYPE_ERROR;
    }

    cy_stc_sar_fifo_read_t fifo_data = {0};
    uint32_t data_count = Cy_SAR_FifoGetDataCount(SAR0);

    while (data_count > 0u)
    {
        --data_count;
        Cy_SAR_FifoRead(SAR0, &fifo_data);

        if (fifo_data.channel < CHANNEL_COUNT)
        {
            latest_counts[fifo_data.channel] = fifo_data.value;
            seen_channels |= (1u << fifo_data.channel);
        }
    }

    memset(readings, 0, sizeof(*readings));

    readings->reference_counts = latest_counts[REF_RESISTOR_CHANNEL];
    readings->thermistor_counts = latest_counts[THERMISTOR_SENSOR_CHANNEL];
    readings->als_counts = latest_counts[ALS_SENSOR_CHANNEL];
    readings->analog0_counts = latest_counts[ANALOG0_CHANNEL];
    readings->analog1_counts = latest_counts[ANALOG1_CHANNEL];

    readings->temperature_c = calculate_temperature_c(readings->thermistor_counts,
                                                       readings->reference_counts);
    readings->light_percent = calculate_light_percent(readings->als_counts);
    readings->adc_ok = (seen_channels & ALL_CHANNELS_SEEN_MASK) == ALL_CHANNELS_SEEN_MASK;

    if (readings->adc_ok)
    {
        return CY_RSLT_SUCCESS;
    }

    return CY_RSLT_TYPE_ERROR;
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
    double normalized;
    int32_t als_level;

    if (adc_count <= (int32_t)ALS_DARK_COUNTS)
    {
        return 0u;
    }

    if (adc_count >= (int32_t)ALS_FULL_SCALE_COUNTS)
    {
        return 100u;
    }

    normalized = log((double)adc_count / ALS_DARK_COUNTS) /
                 log(ALS_FULL_SCALE_COUNTS / ALS_DARK_COUNTS);
    als_level = (int32_t)((normalized * 100.0) + 0.5);

    if (als_level < 0)
    {
        als_level = 0;
    }
    else if (als_level > 100)
    {
        als_level = 100;
    }

    return (uint8_t)als_level;
}
