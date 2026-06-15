# AC1 scripted movement & AI paths

Target **SLUS-01323 (v1.1)**. Static RE (offline disasm). Source: `scratch/re/ai_paths.md`.
Companions: `MISSION_SCRIPT_VM.md` (the chunk-4 VM that hosts scripted motion),
`ENTITY_AI_FRAMEWORK.md` (think handlers).

There are **two** ways an object moves on a path:

## 1. Scripted lerp motion (data-authorable, via the secondary VM)

Most scripted moves (entrances, simple patrols, object slides) are **chunk-4 actor
threads** (`MISSION_SCRIPT_VM.md`) — no bespoke code. The motion primitive:

- **Lerp engine `FUN_800276F4`** (base exe): GTE 12-bit fixed-point linear interpolation
  `out = (a2·src + a3·dst) >> 12` via `GPF(0x3d)`/`GPL(0x3e)` with `sf=1`. The VM feeds
  `a2 = t`, `a3 = 0x1000 − t`, `src = scratch+8` (snapshot), `dst = copy_pos operands`.
- Progress `t = ((total − counter) << 12) / total`; `total`/`counter` at thread record
  `+0x0E`/`+0x10`, counter down-counts 1 per frame. **Motion runs dst→src**: the object
  starts at the `copy_pos_lo` operand pose and ends at the `set_xyz` pose.
- Units: positions are raw **world int16** (same space as entity `+0x08/0A` and the
  placement table). `set_loop` duration is in **frames (60/s)**. Interpolation is
  **linear, no easing**. The mission timer `0x8019F52C` is NOT consulted.

Authoring a path = a thread of `sel_actor; set_xyz <start>; copy_pos_lo <end>; set_loop
<frames>` (chain `copy_pos_lo; set_loop` legs for multi-waypoint patrols). The `0x8000`
ref-pose form of `set_xyz` snaps to another placed object's pose (`0x801A8B08+idx*100`).

**Corrected 24-byte thread record** (re-derived from loader `0x8008C178`; supersedes
`scratch/re/secondary_vm.md` §1): `+0x0C`=thread_hdr, `+0x0E`=total, `+0x10`=counter/state.

Worked examples (byte-exact): M0 thread@12 = 2 s straight entrance; M8 thread@18 = 2-leg
patrol (chained `copy_pos_lo;set_loop`); M6 thread@20 = ref-pose lerp between markers.

## 2. Bespoke path-follower (the train, "on rails")

Some actors run a dedicated think handler instead of lerp threads. The train
(mission 8 / `016.bin`, think `0x801CBD44`, installed by that mission's objective object)
uses its **own route table**, NOT chunk-4 marks:

- Route table at `[entity+0x108] + 0x4E`, **40-byte route-segment records, stride 0x28**.
- Current segment index `entity+0x131`; authoritative route position
  `entity+0x10C/+0x10E/+0x110`.
- Mover `FUN_801CABA4` → `FUN_80073C2C` (collision-aware step-toward-target) → writes
  per-frame velocity `entity+0x76/+0x78/+0x7A`.
- Segment-type dispatch: `0`/`0x80` = move, `1` = fire (`FUN_80077B78`), `2` = aux.

So "on rails" = a per-segment move-toward-target with collision, advancing through the
route table. (Other bespoke handlers — aircraft patterns — are per-mission think funcs in
the objective object; not yet individually decoded.)

## Open / confirm-live
- `init_unit` (0x000A) hw1–5 field meanings (seeds entity velocity `+0x76/78/7A`, id `+0x65`).
- Route-segment fields `+0x12..0x1A` and flags `+0x02`; `thread_hdr==1` END final-snapshot.
- No real example exercises `copy_pos_hi` (rotation track) — needs one.
- Best next: DuckStation mission 8 — breakpoint lerp tail `0x8008BC8C` and route commit
  `0x801CAD44`; watch `entity+0x08` and `entity+0x10C` to confirm lerp direction + train step.
