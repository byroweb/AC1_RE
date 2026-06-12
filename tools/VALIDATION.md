# Tool belt validation report (2026-06-07)

Goal: confirm the new RE tool belt — chiefly `overlay2c.sh` (objdump + m2c
asm→C) — actually works, by re-deriving routines that were previously reverse
engineered **by hand** (live DuckStation disassembly) and checking the output
against the documented findings.

## Method

`tools/ghidra/overlay2c.sh <addr> <len>` carves the byte range out of
`overlay_80050000.bin` (base `0x80050000`), disassembles it with
`mipsel-linux-gnu-objdump`, converts to m2c GAS asm via `objdump2m2c.py`, and
decompiles with m2c. Each target below has an independently-known behaviour from
prior sessions (see `project_ac1_name_rtl_layout`, `farsi_name_input.c`,
`farsi_name_rtl.c`).

## Results — 3/3 reproduced

| Target | Addr | Expected (prior hand-RE) | overlay2c / m2c output | Verdict |
|--------|------|--------------------------|------------------------|---------|
| cursor.update (original) | `0x80083a28` | `cursor_X = n*16 + 8`, clamp to `7*16+8` at n==8; reads n via `0x7c(a0)->0x96` | `temp_a1 = arg0->unk7C->unk96; var_v0 = temp_a1*0x10; if(n==8) var_v0 = 7*0x10; arg0->unk48 = var_v0+8` | ✅ exact |
| name-input handler | `0x80082190` | `farsi_name_input.c`: edge-detect, `SELTAB[row*17+col]` @`0x8004c700`, append to `LOGBUF` @`0x8004c750`, reverse-copy to name buf, END→`draw_confirm(0x8005d8d4)` + state `0x8008240c` | identical structure (mode `unk19!=0x41`, SELTAB index `(unk1F*0x11)+unk1E+0x80050000-0x3900`, LOGBUF reverse, END branch) | ✅ exact |
| box_init (front) | `0x8005b648` | element alloc via `0x800501bc`; update method at `+0x74`; type at `+0xa` | `temp_v0 = func_800501bc(arg4); temp_v0->unk74 = 0x800507FC; temp_v0->unkA = 1; temp_v0->unk84/86 = args` | ✅ exact |

Byte-level cross-check: the dump's `cursor.update` ends in `0800e003 480082ac`,
which is byte-for-byte the documented **original** tail — confirming m2c read the
real bytes, not a misdecode.

## Caveats discovered (and how to handle them)

1. **The static dump is stale.** `overlay_80050000.bin` is a Jun-6 RAM snapshot.
   Routines patched/baked on Jun-7 do **not** appear in it:
   - `cursor.update` `0x80083a28` is still the **original LTR** version (the RTL
     `120−n*16` bake isn't there).
   - `cursext` `0x80082130` and `boxhook` `0x80082108` are **all zeros** (m2c
     correctly reported "no instructions" for `cursext`).
   The name handler `0x80082190` *is* present, so the dump straddles the bake
   history. **Fix:** for current/baked code, dump live RAM (DuckStation
   `read_memory` with `path=`) on the relevant screen and pass it to
   `overlay2c.sh` as `binfile` with the right `base`.

2. **m2c can't resolve `jr` jump/switch tables.** A dispatcher deeper in
   `box_init` triggered "Unable to determine jump table for jr instruction."
   This is an m2c limitation, not a pipeline bug. **Fix:** decode a sub-range
   that avoids the dispatcher, or read those functions in Ghidra / live disasm.

## Hardening applied during validation

`objdump2m2c.py` now tolerates an under-sized `len`: branch targets past the
decoded slice get trailing stub labels (with a delay slot) plus a stderr WARNING
naming the missing targets, instead of failing to parse. Grow `len` when you see
that warning to capture the whole function.

## Conclusion

The tool belt is proven against real prior work. `overlay2c.sh` is reliable for
reading overlay routines provided the input bytes are current (re-dump after
bakes). PSYQ signatures and the helper scripts (`ghidra_restart.sh`,
`resurrect.sh`, `setup_re_tools.sh`) are installed and self-tested; see
`tools/README.md`.
