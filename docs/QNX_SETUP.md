# QNX_SETUP (SDP 8.0 + QEMU)

SDP is Linux/Windows-hosted — do this on Linux (VM OK). Budget extra time for first boot.

1. Install QNX SDP 8.0, source env: `source ~/qnx800/qnxsdp-env.sh`
   Verify: `echo $QNX_HOST $QNX_TARGET; which qcc`
2. Host sanity: `qcc -V gcc_ntoaarch64le --version`
3. Target build: `./qemu/build-qnx.sh` → `build-qnx/` aarch64 binaries.
4. IFS image: in SDP, create `qemu/pulseguard.build` (x86_64/aarch64 virt BSP),
   add lines: `[data=c] build-qnx/controller=/proc/boot/controller` (×5),
   `mkifs qemu/pulseguard.build qemu/ifs-pulseguard.bin`.
5. Boot: `./qemu/run-qnx.sh`. On target: `ls /dev/pulseguard/`, `cat /dev/pulseguard/status`.
6. Publish proof for judges: commit `qemu/*.build`, `qemu/run-qnx.sh`, toolchain version
   (`qcc -V` output), target (`aarch64le`), and a boot log in `docs/`.

Troubleshooting: QEMU `-nographic` needs virtio-net for telemetry fwd;
if no net, use serial to dump `state/status` instead.
