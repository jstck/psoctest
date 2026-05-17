/******************************************************************************
* File Name: led.c
*
* Description: This file contains source code that controls LED.
*
* Related Document: README.md
*
*******************************************************************************
 * (c) 2019-2025, Infineon Technologies AG, or an affiliate of Infineon
 * Technologies AG. All rights reserved.
 * This software, associated documentation and materials ("Software") is
 * owned by Infineon Technologies AG or one of its affiliates ("Infineon")
 * and is protected by and subject to worldwide patent protection, worldwide
 * copyright laws, and international treaty provisions. Therefore, you may use
 * this Software only as provided in the license agreement accompanying the
 * software package from which you obtained this Software. If no license
 * agreement applies, then any use, reproduction, modification, translation, or
 * compilation of this Software is prohibited without the express written
 * permission of Infineon.
 *
 * Disclaimer: UNLESS OTHERWISE EXPRESSLY AGREED WITH INFINEON, THIS SOFTWARE
 * IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING, BUT NOT LIMITED TO, ALL WARRANTIES OF NON-INFRINGEMENT OF
 * THIRD-PARTY RIGHTS AND IMPLIED WARRANTIES SUCH AS WARRANTIES OF FITNESS FOR A
 * SPECIFIC USE/PURPOSE OR MERCHANTABILITY.
 * Infineon reserves the right to make changes to the Software without notice.
 * You are responsible for properly designing, programming, and testing the
 * functionality and safety of your intended application of the Software, as
 * well as complying with any legal requirements related to its use. Infineon
 * does not guarantee that the Software will be free from intrusion, data theft
 * or loss, or other breaches ("Security Breaches"), and Infineon shall have
 * no liability arising out of any Security Breaches. Unless otherwise
 * explicitly approved by Infineon, the Software may not be used in any
 * application where a failure of the Product or any consequences of the use
 * thereof can reasonably be expected to result in personal injury.
*******************************************************************************/

#include "led.h"

#include "cybsp.h"
#include "cyhal.h"

static const cyhal_gpio_t LED_PINS[] = { CYBSP_USER_LED1, CYBSP_USER_LED2 };

cy_rslt_t led_init(void)
{
    cy_rslt_t result;

    for (uint32_t idx = 0; idx < (sizeof(LED_PINS) / sizeof(LED_PINS[0])); ++idx)
    {
        result = cyhal_gpio_init(LED_PINS[idx], CYHAL_GPIO_DIR_OUTPUT,
                                 CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF);
        if (CY_RSLT_SUCCESS != result)
        {
            return result;
        }
    }

    return CY_RSLT_SUCCESS;
}

void led_set(app_led_t led, bool on)
{
    cyhal_gpio_write(LED_PINS[(uint32_t)led],
                     on ? CYBSP_LED_STATE_ON : CYBSP_LED_STATE_OFF);
}

void led_toggle(app_led_t led)
{
    cyhal_gpio_toggle(LED_PINS[(uint32_t)led]);
}
