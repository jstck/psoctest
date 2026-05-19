#ifndef SOURCE_DISPLAY_DIAG_H_
#define SOURCE_DISPLAY_DIAG_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Run I2C/OLED diagnostics on the serial console (no u8g2). */
void display_diag_run(uint8_t oled_addr_7bit);

#ifdef __cplusplus
}
#endif

#endif /* SOURCE_DISPLAY_DIAG_H_ */
