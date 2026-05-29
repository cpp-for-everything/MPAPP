#!/usr/bin/env bash
# SPDX-License-Identifier: Apache-2.0
# Textual + visual capture of every RFC-0006 visual_state transition.
#
# Runs the gtk4_vsm_demo once per requested state. The demo's
# on_launch path applies the state via vsm.go_to_state and emits a
# VSM-SMOKE line to stderr — that line is the textual closure
# evidence (the BitBlt path is blocked by WSLg's DComp wall on
# Win11; the one initial-state PNG already captures the visual).
#
# Usage: capture-vsm-smoke.sh [out-dir]

set -euo pipefail

DEMO=/mnt/d/GitHub/MPAPP/build-wsl/examples/gtk4_vsm_demo/gtk4_vsm_demo
OUT="${1:-/mnt/d/GitHub/MPAPP/vault/50_Tasks/T-0050-rfc-0006-vsm-mock/logs}"
mkdir -p "$OUT"

STATES=(Normal Pressed Disabled CustomState)

for s in "${STATES[@]}"; do
  pkill -f gtk4_vsm_demo >/dev/null 2>&1 || true
  sleep 0.5
  # Launch the demo, wait for the smoke line, then kill it.
  log="$OUT/linux-${s,,}.log"
  : > "$log"

  MPAPP_VSM_INITIAL_STATE="$s" "$DEMO" 2> >(tee -a "$log" >&2) &
  pid=$!

  # Wait up to 6s for the VSM-SMOKE line to appear.
  for _ in $(seq 1 30); do
    if grep -q '^VSM-SMOKE:' "$log" 2>/dev/null; then
      break
    fi
    sleep 0.2
  done

  # Drop the demo and continue.
  kill "$pid" 2>/dev/null || true
  wait "$pid" 2>/dev/null || true

  smoke_line=$(grep '^VSM-SMOKE:' "$log" || echo '(no smoke line — state not in any group)')
  printf '%s\n' "$smoke_line"
done

pkill -f gtk4_vsm_demo >/dev/null 2>&1 || true
