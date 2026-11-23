#!/bin/bash
# run-host.sh — build + launch full host sim
set -e
cd "$(dirname "$0")/.."
mkdir -p state build
make -s
pkill -x sensor_sim 2>/dev/null; pkill -x controller 2>/dev/null
pkill -x watchdog 2>/dev/null; pkill -x resmgr 2>/dev/null
pkill -f telemetry.py 2>/dev/null; true
rm -f state/heartbeat
echo "--- starting PulseGuard host sim (SIMULATION ONLY) ---"
./build/controller & echo "controller $!"
sleep 0.3
./build/sensor_sim & echo "sensor $!"
./build/resmgr &
./build/watchdog &
python3 src/telemetry/telemetry.py &
sleep 1
echo "--- up: dashboard/index.html + http://localhost:8080/api/stats ---"
echo "--- try: ./build/fault_injector --kill sensor | --cpu 4 --secs 10 ---"
echo "--- status: cat state/status ; curl -s localhost:8080/api/stats ---"
wait
