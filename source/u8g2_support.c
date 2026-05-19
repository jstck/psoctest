/******************************************************************************
 * Local override of Infineon u8g2_support.c: finite I2C timeouts so a stuck
 * or missing OLED cannot block forever (stock code uses timeout 0 = wait forever).
 ******************************************************************************/

#if defined(COMPONENT_U8G2)

#include "app_config.h"
#include "cyhal_i2c.h"
#include "cyhal_system.h"
#include "cy_scb_i2c.h"
#include "i2c_bus.h"
#include "u8g2_hw_i2c_stats.h"
#include "u8g2_support.h"

#if defined(__cplusplus)
extern "C" {
#endif

static cyhal_i2c_t *i2c_ptr;
static uint32_t s_writes_ok;
static uint32_t s_writes_fail;
static uint32_t s_buf_overflow;

void u8g2_hw_i2c_reset_stats(void)
{
    s_writes_ok = 0u;
    s_writes_fail = 0u;
    s_buf_overflow = 0u;
}

void u8g2_hw_i2c_get_stats(uint32_t *writes_ok, uint32_t *writes_fail, uint32_t *buf_overflow)
{
    if (NULL != writes_ok)
    {
        *writes_ok = s_writes_ok;
    }
    if (NULL != writes_fail)
    {
        *writes_fail = s_writes_fail;
    }
    if (NULL != buf_overflow)
    {
        *buf_overflow = s_buf_overflow;
    }
}

uint8_t u8x8_byte_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    static uint8_t buffer[64];
    static uint8_t buf_idx = 0;

    cy_rslt_t rslt;
    uint8_t *data;

    (void)u8x8;

    switch (msg)
    {
        case U8X8_MSG_BYTE_SEND:
            data = (uint8_t *)arg_ptr;
            while (arg_int > 0)
            {
                if (buf_idx >= (uint8_t)sizeof(buffer))
                {
                    s_buf_overflow++;
                    return 0;
                }
                buffer[buf_idx++] = *data;
                data++;
                arg_int--;
            }
            break;

        case U8X8_MSG_BYTE_INIT:
            i2c_ptr = i2c_bus_handle();
            if (NULL == i2c_ptr)
            {
                return 0;
            }
            break;

        case U8X8_MSG_BYTE_SET_DC:
            break;

        case U8X8_MSG_BYTE_START_TRANSFER:
            buf_idx = 0;
            break;

        case U8X8_MSG_BYTE_END_TRANSFER:
            if ((NULL == i2c_ptr) || (0u == buf_idx))
            {
                return 0;
            }
            rslt = cyhal_i2c_master_write(i2c_ptr, i2c_bus_ssd1306_addr(), buffer, buf_idx,
                                          APP_I2C_TIMEOUT_MS, true);
            if (CY_SCB_I2C_SUCCESS != (cy_en_scb_i2c_status_t)rslt)
            {
                s_writes_fail++;
                return 0;
            }
            s_writes_ok++;
            break;

        default:
            return 0;
    }
    return 1;
}

uint8_t u8x8_gpio_and_delay_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    (void)u8x8;
    (void)arg_ptr;

    switch (msg)
    {
        case U8X8_MSG_GPIO_AND_DELAY_INIT:
            break;

        case U8X8_MSG_DELAY_MILLI:
            cyhal_system_delay_ms(arg_int);
            break;

        case U8X8_MSG_DELAY_10MICRO:
            cyhal_system_delay_us(10);
            break;

        case U8X8_MSG_DELAY_100NANO:
            cyhal_system_delay_us(1);
            break;

        case U8X8_MSG_GPIO_SPI_CLOCK:
        case U8X8_MSG_GPIO_SPI_DATA:
        case U8X8_MSG_GPIO_CS:
        case U8X8_MSG_GPIO_DC:
        case U8X8_MSG_GPIO_RESET:
        case U8X8_MSG_GPIO_D2:
        case U8X8_MSG_GPIO_D3:
        case U8X8_MSG_GPIO_D4:
        case U8X8_MSG_GPIO_D5:
        case U8X8_MSG_GPIO_D6:
        case U8X8_MSG_GPIO_D7:
        case U8X8_MSG_GPIO_E:
        case U8X8_MSG_GPIO_I2C_CLOCK:
        case U8X8_MSG_GPIO_I2C_DATA:
            break;

        default:
            return 0;
    }
    return 1;
}

#if defined(__cplusplus)
}
#endif

#endif /* defined(COMPONENT_U8G2) */
