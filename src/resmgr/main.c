// resmgr host mirror: keeps state/status fresh for `cat state/status`.
// On QNX 8.0 target this is a real resource manager at /dev/pulseguard/status
// (see docs/ARCHITECTURE.md). Host: polls controller :5002 at 5 Hz.
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "../common/pulseguard.h"

int main(void) {
    printf("resmgr(host): mirroring controller :5002 -> state/status (pid %d)\n", getpid());
    printf("QNX target: registers /dev/pulseguard/status (see docs/ARCHITECTURE.md)\n");
    fflush(stdout);
    while (1) {
        int s = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in a; memset(&a, 0, sizeof(a));
        a.sin_family = AF_INET; a.sin_port = htons(PG_CONTROLLER_TCP_PORT);
        a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        if (connect(s, (struct sockaddr*)&a, sizeof(a)) == 0) {
            char buf[1024]; int n = recv(s, buf, sizeof(buf) - 1, 0);
            if (n > 0) {
                buf[n] = 0;
                FILE *f = fopen("state/status", "w");
                if (f) { fwrite(buf, 1, n, f); fclose(f); }
            }
        }
        close(s);
        usleep(200000);
    }
    return 0;
}
