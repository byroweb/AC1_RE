# RAM-watch concordance — external sources vs. this project

**Living document.** A side-by-side **concordance table** of every published external
RAM location for Armored Core (PS1) against this project's own Ghidra + DuckStation
findings. Purpose: *convergent validation* (independent methods agreeing on an address
is strong evidence both are right) and *gap-finding* (an address one side has and the
other lacks is a lead, not noise).

Target: **SLUS-01323 (v1.1)**. External offsets are NTSC MainRAM raw offsets
(`0x1A_xxxx`); add `0x8000_0000` for the KSEG0 addresses used here (e.g. `0x1A26B8` =
`0x801A26B8`).

Companion docs: [RUNTIME_RAM_MAP.md](RUNTIME_RAM_MAP.md) (the in-mission RAM map +
the BizHawk-gist cross-check), [ENTITY_AI_FRAMEWORK.md](ENTITY_AI_FRAMEWORK.md)
(entity struct & array), [COMBAT_PHYSICS.md](COMBAT_PHYSICS.md) (how fields were found).

## Sources

| key | who / what | method | provenance |
| --- | --- | --- | --- |
| **TAS** | TASVideos *GameResources/PSX/ArmoredCore* RAM-watch table | black-box RAM watch | <https://tasvideos.org/GameResources/PSX/ArmoredCore> |
| **GIST** | Zinfidel "PSX Armored Core Overlay Scripts for BizHawk" | black-box RAM watch (Lua) | <https://gist.github.com/Zinfidel/d47287cec638f0f915c3d99bccef3a7d> |
| **REPO** | this project | white-box (Ghidra static + DuckStation live) | repo `docs/` |

> **Note:** TAS and GIST are both attributed to Zinfidel's lineage but are **not
> identical** — e.g. they disagree on the position offset (TAS reads s16 at `+0x08`,
> GIST reads a 3×s16 vector at `+0x20`). Treat them as two external observations.

Status legend: **✅ confirmed both** (independent agreement) · **⊃ repo generalizes**
(their specific row is an instance of the project's model) · **lead** (external only —
worth chasing) · **⚠ conflict** (addresses agree but interpretation differs — needs
live reconcile).

---

## A. Entity array (`0x801A26B8`, stride `0x170`; idx0 = player)

The external "player / enemy 1 / enemy 2" rows are **slots 0 / 1 / 2** of the single
entity array documented in this repo. Verified: idx1 = base+`0x170` = `0x801A2828`,
idx2 = base+`0x2E0` = `0x801A2998`.

| Concept | TAS (addr · type) | REPO (offset in entity) | Status |
| --- | --- | --- | --- |
| Entity array model | — (per-enemy slots only) | base `0x801A26B8`, stride `0x170` | ⊃ |
| Type / ID | Enemy1 `1A2828` u16 · Enemy2 `1A2998` u16 | `+0x00` (read as u32 ptr → template tbl) | ⚠ |
| Active flags | Enemy1 `1A282A` u16 · Enemy2 `1A299A` u16 | `+0x02` (= high half of the `+0x00` ptr) | ⚠ |
| Position X | Player `1A26C0` · Enemy1 `1A2830` s16 | `+0x08` | ✅ (X), ⚠ (width) |
| Position Y | Player `1A26C2` · Enemy1 `1A2832` s16 | `+0x14` (s32; Z is at `+0x0C` here) | ⚠ |
| Position Z | Player `1A26C4` · Enemy1 `1A2834` s16 | `+0x0C` (s32) | ⚠ |
| Yaw / facing | Player `1A26CA` s16 (= `+0x12`) | `+0x12` (HYPOTHESIS) | ✅ corroborates |
| HP / AP | Enemy1 `1A2988` · Enemy2 `1A2AF8` u16 | `+0x160` (u16) | ✅ |

Cross-validation arithmetic (all exact):
`1A2828 − 1A26B8 = 0x170` · `1A2998 − 1A26B8 = 0x2E0` · `1A2988 − 1A2828 = 0x160` ·
`1A2AF8 − 1A2998 = 0x160` · active-flags & pos offsets identical across slots 1 and 2.

### Predicted (external labels × the repo stride — unverified)
| field | formula | address |
| --- | --- | --- |
| Player HP/AP | idx0 `+0x160` | `0x801A2818` (✅ already confirmed elsewhere) |
| Enemy 1 yaw | idx1 `+0x12` | `0x801A283A` |
| Enemy 3 ID / HP | idx3 `+0x00` / `+0x160` | `0x801A2B68` / `0x801A2CC8` |

---

## B. Global / low-RAM state (not in the entity array)

| Concept | TAS (addr · type) | REPO | Status |
| --- | --- | --- | --- |
| RNG | `009010` u32 | red-AC AI RNG call `0x8002A4A0` (consumer; state word not yet tagged) | **lead** |
| Justice (final boss) HP | `1D9ED6` u16 | `0x801D_xxxx` = the **world-structure / destructible** region (`0x801D0B68`), not the entity array | **lead** |
| Corp Points counter | `041B38` s8 | `0x8004_xxxx` = career/progression block (near mission-result `0x80048610`, stage byte `0x8004121B`) | **lead** |

Implications:
- **RNG `0x80009010`** is in fixed main-exe BSS (`0x8000_xxxx`) → **global, persistent
  across overlay swaps**. A read-watchpoint here enumerates every stochastic consumer
  (aim spread, fire timing, spawn variation) in one pass.
- **Justice HP** sits in the `0x801D_xxxx` world-object region, *not* the AC array →
  bosses are likely implemented as **world-structure objects** (HP in a sub-struct,
  cf. `OBJECT_STATS.md`), not mobile AC entities. (`1D9ED6 − 1D0B68 = 0x936E`, not a
  `0x40` multiple → a separate large allocation in that family, not a destructible-array row.)
- **Corp Points** confirms a low-RAM **progression** block distinct from per-mission
  entity RAM (`0x801A_xxxx`) and the mission-state overlay block (`0x8019F5_xx`).

---

## C. Frame counters (there is more than one clock)

| TAS (addr · type) | role (this project) | Status |
| --- | --- | --- |
| `198804` u16 | engine/global tick (also GIST `FRAME_COUNTER_1`) | HYPOTHESIS |
| `19F52A` u16 | **mission-state block** — 2 bytes before the mission timer `0x8019F52C`, after success/fail flags `0x8019F524` | ✅ tightens block |
| `1AC81C` u16 | alternate/subsystem tick (also GIST `FRAME_COUNTER_2`) | HYPOTHESIS |

`0x8019F5xx` mission-runtime state struct (consolidated): `…F524` success/fail flags ·
`…F52A` frame counter (TAS) · `…F52C` mission timer (this project).

---

## D. Open reconciliations (resolve live next emulation session)

1. **Position width/offset (⚠).** Three readings of "position": TAS s16 at
   `+0x08/+0x0A/+0x0C`; REPO s32 at `+0x08`(X)/`+0x0C`(Z)/`+0x14`(Y); GIST 3×s16 vector
   at `+0x20`. Likely a high-precision integrator vector **and** a separate render/GTE
   vector coexist. Confirm which the game treats as authoritative (watchpoint the one the
   movement integrator `0x8004F024` writes).
2. **`+0x00` / `+0x02` (⚠).** The repo reads `+0x00` as a u32 pointer to the template
   table; TAS reads it as u16 "ID" + u16 "active flags". The low halfword of the template
   ptr varies by entity type (→ looks like an "ID" to a black-box watcher); the high
   halfword is `0x8019` (constant → *not* a flag). Decide whether `+0x00`/`+0x02` are
   genuinely a separate id/flags pair or an artifact of reading a pointer as two shorts.
3. **Array cap 15 vs 16.** Damage router bounds-check = `[base, base+0x1590)` = **15
   slots**; the radar scan loop reportedly iterates 16. Read the exact loop-count
   immediate to settle the true maximum.

---

## How to update this doc

1. New external row → add to the right source table (A/B/C) with its raw offset, type,
   and the project's equivalent (or `—`). Pick a status from the legend.
2. New convergence (independent agreement) → mark **✅** and add the subtraction proof.
3. Resolved an open item → move it out of §D into the relevant table with **✅**/**⊃**
   and a one-line note on how it was confirmed; keep the address arithmetic.
4. Keep "predicted" rows separate until verified; promote them when checked live.
5. Mirror any newly *confirmed* struct field into `ENTITY_AI_FRAMEWORK.md` /
   `RUNTIME_RAM_MAP.md` so the struct tables stay the single source of truth.

_Last updated: 2026-06-14 — initial build from the TASVideos GameResources table._
