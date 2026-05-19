#!/usr/bin/env bash
set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

export MODUSTOOLBOX_HOME="${MODUSTOOLBOX_HOME:-$HOME/Applications/ModusToolbox}"
export CY_TOOLS_PATHS="${CY_TOOLS_PATHS:-$MODUSTOOLBOX_HOME/tools_3.8}"
export PATH="$CY_TOOLS_PATHS/make/bin:$PATH"

exec make -C "$PROJECT_ROOT" deps
