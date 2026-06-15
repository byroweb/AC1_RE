# DuckStation save-state format (offline RAM/VRAM extraction)

RE'd 2026-06-15 against the project's `savestate_backup_20260609/SLUS-01323_*.sav`
(DuckStation MCP fork, state **version 82**). Lets the project read PS1 RAM/VRAM from a
`.sav` **without launching the emulator** — useful for the live-confirmation batch
(read a known RAM address) and as a VRAM source for stage texturing.

Tool: **`tools/duckstation/savestate.py`** (`SaveState` class + CLI). Needs the `zstd`
CLI on PATH; no Python package dependency.

## Container ("DUCCS")

Little-endian header:

| off | type | meaning |
| --- | --- | --- |
| `+0x00` | u32 | magic `'DUCC'` (`0x43435544`) followed by `'S\0\0\0'` |
| `+0x08` | char | display title ("Armored Core (Reprint)") |
| `+0x88` | char | game serial `"SLUS-01323"` |
| `+0xa8` | u32 | state version (`82`) |
| `+0xc0` | u32 | screenshot zstd length |
| `+0xc4` | u32 | screenshot zstd offset |
| `+0xcc` | u32 | main-state zstd **compressed** length |
| `+0xd0` | u32 | main-state **uncompressed** length (`3,819,074` for AC1) |
| `+0xd4` | u32 | main-state zstd offset |

The screenshot thumbnail and the main state are each a single **zstd frame**
(`28 b5 2f fd`). The main state at `+0xd4` decompresses to one blob = the concatenated
component serialization (Bus RAM, CPU, GPU incl. VRAM, SPU RAM, timers, …) with **no
per-field framing**, so RAM and VRAM are found by **content**, not a fixed offset.

`3,819,074 ≈ 2 MB RAM + 1 MB VRAM + 512 KB SPU RAM + CPU/CD/timers`.

## Finding RAM base (the EXE anchor)

The PSX-EXE (`SLUS_013.23`) loads at its header `t_addr` = **`0x80011e6c`** (AC1 v1.1).
So the first bytes of the EXE payload (after the 2 KB `PS-X EXE` header) appear in the
decompressed blob at `ram_base + (t_addr - 0x80000000)`. Find that byte run → `ram_base`.

For the backup states `ram_base = 0xbf6` (consistent across all 10). Then any PS1 address
`A` (`0x800xxxxx`) is `blob[ram_base + (A - 0x80000000)]`.

`exe_anchor_from_disc()` pulls the anchor straight off a MODE2/2352 disc `.bin`
(de-interleaving the 2048-byte data regions — a raw `+0x800` read crosses a sector
boundary and yields garbage).

## Reading game state / identifying the mission

```python
from tools.duckstation.savestate import SaveState
ss = SaveState("SLUS-01323_resume.sav", disc_bin="Armored Core (v1.1).bin")
ss.stage_byte()     # 0x8004121B  (0 = menu/garage)
ss.in_mission()     # objective-object ptr 0x8019F51C != 0 OR stage set
ss.u32(0x801A26B8)  # entity array, etc.
```

CLI:
```sh
python3 tools/duckstation/savestate.py STATE.sav --disc DISC.bin --read 0x8019F51C
python3 tools/duckstation/savestate.py STATE.sav --disc DISC.bin --dump-vram /tmp/vram.bin
```

## Findings on the current backups

All 10 backup states (slots 1–10 + `resume`) are **menu/garage** — `stage_byte=0`,
objective-object ptr `0`, player entity all-zero. So there is **no in-mission VRAM dump
offline** right now; capturing the per-stage texture bank from VRAM needs a fresh
in-mission state from the running emulator. (The companion `docs/screens/vram_full_slot2.png`
is likewise a garage VRAM — it holds parts/UI pages, not stage texture pages.)

## Caveat — VRAM offset

`SaveState.vram()` returns a 1 MB window guessed just past the 2 MB RAM; the exact GPU
component offset within the v82 blob is **not yet pinned** (no in-mission state to validate
against). Use `vram_at(off)` once the offset is confirmed from a live in-mission state.
