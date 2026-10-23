# Full link — P0 readiness report (2026-10-23)

Phase P0 of [FULL_LINK_PLAN.md](FULL_LINK_PLAN.md).

To regenerate:

```
python tools/link/p0_readiness.py -j 24
```

The run takes ~5 min cold and seconds when cached. It writes three files to
`build/link/p0/` (gitignored, since they hold retail addresses and symbol
names):
- `summary.md`;
- `data_refs.tsv`: one row per distinct target address, with its class;
- `boundaries.tsv`.

To make (b) possible, `tools/match/match_fn.py --json` now also reports a
`relocs` list (type, symbol, effective address) for every relocation. The change
is additive; existing callers are unchanged.

## Headline

| check | result |
|---|---|
| (a) every manifest row through the oracle | **5,550 / 5,550 clean MATCH, 0 masked words**, after rebuilding cc1 (below) |
| (b) relocation targets outside every unit's sections | **0** (the plan's D5 "fixed RAM" class is empty) |
| (c) gaps / overlaps in `functions.tsv` .text tiling | **0** |
| (c) manifest rows disagreeing with `functions.tsv` | **1** (fdat074, a boundary error) |
| (c) calls / data references into the middle of a function | **79**, which come down to 3 causes, all explained below |

Nothing found blocks P1.

## (a) Oracle: one stale compiler, no stale source

The first pass, on an older main with 5,541 rows, gave 5,540 / 5,541. `ovl201 0x80085640` mismatched because this
checkout's `tools/match/vendor/cc1.exe` was dated Oct 20. That build predates the
float-constant fix (`patches/gcc-const-double-slots.patch`, PR #86): the diff
showed double literals loaded from memory where retail has `lui` immediates.
After `tools/match/setup.sh --force` the row matches, and so does every other
row. On current main (5,550 rows, cc1 also carrying `gcc-reorg-negate-rtx.patch`
and linked without ASLR) all rows match. Three twin rows compiled from
`ovl202/func_8004C854.c` and one other row crashed cc1 (access violation) once
under `-j 4`, then matched on retry. The checker never caches an ERROR, so a
flaky crash can't hide a real result.

The checker now keys its cache on cc1's SHA-1, so a compiler change forces
re-verification.

**Lesson for P6:** the matcher's correctness depends on a locally built compiler.
The reproducible container has to build cc1 from source with the patch and
record its hash next to the output hashes.

## (b) Where the matched code's addresses point

The class is relative to the referencing unit. "Host" means the program overlay
a mission unit is linked against (ovl202 for missions loaded at `0x801C4B40`,
ovl204 for `0x801C8460`, ovl203 for `0x801B80C8`).

| class | distinct addresses | (function, address) refs |
|---|---:|---:|
| own-text | 4,274 | 8,426 |
| own-data | 1,599 | 2,139 |
| own-bss | 1,222 | 3,920 |
| own-rodata | 1,061 | 1,102 |
| exe-bss | 310 | 1,060 |
| exe-text | 167 | 2,750 |
| host-text | 132 | 5,485 |
| exe-data | 86 | 649 |
| host-bss | 87 | 1,395 |
| host-data | 12 | 194 |
| overlay-text / overlay-rodata (exe → ovl201) | 1 / 1 | 1 / 1 |
| fixed RAM / scratchpad / I/O | 0 | 0 |

Referenced data addresses per unit (a lower bound on symbol counts for P5):

| unit | rodata | data | bss (span) |
|---|---:|---:|---|
| exe | 41 | 91 | 352 (`0x80039CC0..0x8004AC90`) |
| ovl201 | 330 | 578 | 522 (`0x8017E448..0x801F50C0`) |
| ovl202 | 20 | 100 | 215 (`0x80091228..0x801ED854`) |
| ovl203 | 31 | 75 | 166 (`0x8008C848..0x801DE518`) |
| ovl204 | 30 | 82 | 175 (`0x8008C654..0x801EE8B0`) |
| 57 mission units | 609 | 767 | 127 |

### Findings that change the plan

1. **The exe's bss is pinned by the start-up code.** `__SN_ENTRY_POINT`
   (`0x80011E6C`) zeroes `0x80039CB8..0x8004ADA0`. So exe `.data` ends at
   `0x80039CB8`, and the last 0x348 bytes of the 0x29000-byte image are bss that
   falls inside the sector padding. The PS-X EXE header has no bss fields
   (b_addr/b_size are 0). The linker script must make
   `__bss_start`/`_end` come out at exactly those addresses.
2. **What looked like "fixed RAM" is overlay bss, including the mission
   buffer.** ovl202/203/204 load missions at three different addresses. Every
   address above that point moves by the same amount between builds:
   `D_801E8B74` is at `0x801E8B74` / `0x801DC0FC` / `0x801EC494`, a shift of
   −0xCA78 / +0x3920, exactly the difference between the mission bases. So the
   mission buffer is a `.bss` array in each host build, followed by more host
   globals (`g_world_structs`, at `base + 0xC028`). The entity pool at
   `0x801A26B8` is ovl202 bss too.
   - **Consequence for D4:** about 1,100 referenced bss addresses in the program
     overlays, spanning more than 1 MB each, all allocated by psylink from
     `-fcommon` COMMON symbols. Reproducing psylink's COMMON order is now **the
     main technical risk of the project**. Pinning in the linker script is the
     fallback.
   - The upside: nothing is genuinely at a fixed address, so the whole program
     is shiftable in principle.
3. **The exe calls into ovl201, a link cycle.** The boot loop in
   `func_800122A8` loads FDAT 201 and calls `func_8009B414` directly. It also
   calls the overlay header's main-entry slot through `D_8004ADA4`. ovl201 in
   turn links against the exe. Plan: a two-pass link. Pass 1 links the exe with
   that symbol at its retail address, pass 2 relinks it from ovl201's map, and
   `make verify` checks that the second pass changed nothing. Alternatively, one
   `ld` run with an `OVERLAY` statement. The header slot becomes an
   overlay-base symbol (`ovl_header + 4`), not a `D_` constant.
4. **There's a 64th code unit.** `fdat188` is a 4 KB stub (eight header
   pointers to one `jr ra`, already matched) that `CODE_UNITS.tsv` doesn't list.
   It needs a row with its own SHA-1 before P4.

## (c) Boundaries

The `functions.tsv` rows tile every unit's .text exactly. The 79 mid-function
targets come down to three causes:

| count | target | cause | fix (phase) |
|---:|---|---|---|
| 63 | `VectorNormalSS` `0x80027480` (calls) | a PsyQ libgte **alternate entry point** inside `VectorNormalS` (`0x80027440`, 112 B). The library objects export both | none: linking the real `.LIB` (Q1 option A) resolves it. Record multi-entry functions in `functions.tsv` (P2) |
| 12 | `D_80031564` (data, from ovl202/203/204 `func_800672CC`/`func_800674B0` and their twins) | **folded array offset**: the C names `tb = D_80031564` and indexes `tb[t]` with `t ≥ 64`. GCC folded `table[t - 64]` into the base address, so the "symbol" is 128 bytes before a real exe-data table, probably PsyQ's sqrt table, and it lands inside exe text | rewrite as `tb = &SQRT_TABLE[-64]` (same bytes, real symbol plus addend) in P2/P3. **The same pattern will exist in .data/.bss** wherever a `D_` name doesn't start an object; P5 must check each `D_` against real object starts |
| 4 | `0x801C4D50` in fdat074 (data: four function-pointer refs) | **wrong unit boundary**: `0x801C4D48..4D4F` (`0x27100007`, `0x00000010`) is rodata that decodes as an `addiu`. .text starts at `0x801C4D50`, where the matched function is (manifest size 1000 against `functions.tsv`'s 1008, the one size disagreement) | fix fdat074 `text_off`/`text_va` (`0x210` / `0x801C4D50`) and its first `functions.tsv` row at the generator, not by hand, in P1, when the splitter configs are generated from these tables |

## PsyQ libraries (Q1 = option A)

No PsyQ SDK was found on the build machine (no `LIB*.LIB`, `CARD.OBJ` or
`2MBYTE.OBJ`). **P2 needs your PsyQ 3.7 library
directory** (`libsnd` 3.5, `CARD.OBJ` 3.0, `2MBYTE.OBJ`). P1 doesn't, because
the library block stays a T0 slice until P2.

## P1 entry checklist

- [ ] add a `CODE_UNITS.tsv` row for fdat188;
- [ ] fix the fdat074 boundary in the unit/function generator;
- [ ] encode the exe data/bss split `0x80039CB8` and `_end = 0x8004ADA0`;
- [ ] two-pass exe ↔ ovl201 link (`func_8009B414`, header slot);
- [ ] supply the PsyQ 3.7 SDK path (needed by P2).
