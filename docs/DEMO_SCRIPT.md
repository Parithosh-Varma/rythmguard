# DEMO_SCRIPT (3 min) — record in one take, overlay timestamps

0:00–0:30 — Stable patient waveform, jitter low.
 Show dashboard + `curl localhost:8080/api/stats`. Say: "simulation only".

0:30–1:10 — CPU stress. Jitter stays flat.
 Terminal 2: `./build/fault_injector --cpu 4 --secs 20`.
 Narrate QNX SCHED_FIFO vs monolithic.

1:10–2:10 — Kill the sensor. Watchdog restarts in ms, safe mode.
 Terminal 2: `./build/fault_injector --kill sensor`.
 Show dashboard SAFE MODE lamp, then recovery. `cat state/fault.log`.

2:10–2:50 — Fault log + deadline-miss count: 0.
 `cat state/status`, `curl -s localhost:8080/api/stats`.
 On QNX target: `cat /dev/pulseguard/status`, `pidin`.

2:50–3:00 — (Optional) Linux contrast: same kill, jitter spikes / process stays dead.

B-roll checklist: waveform close-up, kill command + log timestamps side by side, status JSON with misses:0.
