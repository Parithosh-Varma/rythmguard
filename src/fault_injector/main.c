// fault_injector: chaos CLI. --kill sensor|controller | --cpu N --secs S | --drop
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

static void cpu_burn(int secs, int workers) {
    printf("fault_injector: CPU storm x%d for %ds\n", workers, secs);
    for (int i = 0; i < workers; i++) {
        if (fork() == 0) {
            time_t end = time(NULL) + secs;
            volatile double x = 0;
            while (time(NULL) < end) for (int k = 0; k < 100000; k++) x += k * 0.001;
            _exit(0);
        }
    }
    sleep(secs + 1);
    system("pkill -f fault_injector_burn 2>/dev/null; true");
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("usage:\n  fault_injector --kill sensor|controller\n"
               "  fault_injector --cpu N --secs S\n");
        return 1;
    }
    if (!strcmp(argv[1], "--kill")) {
        const char *t = argc > 2 ? argv[2] : "sensor_sim";
        const char *proc = strstr(t, "control") ? "controller" : "sensor_sim";
        char cmd[128]; snprintf(cmd, sizeof(cmd), "pkill -9 -x %s", proc);
        printf("fault_injector: killing %s\n", proc);
        FILE *f = fopen("state/fault.log", "a");
        if (f) { fprintf(f, "[inject] kill %s\n", proc); fclose(f); }
        return system(cmd);
    }
    if (!strcmp(argv[1], "--cpu")) {
        int n = argc > 2 ? atoi(argv[2]) : 4, s = 10;
        for (int i = 1; i + 1 < argc; i++)
            if (!strcmp(argv[i], "--secs")) s = atoi(argv[i + 1]);
        FILE *f = fopen("state/fault.log", "a");
        if (f) { fprintf(f, "[inject] cpu storm x%d %ds\n", n, s); fclose(f); }
        cpu_burn(s, n);
        return 0;
    }
    printf("unknown args\n"); return 1;
}
