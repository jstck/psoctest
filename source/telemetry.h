#ifndef SOURCE_TELEMETRY_H_
#define SOURCE_TELEMETRY_H_

#include "capsense_app.h"

void telemetry_print_header(void);
void telemetry_print(const capsense_app_state_t *capsense);

#endif /* SOURCE_TELEMETRY_H_ */
