#ifndef SOURCE_CAPSENSE_APP_H_
#define SOURCE_CAPSENSE_APP_H_

#include <stdbool.h>
#include <stdint.h>
#include "cy_result.h"

typedef struct
{
    bool button0_active;
    bool button1_active;
    bool slider_active;
    uint16_t slider_position;
    uint16_t slider_resolution;
} capsense_app_state_t;

cy_rslt_t capsense_app_init(bool enable_tuner);
void capsense_app_process(void);
const capsense_app_state_t *capsense_app_get_state(void);

#endif /* SOURCE_CAPSENSE_APP_H_ */
