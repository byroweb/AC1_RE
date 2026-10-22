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
ss = SaveState(r"C:\Users\bross\dev\ac1_states\M30-ctrl-01__20261017__v87.sav",
               disc_bin=os.environ["AC1_BIN"])
ss.mission_byte()   # 0x800411F8  (30)
ss.stage_byte()     # 0x8004121B  (0 = menu/garage)
ss.in_mission()     # objective-object ptr 0x8019F51C != 0 OR stage set
ss.u32(0x801A26B8)  # entity array, etc.
```

CLI (`--disc` defaults to `$AC1_BIN`; `--header` needs no disc):
```sh
python tools/duckstation/savestate.py STATE.sav --header
python tools/duckstation/savestate.py STATE.sav --read 0x8019F51C
python tools/duckstation/savestate.py STATE.sav --dump-ram ram.bin
```

## Archived states (what exists offline)

In-mission states **now exist** (2026-10-17/18 live session, build 0.1-12000, state
version 87, pristine disc `48d9a31d7533…`), all in the library and in
`disc_map/states/STATES.tsv`:

| ID | what | reader says |
| --- | --- | --- |
| `GAR-main-01` | garage main menu after first boot | `mission=0 stage=0 objobj=0 in_mission=no` |
| `M30-ctrl-01` | mission 30 "Eliminate Squatters", shortly after player control (saved while running) | `mission=30 stage=0 objobj=0x801C4B40 in_mission=yes` |
| `M30-mid-01` | same mission after two rifle hits on MT slot 1 (HP 2050→1614), parent `M30-ctrl-01` | `mission=30 stage=0 objobj=0x801C4B40 in_mission=yes` |

The reader is **validated on this PC**: `M30-ctrl-01` was read offline (zstd via the
`zstandard` package) and its 2 MB RAM matches the live `dump_ram` of the same state
(`M30-ctrl-01__20261017.ram.bin`, kept in the library) in all 512 pages; the mission byte
and objective-object pointer agree with the emulator's live reads. So an **in-mission VRAM
source exists offline** for the first time — the remaining gap is the VRAM offset (next
section). Note `stage_byte=0` in mission 30 while `objobj` is set: `in_mission()` is true
through the pointer, and the stage/PA byte's meaning in this mission is a question for
MIS-11, not a reader fault.

History: the June backups (slots 1–10 + `resume`, old PC) were all reported as
menu/garage, but with the `0xe6c`-low RAM base described above, so that finding is
unverified and those files are not on this PC. (`mods/farsi/docs/screens/vram_full_slot2.png`
is a garage VRAM — parts/UI pages, not stage texture pages.)

## VRAM offset (RESOLVED 2026-10-22)

VRAM has its own StateWrapper marker inside the GPU component: **`08 00 00 00 "GPU-VRAM"`**,
then the 1 MB (1024×512 16bpp), then `0f 00 00 00 "GPUTextureCache"`.
`savestate.vram_offset(blob)` returns the byte after the marker, and `SaveState.vram()`
uses it. The GPU registers before the marker vary in length: VRAM is `0x2bb` bytes after
the `"GPU"` marker in six of the seven archived v87 states and `0x303` in `M30-mid-01`. So
neither the old "just past RAM" guess nor any fixed offset is right. Checked on every
library state:
- the bytes after the 1 MB are the `GPUTextureCache` marker;
- the in-mission frame buffers sit at (0,0) and (0,240);
- mission 30's 8bpp level pages (tpage `0x85..0x89`) decode through their CLUTs
  (VRAM rows 481–484) as the stage's stone, metal and grating textures
  (`browser-tools/sceneview/`).

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

First entries (2026-10-17/18, build 0.1-12000, state version 87): `GAR-main-01`,
`M30-ctrl-01`, `M30-mid-01` — see "Archived states" above.

Degradation: without a zstd decoder or without `$AC1_BIN`, `archive` still copies the file
and records the header; the RAM-derived columns are written as `?`. Test harness on
synthetic DUCCS files: `python tools/duckstation/states_test.py`.
