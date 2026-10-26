# include/ac1 — shared types for the matching loop

C headers for the Armored Core (SLUS-01323) structures our docs pin down, with our
names and field names, plus an address → name → type table that the tools read.
Everything compiles with the retail compiler (GCC 2.7.2: no `//`, no C99).

| file | contents |
| --- | --- |
| `types.h` | `u8`…`s32` (same guard as `tools/match/include/ac1_types.h`), PsyQ `SVECTOR` / `VECTOR` / `MATRIX`, `AC1_ASSERT_SIZE` |
| `uiobj.h` | overlay 201 front-end object `UiObj` (0xD0), `UiView` (0x38), per-class work area `UiWork` |
| `entity.h` | overlay 202 actor `AcEntity` (0x170), `AcPos`, `AcMover` (0x50 at +0x64), radar/lock list `TargetSlot` (0x30) |
| `projectile.h` | overlay 202 `Projectile` (0xE0), `WeaponDef` (0x1C) |
| `mission.h` | overlay 202 mission control block `MissionCtl` (0x28 at 0x801D0B40), `WorldStruct` (0x40) |
| `symbols.tsv` | address, name, C type, unit — the single source for named globals and functions |
| `symbols.h` | **generated** from `symbols.tsv` by `tools/match/ctx.py gen`; do not edit |
| `ac1.h` | umbrella: includes all of the above (`tools/match/include/ac1/ac1.h` forwards here) |

Every struct ends in `AC1_ASSERT_SIZE(T, size)`, so a wrong pad is a compile error.
Unknown bytes are `unkNN` / `padNN` arrays named by their offset. A field gets a
name only when a doc or the retail code says what it is; the comment says which.

## Using it

**Matched sources** include the umbrella and name things through it:

```c
#include "ac1/ac1.h"
void func_8008AF9C(UiObj *o)
{
    o->kill = 1;
}
```

`tools/match/include/ac1/ac1.h` forwards to this directory, so the oracle finds the
header with no extra flag (it runs cpp from a temporary work dir, where a relative
`-I include` would not resolve). The manifest row adds both symbol tables; any
`--syms` switches off the implicit `ac1_markup.tsv`, so name it too. The paths are
repo-relative: `re/match/check_all.sh` and `tools/loop/drive.py verify` both run from
the repo root.

```
--syms include/ac1/symbols.tsv --syms ac1_markup.tsv
```

Placeholder names that neither table knows (`func_8008DDA0`, `D_801A2318`) still
need a `--defsym`. To name one, add a row to `symbols.tsv`, run
`python tools/match/ctx.py gen`, and drop the `--defsym`.

**m2c** gets the same types as one preprocessed context file:

```bash
python tools/match/ctx.py context
```

That writes `build/ac1_ctx.c` (git-ignored). `ctx.py m2c` runs the whole
carve → objdump → m2c pipeline with it, after rewriting call targets and
`lui`/`%lo` pairs that hit `symbols.tsv` (or an exe name in `ac1_markup.tsv`) into
symbols. Unknown data addresses become `D_XXXXXXXX`. `--proto` types the function itself.

```bash
python tools/match/ctx.py m2c 0x8007CECC --unit 201 --proto "void func_8007CECC(UiObj *o)"
```

Before (`--raw`, what `tools/ghidra/overlay2c.sh` prints):

```c
void func_8007CECC(void *arg0) {
    if (func_80050078(0x8005C2D0) == 0x801BC4F8) {
        func_8005ba2c(0x20, 0x28, 0x88, 0x80, 3, 0, 0);
        func_800210f8(*(s16 *)0x801A2318, 0, 6, 0x3C, 0, 0x7F, 0x7F);
        arg0->unk74 = 0x8007CF64;
    }
}
```

After:

```c
void func_8007CECC(UiObj *o) {
    if (obj_find_by_handle(&func_8005C2D0) == &g_ui_none) {
        func_8005BA2C(0x20, 0x28, 0x88, 0x80, 3, 0, 0);
        SsUtKeyOn(D_801A2318, 0, 6, 0x3C, 0, 0x7F, 0x7F);
        o->update = func_8007CF64;
    }
}
```

`ctx.py m2c` also turns a `j` that lands inside the function (GCC's jump to a shared
epilogue) into a branch; `objdump2m2c.py` renders it as a bogus tail call.

`tools/ghidra/overlay2c.sh` was left unchanged because an unmerged branch rewrites its
m2c line. To use the context with it by hand, write the context, then add the flag to
its last line:

```
"$M2C_PY" "$M2C/m2c.py" -t mipsel-gcc-c --context build/ac1_ctx.c "$tmp/fn.s"
```

`python tools/match/ctx.py check` verifies that `symbols.h` is current, that no header
has a `//` comment, that the headers compile as strict C89 with every size assert
holding, and that m2c parses the context.

## Where the facts come from

Field meanings are ours: `docs/COMBAT_PHYSICS.md`, `ENTITY_AI_FRAMEWORK.md`,
`RUNTIME_RAM_MAP.md`, `MISSION_SYSTEM.md`, `MISSION_STATE_MACHINE.md`,
`MISSION_SCRIPT_VM.md`, `DESTRUCTIBLE_OBJECTS.md`, and the retail code read for this
work (addresses below). Wyrelade's
[Armored-Core-Decomp](https://github.com/Wyrelade/Armored-Core-Decomp) (CC0) was used
only as a cross-check of struct sizes; none of its code or headers is copied. His
sizes below were computed by compiling his `include/fdat201.h` / `fdat202.h` /
`fdat060.h` (upstream HEAD, 2026-10-18) with `mipsel-none-elf-gcc` and `sizeof`.

## Size discrepancies, settled from the retail bytes

| struct | our docs | Wyrelade | retail code | resolution |
| --- | --- | --- | --- | --- |
| AC entity | 0x170, array of 16 at 0x801A26B8 (ENTITY_AI_FRAMEWORK also says "15 × 0x170 window") | `Obj80078CFC` 0x170; per-function views 0x164 (`Obj8007618C`), 0x166 (`Obj80075750`), 0x168 (fdat060) | ovl202 0x80078ADC scans 16 slots (`li a1,16`, `addiu v1,v1,368`); 0x80078A2C allocates from slot 1 for 15 (`li a1,15`) | **0x170 × 16.** The 15-slot window is the allocatable part, slot 0 is the player. His smaller sizes are views that stop at the last field one function touches |
| UI object | 0xD0 pool at 0x801A25E8, 510 slots | `Obj8004DEB0` 0xD0 | ovl201 frame loop `addiu s1,s1,208` up to 0x801BC448 | **0xD0 × 510**, agree |
| projectile | 0xE0, 256 at 0x801AEB3C | `Obj80085394` 0xC0, `Obj800851A8` 0xC4, `Obj80085510` 0xCC, `Obj80085580` 0xD0, `Elem800835A0` 0xE0 | 0x80089ECC `li a1,256`, `addiu v1,v1,224`; one init path writes `sw zero,220(a0)` (+0xDC) | **0xE0 × 256.** His 0xC0–0xD0 structs are truncated views |
| weapon def | "stride ~0x1C" | none | 0x8007C88C: `(id << 3) - id`, `<< 2` = id × 28, base 0x8008E530 | **0x1C**, ids 0..19. Ids 20..39 use a 0x28 table at 0x8008FC48, 40..59 a 0x24 table at 0x8008FEE8 |
| mission "timer struct" 0x801D0B40 | fields to +0x10; thread table "at 0x801D0B60, 24 B/thread"; VM blob pointer at 0x801D0B64 | none | 0x801D0B60 is only ever `lw`/`sw` (a pointer); 0x801D0B68 is the world-struct array | **0x28 block** (`MissionCtl`) whose +0x20 points at the thread table, followed by `WorldStruct[]` |
| target slot | 8 × 0x30 at 0x80041024 | `Elem8007BA2C` 0x30 | widths from every `lui`/`%lo` access in ovl202 | **0x30**, agree |
| `Obj8007A19C` | — | 0x1001 | 0x8007A19C takes an `AcEntity` (`lhu 328(a1)` = +0x148); his struct is the pointer at 0x801A5DC0, which `ac_entity_update` stores from its 2nd argument | not a layout: a byte view that only proves the object behind 0x801A5DC0 is at least 0x1001 bytes |
| `Obj8006F234` | — | 0x104 here (0xE8 in our earlier notes) | reads +0x9E..+0xA4 and a `MATRIX` at +0xE4 | no counterpart in our docs; not modelled. The 0xE8 figure does not match upstream HEAD |
| `Obj800781F0` (0x2C4 in our earlier notes) | — | not in upstream HEAD | — | nothing to compare |

## Field-level corrections the bytes forced

Where our own docs disagreed with each other (or with the task brief), the header
follows the code:

| field | claim | retail code | header |
| --- | --- | --- | --- |
| `AcEntity +0x74/+0x78` | "+0x74 think, +0x78 despawn" | `sh zero,116(s1)` on the player in the fire path (0x8007CD04): +0x74 is an s16 inside the mover block at +0x64. Think/despawn at +0x74/+0x78 is the **UiObj** (0xD0) layout | `AcMover.fire_timer`; `UiObj.update` / `UiObj.destroy` |
| `AcEntity +0x08` | s32 X at +0x08, s32 Z at +0x0C | `ac_entity_update` copies +0x08..+0x0F as one 8-byte unit; the mover stores s16 X/Y/Z (live S-3) | `AcPos pos` (3 × s16 + w) |
| `AcEntity +0x20` | "second X/Z copy (prev / smoothed)" or the train's 16.16 position | `ac_entity_update` saves pos before `fn_update` and stores it at +0x20 after `fn_think` | `prev_pos`, start-of-frame copy |
| `AcEntity +0x16` | — | `ac_entity_update` passes `lh 22(s0)` and `&pos` to 0x8006E6B4 and stores the result back | `section` (world-section index) |
| faction | `+0x2C` (COMBAT_PHYSICS §7) vs `+0x36` (ENTITY_AI_FRAMEWORK) | 0x80073FE8 reads `lhu 44(s4)` with `s4 = ent + 10`, i.e. +0x36; 0x80073768 reads `lhu 54` directly | `+0x36 faction`, `+0x38 ignore_mask` |
| AI state byte | `+0x41` (COMBAT_PHYSICS) vs `+0x40` (ENTITY_AI_FRAMEWORK) | 29 call sites load `lbu a1,0x41(x)` then `jalr` the +0x50 method | `+0x41 state` → `fn_state(e, e->state)`; `+0x40 ai_mode` kept from the doc |
| muzzle flash | `+0x14A/B/C = 180/160/140` (ENTITY_AI_FRAMEWORK) | `sb 334/335/336(s1)` = +0x14E/+0x14F/+0x150 | `flash_r/g/b` at +0x14E |
| `AcEntity +0x00` | pointer vs "u16 id + u16 flags" (TASVideos) | 0x80078ADC compares the whole word with a pointer argument; 0x80078A2C dereferences it (`lhu 20(a1)`) | `void *def` |
| `UiObj +0x01` | matched sources called it "flag"/"active" | frame loop 0x8009BDB0: non-zero → call +0x78, clear +0x01 and +0x00 | `kill` |
| `UiObj +0x78` | "despawn" (COMBAT_PHYSICS) vs "draw" (matched sources) | called once, only on kill | `destroy` |
| `UiObj +0x0C` | Wyrelade `s16 unkC` | 0x800A4340 stores a pointer there (`sw`, matched) | `void *res` |
| `UiObj +0x40` | Wyrelade `u16 unk40/42/44` | frame loop passes +0x40 to RotMatrix with +0x20 | `SVECTOR rot` (signedness made no difference to the matched `+=` code) |
| `UiObj +0x70` | matched sources: "init" | 0x80050078 returns the first used, not-killed object whose +0x70 equals its argument | `ctor`, the lookup key of `obj_find_by_handle` |
| `0x801D0B50` | "objectives complete (3 = all done)" | `vm_load_threads` 0x8008C194 stores its set index there | `MissionCtl.vm_set`: "3" is the index of the thread set that runs when the objectives are done |

Open, not settled here: `AcEntity +0x40` (only the doc names it), the meaning of
`MissionCtl +0x00/+0x02`, and which `WorldStruct` field is current HP (LIVE_CHECKLIST ENT-9).
