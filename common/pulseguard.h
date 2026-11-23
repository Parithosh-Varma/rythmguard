#pragma once
// Shared PulseGuard protocol. Portable C (host + QNX).
#include <stdint.h>

#define PG_SENSOR_UDP_PORT      5001
#define PG_CONTROLLER_TCP_PORT  5002
#define PG_TELEMETRY_HTTP_PORT  8080

#define PG_LOOP_HZ              1000
#define PG_LOOP_PERIOD_NS       1000000L
#define PG_JITTER_TARGET_US     100.0

// sensor -> controller datagram (also QNX pulse payload on target)
typedef struct {
    uint32_t seq;
    int64_t  t_ns;        // sender timestamp (CLOCK_MONOTONIC)
    float    pressure;    // cmH2O synthetic
    float    flow;        // L/min synthetic
    uint32_t drop_test;   // set by fault_injector to simulate drops
} pg_sample_t;

// controller status snapshot (TCP :5002, one JSON line per poll)
typedef struct {
    uint32_t seq;
    uint32_t deadline_misses;
    double   jitter_us_last;
    double   jitter_us_max;
    double   control_out;
    int      safe_mode;   // 1 when sensor stale
    int64_t  t_ns;
} pg_status_t;
