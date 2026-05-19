#include "i2c_bus.h"

#include <stdio.h>
#include <string.h>

#include "app_config.h"
#include "cy_scb_i2c.h"
#include "cybsp.h"
#include "cyhal.h"

static cyhal_i2c_t i2c_obj;
static uint8_t s_ssd1306_addr = APP_SSD1306_I2C_ADDR;

static bool scb_write_ok(cy_rslt_t status)
{
    return CY_SCB_I2C_SUCCESS == (cy_en_scb_i2c_status_t)status;
}

const char *i2c_bus_scb_status_str(cy_en_scb_i2c_status_t status)
{
    switch (status)
    {
        case CY_SCB_I2C_SUCCESS:
            return "OK";
        case CY_SCB_I2C_BAD_PARAM:
            return "BAD_PARAM";
        case CY_SCB_I2C_MASTER_NOT_READY:
            return "MASTER_NOT_READY";
        case CY_SCB_I2C_MASTER_MANUAL_TIMEOUT:
            return "TIMEOUT";
        case CY_SCB_I2C_MASTER_MANUAL_ADDR_NAK:
            return "ADDR_NAK";
        case CY_SCB_I2C_MASTER_MANUAL_NAK:
            return "DATA_NAK";
        case CY_SCB_I2C_MASTER_MANUAL_ARB_LOST:
            return "ARB_LOST";
        case CY_SCB_I2C_MASTER_MANUAL_BUS_ERR:
            return "BUS_ERR";
        default:
            return "OTHER";
    }
}

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

cyhal_i2c_t *i2c_bus_handle(void)
{
    return &i2c_obj;
}

cy_rslt_t i2c_bus_set_frequency(uint32_t frequency_hz)
{
    cyhal_i2c_cfg_t cfg =
    {
        .is_slave = false,
        .address = 0,
        .frequencyhal_hz = frequency_hz,
    };

    return cyhal_i2c_configure(&i2c_obj, &cfg);
}

uint8_t i2c_bus_ssd1306_addr(void)
{
    return s_ssd1306_addr;
}

void i2c_bus_set_ssd1306_addr(uint8_t address)
{
    s_ssd1306_addr = address;
}

void i2c_bus_scan(void)
{
    uint8_t found[16];
    uint8_t count = 0u;
    uint8_t addr;

    for (addr = 0x08u; addr < 0x78u; addr++)
    {
        if (i2c_bus_probe_ssd1306(addr))
        {
            if (count < (uint8_t)(sizeof(found) / sizeof(found[0])))
            {
                found[count++] = addr;
            }
        }
    }

    if (0u == count)
    {
        printf("  (no devices ACK'd)\r\n");
    }
    else
    {
        uint8_t i;

        printf("  ACK at:");
        for (i = 0; i < count; i++)
        {
            printf(" 0x%02X", (unsigned int)found[i]);
        }
        printf("\r\n");
    }
    fflush(stdout);
}

cy_en_scb_i2c_status_t i2c_bus_try_read_byte(uint8_t address, uint8_t *data)
{
    if (NULL == data)
    {
        return CY_SCB_I2C_BAD_PARAM;
    }

    return (cy_en_scb_i2c_status_t)cyhal_i2c_master_read(&i2c_obj, address, data, 1u,
                                                        APP_I2C_TIMEOUT_MS, true);
}

bool i2c_bus_probe_ssd1306(uint8_t address)
{
    const uint8_t ctrl_cmd = 0x00u;

    return scb_write_ok(cyhal_i2c_master_write(&i2c_obj, address, &ctrl_cmd, 1u,
                                               APP_I2C_TIMEOUT_MS, true));
}

bool i2c_bus_probe(uint8_t address)
{
    return i2c_bus_probe_ssd1306(address);
}

cy_rslt_t i2c_bus_probe_result(uint8_t address)
{
    return i2c_bus_probe_ssd1306(address) ? CY_RSLT_SUCCESS : CY_RSLT_TYPE_ERROR;
}

bool i2c_bus_write_reg8(uint8_t addr_7bit, uint8_t reg, uint8_t value)
{
    return scb_write_ok(cyhal_i2c_master_mem_write(&i2c_obj, addr_7bit, reg, 1u, &value, 1u,
                                                   APP_I2C_TIMEOUT_MS));
}

bool i2c_bus_read_reg8(uint8_t addr_7bit, uint8_t reg, uint8_t *value)
{
    if (NULL == value)
    {
        return false;
    }

    return scb_write_ok(cyhal_i2c_master_mem_read(&i2c_obj, addr_7bit, reg, 1u, value, 1u,
                                                  APP_I2C_TIMEOUT_MS));
}
