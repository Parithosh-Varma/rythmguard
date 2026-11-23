// controller: 1 kHz PID loop, jitter + deadline stats, TCP status :5002.
// QNX target: SCHED_FIFO prio 20, MsgReceive pulses, TimerTimeout deadline.
// Host: UDP recv (nonblocking poll) + nanosleep pacing, heartbeat file.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "../common/pulseguard.h"
#ifdef __QNX__
#include <sched.h>
#endif
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static int64_t now_ns(void) {
    struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);
    return (int64_t)ts.tv_sec * 1000000000L + ts.tv_nsec;
}

int main(void) {
#ifdef __QNX__
    // Target: request FIFO priority (needs root/prot). Best-effort fallback.
    struct sched_param sp; sp.sched_priority = 20;
    sched_setscheduler(0, SCHED_FIFO, &sp);
#endif
    int udp = socket(AF_INET, SOCK_DGRAM, 0);
    int tcp = socket(AF_INET, SOCK_STREAM, 0);
    if (udp < 0 || tcp < 0) { perror("socket"); return 1; }
    int opt = 1;
    setsockopt(udp, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    setsockopt(tcp, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in a; memset(&a, 0, sizeof(a));
    a.sin_family = AF_INET; a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    a.sin_port = htons(PG_SENSOR_UDP_PORT);
    if (bind(udp, (struct sockaddr*)&a, sizeof(a)) < 0) { perror("bind udp"); return 1; }
    fcntl(udp, F_SETFL, O_NONBLOCK);
    a.sin_port = htons(PG_CONTROLLER_TCP_PORT);
    if (bind(tcp, (struct sockaddr*)&a, sizeof(a)) < 0) { perror("bind tcp"); return 1; }
    listen(tcp, 4);
    fcntl(tcp, F_SETFL, O_NONBLOCK);

    // PID state (SIMULATION): regulate pressure to 10 cmH2O
    double setpoint = 10.0, integ = 0, prev_err = 0;
    double out = 0, jitter_last = 0, jitter_max = 0;
    uint32_t seq = 0, misses = 0, last_sensor_seq = 0;
    int64_t last_sensor_ns = 0, expected_ns = now_ns();
    float last_p = 0, last_f = 0;

    printf("controller: 1kHz PID, udp :%d tcp :%d (pid %d)\n",
        PG_SENSOR_UDP_PORT, PG_CONTROLLER_TCP_PORT, getpid());
    fflush(stdout);

    while (1) {
        int64_t t0 = now_ns();
        // drain sensor datagrams (keep newest)
        pg_sample_t s; int got = 0;
        while (recv(udp, &s, sizeof(s), 0) == (ssize_t)sizeof(s)) {
            got = 1; last_sensor_seq = s.seq; last_sensor_ns = s.t_ns;
            last_p = s.pressure; last_f = s.flow;
        }
        int stale = (t0 - last_sensor_ns > 50 * 1000000L); // 50ms -> safe mode
        double meas = got || !stale ? last_p : setpoint;
        double err = setpoint - meas;
        integ += err * 0.001;
        double deriv = (err - prev_err) / 0.001;
        prev_err = err;
        out = 2.0 * err + 0.5 * integ + 0.05 * deriv;
        if (out < 0) out = 0; if (out > 100) out = 100;

        // jitter vs ideal schedule
        double jitter_us = fabs((double)(t0 - expected_ns) / 1000.0);
        jitter_last = jitter_us;
        if (jitter_us > jitter_max) jitter_max = jitter_us;
        if (t0 - expected_ns > 2 * PG_LOOP_PERIOD_NS) misses++;
        expected_ns += PG_LOOP_PERIOD_NS;
        if (expected_ns < t0 - 10 * PG_LOOP_PERIOD_NS) expected_ns = t0;

        // heartbeat for watchdog (decimated: every 20 ticks, host file IO is slow)
        if ((seq % 20) == 0) {
            FILE *hb = fopen("state/heartbeat", "w");
            if (hb) { fprintf(hb, "%lld\n", (long long)t0); fclose(hb); }
        }

        // serve one status poller per tick (nonblocking accept)
        int c = accept(tcp, NULL, NULL);
        if (c >= 0) {
            char buf[512];
            int n = snprintf(buf, sizeof(buf),
                "{\"seq\":%u,\"deadline_misses\":%u,\"jitter_us_last\":%.1f,"
                "\"jitter_us_max\":%.1f,\"control_out\":%.2f,\"safe_mode\":%d,"
                "\"pressure\":%.2f,\"flow\":%.2f,\"t_ns\":%lld}\n",
                seq, misses, jitter_last, jitter_max, out, stale ? 1 : 0,
                last_p, last_f, (long long)t0);
            send(c, buf, n, 0);
            close(c);
            // host analogue of /dev/pulseguard/status
            FILE *st = fopen("state/status", "w");
            if (st) { fwrite(buf, 1, n, st); fclose(st); }
        }

        seq++;
        int64_t dt = now_ns() - t0;
        int64_t sleep_ns = PG_LOOP_PERIOD_NS - dt;
        if (sleep_ns > 0) {
            struct timespec rq = { sleep_ns / 1000000000L, sleep_ns % 1000000000L };
            nanosleep(&rq, NULL);
        }
    }
    return 0;
}
