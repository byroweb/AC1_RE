# DuckStation save-state format (offline RAM/VRAM extraction)

RE'd 2026-06-15 against the project's `savestate_backup_20260609/SLUS-01323_*.sav`
(DuckStation MCP fork; header layout re-checked 2026-10-17 against the fork's
`src/core/save_state_version.h`, see the correction below). Lets the project read PS1
RAM/VRAM from a `.sav` **without launching the emulator** — useful for the
live-confirmation batch (read a known RAM address) and as a VRAM source for stage texturing.

Tool: **`tools/duckstation/savestate.py`** (`SaveState` class + CLI). Needs a zstd
decoder: the `zstandard` Python package (`pip install zstandard`, preferred on Windows)
or the `zstd` CLI on PATH. Archiving states under stable IDs is the sibling tool
`states.py` — see "Save-state library" at the end.

## Container ("DUCCS")

Little-endian header = `SAVE_STATE_HEADER` (packed to 4; 0xd8 bytes):

| off | type | meaning |
| --- | --- | --- |
| `+0x00` | u32 | magic `'DUCC'` (`0x43435544`) |
| `+0x04` | u32 | **state version** — the `'S\0\0\0'` seen in the 2026-06 backups is `83` |
| `+0x08` | char[128] | display title ("Armored Core (Reprint)") |
| `+0x88` | char[32] | game serial `"SLUS-01323"` |
| `+0xa8` | u32 | media path length (the `82` previously read here as the version was the length of the disc path string) |
| `+0xac` | u32 | offset of the media path (UTF-8, no terminator) |
| `+0xb0` | u32 | media sub-image index |
| `+0xb4` | u32 | screenshot compression type (`0` none, `1` deflate, `2` zstd, `3` xz; since v69) |
| `+0xb8` | u32 | screenshot width |
| `+0xbc` | u32 | screenshot height |
| `+0xc0` | u32 | screenshot compressed length |
| `+0xc4` | u32 | screenshot offset — decompresses to RGBA8, `pitch × height` bytes |
| `+0xc8` | u32 | main-state compression type (same enum; the MCP fork writes zstd) |
| `+0xcc` | u32 | main-state **compressed** length |
| `+0xd0` | u32 | main-state **uncompressed** length (`3,819,074` for AC1) |
| `+0xd4` | u32 | main-state offset |

> **Correction (2026-10-17).** The 2026-06 table called `+0xa8` the state version (`82`)
> and did not decode `+0x04`. The emulator's struct puts `version` at `+0x04` and
> `media_path_length` at `+0xa8`; the five data/screenshot fields it listed are at the
> offsets the struct gives, which is how the mis-labelling was caught. References to "v82"
> elsewhere in this repo mean those backups; their real version word reads 83. The library
> tool records whatever `+0x04` says, so newer fork builds (HEAD is 87) are handled.

The screenshot thumbnail and the main state are each one compressed frame (zstd:
`28 b5 2f fd`). The main state at `+0xd4` decompresses to one blob = the concatenated
component serialization. Each component is preceded by a `StateWrapper` marker
(u32 length + ASCII name: `06 00 00 00 "System"`, then `"CPU"`, `"Bus"`, `"DMA"`,
`"InterruptController"`, `"GPU"`, …) but fields inside a component carry **no framing**,
so RAM and VRAM are still found by **content**, not a fixed offset.

The System component (from the fork's `System::DoState`) is: marker · region (u8 from
v74, u32 before) · taints u32 (from v75) · **frame_number u32** · internal_frame_number
u32 · BIOS hash (16 bytes, from v58) · marker `"CPU"`. `savestate.frame_numbers()` reads
the two counters at `cpu_marker - 24`, which holds for every version ≥ 58 regardless of
the region width. Checked against the source only — **not yet against a real state**;
the first archived state should confirm it (compare with the emulator's frame counter).

`3,819,074 ≈ 2 MB RAM + 1 MB VRAM + 512 KB SPU RAM + CPU/CD/timers`.

## Finding RAM base (the EXE anchor)

The PSX-EXE (`SLUS_013.23`, disc sector 39) loads at its header `t_addr` (**`+0x18`**) =
**`0x80011000`** (AC1 v1.1; `t_size 0x29000`). Its entry point `pc0` (`+0x10`) is
`0x80011e6c`. So the first bytes of the EXE payload (after the 2 KB `PS-X EXE` header —
the `COM\RTIM.T`, `COM\FDAT.T`, … file-name table) appear in the decompressed blob at
`ram_base + (0x11000)`. Find that byte run → `ram_base`.

For the v87 states archived 2026-10-17 `ram_base = 0x1a62`, i.e. 71 bytes after the
`"Bus"` marker at `0x1a1b` (Bus component fields, then the 2 MB RAM; the `"GPU"` marker
follows at `0x201b20`, right after RAM). Then any PS1 address `A` (`0x800xxxxx`) is
`blob[ram_base + (A - 0x80000000)]`. **Verified** against a live 2 MB RAM dump of the same
session (`M30-ctrl-01`): all 512 4 KB pages identical, and the fields read as
`mission=30`, `objobj=0x801C4B40`, matching the emulator's live reads.

> **Correction (2026-10-18).** The June tool read the header's `+0x10` (`pc0`, `0x80011e6c`)
> as the load address, so its `ram_base = 0xbf6` was `0xe6c` too low and every RAM read
> was taken from the wrong address. The "Findings on the current backups" below (all ten
> 2026-06 backups = menu/garage) were produced with that offset and are therefore
> **unverified**; the backups themselves are not on this PC. `states.py refresh`
> re-reads the automatic columns of every archived state after a reader fix like this one.

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
in-mission state from the running emulator. (The companion `mods/farsi/docs/screens/vram_full_slot2.png`
is likewise a garage VRAM — it holds parts/UI pages, not stage texture pages.)

## Caveat — VRAM offset

`SaveState.vram()` returns a 1 MB window guessed just past the 2 MB RAM; that guess is
**wrong by at least the GPU register block**: in the v87 states the `"GPU"` marker sits at
`0x201b20` (RAM ends at `0x201a62`, then DMA/InterruptController), and VRAM is somewhere
inside the GPU component after its registers. The exact VRAM offset is **not yet pinned**;
LVL-1 is now a one-liner against `M30-ctrl-01` (`blob.find(b"\x03\0\0\0GPU")`, then compare
with DuckStation's own VRAM dump of the staged state). Use `vram_at(off)` once confirmed.

## Save-state library

Slots are scratch; the record is the **library** (a directory outside the repo, default
`~/dev/ac1_states`, env `AC1_STATES_DIR`) plus the committed manifest
[`disc_map/states/STATES.tsv`](../disc_map/states/STATES.tsv). Every archived state has a
stable ID (`M01-ctrl-01`, `GAR-main-01`, …) and **docs cite IDs, never slot numbers** —
that rule exists because the 2026-06 "slot 1 = level start" citations were overwritten and
left the project with no in-mission state at all ("Findings" above).

Tool: **`tools/duckstation/states.py`** — `archive <slot> <id>` (copies the slot file into
the library as `<id>__<YYYYMMDD>__v<version>.sav`, writes a JSON sidecar and the
thumbnail as `.png`, reads mission/stage/objobj/flags/result/frame from the state and
appends the manifest row), `stage <id> <slot>` (back into a slot; refuses to clobber an
un-archived slot), `adopt` (register a hand-copied `.sav`), `check` (sha256 every row,
missing/orphan files), `list`, `show`, `refresh` (re-read the automatic columns),
`set-status`. ID convention, columns and workflow: [`disc_map/states/README.md`](../disc_map/states/README.md).

First entries (2026-10-17, build 0.1-12000, state version 87): `GAR-main-01` (garage after
first boot) and `M30-ctrl-01` (mission 30 shortly after player control — the first
in-mission state the project has had offline).

Degradation: without a zstd decoder or without `$AC1_BIN`, `archive` still copies the file
and records the header; the RAM-derived columns are written as `?`. Test harness on
synthetic DUCCS files: `python tools/duckstation/states_test.py`.
