# Stage-loader investigation (live, 2026-06-11) — narrowing where the walkable stage comes from

Goal: find how the walkable mission environment (room / bridge / oil facility / outdoor
canyon) is loaded, after disproving the `0x8004121B`-is-the-stage-selector theory.

## The `.T` file-open function
`FUN_800165E4`? No — the **open-by-path** function is **`0x80016678`** (resident exe,
not overlay-aliased). It references the `"\GG\"` path prefix (`gp+0x30`=`0x5c47475c`)
and the **MXT registry at `0x8004A2A4`** (12-byte stride). Signature ≈
`load_T_file(slot, path*)`; `a1` = the path string. `FUN_800165E4` is the per-ENTRY
reader (reads one entry from an already-open container).

## Decisive observation
Armed an execute breakpoint on `0x80016678` and played a fresh mission **into a real
walkable outdoor stage**. The breakpoint fired **exactly ONCE** the whole time:
- `a1` → `"P0\PA00.T"`, `a0=0`, **`ra = 0x8004F234`** (the PA00 bootstrap).

**No other `.T` file was opened by path** between mission start and standing in the
stage (hit_count = 1, confirmed; screenshot shows full outdoor gameplay).

→ **The walkable stage geometry is NOT a per-mission `.T` file opened by path.** It is
read **by entry-index from a container that is already open** (opened once at boot and
kept resident — e.g. FDAT/MIS or a dedicated geometry container), via the entry reader
`FUN_800165E4`, not via `0x80016678`.

## Corroborating
- The big environment geometry is **not** in the `0x8019F538` block table either — in
  the outdoor stage that table still held only the small bootstrap records (~100/212,
  150/150), same as training. So the stage env uses a different RAM structure.
- MXT registry `0x8004A2A4`: ~16 entries × 12 B = `{TOC_ptr, w1, w2}`. word0 points at
  each open container's **offset TOC** (ascending uint16 table), confirming several
  containers stay open; word0 is NOT a filename (names come from the path-builder).

## Next decisive experiment (future session)
1. **Entry-reader trace:** execute-bp on **`FUN_800165E4`** armed BEFORE a fresh stage
   load; log `a0` (mxtid/container) + `a1` (entry) for each call during the load
   transition. The stage geometry's container+entry will appear. Noisy (fires per
   entry) — trap during the loading screen, not active play.
2. **Geometry-walker trace:** execute-bp on the walker `0x800574D8` during stage load;
   its `a0` = the geometry block being processed → locate the stage env in RAM, then
   backtrack the source.
3. **Fingerprint:** dump the loaded environment geometry (once located) and match its
   int16 vertex pool against the disc files to identify the source container.

Artifacts: `mxt_names.bin` (registry/TOC dump). Breakpoint cleared after the test.
