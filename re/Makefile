# AC1_RE behavioral reference models — build
CC      ?= cc
CFLAGS  ?= -std=c99 -O2 -Wall -Wextra -Iinclude
BUILD   := build

MODELS  := src/ac1_mxt.c src/ac1_mission.c
EXAMPLES:= mxt_dump mission_sim

.PHONY: all clean test
all: $(addprefix $(BUILD)/,$(EXAMPLES))

$(BUILD)/mxt_dump: examples/mxt_dump.c $(MODELS) | $(BUILD)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD)/mission_sim: examples/mission_sim.c $(MODELS) | $(BUILD)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD):
	mkdir -p $(BUILD)

# Validate the model against the extracted FDAT container, if present.
test: all
	@if [ -f ../fdat_extracted.T ]; then ./$(BUILD)/mxt_dump ../fdat_extracted.T; \
	else echo "skip: ../fdat_extracted.T not present"; fi
	@if [ -f ../fdat_extracted.T ]; then ./$(BUILD)/mission_sim ../fdat_extracted.T; \
	else ./$(BUILD)/mission_sim; fi

clean:
	rm -rf $(BUILD)
