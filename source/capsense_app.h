#ifndef SOURCE_CAPSENSE_APP_H_
#define SOURCE_CAPSENSE_APP_H_

#include <stdbool.h>
#include <stdint.h>
#include "cy_result.h"

/* Seven CSD electrodes: kit P7/P8/P9 pins (J11 rework + A13 for slider0). */
#define APP_CAPSENSE_PAD_COUNT  (7u)

typedef struct
{
    uint16_t pad_diff[APP_CAPSENSE_PAD_COUNT];
    bool pad_active[APP_CAPSENSE_PAD_COUNT];
} capsense_app_state_t;

cy_rslt_t capsense_app_init(bool enable_tuner);
void capsense_app_process(void);
const capsense_app_state_t *capsense_app_get_state(void);

#endif /* SOURCE_CAPSENSE_APP_H_ */
