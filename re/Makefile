# AC1_RE behavioral reference models — build
CC      ?= cc
CFLAGS  ?= -std=c99 -O2 -Wall -Wextra -Iinclude
BUILD   := build

MODELS  := src/ac1_mxt.c
EXAMPLES:= mxt_dump

.PHONY: all clean test
all: $(addprefix $(BUILD)/,$(EXAMPLES))

$(BUILD)/mxt_dump: examples/mxt_dump.c $(MODELS) | $(BUILD)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD):
	mkdir -p $(BUILD)

# Validate the model against the extracted FDAT container, if present.
test: all
	@if [ -f ../fdat_extracted.T ]; then ./$(BUILD)/mxt_dump ../fdat_extracted.T; \
	else echo "skip: ../fdat_extracted.T not present"; fi

clean:
	rm -rf $(BUILD)
