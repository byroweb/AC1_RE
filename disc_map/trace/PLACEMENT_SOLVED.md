# Level-section PLACEMENT — SOLVED (2026-06-12)

The open problem from [PLACEMENT_INVESTIGATION.md](PLACEMENT_INVESTIGATION.md) is closed. **Per-section placement is
TRANSLATION-ONLY, stored in a per-mission table on disc (FDAT entry 2N+1, chunk 7).**
The geometry blocks are PRE-ROTATED on disc; sections are placed by adding a world
translation. Blocks are REUSED across sections (instancing — e.g. Nassau Tunnel's east
leg = one straight-segment block placed 5 times). Verified by assembling Rescue
Transport Truck offline: produces the exact X-with-curved-legs layout (matches the
in-game briefing map). 56 levels batch-assembled (`assemble_levels.py`).

## On-disc format (FDAT entry 2N+1 chunk stream `[u32 len][payload]`)

- **chunk 0**: level geometry blocks (PA format, size-prefixed, LOCAL coords).
- **chunk 7 @ +0**: the SECTION PLACEMENT TABLE, 52-byte records, terminator: s16 `-1` at +6:

| off  | type   | meaning |
|------|--------|---------|
| +0x00| s16[3] | P0 — world bbox MIN of placed section |
| +0x06| s16    | cell/PVS index; **-1 = end of table** |
| +0x08| s16[3] | P1 — world bbox MAX |
| +0x10| s16[3] | **P2 — PLACEMENT TRANSLATION** (`world = block_local + P2`) |
| +0x18| s16[3] | copy of P0 |
| +0x1e| s16    | **GEOMETRY BLOCK INDEX** (into chunk 0; reused → instancing) |
| +0x26| s16    | per-section zone/portal index |
| +0x28| u8     | lighting-record index |
| +0x2a| u16    | flags (runtime: word +0x30 = flags<<16 \| light byte) |

At load the table is expanded to **64-byte records @ `0x801D0B68`**; block pointers
(post-relocation) at **`0x801A6344`** (RTT: 38 ptrs into 0x800B7..0x800D5).

## The real level RENDER path (previously unmapped — none of the object paths)

Found by READ-watching a section vertex (pause lands inside the renderer):

| what | addr |
|------|------|
| visibility walk / visible-list builder | caller of `0x800639D8` (ra=`0x8005D0BC`), reads static table 0x801D0B68 (stride 64; pos +0/+8/+0x10, flags +0x30) |
| per-frame visible list (stack, `0x801FEDxx`) | 24-byte records: composed trans (3×s32 @ +0), variant k (s16 @ +12), descr-offset 0xc/0x28 (@ +14), static-section ptr (@ +16), lighting-record ptr (@ +20) |
| level section renderer | `0x800639D8` (per sub-list), per-section GTE setup @ `0x80063A60` |
| per-section rotation | `rt = [0x801AD6CC + k*32]`, k∈{0..3} — FOUR camera-composed variants 90° apart, composed ONCE per frame |
| per-section light matrix | `llm = lighting_record + k*20` (4 pre-rotated variants) |
| fog/colors | lighting record +0x50 (fog pair → `0x80028234`), +0x62..64 (RGB → `0x80028394`) |
| vertex transform loop | `0x80063B9C` (lwc2 v0 / rtps / swc2 sxy2) |
| **GTE scale note** | composed rotation is **1/8-scaled** so ±28000-unit level coords fit GTE |

## Lighting table (the 0x801ad00c "rotation LUT" demystified)

The load-time builder at **`0x80056958`** copies 32 × 44-byte source records from fixed
staging `0x800B1248` into 32 × 104-byte records @ **`0x801AC964`**:
src +0..19 = GTE light matrix → dst +0; +20/24/28 = packed fog/level pairs → +0x50;
+38 = RGB color → +0x62; then **`0x80056A54`** derives 3 more 90°-rotated light-matrix
variants per record (via `0x800135E8(src,dst,n)`) at +0x14/+0x28/+0x3C.
The "4-orientation rotation LUT @ 0x801ad00c" from the old investigation was record 16's
variants — it is the LIGHTING table, not geometry placement. Variant k matches the
render-time k. (Nassau: records 15–24 form the tunnel-depth brightness/fog gradient.)

## Mission→scene mapping surprises

- **Destroy Gun Emplacement (Chrome Gun Emplacement)** loads its whole scene (lighting +
  14 geometry blocks) from **FDAT entry 200** (282KB) — NOT a 2N+1 pair. e200 = a common/
  shared scene pack; its 14 blocks are also resident during other missions (table @
  `0x801A5FF0`). Entries 41/49/57 (m20/24/28) have only 4 chunks — they likely reference
  shared scenes the same way. **Open**: where the mission→shared-scene selection happens.

## What stays open (minor)

- Static source of render-variant k per section (k=0 observed for all sections of one
  tunnel; mechanism for picking k per section unconfirmed — placement itself needs no k).
- Lighting table's on-disc chunk for normal missions (staged at 0x800B1248 by the scene
  loader from ~0x800F6xxx; exact chunk not pinned).
- The dual descriptor sets per block (+0xc vs +0x28 in visible records) — likely
  opaque/translucent or LOD passes.
- e1 m0 (4 sections), e97/e99/e121/e141 (tiny) — special/training scenes, unverified.

## Object/AC rendering (mapped en route, for completeness)

- AC player: 124-byte part-instance records (RTT: 45 @ `0x800A1238`): mesh idx s16 @
  +0x12, 9×s16 rotation @ +0x28, position s16-in-word @ +0x3C/40/44; renderer
  `0x80065AA8` loop; part pack @ `0x80110000+` (descriptor table `0x801104C8`,
  28-byte descriptors at +12; runtime prim vertex indices are ×16 byte-offsets).
- Objects/MTs: dispatcher `0x8005DEC4` (pos = 3×s16 at struct +0, camera pos @
  `0x801AD690`), ApplyMatrix `0x80027FC4` → `0x800B123C` (camera scratch `0x800B1228` —
  ruled-out approach #7 was reading the CAMERA, mystery explained), cull/sub-object
  renderer `0x80057C44` (same 124-byte record layout, s0 = rec+0x38).

Tools: `assemble_levels.py` (→ `disc_map/levels_assembled/`, 56 levels, world coords,
floor/ceiling/wall groups + top-down PNGs). Supersedes `batch_extract_levels.py`'s
unplaced output.
