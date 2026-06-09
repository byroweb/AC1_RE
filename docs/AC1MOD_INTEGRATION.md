# AC1mod — integrating the PA viewer into PSXmod

How the cracked PA##.T geometry work (this repo) plugs into the **PSXmod** PyQt6 app
to become **AC1mod**, a disc browser + PS1 stage/model viewer. Companion to
`docs/AC1MOD_VISION.md` and `docs/PA_FORMAT.md`.

## Target app (surveyed 2026-06-08)
`~/Desktop/PSXmod` — PyQt6 desktop app (now a **git repo**; renamed to **AC1mod**).
- `main.py` (~2000 lines): `MainWindow` + `DetailPanel`. `DetailPanel` is a
  `QStackedWidget` with **page 0 = image (TIM)**, **1 = text**, **2 = hex**;
  `DetailPanel.show_entry(entry)` (main.py ~L948) routes by `_entry_is_text` /
  `_entry_is_hex` / `is_audio`, else falls through to the image page.
- `core/jpsxdec.py` — disc index wrapper (jPSXdec `.idx` → `IndexEntry` list, same
  index format we parse in `tools/build_filemap.py`).
- `core/project.py`, `core/workers.py` — project file + background workers.
- `core/pa_parser` exists **only as a stale `.pyc`** (no source) — replace it.
- Deps: PyQt6, Pillow. **`PyQt6.QtOpenGLWidgets` is available**; PyOpenGL /
  pyqtgraph are not installed.

## Plan — 3 pieces

### 1. `core/pa_parser.py` (port from this repo)
Port the confirmed decode from `tools/pa_parse.py` (+ the forthcoming
`tools/pa_obj.py`): `.T` TOC read → size-prefixed block → per-sub-object vertex
pools (int16 x,y,z) + variable-length primitive records (`reclen = 4+byte[1]*4`,
`type = byte[3]&0xBC`, pool-relative uint16 indices). Output a plain
`Mesh{vertices:[(x,y,z)], faces:[(i,j,k[,l], color/uv)]}` dataclass — engine-agnostic
so the viewer (and an OBJ exporter) share it. **Single source of truth:** keep the
authoritative decoder in this repo's `tools/` and copy/vendor it into `core/`, or
import it; do not fork the logic.

### 2. `ModelView3D` widget (new page 3 in `DetailPanel`)
PS1 geometry is tiny (hundreds of polys), so this is **laptop-trivial**
(`feedback_ac1_compute_offload`). Two options:
- **(recommended v1) QPainter software renderer** — zero new deps. Project verts
  with a simple perspective matrix, **painter's-algorithm average-Z sort** (mirrors
  the game's own OT avg-Z sort, `0x8005A57C`), fill flat polys with the record's
  vertex colour. Mouse-drag = orbit, wheel = zoom. ~150 lines, robust, no GL context
  headaches. Good enough to "see the stage".
- **(v2 upgrade) `QOpenGLWidget`** — for textured/smooth/large scenes later.
Wire-in: add the widget as `self._stack` index 3; in `show_entry`, detect PA entries
(`entry.name`/path matches `P[0-3]/PA\d\d\.T` or `_entry_is_pa(entry)`) and route to
it, loading the block via `core/pa_parser`.

### 3. Branding / packaging (DONE for branding)
- App renamed **PSXmod → AC1mod** (window title, label, `applicationName`); project
  dialogs accept `*.ac1mod *.psxmod` (existing `.psxmod` projects still load). The
  one remaining "PSXmod" is the intentional fork attribution in the header comment.
- Future: per-sub-object list (a stage = many sub-objects), texture display using the
  PA TIMs once UV/tpage bitfields are confirmed, and OBJ/GLTF export from the same
  `Mesh`.

## Dependencies / sequencing
- **Blocked on** the `re/pa-obj-export` agent task: it resolves the stride-28
  sub-header (per-sub-object vertex-pool boundaries) and the running-counter vs.
  real-index question, and produces `tools/pa_obj.py`. The `Mesh` builder in
  `core/pa_parser.py` should reuse exactly that decode. Do not implement the viewer's
  geometry decode before that lands, or it'll disagree with the validated one.
- After it lands: (a) port → `core/pa_parser.py`, (b) add `ModelView3D` (QPainter
  v1), (c) hook `show_entry`, (d) manually verify a stage renders as a coherent shape.

## Verification
- `python3 tools/pa_obj.py …` produces a mesh whose `f` indices are all valid and
  whose shape is recognizable (open it in any OBJ viewer / Blender).
- In AC1mod: select a `PA##.T` geometry entry → page 3 shows an orbitable mesh that
  matches the OBJ. Cross-check ≥2 stages.
