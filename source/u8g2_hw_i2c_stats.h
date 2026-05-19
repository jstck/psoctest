#ifndef SOURCE_U8G2_HW_I2C_STATS_H_
#define SOURCE_U8G2_HW_I2C_STATS_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void u8g2_hw_i2c_reset_stats(void);
void u8g2_hw_i2c_get_stats(uint32_t *writes_ok, uint32_t *writes_fail, uint32_t *buf_overflow);

#ifdef __cplusplus
}
#endif

#endif /* SOURCE_U8G2_HW_I2C_STATS_H_ */
