# Mission load sequence — live trace via entry reader (2026-06-11)

Mission: **Rescue Transport Truck** (a tunnel). Execute-bp on the `.T` entry reader
`FUN_800165E4(container, entry, dest)`; walked the first 10 reads. `ra` = caller.

| # | container | entry | dest | caller (ra) | meaning |
|---|-----------|-------|------|-------------|---------|
| 1 | 1 | 101 (0x65) | 0x8004ADA0 | 0x800121F8 | resident loader — early mission/overlay setup |
| 2 | **2 (FDAT)** | **202 (0xCA)** | 0x8004ADA0 | 0x80012510 | **mission overlay code** (matches MXT_LOADER) |
| 3 | 1 | 100 (0x64) | 0x800B17C8 | 0x8004F598 | mission scene data (per-mission, 2N) |
| 4 | 1 | 33 (0x21) | 0x800B17C8 | 0x8004F5B4 | scene component |
| 5 | 2 (FDAT) | 200 (0xC8) | 0x800B17C8 | 0x8004F5D4 | shared FDAT resource |
| 6 | **2 (FDAT)** | **66 (0x42)** | **0x801C4B40** | 0x8004F78C | **objective object** (2N, matches MISSION_SYSTEM) |
| 7 | **2 (FDAT)** | **67 (0x43)** | 0x800B74D8 | 0x8004F7AC | **chunk stream** (2N+1) |
| 8 | **0 (PA00)** | 1 | 0x8010140C | 0x8004F24C | bootstrap: PA00 placement directory |
| 9 | 0 (PA00) | 0 | 0x8010140C | 0x8005365C | bootstrap: PA00 master header |
| 10 | 0 (PA00) | 2 | 0x8011140C | 0x8005365C | sub-resource load (item C: byte0+2 → e2) |

## The real loader (finally)
The mission **scene loader** = overlay `FUN_8004F508` (calls visible at `0x8004F598 /
F5B4 / F5D4 / F78C`). It orchestrates the whole load through the resident entry reader
**`FUN_800165E4`**. The PA00 bootstrap (`0x8004F1A8`/`0x8005365C`) is just one part of it.

## Container architecture (mxtid in a0)
- **container 0 = PA00** (common assets — AC/MT parts, props, effects; loaded via the
  bootstrap + the 8 sub-resource selectors. Common to EVERY mission.)
- **container 1 = per-mission scene data** (entries ~100/101 + components like 33).
- **container 2 = FDAT** (overlay code @202, objective @2N=66, chunk stream @2N+1=67).

## Where the walkable environment comes from (narrowed, not yet pinned)
PA00 (container 0) is common, so the **tunnel/room/bridge environment is NOT a dedicated
per-mission "stage PA file."** It must come from the **mission-specific data**: container
1 (scene data, entry ~100) and/or the FDAT chunk stream (entry 2N+1=67). No single giant
"environment geometry" read appeared in the first 10 reads — consistent with the
environment being **embedded in / referenced by the chunk stream**, then assembled by the
overlay. (Container indices 1 vs 2 number missions differently — FDAT 66/67 vs container-1
100/101 — so don't read a single mission number off either.)

## To pin it (offline / next)
1. Map mxtid 1 → disc file (catch its open-by-path `0x80016678` at boot, or match the
   MXT registry `0x8004A2A4`). Container 2 = FDAT (already extractable).
2. Extract container-1 entry 100 and FDAT entry 67, look for geometry (sub-object tables /
   primitive streams per PA_FORMAT) vs pure script/spawn data.
3. Or re-trap `FUN_800165E4` and capture the RETURN size (v0) per read to spot the large
   environment-geometry read directly.
