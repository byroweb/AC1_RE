# tools/link/unit.mk: assemble and link one split code unit.
# The top-level Makefile runs it with -C build/link/<unit> UNIT=<name>, after splat has
# written asm/, bin/, include/, <name>.ld and the undefined_*_auto.txt symbol files there.

CROSS   ?= mipsel-none-elf-
ASFLAGS := -EL -march=r3000 -mtune=r3000 -G0 -no-pad-sections -I include

S_FILES := $(wildcard asm/*.s asm/data/*.s)
O_FILES := $(S_FILES:%=obj/%.o)

$(UNIT).elf: $(O_FILES) $(UNIT).ld undefined_syms_auto.txt undefined_funcs_auto.txt
	$(CROSS)ld -nostdlib -T undefined_syms_auto.txt -T undefined_funcs_auto.txt -T $(UNIT).ld \
		-Map $(UNIT).map -o $@

obj/%.s.o: %.s
	@mkdir -p $(dir $@)
	@$(CROSS)as $(ASFLAGS) -o $@ $<
