# Full-link build of the game's code units: docs/FULL_LINK_PLAN.md, docs/FULL_LINK_P1.md.
#
#   make            build every unit the link covers so far (P1: the resident exe)
#   make verify     build, then check each unit's SHA-1 against overlays/CODE_UNITS.tsv
#
# Needs AC1_BIN (your own disc image), a mipsel binutils on PATH (CROSS), and splat64
# (pip install -r tools/link/requirements.txt; point SPLAT at that Python if it is a venv).

PYTHON ?= python
SPLAT  ?= $(PYTHON) -m splat
CROSS  ?= mipsel-none-elf-

LINK := build/link
EXE  := $(LINK)/exe

# The start-up code (__SN_ENTRY_POINT) zeroes [EXE_DATA_END, EXE_BSS_END).
EXE_DATA_END := 0x80039CB8
EXE_BSS_END  := 0x8004ADA0

.PHONY: all exe verify
all: exe

$(EXE)/exe.yaml: tools/link/gen_exe.py overlays/CODE_UNITS.tsv tools/loop/functions.tsv
	$(PYTHON) tools/link/gen_exe.py --out $(EXE)

$(EXE)/exe.ld: $(EXE)/exe.yaml
	cd $(EXE) && $(SPLAT) split exe.yaml

exe: $(EXE)/exe.ld
	$(MAKE) --no-print-directory -C $(EXE) -f $(CURDIR)/tools/link/unit.mk UNIT=exe CROSS=$(CROSS)
	$(CROSS)objcopy -O binary $(EXE)/exe.elf $(EXE)/exe.img
	$(PYTHON) tools/link/mkpsexe.py $(EXE)/exe.elf $(EXE)/exe.img $(EXE)/SLUS_013.23 \
		--data-end $(EXE_DATA_END) --bss-end $(EXE_BSS_END)

verify: exe
	$(PYTHON) tools/link/verify.py SCUS_941.82=$(EXE)/SLUS_013.23
