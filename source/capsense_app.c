#include "capsense_app.h"

#include "app_config.h"
#include "cybsp.h"
#include "cycfg_capsense.h"
#include "cyhal.h"

#define CAPSENSE_INTR_PRIORITY      (7u)
#define CAPSENSE_TUNER_I2C_ADDR     (8u)

static volatile bool scan_complete;
static bool tuner_enabled;
static capsense_app_state_t state;

static cyhal_ezi2c_t tuner_ezi2c;
static cyhal_ezi2c_slave_cfg_t tuner_slave_cfg;
static cyhal_ezi2c_cfg_t tuner_cfg;

static void capsense_isr(void);
static void capsense_callback(cy_stc_active_scan_sns_t *active_scan);
static cy_rslt_t init_tuner(void);
static void update_state(void);

cy_rslt_t capsense_app_init(bool enable_tuner)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    uint32_t status;

    tuner_enabled = enable_tuner;

    status = Cy_CapSense_Init(&cy_capsense_context);
    if (CYRET_SUCCESS != status)
    {
        return CY_RSLT_TYPE_ERROR;
    }

    cyhal_system_set_isr(csd_interrupt_IRQn, csd_interrupt_IRQn,
                         CAPSENSE_INTR_PRIORITY, capsense_isr);
    NVIC_ClearPendingIRQ(csd_interrupt_IRQn);
    NVIC_EnableIRQ(csd_interrupt_IRQn);

    status = Cy_CapSense_Enable(&cy_capsense_context);
    if (CYRET_SUCCESS != status)
    {
        return CY_RSLT_TYPE_ERROR;
    }

    status = Cy_CapSense_RegisterCallback(CY_CAPSENSE_END_OF_SCAN_E,
                                          capsense_callback,
                                          &cy_capsense_context);
    if (CYRET_SUCCESS != status)
    {
        return CY_RSLT_TYPE_ERROR;
    }

    if (tuner_enabled)
    {
        result = init_tuner();
        if (CY_RSLT_SUCCESS != result)
        {
            return result;
        }
    }

    state.slider_resolution =
        cy_capsense_context.ptrWdConfig[CY_CAPSENSE_LINEARSLIDER0_WDGT_ID].xResolution;

    Cy_CapSense_ScanAllWidgets(&cy_capsense_context);

    return CY_RSLT_SUCCESS;
}

void capsense_app_process(void)
{
    if (!scan_complete)
    {
        return;
    }

    scan_complete = false;

    Cy_CapSense_ProcessAllWidgets(&cy_capsense_context);
    update_state();

    if (tuner_enabled)
    {
        Cy_CapSense_RunTuner(&cy_capsense_context);
    }

    Cy_CapSense_ScanAllWidgets(&cy_capsense_context);
}

const capsense_app_state_t *capsense_app_get_state(void)
{
    return &state;
}

static cy_rslt_t init_tuner(void)
{
    tuner_slave_cfg.buf = (uint8_t *)&cy_capsense_tuner;
    tuner_slave_cfg.buf_rw_boundary = sizeof(cy_capsense_tuner);
    tuner_slave_cfg.buf_size = sizeof(cy_capsense_tuner);
    tuner_slave_cfg.slave_address = CAPSENSE_TUNER_I2C_ADDR;

    tuner_cfg.data_rate = CYHAL_EZI2C_DATA_RATE_400KHZ;
    tuner_cfg.enable_wake_from_sleep = false;
    tuner_cfg.slave1_cfg = tuner_slave_cfg;
    tuner_cfg.sub_address_size = CYHAL_EZI2C_SUB_ADDR16_BITS;
    tuner_cfg.two_addresses = false;

    return cyhal_ezi2c_init(&tuner_ezi2c, CYBSP_I2C_SDA, CYBSP_I2C_SCL,
                            NULL, &tuner_cfg);
}

static void update_state(void)
{
    cy_stc_capsense_touch_t *slider_touch_info;

    state.button0_active = 0u != Cy_CapSense_IsSensorActive(
        CY_CAPSENSE_BUTTON0_WDGT_ID, CY_CAPSENSE_BUTTON0_SNS0_ID,
        &cy_capsense_context);

    state.button1_active = 0u != Cy_CapSense_IsSensorActive(
        CY_CAPSENSE_BUTTON1_WDGT_ID, CY_CAPSENSE_BUTTON1_SNS0_ID,
        &cy_capsense_context);

    slider_touch_info = Cy_CapSense_GetTouchInfo(CY_CAPSENSE_LINEARSLIDER0_WDGT_ID,
                                                 &cy_capsense_context);
    state.slider_active = slider_touch_info->numPosition != 0u;
    state.slider_position = state.slider_active ? slider_touch_info->ptrPosition->x : 0u;
}

static void capsense_isr(void)
{
    Cy_CapSense_InterruptHandler(CYBSP_CSD_HW, &cy_capsense_context);
}

static void capsense_callback(cy_stc_active_scan_sns_t *active_scan)
{
    (void)active_scan;
    scan_complete = true;
}
