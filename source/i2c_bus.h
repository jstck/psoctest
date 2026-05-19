#ifndef SOURCE_I2C_BUS_H_
#define SOURCE_I2C_BUS_H_

#include <stdbool.h>
#include <stdint.h>
#include "cy_result.h"
#include "cy_scb_i2c.h"
#include "cyhal_i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

cy_rslt_t i2c_bus_init(void);
cyhal_i2c_t *i2c_bus_handle(void);
cy_rslt_t i2c_bus_set_frequency(uint32_t frequency_hz);

uint8_t i2c_bus_ssd1306_addr(void);
void i2c_bus_set_ssd1306_addr(uint8_t address);

const char *i2c_bus_scb_status_str(cy_en_scb_i2c_status_t status);
void i2c_bus_scan(void);
cy_en_scb_i2c_status_t i2c_bus_try_read_byte(uint8_t address, uint8_t *data);

bool i2c_bus_probe_ssd1306(uint8_t address);
bool i2c_bus_probe(uint8_t address);
cy_rslt_t i2c_bus_probe_result(uint8_t address);

bool i2c_bus_write_reg8(uint8_t addr_7bit, uint8_t reg, uint8_t value);
bool i2c_bus_read_reg8(uint8_t addr_7bit, uint8_t reg, uint8_t *value);

#ifdef __cplusplus
}
#endif

#endif /* SOURCE_I2C_BUS_H_ */
