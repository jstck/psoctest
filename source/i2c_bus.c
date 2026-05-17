#include "i2c_bus.h"

#include "app_config.h"
#include "cybsp.h"
#include "cyhal.h"

static cyhal_i2c_t i2c_obj;

cy_rslt_t i2c_bus_init(void)
{
    cyhal_i2c_cfg_t cfg =
    {
        .is_slave = false,
        .address = 0,
        .frequencyhal_hz = APP_I2C_FREQ_HZ,
    };

    cy_rslt_t result = cyhal_i2c_init(&i2c_obj, CYBSP_I2C_SDA, CYBSP_I2C_SCL, NULL);
    if (CY_RSLT_SUCCESS != result)
    {
        return result;
    }

    return cyhal_i2c_configure(&i2c_obj, &cfg);
}

bool i2c_bus_probe(uint8_t address)
{
    return CY_RSLT_SUCCESS == cyhal_i2c_master_write(&i2c_obj, address, NULL, 0u,
                                                     10u, true);
}
