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
- **Sub-section layout corrected:** the block holds **per-sub-object** vertex
  pools followed by **variable-length primitive records** (the earlier "fixed
  8-byte primitive @0x15e8" note was a misread — 0x15e8 is int16 coordinate data,
  not primitives). Real primitive-record runs are interleaved after each
  sub-object's vertices (e.g. PA00 entry 2 @**0x1960**, 12 records; PA20 entry 3
  @**0x6c**, 171 records). Decode with `tools/pa_parse.py … --prims 0x1960 12`.

### Primitive record (CONFIRMED — RE'd from overlay + byte-validated)
Variable-length, 32-bit aligned. Walked by reading `byte[1]` (length in **words**):
`record_bytes = 4 + byte[1]*4`. Fields:
```
byte[0] = small length/format tag (= verts*2 + 3..5)
byte[1] = record length in 32-bit words           CONFIRMED (drives the walker)
byte[2] = 0
byte[3] = PRIMITIVE TYPE  (renderer masks it 0xBC; bit 0x80 = TEXTURED)
+0x04…  = per-vertex SHADING block:
            FLAT  (0x20 tri / 0x28 quad): one word "RR GG BB code" (e.g. c8 c8 c8 = grey)
            TEX   (0x24 tri / 0x2c quad / 0x34 / 0x3c): UV0+clut, UV1+tpage, UV2[,UV3]
            (clut marker 0x7980 at hi-half of UV0 word; tpage 0x009b at hi-half of UV1)
+vidx   = N × uint16 VERTEX INDICES into this sub-object's vertex pool
+tail   = optional per-face flag word (often 0)
```
Vertex-index offset & count per type (record-relative; validated PA00+PA20):

| type | shading | verts | reclen | first index @ |
| --- | --- | --- | --- | --- |
| 0x20 | flat | 3 | 16 | +0x08 |
| 0x28 | flat | 4 | 20 | +0x08 |
| 0x24 | textured | 3 | 24 | +0x12 |
| 0x2c | textured | 4 | 32 | +0x14 |
| 0x34 | textured/gouraud | 3 | 28 | +0x12 |
| 0x3c | textured | 4 | 36 | +0x12 |

Validation: every decoded index is `< pool size`, distinct per face, and the record
walk lands exactly on the next sub-section (PA00 e2 → 0x1a4c; PA20 e3 → 0x20c).
In textured records the halfword immediately before the vertex indices is a
**sequential per-poly running index** (0,1,2,…).

### Geometry walker / renderer (CONFIRMED — entry-202 overlay)
| Function | Addr | Role |
| --- | --- | --- |
| relocation walker | **`0x800574D8`** | one-time pass: per stride-28 sub-header, dispatch each record through jump table **`0x8004B184`** (idx = `((byte3 & 0xBC) − 0x20)`, 157 entries) to convert raw vertex indices → byte offsets (×8 into the stride-8 transformed-vertex pool, ×16 into the colour/normal pool). 16 distinct handlers @ `0x800575C0`…`0x80057944`. |
| sub-object cull/transform | `0x80057C44` | per 124-byte sub-object: loads matrix (ctc2 $0–$7), RTPS-transforms 8 bbox corners, NCLIP visibility. |
| **primitive emitter** | **`0x8005A57C`** | per-frame GPU walker. Iterates each record; dispatches on `(byte3 & 0xFD)`: 0x24→tri, 0x2c→quad, 0x34→gouraud tri, 0x3c→gouraud quad, 0xA4/0xAC→textured. Fetches transformed screen XY from pool (`s6 + idx`, entry +0=XY, +4=Z/flag, +6=clip), does NCLIP backface, builds POLY packets (code byte 5/9 = F4/FT4), depth-sorts (`srav` avg-Z) and links into the OT. A second emitter `0x80058B04` handles a matrix-driven variant (called from 0x5D618/0x5E1C8). |

The renderer is fed by the per-object setup `0x80078B14` (which reads the **44-byte
PA record table at `0x8019F538`**, computes the block index via `÷44` magic-multiply
`0xE9BD37A7`, and populates a ~0x168-byte display struct) — see REFERENCE.md §11.

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
1. **Per-sub-object vertex pool boundaries:** decode the stride-28 sub-header at
   block `+12` (read by walker `0x800574D8`) to know where each sub-object's vertex
   pool / primitive run begins, so a generic ripper doesn't have to scan for the
   `XX 0N 00 2T` record signature. (Indices are pool-relative, so this is needed
   for a faithful OBJ export.)
2. **OBJ export:** with the vertex array (int16 x,y,z) + per-type primitive decode
   now confirmed, write `tools/pa_obj.py` to emit a mesh and eyeball a stage.
3. **DuckStation ground-truth (optional):** breakpoint emitter `0x8005A57C`, confirm
   the screen-space verts it submits match a decoded record — and resolve the
   trailing per-face flag word + the UV/tpage exact bitfields.
4. Port walker `0x800574D8` + emitter `0x8005A57C` into PSXmod as **AC1mod**.
