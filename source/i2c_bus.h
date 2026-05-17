#ifndef SOURCE_I2C_BUS_H_
#define SOURCE_I2C_BUS_H_

#include <stdbool.h>
#include <stdint.h>
#include "cy_result.h"

cy_rslt_t i2c_bus_init(void);
bool i2c_bus_probe(uint8_t address);

#endif /* SOURCE_I2C_BUS_H_ */
