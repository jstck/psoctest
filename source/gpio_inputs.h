#ifndef SOURCE_GPIO_INPUTS_H_
#define SOURCE_GPIO_INPUTS_H_

#include <stdint.h>
#include "cy_result.h"

cy_rslt_t gpio_inputs_init(void);
uint16_t gpio_inputs_read_mask(void);

#endif /* SOURCE_GPIO_INPUTS_H_ */
