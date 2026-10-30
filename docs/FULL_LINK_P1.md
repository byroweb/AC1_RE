# Full link — P1: the exe shell (2026-10-27)

Phase P1 of [FULL_LINK_PLAN.md](FULL_LINK_PLAN.md). Exit criterion: the resident exe
rebuilds with a matching SHA-1, with 0% C. **Met:**

```
make verify
SCUS_941.82  OK    ee884a2cb3ce30bc13255142fecd5a2f02ba04ad  build/link/exe/SLUS_013.23
```

A clean build takes about 15 s.

## How to run it

You need:
- `AC1_BIN`: your own disc image (the exe is carved from it, as `match_fn.py` does);
- mipsel binutils on `PATH` (`CROSS`, default `mipsel-none-elf-`);
- splat: `pip install -r tools/link/requirements.txt` (`splat64` 0.50.1). If you
  installed it in a venv, point `SPLAT` at it, for example
  `make verify SPLAT="path/to/venv/python -m splat"`.

Everything is written under `build/link/` (gitignored, because it holds retail bytes
and disassembly).

## The pipeline

| step | tool | output |
|---|---|---|
| 1. config | `tools/link/gen_exe.py` reads `CODE_UNITS.tsv` and `functions.tsv` | `exe.yaml`, `symbol_addrs.txt`, `image.bin` (the exe without its header) |
| 2. split | splat | one `asm/f_<vaddr>.s` per function (694), rodata and data slices, symbolised bss, `exe.ld` |
| 3. assemble and link | `tools/link/unit.mk`: mipsel `as`, then `ld` with the splat script | `exe.elf`, `exe.map` |
| 4. header | `objcopy -O binary`, then `tools/link/mkpsexe.py` | `SLUS_013.23` |
| 5. check | `tools/link/verify.py` | SHA-1 against `CODE_UNITS.tsv` |

- **One object per function**, in address order (D2). P2 swaps a function's asm
  object for its compiled C object, one subsegment at a time.
- **The header is generated, not copied.** `mkpsexe.py` takes pc0 from
  `__SN_ENTRY_POINT`, t_addr and t_size from the link (zero-padded to 0x800), and
  writes the stack 0x801FFFF0 and the North America region string.
- **The exe data/bss split is checked on every build.** `mkpsexe.py` fails unless
  `main_DATA_END` = `main_BSS_START` = `0x80039CB8` and `main_BSS_END` = `0x8004ADA0`,
  the range `__SN_ENTRY_POINT` zeroes (P0 finding 1).

## What the link resolves, and what is still pinned

All text is relocatable asm: calls, branches, and jump-table targets between the
exe's own functions are resolved by `ld`. The bss is symbolised by splat: 290
`.space` objects, so bss references are relocations too.

The remaining 390 addresses are pinned as absolute symbols
(`undefined_syms_auto.txt`, `undefined_funcs_auto.txt`):

| pinned | count | why | phase that removes it |
|---|---:|---|---|
| into the `.rodata` slice | 160 | rodata is a T0 retail slice | P2 (compiled rodata), P5 |
| into the `.data` slice | 222 | data is a T0 retail slice | P5 |
| into the middle of a function | 4 | labels inside PsyQ functions (`_ExitCard` ×2, `_patch_gte`, `func_80030F38`) that mark code ranges the library copies or patches | P2 (the real `.LIB` objects define them) |
| into ovl201 | 4 | see below | P3 |
| overlay header slot `0x8004ADA4` | 1 | the boot loop calls the program overlay's entry slot | P3 (`ovl_header + 4`) |
| `0x8004ADA0` | 1 | the end of bss, as the start-up code's loop bound | P2 (`_end`) |
| constants that look like addresses | 2 | `0x31FFFF`, `0x80000004` | P2 (the C has the constants) |

### The exe ↔ ovl201 cycle is wider than P0 found

P0 found the boot loop's direct call to ovl201 `func_8009B414`. The split shows
three more: the exe's PsyQ `__fixunsdfsi` (`0x80026F94`) calls the soft-float add
(`0x800B59F4`), compare (`0x800B62DC`) and another helper (`0x800B64E0`), which were
linked **into ovl201**. So psylink resolved exe symbols against ovl201 in one link,
which favours the plan's single-`ld`-run `OVERLAY` option for P3 over a two-pass
link. Until P3, the four are absolute symbols at their retail addresses.

### Data islands in text

Four "functions" in `functions.tsv` are data, not code:
- `2MBYTE_OBJ_B4` (`0x80011F18`, four `0x00200000` words: the 2 MB memory
  configuration);
- `0x80027294`, `0x80029A8C`, `0x8002A40C`.

splat emits them as `.word`, and `as` warns "missing .end" for each. The bytes are
right. P2 should mark them as data in `functions.tsv`.

## P0 entry checklist, status

- [x] exe data/bss split `0x80039CB8` and `_end = 0x8004ADA0`: checked by
  `mkpsexe.py`.
- [~] exe ↔ ovl201 cycle: pinned for now (5 symbols above); resolved in P3.
- [ ] `CODE_UNITS.tsv` row for `fdat188`: moved to P4. It needs the generator
  (`tools/ghidra/wyrelade_layout.py`) to take local corrections, and only the
  mission-unit link uses it.
- [ ] fdat074 boundary (`text_off` 0x210, `text_va` 0x801C4D50): moved to P4, same
  generator change. `functions.tsv` already starts fdat074 text at `0x801C4D50`.
- [x] PsyQ 3.7 SDK: not needed. Q1 changed to B′ on 2026-10-29: the library stays asm.

## Next: P2

- Swap the matched game functions' asm objects for compiled C objects: the
  matcher's compile path, factored out of `match_fn.py` (D1).
- Keep the 563 PsyQ library functions as asm (Q1 = B′; no SDK needed).
- Move `re/match` into one `src/` tree (Q3).
- The P2 row in the plan says "125 matched game functions". That number dates from
  before matching finished: every one of the 131 exe game functions is now matched.
