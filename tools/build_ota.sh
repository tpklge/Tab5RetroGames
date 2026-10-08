#!/usr/bin/env bash
set -euo pipefail
ARCADE_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ARCADE_ROOT"
if ! command -v idf.py >/dev/null 2>&1; then
    ARCADE_IDF="${IDF_PATH:-$HOME/esp/esp-idf}"
    if [[ -x "$HOME/.espressif/python_env/idf5.4_py3.9_env/bin/python" ]]; then
        source "$HOME/.espressif/python_env/idf5.4_py3.9_env/bin/activate"
    fi
    source "$ARCADE_IDF/export.sh"
fi
export IDF_COMPONENT_CHECK_NEW_VERSION=0
export IDF_TARGET=esp32p4
idf.py build
python tools/package.py
