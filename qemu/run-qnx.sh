#!/bin/bash
# run-qnx.sh — boot QNX 8.0 in QEMU and launch PulseGuard target stack.
# Fill IFS_QEMU_IMG after following docs/QNX_SETUP.md (SDP 8.0 + BSP).
set -e
cd "$(dirname "$0")/.."
: "${IFS_QEMU_IMG:=ifs-pulseguard.bin}"
if [ ! -f "qemu/$IFS_QEMU_IMG" ]; then
  echo "Missing qemu/$IFS_QEMU_IMG. Follow docs/QNX_SETUP.md to build the IFS,"
  echo "or run the host sim instead: ./scripts/run-host.sh"
  exit 1
fi
echo "Booting $IFS_QEMU_IMG … (SIMULATION ONLY)"
qemu-system-aarch64 -machine virt -cpu cortex-a72 -m 1024 \
  -kernel "qemu/$IFS_QEMU_IMG" -nographic \
  -netdev user,id=n0,hostfwd=tcp::8080-:8080 -device virtio-net-pci,netdev=n0
# on target shell:
#   cat /dev/pulseguard/status
#   pidin -p controller   # show SCHED_FIFO prio
