# `GG/P0–P3/PA00–PA71.T` — geometry asset packs (IDENTITY UNDER REVISION)

Target: **SLUS-01323 (v1.1)**. **72 files** (`PA00.T`..`PA71.T`) across `GG/P0..P3`,
disc sectors **103637–126704**, ~700–900 KB each.

> **CORRECTION (2026-06-11):** these were long labelled "3D stage/map packs," but live
> RE shows that's **not established**. `PA00.T` is loaded as a **common/bootstrap bundle
> at every mission start** (via `FUN_8004F1A8`, stage byte `0x8004121B`=0), and its
> entries decode as **AC/mech parts, props, and effect/sprite objects** — plus the PA
> viewer shows **mission-assignment-screen imagery** in PA files. Meanwhile a write-watch
> on `0x8004121B` never fired across an entire real mission (Reclaim Oil Facility), and
> three visibly-different missions shared byte-identical descriptors — so the walkable
> **stage environment is a separate, not-yet-identified subsystem**, NOT these PA files
> (at least not PA00). Caveat: the older "scene assembly" note below saw PA40 merge into
> a facility-like floor plan, so SOME PA files may still hold level-scale geometry — the
> per-file role is genuinely unresolved. Treat everything below as a validated **geometry
> container/primitive format** (which is solid and file-verified), NOT proof that these
> files are the playable levels. Resolving identity = the live `.T`-loader trace
> (`FUN_800165E4`) during a real stage load. See [mission_stage_map.md](../disc_map/trace/mission_stage_map.md).

## Container (CONFIRMED)
Standard count-first `.T` container, identical mechanism to King's Field (verified
against KFModTool `core/tfile.cpp`): `uint16[0]` = entry count, then `count+1`
uint16 sector offsets (×2048), duplicate offsets = zero-length entries.
- `PA00.T`: 176 entries, 114 non-empty.
- **No text** (probed PA00/20/40/60: 0 English words). Geometry only.
- Tools: `tools/extract/extract_t.py` (extract), `tools/pa/pa_parse.py` (structure dump).

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
| +0x04 | u16 | **VERTEX COUNT** (CORRECTION 2026-06-15: this is **u16**, not u32 — see +0x06) |
| +0x06 | u16 | param (nonzero on the "articulated" block variant; e.g. PA00 e60). Reading +0x04 as u32 swallows this and massively over-reads the vertex count. Byte-exact cross-checked vs `tools/pa/pa_encode.py` (7602/7602). `pa_obj.py` fixed accordingly. |
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
- Decode the table + export with **`tools/pa/pa_obj.py … --entry N`** (prints the table,
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
| 0x24 | textured | 3 | 24 | +0x12 (stride 2) |
| 0x2c | textured | 4 | 32 | **+0x16** (stride 2) |
| 0x34 | gouraud tri | 3 | 28 | **+0x12, stride 4** (RESOLVED) |
| 0x3c | gouraud quad | 4 | 36 | **+0x16, stride 4** (RESOLVED) |

### Textured-primitive MATERIAL words (CONFIRMED 2026-06-15, emitter `FUN_8005A57C`)

For textured types the shading region is a **verbatim PSX texture-poly material**: the
emitter `sw`s the on-disk word straight into the GPU `POLY_FT*/GT*` packet (no
transform), so the on-disk word **is** the GPU CBA/tpage field. Record-relative offsets
(record = the word whose byte3 is the type; `family = type & 0xFD`):

| type | packet | reclen | CLUT word | TPAGE word | UV0/1/2[/3] (u,v bytes) | colour | vtx0 | vtx stride |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 0x24 | FT3 | 24 | +0x06 | +0x0a | +0x04/05, +08/09, +0c/0d | GTE-lit | +0x12 | 2 |
| 0x2c | FT4 | 32 | +0x06 | +0x0a | …+10/11 | GTE-lit | +0x16 | 2 |
| 0x34 | GT3 | 28 | +0x06 | +0x0a | +04/05,+08/09,+0c/0d | GTE ×3 | +0x12 | 4 |
| 0x3c | GT4 | 36 | +0x06 | +0x0a | …+10/11 | GTE ×4 | +0x16 | 4 |
| 0xa4 | FT3 (unlit) | 28 | +0x06 | +0x0a | +04/05,+08/09,+0c/0d | rec+0x10 | +0x14 | 2 |
| 0xac | FT4 (unlit) | 32 | +0x06 | +0x0a | …+10/11 | rec+0x14 | +0x18 | 2 |

CLUT/TPAGE occupy the **high halfword** of the UV0/UV1 words. `u,v` are unsigned 0..255
texel coords within the page. (Dispatch `type & 0xFD`; six handlers `0x8005A744..AE14`;
matrix-variant emitter `0x80058B04` is identical. Full disasm proof in
`scratch/re/pa_textures.md`.)

**Decode formulas (standard PSX, cross-checked vs `tools/farsi/font_render.c`):**
```
CLUT  word: cx = (clut & 0x3F) << 4   (16px X granularity);  cy = clut >> 6   (1px Y)
TPAGE word: px = (tpage & 0xF) * 64;   py = ((tpage>>4)&1) * 256;
            abr = (tpage>>5)&3 (blend);  bpp = (tpage>>7)&3  (0=4bpp,1=8bpp,2/3=15bpp)
```
Observed PA00 markers: CLUT `0x7980`→VRAM (0,486); TPAGE `0x009b`→page (704,256) 8bpp.
Dominant PA00 pages: `0x009c`(768,256) ×7370, `0x009b`(704,256) ×2002, `0x0037`(448,256,
4bpp) ×874. **Re-texturing** an existing prim = overwrite the CLUT word (+0x06), TPAGE
word (+0x0a) and the UV bytes; everything else is topology (then recompute the `.T`
checksum). `pa_encode.py` preserves all of this byte-exact.

**Texture image source:** `GG/COM/RTIM.T` is the common TIM bank (63 Sony TIMs, each
with its own VRAM dest rect) — supplies UI/effect/AC pages (tx 5/8/9/13/15, e.g. page
(448,256)). **CONFIRMED negative:** RTIM does **not** fill PA00's dominant stage pages
(704/768,256); a separate **per-stage** texture bank does. The PA loader `FUN_8004F1A8`
has **no** `LoadImage`/GPU-DMA call — geometry load and texture upload are decoupled.

**Per-stage texture bank — LOCATED on disc (2026-06-15):** the bank is **embedded in
the PA container itself**, not in a sibling file (the `P0..P3` dirs hold only `PA##.T`).
In `PA00.T` the two **non-geometry** entries carry it:
- **entry 0** — a fixed **0x10000-byte (64 KB)** block, `self0` ≠ `len` so it is *not* a
  geometry block; header is a small descriptor (`+0x00` used-size `0xf854`, `+0x0c`
  count `0x1c`=28, then internal offsets `0x188 / 0xedc4 / 0xefd4`). Rendering it raw as
  8bpp or 16bpp is **noise**, so the pixels are inside a **structured/encoded sub-container**
  (offset table at `+0x1c`, regular `08 xx 08 yy` records), not a flat VRAM image.
- **entry 1** — a **~2 KB** block whose body is an **ascending u16 offset table**
  (`0x72,0x82,0xa2,0xea,0x10a,0x152,…`) → a **CLUT/palette directory** (matches the 8bpp
  pages the geometry references).

This matters because **~88% of PA00 faces are textured** (21572 textured vs 2810 flat;
the flat faces use just 2 colours), so a stage cannot be rendered correctly from geometry
alone — the entry-0 bank is required. OPEN: (a) crack the entry-0 sub-container encoding,
and (b) confirm its VRAM upload destination — one DuckStation `LoadImage` trace at mission
load nails the destination (see `scratch/re/pa_textures.md` §4). Offline alternative for a
VRAM source is an **in-mission** DuckStation save state (`tools/duckstation/savestate.py`),
but all 10 current backup states are menu/garage (`stage_byte=0`), so none carry stage VRAM.

**Gouraud stride (RESOLVED 2026-06-11, live RE):** flat/textured types pack their
vertex indices contiguously (stride 2). The **gouraud** types interleave
`[normal_idx, vertex_idx]` pairs, so the vertex indices are **stride 4** (a per-vertex
normal-pool index sits between each). Reading them contiguously (the old `+0x14`
guess) picked up the ×8 normal indices as vertices → out of range. See
`disc_map/trace/gouraud_slot_resolved.md`.

**CORRECTION (2026-06-08, visual RE via the companion AC1mod viewer):** every record begins with a
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
For the **gouraud** types `0x34`/`0x3c` the handlers interleave `[normal, vertex]`
index pairs: vertex slots (shifted `<<4`) are at **record+0x12/0x16/0x1a** (tri) and
**record+0x16/0x1a/0x1e/0x22** (quad) — i.e. first vertex at +0x12 (tri) / +0x16
(quad), **stride 4**; the per-vertex normal index sits 2 bytes before each. **RESOLVED
2026-06-11** by disassembling the live relocation handlers (`0x80057674` tri /
`0x800577a4` quad) in the running game; with the stride-4 layout, out-of-range gouraud
faces drop from ~8% to **0 across all 72 PA files** (175,232 records). Validation:
`disc_map/trace/gouraud_validate.py`; details in `disc_map/trace/gouraud_slot_resolved.md`.

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
(extract with `tools/extract/extract_t.py`; not in the entry-201 Ghidra DB).
- **`FUN_8004F1A8`** builds the path from template `"P0\PA00.T"` (@`0x8008D928`)
  using stage byte `DAT_8004121B`, then `load_T_file(0, path)` — **PA loads into
  file slot 0** — and `read_T_entry(0, 1, dest)` to grab entry 1 (offset directory)
  before the geometry blocks (entries 2..N).
- **`FUN_80073AD8`** registers each block: copies it to mxt work RAM and stores a
  **44-byte record** at table `0x8019F538` (`+0x28`=ptr, `+0x00`/`+0x02`=counts from
  `half[block+4]>>2` / `half[block+6]>>2`, `+0x04`=index). This CONFIRMS the block
  header's halfword count fields are real geometry element counts.

## OBJ exporter (DONE — `tools/pa/pa_obj.py`)
`tools/pa/pa_obj.py GG/P0/PA00.T --entry 2 [--sub N] [-o out.obj]` decodes the stride-28
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
1. ~~**Gouraud vidx (0x34/0x3c):**~~ **DONE 2026-06-11** — RE'd live from the relocation
   handlers `0x80057674`/`0x800577a4`: gouraud verts are stride-4 (interleaved with
   per-vertex normal indices), first vertex at record+0x12 (tri) / +0x16 (quad). 0
   out-of-range across all 72 PA files. Fixed in `tools/pa/pa_obj.py` (and ported into
   the companion AC1mod viewer's `core/pa_parser.py`, separate repo).
2. ~~**Textures/UVs:** decode the UV+clut/tpage shading words~~ **DONE 2026-06-15**
   (see "Textured-primitive MATERIAL words" above): offsets, decode formulas, and the
   RTIM bank are confirmed. Only the per-stage texture bank that fills pages (704/768,256)
   is open (one live `LoadImage` trace).
3. **Stage assembly:** entry 0 / entry 1 directory → how blocks place into a full map
   (the per-block bboxes are part-scale; the map header must position them).
4. Port walker `0x800574D8` + emitter `0x8005A57C` into the companion AC1mod viewer (separate repo).


## Scene assembly (world-coordinate finding, 2026-06-08)
> **CAVEAT (2026-06-11):** this section calls the merged result "the stage," but per the
> top-of-file correction that identity is unproven — PA00 is a common/asset bundle, and
> the real walkable stage loads elsewhere. The PA40 "facility floor plan" below is
> suggestive that *some* PA files hold level-scale geometry, but it has NOT been
> cross-checked against the actual in-game environment. Read "stage" here as "the
> world-coord geometry in this PA file," pending the live `.T`-loader trace.

A PA file's **environment geometry blocks are authored directly in WORLD
coordinates** — merging them unmodified reconstructs the actual stage layout
(verified: PA40 assembles into a bilaterally-symmetric facility floor plan; top-down
view shows a clear designed level). Blocks whose bbox exceeds ~±12000 are the
**object / MT / effect slots** (incl. the universal e125+ gouraud "effect" slots):
they share a LOCAL origin and are positioned **per-instance by mission data** (object
template + entry-1 placement vectors, see docs/PA_HEADER.md / docs/MISSION_SYSTEM.md),
not by their own coords. The companion viewer's `scene_mesh()` / CLI `--scene` merges the in-range
(world-coord) blocks to show the stage; precise MT placement awaits the mission-runtime
decode.


## Sub-object internal anatomy (deep dive 2026-06-08)
A geometry block is a list of **sub-objects** (the parts of an articulated object, or
the pieces of a stage). Each 28-byte sub-object descriptor (table at
`block + block[+8] + 12`; all offsets below relative to that reloc base):

| off | type | field |
| --- | --- | --- |
| +0x00 | u32 | vertex-pool offset |
| +0x04 | u16 | vertex count (u16; +0x06 = u16 param — see correction above) |
| +0x06 | u16 | param (articulated-variant; nonzero e.g. PA00 e60) |
| +0x08 | u32 | **normal-pool** offset (pool2) |
| +0x0c | u16 | **normal count** |
| +0x0e | u16 | flags (0x8000 = skip/terminate) |
| +0x10 | u32 | primitive-stream offset |
| +0x14 | u16 | primitive count (base; + (flags&0x1ff) − 1) |
| +0x16 | u16 | pool4 count/param |
| +0x18 | u32 | pool4 offset (small, ~8 B/sub-object; purpose TBD) |

On-disk order within a sub-object: **[primitive stream][vertices][normals][pool4]**.
- **Vertices** = int16 `x,y,z,flag`, 8 B each.
- **Normals** = int16 unit vectors, 8 B each, **every vector length == 4096** (PSX
  fixed-point 1.0) — CONFIRMED on PA00 e2 (103 normals) + e19. There are MORE normals
  than vertices (e.g. 30 normals / 22 verts) → **per-face-corner normals for smooth
  (Gouraud) lighting**. The renderer had been ignoring these and flat-shading from
  computed face normals; the models actually ship full lighting data.
- The textured/gouraud primitive records carry both a vertex index (into the ×8
  vertex pool) and a colour/normal index (into the normal pool) — see
  `docs/PA_HEADER.md` walker notes.

**Takeaway:** the big object blocks (21 sub-objects) are **complete articulated,
fully-lit MT/AC models**, not loose triangles — vertices + per-corner normals +
primitives per part. Still open: pool4's purpose, per-vertex colour vs normal index
split, and the per-stage texture-image bank (material word decode is now done above;
the upload source for stage pages 704/768,256 awaits a live `LoadImage` trace).
