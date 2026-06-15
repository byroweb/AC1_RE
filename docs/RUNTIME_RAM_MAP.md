# Runtime RAM map — in-mission AC entity array, camera, counters

Target **SLUS-01323 (v1.1)**. Live main-RAM (KSEG0, `0x8000_0000`+) addresses for the
in-mission gameplay state. Companion to [COMBAT_PHYSICS.md](COMBAT_PHYSICS.md) (how
these fields were found and what the code does with them).

## Independent corroboration (Zinfidel)

A full side-by-side concordance of every external RAM-watch row (this gist **and** the
TASVideos GameResources table) against this project's findings lives in
[RAMWATCH_CONCORDANCE.md](RAMWATCH_CONCORDANCE.md).

Several of these addresses are independently corroborated by **Zinfidel's "PSX Armored
Core Overlay Scripts for BizHawk"**
(<https://gist.github.com/Zinfidel/d47287cec638f0f915c3d99bccef3a7d>) — `ac_core.lua`,
`ac_radar.lua`, `ac_speedo.lua`, `ac_wallhack.lua`. Those scripts read the **MainRAM**
domain as **raw NTSC offsets** (`0x1A_xxxx`); convert to the KSEG0 addresses used here
by adding `0x8000_0000` (e.g. his `0x001A26B8` = `0x801A26B8` here). It is an independent
RE (BizHawk RAM-watch vs. this project's Ghidra + DuckStation), so agreement is real
cross-checking, not a shared source.

## AC entity array (in-mission)

- **Base = `0x801A26B8`** — entity index 0 = the **player AC**. **CONFIRMED** (both:
  the player-transform trace, `s3 = 0x801A26B8`; Zinfidel `ENTITY_DATA`).
- **Stride = `0x170` (368 B)**. **CONFIRMED**: enemy AC index 1 sits at `0x801A2828`
  (`= base + 0x170`, the method-table-zero live test) and Zinfidel `ENTITY_OFFSET = 0x170`.
- This is **not** the same structure as the `0xD0`-stride / 510-slot object pool at
  `0x801A25E8` documented in [COMBAT_PHYSICS.md](COMBAT_PHYSICS.md) §1. That `0xD0` pool
  is the **menu/garage** frame-loop pool (`0x8009BD64`), which is *not resident during a
  mission*. The in-mission AC entities use the `0x170` stride above. The two regions
  overlap at the player address but are indexed differently. **Open:** how the `0x170`
  AC array and the `0xD0` pool relate at runtime (re-verify live).

### Per-entity offsets

| off | meaning | type | status |
| --- | --- | --- | --- |
| `+0x00` | ptr → MT/instance template table (`0x8019FAB8`) | u32 | CONFIRMED (⚠ TAS reads as u16 id `+0x00` + u16 flags `+0x02` — reconcile) |
| `+0x04` | ptr → geometry-block record table (`0x8019F538`) | u32 | CONFIRMED |
| `+0x08` | world position **X** (int32; constant moving straight) | s32 | CONFIRMED |
| `+0x0C` | world position **Z** (int32; integrates forward) | s32 | CONFIRMED |
| `+0x12` | **yaw / facing** | s16 | HYPOTHESIS (Zinfidel `ENTITY_YAW`; re-verify) |
| `+0x14` | world position **Y / height** (`0x210000` at rest) | s32 | HYPOTHESIS |
| `+0x20` | position **vector** X/Y/Z | 3×s16 | HYPOTHESIS (Zinfidel `ENTITY_POS`; re-verify) |
| `+0x6A` | AABB collision **width** | s16 | HYPOTHESIS (Zinfidel `ENTITY_AABB_WIDTH`) |
| `+0x6C` | AABB collision **height** | s16 | HYPOTHESIS (Zinfidel `ENTITY_AABB_HEIGHT`) |
| `+0x160` | **AP / health** (`= 0x801A2818` for player) | u16 | CONFIRMED (both; HUD 4408 ⇔ `17632` = ×4) |

> **Position offset to reconcile (open):** the project's trace reads the authoritative position as
> int32 at `+0x08` (X) / `+0x0C` (Z); Zinfidel reads a 3×s16 position vector at `+0x20`.
> These are likely different representations (high-precision integrator vs. a render/GTE
> vector) — confirm live which the game treats as authoritative.

## Camera

- Camera struct ~`0x801AD66C` — position vector `0x801AD66C`, 3×3 matrix at `0x801AD67C`
  (Zinfidel `ac_wallhack.lua`). **HYPOTHESIS** (re-verify). Corroborates the camera
  cluster (`0x801AD690` / `0x801AD6CC`) noted elsewhere — same struct, different sub-fields.

## Frame counters

- `0x80198804` (s16) and `0x801AC81C` (s16) — game/alternate frame counters (Zinfidel
  `FRAME_COUNTER_1/2`). **HYPOTHESIS** (re-verify). Useful as a stable tick for
  frame-stepped RAM diffs.
