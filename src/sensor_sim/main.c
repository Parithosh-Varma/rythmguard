// sensor_sim: 1 kHz synthetic pressure/flow + noise, UDP -> controller.
// QNX target: same loop driven by high-priority timer pulse (see QNX_SETUP).
// Host: nanosleep loop, best-effort.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "../common/pulseguard.h"
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static int64_t now_ns(void) {
    struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);
    return (int64_t)ts.tv_sec * 1000000000L + ts.tv_nsec;
}

int main(void) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) { perror("socket"); return 1; }
    struct sockaddr_in dst;
    memset(&dst, 0, sizeof(dst));
    dst.sin_family = AF_INET;
    dst.sin_port = htons(PG_SENSOR_UDP_PORT);
    dst.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    uint32_t seq = 0;
    printf("sensor_sim: streaming @1kHz -> 127.0.0.1:%d (pid %d)\n", PG_SENSOR_UDP_PORT, getpid());
    fflush(stdout);
    while (1) {
        int64_t t0 = now_ns();
        double t = seq * 0.001;
        // 12 breaths/min sine + noise: SIMULATION ONLY
        double noise_p = ((double)rand() / RAND_MAX - 0.5) * 0.6;
        double noise_f = ((double)rand() / RAND_MAX - 0.5) * 0.4;
        pg_sample_t s;
        s.seq = seq++;
        s.t_ns = t0;
        s.pressure = (float)(8.0 + 6.0 * sin(2 * M_PI * 0.2 * t) + noise_p);
        s.flow = (float)(30.0 * fmax(0.0, sin(2 * M_PI * 0.2 * t)) + noise_f);
        s.drop_test = 0;
        sendto(sock, &s, sizeof(s), 0, (struct sockaddr*)&dst, sizeof(dst));
        // pace to 1 kHz
        int64_t dt = now_ns() - t0;
        int64_t sleep_ns = PG_LOOP_PERIOD_NS - dt;
        if (sleep_ns > 0) {
            struct timespec rq = { sleep_ns / 1000000000L, sleep_ns % 1000000000L };
            nanosleep(&rq, NULL);
        }
    }
    return 0;
}
