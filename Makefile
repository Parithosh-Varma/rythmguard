CC ?= cc
CFLAGS ?= -O2 -Wall -Wextra -std=c11 -Icommon
BUILD = build
STATE = state

BINS = $(BUILD)/sensor_sim $(BUILD)/controller $(BUILD)/watchdog $(BUILD)/fault_injector $(BUILD)/resmgr

all: $(BINS)

$(BUILD):
	mkdir -p $(BUILD) $(STATE)

$(BUILD)/sensor_sim: src/sensor_sim/main.c common/pulseguard.h | $(BUILD)
	$(CC) $(CFLAGS) -o $@ src/sensor_sim/main.c -lm

$(BUILD)/controller: src/controller/main.c common/pulseguard.h | $(BUILD)
	$(CC) $(CFLAGS) -o $@ src/controller/main.c -lm

$(BUILD)/watchdog: src/watchdog/main.c common/pulseguard.h | $(BUILD)
	$(CC) $(CFLAGS) -o $@ src/watchdog/main.c

$(BUILD)/fault_injector: src/fault_injector/main.c common/pulseguard.h | $(BUILD)
	$(CC) $(CFLAGS) -o $@ src/fault_injector/main.c

$(BUILD)/resmgr: src/resmgr/main.c common/pulseguard.h | $(BUILD)
	$(CC) $(CFLAGS) -o $@ src/resmgr/main.c

clean:
	rm -rf $(BUILD)

.PHONY: all clean
