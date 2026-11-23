# PulseGuard (rythmguard) — Self-Healing Real-Time Medical Controller

> **SIMULATION ONLY — NOT A MEDICAL DEVICE.** All waveforms are synthetic. No clinical use.

Hackathon: **QNX Global Hackathon** (deadline Dec 15, 2026, 2pm ET)
Category: **Robotics & Medical** + bonus prize: **Best Use of QNX-specific Technologies**

## The pitch

A simulated ventilator / infusion-pump controller running on **QNX 8.0 in QEMU**.
It keeps hard deadlines even when parts crash — kill components live while the
dashboard proves timing never breaks. The same fault on a monolithic stack
takes the whole controller down.

## Architecture — one process per job, crash isolation

| Process | Job | QNX feature (target) | Host-sim IPC |
|---|---|---|---|
| `sensor_sim` | pressure/flow synth @ 1 kHz + noise | Timers, hi-prio thread | UDP → controller :5001 |
| `controller` | PID loop @ 1 kHz, jitter + deadline stats | SCHED_FIFO, deadline monitor | UDP in, TCP status :5002 |
| `resmgr` | `/dev/pulseguard/status` as files | Resource manager | status file `state/status` |
| `watchdog` | heartbeat monitor + restart | spawn, MsgSend | heartbeat file + `posix_spawn` |
| `fault_injector` | kill procs, CPU spike, packet drop | Chaos test | CLI (`kill`, burn loop) |
| `telemetry` | stream latency/jitter | QNX networking | HTTP :8080 + JSON |
| dashboard | waveform + histogram + fault log | host tool | `dashboard/index.html` |

```
sensor_sim --UDP:5001--> controller --TCP:5002--> telemetry --HTTP:8080--> dashboard
                            ^                        |
                      heartbeat file                 v
                            watchdog ---- restart ---+
```

Why QNX (judging criterion: "why a real-time foundation like QNX is the right choice"):
- **Microkernel isolation:** controller survives telemetry/sensor crash.
- **Message passing + priority inheritance:** demo priority-inversion bug + fix side by side.
- **Deterministic scheduling:** jitter target < 100 µs under CPU storm (QNX target; host sim best-effort).
- **Resource manager:** `cat /dev/pulseguard/status` on target, `cat state/status` on host.

## Quick start (host sim, no QNX needed)

```bash
make
./scripts/run-host.sh
# dashboard: open dashboard/index.html, or http://localhost:8080/api/stats
```

Demo in 4 commands (second terminal):

```bash
# 1. CPU storm — jitter stays flat (QNX target; host shows containment)
./build/fault_injector --cpu 4 --secs 10
# 2. Kill sensor — watchdog restarts in ms, controller enters safe mode
./build/fault_injector --kill sensor
# 3. Check status (host analogue of cat /dev/pulseguard/status)
cat state/status
curl -s localhost:8080/api/stats
# 4. Fault log + deadline misses (must stay 0 on QNX)
cat state/fault.log
```

QNX 8.0 target build:

```bash
cat docs/QNX_SETUP.md
./qemu/build-qnx.sh     # needs QNX SDP 8.0 sourced
./qemu/run-qnx.sh       # boots QEMU, runs target binaries
# on target: cat /dev/pulseguard/status
```

## 3-minute video script

See `docs/DEMO_SCRIPT.md`. Beats: (1) stable waveform, low jitter →
(2) CPU stress, jitter flat → (3) kill sensor, watchdog restart + safe mode →
(4) fault log, deadline-miss count 0 → (5, optional) Linux contrast, jitter spikes.

## Layout

```
common/          shared protocol structs + ports
src/sensor_sim/  1 kHz synthetic sensor
src/controller/  1 kHz PID + jitter/deadline stats
src/resmgr/      QNX resmgr (target) / status-file mirror (host)
src/watchdog/    heartbeat monitor + restart
src/fault_injector/ chaos CLI
src/telemetry/   HTTP + JSON bridge for dashboard
dashboard/       live waveform + histogram + fault log
scripts/         run-host, stress helpers, jitter measure
qemu/            QNX build + QEMU boot scripts
docs/            ARCHITECTURE, DEMO_SCRIPT, QNX_SETUP
state/           runtime (gitignored): status, heartbeat, fault.log
```

## Timeline (to Dec 15)

- Days 1–3: SDP 8.0 + QEMU boot, hello world (`docs/QNX_SETUP.md`)
- Days 4–9: sensor, controller, resmgr
- Days 10–13: watchdog, fault injector, telemetry
- Days 14–17: dashboard + README/build docs
- Days 18–21: video + polish

## License

Apache-2.0 — see `LICENSE`. Repo must stay public with license + QNX 8.0
buildfiles + QEMU run script for judging.
