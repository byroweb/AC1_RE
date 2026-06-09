# AC1mod — vision & roadmap (someday)

Goal (byron, 2026-06-08): grow the AC1 RE tooling into an **"AC1mod"** —
a PSXmod-style toolkit to **read/edit missions and maps, view models, and
re-pack** the disc, so missions can be ripped and authored from scratch.

This session's disc/`.T` work is the foundation layer for that. Capturing the
plan so it isn't lost.

## What already exists (reusable building blocks)
- **Disc file map:** `tools/build_filemap.py` → `disc_map/disc_files.json`
  (every file, exact sectors/size, from the jPSXdec `.idx`).
- **Container reader:** `tools/extract_t.py` — generalized `.T` TOC parser +
  per-entry extractor (count-first / offset-first, zero-length tolerant).
- **Text tooling:** `tools/scan_text.py` + `docs/DISC_TEXT_INVENTORY.md` —
  mission text in `MIS.T`, UI text in `FDAT.T` entry 201.
- **Inject/repack:** `psxinject` round-trip + checksum recompute
  (`REFERENCE.md` §6–7, `patch_draw_string.py`).
- **Live tools:** DuckStation MCP (95 tools) + Ghidra MCP for verification.

## Gaps to close for "rip & author missions"
1. **`.T` writer / repacker** — inverse of `extract_t.py`: rebuild a `.T` from
   edited entries, fix per-entry checksums, keep sector counts stable, then
   `psxinject` back. (Have read; need write.)
2. **MIS.T mission editor** — names (entry 0 array) + briefing/objective text
   (per-mission blocks) + thumbnail TIMs. Text path is solved; needs an
   edit/repack UI and the mission-load linkage (Phase F: `read_T_entry(MIS,…)`
   caller → on-disk mission index, and the mission *parameter* tables in lower
   FDAT entries: enemy spawns, map id, rewards, time limit).
3. **PA##.T map/model reader** — sub-type the geometry entries (TMD model packs,
   TIM textures, collision, object placement). TMD is a documented PSX format;
   KFModTool already does AC model/texture (see `project_ac1_prior_art`) — borrow
   its TMD/TIM parsing rather than redo it.
4. **Mission ↔ map ↔ asset graph** — which `PA##.T` + `MIS.T` entry + FDAT
   parameter row a given mission uses (so a "new mission" knows what to reference).
5. **Packaging** — how to ship it. Options:
   - a) extend the existing **PSXmod** project (`~/Desktop/PSXmod/Armored Core
     (v1.1).psxmod`) with AC1-aware plugins;
   - b) a standalone Python toolkit (`ac1mod/`) wrapping these `tools/` scripts +
     a small GUI/CLI;
   - c) Ghidra/DuckStation MCP-driven workflows for the live-verify loop.
   Decide when we get there; the format-RE layer is provider-agnostic.

## Compute split (where things run)
Claude Code runs on a thin laptop (i7-10510U, 15 GB RAM, integrated GPU); a
dedicated PC (Intel Arc A750, i7-12700, 64 GB RAM) on the LAN handles only the
genuinely heavy jobs.
- **Laptop (default — including the viewer):** the **AC1mod 3D viewer runs here.**
  PS1-class geometry is a few hundred polys + small textures; integrated graphics
  render it trivially — no Arc PC needed to look at models. Also: all RE/edit
  tooling, mipsel compile/inject, git, MCP control, normal-res emulation.
- **Arc PC (offload only when heavy):** local-LLM passes (`tools/llm_local.py`,
  agent `ac1-re-local-helper`), large batch jobs (mass texture decode, whole-disc
  sweeps), and enhanced/heavy DuckStation if needed.
So: build the viewer to run **on the laptop**; reach for the Arc PC only for LLM
inference or batches big enough to choke the laptop. See memory
`feedback_ac1_compute_offload`.

## Suggested next concrete step
Build the **`.T` repacker** (gap 1) — it unlocks *every* edit path (text, then
maps) and is a small, well-scoped inverse of code we already have. Then wire the
first end-to-end **MIS.T mission-text edit** as the proof of concept.
