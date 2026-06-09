# `GG/P0–P3/PA00–PA71.T` — Stage/map packs (format RE in progress)

Target: **SLUS-01323 (v1.1)**. **72 files** (`PA00.T`..`PA71.T`) across `GG/P0..P3`,
disc sectors **103637–126704**, ~700–900 KB each. Loaded via `load_T_file(3, …)`
(file id 3, `REFERENCE.md` §3). These are the **3D stage geometry/map packs** — the
key target for the future AC1mod map/model viewer (`docs/AC1MOD_VISION.md`).

## Container (CONFIRMED)
Standard count-first `.T` container, identical mechanism to King's Field (verified
against KFModTool `core/tfile.cpp`): `uint16[0]` = entry count, then `count+1`
uint16 sector offsets (×2048), duplicate offsets = zero-length entries.
- `PA00.T`: 176 entries, 114 non-empty.
- **No text** (probed PA00/20/40/60: 0 English words). Geometry only.
- Tools: `tools/extract_t.py` (extract), `tools/pa_parse.py` (structure dump).

## Entry roles (CONFIRMED)
| Entry | Role |
| --- | --- |
| **0** | Map **master header** — `u32[0]`=used size, `u32[1]`=**signature `0x03072d39`**, pointer table + an `0xNN08` object-index list. Full layout TBD. |
| **1** | 2048 B **offset directory** — `u32[1]`=0x39 count, then an ascending uint16 pointer table into a record region. TBD. |
| **2 … N** | **Size-prefixed geometry blocks** (112 of them in PA00). |

## Geometry block layout (PARTIAL — verified on PA00 entry 2)
```
u32[0]  = block size in bytes  (== entry length)              CONFIRMED
u32[1]  = 0
u32[2]  = sub-object count? (0x0c)                            hypothesis
u32[3..5] = small counts (1,3,3)
then (offset,count) sub-section descriptors, e.g.:
  (0x1218, 122)  -> VERTEX array
  (0x15e8, 103)  -> PRIMITIVE array
  ...
```
- **Vertices** @0x1218, 122 × **8-byte** records = `int16 x, y, z, flag`.
  Decoded coords are plausible model space: X −150..1287, Y −690..9216,
  Z −222..3600. And `0x1218 + 122*8 = 0x15E8` lands exactly on the next
  sub-section → stride/count CONFIRMED. (4th int16 is a flag/normal-index, not
  always 0.) Decode with `tools/pa_parse.py … --verts 0x1218 122`.
- **Primitives** @0x15e8: **fixed 8-byte records** of the form
  `int16 a, int16 b(≈const per strip), u32 tag(≈0x000000a1)`. These are **NOT**
  Sony TMD variable-length packets (no `olen/ilen/flag/mode` 0x20–0x3f tags) —
  **AC1 uses a custom primitive encoding.** Exact field meaning (vertex indices,
  colour/UV, tpage) is the next decode step.

## Why KFModTool can't open PA files
KFModTool parses **Sony TMD** (`0x41` magic) + TIM. AC1 geometry blocks have no TMD
magic and use a custom primitive format, so KFModTool's `tmdreader`/`model`
handlers don't apply. KFModTool's `datahandlers/model.h` is still the best
**reference** for PSX primitive semantics (flags: single/double-sided, gouraud,
textured, translucent; SVECTOR vertices) when reverse-engineering AC1's variant.

## Loader & registrar (RE 2026-06-08 — see REFERENCE.md §11)
The mission code is **FDAT entry 202** (`0xCA`), an overlay at base `0x8004ADA0`
(extract with `tools/extract_t.py`; not in the entry-201 Ghidra DB).
- **`FUN_8004F1A8`** builds the path from template `"P0\PA00.T"` (@`0x8008D928`)
  using stage byte `DAT_8004121B`, then `load_T_file(0, path)` — **PA loads into
  file slot 0** — and `read_T_entry(0, 1, dest)` to grab entry 1 (offset directory)
  before the geometry blocks (entries 2..N).
- **`FUN_80073AD8`** registers each block: copies it to mxt work RAM and stores a
  **44-byte record** at table `0x8019F538` (`+0x28`=ptr, `+0x00`/`+0x02`=counts from
  `half[block+4]>>2` / `half[block+6]>>2`, `+0x04`=index). This CONFIRMS the block
  header's halfword count fields are real geometry element counts.

## Next steps (to fully crack + render)
1. **Find the geometry walker/renderer:** the per-frame function that reads the
   registered `+0x28` buffers and emits GPU primitives. Locate via xrefs to the
   record table `0x8019F538` once entry 202 is imported into Ghidra at `0x8004ADA0`.
2. **DuckStation MCP:** load a mission, breakpoint GPU poly submission (GP0
   commands) / the walker, watch which block bytes feed each vertex/primitive —
   ground-truths the primitive-record decode.
3. Decode the primitive record (vertex indices + texture/colour/tpage) → export OBJ
   → confirm a recognizable stage mesh.
4. Port the walker + renderer into PSXmod as **AC1mod** (`docs/AC1MOD_VISION.md`).
