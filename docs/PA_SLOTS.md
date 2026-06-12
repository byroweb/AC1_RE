# PA##.T slot structure (cross-file empirical analysis)

Companion to `docs/PA_FORMAT.md`. PA files are not arbitrary bundles — every PA file
follows a **fixed slot skeleton**: a given `.T` entry index ("slot") holds the same
*kind* of object in every file, with per-stage data. Found by analysing all 72 files
with `tools/pa/pa_slots.py` (→ `disc_map/pa_slots.csv`). The *semantics* of each slot
(what the game does with it) are being cracked from the code side in `docs/PA_HEADER.md`.

## Numbers
- 72 PA files, **146 distinct geometry slots**, **84 present in all 72 files**.
- Slot classes: **126 FIXED** (same sub-/face-count in every file that has them,
  bytes differ → a fixed role filled with per-stage data) + **20 VARIABLE**
  (counts differ across files). **0** are byte-identical across *all* 72 (but many
  are identical within a category).

## Two file categories
| Category | # | Files | What |
| --- | --- | --- | --- |
| **stage-only** | 24 | PA01-03,06, PA40-47, PA57-65, PA69-71 | NO big meshes (slots e10-e24 absent) — pure stage/map environment geometry |
| **object** | 48 | the rest | contain the **big 21-sub-object meshes** (e10-e24) |

The 48 "object" files split by a **stride-3 phase** — which of every 3 interleaved
big-mesh slots they fill (e.g. phase 0 → slots 10,13,16,19,22):
| phase | big slots | # files | e.g. |
| --- | --- | --- | --- |
| 0 | 10,13,16,19,22 | 12 | PA00, PA07, PA20 |
| 1 | 11,14,17,20,23 | 12 | PA05, PA09, PA21 |
| 2 | 12,15,18,21,24 | 24 | PA04, PA08, PA16 |

## Slot map (by region, with visual ID from the companion AC1mod viewer)
| Slots | present in | subs | faces | class | looks like |
| --- | --- | --- | --- | --- | --- |
| **e2-e4** | 24 files | 3 | ~210-296 | FIXED | chunky blocky structures (buildings / terrain pieces) |
| **e10-e24** | 12-24 ea | **21** | 386-644 | FIXED | the big **mech / AC-like assemblies** (21 parts; render as upright bipedal-ish forms) |
| **e44-e53** | most | 3 | 93-121 | FIXED | small 3-part objects (props?) |
| **e60-e65** | **all 72** | 6 | VARIABLE | small 6-part objects |
| **e125-e165** | **all 72** | 6 | VARIABLE | **render as near-degenerate slivers** → likely 2D/sprite/effect billboards or shadows present in every stage (a prim type the mesh renderer collapses) |

(The big e10-e24 meshes are the ones earlier mistaken for "all AC parts" — they're a
fixed model slot reused across the object-category files, which is exactly why every
such file looked the same: we were defaulting to the largest slot.)

## What this means
- A PA file = a **fixed-layout resource bank** for a mission: common slots (present
  in all files) + category-specific slots (stage env vs. big object models) +
  per-stage variations. The **stride-3 phase** suggests the big models come in
  variant sets and each mission pulls one phase's worth.
- For the viewer / annotation: the *interesting per-stage* content is the **VARIABLE**
  slots and the stage-only files' geometry; the FIXED slots are shared templates.

## Open (resolve with the code side — `docs/PA_HEADER.md`)
- What the **entry-0 master header** (`0x03072d39` + object-index list) and **entry-1
  directory** say about slot roles — do they name/place slots, confirming the fixed
  skeleton is data-driven vs. hardcoded?
- Identity of the **universal sliver slots** (e125+) — sprites/effects? Needs the
  prim-type / sprite-primitive decode.

  PARTIAL: e125+ are **gouraud-heavy** (types 0x34/0x3c), and gouraud
  records **interleave vertex + per-vertex-colour indices** — the 4 quad vertices are
  at byte offsets 0x14,0x18,0x1c,0x20 (stride 4, skipping the colour halfword), not a
  contiguous run. Decoding that way recovers ~150 faces, but some sub-objects still
  yield outlier coords (±32640) → these universal slots use a different sub-object
  layout (special effect/sky/sprite objects?). Full decode needs the header semantics
  + likely a DuckStation emitter trace.
- Map slot index → game object type (AC part? MT? prop? effect?) once the header
  semantics are known.

Reproduce: `python3 tools/pa/pa_slots.py`  (→ `disc_map/pa_slots.csv` + this summary).
