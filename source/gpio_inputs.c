#include "gpio_inputs.h"

#include "app_config.h"
#include "cybsp.h"
#include "cyhal.h"

cy_rslt_t gpio_inputs_init(void)
{
    for (uint32_t idx = 0; idx < APP_GPIO_INPUT_COUNT; ++idx)
    {
        cy_rslt_t result = cyhal_gpio_init(APP_GPIO_INPUT_PINS[idx],
                                           CYHAL_GPIO_DIR_INPUT,
                                           CYHAL_GPIO_DRIVE_PULLUP,
                                           CYBSP_BTN_OFF);
        if (CY_RSLT_SUCCESS != result)
        {
            return result;
        }
    }

    return CY_RSLT_SUCCESS;
}

uint16_t gpio_inputs_read_mask(void)
{
    uint16_t mask = 0u;

    for (uint32_t idx = 0; idx < APP_GPIO_INPUT_COUNT; ++idx)
    {
        if (cyhal_gpio_read(APP_GPIO_INPUT_PINS[idx]) == 0u)
        {
            mask |= (uint16_t)(1u << idx);
        }
    }

    return mask;
}
