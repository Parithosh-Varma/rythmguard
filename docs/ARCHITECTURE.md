# ARCHITECTURE

One process per job. A crash is isolated by the microkernel (QNX) / OS process boundary (host sim).

## Data flow

sensor_sim (1 kHz, sine 0.2 Hz + noise) —UDP:5001→ controller (PID @1 kHz)
controller —TCP:5002→ telemetry —HTTP:8080→ dashboard
controller → state/heartbeat (host) / MsgSend pulse (QNX)
watchdog → spawn/restart on stale heartbeat or missing pid

## Controller

- Setpoint 10 cmH₂O, PID 2.0 / 0.5 / 0.05, output clamped 0–100.
- Jitter = |now − expected_tick|. Deadline miss if tick > 2× period late.
- Stale sensor (>50 ms) → `safe_mode=1`, holds last output, still meets deadlines.
- QNX target: `SCHED_FIFO prio 20`, `TimerTimeout` on MsgReceive, priority inheritance on; optional priority-inversion demo: run a medium-prio CPU burner with/without inheritance and compare p95.

## Resource manager

- QNX: `/dev/pulseguard/{status,control,fault}` via resmgr (open/read/write), `cat /dev/pulseguard/status`.
- Host: `state/status` file + TCP :5002 (same JSON). `resmgr` binary mirrors it at 5 Hz.

## Watchdog

- Host: polls `state/heartbeat` + `pgrep`, restarts via `posix_spawn`, logs to `state/fault.log`, rate-limits to 1 restart / 3 s.
- QNX: channel + `MsgSend` pulse with 5 ms timeout, `spawn()` restart, same log format.

## Telemetry / dashboard

- `telemetry.py` polls :5002 @10 Hz, keeps 300-sample jitter window, serves `/api/stats`, `/api/log`.
- `dashboard/index.html`: waveform + histogram canvases, safe-mode lamp, fault-log tail. No build step.

## Timing claim

<100 µs jitter is a **QNX 8.0 target** claim under CPU storm. Host sim reports best-effort numbers only — see `scripts/measure-jitter.py` note. Video must label which is which.
