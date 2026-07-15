#!/usr/bin/env bash
# Standalone regression runner for vspa-lib submodule kernels.
# No Docker, no superproject required — uses tools/run_sim.py as simulator backend.
#
# Usage:  ./tools/regression_cycles_all.sh [AU]
#         AU defaults to 16
set -euo pipefail

TOOLS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
VSPA_LIB="$TOOLS_DIR/../vspa-lib"

# Resolve VSPA toolchain: env var > /opt default
if [[ -z "${VSPA_TOOL:-}" ]]; then
  if [[ -x "/opt/VSPA_Tools_vbeta_14_00_781/bin/fsvspacc" ]]; then
    VSPA_TOOL="/opt/VSPA_Tools_vbeta_14_00_781"
  else
    echo "ERROR: VSPA_TOOL is not set and /opt/VSPA_Tools_vbeta_14_00_781 not found."
    echo "       Set VSPA_TOOL to your toolchain root, e.g.:"
    echo "         VSPA_TOOL=/path/to/VSPA_Tools ./tools/regression_cycles_all.sh"
    exit 2
  fi
fi
export VSPA_TOOL
echo "VSPA_TOOL: $VSPA_TOOL"

# Resolve VSPA simulator: env var > /opt default. Fail fast here so the
# per-kernel loop below does not repeat the same guidance for every kernel.
SIMULATOR_PATH="${SIMULATOR_PATH:-/opt/ccssim2/bin/runsim}"
if [[ ! -x "$SIMULATOR_PATH" ]]; then
    echo "ERROR: SIMULATOR_PATH is not set and /opt/ccssim2/bin/runsim not found:"; 
    echo "  Set SIMULATOR_PATH to your  simulator executable, e.g.:";
    echo "  SIMULATOR_PATH=/path/to/ccssim2/bin/runsim"; 
    exit 2
fi
export SIMULATOR_PATH
echo "SIMULATOR_PATH: $SIMULATOR_PATH"


AU="${1:-16}"
DATE_TAG="$(date +%Y%m%d_%H%M%S)"
REPORT_DIR="${REPORT_DIR:-$TOOLS_DIR/../build/reports}"
REPORT_CSV="$REPORT_DIR/regression_${DATE_TAG}.csv"

mkdir -p "$REPORT_DIR"

mapfile -t kernels < <(
  find "$VSPA_LIB" -mindepth 3 -maxdepth 3 -name "Makefile" \
    | grep "/tests/Makefile$" \
    | sed 's|.*/vspa-lib/||; s|/tests/Makefile||' \
    | sort
)

if [[ ${#kernels[@]} -eq 0 ]]; then
  echo "ERROR: no vspa-lib/<kernel>/tests/Makefile found under $VSPA_LIB"
  exit 2
fi

echo "kernel,status,cycles" > "$REPORT_CSV"

pass_count=0
fail_count=0

for k in "${kernels[@]}"; do
  test_dir="$VSPA_LIB/$k/tests"
  build_dir="$REPORT_DIR/../build/$k/tests"
  mkdir -p "$build_dir"
  echo "===== $k ====="

  # gen_vectors step (only if the target exists)
  if make -C "$test_dir" AU="$AU" VSPA_TOOL="$VSPA_TOOL" BUILD_DIR="$build_dir" generate -n >/dev/null 2>&1; then
    make -C "$test_dir" AU="$AU" VSPA_TOOL="$VSPA_TOOL" BUILD_DIR="$build_dir" generate >/dev/null 2>&1 || true
  fi

  # build + test with cycle counting
  out="$(make -C "$test_dir" AU="$AU" VSPA_TOOL="$VSPA_TOOL" BUILD_DIR="$build_dir" CYCLES=--cycles test 2>&1 || true)"

  if echo "$out" | grep -q 'RESULT : PASS'; then
    cycles=$(echo "$out" | grep -oP '\(\K[0-9]+(?= cycles)')
    cycle_str="${cycles:-(n/a)}"
    echo "  PASS  ($cycle_str cycles)"
    echo "$k,PASS,$cycle_str" >> "$REPORT_CSV"
    pass_count=$((pass_count + 1))
  else
    echo "  FAIL"
    echo "$out" | tail -n 15
    echo "$k,FAIL," >> "$REPORT_CSV"
    fail_count=$((fail_count + 1))
  fi

  echo
done

echo "========================================"
echo "Report : $REPORT_CSV"
echo "PASS   : $pass_count"
echo "FAIL   : $fail_count"
echo "========================================"

if [[ $fail_count -gt 0 ]]; then
  exit 1
fi
