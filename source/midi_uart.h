#ifndef SOURCE_MIDI_UART_H_
#define SOURCE_MIDI_UART_H_

#include <stddef.h>
#include <stdint.h>
#include "cy_result.h"

cy_rslt_t midi_uart_init(void);
cy_rslt_t midi_uart_write(const uint8_t *data, size_t length);
cy_rslt_t midi_uart_send_note_on(uint8_t channel, uint8_t note, uint8_t velocity);

#endif /* SOURCE_MIDI_UART_H_ */
