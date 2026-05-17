#!/usr/bin/env bash
set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

export MODUSTOOLBOX_HOME="${MODUSTOOLBOX_HOME:-$HOME/Applications/ModusToolbox}"
export CY_TOOLS_PATHS="${CY_TOOLS_PATHS:-$MODUSTOOLBOX_HOME/tools_3.8}"
export CY_COMPILER_GCC_ARM_DIR="${CY_COMPILER_GCC_ARM_DIR:-$HOME/Applications/mtb-gcc-arm-eabi/14.2.1/gcc}"
export PATH="$CY_TOOLS_PATHS/make/bin:$CY_TOOLS_PATHS/ninja:$CY_COMPILER_GCC_ARM_DIR/bin:$PATH"

exec make -C "$PROJECT_ROOT" "${@:-build}"
