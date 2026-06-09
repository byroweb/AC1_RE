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

## Geometry block layout (CONFIRMED — RE'd from walker FUN_800574D8, validated PA00/PA20)
```
u32[0]  = block size in bytes  (== entry length)              CONFIRMED
u32[1]  = 0
u32[2]  = block[+8] = small offset O to the sub-object section CONFIRMED
            (the walker is called with a0 = block + block[+8])
```
The sub-object SECTION begins at `block + block[+8]`:
```
secbase = block + block[+8]            (PA00 e2: block[+8]=0x0c -> secbase=+0x0c)
secbase[+8]  = u32  SUB-OBJECT COUNT                          CONFIRMED
secbase[+12] = SUB-OBJECT TABLE  (stride 28)                  CONFIRMED
```
**Relocation base** for every in-table offset = `secbase + 12` (the walker does
`field += a0+12`, a0=secbase). So a stored offset O maps to file offset `O + block[+8] + 12`.

### Sub-object descriptor (28 bytes — CONFIRMED from relocation walker)
| off | size | field |
| --- | --- | --- |
| +0x00 | u32 | **VERTEX-POOL offset** (relocated; ×nothing — int16 xyzw stride 8) |
| +0x04 | u32 | **VERTEX COUNT** |
| +0x08 | u32 | 2nd-pool offset (colour/normal pool, relocated) |
| +0x0e | u16 | **FLAGS**: `0x8000`=terminate/skip sub-object; `0x4000`=skip reloc; low 9 bits = prim-count addend |
| +0x10 | u32 | **PRIMITIVE-STREAM offset** (relocated) — the walk start `t0` |
| +0x14 | u16 | **PRIM count base** |
| +0x18 | u32 | 4th pool offset (relocated) |

**Primitive-record count walked** = `u16[+0x14] + (FLAGS & 0x1ff) − 1`.

- **Vertices** = the +0 pool, `int16 x,y,z,flag` stride 8 (4th int16 = flag/normal).
- The walks land EXACTLY on section boundaries. PA00 entry 2 (3 sub-objects):
  - sub0: vtx@**0x1230** ×122, prim@**0x6c** ×163 → ends at 0x1218 (just before next pool)
  - sub1: vtx@0x1a4c ×14,  prim@0x1950 ×12  → ends 0x1a38
  - sub2: vtx@0x1f54 ×44,  prim@0x1b24 ×35  → ends 0x1f34
  Every decoded index is `< pool size` (max == count−1). **Note:** the earlier doc's
  "vertices @0x1218" was off by the +0x18 relocation; the true pool base is 0x1230.
- Decode the table + export with **`tools/pa_obj.py … --entry N`** (prints the table,
  walks each sub-object, validates index ranges). Per-sub-object pools confirm that
  vertex indices are POOL-RELATIVE (each sub-object is a separate OBJ group `o subN`).

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
| 0x20 | flat | 3 | 16 | **+0x0a** |
| 0x28 | flat | 4 | 20 | **+0x0a** |
| 0x24 | textured | 3 | 24 | +0x12 |
| 0x2c | textured | 4 | 32 | **+0x16** |
| 0x34 | textured/gouraud | 3 | 28 | +0x14 (tentative) |
| 0x3c | textured | 4 | 36 | +0x14 (tentative) |

**CORRECTION (2026-06-08, visual RE via AC1mod):** every record begins with a
**per-poly running-counter halfword** (0,1,2,…); the real vertex indices follow it.
The first-index offsets were each one halfword too early for 0x20/0x28/0x2c (they
read the counter as a vertex -> in-range but wrong topology -> spiky meshes).
Skipping the counter (offsets above) makes blocks decode as **clean recognizable
solids** (AC/mech parts), validated by rendering PA00/PA20/PA40. Quads triangulate
in PSX Z/N order `(0,1,2)+(1,3,2)`, not a `(0,1,2)+(0,2,3)` fan.

Validation: every decoded index is `< pool size`, distinct per face, and the record
walk lands exactly on the next sub-section (PA00 e2 → 0x1a4c; PA20 e3 → 0x20c).

**Running-counter vs real index (RESOLVED):** in textured records a halfword
*sequential per-poly running counter* (0,1,2,…) appears in the shading region, NOT
in the vertex-index slots — the vidx offsets in the table above already point past
it at the *real* pool-relative indices (proven: at those offsets every index is
`< vtx_cnt` and `max == vtx_cnt−1`, with 0 out-of-range across PA00 e2 + PA20 e3 for
flat/textured types). The relocation handlers (jump table `0x8004B184`) confirm which
halfwords are geometry: in each handler `a1 = record+4`; halfwords shifted **`<<3`
(×8)** are XYZ-pool vertex indices, **`<<4` (×16)** are colour/normal-pool indices.
For the **gouraud** types `0x34`/`0x3c` the handlers interleave vtx/colour indices
(stride 4 from record+0x14/+0x10) but a residual ~5–30% of records still index OOR —
an extra shading word is suspected; the exporter drops any face with an OOR index so
the mesh stays valid. (0x34/0x3c vidx layout = **TENTATIVE**; needs DuckStation bp.)

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

## OBJ exporter (DONE — `tools/pa_obj.py`)
`tools/pa_obj.py GG/P0/PA00.T --entry 2 [--sub N] [-o out.obj]` decodes the stride-28
sub-object table, reads each sub-object's int16 vertex pool, walks its variable-length
primitive stream, triangulates quads, and writes a Wavefront `.obj` (one `o subN` group
per sub-object; output to `disc_map/`, gitignored). Validated:
- **PA00 entry 2:** 3 sub-objects → 180 verts / 322 tris, bbox X[-150..150] Y[-690..-515]
  Z[-222..165], **100% of verts referenced by faces** (coherent, no orphans),
  0 out-of-range face indices in the OBJ.
- **PA20 (GG/P1) entry 3:** 277 verts / 510 tris, bbox X[-111..111] Y[-675..-519]
  Z[-189..180].
- **PA00 entry 21** (21 sub-objects): 554 verts / 853 tris; all OBJ face indices valid.
Meshes are X-symmetric (±150/±111) at part scale → these blocks are AC-part / object
meshes, not whole-stage hulls. Faces with an out-of-range index (gouraud 0x34/0x3c
edge cases) are dropped so output is always a valid mesh.

## Next steps (to fully crack + render)
1. **Gouraud vidx (0x34/0x3c):** breakpoint emitter `0x8005A57C` in DuckStation to
   nail the exact vertex-index slot for these two types (handler shows interleaved
   vtx/colour but a residual ~5–30% index OOR — likely an extra shading word).
2. **Textures/UVs:** decode the UV+clut(`0x7980`)/tpage(`0x009b`) shading words into
   real CLUT/tpage coords + a TIM source so the OBJ can carry a material.
3. **Stage assembly:** entry 0 / entry 1 directory → how blocks place into a full map
   (the per-block bboxes are part-scale; the map header must position them).
4. Port walker `0x800574D8` + emitter `0x8005A57C` into PSXmod as **AC1mod**.
