#include "telemetry.h"

#include <stdio.h>

#include "app_config.h"
#include "mcp23008_buttons.h"

void telemetry_print_header(void)
{
    printf("\x1b[2J\x1b[;H");
    printf("PSoC 62S4 template telemetry\r\n");
    printf("CapSense pads: P0=J11.5 P1=J11.6 P2=A13 P3=J11.10 P4=J11.9 P5=J11.8 P6=J11.7\r\n\r\n");
}

void telemetry_print(const capsense_app_state_t *capsense)
{
    char mcp_btn[5];

    if (NULL == capsense)
    {
        return;
    }

#if APP_ENABLE_MCP23008
    (void)mcp23008_buttons_poll(mcp_btn);
#else
    mcp_btn[0] = '-';
    mcp_btn[1] = '-';
    mcp_btn[2] = '-';
    mcp_btn[3] = '-';
    mcp_btn[4] = '\0';
#endif

    printf("DIFF P0=%4u P1=%4u P2=%4u P3=%4u P4=%4u P5=%4u P6=%4u MCP=%s\r\n",
           (unsigned int)capsense->pad_diff[0],
           (unsigned int)capsense->pad_diff[1],
           (unsigned int)capsense->pad_diff[2],
           (unsigned int)capsense->pad_diff[3],
           (unsigned int)capsense->pad_diff[4],
           (unsigned int)capsense->pad_diff[5],
           (unsigned int)capsense->pad_diff[6],
           mcp_btn);

    printf("ACT  P0=%u P1=%u P2=%u P3=%u P4=%u P5=%u P6=%u\r\n",
           capsense->pad_active[0] ? 1u : 0u,
           capsense->pad_active[1] ? 1u : 0u,
           capsense->pad_active[2] ? 1u : 0u,
           capsense->pad_active[3] ? 1u : 0u,
           capsense->pad_active[4] ? 1u : 0u,
           capsense->pad_active[5] ? 1u : 0u,
           capsense->pad_active[6] ? 1u : 0u);
}
