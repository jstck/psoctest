# PSoC 62S4 Pioneer Template

Template firmware for the CY8CKIT-062S4 PSoC 62S4 Pioneer Kit.

## Features

- Seven CapSense pads on kit **P7/P8/P9** CSD pins (J11 extended header + A13 after resistor rework); raw diff on serial for tuning.
- User LED heartbeat on LED1.
- Optional SAR ADC (`APP_ENABLE_SAR_ADC`), I2C display, MCP23008 buttons, and MIDI (off by default in `app_config.h`).
- I2C master on P6[4] SCL / P6[5] SDA.
- SSD1306 128x64 OLED via Infineon `display-oled-ssd1306` + u8g2 (see `source/display_gfx.*` for Adafruit-like helpers).
- MIDI UART scaffold on Arduino D1 TX / D0 RX at 31250 baud.
- Human-readable telemetry on the KitProg USB serial UART at 115200 baud.

## Tool Locations

This project defaults to user-local ModusToolbox install paths and can be
overridden with environment variables:

```bash
export MODUSTOOLBOX_HOME=$HOME/Applications/ModusToolbox
export CY_TOOLS_PATHS=$MODUSTOOLBOX_HOME/tools_3.8
export CY_COMPILER_GCC_ARM_DIR=$HOME/Applications/mtb-gcc-arm-eabi/14.2.1/gcc
export PATH=$CY_TOOLS_PATHS/make/bin:$CY_TOOLS_PATHS/ninja:$CY_COMPILER_GCC_ARM_DIR/bin:$PATH
```

The CapSense / Multi-Sense pack is expected at:

```bash
$MODUSTOOLBOX_HOME/packs/ModusToolbox-Multi-Sense-Pack
```

## Build and Program

From this project directory:

```bash
make deps
make build
make program
```

Or use the wrapper:

```bash
./scripts/build.sh
./scripts/build.sh program
```

`./scripts/build.sh` runs `make deps` first. **`make deps`** is an alias for **`make getlibs`**: it fetches Infineon libraries into `mtb_shared/` and third-party assets listed in `deps/*.mtb` (including u8g2 into gitignored `libs/u8g2/`). Fresh clone: run `make deps` once before building.

## Serial Telemetry

Open the KitProg USB serial port at 115200 8N1. Every second you get seven per-pad **difference counts** and active flags (P0–P6 map to J11/A13 after kit rework):

```text
DIFF P0=  12 P1=   8 P2=  15 P3=  20 P4=  11 P5=   9 P6=  14 MCP=----
ACT  P0=0 P1=0 P2=0 P3=0 P4=0 P5=0 P6=0
```

| Index | After J11 rework | MCU pin | CapSense widget |
| --- | --- | --- | --- |
| P0 | J11.5 | P7[0] | Button0 |
| P1 | J11.6 | P7[1] | Button1 |
| P2 | A13 (J2.12) | P9[3] | Pad5 |
| P3 | J11.10 | P8[1] | Pad6 |
| P4 | J11.9 | P8[0] | Pad7 |
| P5 | J11.8 | P7[3] | Pad8 |
| P6 | J11.7 | P7[2] | Pad9 |

Values rise when touched. Tune in CapSense Configurator or Tuner (`APP_ENABLE_CAPSENSE_TUNER`). See kit guide §3.3.1 for resistor swaps (R48/R50, etc.).

Config: `design.cycapsense` (seven `CSD_BUTTON` widgets) and `design.modus` (CSD sense mux). Regenerate after edits:

```bash
export CY_TOOLS_PATHS=$HOME/Applications/ModusToolbox/tools_3.8
capsense-configurator-cli -c bsps/TARGET_APP_CY8CKIT-062S4/config/design.cycapsense \
  -o bsps/TARGET_APP_CY8CKIT-062S4/config/GeneratedSource
device-configurator-cli --build bsps/TARGET_APP_CY8CKIT-062S4/config/design.modus
```

## OLED display (u8g2)

Wire the module to the Arduino header I2C pins (3.3 V, GND, SDA=D14, SCL=D15). Tie **RST** to 3.3 V. **Adafruit 1.3" 128×64** (#1101) is **SH1106** — use `APP_DISPLAY_USE_SH1106=1` and `APP_DISPLAY_SH1106_WINSTAR=1` (correct DC-DC init); u8g2’s `noname` SH1106 driver wrongly sends SSD1306 charge-pump commands. **0.96"** modules are usually SSD1306 (`APP_DISPLAY_USE_SH1106=0`). After `make deps`, the build uses:

- `mtb_shared/display-oled-ssd1306` — low-level SSD1306 init (`mtb_ssd1306_init_i2c` on the shared `i2c_bus`)
- `libs/u8g2/csrc` — u8g2 graphics library (`deps/u8g2.mtb`, fetched by `make deps` into gitignored `libs/`)
- `source/display_gfx.c` — thin wrappers named like Adafruit_GFX for porting Teensy menu code

Text size mapping for `display_gfx_set_text_size()`:

| Size | u8g2 font (approx. Adafruit) |
| --- | --- |
| 0, 1 | 5x8 (menu default; close to Adafruit size 1) |
| 2 | 9x15 |
| 6 | logisoso32 (large digits) |

Boot logos: convert once to XBM and call `display_gfx_draw_bitmap()`.

### MCP23008 buttons (same I2C bus)

On the display board, an **MCP23008** GPIO expander shares the OLED I2C bus at **0x23** (7-bit; A2=0, A1=1, A0=1 → `0x20|0x03`). Boot scans **0x20–0x27**. GP0–GP3 are active-low inputs with internal pull-ups enabled in firmware:

| GPIO | Button |
| --- | --- |
| GP0 | Down |
| GP1 | Menu |
| GP2 | Enter |
| GP3 | Up |

The OLED shows a 4-character line `BTN:____` (nothing pressed) or e.g. `BTN:_M__` (menu only) or `BTN:UDME` (all pressed). Display order is **U D M E** (not GPIO order). USB telemetry includes `MCP=____` on the same schedule.

MCP23008 **INT** is wired to **Arduino D2** (P5[0]). On any button change the firmware prints `BTN:____` immediately on USB serial (I2C read runs in the main loop, not inside the ISR). D2 is not included in the kit `GPIO=0x..` mask (that is D3–D9 only).

Config in `source/app_config.h`: `APP_ENABLE_MCP23008`, `APP_MCP23008_I2C_ADDR` (default `0x20`), `APP_MCP23008_INT_PIN`.

## Pin Map

| Function | Pin(s) |
| --- | --- |
| Debug UART | P3[1] TX, P3[0] RX |
| MIDI UART | P0[3] TX / Arduino D1, P0[2] RX / Arduino D0 |
| I2C | P6[4] SCL / Arduino D15, P6[5] SDA / Arduino D14 |
| GPIO inputs | D3-D9 (D2 = MCP23008 INT) |
| Header analog | A0 / P10[0], A1 / P10[1] |
| Onboard analog | Thermistor/reference on SAR channels 0/1, ALS on channel 2 |
| LEDs | CYBSP_USER_LED1 / CYBSP_USER_LED2 |

## CapSense Tuning

The CapSense widgets live in:

```text
bsps/TARGET_APP_CY8CKIT-062S4/config/design.cycapsense
```

Open that file with the CapSense Configurator when you want to change thresholds or tuning settings. Save it, then run `make build`; the generated CapSense source updates automatically.

`APP_ENABLE_CAPSENSE_TUNER` is disabled in `source/app_config.h` by default so the P6[4]/P6[5] I2C pins remain available as an I2C master bus. Enable it only when using the tuner, because the tuner uses the same KitProg/I2C pins.

## Device Configuration

Board pins, CapSense routing, and SAR ADC setup are in:

```text
bsps/TARGET_APP_CY8CKIT-062S4/config/design.modus
```

The SAR setup was merged from Infineon's low-power thermistor/ALS example and extended with A0/A1 header inputs. Runtime code uses simple periodic polling rather than the low-power FIFO wakeup flow.
