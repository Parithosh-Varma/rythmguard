#!/usr/bin/env python3
"""telemetry: poll controller :5002 at 10 Hz, keep rolling stats, serve HTTP :8080.
Endpoints: /api/stats (latest + jitter histogram), /api/log (fault.log tail).
Dashboard polls these. SIMULATION ONLY."""
import json, socket, time, threading, os
from http.server import BaseHTTPRequestHandler, HTTPServer

HOST = "127.0.0.1"
CTL_PORT = 5002
HTTP_PORT = 8080
STATE_DIR = "state"

latest = {"note": "waiting for controller…"}
history = []  # last 300 jitter samples

def poll_controller():
    global latest
    while True:
        try:
            s = socket.create_connection((HOST, CTL_PORT), timeout=1)
            data = s.recv(4096).decode()
            s.close()
            st = json.loads(data.strip().split("\n")[0])
            latest = st
            history.append(st.get("jitter_us_last", 0))
            del history[:-300]
            # mirror status file (same as `cat /dev/pulseguard/status` on target)
            os.makedirs(STATE_DIR, exist_ok=True)
            open(f"{STATE_DIR}/status", "w").write(json.dumps(st) + "\n")
        except Exception as e:
            latest = {"error": f"controller unreachable: {e}", "safe_mode": 1,
                      "deadline_misses": latest.get("deadline_misses", 0)}
        time.sleep(0.1)

class H(BaseHTTPRequestHandler):
    def log_message(self, *a): pass
    def end_headers(self):
        self.send_header("Access-Control-Allow-Origin", "*")
        super().end_headers()
    def do_GET(self):
        if self.path == "/api/stats":
            body = json.dumps({"latest": latest, "jitter_hist": history[-120:]}).encode()
            self.send_response(200); self.send_header("Content-Type", "application/json")
            self.send_header("Content-Length", str(len(body))); self.end_headers()
            self.wfile.write(body)
        elif self.path == "/api/log":
            try: log = open(f"{STATE_DIR}/fault.log").read().splitlines()[-50:]
            except FileNotFoundError: log = []
            body = json.dumps(log).encode()
            self.send_response(200); self.send_header("Content-Type", "application/json")
            self.send_header("Content-Length", str(len(body))); self.end_headers()
            self.wfile.write(body)
        else:
            self.send_response(404); self.end_headers()

threading.Thread(target=poll_controller, daemon=True).start()
print(f"telemetry: controller :{CTL_PORT} -> http :{HTTP_PORT} (pid {os.getpid()})", flush=True)
HTTPServer((HOST, HTTP_PORT), H).serve_forever()
