#ifndef SOURCE_DISPLAY_SSD1306_H_
#define SOURCE_DISPLAY_SSD1306_H_

#include <stdbool.h>
#include <stdint.h>

#include "capsense_app.h"
#include "cy_result.h"
#include "display_gfx.h"
#include "sensors_adc.h"
#include "u8g2.h"

u8g2_t *display_u8g2(void);
display_gfx_t *display_gfx(void);

cy_rslt_t display_init(void);
void display_update(const sensor_readings_t *sensors,
                    const capsense_app_state_t *capsense,
                    uint16_t gpio_mask);

#endif /* SOURCE_DISPLAY_SSD1306_H_ */
