#!/bin/bash
# build-qnx.sh — cross-build for QNX 8.0 (needs SDP sourced: $QNX_HOST, $QNX_TARGET, qcc)
set -e
cd "$(dirname "$0")/.."
: "${QNX_HOST:?source qnxsdp-env.sh first}" "${QNX_TARGET:?source qnxsdp-env.sh first}"
mkdir -p build-qnx
for p in sensor_sim controller resmgr watchdog fault_injector; do
  echo "== $p"
  qcc -V gcc_ntoaarch64le -O2 -Wall -Icommon -o build-qnx/$p src/$p/main.c -lm
done
echo "QNX aarch64 binaries in build-qnx/ — pack into IFS, see run-qnx.sh"
