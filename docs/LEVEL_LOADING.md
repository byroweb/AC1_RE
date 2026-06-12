# AC1 Level Loading & Placement — mission select to on-screen geometry

Target **SLUS-01323 (v1.1)**. The consolidated answer to "how does Armored Core 1
load its levels?" — from the mission menu to walkable geometry on screen, every hop
live-verified in DuckStation (2026-06-11/12). Companion docs: [MISSION_SYSTEM.md](MISSION_SYSTEM.md)
(objective code, spawns), [PA_FORMAT.md](PA_FORMAT.md) (geometry block internals), [MXT_LOADER.md](MXT_LOADER.md)
(.T containers). Trace logs + breadcrumbs: [PLACEMENT_SOLVED.md](../disc_map/trace/PLACEMENT_SOLVED.md),
[mission_load_sequence.md](../disc_map/trace/mission_load_sequence.md), [LEVELS_FOUND.md](../disc_map/trace/LEVELS_FOUND.md).

Notation: `CONFIRMED` = live-verified in the running game and/or byte-verified on
disc. `HYPOTHESIS` = inferred, not yet ground-truthed.

---

## TL;DR — the whole pipeline (CONFIRMED)

```
mission select (menu)
  └► scene loader FUN_8004F508
       ├► bootstrap: FUN_8004F1A8 opens P0\PA00.T (COMMON object/AC/MT/effect pack —
       │   loaded EVERY mission; stage byte 0x8004121B is NOT a level selector)
       ├► entry reader FUN_800165E4(container, entry, dest) pulls FDAT.T entries:
       │     2N   = objective object (per-mission MIPS code → 0x801C4B40)
       │     2N+1 = the SCENE chunk stream  [u32 len][payload]…
       │            chunk 0  geometry BLOCKS (PA format, local coords, PRE-ROTATED)
       │            chunk 7  SECTION PLACEMENT TABLE (translation-only, 52B records)
       │            chunk 12 object/MT spawn table (256×40)
       │   (lighting source is staged to fixed RAM 0x800B1248)
       ├► geometry relocation FUN_80053848 → blocks in work RAM,
       │   block pointer table 0x801A6344; placement table expands to 0x801D0B68
       └► lighting build 0x80056958/0x80056A54 → 0x801AC964
            (104B records: fog + RGB + light matrix ×4 pre-rotated variants)

every frame:
  visibility walk (caller of 0x800639D8) over the static section table 0x801D0B68
    └► stack list of visible sections (24B records: composed translation,
        orientation variant k, section ptr, lighting ptr)
         └► level renderer 0x800639D8: per section ctc2 rotation = 0x801AD6CC+k*32
             (4 camera-composed variants, 1/8-scaled), translation = composed,
             vertex loop 0x80063B9C (lwc2/rtps) → GPU packets
```

**The placement headline:** a level is a bag of reusable geometry sections
(tunnel segments, room pieces) plus a table that stamps them into the world by
**pure translation**. Sections are instanced — Nassau Tunnel's straight east leg is
one block placed 5 times. Rotation never appears in placement: differently-oriented
copies are stored as distinct pre-rotated blocks on disc.

---

## 1. Where a level lives on disc (CONFIRMED)

`GG/COM/FDAT.T` (27 MB, count-first .T container; mxtid map by registry
`0x8004A2A4`: **0 = PA00** 880KB common pack, **1 = RTIM** 17MB textures,
**2 = FDAT**). Mission N uses the entry pair `2N` / `2N+1`; the walkable level is
inside **entry `2N+1`**, a chunk stream of `[u32 len][payload]` records:

| chunk | content |
|------:|---------|
| 0 | **geometry blocks**: from payload +8 up to `u32[0]`, a run of size-prefixed PA-format blocks (see `PA_FORMAT.md`; decode with `tools/pa/pa_obj.py`). LOCAL coordinates, pre-rotated. |
| 1, 2 | geometry-handler siblings (relocated alongside; LOD/extra mesh data) HYPOTHESIS |
| 7 | **section placement table** (below) — at chunk offset 0 |
| 12 | object/MT **spawn table** (256×40-byte records; see `MISSION_SYSTEM.md`) |
| others | scripts/PVS/portal data (chunk 7's tail beyond the table), unexplored |

56 of the mission entries carry a chunk-7 table (3–505 sections each). Spawn
coordinates (chunk 12) and section translations share the same world space —
verified by overlaying spawns on assembled levels in the companion AC1mod viewer (separate repo).

### The placement table (chunk 7, CONFIRMED)

52-byte records, table ends at the first record with s16 `-1` at +6:

| off | type | meaning |
|-----|------|---------|
| +0x00 | s16[3] | P0 — world bbox MIN of the placed section |
| +0x06 | s16 | cell/PVS index; **-1 = table terminator** |
| +0x08 | s16[3] | P1 — world bbox MAX |
| +0x10 | s16[3] | **TRANSLATION** — `world = block_local + this` |
| +0x18 | s16[3] | copy of P0 |
| +0x1e | s16 | **geometry block index** into chunk 0 (reused → instancing) |
| +0x26 | s16 | zone/portal index |
| +0x28 | u8 | lighting-record index (into the 104B lighting table) |
| +0x2a | u16 | flags (runtime keeps `flags<<16 \| light` at record +0x30) |

At load the table expands to 64-byte records at `0x801D0B68`; relocated block
pointers land at `0x801A6344` (e.g. Rescue Transport Truck: 38 blocks).

Validation: assembling Rescue Transport Truck (e67: 38 blocks, 62 sections)
offline reproduces the exact X-with-curved-legs layout of the in-game briefing
map. `disc_map/trace/assemble_levels.py` batch-assembles all 56 levels.

---

## 2. Per-section lighting (CONFIRMED)

Per mission, 32 × 44-byte source records are staged at fixed RAM `0x800B1248`;
the builder `0x80056958` copies them into 32 × 104-byte records at `0x801AC964`:

- +0x00: GTE **light-direction matrix** (3 lights; e.g. straight-down sun
  `[0,-4096,0]` + two fills) — `0x80056A54` then derives three more 90°-rotated
  variants (via `0x800135E8(src,dst,n)`) at +0x14/+0x28/+0x3C
- +0x50: packed fog near/far pairs → `0x80028234` at draw time
- +0x62: RGB light color → `0x80028394`

Sections reference a lighting record by index (+0x28 of the placement record);
graded record runs implement effects like Nassau Tunnel's depth-darkening
(records 15–24 fade color 0x48→0xb8 with matching fog).

*History note:* these 4 rotated light matrices were earlier mistaken for a
"4-orientation geometry rotation LUT @ 0x801ad00c" — that address is record 16's
variants inside this table.

---

## 3. The level render path (CONFIRMED, previously unmapped)

The level does NOT go through the object renderers (`0x80057C44` cull loop,
`0x8005DEC4` dispatcher — those draw the AC, MTs and props). It has its own path,
found by read-watching a section vertex:

| step | addr | detail |
|------|------|--------|
| visibility walk | caller of `0x800639D8` (ra `0x8005D0BC`) | per frame, walks the static section table `0x801D0B68` (stride 64: pos +0/+8/+0x10, flags +0x30), frustum/PVS tests, emits a stack list |
| visible record (24B) | stack `0x801FEDxx` | +0 composed translation (3×s32), +12 orientation variant k (s16), +14 block-descriptor offset (0xc/0x28 — dual sets, likely opaque/translucent), +16 section-record ptr, +20 lighting-record ptr |
| GTE setup | `0x80063A60` | `rt = [0x801AD6CC + k*32]` — only **4 rotation matrices**, camera-composed ONCE per frame, 90° apart; `llm = lighting_record + k*20`; fog/color from the lighting record |
| vertex loop | `0x80063B9C` | `lwc2 v0 / rtps / swc2 sxy2` over the section's vertex pool |
| scale trick | — | composed rotations are **1/8-scaled** so ±28000-unit world coordinates survive the GTE's 16-bit IR clamps |

Open: which static field selects k per section (k=0 observed for one whole tunnel;
placement itself needs no rotation, so this only matters for exactness of the
lighting variant). HYPOTHESIS: derived from the section's quadrant/flags.

---

## 4. Shared scenes — the exception (CONFIRMED existence, mechanism open)

Not every mission stores its own scene. **Destroy Gun Emplacement** ("Chrome Gun
Emplacement") loads its entire arena — lighting + 14 geometry blocks — from
**FDAT entry 200** (282 KB, an 8-chunk shared scene pack whose blocks stay
resident during other missions too; runtime table `0x801A5FF0`). Its mission
entries (and e41/e49, the other 4-chunk odd entries) carry no chunk-7 table.
HYPOTHESIS: the per-mission objective code (entry 2N) or a scene-id field selects
the shared pack. e200's internal layout differs (geometry from +0x28; lighting at
+0x448b4 = its chunk 7) and is not yet fully decoded.

---

## 5. Corrections trail (for readers of older docs)

1. **PA##.T files are NOT the stages.** All 72 are object/AC/MT/effect packs;
   PA00 is a common bootstrap bundle loaded every mission. The stage byte
   `0x8004121B` is a bootstrap selector, not a mission-level selector.
   (`PA_FORMAT.md` banner, `mission_stage_map.md` retraction.)
2. **Levels are not world-positioned in the blocks** (early LEVELS_FOUND draft) —
   blocks are local; placement = the chunk-7 translation table.
3. **Placement is not rotation+translation** (PLACEMENT_INVESTIGATION) — blocks
   are pre-rotated on disc; placement is translation-only. The "4 orientations"
   live in the render/lighting path, not in placement data.
4. **Floor/ceiling convention:** PSX Y is DOWN — a walkable floor's front face
   normal points -Y. (Earlier extractor tagged these inverted.)

---

## 6. Tools & viewer

| tool | does |
|------|------|
| `disc_map/trace/assemble_levels.py` | batch-assembles all 56 levels from FDAT → OBJ (`o floor/ceiling/wall` groups) + top-down PNGs (the PNG renders are derived analysis imagery; the OBJ geometry is extracted game data — keep it local, don't redistribute) |
| `tools/pa/pa_obj.py` | PA geometry block decoder (gouraud 0x34/0x3c stride-4 RESOLVED — see `PA_FORMAT.md`) |
| `tools/mission/mission_parse.py` | spawn table / chunk stream inspection |
| companion AC1mod viewer (separate repo) | `core/level.py` + CLI `level N --no-ceiling --spawns`; GUI Missions window has Level/Ceilings toggles; spawns render in-place on the level |

---

## 7. Remaining open items

- Shared-scene selection mechanism (e200 ↔ missions m20/m24/m28-style entries).
- Static source of the render orientation variant k per section.
- Chunk 1 (large geometry sibling) and chunk 7's post-table tail (PVS/portals).
- e200's internal header layout (geometry at +0x28, multi-descriptor blocks).
- Texturing: UV/CLUT decode exists for objects (`PA_FORMAT.md`) but levels render
  untextured in the viewer so far.
