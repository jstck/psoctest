#include "mcp23008_buttons.h"

#include <stdio.h>
#include <string.h>

#include "app_config.h"
#include "cybsp.h"
#include "cyhal.h"
#include "i2c_bus.h"

/* MCP23008 register map */
#define MCP23008_REG_IODIR    (0x00u)
#define MCP23008_REG_GPINTEN  (0x02u)
#define MCP23008_REG_INTCON   (0x04u)
#define MCP23008_REG_IOCON    (0x05u)
#define MCP23008_REG_GPPU     (0x06u)
#define MCP23008_REG_INTF     (0x07u)
#define MCP23008_REG_GPIO     (0x09u)

#define MCP23008_ADDR_MIN     (0x20u)
#define MCP23008_ADDR_MAX     (0x27u)

/* IOCON: INT open-drain, active low */
#define MCP23008_IOCON_INTCFG (0x02u)

typedef struct
{
    uint8_t gpio_bit;
    char label;
} btn_map_t;

/* Display order U D M E; GPIO GP3=Up, GP0=Down, GP1=Menu, GP2=Enter. */
static const btn_map_t s_btn_map[] =
{
    { 3u, 'U' },
    { 0u, 'D' },
    { 1u, 'M' },
    { 2u, 'E' },
};

static bool s_ready = false;
static uint8_t s_i2c_addr = APP_MCP23008_I2C_ADDR;
static volatile bool s_irq_pending = false;
static uint8_t s_last_gpio = APP_MCP23008_BTN_MASK;

static cyhal_gpio_callback_data_t s_int_cb =
{
    .callback = NULL,
    .callback_arg = NULL,
    .next = NULL,
    .pin = NC,
};

static void mcp23008_int_isr(void *callback_arg, cyhal_gpio_event_t event);
static void format_buttons(uint8_t gpio, char *out_5);
static void mcp23008_print_state(void);

static bool mcp23008_probe_address(uint8_t addr_7bit)
{
    uint8_t gpio = 0u;

    return i2c_bus_read_reg8(addr_7bit, MCP23008_REG_GPIO, &gpio);
}

static void mcp23008_scan_addresses(void)
{
    uint8_t addr;

    printf("init: scanning MCP23008 addresses 0x%02X–0x%02X...\r\n",
           (unsigned int)MCP23008_ADDR_MIN, (unsigned int)MCP23008_ADDR_MAX);
    fflush(stdout);

    for (addr = MCP23008_ADDR_MIN; addr <= MCP23008_ADDR_MAX; addr++)
    {
        printf("init: probing MCP23008 at 0x%02X...\r\n", (unsigned int)addr);
        fflush(stdout);
        printf("init: 0x%02X %s\r\n", (unsigned int)addr,
               mcp23008_probe_address(addr) ? "ACK" : "no ACK");
        fflush(stdout);
    }
}

static bool mcp23008_select_address(void)
{
    uint8_t addr;
    uint8_t first_found = 0u;
    bool have_first = false;

    if (mcp23008_probe_address(APP_MCP23008_I2C_ADDR))
    {
        s_i2c_addr = APP_MCP23008_I2C_ADDR;
        return true;
    }

    for (addr = MCP23008_ADDR_MIN; addr <= MCP23008_ADDR_MAX; addr++)
    {
        if (mcp23008_probe_address(addr))
        {
            if (!have_first)
            {
                first_found = addr;
                have_first = true;
            }
        }
    }

    if (have_first)
    {
        s_i2c_addr = first_found;
        if (s_i2c_addr != APP_MCP23008_I2C_ADDR)
        {
            printf("init: MCP23008 using 0x%02X (configured 0x%02X not found)\r\n",
                   (unsigned int)s_i2c_addr, (unsigned int)APP_MCP23008_I2C_ADDR);
            fflush(stdout);
        }
        return true;
    }

    return false;
}

static bool mcp23008_configure_chip(void)
{
    uint8_t gpio = 0u;

    if (!i2c_bus_write_reg8(s_i2c_addr, MCP23008_REG_IODIR, 0xFFu))
    {
        return false;
    }

    if (!i2c_bus_write_reg8(s_i2c_addr, MCP23008_REG_GPPU, APP_MCP23008_BTN_MASK))
    {
        return false;
    }

    if (!i2c_bus_write_reg8(s_i2c_addr, MCP23008_REG_IOCON, MCP23008_IOCON_INTCFG))
    {
        return false;
    }

    /* Compare to previous pin value (interrupt on any change). */
    if (!i2c_bus_write_reg8(s_i2c_addr, MCP23008_REG_INTCON, 0x00u))
    {
        return false;
    }

    if (!i2c_bus_write_reg8(s_i2c_addr, MCP23008_REG_GPINTEN, APP_MCP23008_BTN_MASK))
    {
        return false;
    }

    /* Read INTF then GPIO: clears interrupt flags and releases INT pin. */
    if (!i2c_bus_read_reg8(s_i2c_addr, MCP23008_REG_INTF, &gpio))
    {
        return false;
    }
    if (!i2c_bus_read_reg8(s_i2c_addr, MCP23008_REG_GPIO, &gpio))
    {
        return false;
    }
    s_last_gpio = (uint8_t)(gpio & APP_MCP23008_BTN_MASK);

    return true;
}

static void mcp23008_int_isr(void *callback_arg, cyhal_gpio_event_t event)
{
    (void)callback_arg;
    (void)event;
    s_irq_pending = true;
}

static bool mcp23008_clear_chip_int(uint8_t *gpio_out)
{
    uint8_t intf = 0u;
    uint8_t gpio = APP_MCP23008_BTN_MASK;

    if (!i2c_bus_read_reg8(s_i2c_addr, MCP23008_REG_INTF, &intf))
    {
        return false;
    }
    if (!i2c_bus_read_reg8(s_i2c_addr, MCP23008_REG_GPIO, &gpio))
    {
        return false;
    }
    if (NULL != gpio_out)
    {
        *gpio_out = gpio;
    }
    return true;
}

static bool mcp23008_configure_mcu_int(void)
{
    cy_rslt_t result;

    /* D2 is not in gpio_inputs_init(); must init before IRQ. */
    result = cyhal_gpio_init(APP_MCP23008_INT_PIN,
                             CYHAL_GPIO_DIR_INPUT,
                             CYHAL_GPIO_DRIVE_PULLUP,
                             1);
    if (CY_RSLT_SUCCESS != result)
    {
        printf("init: MCP23008 INT pin init failed 0x%08lX\r\n", (unsigned long)result);
        fflush(stdout);
        return false;
    }

    s_int_cb.callback = mcp23008_int_isr;
    cyhal_gpio_register_callback(APP_MCP23008_INT_PIN, &s_int_cb);
    /* BOTH: INT is active-low; need a rising edge after GPIO read before next fall. */
    cyhal_gpio_enable_event(APP_MCP23008_INT_PIN, CYHAL_GPIO_IRQ_BOTH,
                            APP_MCP23008_INT_IRQ_PRIORITY, true);

    printf("init: MCP23008 INT on D2 / P5[0] (active low)\r\n");
    fflush(stdout);
    return true;
}

static void format_buttons(uint8_t gpio, char *out_5)
{
    uint32_t i;

    for (i = 0; i < (sizeof(s_btn_map) / sizeof(s_btn_map[0])); i++)
    {
        uint8_t bit = (uint8_t)(1u << s_btn_map[i].gpio_bit);

        if (0u == (gpio & bit))
        {
            out_5[i] = s_btn_map[i].label;
        }
        else
        {
            out_5[i] = '_';
        }
    }
    out_5[4] = '\0';
}

static void mcp23008_print_state_if_changed(uint8_t gpio)
{
    char btn[5];
    uint8_t masked = (uint8_t)(gpio & APP_MCP23008_BTN_MASK);

    if (masked == s_last_gpio)
    {
        return;
    }
    s_last_gpio = masked;
    format_buttons(masked, btn);
    printf("BTN:%s\r\n", btn);
    fflush(stdout);
}

static void mcp23008_service_interrupt(void)
{
    uint8_t gpio = APP_MCP23008_BTN_MASK;

    if (!mcp23008_clear_chip_int(&gpio))
    {
        return;
    }
    mcp23008_print_state_if_changed(gpio);
}

cy_rslt_t mcp23008_buttons_init(void)
{
    s_ready = false;
    s_irq_pending = false;
    s_i2c_addr = APP_MCP23008_I2C_ADDR;

    mcp23008_scan_addresses();

    if (!mcp23008_select_address())
    {
        printf("init: MCP23008 not found (0x%02X–0x%02X)\r\n",
               (unsigned int)MCP23008_ADDR_MIN, (unsigned int)MCP23008_ADDR_MAX);
        fflush(stdout);
        return CY_RSLT_TYPE_ERROR;
    }

    if (!mcp23008_configure_chip())
    {
        printf("init: MCP23008 configure failed @ 0x%02X\r\n", (unsigned int)s_i2c_addr);
        fflush(stdout);
        return CY_RSLT_TYPE_ERROR;
    }

    if (!mcp23008_configure_mcu_int())
    {
        return CY_RSLT_TYPE_ERROR;
    }

    s_ready = true;
    printf("init: MCP23008 @ 0x%02X OK\r\n", (unsigned int)s_i2c_addr);
    fflush(stdout);
    {
        char btn[5];

        format_buttons(s_last_gpio, btn);
        printf("BTN:%s\r\n", btn);
        fflush(stdout);
    }
    return CY_RSLT_SUCCESS;
}

bool mcp23008_buttons_ready(void)
{
    return s_ready;
}

bool mcp23008_buttons_poll(char *out_5)
{
    uint8_t gpio = APP_MCP23008_BTN_MASK;

    if (NULL == out_5)
    {
        return false;
    }

    if (!s_ready)
    {
        (void)strncpy(out_5, "----", 5);
        return false;
    }

    if (!i2c_bus_read_reg8(s_i2c_addr, MCP23008_REG_GPIO, &gpio))
    {
        (void)strncpy(out_5, "----", 5);
        return false;
    }

    format_buttons((uint8_t)(gpio & APP_MCP23008_BTN_MASK), out_5);
    return true;
}

void mcp23008_buttons_process_events(void)
{
    bool int_active;

    if (!s_ready)
    {
        return;
    }

    int_active = (cyhal_gpio_read(APP_MCP23008_INT_PIN) == 0);
    if (!s_irq_pending && !int_active)
    {
        return;
    }

    s_irq_pending = false;
    mcp23008_service_interrupt();
}
