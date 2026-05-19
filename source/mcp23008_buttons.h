#ifndef SOURCE_MCP23008_BUTTONS_H_
#define SOURCE_MCP23008_BUTTONS_H_

#include <stdbool.h>
#include <stdint.h>
#include "cy_result.h"

#ifdef __cplusplus
extern "C" {
#endif

cy_rslt_t mcp23008_buttons_init(void);
bool mcp23008_buttons_ready(void);
bool mcp23008_buttons_poll(char *out_5);
void mcp23008_buttons_process_events(void);

#ifdef __cplusplus
}
#endif

#endif /* SOURCE_MCP23008_BUTTONS_H_ */
