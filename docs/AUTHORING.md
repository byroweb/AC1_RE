# AC1 mission authoring — pipeline & status

Target: SLUS-01323 (v1.1). Master doc for the authoring/definition-side work tracked
in the plan "Attacking the authoring/definition-side gaps". This is the *write* side
(building/editing mission data on disc), complementing the read/RE docs
(`MISSION_SYSTEM.md`, `OBJECT_STATS.md`, `MISSION_STAGES.md`, `PA_FORMAT.md`).

## The edit→run loop (CONFIRMED end-to-end, offline portion)

The read→edit→repack→reinject chain is proven (`scratch/phase0_slice.py`,
PASS 2026-06-15): editing a confirmed spawn field lands on a bootable disc with all
FDAT checksums intact (the NOW-LOADING hang is the checksum, handled by t_repack).

```
read   tools/extract/t_repack.read_flat_from_disc(fm, rec)  # de-sector FDAT.T from pristine .bin
       t_repack.unpack(flat) -> Container (205 entries)
edit   tools/mission/spawn_edit.set_field(stream, rec, field, value)   # mission N stream = entry 2N+1
repack t_repack.replace_entry(c, 2N+1, new_stream)          # recomputes that entry's checksum word
       t_repack.pack(c) -> flat .T                          # must stay same byte length (in-place)
reinject tools/disc/reinject.py --file GG/COM/FDAT.T --t <flat.T> --out <test>.bin
run    boot <test>.cue in DuckStation
```

### Worked example (the Phase-0 slice)

Mission 0 (chunk-stream = FDAT entry 1), spawn record 4 (type 34): moved `x` -26 -> 6000.

```
python3 scratch/phase0_slice.py
# -> [SLICE].bin + .cue in AC_1_USA_test/, value confirmed on re-extract, 0 bad checksums
```

CLI equivalents:
```
tools/mission/mission_parse.py --fdat disc_map/extracted/FDAT_T/entries --mission 0 --spawns
tools/mission/spawn_edit.py --stream <stream.bin> --rec 4 --field x --set 6000 -o <edited.bin>
tools/disc/reinject.py --file GG/COM/FDAT.T --t <rebuilt.T> --out "<test>.bin"
```

## Tools (write side)

| tool | role |
| --- | --- |
| `tools/extract/t_repack.py` | de-sector / unpack / `replace_entry` / `pack` / checksum (90/90 round-trip) |
| `tools/mission/spawn_edit.py` | read/set a chunk-12 spawn field (x/y/z/blk/rot/type/hwN) in a stream |
| `tools/mission/mission_script.py` | secondary-VM (chunk 4) **disassembler + assembler**; `--selftest` = 56/56 byte-exact round-trip (the script authoring write-path) |
| `tools/mission/type_catalog.py` | cross-mission object-type catalog → `disc_map/type_catalog{,_long}.csv` |
| `tools/mission/mission_parse.py` | chunk walk + spawn dump; now prints per-chunk ROLE labels |
| `tools/re/disasm_batch.py` | batch objdump+m2c of `ADDR:LEN[:NAME]` targets → `scratch/re/disasm/` |
| `tools/pa/pa_encode.py` | PA geometry **encoder** (byte-exact parse↔encode, **7602/7602** blocks; `--translate` edit demo) — the geometry write-path |
| `tools/mission/mission_geom.py` | per-mission geometry (chunk-0 PA blocks): parse/edit/splice; `--selftest` **59/59 streams, 1740/1740 blocks** byte-exact |
| `tools/disc/reinject.py` | write a rebuilt flat `.T` back into a `.bin` at the file's sector span (in-place) |
| `tools/farsi/build_rtl_patch.py` | reference: original FDAT reinsert + overlay-patch flow |

## Decoded systems (RE docs)

- `MISSION_SCRIPT_VM.md` — the chunk-4 actor-thread scripting language (22 opcodes,
  blob/set/thread format, assembler spec). Drives scripted spawns, movement/lerp,
  timed cues, and the data-driven mission result.
- `ENTITY_TYPES.md` — type→behaviour binding (per-mission objective object @
  `0x801C4B40`, INIT column installs `+0x58` think), the two type namespaces
  (model id `hw7` vs logical dispatch type), and the win/fail condition taxonomy.

Disc images (`disc_map/disc_files.json` -> `source_bin`):
- pristine: `~/Desktop/Armored_Core_Hacks/AC_1_USA_backup/Armored Core (v1.1).bin` (read from here)
- test outputs: `~/Desktop/Armored_Core_Hacks/AC_1_USA_test/…[TAG].bin` (write here)

## Constraints / gotchas

- **Checksum:** every edited `.T` entry needs its trailing word recomputed (seed
  `0x12345678` + word sum); `replace_entry` does this. Wrong checksum = infinite
  NOW-LOADING (see `MXT_LOADER.md`).
- **In-place vs grow:** `reinject.py` overwrites sectors in place when the rebuilt `.T`
  fits the original sector span (fast; spawn/field/script edits keep size constant). If
  it **grows** (new geometry, added chunks/threads), `reinject.py` auto-switches to a
  **`psxinject` rebuild** (or force with `--grow`) — `psxinject <image+.cue> <iso_path>
  <new_content>` replaces the file and rebuilds the filesystem. Verified: edit lands,
  0 bad checksums. (`psxinject` needs a matching `.cue`; `reinject.py` writes one.)
- **Mode2/2352:** disc is MODE2/2352, user data at byte offset 24 per 2352-B sector.

## Status vs. plan

- [x] Phase 0 vertical slice — offline round-trip PASS; in-game confirmation pending (DuckStation).
- [~] Phase 1 spawn fields — chunk-12→template field map being traced statically
  (`FUN_80073B74`); HP/AI semantics need live ground-truth. Spawn editor done.
- [x] Phase 2 mission VM / objectives — both VMs decoded; secondary VM (dis)assembler
  round-trips 56/56; objective conditions taxonomized (`MISSION_SCRIPT_VM.md`,
  `ENTITY_TYPES.md`). Remaining: `init_unit` inline fields, prompt-cmd live trigger.
- [x] Phase 3 AI path/waypoint — DECODED (`AI_PATHS.md`): scripted motion = secondary-VM
  lerp (engine `FUN_800276F4`); bespoke "rails" = per-actor route table (train). Remaining:
  `init_unit` fields, live lerp-direction confirm.
- [x] Phase 4 collision — DECODED (`COLLISION.md`): **no separate collision data**, it is
  derived from the render mesh (object swept-AABB + world per-face). So **no collision
  encoder needed** — editing geometry/placement suffices. (One integrator attribution is
  disputed vs `COMBAT_PHYSICS.md` — live confirm.)
- [x] Phase 5 type→behaviour — binding mechanism + per-mission tables decoded;
  dispatch type = `hw3`, resource id = `hw7`, HP = `hw11` (`spawn_marshal`/`ENTITY_TYPES.md`);
  `type_catalog.py` covers 59 missions. Remaining: numeric type→think for all missions.
- [x] Encoders / write-path: **PA geometry encoder** (`pa_encode.py`, 7602/7602; fixed a
  latent `pa_obj.py` over-read), **per-mission geometry** (`mission_geom.py`, 59/59 +
  1740/1740), **size-growth disc rebuild** (`reinject.py --grow` → psxinject). Only
  **OBJ→PA import** is deferred (build new geometry from scratch; editing existing
  geometry works now). Collision needs no encoder (Phase 4).
- [x] **Capstone (offline)**: `scratch/capstone.py` authors a mission-0 variant editing
  **four subsystems at once** — geometry + spawn position + HP + mission script — repacks,
  reinjects, and verifies every edit on re-extract with 0 bad checksums →
  `…[CAPSTONE].bin`. Live in-game load is the only step left (DuckStation batch).
