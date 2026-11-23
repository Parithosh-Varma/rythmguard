// watchdog: heartbeat monitor + restart. QNX: spawn() + MsgSend pulse.
// Host: watches state/heartbeat mtime, restarts ./build/<proc> via fork/exec.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <signal.h>
#include <spawn.h>
extern char **environ;

static int64_t now_ns(void) {
    struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);
    return (int64_t)ts.tv_sec * 1000000000L + ts.tv_nsec;
}

static void log_fault(const char *msg) {
    FILE *f = fopen("state/fault.log", "a");
    if (f) {
        time_t t = time(NULL);
        char ts[64]; strftime(ts, sizeof(ts), "%F %T", localtime(&t));
        fprintf(f, "[%s] %s\n", ts, msg);
        fclose(f);
    }
    printf("watchdog: %s\n", msg); fflush(stdout);
}

static void start_proc(const char *name) {
    pid_t pid;
    char path[256]; snprintf(path, sizeof(path), "./build/%s", name);
    char *argv[] = { (char*)path, NULL };
    if (posix_spawn(&pid, path, NULL, NULL, argv, environ) == 0) {
        char m[256]; snprintf(m, sizeof(m), "restarted %s (pid %d)", name, pid);
        log_fault(m);
    } else {
        char m[256]; snprintf(m, sizeof(m), "FAILED to restart %s", name);
        log_fault(m);
    }
}

int main(void) {
    printf("watchdog: monitoring heartbeat + procs (pid %d)\n", getpid());
    fflush(stdout);
    log_fault("watchdog online");
    const char *procs[] = { "sensor_sim", "controller", NULL };
    while (1) {
        struct stat st;
        int stale = 1;
        if (stat("state/heartbeat", &st) == 0) {
            int64_t hb = 0;
            FILE *f = fopen("state/heartbeat", "r");
            if (f) { fscanf(f, "%lld", (long long*)&hb); fclose(f); }
            // stale if heartbeat older than 200ms (wall-clock approx via mtime)
            time_t now = time(NULL);
            stale = (now - st.st_mtime > 2); // generous on host; target uses 5ms MsgSend timeout
            (void)hb;
        }
        if (stale) {
            static time_t last = 0; time_t n = time(NULL);
            if (n - last > 3) { // rate-limit restarts
                last = n;
                log_fault("ALERT: heartbeat stale — controller safe-mode, restarting sensor_sim");
                start_proc("sensor_sim");
            }
        }
        // also revive missing binaries (killed via fault_injector)
        for (int i = 0; procs[i]; i++) {
            char cmd[256];
            snprintf(cmd, sizeof(cmd), "pgrep -x %s >/dev/null 2>&1", procs[i]);
            if (system(cmd) != 0) {
                char m[256]; snprintf(m, sizeof(m), "missing %s — restarting", procs[i]);
                log_fault(m);
                start_proc(procs[i]);
            }
        }
        sleep(1);
    }
    return 0;
}
