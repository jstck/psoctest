#include "midi_uart.h"

#include "app_config.h"
#include "cybsp.h"
#include "cyhal.h"

static cyhal_uart_t midi_uart;

cy_rslt_t midi_uart_init(void)
{
    cyhal_uart_cfg_t cfg =
    {
        .data_bits = 8,
        .stop_bits = 1,
        .parity = CYHAL_UART_PARITY_NONE,
        .rx_buffer = NULL,
        .rx_buffer_size = 0,
    };
    cy_rslt_t result;

    result = cyhal_uart_init(&midi_uart, CYBSP_D1, CYBSP_D0, NC, NC, NULL, &cfg);
    if (CY_RSLT_SUCCESS != result)
    {
        return result;
    }

    return cyhal_uart_set_baud(&midi_uart, APP_MIDI_BAUDRATE, NULL);
}

cy_rslt_t midi_uart_write(const uint8_t *data, size_t length)
{
    for (size_t idx = 0; idx < length; ++idx)
    {
        cy_rslt_t result = cyhal_uart_putc(&midi_uart, data[idx]);
        if (CY_RSLT_SUCCESS != result)
        {
            return result;
        }
    }

    return CY_RSLT_SUCCESS;
}

cy_rslt_t midi_uart_send_note_on(uint8_t channel, uint8_t note, uint8_t velocity)
{
    uint8_t msg[3] =
    {
        (uint8_t)(0x90u | (channel & 0x0Fu)),
        (uint8_t)(note & 0x7Fu),
        (uint8_t)(velocity & 0x7Fu),
    };

    return midi_uart_write(msg, sizeof(msg));
}
