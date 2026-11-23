#!/usr/bin/env python3
"""measure-jitter.py — poll controller, report jitter percentiles. Host = best-effort."""
import json, socket, time, statistics
N = 500
jit = []
for _ in range(N):
    try:
        s = socket.create_connection(("127.0.0.1", 5002), timeout=1)
        st = json.loads(s.recv(4096).decode().split("\n")[0])
        s.close()
        jit.append(st.get("jitter_us_last", 0))
    except Exception as e:
        print("unreachable:", e); break
    time.sleep(0.01)
if jit:
    jit.sort()
    print(f"n={len(jit)} p50={statistics.median(jit):.1f}us "
          f"p95={jit[int(len(jit)*0.95)]:.1f}us max={max(jit):.1f}us")
    print("NOTE: host numbers are best-effort; QNX 8.0 target is where <100us is claimed.")
